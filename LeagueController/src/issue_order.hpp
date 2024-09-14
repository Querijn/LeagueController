#pragma once

#include <glm/vec3.hpp>

namespace LeagueController
{
	struct GameObject;

	bool StopMoving();
	bool TryAttack(GameObject* object);
	bool MovePlayerTo(const glm::vec3& pos);

	void InitOrders();
	void UpdateOrders();
	void DestroyOrders();
}
