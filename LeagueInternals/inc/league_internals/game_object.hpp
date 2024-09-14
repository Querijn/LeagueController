#pragma once

#include <league_internals/flag_duo.hpp>

#include <glm/vec3.hpp>
#include <spek/util/types.hpp>
#include <vector>
#include <string>

namespace LeagueController
{
	struct StdString
	{
		union Data
		{
			struct { const char* pointer; };
			struct { char content[16]; };
		};

		Data data;
		u64 length;
		u64 capacity;

		static const u64 SSO_MAX_LEN = 15;

		bool is_sso() const { return capacity <= StdString::SSO_MAX_LEN; }

		const char* c_str() const
		{
			if (is_sso())
				return &data.content[0];
			else
				return data.pointer;
		}

		bool operator==(const char* other) const;
		bool operator!=(const char* other) const;

		bool operator==(const StdString& other) const;
		bool operator!=(const StdString& other) const;
	};

	enum SpellSlotID
	{
		Q = 0,
		W = 1,
		E = 2,
		R = 3,

		D = 4,
		F = 5,

		Item1 = 6,
		Item2 = 7,
		Item3 = 8,
		Item4 = 9,
		Item5 = 10,
		Item6 = 11,
		Trinket = 12,
		
		Max = 13,
	};

	struct SpellInfo
	{
		static u64 nameOffset;
		static u64 notCastableOffset;

		const StdString& GetName() const;
		bool IsCastable() const;
	};

	struct Spell
	{
		static u64 slotOffset;
		static u64 levelOffset;
		static u64 cooldownOffset;
		static u64 spellInfoOffset;

		bool IsOnCooldown() const;
		bool IsCastable() const;
		const char* GetName() const;

		int GetSlot() const;
		int GetLevel() const;
		f32 GetCooldown() const;
		const SpellInfo* GetSpellInfo() const;
	};

	struct SpellBook
	{
		static u64 activeCastOffset;
		static u64 activeCastSlotOffset;
		static u64 spellsOffset;

		const Spell* GetSpell(int index) const;
		const Spell* GetActiveCast() const;
	};

	enum GameObjectType
	{
		Unknown,
		Champion,
		Minion,
		Turret,
		Monster,
		Plant,
		Inhibitor,
		Nexus
	};

	struct GameObject
	{
		static u64 healthOffset;
		static u64 positionOffset;
		static u64 attackRangeOffset;
		static u64 spellBookOffset;
		static u64 itemSlotsOffset;
		static u64 championNameOffset;
		static u64 championLevelOffset;
		static bool ShouldBeValid();

		union
		{
			DEFINE_MEMBER_N(bool isTargetable,			0x0D04);
			DEFINE_MEMBER_N(int team,					0x0034);
			DEFINE_MEMBER_N(StdString name,				0x0054);
			DEFINE_MEMBER_N(bool isAlive,				0x0001); // REDACTED: THIS IS NOT THE CORRECT TYPE NOR OFFSET
			DEFINE_MEMBER_N(bool isVisible,				0x0274);
		};

		int GetTeam() const;
		bool IsDead() const;
		f32 GetAttackRange() const;
		f64 GetAttackRangeF64() const;
		glm::vec3 GetPosition() const;
		f32 GetHealth() const;
		f32 GetMaxHealth() const;
		const char* GetName() const;
		const char* GetTypeName() const;
		GameObjectType GetType() const;
		const SpellBook* GetSpellBook() const;
		const char* GetChampionName() const;
		int* GetChampionLevel() const;
	};

	struct ObjectManager
	{
		union
		{
			DEFINE_MEMBER_N(GameObject** begin,	18);
			DEFINE_MEMBER_N(GameObject** end,	22);
		};
	};

	bool IsValidGameObject(const GameObject* object);
	bool IsValidSpell(const Spell* spell);
}
