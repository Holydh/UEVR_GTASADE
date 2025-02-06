#include <memory>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "uevr/Plugin.hpp"
#include <windows.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#define _USE_MATH_DEFINES
#include <math.h>



DWORD PID;

using namespace uevr;

#define PLUGIN_LOG_ONCE(...) {\
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }}

class GTASA_VRmod : public uevr::Plugin {
private:

	//Addresses & offsets
	uintptr_t baseAddressGameEXE = 0;
	uintptr_t baseAddressUEVR = 0;

	std::vector<std::pair<uintptr_t, size_t>> matrixInstructionsRotationAddresses = {
		{0x111DE7E, 7}, {0x111DE85, 1},
		{0x111DECC, 7}, {0x111DED3, 1},
		{0x111DED9, 7}, {0x111DEE0, 1},
		{0x111DE5C, 7}, {0x111DE63, 1}, {0x111DE64, 1},
		{0x111DE3B, 7}, {0x111DE42, 1},
		{0x111DE68, 7}, {0x111DE6F, 1},
		{0x111DE75, 7},	{0x111DE7C, 1},	{0x111DE7D, 1},
		{0x111DE8F, 7},	{0x111DE96, 1},	{0x111DE97, 1},
		{0x111DE98, 7},	{0x111DE9F, 1},	{0x111DEA0, 1}
	};

		std::vector<std::pair<uintptr_t, size_t>> matrixInstructionsPositionAddresses = {
		{0x111DEA5, 7},	{0x111DEAC, 1},	{0x111DEAD, 1},	{0x111DF57, 7},	{0x111DF5E, 1},
		{0x111DEB3, 7},	{0x111DEBA, 1},	{0x111DF72, 7},	{0x111DF79, 1},
		{0x111DEBE, 7},	{0x111DEC5, 1},	{0x111DEC6, 1},	{0x111DF8D, 7},	{0x111DF94, 1},
	};
	
	std::vector<std::pair<uintptr_t, size_t>> ingameCameraPositionInstructionsAddresses = {
		{0x1109F20, 3},	{0x1109F23, 1},
		{0x1109F96, 3},	{0x1109F99, 1},
		{0x110A28E, 3},	{0x110A291, 1},
		{0x11255AB, 3},	{0x11255AE, 1},
		{0x11070E2, 3},	{0x11070E5, 1},
		{0x110A3BD, 3},	{0x110A3C0, 1},
		{0x11080C6, 7},
		{0x1109F24, 3},
		{0x1109FBC, 5},
		{0x110A252, 5},	{0x110A257, 1},
		{0x110A2C0, 5},
		{0x11255B4, 5},
		{0x11070FF, 5},
		{0x110A3DD, 5},
		{0x1108165, 5},	{0x110816A, 1},
		{0x1109FA4, 5},
		{0x110A29C, 5},
		{0x11255B3, 5},
		{0x11070F0, 5},
		{0x110A3CB, 5}
	};

	std::vector<std::pair<uintptr_t, size_t>> aimingForwardVectorInstructionsAddresses = {
		{0x11090E8, 5},
		{0xAE0410, 5},
		{0x1109EA5, 5},
		{0x1105AAC, 7},	{0x1105AB3, 1},
		{0x1107E3B, 7},	{0x1107E42, 1},
		{0x1108E75, 5},
		{0xAE0406, 5},
		{0x11090ED, 3},	{0x11090F0, 1},
		{0xAE040B, 5},
		{0x1109EAA, 3},	{0x1109EAD, 1},
		{0x1105AC9, 5},	{0x1105ACE, 1},
		{0x1107E43, 5},	{0x1107E48, 1},
		{0x1108E7A, 3},	{0x1108E7D, 1}
	};

	std::vector<std::pair<uintptr_t, size_t>> aimingUpVectorInstructionsAddresses = {
		{0x1105840, 5}, {0x1105845, 3},
		{0x1105A00, 5}, {0x1105A05, 3},
		{0x1105854, 5}, {0x1105859, 1},
		{0x1105A08, 5}, {0x1105A0D, 1},
	};

