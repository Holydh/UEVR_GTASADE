#pragma once

#define GLM_FORCE_QUAT_DATA_XYZW
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#define _USE_MATH_DEFINES
#include <math.h>

#include "uevr/API.hpp"
#include "MemoryManager.h"
#include "SettingsManager.h"
#include "PlayerManager.h"
#include "Utilities.h"


class CameraController {
private:
	MemoryManager* memoryManager;
	SettingsManager* settingsManager;
	PlayerManager* playerManager;

public:
	CameraController(MemoryManager* mm, SettingsManager* sm, PlayerManager* pm) : memoryManager(mm), settingsManager(sm), playerManager(pm) {}

	float cameraMatrixValues[16] = { 0.0f };
	glm::mat4 accumulatedJoystickRotation = glm::mat4(1.0f);
	glm::mat4 baseHeadRotation = glm::mat4(1.0f);

	glm::fvec3 cameraPositionUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 forwardVectorUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 rightVectorUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 upVectorUE = { 0.0f, 0.0f, 0.0f };

	bool camResetRequested = false;
	int cameraModeIs = 0;
	int cameraModeWas = 0;
	float keepCameraHeightTime = 2.0f;
	float keepCameraHeightTimer = 0.0f;
	bool keepCameraHeight = false;

	bool waterViewFixed = false;

	void ProcessCameraMatrix(float delta);
	void UpdateCameraMatrix();
	void ProcessHookedHeadPosition(float delta);
	void FixUnderwaterView(bool enableFix);
};