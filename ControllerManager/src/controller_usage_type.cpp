#include "controller_usage_type.hpp"

namespace LeagueController
{
	const char* GetUsageTypeName(UsageType a_Input)
	{
		switch (a_Input)
		{
#define DEFINE_ENUM(a, b) case UsageType::a: return #a;
#define DEFINE_ENUM_ALT(a, b)
#define DEFINE_ENUM_INFO(a, b)
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		}

		return "Unknown";
	}

	UsageType* GetUsageTypeList()
	{
		static UsageType t_List[] =
		{
#define DEFINE_ENUM(a, b) UsageType::a,
#define DEFINE_ENUM_ALT(a, b)
#define DEFINE_ENUM_INFO(a, b)
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		};

		return t_List;
	}

	const char** GetUsageTypeNameList()
	{
		static const char* t_List[] =
		{
#define DEFINE_ENUM(a, b) #a,
#define DEFINE_ENUM_ALT(a, b)
#define DEFINE_ENUM_INFO(a, b)
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		};

		return t_List;
	}

	// TODO: Also make switch hash alternative?
	UsageType GetUsageTypeByName(std::string_view a_Name)
	{
#define DEFINE_ENUM(a, b) if (#a == a_Name) return UsageType::a;
#define DEFINE_ENUM_ALT(a, b)
#define DEFINE_ENUM_INFO(a, b)
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO

		return UsageType::Undefined;
	}

	size_t GetUsageTypeCount()
	{
		return (size_t)UsageType::Max;
	}

	bool IsValidUsageType(UsageType a_UsageType)
	{
		switch (a_UsageType)
		{
#define DEFINE_ENUM(a, b) case UsageType::a: return true;
#define DEFINE_ENUM_ALT(a, b)
#define DEFINE_ENUM_INFO(a, b) case UsageType::a: return false;
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
		}

		return false;
	}
}