	std::vector<std::pair<uintptr_t, size_t>> rocketLauncherAimingVectorInstructionsAddresses = {
		{0x110E71D, 5},	{0x110E722, 1},
		{0x110E70B, 7},	{0x110E712, 1}
	};

	std::vector<std::pair<uintptr_t, size_t>> sniperAimingVectorInstructionsAddresses = {
		{0x110E19E, 5},	{0x110E1A3, 1},
		{0x110E196, 7},	{0x110E19D, 1}
	};

	std::vector<std::pair<uintptr_t, size_t>> carAimingVectorInstructionsAddresses = {
		{0x110BB78, 3},	{0x110BB7B, 1},
		{0x110C5A4, 3},	{0x110C5A7, 1},
		{0x110C59E, 5},	{0x110C5A3, 1},
		{0x110BB68, 5},	{0x110BB6D, 1}
	};

	bool instructionNoped = false;


	uintptr_t cameraMatrixAddresses[16] = {
		0x53E2C00, 0x53E2C04, 0x53E2C08, 0x53E2C0C,
		0x53E2C10, 0x53E2C14, 0x53E2C18, 0x53E2C1C,
		0x53E2C20, 0x53E2C24, 0x53E2C28, 0x53E2C2C,
		0x53E2C30, 0x53E2C34, 0x53E2C38, 0x53E2C3C
	};
	uintptr_t aimForwardVectorAddresses[3] // x, y, z
	{
		0x53E2668, 0x53E266C, 0x53E2670
	};
	uintptr_t aimUpVectorAddresses[3] // x, y, z
	{
		0x53E268C, 0x53E2690, 0x53E2694
	};

	uintptr_t cameraPositionAddresses[3] // x, y, z
	{
		0x53E2674, 0x53E2678, 0x53E267C
	};

	//uintptr_t baseGunFlashSocketRotationAddress = 0x53EB720;
	//std::vector<unsigned int> gunFlashSocketOffsets = { 0x5E0, 0xF0, 0x0, 0x700, 0x1A0, 0x10, 0x190 };
	//uintptr_t gunFlashSocketPositionAddresses[3] = {};
	//uintptr_t gunFlashSocketRotationAddresses[3] = {};
	//uintptr_t baseCameraYoffsetAddressUEVR = 0x08D9E00;
	//std::vector<unsigned int> cameraY_UEVROffsets = { 0x330, 0x8, 0x20, 0x150, 0x0, 0x390, 0x48 };
	//uintptr_t cameraYoffsetAddressUEVR = 0;


	uintptr_t weaponWheelOpenAddress = 0x507C580;

	//borrowed empty addresses
	uintptr_t fpsCamInitializedAddress = 0x53DACC6;
	uintptr_t equippedWeaponAddress = 0x53DACC7;
	uintptr_t characterIsDuckingAddress = 0x53DAD11;
	uintptr_t currentDuckOffsetAddress = 0x53DACDA;
	uintptr_t characterHeadingAddress = 0x53DACF1;
	uintptr_t characterIsInCarAddress = 0x53DACCE;
	uintptr_t characterIsGettingInACarAddress = 0x53DAD01;

	uintptr_t characterIsShootingAddress = 0x53DACE1;


	//variables
	float initialCameraYoffset = 0.0f;
	float currentDuckOffset = 0.0f;
	float lastWrittenOffset = 0.0f;
	float maxDuckOffset = 60.0f;  // Maximum offset when crouching
	float duckSpeed = 2.5f;     // Speed per frame adjustment

