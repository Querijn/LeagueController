#include "league_internals/game_object.hpp"
#include "league_internals/offsets.hpp"
#include <league_controller/settings.hpp>

#if LEAGUEHACKS_DEBUG_LOG
#include <game_overlay/overlay.hpp>
#include <game_overlay/log.hpp>
#endif

#include <windows.h>
#include <algorithm>

namespace LeagueController
{
#if LEAGUEHACKS_DEBUG_LOG
	extern GameOverlay::LogCategory g_offsetLog;
#else
	extern int g_offsetLog;
#define GameOverlay_AssertF
#endif

	u64 GameObject::healthOffset = 0;
	u64 GameObject::positionOffset = 0;
	u64 GameObject::attackRangeOffset = 0;
	u64 GameObject::spellBookOffset = 0;
	u64 GameObject::itemSlotsOffset = 0;
	u64 GameObject::championNameOffset = 0;
	u64 GameObject::championLevelOffset = 0;

	// TODO:
	u64 Spell::slotOffset = 0x4;
	u64 Spell::levelOffset = 0x1C;
	u64 Spell::cooldownOffset = 0x24;
	u64 Spell::spellInfoOffset = 0; // Already automated

	u64 SpellInfo::nameOffset = 0x18;
	u64 SpellInfo::notCastableOffset = 0x11;
	
	u64 SpellBook::activeCastOffset = 0x20;
	u64 SpellBook::activeCastSlotOffset = 0xC;
	u64 SpellBook::spellsOffset = 0x4C8;

	template<typename T>
	const T* Deoffset(const void* base, u64 offset)
	{
		return reinterpret_cast<const T*>(reinterpret_cast<const u8*>(base) + offset);
	}

	bool Spell::IsOnCooldown() const
	{
		Leacon_Profiler;
		return GetGameTime() < *Deoffset<f32>(this, cooldownOffset);
	}

	bool Spell::IsCastable() const
	{
		Leacon_Profiler;
		if (IsValidSpell(this) == false)
			return false;

		if (GetSpellInfo()->IsCastable() == false || GetLevel() <= 0)
			return false;

		return IsOnCooldown() == false;
	}
	
	const char* Spell::GetName() const
	{
		Leacon_Profiler;
		return IsValidSpell(this) ? GetSpellInfo()->GetName().c_str() : nullptr;
	}

	int Spell::GetSlot() const						{ return this ? *Deoffset<int>		 (this, slotOffset)		 : -1; }
	int Spell::GetLevel() const						{ return this ? *Deoffset<int>		 (this, levelOffset)	 : -1; }
	f32 Spell::GetCooldown() const					{ return this ? *Deoffset<f32>		 (this, cooldownOffset)	 : 0; }
	const SpellInfo* Spell::GetSpellInfo() const	{ return this ? *Deoffset<SpellInfo*>(this, spellInfoOffset) : nullptr; }

	const Spell* SpellBook::GetSpell(int index) const
	{
		Leacon_Profiler;
		if (index < 0 || index >= SpellSlotID::Max)
			return nullptr;

		Spell* const * const e = Deoffset<Spell*>(this, spellsOffset);
		return e[index];
	}

	const Spell* SpellBook::GetActiveCast() const
	{
		const void* activeCastSpellInstance = this ? *Deoffset<void*>(this, activeCastOffset) : nullptr;
		if (activeCastSpellInstance == nullptr)
			return nullptr;

		int slotIndex = *Deoffset<int>(activeCastSpellInstance, activeCastSlotOffset);
		return GetSpell(slotIndex);
	}

	const StdString& SpellInfo::GetName() const
	{
		static StdString garbage;
		return this ? *Deoffset<StdString>(this, nameOffset) : garbage;
	}

	bool SpellInfo::IsCastable() const
	{
		bool notCastable = this ? *Deoffset<bool>(this, notCastableOffset) : true;
		return !notCastable;
	}
	
	bool GameObject::ShouldBeValid()
	{
		Leacon_Profiler; // TODO: Update this
		return positionOffset > 0 && 
			attackRangeOffset > 0 &&
			itemSlotsOffset > 0;
	}

	f32 GameObject::GetAttackRange() const
	{
		Leacon_Profiler;
		if (attackRangeOffset <= 0)
			return 0.0f;
		return *(f32*)(((u8*)this) + attackRangeOffset);
	}

	f64 GameObject::GetAttackRangeF64() const
	{
		Leacon_Profiler;
		return GetAttackRange();
	}

	const char* GameObject::GetName() const
	{
		return name.c_str();
	}

	const char* GameObject::GetChampionName() const
	{
		Leacon_Profiler;
		if (GameObject::championNameOffset == 0)
			return nullptr;

		return (const char*)((MemoryHelper::Address)this + GameObject::championNameOffset);
	}

	int* GameObject::GetChampionLevel() const
	{
		Leacon_Profiler;
		if (GameObject::championLevelOffset == 0)
			return nullptr;

		return (int*)((MemoryHelper::Address)this + GameObject::championLevelOffset);
	}

