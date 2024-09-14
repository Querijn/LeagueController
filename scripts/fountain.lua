local Fountain = {};

function Fountain.IsOnFountain(localPlayer)
	if (localPlayer == nil) then
		return true;
	end

	local playerPos = localPlayer.position;

	-- TODO: Add support for other maps

	if playerPos.x < 2000.0 then
		local fountain = Vec3.new(375, 182, 432);

		local dist = (playerPos - fountain).length2;
		return dist < FOUNTAIN_RADIUS * FOUNTAIN_RADIUS;
	end

	local fountain = Vec3.new(14284, 172, 14376);
	local dist = (playerPos - fountain).length2;
	return dist < FOUNTAIN_RADIUS * FOUNTAIN_RADIUS;
end

return Fountain;