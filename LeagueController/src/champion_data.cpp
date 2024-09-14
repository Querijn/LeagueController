#include "champion_data.hpp"

#include <league_internals/offsets.hpp>
#include <league_internals/game_object.hpp>

#include <game_overlay/overlay.hpp>
#include <skerm/tokeniser.hpp>

#include <set>
#include <string>

namespace LeagueController
{
	GameOverlay::LogCategory g_champData("ChampionData");

	using namespace LeagueLib;
	using namespace Spek;
	ChampionData::TargetingType GetTargetingTypeInternal(BinVarPtr ability);
	bool IsDash(BinVarPtr ability);

	void ChampionData::FillDataCallback(LeagueLib::Bin& bin, std::string_view upperFirst)
	{
		Leacon_Profiler;
		if (bin.GetLoadState() != Spek::File::LoadState::Loaded)
		{
			GameOverlay_LogInfo(g_champData, "Could not load champion data for '%s'", upperFirst.data());
			return;
		}

		char rootName[1024];
		snprintf(rootName, 1024, "Characters/%s/CharacterRecords/Root", upperFirst.data());

		std::set<u32> abilityHashes;
		BinVarRef root = bin[rootName];

		// Find gameplay radius
		const double* gameplayRangeOverride = root["overrideGameplayCollisionRadius"].As<double>();
		if (gameplayRangeOverride)
		{
			m_gameplayRadius = *gameplayRangeOverride;
			GameOverlay_LogDebug(g_champData, "'%s' has a gameplay radius override: %f", upperFirst.data(), m_gameplayRadius);
		}

		// Find spells
		BinVarRef abilityArray = root["mAbilities"];
		for (BinVarRef abilityEntry : *abilityArray.As<BinArray>())
		{
			BinVarRef ability = bin[*abilityEntry.As<u32>()];
			u32 rootSpellHash = *ability["mRootSpell"].As<u32>();
			abilityHashes.insert(rootSpellHash);

			const BinArray& childSpellArray = *ability["mChildSpells"].As<BinArray>();
			for (BinVarRef childSpell : childSpellArray)
				abilityHashes.insert(*childSpell.As<u32>());
		}

		for (u32 abilityHash : abilityHashes)
			m_abilities.push_back(&bin[abilityHash]);

		for (auto ability : m_abilities)
			GetTargetingTypeInternal(ability);
		GameOverlay_LogInfo(g_champData, "Loaded champion data for '%s'", upperFirst.data());
	}

	const LeagueLib::BinVariable* ChampionData::GetAbilityByName(std::string_view abilityName) const
	{
		Leacon_Profiler;
		if (abilityName.empty())
			return nullptr;

		u32 abilityHash = Spek::FNV(abilityName.data());
		auto hasName = [abilityHash, abilityName](BinVarRef nameContainer)
		{
			// if (nameContainer.IsValid() == false)
			// 	return false;

			const std::string* scriptName = nameContainer.As<std::string>();
			if (scriptName != nullptr && *scriptName == abilityName)
				return true;

			const u32* scriptNameHash = nameContainer.As<u32>();
			if (scriptNameHash != nullptr && *scriptNameHash == abilityHash)
				return true;
			return false;
		};

		for (auto ability : m_abilities)
		{
			if (ability == nullptr) // Is this possible?
				continue;

			BinVarRef scriptNameContainer = (*ability)["mScriptName"];
			if (hasName(scriptNameContainer))
				return ability;

			BinVarRef objectNameContainer = (*ability)["mSpell"]["mClientData"]["mTooltipData"]["mObjectName"];
			if (hasName(objectNameContainer))
				return ability;
		}

		return nullptr;
	}

	template<typename T>
	void from_json(const nlohmann::json& src, T& target)
	{
		target = src.get<T>();
	}

