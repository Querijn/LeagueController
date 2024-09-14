#pragma once

#include <league_controller/json.hpp>

#include <league_lib/bin/bin.hpp>

#include <skerm/tokens.hpp>
#include <spek/util/hash.hpp>
#include <glm/vec3.hpp>
#include <string_view>
#include <vector>

namespace LeagueController
{
	struct GameObject;
	bool IsValidChampionId(const char* inId);

	struct LOLStaticData
	{
		Spek::File::Handle handle;
		nlohmann::json json;
	};

	struct StatRow
	{
		i32 base = 0;
		i32 bonus = 0;
		i32 total = 0;
	};

	struct ChampionStat
	{
		i32 flat = 0;
		i32 percent = 0;
		i32 perLevel = 0;
		i32 percentPerLevel = 0;

		i32 ByKey(std::string_view key) const;
	};

	struct ChampionStats
	{
		ChampionStat health;
		ChampionStat healthRegen;
		ChampionStat mana;
		ChampionStat manaRegen;
		ChampionStat armor;
		ChampionStat magicResistance;
		ChampionStat attackDamage;
		ChampionStat attackRange;
		ChampionStat attackSpeed;
		ChampionStat attackSpeedBonus;
		ChampionStat movementSpeed;
		ChampionStat gameplayRadius;

		const ChampionStat& ByKey(std::string_view key) const;
	};

	struct ScalingComponent
	{
		std::string label;
		std::vector<i32> value;
		std::vector<std::string> scaling;
	};

	struct ScalingNumber
	{
		std::string label;
		std::string formula;
		std::vector<ScalingComponent> components;
	};

	struct SkillComponent
	{
		std::string name;
		std::vector<std::string> conditions;
		std::string description;
		std::vector<ScalingNumber> scaling;
		ScalingNumber cost;
		ScalingNumber range;
		ScalingNumber cooldown;
	};

	struct SkillLevel
	{
		i32 start;
		i32 max;
		std::vector<i32> requirement;
	};
	
	struct ChampionSkill
	{
		std::string key; // "A" | "P" | "Q" | "W" | "E" | "R";
		std::vector<SkillComponent> components;
		SkillLevel level;
		std::string transform; // 'automatic' | 'manual' | 'none';
	};

	struct ChampionForm
	{
		// std::string transform; // this will contain transform conditions that automate the change in case of iChampion.transform == 'condition' || 'skill'
		std::string name;
		std::string icon;
		std::vector<ChampionSkill> skills;
		ChampionStats stats;
	};

	class ChampionData
	{
	public:
		enum class TargetingType : u32
		{
			None = 0,
			Area = Spek::FNV("Area"),
			AreaClamped = Spek::FNV("AreaClamped"),
			Cone = Spek::FNV("Cone"),
			DragDirection = Spek::FNV("DragDirection"),
			Location = Spek::FNV("Location"),
			LocationClamped = Spek::FNV("LocationClamped"),
			Self = Spek::FNV("Self"),
			SelfAoe = Spek::FNV("SelfAoe"),
			Target = Spek::FNV("Target"),
			TargetOrLocation = Spek::FNV("TargetOrLocation"),
			TerrainLocation = Spek::FNV("TerrainLocation"),
			TerrainType = Spek::FNV("TerrainType"),
			Direction = Spek::FNV("Direction"),
			ZeriDirection = 0x5b9614dc, // TODO
		};

		ChampionData() {}

		void AddChampion(std::string_view champName);

		size_t GetCount() const;
		size_t GetAbilityCount() const;

		float GetMaxCastRange() const;
		float GetCastRange(int abilityIndex, const GameObject& champion) const;
		TargetingType GetTargetingType(int abilityIndex) const;
		TargetingType GetTargetingType(std::string_view abilityName) const;
		bool IsDash(int abilityIndex) const;
		bool IsDash(std::string_view abilityName) const;

		float GetGameplayRadius() const;

	private:
		std::map<std::string, LOLStaticData> m_jsonFiles;
		std::map<std::string, LeagueLib::Bin> m_binFiles;
		std::map<std::string, const LeagueLib::BinVariable*> m_abilityByName;
		std::vector<const LeagueLib::BinVariable*> m_abilities;
		std::map<std::string, Skerm::EquationToken> m_rangeCache;
		std::unordered_map<i32, ChampionForm> m_formCache;
		std::map<std::string, bool> m_loadedChampions;
		float m_gameplayRadius = 65.0f;

		void FillDataCallback(LeagueLib::Bin& bin, std::string_view championName);
		const LeagueLib::BinVariable* GetAbilityByName(std::string_view abilityName) const;
		const LeagueLib::BinVariable* GetAbilityByIndex(int abilityIndex) const;
		float GetCastRange(const LeagueLib::BinVariable& ability) const;
		void ParseFormulas(std::string_view championName, int lvl);
		void ParseStats(std::string_view championName, int lvl, ChampionForm& form, std::unordered_map<std::string, StatRow>& calculatedStats, std::unordered_map<std::string, i32>& current, std::unordered_map<std::string, i32>& overriden);
		const ChampionForm& GetForm(const nlohmann::json& src, i32 formIndex);
	};
}