#include "lua_wrapper.hpp"
#include "script_ui.hpp"

#include <league_internals/offsets.hpp>

#include <game_overlay/log.hpp>
#include <game_overlay/skerm.hpp>
#include <game_overlay/radial_menu.hpp>

#include "setup.hpp"

#include <set>

namespace LeagueController
{
	extern GameOverlay::LogCategory g_scriptLog;
	using InterfacePtr = GameOverlay::SourceHandler::Source::Ptr;
	static std::shared_ptr<GameOverlay::SourceHandler> g_sourceHandler;

	struct InterfaceInstance
	{
		InterfaceInstance(int inX, int inY, int inWidth, int inHeight, const char* uiFile) :
			x(inX),
			y(inY),
			width(inWidth),
			height(inHeight)
		{
			file = g_sourceHandler->CompileFile(uiFile, [this](InterfacePtr inSource, Spek::File::Handle file, Spek::File::LoadState state)
			{
				assert(state == Spek::File::LoadState::Loaded);
				auto namespaceName = inSource ? inSource->Output.Name : "Unknown";
				auto fileName = inSource->FileHandle->GetName();
				GameOverlay_LogDebug(g_scriptLog, "LoadUI CompileFile result -> %s File = 0x%x 0x%x (Namespace: %s, File: %s)", state == Spek::File::LoadState::Loaded ? "Loaded" : "Not Loaded", file.get(), inSource.get(), namespaceName.c_str(), fileName.c_str());
				source = inSource;
			});
		}

		bool IsLoaded() const 
		{
			return source && file && file->GetLoadState() == Spek::File::LoadState::Loaded;
		}

		bool IsVisible() const { return visible && IsLoaded(); }
		int GetWidth() const { return IsFullscreen() ? GameOverlay::GetWidth() : width; }
		int GetHeight() const { return IsFullscreen() ? GameOverlay::GetHeight() : height; }
		int GetX() const { return IsFullscreen() ? 0 : x; }
		int GetY() const { return IsFullscreen() ? 0 : y; }
		bool IsFullscreen() const { return x < 0 || y < 0 || width < 0 || height < 0; }
		const InterfacePtr GetSource() const { return source; }
		const Spek::File::Handle GetFile() const { return file; }

		void SetVisible(bool newValue)
		{
			// GameOverlay_LogDebug(g_scriptLog, "%s %s", newValue ? "Showing" : "Hiding", source ? source->Output.Name.c_str() : "Unknown");
			visible = newValue;
		}

		void SetFullscreen(bool newValue) { if (!newValue) return; x = -1; y = -1; width = -1; height = -1; }
		void SetWidth(int inWidth) { width = inWidth; }
		void SetHeight(int inHeight) { height = inHeight; }
		void SetX(int inX) { x = inX; }
		void SetY(int inY) { y = inY; }

		Skerm::ASDF::UIContainer* GetContainer(const char* name)
		{
			if (!IsLoaded())
				return nullptr;

			for (const auto& container : source->Output.Elements)
				if (container->Name == name)
					return container.get();
			return nullptr;
		}

		bool operator<(const InterfaceInstance& other) const
		{
			return file < other.file;
		}
	
	private:
		InterfacePtr source = nullptr;
		Spek::File::Handle file = nullptr;

		bool visible = true;
		int x;
		int y;
		int width;
		int height;
	};
	using InterfaceInstancePtr = std::shared_ptr<InterfaceInstance>;

	static std::set<InterfaceInstancePtr> g_uiFiles;
	static std::vector<GameOverlay::RadialMenu> g_radialMenus;

	InterfaceInstancePtr LoadUI(const char* uiFile, int x, int y, int width, int height)
	{
		if (g_sourceHandler == nullptr)
			g_sourceHandler = std::make_shared<GameOverlay::SourceHandler>(GetLauncherLocation());

		// Check if this UI is already instantiated
		auto existingFile = g_sourceHandler->GetSourceByName(uiFile);
		if (existingFile != nullptr)
			for (auto& file : g_uiFiles)
				if (file->GetSource() == existingFile)
					return file;

		InterfaceInstancePtr instance = std::make_shared<InterfaceInstance>(x, y, width, height, uiFile);
		g_uiFiles.insert(instance);

		Spek::File::Update();
		return instance;
	}
	
	InterfaceInstancePtr LoadFullscreenUI(const char* uiFile)
	{
		return LoadUI(uiFile, -1, -1, -1, -1);
	}

	Skerm::ASDF::UIContainer* GetChildByName(Skerm::ASDF::UIContainer* parent, const char* name)
	{
		if (!parent)
			return nullptr;

		for (const auto& child : parent->Children)
			if (child->Name == name)
				return child.get();
		return nullptr;
	}

	Skerm::ASDF::UIText* ToText(Skerm::ASDF::UIContainer* element)
	{
		if (!element || element->Type != Skerm::ASDF::UIType::TextType)
			return nullptr;
		return static_cast<Skerm::ASDF::UIText*>(element);
	}

	Skerm::ASDF::UIImage* ToImage(Skerm::ASDF::UIContainer* element)
	{
		if (!element || element->Type != Skerm::ASDF::UIType::ImageType)
			return nullptr;
		return static_cast<Skerm::ASDF::UIImage*>(element);
	}

