local LockOn = {};

Targeting = require("targeting");

LOCK_ON_TARGET = nil;
LOCK_ON_DIR = nil;
LOCK_ON_LOCAL_START_POS = nil;
LOCK_ON_TARGET_START_POS = nil;
LOCK_ON_POS = nil;
SWAP_DELTA = nil;
IS_CASTING_SPELL = false;
CASTING_SPELL = nil;

-- Constants
MAX_DIST_LOCK_ON_SQR = 0.1 * 0.1;
TARGET_SUGGEST_AMOUNT = 0.7;
SELF_SUGGEST_AMOUNT = 0.7;

function LockOn.GetLockOnType()
	if IS_CASTING_SPELL then
		if CASTING_SPELL ~= nil then
			return GetSpellLockOnType(CASTING_SPELL);
		end
		return LockOnType.Soft;
	else
		return LockOnType.Hard;
	end
end

function LockOn.IsSoft()
	return LockOn.GetLockOnType() == LockOnType.Soft;
end

function LockOn.GetTypeName()
	if LockOn.IsOn() == false then
		return "None";
	end

	if LockOn.IsSoft() then
		return "Soft";
	else
		return "Hard";
	end
end

function LockOn.Force(target, analogDir)
	LOCK_ON_TARGET = target;
	LOCK_ON_DIR = analogDir;
end

function LockOn.Set(target, analogDir)
	if LOCK_ON_DIR == nil then
		LockOn.Force(target, analogDir);
		LogDebug("Setting " .. LockOn.GetTypeName() .. " lock-on" .. (target and " to " .. target.name or "") .. " with dir " .. tostring(analogDir));
	end

	return LockOn.IsOn();
end

function LockOn.Update(localPlayer, originalAttackPos)
	if LockOn.IsOn() == false then
		return;
	end

	SoftLockOnUpdate(localPlayer, originalAttackPos);
end

function CheckForBetterTarget(localPlayer, nearestObject, attackRange, origTarget)
	if LOCK_ON_TARGET ~= nearestObject and Targeting.IsValidTarget(localPlayer, nearestObject, nil, attackRange, true) then
		local nearestObjectPrio = Targeting.GetPriority(nearestObject);
		local currentPrio = Targeting.GetPriority(origTarget);
		if nearestObjectPrio > currentPrio then
			return nearestObject;
		end
	end

	return origTarget;
end

function SetSwapDelta(localPlayer, analogDir)
	local spellKey = KeyboardImpl.GetSpellKey();
	local castDuration = GetGameTime() - KeyboardImpl.GetSpellCastTime();
	local attackRange = GetSpellCastRange(spellKey, castDuration);
	if IsValidGameObject(LOCK_ON_TARGET) == false then
		return;
	end

	local attackPos = localPlayer.position + analogDir * attackRange;

	-- Cheat a bit, check if there's a better target at the actual aim location
	local nearestChampion = GetNearestChampion(attackPos);
	local bestTarget = CheckForBetterTarget(localPlayer, nearestChampion, attackRange, LOCK_ON_TARGET);
	local nearestObject = GetNearestObject(attackPos);
	bestTarget = CheckForBetterTarget(localPlayer, nearestObject, attackRange, bestTarget);

	if bestTarget ~= LOCK_ON_TARGET and bestTarget ~= nil then -- Should not be nil but just in case
		Log("SetSwapDelta: Swapped to " .. bestTarget.name .. " from " .. LOCK_ON_TARGET.name);
		LockOn.Force(bestTarget, analogDir);
		return;
	end

	-- If there's no better target, make sure our cursor stays at the same location by calculating the delta between the target and where we're actually aiming
	local attackPos = localPlayer.position + analogDir * attackRange;
	SWAP_DELTA = LOCK_ON_TARGET.position - attackPos;
end

function SoftLockOnUpdate(localPlayer, originalAttackPos)
	if LockOn.IsSoft() == false then
		return;
	end

	local targetPos = LOCK_ON_TARGET.position; -- TODO: Set to center of hitbox;
	local ownPos = localPlayer.position;

	-- Set initial aim assist location
	if (LOCK_ON_TARGET_START_POS == nil) then
		LOCK_ON_TARGET_START_POS = targetPos;
		LOCK_ON_LOCAL_START_POS = ownPos;
	end

	-- Get delta of when we swapped from a hard lock-on to a soft one
	local swapDelta = Vec3.new(0,0,0);
	if SWAP_DELTA ~= nil then swapDelta = SWAP_DELTA; end

	local targetDelta = targetPos - LOCK_ON_TARGET_START_POS; -- Check where the object is relative to its starting location
	local ownDelta = ownPos - LOCK_ON_LOCAL_START_POS; -- Check where you are, relative to your starting location
	LOCK_ON_POS = originalAttackPos + targetDelta * TARGET_SUGGEST_AMOUNT - ownDelta * SELF_SUGGEST_AMOUNT + swapDelta; -- Suggest the mouse to the new location

	ReportVec3("LockOn/Update/targetPos", targetPos);
	ReportVec3("LockOn/Update/targetDelta", targetDelta);
	ReportVec3("LockOn/Update/ownPos", ownPos);
	ReportVec3("LockOn/Update/ownDelta", ownDelta);
