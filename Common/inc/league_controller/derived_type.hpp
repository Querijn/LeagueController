#pragma once

#include <type_traits>

// This define defines TemplateTypeName and enables it if it is a base of ParentName.
#define DerivedType(TemplateTypeName, ParentName) typename TemplateTypeName, typename std::enable_if<std::is_base_of<ParentName, TemplateTypeName>::value>::type* = nullptr