	float cameraMatrixValues[16] = { 0.0f };
	float newAimingVector[3] = { 0.0f, 0.0f, 0.0f };
	float newCameraPositionVector[3] = { 0.0f, 0.0f, 0.0f };
	float yawOffsetDegrees = 0.0f;
	float xAxisSensitivity = 125.0f;
	glm::fvec3 actualPlayerPositionUE = { 0.0f, 0.0f, 0.0f };
	float characterHeading = 0.0f;
	glm::mat4 accumulatedJoystickRotation = glm::mat4(1.0f);
	glm::mat4 baseHeadRotation = glm::mat4(1.0f);
	float characterHeadingOffset = 0.0f;
	float previousHeading = 0.0f;

	bool characterIsGettingInACar = false;
	bool characterIsInCar = false;
	bool characterWasInCar = false;
	int equippedWeaponIndex = 0;
	uevr::API::UObject* playerController;
	uevr::API::UObject* playerHead;
	uevr::API::UObject* weapon;
	uevr::API::UObject* weaponMesh;

	float pitchOffset = 0.0f;
	float yawOffset = 0.0f;

	float forwardOffset = 0.0f;
	float upOffset = 0.0f;
	float rightOffset = 0.0f;

	glm::fvec3 crosshairOffset = { 0.0f, -1.0f, 2.0f };
	int boneIndex = 0;

	bool fpsCamWasInitialized = false;
	bool camResetRequested = false;

	glm::fvec3 defaultWeaponRotationEuler = { 0.4f, 0.0f, 0.0f };
	glm::fvec3 defaultWeaponPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilRotationEuler = { 0.0f, 0.0f, 0.0f };
	float recoilPositionRecoverySpeed = 10.0f;
	float recoilRotationRecoverySpeed = 8.0f;

public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");
		baseAddressGameEXE = GetModuleBaseAddress(nullptr);
		baseAddressUEVR = GetModuleBaseAddress(TEXT("UEVRBackend.dll"));
		//cameraYoffsetAddressUEVR = FindDMAAddy(baseAddressUEVR + baseCameraYoffsetAddressUEVR, cameraY_UEVROffsets);
		AdjustAddresses();
		//ResolveGunFlashSocketMemoryAddresses();
		FetchRequiredUObjects();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		FetchRequiredUObjects();		

		bool fpsCamInitialized = *(reinterpret_cast<int*>(fpsCamInitializedAddress)) > 0;
		bool weaponWheelOpen = *(reinterpret_cast<int*>(weaponWheelOpenAddress)) > 30;
		characterIsGettingInACar = *(reinterpret_cast<int*>(characterIsGettingInACarAddress)) > 0;
		characterIsInCar = *(reinterpret_cast<int*>(characterIsInCarAddress)) > 0;
		//API::get()->log_info("weaponWheelOpen = %i", weaponWheelOpen);

		
		if (GetAsyncKeyState(VK_NUMPAD0) && !instructionNoped)
		{
			NopMemory(matrixInstructionsRotationAddresses);
			NopMemory(matrixInstructionsPositionAddresses);
			NopMemory(ingameCameraPositionInstructionsAddresses);
			NopMemory(aimingForwardVectorInstructionsAddresses);
			NopMemory(aimingUpVectorInstructionsAddresses);
			NopMemory(rocketLauncherAimingVectorInstructionsAddresses);
			NopMemory(sniperAimingVectorInstructionsAddresses);
			NopMemory(carAimingVectorInstructionsAddresses);
			instructionNoped = true;
		}
		if (GetAsyncKeyState(VK_NUMPAD1) && instructionNoped)
		{
			RestoreMemory(matrixInstructionsRotationAddresses);
			RestoreMemory(matrixInstructionsPositionAddresses);
			RestoreMemory(ingameCameraPositionInstructionsAddresses);
			RestoreMemory(aimingForwardVectorInstructionsAddresses);
			RestoreMemory(aimingUpVectorInstructionsAddresses);
			RestoreMemory(rocketLauncherAimingVectorInstructionsAddresses);
			RestoreMemory(sniperAimingVectorInstructionsAddresses);
			RestoreMemory(carAimingVectorInstructionsAddresses);
			instructionNoped = false;
		}