end

function VerifyInternal(localPlayer, hasDir, analogDir)
	if LOCK_ON_DIR == nil then -- It's already not locked on.
		return;
	end

	local distance = 200;
	if hasDir then
		-- Get correct attack range
		local attackRange = 0;
		if not IS_CASTING_SPELL then
			attackRange = localPlayer.attackRange;
		else
			local spellKey = KeyboardImpl.GetSpellKey();
			local castDuration = GetGameTime() - KeyboardImpl.GetSpellCastTime();
			attackRange = GetSpellCastRange(spellKey, castDuration);
		end

		local isValidTarget = Targeting.IsValidTarget(localPlayer, LOCK_ON_TARGET, nil, attackRange, IS_CASTING_SPELL)
		ReportString("LockOn/Verify/isValidTarget", tostring(isValidTarget));
		if isValidTarget then
			-- Check if we moved the stick a bit
			local delta = analogDir - LOCK_ON_DIR;
			distance = delta.length2;
		else
			distance = 300;
		end
	end
	ReportString("LockOn/Verify/distance", tostring(distance));

	-- If moved or released, reset
	if distance > MAX_DIST_LOCK_ON_SQR then
		Log("Released lock on (hasDir = " .. tostring(hasDir) .. ", IS_CASTING_SPELL = " .. tostring(IS_CASTING_SPELL) .. ", IsSoft = " .. tostring(LockOn.IsSoft()) .. ", distance = " .. tostring(distance) .. ", LOCK_ON_TARGET = " .. tostring(LOCK_ON_TARGET) .. ", LOCK_ON_DIR = " .. tostring(LOCK_ON_DIR) .. ")");
		LOCK_ON_TARGET = nil;
		LOCK_ON_DIR = nil;
		LOCK_ON_LOCAL_START_POS = nil;
		LOCK_ON_TARGET_START_POS = nil;
		LOCK_ON_POS = nil;
		SWAP_DELTA = nil;
	end
end

function LockOn.Verify(localPlayer, hasDir, analogDir, isSpellBeingCast)
	if LockOn.IsOn() then
		if IS_CASTING_SPELL and not isSpellBeingCast then Log("Swapping to hard lock-on"); end
		if not IS_CASTING_SPELL and isSpellBeingCast then Log("Swapping to soft lock-on"); SetSwapDelta(localPlayer, analogDir); end
		IS_CASTING_SPELL = isSpellBeingCast;
		CASTING_SPELL = KeyboardImpl.GetSpellKey();
	end

	VerifyInternal(localPlayer, hasDir, analogDir);
	ReportString("LockOn/type", LockOn.GetTypeName());
	ReportVec3("LockOn/SWAP_DELTA", SWAP_DELTA);
	ReportGameObject("LockOn/LOCK_ON_TARGET", LOCK_ON_TARGET);
	ReportString("LockOn/LOCK_ON_DIR", tostring(LOCK_ON_DIR));
	ReportString("LockOn/LOCK_ON_LOCAL_START_POS", tostring(LOCK_ON_LOCAL_START_POS));
	ReportString("LockOn/LOCK_ON_TARGET_START_POS", tostring(LOCK_ON_TARGET_START_POS));
	ReportString("LockOn/LOCK_ON_POS", tostring(LOCK_ON_POS));
end

function LockOn.IsOn()
	return LOCK_ON_DIR ~= nil and IsValidGameObject(LOCK_ON_TARGET);
end

function LockOn.GetTargetInfo(localPlayer, originalAttackPos)
	if not LockOn.IsSoft() and LOCK_ON_TARGET ~= nil then
		return LOCK_ON_TARGET, LOCK_ON_TARGET.position;
	end

	if LOCK_ON_POS == nil then
		SoftLockOnUpdate(localPlayer, originalAttackPos);
	end
	return LOCK_ON_TARGET, LOCK_ON_POS;
end

return LockOn;