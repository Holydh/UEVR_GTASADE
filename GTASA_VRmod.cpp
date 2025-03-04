#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "uevr/Plugin.hpp"
#define _USE_MATH_DEFINES
#include <math.h>
#include "MemoryManager.h"
#include <windows.h>
#include <iostream>
#include <string>


using namespace uevr;

#define PLUGIN_LOG_ONCE(...) {\
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }}

class GTASA_VRmod : public uevr::Plugin {
private:
	std::string configFilePath;
	FILETIME lastWriteTime;

	MemoryManager memoryManager;

	//variables
	float maxCrouchOffset = 60.0f;  // Maximum offset when crouching
	float duckSpeed = 2.5f;     // Speed per frame adjustment
	bool isCrouching;

	float cameraMatrixValues[16] = { 0.0f };
	float xAxisSensitivity = 125.0f;
	glm::fvec3 actualPlayerPositionUE = { 0.0f, 0.0f, 0.0f };
	glm::mat4 accumulatedJoystickRotation = glm::mat4(1.0f);
	glm::mat4 baseHeadRotation = glm::mat4(1.0f);

	//bool characterIsGettingInACar = false;
	bool characterIsInVehicle = false;
	bool characterWasInVehicle = false;
	int equippedWeaponIndex = 0;
	uevr::API::UObject* playerController = nullptr;
	uevr::API::UObject* playerHead = nullptr;
	uevr::API::UObject* weapon = nullptr;
	uevr::API::UObject* weaponMesh = nullptr;
	uevr::API::UObject* weaponStaticMesh = nullptr;

	glm::fvec3 crosshairOffset = { 0.0f, -1.0f, 2.0f };

	bool playerIsInControl = false;
	bool playerWasInControl = false;
	bool camResetRequested = false;
	int cameraMode = 0;
	int cameraModeWas = 0;

	//recoil
	glm::fvec3 defaultWeaponRotationEuler = { 0.4f, 0.0f, 0.0f };
	glm::fvec3 defaultWeaponPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilRotationEuler = { 0.0f, 0.0f, 0.0f };
	float recoilPositionRecoverySpeed = 10.0f;
	float recoilRotationRecoverySpeed = 8.0f;
	bool isShooting = false;

	std::unordered_map<std::wstring, int> weaponNameToIndex = {
		{L"SM_unarmed", 0},           // Unarmed
		{L"SM_brassknuckle", 1},    // BrassKnuckles
		{L"SM_golfclub", 2},         // GolfClub
		{L"SM_nightstick", 3},       // NightStick
		{L"SM_knifecur", 4},            // Knife
		{L"SM_bat", 5},      // BaseballBat
		{L"SM_shovel", 6},           // Shovel
		{L"SM_poolcue", 7},          // PoolCue
		{L"SM_katana", 8},           // Katana
		{L"SM_chnsaw", 9},         // Chainsaw
		{L"SM_gun_dildo1", 10},          // Dildo1
		{L"SM_gun_dildo2", 11},          // Dildo2
		{L"SM_gun_vibe1", 12},           // Vibe1
		{L"SM_gun_vibe2", 13},           // Vibe2
		{L"SM_flowera", 14},         // Flowers
		{L"SM_gun_cane", 15},            // Cane
		{L"SM_grenade", 16},         // Grenade
		{L"SM_teargas", 17},         // Teargas
		{L"SM_molotov", 18},         // Molotov
		{L"SM_colt45", 22},          // Pistol Colt 45
		{L"SM_silenced", 23},        // Silenced Pistol
		{L"SM_desert_eagle", 24},     // Desert Eagle
		{L"SM_chromegun", 25},         // Shotgun
		{L"SM_sawnoff", 26},         // Sawnoff Shotgun
		{L"SM_shotgspa", 27},          // Spas12
		{L"SM_micro_uzi", 28},             // MicroUzi
		{L"SM_mp5lng", 29},             // MP5
		{L"SM_ak47", 30},            // AK47
		{L"SM_m4", 31},              // M4
		{L"SM_tec9", 32},            // Tec9
		{L"SM_cuntgun", 33},         // Rifle (Cuntgun)
		{L"SM_sniper", 34},          // Sniper Rifle
		{L"SM_rocketla", 35},  // RocketLauncher
		{L"SM_heatseek", 36},// RocketLauncherHeatSeek
		{L"SM_flame", 37},    // Flamethrower
		{L"SM_minigun", 38},         // Minigun
		{L"SM_satchel", 39},         // Satchel
		{L"SM_detonator", 40},       // Detonator
		{L"SM_spraycan", 41},        // SprayCan
		{L"SM_fire_ex", 42},    // Extinguisher
		{L"SM_camera", 43},          // Camera
		{L"SM_nvgoggles", 44},     // NightVision
		{L"SM_irgoggles", 45},        // Infrared
		{L"SM_gun_para", 46}        // Parachute
	};

public:
	GTASA_VRmod() = default;

