#pragma once

#include <string_view>
#include <string>

namespace LeagueController
{
	void LoadItems(std::string_view workingDirectory);
	bool GetItemNameBySpell(const char* spellName, std::string& name);
	bool GetSpellNameByItemId(const char* itemId, std::string& name);
	bool GetItemIdBySpellName(const char* spellName, std::string& itemId);
	void DestroyItems();
}