	template<typename T>
	void from_json(const nlohmann::json& src, std::vector<T>& target)
	{
		for (const nlohmann::json& entry : src)
		{
			T targetEntry;
			from_json(entry, targetEntry);
			target.push_back(targetEntry);
		}
	}

	void from_json(const nlohmann::json& src, StatRow& target)
	{
		from_json(src["base"], target.base);
		from_json(src["bonus"], target.bonus);
		from_json(src["total"], target.total);
	}

	void from_json(const nlohmann::json& src, ChampionStat& target)
	{
		from_json(src["flat"], target.flat);
		from_json(src["percent"], target.percent);
		from_json(src["perLevel"], target.perLevel);
		from_json(src["percentPerLevel"], target.percentPerLevel);
	}

	void from_json(const nlohmann::json& src, ChampionStats& target)
	{
		from_json(src["health"], target.health);
		from_json(src["healthRegen"], target.healthRegen);
		from_json(src["mana"], target.mana);
		from_json(src["manaRegen"], target.manaRegen);
		from_json(src["armor"], target.armor);
		from_json(src["magicResistance"], target.magicResistance);
		from_json(src["attackDamage"], target.attackDamage);
		from_json(src["attackRange"], target.attackRange);
		from_json(src["attackSpeed"], target.attackSpeed);
		from_json(src["attackSpeedBonus"], target.attackSpeedBonus);
		from_json(src["movementSpeed"], target.movementSpeed);
	}

	void from_json(const nlohmann::json& src, ScalingComponent& target)
	{
		from_json(src["label"], target.label);
		from_json(src["value"], target.value);
		from_json(src["scaling"], target.scaling);
	}

	void from_json(const nlohmann::json& src, ScalingNumber& target)
	{
		from_json(src["label"], target.label);
		from_json(src["formula"], target.formula);
		from_json(src["components"], target.components);
	}

	void from_json(const nlohmann::json& src, SkillComponent& target)
	{
		from_json(src["name"], target.name);
		from_json(src["conditions"], target.conditions);
		from_json(src["description"], target.description);
		from_json(src["scaling"], target.scaling);
		from_json(src["cost"], target.cost);
		from_json(src["range"], target.range);
		from_json(src["cooldown"], target.cooldown);
	}

	void from_json(const nlohmann::json& src, SkillLevel& target)
	{
		from_json(src["start"], target.start);
		from_json(src["max"], target.max);
		from_json(src["requirement"], target.requirement);
	}

	void from_json(const nlohmann::json& src, ChampionSkill& target)
	{
		from_json(src["key"], target.key);
		from_json(src["components"], target.components);
		from_json(src["level"], target.level);
		//from_json(src["transform"], target.transform);
	}

	void from_json(const nlohmann::json& src, ChampionForm& target)
	{
		//from_json(src["transform"], target.transform);
		from_json(src["name"], target.name);
		from_json(src["icon"], target.icon);
		from_json(src["skills"], target.skills);
		from_json(src["stats"], target.stats);
	}

	const ChampionForm& ChampionData::GetForm(const nlohmann::json& src, i32 formIndex)
	{
		Leacon_Profiler;
		auto index = m_formCache.find(formIndex);
		if (index != m_formCache.end())
			return index->second;

		if (src["forms"].is_null())
			return ChampionForm();

		if (src["forms"].is_array() == false)
			return ChampionForm();

		if (formIndex < 0 || formIndex >= src.size())
			return ChampionForm();

		const auto& form = src["forms"][formIndex];
		if (form.is_null())
			return ChampionForm();

		ChampionForm result;
		from_json(form, result);
		m_formCache[formIndex] = result;
		return m_formCache[formIndex];
	}