	void UpdateSettings()
	{
		xAxisSensitivity = GetFloatValueFromFile(configFilePath, "VR_AimSpeed", 125.0f) * 10;
	}

	void on_dllmain_attach() override {}

	void on_dllmain_detach() override {
		memoryManager.RemoveBreakpoints();
		memoryManager.RemoveExceptionHandler();
	}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");
		configFilePath = GetConfigFilePath();
		API::get()->log_info("%s", configFilePath.c_str());
		

		memoryManager.baseAddressGameEXE = memoryManager.GetModuleBaseAddress(nullptr);
		memoryManager.AdjustAddresses();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);

		UpdateSettingsIfModified(configFilePath);

		playerIsInControl = *(reinterpret_cast<uint8_t*>(memoryManager.playerHasControl)) == 0;

	/*	API::get()->log_info("playerIsInControl = %i",playerIsInControl);*/
		//Debug
		//if (GetAsyncKeyState(VK_UP)) fpsCamInitialized = true;
		//if (GetAsyncKeyState(VK_DOWN)) fpsCamInitialized = false;

		bool weaponWheelOpen = *(reinterpret_cast<int*>(memoryManager.weaponWheelOpenAddress)) > 30;
		/*characterIsGettingInACar = *(reinterpret_cast<byte*>(memoryManager.characterIsGettingInACarAddress)) > 0;*/
		
		characterIsInVehicle = *(reinterpret_cast<uint8_t*>(memoryManager.characterIsInVehicleAddress)) > 0;
		//API::get()->log_info("characterIsInCar = %i", characterIsInCar);
		cameraMode = *(reinterpret_cast<int*>(memoryManager.cameraModeAddress));
		//API::get()->log_info("weaponWheelOpen = %i", weaponWheelOpen);

		isShooting = memoryManager.isShooting;
		memoryManager.isShooting = false;

		isCrouching = characterIsInVehicle ? false : memoryManager.isCrouching;
		memoryManager.isCrouching = false;

		
		if (playerIsInControl)
			FetchRequiredUObjects();
		
		if (playerIsInControl && !playerWasInControl)
		{
			camResetRequested = true;
			memoryManager.ToggleAllMemoryInstructions(false);
			HandleCutscenes();

			memoryManager.InstallBreakpoints();
		}
		else
			camResetRequested = false;

		if (!playerIsInControl && playerWasInControl)
		{
			memoryManager.ToggleAllMemoryInstructions(true);
			HandleCutscenes();

			memoryManager.RemoveBreakpoints();
			memoryManager.RemoveExceptionHandler();
			//API::get()->log_info("RemoveBreakpoints");
			/*API::get()->log_info("playerHasControl = %i", playerIsInControl);*/
		}

		if (playerIsInControl && ((characterIsInVehicle && !characterWasInVehicle) || (characterIsInVehicle && cameraMode != 55 && cameraModeWas == 55)))
		{
			memoryManager.RestoreVehicleRelatedMemoryInstructions();
		}

		if (playerIsInControl && ((!characterIsInVehicle && characterWasInVehicle) || (characterIsInVehicle && cameraMode == 55 && cameraModeWas != 55)))
		{
			memoryManager.NopVehicleRelatedMemoryInstructions();
		}

		if (playerIsInControl)
		{
			if (!weaponWheelOpen)
			{
				UpdateCameraMatrix(delta);
				UpdateAimingVectors();
				PlayerCrouching();
			}

			if (weaponMesh != nullptr)
			{
				FixWeaponVisibility();
				WeaponHandling(delta);
			}
		}

		playerWasInControl = playerIsInControl;
		characterWasInVehicle = characterIsInVehicle;
		cameraModeWas = cameraMode;
	}

	void on_post_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);
	}

	void on_pre_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
		PLUGIN_LOG_ONCE("Pre Slate Draw Window");
	}

	void on_post_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
		PLUGIN_LOG_ONCE("Post Slate Draw Window");
	}

	void FixWeaponVisibility()
	{
		struct {
			bool ownerNoSee = false;
		} setOwnerNoSee_params;
		weaponMesh->call_function(L"SetOwnerNoSee", &setOwnerNoSee_params);
	}

	void UpdateCameraMatrix(float delta) {
		struct {
			glm::fvec3 ForwardVector;
		} forwardVector_params;

		struct {
			glm::fvec3 UpVector;
		} upVector_params;

		struct {
			glm::fvec3 RightVector;
		} rightVector_params;

		playerHead->call_function(L"GetForwardVector", &forwardVector_params);
		playerHead->call_function(L"GetUpVector", &upVector_params);
		playerHead->call_function(L"GetRightVector", &rightVector_params);


		// Camera Matrix rotation ---------------------------------------------------
		UEVR_Vector2f rightJoystick{};
		API::get()->param()->vr->get_joystick_axis(API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);

		// Create a full rotation matrix from the head's forward, up, and right vectors
		glm::mat4 headRotationMatrix = glm::mat4(1.0f);

		headRotationMatrix[0] = glm::vec4(-forwardVector_params.ForwardVector.x, forwardVector_params.ForwardVector.y, -forwardVector_params.ForwardVector.z, 0.0f);
		headRotationMatrix[1] = glm::vec4(-rightVector_params.RightVector.x, rightVector_params.RightVector.y, -rightVector_params.RightVector.z, 0.0f); // Right vector
		headRotationMatrix[2] = glm::vec4(upVector_params.UpVector.x, -upVector_params.UpVector.y, upVector_params.UpVector.z, 0.0f);      // Up vector 

		float joystickYaw = 0.0f;

		if (characterIsInVehicle && !characterWasInVehicle)
		{
			accumulatedJoystickRotation = glm::mat4(1.0f);
		}
		if ((!characterIsInVehicle && characterWasInVehicle) || (!characterIsInVehicle && camResetRequested))
		{
			//camResetRequested = true;
			accumulatedJoystickRotation = glm::mat4(1.0f);
			baseHeadRotation = headRotationMatrix;
		}

		// Calculate the delta rotation matrix. 
		// Store the base head rotation on the frame the character is out of the car, so the accumulatedJoystickRotation drives it.
		// If the player is in a car, keep the headRotationMatrix drive so the camera follows the car heading.
		glm::mat4 deltaRotationMatrix = characterIsInVehicle && cameraMode != 55 ? glm::inverse(accumulatedJoystickRotation) * headRotationMatrix : glm::inverse(accumulatedJoystickRotation) * baseHeadRotation;

		// Apply joystick input to adjust the local yaw rotation
		const float DEADZONE = 0.1f;
		joystickYaw = /*camResetRequested ? 180.0f : */0.0f;
		if (abs(rightJoystick.x) > DEADZONE) {
			joystickYaw = -rightJoystick.x * delta * xAxisSensitivity;
		}

		// Convert joystick yaw to radians
		float yawRadians = joystickYaw * (M_PI / 180.0f);

		// Create a yaw rotation matrix for the joystick input
		glm::mat4 joystickYawRotation = glm::rotate(glm::mat4(1.0f), yawRadians, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around the Y-axis

		// Combine the delta rotation with the joystick yaw rotation
		accumulatedJoystickRotation = accumulatedJoystickRotation * joystickYawRotation;

		// Combine the delta rotation with the joystick yaw rotation
		glm::mat4 totalDeltaRotation = accumulatedJoystickRotation * deltaRotationMatrix;

		//// Combine the head rotation matrix with the joystick yaw rotation
		glm::mat4 finalRotationMatrix = accumulatedJoystickRotation * totalDeltaRotation;

		// Copy the modified matrix back to cameraMatrixValues
		for (int i = 0; i < 16; ++i) {
			cameraMatrixValues[i] = finalRotationMatrix[i / 4][i % 4];
		}

		// Write the modified matrix back to memory
		for (int i = 0; i < 12; ++i) {
			*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[i])) = cameraMatrixValues[i];
		}

		//If player loads a save or after a cinematic, reset the camera to the camera heading direction
		if (camResetRequested) {
			*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[0])) = -1;
			*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[5])) = 1;
			*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[10])) = 1;
		}

		// Update the camera position based on the head's socket location
		struct {
			API::FName InSocketName = API::FName(L"head");
			glm::fvec3 Location;
		} socketLocation_params;

		playerHead->call_function(L"GetSocketLocation", &socketLocation_params);

		//fix audio listener unaligned with original game
		glm::fvec3 offsetedPosition = OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, glm::fvec3(49.5, 0.0, 0.0));

		*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[12])) = offsetedPosition.x * 0.01f;
		*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[13])) = -offsetedPosition.y * 0.01f;
		*(reinterpret_cast<float*>(memoryManager.cameraMatrixAddresses[14])) = offsetedPosition.z * 0.01f;
		actualPlayerPositionUE = socketLocation_params.Location;
	}

	void WeaponHandling(float delta)
	{
		if (characterIsInVehicle && !characterWasInVehicle)
		{
			UpdateActualWeaponMesh();
		}

		if (!characterIsInVehicle && characterWasInVehicle)
		{
			UpdateActualWeaponMesh();
		}

	/*	if ((characterIsInVehicle && cameraMode != 55)) //check a shooting on car scenario before deleting
			return;*/
			
		glm::fvec3 positionRecoilForce = { 0.0f, 0.0f, 0.0f };
		glm::fvec3 rotationRecoilForceEuler = { 0.0f, 0.0f, 0.0f };
		switch (equippedWeaponIndex)
		{
			//case 0:  // Unarmed
			//case 1:  // BrassKnuckles
			//case 2:  // GolfClub
			//case 3:  // NightStick
			//case 4:  // Knife
			//case 5:  // BaseballBat
			//case 6:  // Shovel
			//case 7:  // PoolCue
			//case 8:  // Katana
			//case 9:  // Chainsaw
			//case 10: // Dildo1
			//case 11: // Dildo2
			//case 12: // Vibe1
			//case 13: // Vibe2
			//case 14: // Flowers
			//case 15: // Cane
			//case 16: // Grenade
			//case 17: // Teargas
			//case 18: // Molotov
		case 22: //Pistol colt 45
			positionRecoilForce = { 0.0f, -0.005f, -0.5f };
			rotationRecoilForceEuler = { -0.08f, 0.0f, 0.0f };
			break;
		case 23: // PistolSilenced
			positionRecoilForce = { 0.0f, -0.005f, -0.5f };
			rotationRecoilForceEuler = { -0.05f, 0.0f, 0.0f };
			break;
		case 24: // DesertEagle
			positionRecoilForce = { 0.0f, -0.005f, -2.5f };
			rotationRecoilForceEuler = { -0.15f, 0.0f, 0.0f };
			break;
		case 25: // Shotgun
			positionRecoilForce = { 0.0f, -0.005f, -5.0f };
			rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
			break;
		case 26: // Sawnoff
			positionRecoilForce = { 0.0f, -0.005f, -5.0f };
			rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
			break;
		case 27: // Spas12
			positionRecoilForce = { 0.0f, -0.005f, -5.0f };
			rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
			break;
		case 28: // MicroUzi
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 29: // Mp5
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 30: //AK47
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 31: // M4
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 32: // Tec9
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 33: //Rifle cuntgun
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 34: // Sniper
			positionRecoilForce = { 0.0f, -0.005f, -1.0f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 35: // RocketLauncher
			positionRecoilForce = { 0.0f, -0.005f, -1.5f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 36: // RocketLauncherHeatSeek
			positionRecoilForce = { 0.0f, -0.005f, -1.5f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
		case 37: // Flamethrower
			positionRecoilForce = { 0.0f, -0.00001f, -0.0001f };
			rotationRecoilForceEuler = { -0.0001f, 0.0f, 0.0f };
			break;
		case 38: // Minigun
			positionRecoilForce = { 0.0f, -0.005f, -1.5f };
			rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
			break;
			//case 39: // Satchel
			//case 40: // Detonator
			//case 41: // SprayCan
			//case 42: // Extinguisher
			//case 43: // Camera
			//case 44: // NightVision
			//case 45: // Infrared
			//case 46: // Parachute

		default:
			return;
		}

		auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
		

		if (isShooting)
		{
			// use the UEVR uobject attached offset : 
			currentWeaponRecoilPosition += positionRecoilForce;
			currentWeaponRecoilRotationEuler += rotationRecoilForceEuler;
			UEVR_Vector3f recoilPosition = { currentWeaponRecoilPosition.x, currentWeaponRecoilPosition.y, currentWeaponRecoilPosition.z };
			glm::fquat weaponRecoilRotationQuat = glm::fquat(currentWeaponRecoilRotationEuler);
			UEVR_Quaternionf recoilRotation = { weaponRecoilRotationQuat.w, weaponRecoilRotationQuat.x, weaponRecoilRotationQuat.y, weaponRecoilRotationQuat.z };
			motionState->set_location_offset(&recoilPosition);
			motionState->set_rotation_offset(&recoilRotation);
			motionState->set_permanent(true);
		}
		else
		{
			// Smoothly return position to base
			currentWeaponRecoilPosition = glm::mix(currentWeaponRecoilPosition, defaultWeaponPosition, delta * recoilPositionRecoverySpeed);
			UEVR_Vector3f recoveredPositionFromRecoil = { currentWeaponRecoilPosition.x, currentWeaponRecoilPosition.y, currentWeaponRecoilPosition.z };
			motionState->set_location_offset(&recoveredPositionFromRecoil);

			// Convert Euler angles (radians) to quaternions for smooth rotation recovery
			glm::fquat weaponRecoilRotationQuat = glm::fquat(currentWeaponRecoilRotationEuler);
			glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);

			// Smoothly interpolate between current rotation and default rotation
			glm::fquat smoothedWeaponRotationQuat = glm::slerp(weaponRecoilRotationQuat, defaultWeaponRotationQuat, delta * recoilRotationRecoverySpeed);
			currentWeaponRecoilRotationEuler = glm::eulerAngles(smoothedWeaponRotationQuat);
			UEVR_Quaternionf recoveredRotationFromRecoil = { smoothedWeaponRotationQuat.w, smoothedWeaponRotationQuat.x, smoothedWeaponRotationQuat.y, smoothedWeaponRotationQuat.z };
			motionState->set_rotation_offset(&recoveredRotationFromRecoil);
		}

	}

	void PlayerCrouching()
	{
		//duck
		//Ducking -----------------------------
		// Check if the player is crouching
		float currentCrouchOffset = *(reinterpret_cast<float*>(memoryManager.currentCrouchOffsetAddress));

		if (isCrouching && currentCrouchOffset > -maxCrouchOffset) {
			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = glm::fvec3(0.0f, 0.0f, -duckSpeed);
			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);

			*(reinterpret_cast<float*>(memoryManager.currentCrouchOffsetAddress)) = currentCrouchOffset - duckSpeed;
		}
		else if (isCrouching && currentCrouchOffset <= -maxCrouchOffset)
		{
			*(reinterpret_cast<float*>(memoryManager.currentCrouchOffsetAddress)) = -maxCrouchOffset;
			/*	*(reinterpret_cast<uint8_t*>(characterWasDuckingAddress)) = 1;*/
		}


		if (!isCrouching && currentCrouchOffset < 0.0f) {
			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = glm::fvec3(0.0f, 0.0f, duckSpeed);
			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);

			*(reinterpret_cast<float*>(memoryManager.currentCrouchOffsetAddress)) = currentCrouchOffset + duckSpeed;
		}
		else if (!isCrouching && currentCrouchOffset >= 0.0f)
		{
			*(reinterpret_cast<float*>(memoryManager.currentCrouchOffsetAddress)) = 0.0f;
		}

		//// Update the previous crouch state
		//wasDucking = isDucking;
	}

	void UpdateAimingVectors()
	{
		if (weaponMesh != nullptr) {
			struct {
				glm::fvec3 ForwardVector;
			} forwardVector_params;

			struct {
				glm::fvec3 UpVector;
			} upVector_params;

			struct {
				glm::fvec3 RightVector;
			} rightVector_params;


			weaponMesh->call_function(L"GetForwardVector", &forwardVector_params);
			weaponMesh->call_function(L"GetUpVector", &upVector_params);
			weaponMesh->call_function(L"GetRightVector", &rightVector_params);

			glm::fvec3 point1Offsets = { 0.0f, 0.0f, 0.0f };
			glm::fvec3 point2Offsets = { 0.0f, 0.0f, 0.0f };
			bool socketAvailable = true;

			//mesh alignement weapon offsets
			switch (equippedWeaponIndex)
			{
				//case 0:  // Unarmed
				//case 1:  // BrassKnuckles
				//case 2:  // GolfClub
				//case 3:  // NightStick
				//case 4:  // Knife
				//case 5:  // BaseballBat
				//case 6:  // Shovel
				//case 7:  // PoolCue
				//case 8:  // Katana
				//case 9:  // Chainsaw
				//case 10: // Dildo1
				//case 11: // Dildo2
				//case 12: // Vibe1
				//case 13: // Vibe2
				//case 14: // Flowers
				//case 15: // Cane
				//case 16: // Grenade
				//case 17: // Teargas
				//case 18: // Molotov
			case 22: //Pistol colt 45
				point1Offsets = { 2.82819, -2.52103, 9.92684 };
				point2Offsets = { 21.7272, -3.89487, 12.9088 +0.2 };
				break;
			case 23: // PistolSilenced
				point1Offsets = { 2.80735, -2.52308, 9.9193 };
				point2Offsets = { 17.3316, -3.5591 + 0.1, 12.2129 + 0.6 };
				break;
			case 24: // DesertEagle
				point1Offsets = { 7.06492 , -2.25853 , 11.9386 + 0.5 };
				point2Offsets = { 33.5914, -1.46079 - 0.5, 11.9439 - 0.5 };
				break;
			case 25: // Shotgun
				point1Offsets = { 31.3429, -0.670153, 15.2663 };
				point2Offsets = { 73.6795 , 4.2357 - 1 , 22.2237 - 2 };
				break;
			case 26: // Sawnoff
				point1Offsets = { 21.2896, -2.13098 , 13.0224 };
				point2Offsets = { 55.8867 , -2.10406 - 1, 16.3934 - 2 };
				break;
			case 27: // Spas12
				point1Offsets = { 51.9659 , 1.30133, 19.5475 };
				point2Offsets = { 70.459 , 3.20646 , 22.5404 };
				break;
			case 28: // MicroUzi
				point1Offsets = { -0.267532, -2.19868 , 10.2951 };
				point2Offsets = { 12.9468 , -0.996034+0.4, 11.293+0.9 };
				break;
			case 29: // Mp5
				point1Offsets = { 6.8924, -1.74509 , 19.3761 };
				point2Offsets = { 21.3778 , 0.000536+0.2, 21.2535+1 };
				break;
			case 30: //AK47
				point1Offsets = { 3.8416 , -2.83908, 14.3539 };
				point2Offsets = { 36.3719, 0.193737-0.2, 16.1544 -0.2 };
				break;
			case 31: // M4
				point1Offsets = { 5.85945 , -1.78476 , 15.1271 };
				point2Offsets = { 60.0434  , 2.99539 - 1 , 16.4006 - 1.5 };
				break;
			case 32: // Tec9
				point1Offsets = { 1.1631 , -3.60654, 11.7162 };
				point2Offsets = { 24.9241 , -3.60654, 13.9038 - 1 };
				break;
			case 33: //Rifle cuntgun
				point1Offsets = { 7.92837 , -3.48911 , 11.4936 };
				point2Offsets = { 71.2598, 4.09339 - 0.75, 20.9391 - 1.5 }; //additional offsets required. Crosshair offset is probably different for that weapon
				break;
			case 34: // Sniper
				point1Offsets = { 3.00373 , -3.05089 , 10.5162 };
				point2Offsets = { 76.0552 , 4.39762, 17.8463 };
				break;
				//case 35: // RocketLauncher
				//	point1Offsets = { 2.41748 , -3.88386 , 14.4056  };
				//	point2Offsets = { 29.0589, -3.88386, 14.4056  };
				//	break;
				//case 36: // RocketLauncherHeatSeek
				//	point1Offsets = { -57.665 , -3.74195 , 20.2618  };
				//	point2Offsets = { 34.8035, -3.52085 , 20.1928   };
				//	break;
			case 37: // Flamethrower
				point1Offsets = { 48.0165 , -1.65182 , 16.1683 };
				point2Offsets = { 76.7885, 0.537026 , 31.6837 };
				break;
			case 38: // Minigun
				point1Offsets = { 48.1025 , -2.9978 , 14.3878 };
				point2Offsets = { 86.6453 , 0.429413 -0.5 , 35.9644 - 0.5 };
				break;
				//case 39: // Satchel
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 40: // Detonator
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 41: // SprayCan
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 42: // Extinguisher
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 43: // Camera
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 44: // NightVision
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 45: // Infrared
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;
				//case 46: // Parachute
				//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
				//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
				//	break;

			default:
				point1Offsets = { 0.0f , 0.0f, 0.0f };
				point2Offsets = { 0.0f , 0.0f, 0.0f };
				socketAvailable = false;
				break;
			}

			glm::fvec3 point1Position = { 0.0f , 0.0f, 0.0f };
			glm::fvec3 point2Position = { 0.0f , 0.0f, 0.0f };
			glm::fvec3 aimingDirection = { 0.0f , 0.0f, 0.0f };

			if (socketAvailable)
			{
				struct {
					const struct API::FName& InSocketName = API::FName(L"gunflash");
					glm::fvec3 Location;
				} socketLocation_params;

				weaponMesh->call_function(L"GetSocketLocation", &socketLocation_params);
				//API::get()->log_info("ForwardVector : x = %f, y = %f, z = %f", forwardVector_params.ForwardVector.x,  forwardVector_params.ForwardVector.y, forwardVector_params.ForwardVector.z);

				point1Position = OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point1Offsets);
				point2Position = OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point2Offsets);

				aimingDirection = glm::normalize(point2Position - point1Position);

				glm::fvec3 projectedToFloorVector = glm::fvec3(aimingDirection.x, aimingDirection.y, 0.0);
				// Safeguard: Normalize projectedToFloorVector only if valid
				if (glm::length(projectedToFloorVector) > 0.0f) {
					projectedToFloorVector = glm::normalize(projectedToFloorVector);
				}
				else {
					projectedToFloorVector = glm::fvec3(1.0f, 0.0f, 0.0f); // Fallback vector
				}


				glm::fvec3 yawRight = glm::cross(glm::fvec3(0.0f, 0.0f, 1.0f), projectedToFloorVector);
				if (glm::length(yawRight) > 0.0f) {
					yawRight = glm::normalize(yawRight);
				}
				else {
					yawRight = glm::fvec3(0.0f, -1.0f, 0.0f); // Fallback vector if collinear
				}


				glm::fvec3 yawUp = glm::cross(yawRight, projectedToFloorVector);
				if (glm::length(yawUp) > 0.0f) {
					yawUp = glm::normalize(yawUp);
				}
				else {
					yawUp = glm::fvec3(0.0f, 0.0f, 1.0f); // Fallback vector
				}

				point2Position = OffsetLocalPositionFromWorld(point2Position, projectedToFloorVector, yawUp, yawRight, crosshairOffset);

				// Safeguard: Recalculate aiming direction and normalize
				aimingDirection = point2Position - point1Position;
				if (glm::length(aimingDirection) > 0.0f) {
					aimingDirection = glm::normalize(aimingDirection);
				}
				else {
					aimingDirection = glm::fvec3(1.0f, 0.0f, 0.0f); // Fallback vector
				}
			}
			else
			{
				struct {
					glm::fvec3 Location;
				} componentToWorld_params;
				weaponMesh->call_function(L"K2_GetComponentLocation", &componentToWorld_params);
				point1Position = componentToWorld_params.Location;
				aimingDirection = forwardVector_params.ForwardVector;
			}



			if (characterIsInVehicle && cameraMode != 55)
			{
				// Apply new values to memory - This messes up the aiming vector
				//*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = actualPlayerPositionUE.x * 0.01f;;
				//*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -actualPlayerPositionUE.y * 0.01f;;
				//*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = actualPlayerPositionUE.z * 0.01f;;


				////forward vector
				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[0])) = cameraMatrixValues[4];
				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[1])) = cameraMatrixValues[5];
				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[2])) = cameraMatrixValues[6];

				////forward vector
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[0])) = cameraMatrixValues[8];
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[1])) = cameraMatrixValues[9];
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[2])) = cameraMatrixValues[10];
			}
			else
			{
				//Apply new values to memory
				*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[0])) = point1Position.x * 0.01f;
				*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
				*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[2])) = point1Position.z * 0.01f;

				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[0])) = aimingDirection.x;
				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[1])) = -aimingDirection.y;
				*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[2])) = aimingDirection.z;
			}
		}
		else //unarmed
		{
			*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[0])) = actualPlayerPositionUE.x * 0.01f;
			*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[1])) = -actualPlayerPositionUE.y * 0.01f;
			*(reinterpret_cast<float*>(memoryManager.cameraPositionAddresses[2])) = actualPlayerPositionUE.z * 0.01f;

			*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[0])) = cameraMatrixValues[4];
			*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[1])) = cameraMatrixValues[5];
			*(reinterpret_cast<float*>(memoryManager.aimForwardVectorAddresses[2])) = cameraMatrixValues[6];
		}
	}

	void FetchRequiredUObjects()
	{
		playerController = API::get()->get_player_controller(0);
		if (playerController == nullptr)
			return;

		static auto gta_playerActor_c = API::get()->find_uobject<API::UClass>(L"Class /Script/GTABase.GTAPlayerActor");
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		for (auto child : children) {
			if (child->is_a(gta_playerActor_c)) {
				auto playerActor = child;
				playerHead = playerActor->get_property<API::UObject*>(L"head");
				//API::get()->log_info("playerHead : %ls", playerHead->get_full_name().c_str());
				break;
			}
		};
				

		//const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		//const auto& playerCharacter = children.data[3];
		//playerHead = playerCharacter->get_property<API::UObject*>(L"head");
		////API::get()->log_info("%ls", playerHead->get_full_name().c_str());
		UpdateActualWeaponMesh();
	}

	void HandleCutscenes()
	{
		if (playerIsInControl)
			uevr::API::UObjectHook::set_disabled(false);
		else
		{
			//uevr::API::UObjectHook::remove_all_motion_controller_states();
			uevr::API::UObjectHook::set_disabled(true);
		}
	}

	void UpdateActualWeaponMesh()
	{
		static auto gta_weapon_c = API::get()->find_uobject<API::UClass>(L"Class /Script/GTABase.GTAWeapon");
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		/*API::get()->log_info("children = %ls", children.data[0]->get_full_name().c_str());*/
		for (auto child : children) {
			//API::get()->log_info("child = %ls", child->get_full_name().c_str());
			if (child->is_a(gta_weapon_c)) {
				weapon = child;
				weaponMesh = weapon->get_property<API::UObject*>(L"WeaponMesh");
				/*API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());*/
				weaponStaticMesh = weaponMesh->get_property<API::UObject*>(L"StaticMesh");
				/*API::get()->log_info("%ls", weaponStaticMesh->get_full_name().c_str());*/

				if (!characterIsInVehicle || cameraMode == 55)
				{
					auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
					glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);
					UEVR_Quaternionf defaultWeaponRotationQuat_UEVR = { defaultWeaponRotationQuat.w , defaultWeaponRotationQuat.x, defaultWeaponRotationQuat.y, defaultWeaponRotationQuat.z };
					motionState->set_rotation_offset(&defaultWeaponRotationQuat_UEVR);
					motionState->set_hand(1);
					motionState->set_permanent(true);
				}
				if ((/*characterIsGettingInACar || */characterIsInVehicle) && cameraMode != 55)
				{
					uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);
				}
				break;
			}	
		}

		if (weaponMesh == nullptr)
		{
			equippedWeaponIndex = 0;
			return;
		}
		
		std::wstring weaponName = weaponStaticMesh->get_full_name();
		
		// Extract only the weapon name from the full path
		size_t lastDot = weaponName.find_last_of(L'.');
		if (lastDot != std::wstring::npos) {
			weaponName = weaponName.substr(lastDot + 1);
		}
		/*API::get()->log_info("%ls", weaponName.c_str());*/
		// Look up the weapon in the map
		auto it = weaponNameToIndex.find(weaponName);
		if (it != weaponNameToIndex.end()) {
			equippedWeaponIndex = it->second;
		}
		/*API::get()->log_info("%i", equippedWeaponIndex);*/
	}

	glm::fvec3 OffsetLocalPositionFromWorld(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
	{
		// Apply the offsets along the local axes
		glm::fvec3 offset = (forwardVector * offsets.x) + (rightVector * offsets.y) + (upVector * offsets.z);

		// Calculate the new position
		glm::fvec3 pointWorldPosition = worldPosition + offset;

		return pointWorldPosition;
	}

	
	std::string GetDLLDirectory()
	{
		char path[MAX_PATH];
		HMODULE hModule = GetModuleHandleA("VRmod.dll"); // Get handle to the loaded DLL

		if (hModule)
		{
			GetModuleFileNameA(hModule, path, MAX_PATH); // Get full DLL path
			std::string fullPath = path;

			// Remove the DLL filename to get the directory
			size_t pos = fullPath.find_last_of("\\/");
			if (pos != std::string::npos)
			{
				return fullPath.substr(0, pos + 1); // Keep the trailing slash
			}
		}

		return "Unknown";
	}

	std::string GetConfigFilePath()
	{
		std::string fullPath = GetDLLDirectory();

		// Remove "SanAndreas\plugins\VRmod.dll" part, leaving "SanAndreas"
		size_t pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos)
		{
			fullPath = fullPath.substr(0, pos); // Remove "\plugins"
			pos = fullPath.find_last_of("\\/");
			if (pos != std::string::npos)
			{
				fullPath = fullPath.substr(0, pos + 1); // Keep "SanAndreas\"
			}
		}

		return fullPath + "config.txt"; // Append "config.txt"
	}

	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue)
	{
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			API::get()->log_info("%s", "Failed to open config.txt file");
			return defaultValue;
		}

		DWORD bytesRead;
		char buffer[1024];  // Buffer to read the file content
		std::string fileContents;

		// Read the file into memory
		while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0'; // Null terminate the string
			fileContents.append(buffer);
		}
		CloseHandle(hFile);

		// Look for the key in the file contents
		size_t pos = fileContents.find(key);
		if (pos != std::string::npos)
		{
			size_t equalPos = fileContents.find('=', pos);
			if (equalPos != std::string::npos)
			{
				std::string valueStr = fileContents.substr(equalPos + 1);
				try
				{
					return std::stof(valueStr); // Convert the string to float
				}
				catch (const std::invalid_argument&)
				{
					API::get()->log_info("%s","%s", "Error: Invalid float value for key: ", key.c_str());
					return defaultValue;
				}
			}
		}

		return defaultValue;  // Return default if the key is not found
	}

	bool UpdateSettingsIfModified(const std::string& filePath)
	{
		HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			API::get()->log_info("%s", "Failed to open config.txt file");
			return false;
		}

		FILETIME currentWriteTime;
		if (GetFileTime(hFile, NULL, NULL, &currentWriteTime))
		{
			if (CompareFileTime(&lastWriteTime, &currentWriteTime) != 0)
			{
				lastWriteTime = currentWriteTime;  // Update last write time
				UpdateSettings();
				CloseHandle(hFile);
				return true;  // File has been modified
			}
		}

		CloseHandle(hFile);
		return false;  // No change
	}


	struct FQuat {
		float x;
		float y;
		float z;
		float w;
	};

	struct FVector {
		float x, y, z;
	};

	enum class EBoneSpaces
	{
		WorldSpace = 0,
		ComponentSpace = 1,
		EBoneSpaces_MAX = 2,
	};



