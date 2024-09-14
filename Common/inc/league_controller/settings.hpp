#pragma once

#include <league_controller/config.hpp>
#include <league_controller/types.hpp>
#include <string>


#ifndef LEAGUEHACKS_DEBUG_LOG
#define LEAGUEHACKS_DEBUG_LOG true
#endif

#if LEACON_FINAL
#include <string_view>

namespace Hash
{
	namespace __fnv_internal
	{
		consteval char ToLowerCase(char c)
		{
			return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
		}
	}

	consteval u32 FNV1Hash(std::string_view str)
	{
		u32 hash = 0x811c9dc5;
		for (char c : str)
			hash = ((hash ^ __fnv_internal::ToLowerCase(c)) * 0x01000193) % 0x100000000;

		return (u32)hash;
	}
}

#endif

#include "json.hpp"

namespace LeagueController
{
	using Offset = u64;
	
	struct AddressData
	{
#define ADDRESS_DATA_FIELDS(Func) \
		Func(gameTimeAddress) \
		Func(heroManagerAddress) \
		Func(localPlayerAddress) \
		Func(viewMatrixAddress) \
		Func(projMatrixAddress) \
		Func(attackableManagerAddress) \
		Func(underMouseObjectAddress) \
		Func(guiMenuAddress) \
		\
		Func(hudManagerAddress) \
		Func(cursorTargetPosRawAddress) \
		Func(cursorTargetLogicAddress) \
		\
		Func(healthOffset) \
		Func(positionOffset) \
		Func(spellBookOffset) \
		Func(attackRangeOffset) \
		Func(championNameOffset) \
		Func(championLevelOffset) \
		\
		Func(spellSlotOffset) \
		Func(spellLevelOffset) \
		Func(spellCooldownOffset) \
		Func(spellInfoOffset) \
		\
		Func(spellInfoNameOffset) \
		Func(spellInfoNotCastableOffset) \
		\
		Func(spellBookActiveCastOffset) \
		Func(spellBookSpellsOffset) \

#define ADDRESS_DATA_DEFINE_FIELD(Name) Offset Name = 0;
		ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_FIELD)
#undef  ADDRESS_DATA_DEFINE_FIELD

#if !LEACON_FINAL
			using AddressName = const char*;
#else
			using AddressName = u32;
#endif
		
		template<typename Serialiser>
		void ForEachNamedAddress(Serialiser serialiser)
		{
#if !LEACON_FINAL
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) serialiser(#Name, Name);
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
#else
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) serialiser(Hash::FNV1Hash(#Name), Name);
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
#endif
		}
		
		template<typename Serialiser>
		AddressName FindFirstAddress(Serialiser serialiser)
		{
#if !LEACON_FINAL
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) if (serialiser(Name)) return #Name;
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
#else
			
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) static constexpr u32 nameHash##Name = Hash::FNV1Hash(#Name); if (serialiser(Name)) return nameHash##Name;
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
#endif
			return 0;
		}

		template<typename Serialiser>
		void ForEachAddress(Serialiser serialiser)
		{
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) serialiser(Name);
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
		}

		template<typename Serialiser>
		void ForEachAddress(Serialiser serialiser) const
		{
	#define ADDRESS_DATA_DEFINE_SERIALISER(Name) serialiser(Name);
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_SERIALISER)
	#undef	ADDRESS_DATA_DEFINE_SERIALISER
		}
		
		bool operator ==(const AddressData& other) const
		{
	#define ADDRESS_DATA_DEFINE_COMPARE(Name) if (!(Name == other.Name)) return false;
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_COMPARE)
	#undef	ADDRESS_DATA_DEFINE_COMPARE
			return true;
		}

#if !LEACON_FINAL
		const Offset* GetByName(const char* name) const
		{
	#define ADDRESS_DATA_DEFINE_GETTER(Name) if (strcmp(#Name, name) == 0) return &Name;
			ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_GETTER)
	#undef ADDRESS_DATA_DEFINE_COMPARE
			return nullptr;
		}
#else
		const Offset* GetByName(u32 name) const
		{
			switch (name)
			{
#define ADDRESS_DATA_DEFINE_GETTER(Name) case Hash::FNV1Hash(#Name): return &Name;
				ADDRESS_DATA_FIELDS(ADDRESS_DATA_DEFINE_GETTER)
#undef ADDRESS_DATA_DEFINE_COMPARE
			}
			
			return nullptr;
		}
