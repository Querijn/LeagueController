#include "script_controller.hpp"
#include "script_controller_mapper.hpp"
#include "controller.hpp"
#include "script_debug.hpp"
#include "script_game.hpp"

#include <league_internals/offsets.hpp>

#include <league_controller/settings.hpp>
#include <league_controller/controller_manager.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller.hpp>

#include <game_overlay/log.hpp>
#include <spek/util/hash.hpp>

#include <optional>
#include <functional>
#include <algorithm>
#include <sol/sol.hpp>

namespace LeagueController
{
	struct CustomAction;
	extern GameOverlay::LogCategory g_scriptLog;
	static std::unordered_map<ControllerInput, std::vector<CustomAction>> g_customActionMap;
	static std::unordered_map<GameOverlay::KeyboardKey, ControllerInput> g_reverseKeyMap;
	static std::unordered_map<ControllerInput, f32> g_pressTime;
	static std::unordered_map<std::string, ControllerInput> g_boundSpells;
	static std::vector<ControllerInput> g_unboundKeys;
	static bool g_cacheGenerated = false;
	static sol::state* g_state = nullptr;
	
#define CustomActionType_Fields(KeyFunc, ActFunc) \
	KeyFunc(Spell1,		Q,			!IsLizardModeEnabled()) \
	KeyFunc(Spell2,		W,			!IsLizardModeEnabled()) \
	KeyFunc(Spell3,		E,			!IsLizardModeEnabled()) \
	KeyFunc(Spell4,		R,			!IsLizardModeEnabled()) \
	KeyFunc(Item1,		1,			!IsLizardModeEnabled()) \
	KeyFunc(Item2,		2,			!IsLizardModeEnabled()) \
	KeyFunc(Item3,		3,			!IsLizardModeEnabled()) \
	KeyFunc(Item4,		5,			!IsLizardModeEnabled()) \
	KeyFunc(Item5,		6,			!IsLizardModeEnabled()) \
	KeyFunc(Item6,		7,			!IsLizardModeEnabled()) \
	KeyFunc(Trinket,	4,			!IsLizardModeEnabled()) \
	KeyFunc(Summoner1,	D,			!IsLizardModeEnabled()) \
	KeyFunc(Summoner2,	F,			!IsLizardModeEnabled()) \
	KeyFunc(Recall,		B,			!IsLizardModeEnabled()) \
	KeyFunc(Shop,		P,			true) \
	KeyFunc(Menu,		Escape,		true) \
	ActFunc(Potion,					!IsLizardModeEnabled()) \

	enum class CustomActionType
	{
	#define AsEnum(Name, Button) Name,
		CustomActionType_Fields(AsEnum, AsEnum)
	#undef AsEnum
	};

	enum class ButtonState
	{
		None,
		Down,
		Up,
	};

	static std::string CustomActionTypeToString(CustomActionType action)
	{
		Leacon_Profiler;
		switch (action)
		{
		#define AsString(Name, Button) case CustomActionType::Name: return #Name;
			CustomActionType_Fields(AsString, AsString)
		#undef AsString
		}
		return "Unknown";
	}

	static std::optional<CustomActionType> StringToCustomActionType(const std::string& str)
	{
		Leacon_Profiler;
	#define AsAction(Name, Button) if (str == #Name) return CustomActionType::Name;
		CustomActionType_Fields(AsAction, AsAction)
	#undef AsAction
		return std::nullopt;
	}

	using CustomActionVoidFunctor = std::function<void(CustomAction&)>;
	using CustomActionBoolFunctor = std::function<bool(CustomAction&)>;

	struct CustomAction
	{
		CustomAction(CustomActionVoidFunctor inOnPress, CustomActionVoidFunctor inOnRelease, CustomActionBoolFunctor inUseCondition) :
			onPress(inOnPress),
			onRelease(inOnRelease),
			useCondition(inUseCondition)
		{
		}
		
		CustomActionVoidFunctor onPress;
		CustomActionVoidFunctor onRelease;
		CustomActionBoolFunctor useCondition;

		// Small hacks for reverse map
		std::string spellName = "";
		GameOverlay::KeyboardKey keyboardKey = GameOverlay::KeyboardKey::KeyNone;
		GameOverlay::MouseButton mouseButton = GameOverlay::MouseButton::None;

		virtual bool IsValid() const { return true; }
	};