	void ChampionData::ParseStats(std::string_view championName, int lvl, ChampionForm& currentForm, std::unordered_map<std::string, StatRow>& calculatedStats, std::unordered_map<std::string, i32>& current, std::unordered_map<std::string, i32>& overriden)
	{
		Leacon_Profiler;
		const nlohmann::json& src = m_jsonFiles[championName.data()].json;
		static const std::vector<const char*> basicStats = { "health", "healthRegen", "mana", "manaRegen", "armor", "magicResistance", "movementSpeed", "attackDamage", "attackSpeed", "attackRange", "gameplayRadius" };

		auto set = [&current](const std::string& s, i32 n)
		{
			current[s] = n;
		};
		auto get = [&calculatedStats, &current, &overriden, set](const std::string& i, const std::string& s) -> i32
		{
			auto index = current.find(s);
			if (i == "current")
			{
				if (index == current.end())
					set(s, 0);

				index = overriden.find(s);
				return (overriden.end() == index) ? current[s] : overriden[s];
			}

			if (i == "percent" && index != current.end())
				return current[s] / calculatedStats[s].total;
			else if (i == "missingpercent" && index != current.end())
				return 1 - current[s] / calculatedStats[s].total;
			else
			{
				if (i == "base") return calculatedStats[s].base;
				if (i == "bonus") return calculatedStats[s].bonus;
				if (i == "total") return calculatedStats[s].total;
			}

			GameOverlay_AssertF(g_champData, false, "Unable to get %s, %s", i.c_str(), s.c_str());
			return 0;
		};

		const auto& baseStats = currentForm.stats;
		auto statGrowthScam = 0.7025 + 0.0175 * lvl;
		static ChampionStat zeroStat;

		for (auto key : basicStats)
		{
			const ChampionStat& stat = baseStats.ByKey(key);
			auto result = stat.flat + (stat.perLevel * lvl * statGrowthScam);
			calculatedStats[key].base = result;

			// Percent stuff
			result *= 1 + (stat.percent + stat.percentPerLevel * lvl) / 100;
			if (key == "attackSpeed")
				result += baseStats.attackSpeedBonus.flat * (stat.perLevel * lvl / 100) * statGrowthScam;

			calculatedStats[key].total = result;
			calculatedStats[key].bonus = result - calculatedStats[key].base;
		}
	}

	template <typename T>
	std::vector <T> split(T string, T delim) {
		std::vector <T> result;
		size_t from = 0, to = 0;
		while (T::npos != (to = string.find(delim, from))) {
			result.push_back(string.substr(from, to - from));
			from = to + delim.length();
		}
		result.push_back(string.substr(from, to));
		return result;
	}

	std::string split_join(const std::string& target, const std::string& denom, const std::string& join)
	{
		auto strings = split(target, denom);
		
		std::string imploded;
		bool isFirst = true;
		for (const auto& s : strings)
		{
			if (isFirst)
				isFirst = false;
			else
				imploded += join;
			imploded += s;
		}
		return imploded;
	}

