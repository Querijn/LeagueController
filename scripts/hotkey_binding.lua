local HotkeyBinding = {};

KeyboardImpl = require("keyboard");
Fountain = require("fountain");

IS_SELECTING_SPELL_BUTTON = false;
CURRENTLY_BINDING_SPELL = nil;
CURRENTLY_BINDING_SPELL_INDEX = nil;
SPELL_CHOOSING_UI = LoadFullscreenUI("data/ui/spells.ui");
FOUNTAIN_RADIUS = 1000;

function HotkeyBinding.GetCurrentlyBindingSpell()
	return CURRENTLY_BINDING_SPELL;
end

function HotkeyBinding.MarkDone()
	CURRENTLY_BINDING_SPELL = nil;
end

function HotkeyBinding.Update(localPlayer)

	-- First check if we're on the fountain
	if (Fountain.IsOnFountain(localPlayer) == false) then
		-- Log("Not on fountain");
		if SPELL_CHOOSING_UI ~= nil then
			SPELL_CHOOSING_UI.visible = false;
		end
		return false;
	end

	-- Check if we have any unbound spells
	local unboundSpells = KeyboardImpl.GetUnboundSpells();
	if (#unboundSpells == 0) then
		-- Log("No unbound spells");
		if SPELL_CHOOSING_UI ~= nil then
			SPELL_CHOOSING_UI.visible = false;
		end
		return false;
	end

	-- Ensure UI is loaded
	if (SPELL_CHOOSING_UI == nil or SPELL_CHOOSING_UI.loaded == false) then
		if SPELL_CHOOSING_UI == nil then
			Break("UI was not found!");
		end
		Log("Waiting for spell ui to load, loaded = " .. tostring(SPELL_CHOOSING_UI.loaded));
		return false;
	end

	if (CURRENTLY_BINDING_SPELL == nil) then
		CURRENTLY_BINDING_SPELL = unboundSpells[1];
		Log("Setting current spell to bind = " .. CURRENTLY_BINDING_SPELL.itemName .. ".")

		if (CURRENTLY_BINDING_SPELL == nil) then
			Log("Waiting for CURRENTLY_BINDING_SPELL.")
			return false;
		end

		local text = SPELL_CHOOSING_UI:GetContainer("SpellContainer"):GetChildByName("Title").asText;
		text.content = "Press a key to bind " .. CURRENTLY_BINDING_SPELL.itemName;
		Log("Press a key to bind " .. CURRENTLY_BINDING_SPELL.itemName);
	end

	if (SPELL_CHOOSING_UI.visible == false) then
		SPELL_CHOOSING_UI.visible = true;
	end

	return true;
end

return HotkeyBinding;