local Mouse = {};

KeyboardImpl =	require("keyboard");
SpellImpl =	require("spells");
Targeting = require("targeting");
LockOn = require("lock_on");

AUTOATTACK_TARGET = nil;
LAST_RANGE = 0;
LAST_RANGE_FRAME_COUNT = 0;

function DetermineTargets(localPlayer, autoAttackPos, attackRange, isSpellBeingCast, analogDir)
	if LockOn.IsOn() then
		LockOn.Update(localPlayer, autoAttackPos);
		return LockOn.GetTargetInfo(localPlayer, autoAttackPos);
	end

	-- Check if we have an object nearby
	local nearestObject = GetNearestObject();
	local nearestChampion = GetNearestChampion();
	if nearestObject == nil and nearestChampion == nil then
		return nil, autoAttackPos;
	end

	-- Nearest champion
	if Targeting.IsValidTarget(localPlayer, nearestChampion, autoAttackPos, attackRange, isSpellBeingCast) then
		-- Check if we can lock on.
		if LockOn.Set(nearestObject, analogDir) then
			return LockOn.GetTargetInfo(localPlayer, autoAttackPos);
		end
		return nearestChampion, nearestChampion.position;

	-- Nearest object
	elseif Targeting.IsValidTarget(localPlayer, nearestObject, autoAttackPos, attackRange, isSpellBeingCast) then
		-- Check if we can lock on.
		if LockOn.Set(nearestObject, analogDir) then
			return LockOn.GetTargetInfo(localPlayer, autoAttackPos);
		end
		return nearestObject, nearestObject.position;
	end
	
	return nil, autoAttackPos;
end

function MousePosInternal(localPlayer, isSpellBeingCast)
	local hasDir, analogDir = GetRightAnalogDir(true, false);
	LockOn.Verify(localPlayer, hasDir, analogDir, isSpellBeingCast);
	ReportGameObject("Mouse/localPlayer", localPlayer);
	ReportBool("Mouse/isSpellBeingCast", isSpellBeingCast);

	if isSpellBeingCast then -- If we're casting a dash, we want toaim with the left analog over the right analog
		local spellKey = KeyboardImpl.GetSpellKey();
		if IsDashSpell(spellKey) then
			local hasLeftDir, leftDir = GetLeftAnalogDir(true, false);
			if hasLeftDir then
				hasDir = true;
				analogDir = leftDir;
			end
		end
	end

	if hasDir == false then

		-- If we're casting a spell, we might want to aim with the left analog
		if not isSpellBeingCast then
			return nil, nil;
		end

		-- Use left dir instead
		hasDir, analogDir = GetLeftAnalogDir(true, false);
		if hasDir == false then
			return nil, nil;
		end
	end

	local attackRange = localPlayer.attackRange;
	if isSpellBeingCast then
		-- Check spell range
		local spellKey = KeyboardImpl.GetSpellKey();
		local castDuration = GetGameTime() - KeyboardImpl.GetSpellCastTime();
		attackRange = GetSpellCastRange(spellKey, castDuration) * 0.99;

		-- This makes sure we always have a range - This fixes Xerath Q for instance
		-- While charging he has a range, but on cast it immediately jumps to 0 because
		-- the spell becomes different after release.
		if attackRange == 0 then
			attackRange = LAST_RANGE;
		end
		LAST_RANGE = attackRange;
		LAST_RANGE_FRAME_COUNT = 3;
	end

	if LAST_RANGE_FRAME_COUNT > 0 then
		attackRange = LAST_RANGE;
		LAST_RANGE_FRAME_COUNT = LAST_RANGE_FRAME_COUNT - 1;
	end
	
	local attackDir = analogDir * attackRange; -- This gives us the direction we're aiming in, in world space
	local attackPos = localPlayer.position + attackDir; -- This gives us the position we're aiming at, in world space

	-- Check if we have a lock-on target
	local nearestObject, overridePosition = DetermineTargets(localPlayer, attackPos, attackRange, isSpellBeingCast, analogDir);
	ReportGameObject("Mouse/nearestObject", nearestObject);
	ReportVec3("Mouse/overridePosition", overridePosition);
	if nearestObject ~= nil then
		attackPos = overridePosition;
	end

	local mousePos = WorldToScreen(attackPos); -- This gives us the position we're aiming at, in screen space

	ReportVec3("Mouse/attackDir", attackDir);
	ReportVec3("Mouse/analogDir", analogDir);
	ReportFloat("Mouse/analogDirLength", analogDir.length);
	ReportFloat("Mouse/attackRange", attackRange);
	return mousePos, nearestObject;
end

function Mouse.Init()
end

function Mouse.UpdatePos(localPlayer, isMoving)
	if IsLizardModeEnabled() then
		ShowCrosshair(false);
		return;
	end

	local isSpellBeingCast = SpellImpl.IsSpellBeingCast(localPlayer);
	local mousePos, targetObject = MousePosInternal(localPlayer, isSpellBeingCast);
	local isAiming = mousePos ~= nil;
	ReportBool("Mouse/isSpellBeingCast", isSpellBeingCast);
	ReportBool("Mouse/isAiming", isAiming);
	ReportBool("Mouse/isMoving", isMoving);
	ReportVec2("Mouse/mousePos", mousePos);
	if isAiming then
		if isSpellBeingCast then
			SetInstantMouseTarget(mousePos, isAiming);
		else
			SetMouseTarget(mousePos, isAiming);
		end
		SetCrosshairPos(mousePos);
	end

	if isMoving or isSpellBeingCast then AUTOATTACK_TARGET = nil; end -- Reset auto-attack request
	if not isMoving and targetObject ~= nil and AUTOATTACK_TARGET ~= targetObject and not isSpellBeingCast then
		localPlayer.TryAttack(targetObject);
		AUTOATTACK_TARGET = targetObject;
	end
	ShowCrosshair(isAiming);
end

return Mouse;