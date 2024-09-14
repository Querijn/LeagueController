function GetSpellRange(spell, castTime, championLevel)
	
	if (spell == Keyboard.KeyW) then
		return math.min(castTime, 1) * 400 + 500;
	end

	return 0;
end