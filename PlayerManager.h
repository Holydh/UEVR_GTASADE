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
	const glm::fvec3 defaultBikeLocalOffsetUE = { 0.0f, -30.0f, 0.0f };
	uevr::API::UObject* playerController = nullptr;
	uevr::API::UObject* playerHead = nullptr;
	bool isInControl = false;
	bool wasInControl = false;
	bool isInVehicle = false;
	bool wasInVehicle = false;
	enum VehicleType {
		OnFoot = 4,
		CarOrBoat = 10,
		Bike = 13,
		Helicopter = 16,
		Plane = 19,
	};
	VehicleType vehicleType = OnFoot;
	bool shootFromCarInput = false;
	bool weaponWheelEnabled = false;

	void FetchPlayerUObjects();
	void RepositionUnhookedUobjects();
};