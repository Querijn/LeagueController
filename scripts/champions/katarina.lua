function GetSpellRange(spell, castTime, championLevel)
	
	if (spell == Keyboard.KeyW) then
		return 125 + 65; -- Little hack to just use the range of auto-attacks
	end

	return 0;
end

function GetSpellLockOnTypeOverride(spell)
	if (spell == Keyboard.KeyQ) then
		return LockOnType.Hard;
	end

	return LockOnType.Soft;
end