	void ChampionData::ParseFormulas(std::string_view championName, int lvl)
	{
		Leacon_Profiler;
		const nlohmann::json& src = m_jsonFiles[championName.data()].json;
		
		std::unordered_map<std::string, StatRow> calculatedStats;
		std::unordered_map<std::string, i32> current;
		std::unordered_map<std::string, i32> overriden;

		auto set = [&current](const std::string& s, i32 n)
		{
			current[s] = n;
		};
		auto get = [&calculatedStats, &current, &overriden, set](const std::string& i, const std::string& s) -> i32
		{
			auto index = current.find(s);
			if (i == "current")
			{
				if (index == current.end())
					set(s, 0);

				index = overriden.find(s);
				return (overriden.end() == index) ? current[s] : overriden[s];
			}

			if (i == "percent" && index != current.end())
				return current[s] / calculatedStats[s].total;
			else if (i == "missingpercent" && index != current.end())
				return 1 - current[s] / calculatedStats[s].total;
			else
			{
				if (i == "base") return calculatedStats[s].base;
				if (i == "bonus") return calculatedStats[s].bonus;
				if (i == "total") return calculatedStats[s].total;
			}

			GameOverlay_AssertF(g_champData, false, "Unable to get %s, %s", i.c_str(), s.c_str());
			return 0;
		};
		
		set("championLevel", lvl);
		ChampionForm currentForm = GetForm(src, get("current", "championForm"));
		ParseStats(championName, lvl, currentForm, calculatedStats, current, overriden);

		//iterate through skills
		for (ChampionSkill& skill : currentForm.skills)
		{
			Leacon_ProfilerSection("ParseSkill");
			char skillFormKey[64];
			snprintf(skillFormKey, 64, "skillForm%s", skill.key.c_str());

			auto rangeData = skill.components[get("current", skillFormKey)].range;
			f32 calculatedRange = 0;
			auto formula = rangeData.formula;
			//iterate through components
			for(auto& component : rangeData.components)
			{
				i32 skillRank = get(component.scaling[2], component.scaling[1]);

				i32 value;
				size_t index = component.scaling[1].find("Level");
				if (index != std::string::npos)
					value = component.value[skillRank ? skillRank - 1 : 0];
				else
					value = skillRank * component.value[0];


				char componentLabelIdent[256];
				snprintf(componentLabelIdent, 256, "@%s@", component.label.c_str());
				formula = split_join(formula, componentLabelIdent, std::to_string(value));
			}

			{
				Leacon_ProfilerSection("Tokenise");
				auto result = Skerm::Tokenize(formula.c_str());
				GameOverlay_AssertF(g_champData, result.Errors.empty(), "Errors occurred trying to reduce the equation '%s'.", formula.c_str());
				if (result.Errors.empty())
				{
					Leacon_ProfilerSection("Reduce");
					result.Equation.ReduceSelf();
					m_rangeCache[skill.key] = result.Equation;
				}
			}
		}

		GameOverlay_LogDebug(g_champData, "%s's formulas are now (re-)initialised.", championName.data());
	}

	void ChampionData::AddChampion(std::string_view champName)
	{
		Leacon_Profiler;
		// Read champion data file
		// https://raw.communitydragon.org/pbe/game/data/characters/akali/akali.bin.json
		
		// Create 'Viego' from 'viego'
		std::string upperFirst = std::string("Z") + champName.substr(1).data();
		upperFirst[0] = champName[0] >= 'a' && champName[0] <= 'z' ? champName[0] + ('A' - 'a') : champName[0];

		// Check if exists
		if (m_binFiles.find(upperFirst) != m_binFiles.end())
			return;

		char binFileName[1024];
		snprintf(binFileName, 1024, "data/characters/%s/%s.bin", champName.data(), champName.data());

		GameOverlay_LogInfo(g_champData, "Attempting to load '%s'", binFileName);
		m_binFiles[upperFirst].Load(binFileName, [this, upperFirst](LeagueLib::Bin& bin) { FillDataCallback(bin, upperFirst); });
		
		char jsonFileName[1024];
		snprintf(jsonFileName, 1024, "Data/staticdata/champion/%s.json", upperFirst.data());
		GameOverlay_LogInfo(g_champData, "Attempting to load '%s'", jsonFileName);

		std::string* champNamePtr = new std::string(upperFirst);
		m_jsonFiles[upperFirst].handle = Spek::File::Load(jsonFileName, [this, champNamePtr](Spek::File::Handle file)
		{
			GameOverlay_AssertF(g_champData, file && file->GetLoadState() == Spek::File::LoadState::Loaded, "Failed to load json file '%s', 0x%x, %s", file ? file->GetName().c_str() : "nullptr", file.get(), champNamePtr->c_str());
			m_jsonFiles[*champNamePtr].json = nlohmann::json::parse(file->GetDataAsString());
			ParseFormulas(*champNamePtr, 1);
			delete champNamePtr;
		});
	}
	
	size_t ChampionData::GetCount() const
	{
		return m_binFiles.size();
	}

