#include "debug_render.hpp"
#include "setup.hpp"
#include "controller.hpp"
#include "script.hpp"
#include "script_debug.hpp"
#include "script_controller_mapper.hpp"

// Controller manager
#include <league_controller/config.hpp>
#include <league_controller/controller_input.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller.hpp>
#include <league_controller/config.hpp>

// League lib
#include <league_lib/navgrid/navgrid.hpp>

// Game Overlay
#include <game_overlay/window.hpp>
#include <game_overlay/image.hpp>
#include <game_overlay/overlay.hpp>
#if !LEACON_SUBMISSION
#include <game_overlay/imgui_memory_editor.hpp>
#endif
#include <game_overlay/simple_input.hpp>
#include <game_overlay/debug_renderer.hpp>

// LeagueInternals
#include <league_internals/game_object.hpp>
#include <league_internals/offsets.hpp>

// Other
#include <imgui.h>
#include <sokol.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/norm.hpp>

namespace LeagueController
{
	using Duration = Spek::Duration;
	using namespace Spek;
	struct DrawOperation;

	GameOverlay::LogCategory g_debugRenderLog("DebugRender");
	extern glm::vec2 g_currentMousePos;
	extern glm::vec2 g_targetMousePos;

	static int g_issuedOrders = 0;
	static std::vector<Duration> g_issuedOrderTimes;
	static bool g_isOpen = IF_SUBMISSION_ELSE(false, true);
	static std::vector<DrawOperation> g_drawOperations;

	struct DrawOperation
	{
		std::function<void()> draw;
		Duration endTime;
	};
	
	struct
	{
		GameOverlay::Image texture;
		IF_NOT_SUBMISSION(MemoryEditor memViewer);
	} g_renderData;

	bool CanRenderImGui()
	{
		return IF_NOT_SUBMISSION(IsApp() || ) GameOverlay::IsImguiEnabled();
	}

	static void ImGuiSlider(const char* name, f32 value, bool isOdd, f32 windowWidth, f32& biggestWidth, f32 min = 0.0f, f32 max = 1.0f)
	{
		if (!CanRenderImGui())
			return;

		Leacon_Profiler;
		if (isOdd)
		{
			ImGui::SameLine();
			if (biggestWidth < ImGui::GetCursorPosX())
				biggestWidth = ImGui::GetCursorPosX();
			ImGui::SetCursorPosX(biggestWidth);
		}
		ImGui::SetNextItemWidth(windowWidth * 0.25f);
		ImGui::SliderFloat(name, &value, 0, 1);
	}

	bool IsValidChampionId(const char* championName);
	
	static void RenderChampionList()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;

		ImGui::Text("Champions:");

		std::string heroesLine;
		Manager* heroManager = GetHeroManager();
		if (heroManager)
		{
			GameObject** begin = heroManager->items.begin;
			GameObject** end = &heroManager->items.begin[heroManager->items.count];
			for (GameObject** i = begin; i != end; i++)
			{
				if (i != begin)
					heroesLine += ", ";

				auto hero = *i;
				if (hero == nullptr || IsValidChampionId(hero->GetChampionName()) == false)
					continue;

				const char* champName = hero->GetChampionName() && strlen(hero->GetChampionName()) != 0 ? hero->GetChampionName() : nullptr;
				heroesLine += champName;
			}
		}
		ImGui::Text("- %s", heroesLine.c_str());
	}

#if !LEACON_SUBMISSION
	static void RenderOffsetData()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		ImGui::Text("Offsets are %sloaded.", AreOffsetsLoaded() ? "" : "not ");

		auto menu = GetGuiMenu();
		GameObject* localPlayer = GetLocalPlayer();
		if (AreOffsetsLoaded() == false)
		{
			ImGui::Text("Missing elements: %s", GetMissingAddresses().c_str());
		}
		else
		{
			ImGui::NewLine();
			GetSettings().addresses.ForEachNamedAddress([](AddressData::AddressName name, Offset offset)
			{
				ImGui::Text("%s: 0x%x", name, offset);
			});
			ImGui::NewLine();

			ImGui::Text("Issued %d orders total.", g_issuedOrders);

			int countInLast5Seconds = 0;
			Duration minTime = GetTimeSinceStart() - 5_sec; // Anything in the last 5 seconds.
			for (auto i = g_issuedOrderTimes.rbegin(); i != g_issuedOrderTimes.rend(); i++)
			{
				if (*i < minTime)
					break;
				countInLast5Seconds++;
			}
			ImGui::Text("Issued %d orders in the last 5 seconds.", countInLast5Seconds);
			ImGui::NewLine();
		}
	}
