#include "lua_wrapper.hpp"
#include "script_game.hpp"
#include "controller.hpp"
#include "champion_data.hpp"
#include "issue_order.hpp"
#include "debug_render.hpp"
#include "setup.hpp"
#include "script.hpp"
#include "item.hpp"

#include <game_overlay/log.hpp>
#include <game_overlay/window.hpp>

#include <league_internals/offsets.hpp>
#include <league_internals/game_object.hpp>

// ControllerManager
#include <league_controller/controller.hpp>
#include <league_controller/controller_listener.hpp>
#include <league_controller/controller_manager.hpp>
#include "script_require.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/norm.hpp>

namespace LeagueController
{
	extern GameOverlay::LogCategory g_scriptLog;
	static GameObject* g_nearestObject;
	static GameObject* g_nearestChampion;

	enum LockOnType
	{
		Hard,
		Soft,
		None
	};

	static bool IsGameReady()
	{
		return GetLocalPlayer() && GetGameTime() > 0.1f; // Kind of crude but works pretty well
	}

	static std::tuple<bool, glm::vec2> GetWindowCenter()
	{
		Leacon_Profiler;
		int x = -1, y = -1;
		bool result = GameOverlay::GetWindowCenter(x, y);
		return std::make_tuple(result, glm::vec2((f32)x, (f32)y));
	}

	static std::tuple<bool, glm::vec2> GetClientGlobalPosition()
	{
		Leacon_Profiler;
		int x = -1, y = -1;
		bool result = GameOverlay::GetClientGlobalPosition(x, y);
		return std::make_tuple(result, glm::vec2((f32)x, (f32)y));
	}

	static bool DetermineNearestObject(GameObject*& target, Manager* manager, const glm::vec3* suggestionPos = nullptr)
	{
		Leacon_Profiler;
		target = nullptr;

		glm::vec3 mousePos;
		auto localPlayer = GetLocalPlayer();
		if (manager == nullptr || localPlayer == nullptr)
			return false;

		if (suggestionPos == nullptr)
		{
			if (GetMouseWorldPosition(mousePos) == false)
				return false;
		}
		else
		{
			mousePos = *suggestionPos;
		}

		f32 lowestDist = 9e9f;

		GameObject** begin = manager->items.begin;
		GameObject** end = &manager->items.begin[manager->items.count];
		for (auto i = begin; i != end; i++)
		{
			auto gameObject = *i;
			if (gameObject == nullptr || ((u32)gameObject & 1))
				continue;

			if (gameObject->team == localPlayer->team)
				continue;

			if (gameObject->isTargetable == false || gameObject->IsDead())
				continue;

			glm::vec3 pos = gameObject->GetPosition();
			glm::vec3 delta = pos - mousePos;
			f32 newDist = glm::length2(delta);
			if (newDist < lowestDist)
			{
				lowestDist = newDist;
				target = gameObject;
			}
		}

		return target != nullptr;
	}

	static GameObject* GetNearestObject(const glm::vec3* pos)
	{
		if (pos == nullptr)
			return g_nearestObject;

		GameObject* result;
		DetermineNearestObject(result, GetAttackableManager(), pos);
		return result;
	}

	static GameObject* GetNearestChampion(const glm::vec3* pos)
	{
		if (pos == nullptr)
			return g_nearestChampion;

		GameObject* result;
		DetermineNearestObject(result, GetHeroManager(), pos);
		return result;
	}

	static GameObject* GetObjectUnderMouse()
	{
		return g_nearestObject;
	}

