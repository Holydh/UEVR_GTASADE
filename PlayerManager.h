#pragma once
#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H
#include "uevr/API.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "Utilities.h"

class PlayerManager {
private:


public:
	glm::fvec3 actualPlayerPositionUE = { 0.0f, 0.0f, 0.0f };
	uevr::API::UObject* playerController = nullptr;
	uevr::API::UObject* playerHead = nullptr;
	bool playerIsInControl = false;
	bool playerWasInControl = false;
	bool characterIsInVehicle = false;
	bool characterWasInVehicle = false;
	bool isShooting = false;
	bool playerShootFromCarInput = false;

	void FetchPlayerUObjects();
	void DisablePlayerUObjectsHook();
};

#endif