#endif

	static const char* GetOpenedLayerName()
	{
		auto menu = GetGuiMenu();
		if (menu == nullptr)
			return "No menu data available";

		switch (menu->openedLayer)
		{
		case GuiMenuLayerType::None: return "None";
		case GuiMenuLayerType::Chat: return "Chat";
		case GuiMenuLayerType::Shop: return "Shop";
		}
		return "Unknown";
	}

	static void RenderHudData()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;

		ImGui::Text("Settings loaded: %s", AreSettingsLoaded() ? "yes" : "no");
		ImGui::Text("Patch verified: %s", AreOffsetsLoaded() ? "yes" : "no");
		if (HasGameTime())
			ImGui::Text("Game time: %f", GetGameTime());
		else
			ImGui::Text("Game time was not yet loaded");
		
		ImGui::NewLine();
		RenderChampionList();
		ImGui::NewLine();

		ImGui::Text("Currently opened menu: 0x%s", GetOpenedLayerName());
		
		glm::vec3 mousePos;
		if (GetMouseWorldPosition(mousePos))
		{
			ImGui::Text("Current pos: %.1f, %.1f", g_currentMousePos.x, g_currentMousePos.y);
			ImGui::Text("Target pos: %.1f, %.1f", g_targetMousePos.x, g_targetMousePos.y);
			ImGui::Text("Mousepos 3D: %.1f, %.1f, %.1f", mousePos.x, mousePos.y, mousePos.z);
		}
		else
		{
			ImGui::Text("HUD was not loaded.");
		}
	}

	static void RenderLocalPlayerData()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		GameObject* localPlayer = GetLocalPlayer();
		if (localPlayer == nullptr)
		{
			ImGui::Text("Local player was not loaded.");
			return;
		}

		float gameTime = GetGameTime();
		glm::vec3 position = localPlayer->GetPosition();

		const char* userName = localPlayer->GetName() && strlen(localPlayer->GetName()) != 0 ? localPlayer->GetName() : nullptr;
		const char* champName = localPlayer->GetChampionName() && strlen(localPlayer->GetChampionName()) != 0 ? localPlayer->GetChampionName() : nullptr;
		ImGui::Text("Local player (%s, %s)", userName, champName);
		ImGui::Indent();
		ImGui::Text("- Health = %.1f / %.1f", localPlayer->GetHealth(), localPlayer->GetMaxHealth());
		ImGui::Text("- Pos = %.1f, %.1f, %.1f", position.x, position.y, position.z);
		ImGui::Text("- Range = %.0f", localPlayer->GetAttackRange());

		// TODO: Log items
		const SpellBook* spellbook = localPlayer->GetSpellBook();
		if (spellbook == nullptr)
		{
			ImGui::Unindent();
			return;
		}
		
		ImGui::NewLine();

		if (spellbook->GetActiveCast())
		{
			ImGui::Text(" - Casting: %s (level %d)", spellbook->GetActiveCast()->GetName() ? spellbook->GetActiveCast()->GetName() : "Auto attack?", spellbook->GetActiveCast()->GetLevel());
		}
		else
		{
			ImGui::Text(" - Casting: Nothing");
		}

		for (auto i = 0; i < SpellSlotID::Max; i++)
		{
			const Spell* spell = spellbook->GetSpell(i);
			if (IsValidSpell(spell) == false)
			{
				ImGui::Text("- Spell %d -> (invalid or null)", i);
				continue;
			}

			const SpellInfo* spellInfo = spell->GetSpellInfo();
			const char* name = spellInfo && spellInfo->GetName().length != 0 ? spellInfo->GetName().c_str() : nullptr;
			if (name && spellInfo->IsCastable() && gameTime < spell->GetCooldown())
				ImGui::Text("- Spell %d -> %s (on cooldown for %f seconds)", i, name, spell->GetCooldown() - gameTime);
			else
				ImGui::Text("- Spell %d -> %s (%s)", i, name, spell->IsCastable() ? "castable" : "not castable");
		}

	#if 0 // !LEACON_SUBMISSION
		static MemoryEditor mem_edit;
		static int selectedIndex = 0;
		static bool useSpellInfo = false;
		ImGui::SliderInt("Spell", &selectedIndex, 0, SpellSlotID::Max);
		if (selectedIndex < 0)
			selectedIndex = 0;
		else if (selectedIndex >= SpellSlotID::Max)
			selectedIndex = SpellSlotID::Max - 1;
		const Spell* spell = selectedIndex >= 0 ? spellbook->GetSpell(selectedIndex) : nullptr;
		if (IsValidSpell(spell))
		{
			ImGui::Checkbox("Use spell info", &useSpellInfo);
			void* debugTarget = useSpellInfo ? (void*)spell->GetSpellInfo() : (void*)spell;
			mem_edit.DrawContents((void*)debugTarget, 0x1000, (size_t)debugTarget);
		}
	#endif

		ImGui::Unindent();
	}

	class DebugControllerListener : ControllerListener
	{
	public:
		using ControllerStateMap = std::unordered_map<const Controller*, ControllerState>;

		DebugControllerListener(ControllerManager& manager) :
			ControllerListener(manager)
		{}

		void OnControllerConnected(const Controller& controller) override
		{
			std::lock_guard t(m_statesMutex);
			m_states[&controller] = controller.GetCurrentState();
		}

		void OnControllerDisconnected(const Controller& controller) override
		{
			std::lock_guard t(m_statesMutex);
			GameOverlay_LogInfo(g_debugRenderLog, "Controller '%s' has disconnected.", controller.GetName());
			while (true)
			{
				auto index = m_states.find(&controller);
				if (index == m_states.end())
					break;
				
				m_states.erase(index);
			}
		}
		
		virtual void OnInput(const Controller& controller)
		{
			m_states[&controller] = controller.GetCurrentState();
		}

		ControllerStateMap GetStates()
		{
			std::lock_guard t(m_statesMutex);
			return m_states;
		}

	private:
		std::mutex m_statesMutex;
		ControllerStateMap m_states;
	};

	static void RenderControllerState(const char* controllerName, const ControllerState& state, f32& lastWindowWidth)
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		char headerName[64];
		snprintf(headerName, 64, "%s", controllerName);

		if (ImGui::CollapsingHeader(headerName))
		{
			ImGui::Indent();

			f32 windowWidth = ImGui::GetWindowWidth();
			f32 windowWidthRatio = (lastWindowWidth > 0.01f ? (windowWidth / lastWindowWidth) : 1.0f);

			static f32 lastBiggestWidth = 0;
			f32 biggestWidth = lastBiggestWidth * windowWidthRatio;
			for (int i = 0; i < (u32)ControllerInput::InputCount - 1; i++)
				ImGuiSlider(GetControllerInputName((ControllerInput)i), state.button[i], (i % 2), windowWidth, biggestWidth);

			ImGui::NewLine();

			ImGuiSlider("Left Analog X", state.leftAnalog.X, 0, windowWidth, biggestWidth, -1.0f, 1.0f);
			ImGuiSlider("Left Analog Y", state.leftAnalog.Y, 1, windowWidth, biggestWidth, -1.0f, 1.0f);

			ImGuiSlider("Left DPad X", state.leftDPad.X, 0, windowWidth, biggestWidth, -1.0f, 1.0f);
			ImGuiSlider("Left DPad Y", state.leftDPad.Y, 1, windowWidth, biggestWidth, -1.0f, 1.0f);

			ImGuiSlider("Right Analog X", state.rightAnalog.X, 0, windowWidth, biggestWidth, -1.0f, 1.0f);
			ImGuiSlider("Right Analog Y", state.rightAnalog.Y, 1, windowWidth, biggestWidth, -1.0f, 1.0f);

			lastWindowWidth = ImGui::GetWindowWidth();
			lastBiggestWidth = biggestWidth;
			
			ImGui::Unindent();
		}

	}

	static void RenderControllerData()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		static DebugControllerListener g_controllerListener(GetControllerManager());
		static f32 lastWindowWidth = ImGui::GetWindowWidth();

		ImGui::Indent();
		if (ImGui::CollapsingHeader("Key bindings"))
		{
			ImGui::Indent();

			ImGui::Text("Unbound keys:");
			for (auto& key : GetUnboundKeys())
			{
				ImGui::Text("- %s", GetControllerInputName(key));
			}

			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("InputState"))
		{
			ImGui::Indent();

			for (auto controllerInfo : g_controllerListener.GetStates())
			{
				const Controller* controller = controllerInfo.first;
				const ControllerState& state = controllerInfo.second;
				if (controller != nullptr)
					RenderControllerState(controller->GetName(), state, lastWindowWidth);
			}

			RenderControllerState("All Controllers", GetControllerState(), lastWindowWidth);

			ImGui::Unindent();
		}
		ImGui::Unindent();
	}

	static void RenderNavGrid()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		auto navGrid = GetNavGrid();
		if (navGrid == nullptr)
		{
			ImGui::Text("NavGrid was not found.");
			return;
		}
		
		switch (navGrid->GetLoadState())
		{

		case Spek::File::LoadState::Loaded:
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			int w = navGrid->GetCellCountX();
			int h = navGrid->GetCellCountY();
			// ImGui::Text("Size: %d, %d", w, h);

			auto base = ImGui::GetCursorScreenPos();
			ImGui::Image((ImTextureID)g_renderData.texture.GetAsDX11(), ImVec2(w, h));

			// Draw mouse pos on map
			glm::vec3 mousePos;
			if (GetMouseWorldPosition(mousePos))
			{
				glm::vec3 localGridPos = navGrid->TranslateToNavGrid(mousePos);
				ImVec2 localPosOnImage = ImVec2(base.x + localGridPos.x, base.y + localGridPos.z);
				drawList->AddCircle(localPosOnImage, 3.0f, 0xFFFF00FF);
			}

			auto attackableManager = GetAttackableManager();
			if (attackableManager)
			{
				GameObject** begin = attackableManager->items.begin;
				GameObject** end = &attackableManager->items.begin[attackableManager->items.count];
				for (auto i = begin; i != end; i++)
				{
					auto gameObject = *i;
					if (gameObject == nullptr || ((u32)gameObject & 1))
						continue;

					if (gameObject->isTargetable == false || gameObject->IsDead())
						continue;

					u32 col = IM_COL32(0, 0xFF, 0, 0xFF);
					if (gameObject->team == 300) // Gaia
						col = IM_COL32(0, 0xFF, 0, 0xFF);
					else if (gameObject->team != GetLocalPlayer()->team) // EnemyW
						col = IM_COL32(0xFF, 0, 0, 0xFF);
					else if (gameObject->team == GetLocalPlayer()->team) // Ally
						col = IM_COL32(0, 0xFF, 0, 0xFF);

					auto rad = 3.0f;
					glm::vec3 localGridPos = navGrid->TranslateToNavGrid((*i)->GetPosition());
					ImVec2 localPosOnImage = ImVec2(base.x + localGridPos.x, base.y + localGridPos.z);
					drawList->AddCircle(localPosOnImage, rad, col);

					if (gameObject->GetTypeName())
					{
						localPosOnImage.x += 5;
						drawList->AddText(localPosOnImage, col, gameObject->GetTypeName());
					}
				}
			}

			if (GetLocalPlayer())
			{
				glm::vec3 localPos = GetLocalPlayer()->GetPosition();
				glm::vec3 localGridPos = navGrid->TranslateToNavGrid(localPos);
				ImVec2 localPosOnImage = ImVec2(base.x + localGridPos.x, base.y + localGridPos.z);
				drawList->AddCircle(localPosOnImage, 3.0f, 0xFF0000FF);

				glm::vec3 controllerDir;
				if (LeagueController::GetLeftAnalogDir(controllerDir, true) == false)
				{
					// ImGui::Text("(No valid controller dir)");
					break;
				}

				glm::vec3 gridHit;
				if (navGrid->CastRay2DPassable(localPos, controllerDir, 100000, gridHit))
				{
					gridHit = navGrid->TranslateToNavGrid(gridHit);
					ImVec2 gridHitImage = ImVec2(base.x + gridHit.x, base.y + gridHit.z);
					ImGui::Text("AimDir => %f, %f -> %f %f", localGridPos.x, localGridPos.z, gridHit.x, gridHit.z);
					drawList->AddLine(localPosOnImage, gridHitImage, 0xFF0000FF);
				}
			}
			break;
		}

		case Spek::File::LoadState::NotFound:
		{
			ImGui::Text("NavGrid was not found.");
			break;
		}

		case Spek::File::LoadState::FailedToLoad:
		{
			ImGui::Text("NavGrid failed to load.");
			break;
		}

		case Spek::File::LoadState::NotLoaded:
		{
			ImGui::Text("NavGrid is loading..");
			break;
		}
		}
	}

	static void RenderConnection()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		const LauncherData* launcherData = GetLauncherData();
		if (launcherData == nullptr)
		{
			ImGui::Text("Connection has not been initialised.");
			return;
		}

		ImGui::Text("CWD: %s", launcherData->currentWorkingDirectory);
		ImGui::Text("Should free: %s", launcherData->shouldFreeSelf ? "true" : "false");
	}
	
	static f64 lerp(f64 a, f64 b, f64  t)
	{
		if (t < 0.0)
			return a;
		if (t > 1.0)
			return b;
		return a + t * (b - a);
	}

	static std::vector<std::string> string_split(const std::string& s, const std::string& delim)
	{
		std::vector<std::string> result;
		auto start = 0U;
		auto end = s.find(delim);
		while (end != std::string::npos)
		{
			result.push_back(s.substr(start, end - start));
			start = end + delim.length();
			end = s.find(delim, start);
		}
		result.push_back(s.substr(start));
		return result;
	}

	struct TreeNode
	{
		using Pair = std::pair<std::string, TreeNode>;
		using PairPtr = std::shared_ptr<Pair>;
		std::vector<PairPtr> children;
		DebugReport report;
		bool hasValue = false;
	};
	TreeNode g_root;
	static const Spek::Duration g_staleTime = 10_sec;

	template <typename Iterator, typename Pred>
	static void BubbleSort(Iterator begin, Iterator end, Pred a)
	{
		Leacon_Profiler;
		for (auto i = begin; i != end; ++i)
		{
			auto min = i;
			for (auto j = i + 1; j != end; ++j)
				if (a(*j, *min))
					min = j;

			auto& temp = i;
			i = min;
			min = temp;
		}
	}
	
	static void RenderLuaDataTreeNode(const std::string& key, const TreeNode& node, bool render = true)
	{
		Leacon_Profiler;
		bool hasHeader = node.children.empty() == false;
		bool showContents = false;
		if (hasHeader)
		{
			showContents = ImGui::CollapsingHeader(key.c_str());
		}

		const auto& value = node.report;
		Spek::Duration timeAgo = GetTimeSinceStart() - value.time;
		if (node.hasValue)
		{
			ImVec4 red = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
			ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
			f64 lerpValue = timeAgo / g_staleTime;
			ImVec4 color = ImVec4(lerp(green.x, red.x, lerpValue), lerp(green.y, red.y, lerpValue), lerp(green.z, red.z, lerpValue), 1.0f);
			ImGui::TextColored(color, "%s: %s", key.c_str(), value.message.c_str());
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				if (timeAgo > 100_ms)
					ImGui::Text("Last update %.1f sec ago", timeAgo.ToSecF64());
				else
					ImGui::TextUnformatted("Last update just now");
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
		}

		if (hasHeader && showContents)
		{
			ImGui::Indent();
			auto begin = &node.children[0];
			auto end = &node.children[node.children.size()-1]+1;
			BubbleSort(begin, end, [](const TreeNode::PairPtr& a, const TreeNode::PairPtr& b) { return a->first < b->first; });
			for (const auto& child : node.children)
				RenderLuaDataTreeNode(child->first, child->second, render);
			ImGui::Unindent();
			ImGui::NewLine();
		}
	}

	static void RenderLuaData()
	{
		Leacon_Profiler;
		if (!CanRenderImGui())
			return;
		const LuaDebugReportMap& debugReportMap = GetLuaDebugReportMap();

		for (const auto& [key, value] : debugReportMap)
		{
			Spek::Duration timeAgo = GetTimeSinceStart() - value.time;
			auto groups = string_split(key, "/");
			TreeNode* node = &g_root;
			for (const auto& group : groups)
			{
				auto index = std::find_if(node->children.begin(), node->children.end(), [group](const auto& a) { return a->first == group; });
				if (index == node->children.end())
					node = &node->children.emplace_back(std::make_unique<TreeNode::Pair>(std::make_pair(group, TreeNode())))->second;
				else
					node = &(*index)->second;
			}

			node->report = value;
			node->hasValue = true;
		}

		BubbleSort(g_root.children.begin(), g_root.children.end(), [](const TreeNode::PairPtr& a, const TreeNode::PairPtr& b) { return a->first < b->first; });
		for (auto& ptr : g_root.children)
			RenderLuaDataTreeNode(ptr->first, ptr->second);
		if (ImGui::Button("Clear"))
			g_root.children.clear();
	}