	bool IsOnHeroList(const GameObject& object)
	{
		Leacon_Profiler;
		Manager* heroManager = GetHeroManager();
		if (heroManager == nullptr)
			return false;
		
		GameObject** begin = heroManager->items.begin;
		GameObject** end = &heroManager->items.begin[heroManager->items.count];
		for (GameObject** i = begin; i != end; i++)
			if (*i == &object)
				return true;

		return false;
	}

	GameObjectType GameObject::GetType() const
	{
		Leacon_Profiler;
		if (IsOnHeroList(*this))
			return GameObjectType::Champion;

		if (name.length != 0)
		{
			std::string n = name.c_str();
			if (n.find("Minion", 0) != std::string::npos)
				return GameObjectType::Minion;
			if (n.find("Turret", 0) != std::string::npos)
				return GameObjectType::Turret;
			if (n.find("Plant", 0) != std::string::npos)
				return GameObjectType::Plant;
		}

		if (team == 300) // Gaia
			return GameObjectType::Monster;

		return GameObjectType::Unknown;
	}

	const SpellBook* GameObject::GetSpellBook() const
	{
		Leacon_Profiler;
		if (GameObject::spellBookOffset == 0)
			return nullptr;

		return (const SpellBook*)((MemoryHelper::Address)this + GameObject::spellBookOffset);
	}

	const char* GameObject::GetTypeName() const
	{
		Leacon_Profiler;
		switch (GetType())
		{
		// case GameObjectType::Unknown: return "Unknown";
		case GameObjectType::Champion: return "Champion";
		case GameObjectType::Minion: return "Minion";
		case GameObjectType::Turret: return "Turret";
		case GameObjectType::Monster: return "Monster";
		case GameObjectType::Plant: return "Plant";
		case GameObjectType::Inhibitor: return "Inhibitor";
		case GameObjectType::Nexus: return "Nexus";
		}
		return GetName();
	}

	glm::vec3 GameObject::GetPosition() const
	{
		Leacon_Profiler;
		if (positionOffset <= 0)
			return glm::vec3();
		return *(glm::vec3*)(((u8*)this) + positionOffset);
	}

	f32 GameObject::GetHealth() const
	{
		Leacon_Profiler;
		if (healthOffset <= 0)
			return 0.0f;
		return *(f32*)(((u8*)this) + healthOffset);
	}

	f32 GameObject::GetMaxHealth() const
	{
		Leacon_Profiler;
		if (healthOffset <= 0)
			return 0.0f;
		return *(f32*)(((u8*)this) + healthOffset + 0x10);
	}

	int GameObject::GetTeam() const
	{
		return team;
	}

	bool GameObject::IsDead() const
	{
		f32 health = GetHealth();
		bool alive = isAlive; // REDACTED for security reasons
		return health <= 0.1f || !alive;
	}

	bool IsValidGameObject(const GameObject* object)
	{
		Leacon_Profiler;
		if (object == nullptr)
			return false;

		auto manager = GetAttackableManager();
		auto localPlayer = GetLocalPlayer();
		if (manager == nullptr || localPlayer == nullptr)
			return false;

		GameObject** begin = manager->items.begin;
		GameObject** end = &manager->items.begin[manager->items.count];
		for (auto i = begin; i != end; i++)
		{
			auto gameObject = *i;
			if (gameObject != object)
				continue;

			// Object was found, check if we can attack it
			if (gameObject->isTargetable == false || gameObject->IsDead())
				return false;

			return true;
		}

		return false;
	}
	
	bool IsValidSpell(const Spell* spell)
	{
		Leacon_Profiler;

		if (Spell::slotOffset == 0) return false;
		if (Spell::levelOffset == 0) return false;
		if (Spell::cooldownOffset == 0) return false;
		if (Spell::spellInfoOffset == 0) return false;

		if (SpellInfo::nameOffset == 0) return false;
		if (SpellInfo::notCastableOffset == 0) return false;

		if (SpellBook::activeCastOffset == 0) return false;
		if (SpellBook::spellsOffset == 0) return false;

		if (spell == nullptr)
			return false;

		auto spellInfo = spell->GetSpellInfo();
		if (spellInfo == nullptr)
			return false;

		StdString name = spell->GetSpellInfo()->GetName();
		if (name.c_str() == nullptr || name.length == 0)
			return false;
		
		if (name == "BaseSpell")
			return false;
		return true;
	}
	
	bool StdString::operator==(const char* other) const
	{
		return strcmp(c_str(), other) == 0;
	}

	bool StdString::operator!=(const char* other) const
	{
		return operator==(other) == false;
	}
	
	bool StdString::operator==(const StdString& other) const
	{
		return strcmp(c_str(), other.c_str()) == 0;
	}

	bool StdString::operator!=(const StdString& other) const
	{
		return operator==(other) == false;
	}
}