#pragma once
#if LEACON_LUAJIT
#include <lua.hpp>
#else
extern "C"
{
#include <lua.h>
}
#endif

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>