	size_t ChampionData::GetAbilityCount() const
	{
		return m_abilities.size();
	}

	float ChampionData::GetCastRange(BinVarRef ability) const
	{
		Leacon_Profiler;
		float max = 0.0f;
		auto handle = [&max](const BinArray* radiusList)
		{
			for (BinVarRef castRadius : *radiusList)
			{
				const f64* asFloat = castRadius.As<f64>();
				if (asFloat && *asFloat > max)
				{
					max = *asFloat;
					continue;
				}

				/*const i32* asInteger = castRadius.As<i32>();
				if (asInteger && *asInteger > max)
				{
					max = (float)*asInteger;
					continue;
				}*/
			}
		};
		BinVarRef spell = ability["mSpell"];
		if (const BinArray* castRadiusDisplayList = spell["castRangeDisplayOverride"].As<BinArray>())
			handle(castRadiusDisplayList);
		else if (const BinArray* castRadius = spell["castRadius"].As<BinArray>())
			handle(castRadius);

		return max;
	}

	float ChampionData::GetMaxCastRange() const
	{
		Leacon_Profiler;
		float max = 0.0f;
		for (auto ability : m_abilities)
		{
			if (ability == nullptr) // Is this possible?
				continue;

			float castRange = GetCastRange(*ability);
			if (max < castRange)
				max = castRange;
		}

		return max;
	}

	const Spell* GetLocalPlayerSpellByIndex(int abilityIndex)
	{
		Leacon_Profiler;
		if (abilityIndex < 0 || abilityIndex >= 4)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get local player spell ID by index %d, because it is not a valid ability index (0~3)", abilityIndex);
			return nullptr;
		}

		GameObject* localPlayer = GetLocalPlayer();
		if (localPlayer == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get local player spell ID by index %d, because we have no local player.", abilityIndex);
			return nullptr;
		}

		const SpellBook* spellBook = localPlayer->GetSpellBook();
		if (spellBook == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get local player spell ID by index %d, because we have no spellbook.", abilityIndex);
			return nullptr;
		}

