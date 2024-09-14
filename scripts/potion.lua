local Potion = {};

HEALTH_POTION_ITEM_ID = "2003"; -- Health Potion
CORRUPTING_POTION_ITEM_ID = "2033"; -- Corrupting Potion
BISCUIT_ITEM_ID = "2010"; -- Total Biscuit of Everlasting Will
REFILLING_POTION_ITEM_ID = "2031"; -- Refillable Potion
ELIXIR_OF_IRON_ITEM_ID = "2138"; -- Elixir of Iron

function Potion.IsPotion(itemId)
	return  itemId == HEALTH_POTION_ITEM_ID or
			itemId == CORRUPTING_POTION_ITEM_ID or 
			itemId == BISCUIT_ITEM_ID or 
			itemId == REFILLING_POTION_ITEM_ID or 
			itemId == ELIXIR_OF_IRON_ITEM_ID;
end

function Potion.UseBestPotion(localPlayer)
	if (localPlayer == nil) then
		Log("Pressed UseBestPotion button, but localPlayer is nil");
		return
	end

	local healthNeeded = localPlayer.maxHealth - localPlayer.health;
	if (healthNeeded <= 0.1) then
		Log("Pressed UseBestPotion button, but we don't need any health.");
		return -- TODO: Mana check
	end

	Log("Pressed UseBestPotion button, we need " .. healthNeeded .. " health.");
	local healthRestoredDist = 10000;
	local bestItemId = nil;

	local itemToConsider = HEALTH_POTION_ITEM_ID;
	local itemName = "Health Potion";
	local healthRestored = 120;
	if (HasItem(itemToConsider)) then
		local potentialHealthRestoredDist = math.abs(healthNeeded - healthRestored);
		Log("- " .. itemName .. ", health restored = " .. healthRestored .. ", dist = " .. potentialHealthRestoredDist);
		if (potentialHealthRestoredDist < healthRestoredDist) then
			bestItemId = itemToConsider;
			healthRestoredDist = potentialHealthRestoredDist;
		end
	end

	itemToConsider = CORRUPTING_POTION_ITEM_ID;
	itemName = "Corrupting Potion";
	healthRestored = 100;
	if (HasItem(itemToConsider)) then
		local potentialHealthRestoredDist = math.abs(healthNeeded - healthRestored);
		Log("- " .. itemName .. ", health restored = " .. healthRestored .. ", dist = " .. potentialHealthRestoredDist);
		if (potentialHealthRestoredDist < healthRestoredDist) then
			bestItemId = itemToConsider;
			healthRestoredDist = potentialHealthRestoredDist;
		end
	end

	itemToConsider = BISCUIT_ITEM_ID;
	itemName = "Biscuit";
	healthRestored = 0.08 * healthNeeded;
	if (HasItem(itemToConsider)) then
		local potentialHealthRestoredDist = math.abs(healthNeeded - healthRestored);
		Log("- " .. itemName .. ", health restored = " .. healthRestored .. ", dist = " .. potentialHealthRestoredDist);
		if (potentialHealthRestoredDist < healthRestoredDist) then
			bestItemId = itemToConsider;
			healthRestoredDist = potentialHealthRestoredDist;
		end
	end

	itemToConsider = REFILLING_POTION_ITEM_ID;
	itemName = "Refilling Potion";
	healthRestored = 100;
	if (HasItem(itemToConsider)) then
		local potentialHealthRestoredDist = math.abs(healthNeeded - healthRestored);
		Log("- " .. itemName .. ", health restored = " .. healthRestored .. ", dist = " .. potentialHealthRestoredDist);
		if (potentialHealthRestoredDist < healthRestoredDist) then
			bestItemId = itemToConsider;
			healthRestoredDist = potentialHealthRestoredDist;
		end
	end

	itemToConsider = ELIXIR_OF_IRON_ITEM_ID;
	itemName = "Elixir of Iron";
	healthRestored = 300;
	if (HasItem(itemToConsider)) then
		local potentialHealthRestoredDist = math.abs(healthNeeded - healthRestored);
		Log("- " .. itemName .. ", health restored = " .. healthRestored .. ", dist = " .. potentialHealthRestoredDist);
		if (potentialHealthRestoredDist < healthRestoredDist) then
			bestItemId = itemToConsider;
			healthRestoredDist = potentialHealthRestoredDist;
		end
	end

	if (bestItemId == nil) then
		Log("No potion found to use.");
		return;
	end

	local spellName = GetSpellNameByItemId(bestItemId);
	if (spellName == nil or spellName == "") then
		Log("No spell found for item " .. bestItemId .. ".");
		return;
	end

	Log("Using item " .. bestItemId .. " (" .. spellName .. ") to restore health.");
	CastSpellByName(spellName);
end

return Potion;