#if !LEACON_SUBMISSION
	static void RenderGameObjectMemory()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		static MemoryEditor mem_edit;
		Manager* attackableManager = GetAttackableManager();
		if (attackableManager == nullptr)
			return;

		static int selectedIndex = 0;
		int max = attackableManager->items.count;
		ImGui::SliderInt("GameObject", &selectedIndex, -1, max);
		if (selectedIndex < -1)
			selectedIndex = 0;
		else if (selectedIndex >= max)
			selectedIndex = max - 1;
		void* attackable = selectedIndex >= 0 ? attackableManager->items.begin[selectedIndex] : GetLocalPlayer();
		if (attackable == nullptr)
			return;

		mem_edit.DrawContents(attackable, 0x3000);
	}
#endif

	void CheckNavGridLoaded()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		auto grid = GetNavGrid();
		if (grid == nullptr || grid->GetLoadState() != File::LoadState::Loaded)
			return;

		if (g_renderData.texture.HasData())
			return;

		const std::vector<LeagueLib::NavGridCellFlag>& flags = grid->GetCellState();
		std::vector<u32> colours(flags.size());

		auto w = grid->GetCellCountX();
		auto h = grid->GetCellCountY();
		for (int y = 0; y < h; y++)
		{
			int yOff = y * w;
			for (int x = 0; x < w; x++)
			{
				u8* c = (u8*)&colours[x + yOff];
				u8 src = LeagueLib::IsCellPassable(flags[x + yOff]) ? 0 : 255;
				c[0] = src;
				c[1] = src;
				c[2] = src;
				c[3] = 0xFF;
			}
		}

		g_renderData.texture = GameOverlay::Image(colours.data(), w, h);
	}
	
	void RenderImGui()
	{
		if (!CanRenderImGui())
			return;
		Leacon_Profiler;
		CheckNavGridLoaded();

	#define COLLAPSE_MENU(Name, Func) do {\
			if (ImGui::CollapsingHeader(Name))\
			{\
				ImGui::Indent();\
				Func; \
				ImGui::Unindent();\
				ImGui::NewLine();\
			}\
		} while(false)
		GameOverlay::RenderLogImgui();

		ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("LeagueController"))
		{
			IF_NOT_SUBMISSION(COLLAPSE_MENU("Offset Data", RenderOffsetData()));
			COLLAPSE_MENU("HUD Data", RenderHudData());
			COLLAPSE_MENU("Local Player", RenderLocalPlayerData();ImGui::NewLine());
			COLLAPSE_MENU("NavGrid", RenderNavGrid();ImGui::NewLine());
			COLLAPSE_MENU("Controller", RenderControllerData();ImGui::NewLine());
			COLLAPSE_MENU("Connection", RenderConnection(); ImGui::NewLine());
			COLLAPSE_MENU("Lua", RenderLuaData(); ImGui::NewLine());
			IF_NOT_SUBMISSION(COLLAPSE_MENU("Game Object Memory", RenderGameObjectMemory(); ImGui::NewLine()));

			ImGui::NewLine();
		#if !LEACON_SUBMISSION
			if (ImGui::Button("Close"))
			{
				system("taskkill /f /im \"League of Legends.exe\"");
			}
			ImGui::SameLine();
			if (ImGui::Button("Unload"))
			{
				UnloadSelf();
			}
			ImGui::SameLine();
		#endif
			if (ImGui::Button(IF_SUBMISSION_ELSE("Reset", "Reset scripts")))
			{
				FlagScriptsForReload();
			}
		}
		ImGui::End();

		for (auto& op : g_drawOperations)
			if (op.endTime > GetTimeSinceStart())
				op.draw();

	#undef COLLAPSE_MENU
	}
	
	void IssueWasOrdered()
	{
		Leacon_Profiler;
		g_issuedOrders++;
		if (g_issuedOrders > 10000)
			g_issuedOrderTimes.clear();
		g_issuedOrderTimes.push_back(GetTimeSinceStart());
	}

	void DrawLine3D(const glm::vec3& start, const glm::vec3& end, GameOverlay::Pixel color, f32 thickness, Spek::Duration duration)
	{
		Leacon_Profiler;
		GameOverlay::ValidateDebugDraw();
		g_drawOperations.emplace_back(DrawOperation
		{
			[start, end, color]()
			{
				glm::vec2 left = WorldToScreen(start);
				glm::vec2 right = WorldToScreen(end);
				GameOverlay::DebugDraw.Line(left.x, left.y, right.x, right.y, color);
			},
			GetTimeSinceStart() + duration + 16_ms
		});
	}
}