		return spellBook->GetSpell(abilityIndex);
	}

	const LeagueLib::BinVariable* ChampionData::GetAbilityByIndex(int abilityIndex) const
	{
		Leacon_Profiler;
		const Spell* spell = GetLocalPlayerSpellByIndex(abilityIndex);
		if (spell == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get ability by index %d, because we were unable to get local player spell by index.", abilityIndex);
			return nullptr;
		}

		const char* spellName = spell->GetName();
		BinVarPtr ability = GetAbilityByName(spellName);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get ability by index %d, because we were unable to resolve the name '%s'.", abilityIndex, spellName);
			return nullptr;
		}

		return ability;
	}

	const char* GetSpellKeyByAbilityIndex(int abilityIndex)
	{
		switch (abilityIndex)
		{
		case 0: return "Q";
		case 1: return "W";
		case 2: return "E";
		case 3: return "R";
		}
		
		GameOverlay_AssertF(g_champData, false, "Invalid ability index %d", abilityIndex);
		return nullptr;
	}

	float ChampionData::GetCastRange(int abilityIndex, const GameObject& champion) const
	{
		Leacon_Profiler;
		auto index = m_rangeCache.find(GetSpellKeyByAbilityIndex(abilityIndex));
		i32* level = champion.GetChampionLevel();
		if (index != m_rangeCache.end() && level)
		{
			// TODO: Bust this cache better
			static i32 lastLevel = 1;
			if (*level != lastLevel)
			{
				const_cast<ChampionData&>(*this).ParseFormulas(champion.GetChampionName(), *champion.GetChampionLevel());
				lastLevel = *level;
			}

			double result;
			if (index->second.Calculate(&result, 1) == 1)
				return result;
		}

		BinVarPtr ability = GetAbilityByIndex(abilityIndex);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get range of ability %d, see previous errors.", abilityIndex);
			return 0.0f;
		}

		return GetCastRange(*ability);
	}

	ChampionData::TargetingType GetTargetingTypeInternal(BinVarPtr ability)
	{
		Leacon_Profiler;
		const BinObject* targetingTypeData = (*ability)["mSpell"]["mTargetingTypeData"].As<BinObject>();
		if (targetingTypeData == nullptr)
			return ChampionData::TargetingType::Target;

		return (ChampionData::TargetingType)targetingTypeData->GetTypeHash();
	}

	bool IsDash(BinVarPtr ability)
	{
		Leacon_Profiler;
		const BinArray* targetingTypeData = (*ability)["mSpell"]["mSpellTags"].As<BinArray>();
		if (targetingTypeData == nullptr)
			return false;

		for (const auto& e : *targetingTypeData)
		{
			const std::string* val = e.As<std::string>();
			if (val != nullptr && *val == "PositiveEffect_MoveBlock")
				return true;
		}

		return false;
	}

	ChampionData::TargetingType ChampionData::GetTargetingType(int abilityIndex) const
	{
		Leacon_Profiler;
		BinVarPtr ability = GetAbilityByIndex(abilityIndex);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get targeting type for ability index %d, see previous errors.", abilityIndex);
			return TargetingType::None;
		}

		return GetTargetingTypeInternal(ability);
	}

	ChampionData::TargetingType ChampionData::GetTargetingType(std::string_view abilityName) const
	{
		Leacon_Profiler;
		BinVarPtr ability = GetAbilityByName(abilityName);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get targeting type for ability '%s', see previous errors.", abilityName.data());
			return TargetingType::None;
		}

		return GetTargetingTypeInternal(ability);
	}

	bool ChampionData::IsDash(int abilityIndex) const
	{
		Leacon_Profiler;
		BinVarPtr ability = GetAbilityByIndex(abilityIndex);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get targeting type for ability index %d, see previous errors.", abilityIndex);
			return false;
		}

		return LeagueController::IsDash(ability);
	}

	bool ChampionData::IsDash(std::string_view abilityName) const
	{
		Leacon_Profiler;
		BinVarPtr ability = GetAbilityByName(abilityName);
		if (ability == nullptr)
		{
			GameOverlay_LogWarning(g_champData, "Unable to get targeting type for ability '%s', see previous errors.", abilityName.data());
			return false;
		}

		return LeagueController::IsDash(ability);
	}

	float ChampionData::GetGameplayRadius() const
	{
		return m_gameplayRadius;
	}

	bool IsValidChampionId(const char* championName)
	{
		Leacon_Profiler;
		if (championName == nullptr || strlen(championName) == 0)
			return false;
		
		// Champion name starts with one of these
		char c = championName[0];
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	}

	i32 ChampionStat::ByKey(std::string_view key) const
	{
		if (key == "flat") return flat;
		if (key == "percent") return percent;
		if (key == "perLevel") return perLevel;
		if (key == "percentPerLevel") return percentPerLevel;

		GameOverlay_Assert(g_champData, false);
		return *(i32*)(nullptr); // fuck u
	}

	const ChampionStat& ChampionStats::ByKey(std::string_view key) const
	{
		if (key == "health") return health;
		if (key == "healthRegen") return healthRegen;
		if (key == "mana") return mana;
		if (key == "manaRegen") return manaRegen;
		if (key == "armor") return armor;
		if (key == "magicResistance") return magicResistance;
		if (key == "attackDamage") return attackDamage;
		if (key == "attackRange") return attackRange;
		if (key == "attackSpeed") return attackSpeed;
		if (key == "attackSpeedBonus") return attackSpeedBonus;
		if (key == "movementSpeed") return movementSpeed;
		if (key == "gameplayRadius") return gameplayRadius;

		GameOverlay_Assert(g_champData, false);
		return *(ChampionStat*)(nullptr); // fuck u
	}
}