	Skerm::ASDF::UIContainer* ToContainer(Skerm::ASDF::UIContainer* element)
	{
		if (!element || element->Type != Skerm::ASDF::UIType::ContainerType)
			return nullptr;
		return static_cast<Skerm::ASDF::UIContainer*>(element);
	}

	void SetMenuItemFunction(GameOverlay::RadialMenu::MenuItem& item, sol::function func)
	{
		if (func.valid() == false)
		{
			item.onClick = nullptr;
			return;
		}

		item.onClick = [func](const GameOverlay::RadialMenu::MenuItem& item)
		{
			func(item);
		};
	}

	void AddUIScriptSystem(sol::state& state)
	{
		state.set_function("LoadFullscreenUI", &LoadFullscreenUI);
		state.set_function("LoadUI", &LoadUI);

		state.new_usertype<InterfaceInstance>
		(
			"InterfaceInstance",

			"x", sol::property(&InterfaceInstance::GetX, &InterfaceInstance::SetX),
			"y", sol::property(&InterfaceInstance::GetY, &InterfaceInstance::SetY),

			"width", sol::property(&InterfaceInstance::GetWidth, &InterfaceInstance::SetWidth),
			"height", sol::property(&InterfaceInstance::GetHeight, &InterfaceInstance::SetHeight),
			
			"visible", sol::property(&InterfaceInstance::IsVisible, &InterfaceInstance::SetVisible),
			"loaded", sol::property(&InterfaceInstance::IsLoaded),
			"fullscreen", sol::property(&InterfaceInstance::IsFullscreen, &InterfaceInstance::SetFullscreen),

			"GetContainer", &InterfaceInstance::GetContainer
		);

		state["UIType"] = state.create_table();
		state["UIType"]["ContainerType"] = Skerm::ASDF::UIType::ContainerType;
		state["UIType"]["ImageType"] = Skerm::ASDF::UIType::ImageType;
		state["UIType"]["TextType"] = Skerm::ASDF::UIType::TextType;

		state.new_usertype<Skerm::ASDF::UIContainer>
		(
			"UIContainer",

			"type", sol::property(&Skerm::ASDF::UIContainer::Type),
			"name", sol::property(&Skerm::ASDF::UIContainer::Name),
			"children", sol::property(&Skerm::ASDF::UIContainer::Children),
			"GetChildByName", &GetChildByName,
			"asText", sol::property(&ToText),
			"asImage", sol::property(&ToImage),
			"asContainer", sol::property(&ToContainer)
		);

		state.new_usertype<Skerm::ASDF::UIText>
		(
			"UIText",
			sol::base_classes, sol::bases<Skerm::ASDF::UIContainer>(),
			
			"content",  &Skerm::ASDF::UIText::Content,
			"fontFile", &Skerm::ASDF::UIText::FontFile
		);
		
		state.new_usertype<GameOverlay::RadialMenu::MenuItem>
		(
			"RadialMenuItem",

			"icon", &GameOverlay::RadialMenu::MenuItem::icon,
			"text", &GameOverlay::RadialMenu::MenuItem::text,
			"SetFunction", &SetMenuItemFunction
		);
		
		state.new_usertype<GameOverlay::RadialMenu>
		(
			"RadialMenu",

			"pos", sol::property(&GameOverlay::RadialMenu::GetPos, &GameOverlay::RadialMenu::SetPos),
			"size", sol::property(&GameOverlay::RadialMenu::GetSize, &GameOverlay::RadialMenu::SetSize),
			"wantsInput", sol::property(&GameOverlay::RadialMenu::WantsInput)
		);
	}

	void InitScriptUI(sol::state& state)
	{
		DestroyScriptUI();
	}

	void RenderScriptUI(Spek::Duration inDt, int screenWidth, int screenHeight)
	{
		Leacon_Profiler;
		glm::vec2 pos;
		glm::vec2 size;
		glm::vec2 screen(screenWidth, screenHeight);
		static glm::vec2 g_zero(0);

		// Render Radial menus
		glm::mat4* viewMatrix = GetViewMatrix();
		glm::mat4* projectionMatrix = GetProjMatrix();
		if (viewMatrix && projectionMatrix)
		{
			glm::mat4 vp = *projectionMatrix * *viewMatrix;
			for (auto& menu : g_radialMenus)
				menu.Draw(inDt, vp, GameOverlay::GetWidth(), GameOverlay::GetHeight());
		}

		// Render Skerm
		for (const auto& instance : g_uiFiles)
		{
			Leacon_ProfilerSection("RenderInstance");
			if (instance->IsVisible() == false)
				return;

			bool isFullScreen = instance->IsFullscreen();
			if (isFullScreen == false)
			{
				pos = glm::vec2(instance->GetX(), instance->GetY());
				size = glm::vec2(pos.x + instance->GetWidth(), pos.y + instance->GetHeight());
			}
			
			for (const auto& container : instance->GetSource()->Output.Elements)
			{
				if (isFullScreen)
					GameOverlay::RenderUI(container.get(), g_zero, screen, screenWidth, screenHeight);
				else
					GameOverlay::RenderUI(container.get(), pos, size, screenWidth, screenHeight);
			}
		}
	}
	
	void DestroyScriptUI()
	{
		Leacon_Profiler;
		g_uiFiles.clear();
		if (g_sourceHandler)
			g_sourceHandler->Clear();
	}
}