	struct KeyboardCustomAction : public CustomAction
	{
		KeyboardCustomAction(GameOverlay::KeyboardKey inKey, CustomActionBoolFunctor inUseCondition) :
			CustomAction([inKey](CustomAction&) { FeignKeyPress(inKey); }, [inKey](CustomAction&) { FeignKeyRelease(inKey); }, inUseCondition)
		{
			keyboardKey = inKey;
		}

		virtual bool IsValid() const { return true; }
	};

	struct MouseCustomAction : public CustomAction
	{
		MouseCustomAction(GameOverlay::MouseButton inButton, CustomActionBoolFunctor inUseCondition) :
			CustomAction([inButton](CustomAction&) { FeignMouseClick(inButton); }, nullptr, inUseCondition)
		{
			mouseButton = inButton;
		}

		virtual bool IsValid() const { return true; }
	};
	
	struct SpellCustomAction : public CustomAction
	{
		SpellCustomAction(const char* name) :
			CustomAction([](auto& a) { SpellCustomAction::OnPress(a); }, [](auto& a) { SpellCustomAction::OnRelease(a); }, [](auto& a) { return SpellCustomAction::CanUse(a); })
		{
			spellName = name;
		}

		static void OnPress(CustomAction& action)
		{
			Leacon_Profiler;
			Keyboard key = GetSpellKeyByName(action.spellName);
			if (key != Keyboard::KeyNone)
				FeignKeyPress(key);
		}

		static void OnRelease(CustomAction& action)
		{
			Leacon_Profiler;
			Keyboard key = GetSpellKeyByName(action.spellName);
			if (key != Keyboard::KeyNone)
				FeignKeyRelease(key);
		}

		static bool CanUse(CustomAction& action) { return !IsLizardModeEnabled(); }
	};

	struct PotionCustomAction : public CustomAction
	{
		PotionCustomAction() :
			CustomAction([](auto& a) { PotionCustomAction::OnPress(a); }, nullptr, [](auto& a) { return PotionCustomAction::CanUse(a); })
		{
		}

		static void OnPress(CustomAction& action)
		{
			Leacon_Profiler;
			GameOverlay_Assert(g_scriptLog, g_state != nullptr);
			auto potionTable = Leacon_ProfilerEvalRet((*g_state)["Potion"]);
			GameOverlay_Assert(g_scriptLog, potionTable.valid());
			if (potionTable.valid() == false)
				return;
			
			auto useBestPotion = Leacon_ProfilerEvalRet(potionTable["UseBestPotion"]);
			GameOverlay_Assert(g_scriptLog, useBestPotion.valid());
			if (useBestPotion.valid() == false)
				return;
			
		#if !LEACON_SUBMISSION
			std::string updateFunction = "Potion.UseBestPotion(GetLocalPlayer());";
			InjectDebugData("UseBestPotion", updateFunction, true);
			Leacon_ProfilerEval(g_state->safe_script(updateFunction));
		#else
			Leacon_ProfilerEval(useBestPotion(GetLocalPlayer()));
		#endif
		}

		static bool CanUse(CustomAction& action) { return GetLocalPlayer() && g_state && !IsLizardModeEnabled(); }
	};

	static CustomAction GetActionByName(std::string_view name)
	{
		Leacon_Profiler;
		std::string lcName;
		auto hash = Spek::FNV(name.data());

		switch (hash)
		{
		#define DefineKey(Name, Input, Condition) case Spek::FNV(#Name): return KeyboardCustomAction(GameOverlay::KeyboardKey::Key##Input, [](CustomAction& action) { return Condition; });
		#define DefineNothing(Name, Condition)
			CustomActionType_Fields(DefineKey, DefineNothing)
		#undef DefineKey
		#undef DefineNothing
		
		// Mouse buttons
		case Spek::FNV("LeftClick"):		return MouseCustomAction(GameOverlay::MouseButton::Left,   [](CustomAction& action) { return IsLizardModeEnabled(); });
		// case Spek::FNV("MiddleClick"):	return MouseCustomAction(GameOverlay::MouseButton::Middle, [](CustomAction& action) { return IsLizardModeEnabled(); });
		case Spek::FNV("RightClick"):		return MouseCustomAction(GameOverlay::MouseButton::Right,  [](CustomAction& action) { return IsLizardModeEnabled(); });

		case Spek::FNV("Ctrl"):				return KeyboardCustomAction(GameOverlay::KeyboardKey::KeyCtrl, [](CustomAction& action) { return !IsLizardModeEnabled(); });
		case Spek::FNV("Potion"):			return PotionCustomAction();
		}

		return CustomAction(nullptr, nullptr, [](CustomAction& action) { return false; });
	}