		//if (characterIsInCar && !characterWasInCar)
		//{
		//	RestoreMemory(matrixInstructionsRotationAddresses);
		//	RestoreMemory(matrixInstructionsPositionAddresses);
		//	//RestoreMemory(ingameCameraPositionInstructionsAddresses);
		//	//RestoreMemory(aimingForwardVectorInstructionsAddresses);
		//	//RestoreMemory(carAimingVectorInstructionsAddresses);
		//}

		//if (!characterIsInCar && characterWasInCar)
		//{
		//	NopMemory(matrixInstructionsRotationAddresses);
		//	NopMemory(matrixInstructionsPositionAddresses);
		//	//NopMemory(ingameCameraPositionInstructionsAddresses);
		//	//NopMemory(aimingForwardVectorInstructionsAddresses);
		//	//NopMemory(carAimingVectorInstructionsAddresses);
		//}

		if (fpsCamInitialized && !fpsCamWasInitialized)
		{
			camResetRequested = characterIsInCar ? false: true;
			FetchRequiredUObjects();
			API::get()->log_info("fpsCamInitialized = %i", fpsCamInitialized);
		}
		else
			camResetRequested = false;

		fpsCamWasInitialized = fpsCamInitialized;

		if (fpsCamInitialized && !weaponWheelOpen)
		{

			UpdateCameraMatrix(delta, camResetRequested);
			UpdateAimingVectors();
			UpdateWeaponMeshOnChange();
			FixWeaponVisibility();
			WeaponHandling(delta);
			PlayerDucking();
		}

		characterWasInCar = characterIsInCar;
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

	void UpdateCameraMatrix(float delta, bool camResetRequested) {
		// Retrieve the original camera matrix from memory
		//glm::mat4 originalMatrix;
		//for (int i = 0; i < 16; ++i) {
		//    originalMatrix[i / 4][i % 4] = *(reinterpret_cast<float*>(cameraMatrixAddresses[i]));
		//}
		
			// Retrieve the head's forward, up, and right vectors
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

		headRotationMatrix[0] = characterIsInCar ? glm::vec4(-forwardVector_params.ForwardVector.x, forwardVector_params.ForwardVector.y, -forwardVector_params.ForwardVector.z, 0.0f) :
			glm::vec4(forwardVector_params.ForwardVector.x, -forwardVector_params.ForwardVector.y, forwardVector_params.ForwardVector.z, 0.0f); // Negated forward vector
		headRotationMatrix[1] = glm::vec4(-rightVector_params.RightVector.x, rightVector_params.RightVector.y, -rightVector_params.RightVector.z, 0.0f); // Right vector
		headRotationMatrix[2] = glm::vec4(upVector_params.UpVector.x, -upVector_params.UpVector.y, upVector_params.UpVector.z, 0.0f);      // Up vector 

		float joystickYaw = 0.0f;

		if (characterIsInCar && !characterWasInCar)
		{
			accumulatedJoystickRotation = glm::mat4(1.0f);
		}
		if ((!characterIsInCar && characterWasInCar) || (!characterIsInCar && camResetRequested))
		{
			camResetRequested = true;
			baseHeadRotation = headRotationMatrix;
		}

		// Calculate the delta rotation matrix. 
		// Store the base head rotation on the frame the character is out of the car, so the accumulatedJoystickRotation drives it.
		// If the player is in a car, keep the headRotationMatrix drive so the camera follows the car heading.
		glm::mat4 deltaRotationMatrix = characterIsInCar ? glm::inverse(accumulatedJoystickRotation) * headRotationMatrix : glm::inverse(accumulatedJoystickRotation) * baseHeadRotation;

		// Apply joystick input to adjust the local yaw rotation
		const float DEADZONE = 0.1f;
		joystickYaw = camResetRequested ? 180.0f : 0.0f;
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
			*(reinterpret_cast<float*>(cameraMatrixAddresses[i])) = cameraMatrixValues[i];
		}

