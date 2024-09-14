function GetSpellRange(spell, castTime, championLevel)
	
	if (spell == Keyboard.KeyQ) then
		return math.min(castTime, 1.25) * 380 + 250;
	end

	return 0;
end