	std::string GetBindingString(ControllerInput input)
	{
		Leacon_Profiler;
		auto& settings = GetSettings();
		auto& controllerSettings = settings.serialisedSettings.controller;

		switch (input)
		{
		#define FieldCase(Type, Name, DefaultValue, Input) case ControllerInput::Button##Input: return controllerSettings.Name;
			ControllerSettings_BindingFields(FieldCase)
		#undef FieldCase
		}

		return "";
	}

	std::string Trim(const std::string_view input)
	{
		Leacon_Profiler;
		std::string output;
		output.reserve(input.size());
		for (auto c : input)
			if (c != ' ')
				output.push_back(c);
		return output;
	}

	static bool GetControllerBindingsCache(ControllerInput input, std::vector<CustomAction>& keys)
	{
		Leacon_Profiler;
		auto index = g_customActionMap.find(input);
		if (index == g_customActionMap.end())
			return false;

		keys = index->second;
		return true;
	}
	
	std::vector<CustomAction> GetControllerBindings(ControllerInput input, bool onlyUsable = false)
	{
		Leacon_Profiler;

		// First, lookup in cache
		std::vector<CustomAction> keys;
		if (!onlyUsable && GetControllerBindingsCache(input, keys))
			return keys;

		// Generate bindings if not found in cache
		std::string binding = GetBindingString(input);
		if (binding.empty())
		{
			if (!onlyUsable)
			{
				g_unboundKeys.push_back(input);
				g_customActionMap[input] = keys;
			}
			return keys;
		}

		auto addAction = [&keys, onlyUsable](std::string_view name)
		{
			std::string keyName = Trim(name);
			auto action = GetActionByName(keyName);

			// If we only want usable actions, skip the ones that are not
			bool isValid = action.IsValid();
			bool isUsable = action.useCondition == nullptr || action.useCondition(action);
			if (onlyUsable && (!isValid || !isUsable))
				return;
			
			keys.push_back(action);
		};

		size_t index = 0;
		while (true)
		{
			size_t commaPos = binding.find(',', index);
			if (commaPos == std::string::npos)
				break;

			addAction(binding.substr(index, commaPos - index));
			index = commaPos + 1;
		}

		addAction(binding.substr(index));

		if (!onlyUsable)
		{
			// Add to cache
			g_customActionMap[input] = keys;
			if (keys.empty())
				g_unboundKeys.push_back(input);
		}
		return keys;
	}

	std::vector<ControllerInput> GetUnboundKeys() { return g_unboundKeys; }

	void AddSpellMapping(ControllerInput input, const char* spellName)
	{
		Leacon_Profiler;
		auto& actionArray = g_customActionMap[input];
		for (auto& action : actionArray)
			if (action.spellName == spellName)
				return;

		g_boundSpells[spellName] = input;
		actionArray.push_back(SpellCustomAction(spellName));
		g_unboundKeys.erase(std::remove(g_unboundKeys.begin(), g_unboundKeys.end(), input), g_unboundKeys.end());
	}

	bool IsSpellBound(const char* spellName)
	{
		Leacon_Profiler;
		return g_boundSpells.find(spellName) != g_boundSpells.end();
	}

	static GameOverlay::KeyboardKey GetKeyByControllerInput(ControllerInput input)
	{
		Leacon_Profiler;
		auto bindings = GetControllerBindings(input);
		if (bindings.empty())
			return GameOverlay::KeyboardKey::KeyNone;

		for (auto& binding : bindings)
		{
			if (binding.useCondition && binding.useCondition(binding) == false)
				continue;

			if (binding.keyboardKey != GameOverlay::KeyboardKey::KeyNone)
				return binding.keyboardKey;
		}

		return GameOverlay::KeyboardKey::KeyNone;
	}

	static bool IsCustomActionPressed(const char* name)
	{
		Leacon_Profiler;
		auto action = GetActionByName(name);
		if (action.useCondition && action.useCondition(action) == false)
			return false;

		if (action.keyboardKey != GameOverlay::KeyboardKey::KeyNone)
		{
			auto index = g_reverseKeyMap.find(action.keyboardKey);
			if (index != g_reverseKeyMap.end())
				return IsButtonDown(index->second);
		}

		// TODO: Mouse buttons
		return false;
	}
	