	static sol::state& GetChampionScript(const char* championName)
	{
		Leacon_Profiler;
		static std::unordered_map<std::string, sol::state> scriptMap;

		std::string result = championName;
		std::transform(result.begin(), result.end(), result.begin(), ::tolower);
		championName = result.c_str();

		// Load existing
		const auto& index = scriptMap.find(championName);
		if (index != scriptMap.end())
			return index->second;

		// Initial load
		sol::state& lua = scriptMap[championName];
		SetupDefaultScriptState(lua, false);
		std::string path = "scripts/champions/" + std::string(championName) + ".lua";
		if (LoadFileToState(lua, path.c_str(), false))
		{
			GameOverlay_LogDebug(g_scriptLog, "Loaded champion script '%s'", path.c_str());
			return lua;
		}

		// If we can't find the champion script, load the default one.
		if (LoadFileToState(lua, "scripts/champions/none.lua", false))
		{
			GameOverlay_LogDebug(g_scriptLog, "Loaded champion script '%s'", path.c_str());
			return lua;
		}

		GameOverlay_AssertF(g_scriptLog, false, "Cannot load champion script for '%s' nor the 'none.lua' script. Expect a crash.", path.c_str());
		static sol::state empty;
		return empty;
	}

	static float GetSpellCastRange(Keyboard key, f32 castTime)
	{
		Leacon_Profiler;
		const ChampionData& championData = GetChampionData();

		// Get the override from scripts
		auto localPlayer = GetLocalPlayer();
		if (localPlayer == nullptr || localPlayer->GetChampionName() == nullptr)
		{
			GameOverlay_LogWarning(g_scriptLog, "GetSpellCastRange requires localPlayer (0x%p) and their champion name (%s) but it was null!",
				localPlayer,
				localPlayer && localPlayer->GetChampionName() ? localPlayer->GetChampionName() : "nullptr");
			return 0.0f;
		}

		// Get the range from the champion data
		f32 range = 0;
		if (key == Keyboard::KeyQ)
			range = championData.GetCastRange(0, *localPlayer);
		else if (key == Keyboard::KeyW)
			range = championData.GetCastRange(1, *localPlayer);
		else if (key == Keyboard::KeyE)
			range = championData.GetCastRange(2, *localPlayer);
		else if (key == Keyboard::KeyR)
			range = championData.GetCastRange(3, *localPlayer);

		auto getSpellRange = GetChampionScript(localPlayer->GetChampionName())["GetSpellRange"];
		if (getSpellRange.valid())
		{
			f32 newRange = getSpellRange(key, castTime, *localPlayer->GetChampionLevel());
			if (newRange > 0.1f)
			{
				range = newRange;
				// GameOverlay_LogDebug(g_scriptLog, "%s.GetSpellRange(%c, %.1f, %d) => %.0f", localPlayer->GetChampionName(), key, castTime, localPlayer->GetChampionLevel(), newRange);
			}
			else
			{
				// GameOverlay_LogDebug(g_scriptLog, "%s.GetSpellRange(%c, %.1f, %d) was not processed.", localPlayer->GetChampionName(), key, castTime, localPlayer->GetChampionLevel());
			}
		}
		else
		{
			// GameOverlay_LogWarning(g_scriptLog, "GetSpellRange was not available for %s", localPlayer->GetChampionName());
		}

		return range;
	}

	Keyboard GetSpellKeyByName(std::string_view spellIdentifier)
	{
		Leacon_Profiler;
		auto localPlayer = GetLocalPlayer();
		if (localPlayer == nullptr)
			return Keyboard::KeyNone;

		const SpellBook* spellBook = localPlayer->GetSpellBook();
		if (spellBook == nullptr)
			return Keyboard::KeyNone;
		
		for (u32 i = 0; i < SpellSlotID::Max; i++)
		{
			auto spell = spellBook->GetSpell(i);
			if (IsValidSpell(spell) == false || spell->GetName() != spellIdentifier)
				continue;

			// Spells
			switch (i)
			{
			case 0: return Keyboard::KeyQ;
			case 1: return Keyboard::KeyW;
			case 2: return Keyboard::KeyE;
			case 3: return Keyboard::KeyR;
				
			case 4: return Keyboard::KeyD;
			case 5: return Keyboard::KeyF;

			case 6: return Keyboard::Key1;
			case 7: return Keyboard::Key2;
			case 8: return Keyboard::Key3;
			case 9: return Keyboard::Key5; // NOTE: Skipped Trinket (button 4)
			case 10: return Keyboard::Key6;
			case 11: return Keyboard::Key7;
			}
		}

		return Keyboard::KeyNone;
	}

