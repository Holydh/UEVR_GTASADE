#include "Utilities.h"

glm::fvec3 Utilities::OffsetLocalPositionFromWorld(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
{
	// Apply the offsets along the local axes
	glm::fvec3 offset = (forwardVector * offsets.x) + (rightVector * offsets.y) + (upVector * offsets.z);

	// Calculate the new position
	glm::fvec3 pointWorldPosition = worldPosition + offset;

	return pointWorldPosition;
}