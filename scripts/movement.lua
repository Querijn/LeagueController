local Movement = {};

LAST_DIR = nil
IS_MOVING = false
WAS_CASTING = false
FRAME_INDEX = 0

function DebugRaycast(localPos, leftDir, color)
	local isCast, pos = CastPassableRay(localPos, leftDir, 10000.0);
	-- DrawLine3D(localPos, pos, color, 1.0, 1.0);
	return isCast, pos;
end

function Movement.Update(localPlayer)
	if (localPlayer == nil) then
		return false;
	end

	local hasLeftDir, leftDir = GetLeftAnalogDir(true, true);

	-- Log("WAS_CASTING = " .. tostring(WAS_CASTING) .. " hasLeftDir = " .. tostring(hasLeftDir));

	-- First, check if we were casting. If we are casting, hold that information.
	if (localPlayer.spellBook.activeCast and localPlayer.spellBook.activeCast.name ~= nil) then
		if WAS_CASTING == false then
			LogDebug("Preparing for movement command reset (" .. localPlayer.spellBook.activeCast.name .. ")");
		end
		WAS_CASTING = true;

	-- If we were casting, and we're still trying to move, reset cache so we input a new move command
	elseif WAS_CASTING and hasLeftDir then
		LAST_DIR = nil;
		WAS_CASTING = false;
		LogDebug("Resetting movement command");
	end

	-- Stop moving if we aren't inputting.
	if hasLeftDir == false then
		if IS_MOVING then
			IS_MOVING = localPlayer.StopMoving() == false;
			LAST_DIR = nil;
		end
		return false;
	end

	-- This prevents updates from becoming too frequent
	local angleDiff = 0.3;
	if LAST_DIR ~= nil then
		angleDiff = math.abs(Dot(leftDir, LAST_DIR) - 1.0);
	end

	if (angleDiff < 0.03) then
		return true;
	end

	local rightAngle = Vec3.new(leftDir.z, leftDir.y, -leftDir.x);

	local localPos = localPlayer.position + (leftDir * 75.0);
	local isCast, targetLocation = DebugRaycast(localPos, leftDir,    0xFFFF0000);

	local localPos2 = localPos + rightAngle * 75.0;
	local isCast2, targetLocation2 = DebugRaycast(localPos2, leftDir, 0xFF00FF00);

	local localPos3 = localPos - rightAngle * 75.0;
	local isCast3, targetLocation3 = DebugRaycast(localPos3, leftDir, 0xFF0000FF);

	if isCast == false then
		if isCast2 then
			isCast = isCast2;
			targetLocation = targetLocation2;
		elseif isCast3 then
			isCast = isCast3;
			targetLocation = targetLocation3;
		else
			return IS_MOVING;
		end
	end

	local origDelta = targetLocation - localPlayer.position;
	if isCast2 then
		local delta = targetLocation2 - targetLocation;
		if delta.length2 > origDelta.length2 then
			isCast = isCast2;
			targetLocation = targetLocation2;
			origDelta = targetLocation - localPlayer.position;
		end
	end
	if isCast3 then
		local delta = targetLocation3 - targetLocation;
		if delta.length2 > origDelta.length2 then
			isCast = isCast3;
			targetLocation = targetLocation3;
			origDelta = targetLocation - localPlayer.position;
		end
	end

	if isCast == false then
		return IS_MOVING;
	end
	-- Log("Moving to " .. string.format("%.3f", localPos.x) .. ", " .. string.format("%.3f", localPos.z));

	LAST_DIR = leftDir;
	IS_MOVING = localPlayer.MoveTo(targetLocation);
	return true;
end

return Movement;