	static bool HasSpell(std::string_view spellIdentifier)
	{
		return GetSpellKeyByName(spellIdentifier) != Keyboard::KeyNone;
	}
	
	static bool HasItem(std::string_view itemId)
	{
		std::string spellId;
		if (GetSpellNameByItemId(itemId.data(), spellId) == false)
			return false;
		
		return HasSpell(spellId);
	}

	static bool CastSpellByName(std::string_view spellIdentifier)
	{
		Leacon_Profiler;
		Keyboard key = GetSpellKeyByName(spellIdentifier);
		if (key == Keyboard::KeyNone)
			return false;

		FeignKeyPress(key);
		return true;
	}

	static const Spell* GetSpellByKey(const SpellBook* spellbook, Keyboard key)
	{
		Leacon_Profiler;
		switch (key)
		{
		case Keyboard::KeyQ: return spellbook->GetSpell(SpellSlotID::Q);
		case Keyboard::KeyW: return spellbook->GetSpell(SpellSlotID::W);
		case Keyboard::KeyE: return spellbook->GetSpell(SpellSlotID::E);
		case Keyboard::KeyR: return spellbook->GetSpell(SpellSlotID::R);

		case Keyboard::Key1: return spellbook->GetSpell(SpellSlotID::Item1);
		case Keyboard::Key2: return spellbook->GetSpell(SpellSlotID::Item2);
		case Keyboard::Key3: return spellbook->GetSpell(SpellSlotID::Item3);
		case Keyboard::Key4: return spellbook->GetSpell(SpellSlotID::Item4);
		case Keyboard::Key5: return spellbook->GetSpell(SpellSlotID::Item5);
		case Keyboard::Key6: return spellbook->GetSpell(SpellSlotID::Item6);
		}

		return nullptr;
	}

	static LockOnType GetSpellLockOnType(Keyboard key)
	{
		Leacon_Profiler;

		// Get the override from scripts
		auto localPlayer = GetLocalPlayer();
		if (IsValidGameObject(localPlayer) == false)
			return LockOnType::Soft;

		auto getOverride = GetChampionScript(localPlayer->GetChampionName())["GetSpellLockOnTypeOverride"];
		if (getOverride.valid())
			return getOverride(key);

		const auto* spell = GetSpellByKey(localPlayer->GetSpellBook(), key);
		if (IsValidSpell(spell) == false)
			return LockOnType::Soft;

		auto targetingType = GetChampionData().GetTargetingType(spell->GetName());
		switch (targetingType)
		{
		// case ChampionData::TargetingType::TargetOrLocation: // This type has some garbage in it 
		case ChampionData::TargetingType::Target:
			return LockOnType::Hard;
		}

		return LockOnType::Soft;
	}

	static bool IsDashSpell(Keyboard key)
	{
		Leacon_Profiler;

		// Get the override from scripts
		auto localPlayer = GetLocalPlayer();
		if (IsValidGameObject(localPlayer) == false)
			return false;

		auto getOverride = GetChampionScript(localPlayer->GetChampionName())["GetIsDashSpellOverride"];
		if (getOverride.valid())
			return getOverride(key);

		const auto* spell = GetSpellByKey(localPlayer->GetSpellBook(), key);
		if (IsValidSpell(spell) == false)
			return LockOnType::Soft;

		return GetChampionData().IsDash(spell->GetName());
	}

	static std::vector<const Spell*> GetItemSpells()
	{
		std::vector<const Spell*> result;
	#if _DEBUG
		if (IsApp())
		{
			GameOverlay_Assert(g_scriptLog, false); // TODO!
			/*static SpellInfo testSpellInfo;
			strcpy(testSpellInfo.name.data.content, "Item2420");
			testSpellInfo.name.length = strlen(testSpellInfo.name.data.content);
			testSpellInfo.notCastable = false;

			static Spell testSpell;
			testSpell.cooldown = 0.0f;
			testSpell.level = 1;
			testSpell.slot = 0;
			testSpell.spellInfo = &testSpellInfo;
			
			result.push_back(&testSpell);*/
			return result;
		}
	#endif

		Leacon_Profiler;
		GameObject* localPlayer = GetLocalPlayer();
		auto spellBook = localPlayer ? localPlayer->GetSpellBook() : nullptr;
		if (spellBook == nullptr)
			return result;
		
		// Skip first 6 (2 summoner spells, 4 champion spells)
		for (u32 i = 6; i <= SpellSlotID::Item6; i++)
		{
			auto spell = spellBook->GetSpell(i);
			if (IsValidSpell(spell))
				result.push_back(spell);
		}

		return result;
	}

