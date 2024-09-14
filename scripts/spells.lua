local SpellImpl = {};
KeyboardImpl = require("keyboard");

function SpellImpl.IsSpellBeingCast(localPlayer)
	if (localPlayer == nil) then
		--LogWarning("IsSpellBeingCast failed: unable to determine local player");
		ReportString("IsSpellBeingCast/reason", "No: Missing player");
		return false;
	end
	if (localPlayer.spellBook == nil) then
		LogWarning("IsSpellBeingCast failed: unable to determine local player's spell book");
		ReportString("IsSpellBeingCast/reason", "No: Missing spellbook");
		return false;
	end

	if (localPlayer.spellBook.activeCast and localPlayer.spellBook.activeCast.name ~= nil) then
		ReportString("IsSpellBeingCast/reason", "Yes: activeCast = " .. localPlayer.spellBook.activeCast.name);
		return true;
	end

	if (not KeyboardImpl.IsSpellBeingCast()) then
		ReportString("IsSpellBeingCast/reason", "No: No keyboard spell button is pressed");
		return false;
	end

	local spellKey = KeyboardImpl.GetSpellKey();
	local spell = localPlayer.spellBook:GetSpellByKey(spellKey);
	if (spell.isValid == false) then
		ReportString("IsSpellBeingCast/reason", "No: Spell is not valid");
		LogWarning("IsSpellBeingCast failed: unable to determine local player's spell (".. tostring(spellKey) ..")");
		return false;
	end

	if (spell.isCastable == false) then
		ReportString("IsSpellBeingCast/reason", "No: Spell is not castable");
		return false;
	end

	local isCast = spell.isOnCooldown == false;
	local isCastString = "No"; if (isCast) then isCastString = "Yes"; end
	ReportString("IsSpellBeingCast/reason", isCastString .. ": Spell cooldown");
	return isCast;
end

return SpellImpl;