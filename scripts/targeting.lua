local Targeting = {};

MAX_DIST_TO_NEAREST_OTHER_SQR = 100.0 * 100.0;
MAX_DIST_TO_NEAREST_CHAMPION_SQR = 200.0 * 200.0;
MAX_DIST_TO_NEAREST_BUILDING_SQR = 400.0 * 400.0;

function GetDistanceRequirement(type)
	if (type == GameObjectType.Champion) then
		return MAX_DIST_TO_NEAREST_CHAMPION_SQR;
	end
	if (type == GameObjectType.Turret) then
		return MAX_DIST_TO_NEAREST_BUILDING_SQR;
	end
	if (type == GameObjectType.Inhibitor) then
		return MAX_DIST_TO_NEAREST_BUILDING_SQR;
	end
	if (type == GameObjectType.Nexus) then
		return MAX_DIST_TO_NEAREST_BUILDING_SQR;
	end
	
	return MAX_DIST_TO_NEAREST_OTHER_SQR;
end

function Targeting.GetPriority(gameObject)
	if IsValidGameObject(gameObject) == false then
		return 0;
	end

	local type = gameObject.type;
	if (type == GameObjectType.Nexus) then
		return 6;
	end
	if (type == GameObjectType.Champion) then
		return 5;
	end
	if (type == GameObjectType.Turret) then
		return 4;
	end
	if (type == GameObjectType.Inhibitor) then
		return 3;
	end
	if (type == GameObjectType.Minion or type == GameObjectType.Monster) then
		return 2;
	end
	
	return 1;
end

function Targeting.IsValidTarget(localPlayer, targetObject, attackPos, attackRange, isSpellBeingCast)
	if IsValidGameObject(targetObject) == false then
		-- ReportBool("Targeting/IsValidTarget/" .. targetObject.name .. "/isInRange", false);
		-- ReportString("Targeting/IsValidTarget/" .. targetObject.name .. "/rangeReason", "Does not exist");
		return false;
	end

	-- If the type of the object is not a plant and we're casting a spell, return false
	if (targetObject.type == GameObjectType.Plant and isSpellBeingCast) then
		-- ReportBool("Targeting/IsValidTarget/" .. targetObject.name .. "/isInRange", false);
		-- ReportString("Targeting/IsValidTarget/" .. targetObject.name .. "/rangeReason", "Is plant (casting spell)");
		return false;
	end

	local deltaToPlayer = localPlayer.position - targetObject.position;
	if (deltaToPlayer.length2 > attackRange * attackRange) then
		-- ReportBool("Targeting/IsValidTarget/" .. targetObject.name .. "/isInRange", false);
		-- ReportString("Targeting/IsValidTarget/" .. targetObject.name .. "/rangeReason", "attackRange (" .. deltaToPlayer.length .. " > " .. attackRange .. ")");
		return false;
	end

	local isInRange = true;
	if attackPos ~= nil then
		local delta = targetObject.position - attackPos;
		isInRange = delta.length2 < GetDistanceRequirement(targetObject.type);
		-- ReportBool("Targeting/IsValidTarget/" .. targetObject.name .. "/isInRange", isInRange);
		-- ReportString("Targeting/IsValidTarget/" .. targetObject.name .. "/rangeReason", "GetDistanceRequirement");
	end

	-- ReportBool("Targeting/IsValidTarget/" .. targetObject.name .. "/isInRange", isInRange);
	-- ReportString("Targeting/IsValidTarget/" .. targetObject.name .. "/rangeReason", "Can hit");
	return isInRange;
end

return Targeting;