	bool IsShopOpen()
	{
		Leacon_Profiler;
		auto gui = GetGuiMenu();
		return gui ? gui->openedLayer == GuiMenuLayerType::Shop : false;
	}

	void ToggleShop()
	{
		Leacon_Profiler;
		FeignKeyPress(Keyboard::KeyP);
	}

	std::string GetItemName(const Spell* spell)
	{
		Leacon_Profiler;
		if (IsValidSpell(spell) == false)
			return "InvalidSpell";

		std::string result;
		if (GetItemNameBySpell(spell->GetName(), result))
			return result;

		return spell->GetName();
	}

	std::string GetItemId(Spell* spell)
	{
		Leacon_Profiler;
		if (IsValidSpell(spell) == false)
			return "0";

		std::string result;
		if (GetItemIdBySpellName(spell->GetName(), result))
			return result;

		return "0";
	}

	bool IsLikelyBasicAttack(Spell* spell)
	{
		Leacon_Profiler;
		if (IsValidSpell(spell) == false)
			return false;

		return spell->GetName() == nullptr && spell->GetLevel() == 0;
	}

	static Keyboard GetKeyboardKeyBySpell(const char* spellIdentifier)
	{
#if _DEBUG
		if (IsApp())
		{
			return Keyboard::Key1;
		}
#endif

		Leacon_Profiler;
		GameObject* localPlayer = GetLocalPlayer();
		auto spellBook = localPlayer ? localPlayer->GetSpellBook() : nullptr;
		if (spellBook == nullptr)
			return Keyboard::KeyNone;

		for (u32 i = 6; i <= SpellSlotID::Item6; i++)
		{
			auto spell = spellBook->GetSpell(i);
			if (spell == nullptr)
				continue;

			std::string itemName = GetItemName(spell);
			if (itemName == spellIdentifier)
			{
				int keyIndex = (int)Keyboard::Key0 + i - 5;
				return (Keyboard)keyIndex;
			}
		}

		return Keyboard::KeyNone;
	}

	static std::string GetSpellNameByItemIdLua(const char* itemId)
	{
		Leacon_Profiler;
		std::string result;
		if (GetSpellNameByItemId(itemId, result))
			return result;
		return nullptr;
	}