#endif

		bool operator !=(const AddressData& other) const
		{
			return operator==(other) == false;
		}
	};

	struct ControllerSettings
	{
	#define ControllerSettings_BindingFields(FuncBinding) \
		FuncBinding(std::string,	bindingL1,			"Spell1",				L1) \
		FuncBinding(std::string,	bindingL2,			"Spell2, RightClick",	L2) \
		FuncBinding(std::string,	bindingR1,			"Spell3",				R1) \
		FuncBinding(std::string,	bindingR2,			"Spell4, LeftClick",	R2) \
		\
		FuncBinding(std::string,	bindingL3,			"",						L3) \
		FuncBinding(std::string,	bindingR3,			"Summoner1",			R3) \
		\
		FuncBinding(std::string,	bindingSelect,		"Shop",					Select) \
		FuncBinding(std::string,	bindingStart,		"Escape",				Start) \
		\
		FuncBinding(std::string,	bindingDPadLeft,	"",						DPadLeft) \
		FuncBinding(std::string,	bindingDPadUp,		"",						DPadUp) \
		FuncBinding(std::string,	bindingDPadRight,	"Potion",				DPadRight) \
		FuncBinding(std::string,	bindingDPadDown,	"Recall",				DPadDown) \
		\
		FuncBinding(std::string,	bindingSquare,		"",						Square) \
		FuncBinding(std::string,	bindingTriangle,	"Ctrl",					Triangle) \
		FuncBinding(std::string,	bindingCircle,		"",						Circle) \
		FuncBinding(std::string,	bindingCross,		"Summoner2",			Cross) \
			
	#define ControllerSettings_Fields(Func) \
		Func(f32, deadZone, 0.2f) \
		Func(bool, useAimAssist, true) \
		ControllerSettings_BindingFields(Func) \

	#define FieldDefine(Type, Name, DefaultValue) Type Name = DefaultValue;
		ControllerSettings_Fields(FieldDefine)
	#undef  FieldDefine
	};

	struct KeyboardSettings
	{
	#define KeyboardSettings_Fields(Func) \
		Func(std::string, bindingQ, "Q") \
		Func(std::string, bindingW, "W") \
		Func(std::string, bindingE, "E") \
		Func(std::string, bindingR, "R") \
		Func(std::string, bindingSummoner1, "D") \
		Func(std::string, bindingSummoner2, "F") \
		Func(std::string, bindingItem1, "1") \
		Func(std::string, bindingItem2, "2") \
		Func(std::string, bindingItem3, "3") \
		Func(std::string, bindingItem4, "5") \
		Func(std::string, bindingItem5, "6") \
		Func(std::string, bindingItem6, "7") \
		Func(std::string, bindingRecall, "B") \
		Func(std::string, bindingShop, "P") \
		Func(std::string, bindingCenterCamera, "Y") \

	#define FieldDefine(Type, Name, DefaultValue) Type Name = DefaultValue;
		KeyboardSettings_Fields(FieldDefine)
	#undef  FieldDefine
	};

	struct InternalSettings
	{
		static constexpr u32 currentVersion = 4;

	#define InternalSettings_Fields(Func, FuncNoValue) \
		Func(u32, settingsVersion, currentVersion) \
		Func(u32, majorVersion, LeagueController::MajorVersion) \
		Func(u32, minorVersion, LeagueController::MinorVersion) \
		Func(bool, patchVerified, false) \
		FuncNoValue(std::string, gameVersion) \
		FuncNoValue(std::string, hash) \

	#define FieldDefineNoVal(Type, Name) Type Name;
	#define FieldDefine(Type, Name, DefaultValue) Type Name = DefaultValue;
		InternalSettings_Fields(FieldDefine, FieldDefineNoVal)
	#undef  FieldDefine
	#undef  FieldDefineNoVal
	};

	struct Settings;
	struct SerialisedSettings
	{
		static SerialisedSettings Load(const char* fileName, const char* gameVersion);
		void Save(const char* fileName, nlohmann::json& json);

	#define SerialisedSettings_Fields(FuncNoValue) \
		FuncNoValue(InternalSettings, leagueController) \
		FuncNoValue(KeyboardSettings, keyboard) \
		FuncNoValue(ControllerSettings, controller) \

	#define FieldDefineNoVal(Type, Name) Type Name;
		SerialisedSettings_Fields(FieldDefineNoVal)
	#undef  FieldDefineNoVal

		bool ToSettings(Settings& outSettings, const char* gameVersionString) const;
		bool FromSettings(const Settings& settings, const char* gameVersionString);
	};

	struct Settings
	{
		std::string fileName;

		SerialisedSettings serialisedSettings;
		u32 gameVersionHash;
		u32 addressesHash;
		AddressData addresses;
		bool patchVerified = false;
	};
	
	bool AreSettingsLoaded();
	bool LoadSettings(std::string_view cwd, bool isApp);
	bool WriteSettings(std::string_view cwd);
	Settings& GetSettings();

#if _WIN32
	bool GetExecutableVersion(std::string& version);
	bool GetExecutableVersion(const char* filePath, std::string& result);
#endif
}
