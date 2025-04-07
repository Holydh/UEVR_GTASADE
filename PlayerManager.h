#pragma once

#define GLM_FORCE_QUAT_DATA_XYZW
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "uevr/API.hpp"
#include "Utilities.h"
#include "SettingsManager.h"

class PlayerManager {
private:
	SettingsManager* const settingsManager;

public:
	PlayerManager(SettingsManager* sm) : settingsManager(sm) {};
	glm::fvec3 actualPlayerPositionUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 actualPlayerHeadPositionUE = { 0.0f, 0.0f, 0.0f };
	const glm::fvec3 defaultPlayerHeadLocalPositionUE = { 0.0f, 0.0f, 69.0f };
	uevr::API::UObject* playerController = nullptr;
	uevr::API::UObject* playerHead = nullptr;
	uevr::API::UObject* playerHmd = nullptr;
	bool isInControl = false;
	bool wasInControl = false;
	bool isInVehicle = false;
	bool wasInVehicle = false;
	bool shootFromCarInput = false;

	void FetchPlayerUObjects();
	void DisablePlayerUObjectsHook();
};