	void AddGameScriptSystem(sol::state& state)
	{
		Leacon_Profiler;
		state["gameTime"] = 0.0f;

		state.set_function("IsGameReady", &IsGameReady);
		state.set_function("GetGameTime", &GetGameTime);
		state.set_function("GetObjectUnderMouse", &GetObjectUnderMouse);
		state.set_function("GetLocalPlayer", &GetLocalPlayer);
		state.set_function("GetNearestObject", sol::overload(&GetNearestObject, []() { return GetNearestObject(nullptr); }));
		state.set_function("GetNearestChampion", sol::overload(&GetNearestChampion, []() { return GetNearestChampion(nullptr); }));
		state.set_function("GetWindowCenter", &GetWindowCenter);
		state.set_function("GetClientGlobalPosition", &GetClientGlobalPosition);
		state.set_function("WorldToScreen", &WorldToScreen);
		state.set_function("GetSpellCastRange", &GetSpellCastRange);
		state.set_function("IsValidGameObject", &IsValidGameObject);
		state.set_function("GetSelectedGameObject", &GetSelectedGameObject);
		state.set_function("HasSpell", &HasSpell);
		state.set_function("HasItem", &HasItem);
		state.set_function("GetSpellKeyByName", &GetSpellKeyByName);
		state.set_function("CastSpellByName", &CastSpellByName);
		state.set_function("GetKeyboardKeyBySpell", &GetKeyboardKeyBySpell);
		state.set_function("IsValidSpell", &IsValidSpell);
		state.set_function("GetSpellNameByItemId", &GetSpellNameByItemIdLua);
		state.set_function("GetItemSpells", &GetItemSpells);
		state.set_function("GetSpellLockOnType", &GetSpellLockOnType);
		state.set_function("IsDashSpell", &IsDashSpell);
		state.set_function("IsShopOpen", &IsShopOpen);
		state.set_function("ToggleShop", &ToggleShop);

		state["GameObjectType"] = state.create_table();
		state["GameObjectType"]["Unknown"] = GameObjectType::Unknown;
		state["GameObjectType"]["Champion"] = GameObjectType::Champion;
		state["GameObjectType"]["Minion"] = GameObjectType::Minion;
		state["GameObjectType"]["Turret"] = GameObjectType::Turret;
		state["GameObjectType"]["Monster"] = GameObjectType::Monster;
		state["GameObjectType"]["Plant"] = GameObjectType::Plant;
		state["GameObjectType"]["Inhibitor"] = GameObjectType::Inhibitor;
		state["GameObjectType"]["Nexus"] = GameObjectType::Nexus;
		
		state["LockOnType"] = state.create_table();
		state["LockOnType"]["Hard"] = LockOnType::Hard;
		state["LockOnType"]["Soft"] = LockOnType::Soft;
		state["LockOnType"]["None"] = LockOnType::None;
		
		state.new_usertype<Spell>
		(
			"Spell",
			
			"level", sol::property(&Spell::GetLevel),
			"isCastable", sol::property(&Spell::IsCastable),
			"isOnCooldown", sol::property(&Spell::IsOnCooldown),
			"name", sol::property(&Spell::GetName),
			"itemName", sol::property(&GetItemName),
			"itemId", sol::property(&GetItemId),
			"isValid", sol::property(&IsValidSpell),
			"isLikelyBasicAttack", sol::property(&IsLikelyBasicAttack)
		);

		state.new_usertype<SpellBook>
		(
			"SpellBook",

			"activeCast", sol::property(&SpellBook::GetActiveCast),

			"GetSpell", &SpellBook::GetSpell,
			"GetSpellByKey", &GetSpellByKey
		);

		state.new_usertype<GameObject>
		( 
			"GameObject",
			"name", sol::property(&GameObject::GetName),
			"position", sol::property(&GameObject::GetPosition),
			"team", sol::property(&GameObject::GetTeam),
			"attackRange", sol::property([](GameObject* obj) { return obj ? obj->GetAttackRangeF64() + GetChampionData().GetGameplayRadius() : 0.0; }),
			"type", sol::property(&GameObject::GetType),
			"spellBook", sol::property(&GameObject::GetSpellBook),
			"health", sol::property(&GameObject::GetHealth),
			"maxHealth", sol::property(&GameObject::GetMaxHealth),

			"TryAttack", &TryAttack,
			"MoveTo", &MovePlayerTo,
			"StopMoving", &StopMoving,

			sol::meta_function::equal_to, [](GameObject* a, GameObject* b) { return a == b; }
		);
	}

	void InitScriptGame(sol::state& state)
	{
		InitOrders();
	}

	void UpdateScriptGame(sol::state& state)
	{
		Leacon_Profiler;
		state["gameTime"] = GetGameTime();

		// Get nearest object
		DetermineNearestObject(g_nearestObject, GetAttackableManager());
		
		// Get nearest champion
		if (g_nearestObject && g_nearestObject->GetType() == GameObjectType::Champion)
			g_nearestChampion = g_nearestObject;
		else
			DetermineNearestObject(g_nearestChampion, GetHeroManager());

		UpdateOrders();
	}

	void DestroyScriptGame()
	{
		Leacon_Profiler;
		g_nearestObject = nullptr;
		g_nearestChampion = nullptr;
	}
}