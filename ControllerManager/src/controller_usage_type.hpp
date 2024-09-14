#pragma once

#include <string_view>

namespace LeagueController
{
	enum class UsageType
	{
#define DEFINE_ENUM(a, b) a = b,
#define DEFINE_ENUM_ALT(a, b) a = b,
#define DEFINE_ENUM_INFO(a, b) a = b,
#include "controller_usage_type_definition.hpp"
#undef DEFINE_ENUM
#undef DEFINE_ENUM_ALT
#undef DEFINE_ENUM_INFO
	};

	const char* GetUsageTypeName(UsageType a_Input);
	UsageType* GetUsageTypeList();
	const char** GetUsageTypeNameList();
	UsageType GetUsageTypeByName(std::string_view a_Name);
	size_t GetUsageTypeCount();
	bool IsValidUsageType(UsageType a_UsageType);
}