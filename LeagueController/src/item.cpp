#include "item.hpp"

#include <league_controller/json.hpp>
#include <league_controller/profiler.hpp>

#include <league_lib/bin/bin.hpp>

#include <game_overlay/log.hpp>

#include <fstream>
#include <unordered_map>

namespace LeagueController
{
	GameOverlay::LogCategory g_itemLog("Items");
	static bool g_areItemsLoaded = false;
	static bool g_areItemsLoading = false;
	static LeagueLib::Bin g_leagueBin;
	static std::unordered_map<std::string, std::string> g_itemSpellNames;
	static std::unordered_map<std::string, std::string> g_itemIdToSpellName;
	static std::unordered_map<std::string, std::string> g_spellNameToItemId;

	void LoadItems(std::string_view workingDirectory)
	{
		using namespace LeagueLib;
		if (g_areItemsLoaded || g_areItemsLoading)
			return;
		g_areItemsLoading = true;

		Leacon_Profiler;
		std::string json;
		{
			std::ifstream file(std::string(workingDirectory) + "/Data/items.json");
			GameOverlay_AssertF(g_itemLog, file.is_open(), "Could not open items.json");

			std::stringstream fileStream;
			fileStream << file.rdbuf();
			json = fileStream.str();
		}

		nlohmann::json itemsJson = nlohmann::json::parse(json);
		std::vector<std::tuple<std::string, std::string>> itemIdAndName;
		for (auto i = itemsJson["data"].begin(); i != itemsJson["data"].end(); i++)
		{
			nlohmann::json& itemJson = i.value();
			std::string itemId = i.key();
			std::string itemName = itemJson["name"].get<std::string>();
			itemIdAndName.push_back({ itemId, itemName });
		}

		g_leagueBin.Load("global/items/items.bin", [itemIdAndName](LeagueLib::Bin& bin)
		{
			Leacon_Profiler;
			GameOverlay_AssertF(g_itemLog, bin.GetLoadState() == Spek::File::LoadState::Loaded, "Could not open items.bin");	
			if (bin.GetLoadState() == Spek::File::LoadState::Loaded)
			{
				char rootName[1024];
				for (const auto& [itemId, itemName] : itemIdAndName)
				{
					snprintf(rootName, 1024, "Items/%s", itemId.c_str());
					BinVarRef root = bin[rootName];
					BinVarRef spellNameRef = root["spellName"];
					if (!spellNameRef.Is<std::string>())
						continue;

					const std::string& spellName = *spellNameRef.As<std::string>();
					g_itemSpellNames[spellName] = itemName;
					g_itemIdToSpellName[itemId] = spellName;
					g_spellNameToItemId[spellName] = itemId;
				}

				g_areItemsLoaded = true;
			}

			g_areItemsLoading = false;
		});
	}

	bool GetItemNameBySpell(const char* spellName, std::string& name)
	{
		auto index = g_itemSpellNames.find(spellName);
		if (index == g_itemSpellNames.end())
			return false;

		name = index->second;
		return true;
	}

	bool GetSpellNameByItemId(const char* itemId, std::string& name)
	{
		auto index = g_itemIdToSpellName.find(itemId);
		if (index == g_itemIdToSpellName.end())
			return false;

		name = index->second;
		return true;
	}

	bool GetItemIdBySpellName(const char* spellName, std::string& itemId)
	{
		auto index = g_spellNameToItemId.find(spellName);
		if (index == g_spellNameToItemId.end())
			return false;

		itemId = index->second;
		return true;
	}

	void DestroyItems()
	{
		// g_leagueBin.Clear(); // TODO
		g_areItemsLoaded = false;
	}
}