		//If player loads a save or after a cinematic, reset the camera to the camera heading direction
		if (camResetRequested) {
			*(reinterpret_cast<float*>(cameraMatrixAddresses[0])) = -1;
			*(reinterpret_cast<float*>(cameraMatrixAddresses[5])) = 1;
			*(reinterpret_cast<float*>(cameraMatrixAddresses[10])) = 1;
		}

		// Update the camera position based on the head's socket location
		struct {
			API::FName InSocketName = API::FName(L"head");
			glm::fvec3 Location;
		} socketLocation_params;

		playerHead->call_function(L"GetSocketLocation", &socketLocation_params);

		//fix audio listener unaligned with original game
		glm::fvec3 offsetedPosition = OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, glm::fvec3(49.5, 0.0, 0.0));

		*(reinterpret_cast<float*>(cameraMatrixAddresses[12])) = offsetedPosition.x * 0.01f;
		*(reinterpret_cast<float*>(cameraMatrixAddresses[13])) = -offsetedPosition.y * 0.01f;
		*(reinterpret_cast<float*>(cameraMatrixAddresses[14])) = offsetedPosition.z * 0.01f;
		actualPlayerPositionUE = socketLocation_params.Location;
	}

	void WeaponHandling(float delta)
	{
		if (characterIsInCar && !characterWasInCar)
		{
			UpdateActualWeaponMesh();
		}

		if (!characterIsInCar && characterWasInCar)
		{
			UpdateActualWeaponMesh();
		}

		if (characterIsInCar)
			return;
			

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

		bool isShooting = *(reinterpret_cast<uint8_t*>(characterIsShootingAddress)) > 0;
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

	void PlayerDucking()
	{
		//duck
		//Ducking -----------------------------
		// Check if the player is crouching
		bool isDucking = *(reinterpret_cast<uint8_t*>(characterIsDuckingAddress)) > 0;
		float currentDuckOffset = *(reinterpret_cast<float*>(currentDuckOffsetAddress));

		if (isDucking && currentDuckOffset > -maxDuckOffset) {
			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = glm::fvec3(0.0f, 0.0f, -duckSpeed);
			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);

			*(reinterpret_cast<float*>(currentDuckOffsetAddress)) = currentDuckOffset - duckSpeed;
		}
		else if (isDucking && currentDuckOffset <= -maxDuckOffset)
		{
			*(reinterpret_cast<float*>(currentDuckOffsetAddress)) = -maxDuckOffset;
			/*	*(reinterpret_cast<uint8_t*>(characterWasDuckingAddress)) = 1;*/
		}


		if (!isDucking && currentDuckOffset < 0.0f) {
			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = glm::fvec3(0.0f, 0.0f, duckSpeed);
			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);

			*(reinterpret_cast<float*>(currentDuckOffsetAddress)) = currentDuckOffset + duckSpeed;
		}
		else if (!isDucking && currentDuckOffset >= 0.0f)
		{
			*(reinterpret_cast<float*>(currentDuckOffsetAddress)) = 0.0f;
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
				point2Offsets = { 21.7272, -3.89487, 12.9088 };
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
				point2Offsets = { 12.9468 , -0.996034, 11.293 };
				break;
			case 29: // Mp5
				point1Offsets = { 6.8924, -1.74509 , 19.3761 };
				point2Offsets = { 21.3778 , 0.000536, 21.2535 };
				break;
			case 30: //AK47
				point1Offsets = { 3.8416 , -2.83908, 14.3539 };
				point2Offsets = { 36.3719, 0.193737, 16.1544 };
				break;
			case 31: // M4
				point1Offsets = { 5.85945 , -1.78476 , 15.1271 };
				point2Offsets = { 60.0434  , 2.99539 - 1 , 16.4006 - 1.5 };
				break;
			case 32: // Tec9
				point1Offsets = { 1.1631 , -3.60654, 11.7162 };
				point2Offsets = { 24.9241 , -3.60654, 13.9038 - 1.5 };
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
				point2Offsets = { 86.6453 , 0.429413 , 35.9644 };
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

				//Check if the return value is ok, if not, reset the UObject
	/*			if (glm::length(socketLocation_params.Location - actualPlayerPositionUE) > 200)
				{
					FetchRequiredUObjects();
					API::get()->log_info("bad values retrieved, refetching UObject");
					return;
				}*/

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



			if (characterIsInCar)
			{
				// Apply new values to memory
				*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = actualPlayerPositionUE.x * 0.01f;;
				*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -actualPlayerPositionUE.y * 0.01f;;
				*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = actualPlayerPositionUE.z * 0.01f;;


				//forward vector
				*(reinterpret_cast<float*>(aimForwardVectorAddresses[0])) = cameraMatrixValues[4];
				*(reinterpret_cast<float*>(aimForwardVectorAddresses[1])) = cameraMatrixValues[5];
				*(reinterpret_cast<float*>(aimForwardVectorAddresses[2])) = cameraMatrixValues[6];

				////up vector
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[0])) = -cameraMatrixValues[0];
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[1])) = cameraMatrixValues[1];
				//*(reinterpret_cast<float*>(aimUpVectorAddresses[2])) = cameraMatrixValues[2];
			}
			else
			{
				//Apply new values to memory
				*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = point1Position.x * 0.01f;
				*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
				*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = point1Position.z * 0.01f;

				*(reinterpret_cast<float*>(aimForwardVectorAddresses[0])) = aimingDirection.x;
				*(reinterpret_cast<float*>(aimForwardVectorAddresses[1])) = -aimingDirection.y;
				*(reinterpret_cast<float*>(aimForwardVectorAddresses[2])) = aimingDirection.z;
			}
		}
		else
		{
			API::get()->log_info("%s", "mesh not found");
		}
	}

	void FetchRequiredUObjects()
	{
		playerController = API::get()->get_player_controller(0);
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		const auto& playerCharacter = children.data[3];
		playerHead = playerCharacter->get_property<API::UObject*>(L"head");
		//API::get()->log_info("%ls", playerHead->get_full_name().c_str());
		UpdateActualWeaponMesh();
	}

	void UpdateWeaponMeshOnChange() {
		int actualWeaponIndex = *(reinterpret_cast<int*>(equippedWeaponAddress));
		if (equippedWeaponIndex != actualWeaponIndex) {
			UpdateActualWeaponMesh();
			equippedWeaponIndex = actualWeaponIndex;
		}
	}

	void UpdateActualWeaponMesh()
	{
		static auto gta_weapon_c = API::get()->find_uobject<API::UClass>(L"Class /Script/GTABase.GTAWeapon");
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");

		for (auto child : children) {
			if (child->is_a(gta_weapon_c)) {
				weapon = child;
				weaponMesh = weapon->get_property<API::UObject*>(L"WeaponMesh");
				//API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());
				break;
			}
		}

		if (!characterIsInCar)
		{
			auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
			glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);
			UEVR_Quaternionf defaultWeaponRotationQuat_UEVR = { defaultWeaponRotationQuat.w , defaultWeaponRotationQuat.x, defaultWeaponRotationQuat.y, defaultWeaponRotationQuat.z };
			motionState->set_rotation_offset(&defaultWeaponRotationQuat_UEVR);
			motionState->set_hand(1);
			motionState->set_permanent(true);
		}
		if (characterIsGettingInACar || characterIsInCar)
		{
			uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);
		}
	}

	glm::fvec3 OffsetLocalPositionFromWorld(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
	{
		// Apply the offsets along the local axes
		glm::fvec3 offset = (forwardVector * offsets.x) + (rightVector * offsets.y) + (upVector * offsets.z);

		// Calculate the new position
		glm::fvec3 pointWorldPosition = worldPosition + offset;

		return pointWorldPosition;
	}

	uintptr_t GetModuleBaseAddress(LPCTSTR moduleName) {
		HMODULE hModule = GetModuleHandle(moduleName);
		if (hModule == nullptr) {
			API::get()->log_info("Failed to get the base address of the module.");
			return 0;
		}
		return reinterpret_cast<uintptr_t>(hModule);
	}

	void AdjustAddresses() {
		for (auto& [address, size] : matrixInstructionsRotationAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : matrixInstructionsPositionAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : ingameCameraPositionInstructionsAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : aimingForwardVectorInstructionsAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : aimingUpVectorInstructionsAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : rocketLauncherAimingVectorInstructionsAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : sniperAimingVectorInstructionsAddresses) {address += baseAddressGameEXE;}
		for (auto& [address, size] : carAimingVectorInstructionsAddresses) {address += baseAddressGameEXE;}

		for (auto& address : cameraMatrixAddresses) address += baseAddressGameEXE;
		for (auto& address : aimForwardVectorAddresses) address += baseAddressGameEXE;
		for (auto& address : aimUpVectorAddresses) address += baseAddressGameEXE;
		for (auto& address : cameraPositionAddresses) address += baseAddressGameEXE;

		fpsCamInitializedAddress += baseAddressGameEXE;

		equippedWeaponAddress += baseAddressGameEXE;
		characterHeadingAddress += baseAddressGameEXE;
		characterIsInCarAddress += baseAddressGameEXE;
		characterIsGettingInACarAddress += baseAddressGameEXE;
		characterIsShootingAddress += baseAddressGameEXE;

		characterIsDuckingAddress += baseAddressGameEXE;
		currentDuckOffsetAddress += baseAddressGameEXE;

		weaponWheelOpenAddress += baseAddressGameEXE;

	}

	//void ResolveGunFlashSocketMemoryAddresses()
	//{
	//	gunFlashSocketRotationAddresses[0] = FindDMAAddy(baseAddressGameEXE + baseGunFlashSocketRotationAddress, gunFlashSocketOffsets);
	//	gunFlashSocketRotationAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x4;
	//	gunFlashSocketRotationAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x8;

	//	gunFlashSocketPositionAddresses[0] = gunFlashSocketRotationAddresses[0] + 0x40;
	//	gunFlashSocketPositionAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x44;
	//	gunFlashSocketPositionAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x48;
	//}

	// Structure to store original bytes and their addresses
	struct OriginalByte {
		uintptr_t address;
		uint8_t value;
	};

	// Map to store original bytes for each address
	std::unordered_map<uintptr_t, OriginalByte> originalBytes;

	// Function to NOP a batch of addresses
	void NopMemory(const std::vector<std::pair<uintptr_t, size_t>>& addresses) {
		for (const auto& [address, size] : addresses) {
			DWORD oldProtect;
			VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);

			for (size_t i = 0; i < size; ++i) {
				uintptr_t currentAddr = address + i;
				if (originalBytes.find(currentAddr) == originalBytes.end()) { // Avoid overwriting stored bytes
					originalBytes[currentAddr] = { currentAddr, *reinterpret_cast<uint8_t*>(currentAddr) };
				}
				*reinterpret_cast<uint8_t*>(currentAddr) = 0x90; // Write NOP
			}

			VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
		}
	}

	// Function to restore original bytes for a batch of addresses
	void RestoreMemory(const std::vector<std::pair<uintptr_t, size_t>>& addresses) {
		for (const auto& [address, size] : addresses) {
			DWORD oldProtect;
			VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);

			for (size_t i = 0; i < size; ++i) {
				uintptr_t currentAddr = address + i;
				if (originalBytes.find(currentAddr) != originalBytes.end()) {
					*reinterpret_cast<uint8_t*>(currentAddr) = originalBytes[currentAddr].value; // Restore original byte
				}
			}

			VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
		}
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