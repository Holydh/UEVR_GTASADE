#pragma once
#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "MemoryManager.h"
#include "SettingsManager.h"
#include "PlayerManager.h"
#include "Utilities.h"
#include "uevr/API.hpp"
#define _USE_MATH_DEFINES
#include <math.h>


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

	bool camResetRequested = false;
	int cameraModeIs = 0;
	int cameraModeWas = 0;

	bool waterViewFixed = false;

	void UpdateCameraMatrix(float delta);
	void ProcessHookedHeadPosition();
	void FixUnderwaterView(bool enableFix);
};

#endif