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
	MemoryManager* const memoryManager;
	SettingsManager* const settingsManager;
	PlayerManager* const playerManager;

	glm::mat4 accumulatedJoystickRotation = glm::mat4(1.0f);
	glm::mat4 baseHeadRotation = glm::mat4(1.0f);

	float keepCameraHeightTime = 2.0f;
	float keepCameraHeightTimer = 0.0f;
	bool keepCameraHeight = false;

	void UpdateCameraMatrix();

public:
	CameraController(MemoryManager* mm, SettingsManager* sm, PlayerManager* pm) : memoryManager(mm), settingsManager(sm), playerManager(pm) {}

	float cameraMatrixValues[16] = { 0.0f };
	
	glm::fvec3 cameraPositionUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 forwardVectorUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 rightVectorUE = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 upVectorUE = { 0.0f, 0.0f, 0.0f };

	enum CameraMode {
		None = 0,
		TopDown = 1,
		GTAClassic = 2,
		BehindCar = 3,
		FollowPed = 4,
		Aiming = 5,
		Debug = 6,
		Sniper = 7,
		RocketLauncher = 8,
		ModelView = 9,
		Bill = 10,
		Syphon = 11,
		Circle = 12,
		CheezyZoom = 13,
		WheelCam = 14,
		Fixed = 15,
		FirstPerson = 16,
		Flyby = 17,
		CamOnAString = 18,
		Reaction = 19,
		FollowPedWithBind = 20,
		Chris = 21,
		BehindBoat = 22,
		PlayerFallenWater = 23,
		CamOnTrainRoof = 24,
		CamRunningSideTrain = 25,
		BloodOnTheTracks = 26,
		ImThePassengerWooWoo = 27,
		SyphonCrimInFront = 28,
		PedDeadBaby = 29,
		PillowsPaps = 30,
		LookAtCars = 31,
		ArrestCamOne = 32,
		ArrestCamTwo = 33,
		M16FirstPerson = 34,
		SpecialFixedForSyphon = 35,
		FightCam = 36,
		TopDownPed = 37,
		Lighthouse = 38,
		SniperRunabout = 39,
		RocketLauncherRunabout = 40,
		FirstPersonRunabout = 41,
		M16FirstPersonRunabout = 42,
		FightCamRunabout = 43,
		Editor = 44,
		HelicannonFirstPerson = 45,
		Camera = 46,
		AttachCam = 47,
		TwoPlayer = 48,
		TwoPlayerInCarAndShooting = 49,
		TwoPlayerSeparateCars = 50,
		RocketLauncherHs = 51,
		RocketLauncherRunaboutHs = 52,
		AimWeapon = 53,
		TwoPlayerSeparateCarsTopDown = 54,
		AimWeaponFromCar = 55,
		DwHeliChase = 56,
		DwCamMan = 57,
		DwBirdy = 58,
		DwPlaneSpotter = 59,
		DwDogFight = 60,
		DwFish = 61,
		DwPlaneCam1 = 62,
		DwPlaneCam2 = 63,
		DwPlaneCam3 = 64,
		AimWeaponAttached = 65
	};
	CameraMode currentCameraMode = None;
	CameraMode previousCameraMode = None;
	bool camResetRequested = false;

	//bool isCutscenePlaying = false;
	//bool wasCutscenePlaying = false;

	void ProcessCameraMatrix(float delta);
	void ProcessHookedHeadPosition(float delta);
	void FixUnderwaterView(bool enableFix);
	bool underwaterViewFixed = false;
};