	static f32 GetControllerInputPressTime(ControllerInput input)
	{
		Leacon_Profiler;
		auto index = g_pressTime.find(input);
		if (index == g_pressTime.end())
			return 0.0f;
		return index->second;
	}

	void GenerateCache()
	{
		Leacon_Profiler;

		// Generate bindings and reverse map
		if (g_cacheGenerated)
			return;

		for (u32 i = 0; i < (u32)ControllerInput::ButtonLeftAnalogUp; i++) // Hack, don't allow binding to analog sticks
		{
			auto bindings = GetControllerBindings((ControllerInput)i);
			for (auto& binding : bindings)
				if (binding.keyboardKey != GameOverlay::KeyboardKey::KeyNone)
					g_reverseKeyMap[binding.keyboardKey] = (ControllerInput)i;
		}
		g_cacheGenerated = true;
	}

	void InitScriptControllerMapper(sol::state& state)
	{
		Leacon_Profiler;

		g_state = &state;

		// Clear cache
		g_customActionMap.clear();
		g_reverseKeyMap.clear();
		g_pressTime.clear();
		g_boundSpells.clear();
		g_unboundKeys.clear();
		g_cacheGenerated = false;
		GenerateCache();

		GameOverlay_LogDebug(g_scriptLog, "Initialized controller mapper");
	}

	void AddControllerMapperScriptSystem(sol::state& state)
	{
		Leacon_Profiler;

		state.set_function("GetControllerBindings", &GetControllerBindings);
		state.set_function("GetUnboundKeys", &GetUnboundKeys);
		state.set_function("GetKeyByControllerInput", &GetKeyByControllerInput);
		state.set_function("IsCustomActionPressed", &IsCustomActionPressed);
		state.set_function("GetControllerInputPressTime", &GetControllerInputPressTime);
		state.set_function("AddSpellMapping", &AddSpellMapping);
		state.set_function("IsSpellBound", &IsSpellBound);
		
		state["ButtonState"] = state.create_table();
		state["ButtonState"]["None"] = ButtonState::None;
		state["ButtonState"]["Down"] = ButtonState::Down;
		state["ButtonState"]["Up"] = ButtonState::Up;
	}

	static void UpdateButtonStates()
	{
		Leacon_Profiler;
		const ControllerState& state = GetControllerState();
		static ControllerState lastState = state;

		// Little lambda to ensure we handle button-presses between buttons and directional correctly
		auto commonOnChange = [](ControllerInput input, bool isCurrentDown)
		{
			Leacon_Profiler;
			auto bindings = GetControllerBindings(input);
			for (auto& binding : bindings)
			{
				if (binding.useCondition && binding.useCondition(binding) == false)
					continue;

				if (isCurrentDown && binding.onPress)
				{
					g_pressTime[input] = GetGameTime();
					binding.onPress(binding);
				}
				else if (!isCurrentDown && binding.onRelease)
				{
					g_pressTime[input] = 0.0f;
					binding.onRelease(binding);
				}
			}
		};

		for (int i = 0; i < (u32)ControllerInput::ButtonCount; i++)
		{
			bool isCurrentDown = IsButtonDown((ControllerInput)i, &state);
			bool wasLastDown = IsButtonDown((ControllerInput)i, &lastState);
			if (isCurrentDown == wasLastDown)
				continue;

			commonOnChange((ControllerInput)i, isCurrentDown);
		}

		auto inputTypes = { DirectionalInputType::DPad /*, DirectionalInputType::LeftAnalog, DirectionalInputType::RightAnalog*/ };
		auto inputDirections = { InputDirection::Up, InputDirection::Down, InputDirection::Left, InputDirection::Right };
		for (auto& inputDirection : inputDirections)
		{
			for (auto& inputType : inputTypes)
			{
				bool isCurrentDown = IsButtonDown(inputType, inputDirection, &state);
				bool wasLastDown = IsButtonDown(inputType, inputDirection, &lastState);
				if (isCurrentDown == wasLastDown)
					continue;

				ControllerInput input = GetControllerInputFromDirectionalInput(inputType, inputDirection);
				commonOnChange(input, isCurrentDown);
			}
		}

		lastState = state;
	}

	void UpdateScriptControllerMapper()
	{
		Leacon_Profiler;
		UpdateButtonStates();
	}

	void DestroyScriptControllerMapper()
	{
		Leacon_Profiler;
	}
}
