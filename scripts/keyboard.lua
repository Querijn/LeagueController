local KeyboardImpl = {};

Potion = require("potion");

function KeyboardImpl.HasCustomMapping(controllerInput)
	return false;
end

function KeyboardImpl.GetUnboundSpells()
	local itemSpells = GetItemSpells();
	local unboundSpells = {};
	for i = 1, #itemSpells do
		local spell = itemSpells[i];
		if (spell == nil or spell.itemName == nil) then
			ReportString("Item/Spell" .. i, "nil");
			goto continue;
		end

		ReportString("Item/Spell "..i.."/ItemId", spell.itemId);
		ReportString("Item/Spell "..i.."/ItemName", spell.itemName);
		ReportString("Item/Spell "..i.."/Name", spell.name);

		-- Ignore potions
		if Potion.IsPotion(spell.itemId) then
			ReportString("Item/Spell "..i.."/Bound", "as potion");
			goto continue;
		end

		if IsSpellBound(spell.name) then
			ReportString("Item/Spell "..i.."/Bound", "yes");
			goto continue;
		end

		ReportString("Item/Spell "..i.."/Bound", "no");
		LogDebug("Unbound spell = " .. spell.itemName);
		unboundSpells[#unboundSpells + 1] = spell;
		::continue::
	end

	return unboundSpells;
end

function ProcessCustomMapBinding(localPlayer)
	local spell = HotkeyBinding.GetCurrentlyBindingSpell();
	if (spell == nil) then
		LogWarning("Unexpected error: no spell going through HotkeyBinding?");
		return;
	end

	local HotkeyBinding = require("hotkey_binding");
	-- Get first key that is pressed out the following list
	local unmappedKeys = GetUnboundKeys();
	for i = 1, #unmappedKeys do
		local controllerInput = unmappedKeys[i];
		if (IsButtonDown(controllerInput) == true) then
			AddSpellMapping(controllerInput, spell.name);
			Log("Bound " .. GetControllerInputName(controllerInput) .. " to " .. spell.name .. " (" .. spell.itemName .. ").");
			HotkeyBinding.MarkDone();
			break;
		end
	end
end
-- Champion spell key to keyboard key (TODO: Might be buggy to use? What if multiple are pressed?)
function KeyboardImpl.GetChampionSpellKey()
	if IsCustomActionPressed("Spell1") then -- TODO: Replace keys with game config values
		return Keyboard.KeyQ;
	end
	if IsCustomActionPressed("Spell2") then
		return Keyboard.KeyW;
	end
	if IsCustomActionPressed("Spell3") then
		return Keyboard.KeyE;
	end
	if IsCustomActionPressed("Spell4") then
		return Keyboard.KeyR;
	end
	return Keyboard.KeyNone;
end

-- Keyboard implementation for whether or not the champ spell is being cast.
function KeyboardImpl.IsChampionSpellBeingCast()
	return KeyboardImpl.GetChampionSpellKey() ~= Keyboard.KeyNone;
end

-- Is any spell being cast? (Champion, summoner or item)
function KeyboardImpl.IsSpellBeingCast()
	return KeyboardImpl.IsChampionSpellBeingCast();
end

-- Get spell key being cast
function KeyboardImpl.GetSpellKey()
	return KeyboardImpl.GetChampionSpellKey();
end

-- Get time at which the champion spell being cast was started
function KeyboardImpl.GetSpellCastTime()
	return GetControllerInputPressTime(KeyboardImpl.GetChampionSpellKey());
end

function KeyboardImpl.Init()
end

-- The update function, simply maps all keys to the default controller input
function KeyboardImpl.Update(localPlayer, isBindingInProgress)
	if (isBindingInProgress) then
		ProcessCustomMapBinding(localPlayer);
		return
	end

	KeyboardImpl.GetUnboundSpells();
end

return KeyboardImpl;