#pragma pack(push, 1)
	struct FTransform {
		glm::fquat Rotation;
		glm::fvec3 Location;
		uint8_t Padding[4];
		glm::fvec3 Scale3D;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_GetBoneTransformByName
	{
		API::FName BoneName;
		EBoneSpaces BoneSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_GetSocketTransform {
		API::FName InSocketName;
		uint8_t ERelativeTransformSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_K2_AddLocalOffset final
	{
	public:
		glm::fvec3 DeltaLocation;
		bool bSweep;
		uint8_t Pad_D[3];
		uint8_t Padding[0x8C];
		bool bTeleport;
		uint8_t Pad_9D[3];
	};
#pragma pack(pop)

	uintptr_t FindDMAAddy(uintptr_t baseAddress, const std::vector<unsigned int>& offsets) {
		uintptr_t addr = baseAddress;

		for (size_t i = 0; i < offsets.size(); ++i) {
			if (addr == 0) {
				// If at any point the address is invalid, return 0
				API::get()->log_error("%s", "Cant find gunflash socket address");
				return 0;
			}
			// Dereference the pointer
			addr = *reinterpret_cast<uintptr_t*>(addr);

			// Add the offset
			addr += offsets[i];
		}
		return addr;
	}
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASA_VRmod> g_plugin{ new GTASA_VRmod() };