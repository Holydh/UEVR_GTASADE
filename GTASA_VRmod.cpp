#include <memory>

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "uevr/Plugin.hpp"
#include <windows.h>
#include <iostream>
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

	uintptr_t fpsCamInitializedAddress = 0x53DACD3;

	uintptr_t cameraMatrixAddresses[16] = {
		0x53E2C00, 0x53E2C04, 0x53E2C08, 0x53E2C0C,
		0x53E2C10, 0x53E2C14, 0x53E2C18, 0x53E2C1C,
		0x53E2C20, 0x53E2C24, 0x53E2C28, 0x53E2C2C,
		0x53E2C30, 0x53E2C34, 0x53E2C38, 0x53E2C3C
	};
	uintptr_t aimVectorAddresses[3]
	{
		0x53E2668, 0x53E266C, 0x53E2670
	};
	uintptr_t cameraPositionAddresses[3]
	{
		0x53E2674, 0x53E2678, 0x53E267C
	};
	uintptr_t baseGunFlashSocketRotationAddress = 0x53EB720;
	std::vector<unsigned int> gunFlashSocketOffsets = { 0x5E0, 0xF0, 0x0, 0x700, 0x1A0, 0x10, 0x190 };
	uintptr_t gunFlashSocketPositionAddresses[3] = {};
	uintptr_t gunFlashSocketRotationAddresses[3] = {};
	uintptr_t baseCameraYoffsetAddressUEVR = 0x08D9E00;
	std::vector<unsigned int> cameraY_UEVROffsets = { 0x330, 0x8, 0x20, 0x150, 0x0, 0x390, 0x48 };
	uintptr_t cameraYoffsetAddressUEVR = 0;
	uintptr_t equippedWeaponAddress = 0x53DACC6;
	uintptr_t characterHeadingAddress = 0x53DACCA;
	uintptr_t characterIsInCarAddress = 0x53DACCE;
	uintptr_t characterIsDuckingAddress = 0x53DACD2;

	//variables
	float initialCameraYoffset = 0.0f;
	float currentDuckOffset = 0.0f;
	float lastWrittenOffset = 0.0f;
	float maxDuckOffset = 60.0f;  // Maximum offset when crouching
	float duckSpeed = 2.5f;     // Speed per frame adjustment
	bool wasDucking = false;

	float cameraMatrixValues[16] = { 0.0f };
	float newAimingVector[3] = { 0.0f, 0.0f, 0.0f };
	float newCameraPositionVector[3] = { 0.0f, 0.0f, 0.0f };
	float yawOffsetDegrees = 0.0f;
	float xAxisSensitivity = 125.0f;
	glm::fvec3 actualPlayerPositionUE =  { 0.0f, 0.0f, 0.0f };
	float characterHeading = 0.0f;
	float characterHeadingOffset = 0.0f;
	float previousHeading = 0.0f;

	bool characterIsInCar = false;
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

	glm::fvec3 crosshairOffset = {0.0f, -1.0f, 2.0f};
	int boneIndex = 0;

	bool fpsCamWasInitialized = false;
	bool camResetRequested = false;

public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
        API::get()->log_info("%s", "VR cpp mod initializing");
        baseAddressGameEXE = GetModuleBaseAddress(nullptr);
        baseAddressUEVR = GetModuleBaseAddress(TEXT("UEVRBackend.dll"));
        cameraYoffsetAddressUEVR = FindDMAAddy(baseAddressUEVR + baseCameraYoffsetAddressUEVR, cameraY_UEVROffsets);

        AdjustAddresses();
        ResolveGunFlashSocketMemoryAddresses();
		FetchRequiredUObjects();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		
		bool fpsCamInitialized = *(reinterpret_cast<int*>(fpsCamInitializedAddress)) > 0;
		
		if (fpsCamInitialized && !fpsCamWasInitialized)
		{
			camResetRequested = true;
			FetchRequiredUObjects();
			API::get()->log_info("fpsCamInitialized = %i", fpsCamInitialized);
		}
		else
			camResetRequested = false;

		fpsCamWasInitialized = fpsCamInitialized;

		if (fpsCamInitialized)
		{
			UpdateCameraMatrix(delta, camResetRequested);
			UpdateWeaponMeshOnChange();
			UpdateAimingVectors();
			FixWeaponVisibility();
			HandlePlayerDuck();
		}
	}

	void on_post_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);
		// Matrix camera position
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

	void UpdateCameraMatrix(float delta, bool camResetRequested)
	{
		float originalMatrix[16];
		for (int i = 0; i < 16; ++i) {
			originalMatrix[i] = *(reinterpret_cast<float*>(cameraMatrixAddresses[i]));
			cameraMatrixValues[i] = originalMatrix[i];
		}

		/*	API::get()->log_info("Original Matrix:\n");
			printMatrix(originalMatrix);*/

			// Camera Matrix Yaw movements ---------------------------------------------------
		UEVR_Vector2f rightJoystick{};
		API::get()->param()->vr->get_joystick_axis(API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);
		//API::get()->log_info("Joystick Input X: %f", rightJoystick.x);
		characterIsInCar = *(reinterpret_cast<int*>(characterIsInCarAddress)) > 0;

		
		//When player is in car, the heading will also make the camera turn so the camera can stay aligned with the car
		float currentHeading = characterIsInCar ? -*(reinterpret_cast<float*>(characterHeadingAddress)) : 0.0f;

		// Calculate heading delta (handles wrap-around at 360 degrees)
		float headingDelta = 0.0f;
		if (characterIsInCar || camResetRequested) {
			headingDelta = currentHeading - previousHeading;
			// Handle wrap-around cases
			if (headingDelta > 180.0f) headingDelta -= 360.0f;
			if (headingDelta < -180.0f) headingDelta += 360.0f;
		}
		previousHeading = currentHeading;

		const float DEADZONE = 0.1f;
		float joystickYaw = 0.0f;
		if (abs(rightJoystick.x) > DEADZONE) {
			joystickYaw = rightJoystick.x * delta * xAxisSensitivity;
		}
		// Combine joystick and heading changes
		float totalYawDegrees = joystickYaw + headingDelta;
		
		float yawRadians = totalYawDegrees * (M_PI / 180.0f);

		// Create a yaw rotation matrix
		float cosYaw = std::cos(yawRadians);
		float sinYaw = std::sin(yawRadians);
		float yawRotationMatrix[16] = {
			cosYaw, -sinYaw, 0.0f, 0.0f,
			sinYaw,   cosYaw, 0.0f,   0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f,   0.0f, 0.0f,   1.0f
		};

		// Multiply the original matrix by the yaw rotation matrix
		float modifiedMatrix[16];
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				modifiedMatrix[i * 4 + j] = 0.0f;
				for (int k = 0; k < 4; ++k) {
					modifiedMatrix[i * 4 + j] += originalMatrix[i * 4 + k] * yawRotationMatrix[k * 4 + j];
				}
			}
		}

		// Copy the modified matrix back to cameraMatrixValues
		for (int i = 0; i < 16; ++i) {
			cameraMatrixValues[i] = modifiedMatrix[i];
		}
		//API::get()->log_info("Modified Matrix:\n");
		//printMatrix(cameraMatrixValues);

		for (int i = 0; i < 12; ++i) {
			*(reinterpret_cast<float*>(cameraMatrixAddresses[i])) = cameraMatrixValues[i];
		}

		// If player loads a save or after a cinematic, reset the camera to the camera heading direction
		if (camResetRequested)
		{
			*(reinterpret_cast<float*>(cameraMatrixAddresses[0])) = -1;
			*(reinterpret_cast<float*>(cameraMatrixAddresses[5])) = 1;
			*(reinterpret_cast<float*>(cameraMatrixAddresses[10])) = 1;
			//currentHeading = -*(reinterpret_cast<float*>(characterHeadingAddress));
			//API::get()->log_error("Here");
		}

		// Log some matrix values
		// API::get()->log_info("Updated rotation matrix values -> matrix0: %f, matrix1: %f, matrix2: %f", cameraMatrixValues[0], cameraMatrixValues[1], cameraMatrixValues[2]);

		//End of camera matrix yaw movements --------------------------------------------------

		// camera matrix position - affects Unreal's objects LOD levels around the player
		struct {
			API::FName InSocketName = API::FName(L"head");
			glm::fvec3 Location;
		} socketLocation_params;

		playerHead->call_function(L"GetSocketLocation", &socketLocation_params);

		//API::get()->log_info("Head Location : x = %f, y = %f, z = %f ", socketLocation_params.Location.x, socketLocation_params.Location.y, socketLocation_params.Location.z);

		*(reinterpret_cast<float*>(cameraMatrixAddresses[12])) = socketLocation_params.Location.x * 0.01f;
		*(reinterpret_cast<float*>(cameraMatrixAddresses[13])) = - socketLocation_params.Location.y * 0.01f;
		*(reinterpret_cast<float*>(cameraMatrixAddresses[14])) = socketLocation_params.Location.z * 0.01f;

		actualPlayerPositionUE = socketLocation_params.Location;
	}

	void HandlePlayerDuck()
	{
		//duck
		//Ducking -----------------------------
		// Check if the player is crouching
		bool isDucking = *(reinterpret_cast<uint8_t*>(characterIsDuckingAddress)) > 0;
		 
		API::get()->log_info("Is Ducking: %d, wasDucking: %d", isDucking, wasDucking);

		if (isDucking && !wasDucking) {

			struct {
				glm::fvec3 Location;
			}getComponentLocation_params;
			playerHead->call_function(L"K2_GetComponentLocation", &getComponentLocation_params);


			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = -glm::fvec3(0.0f, 0.0f, maxDuckOffset);

			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);
		}

		if (!isDucking && wasDucking) {
			struct {
				glm::fvec3 Location;
			}getComponentLocation_params;
			playerHead->call_function(L"K2_GetComponentLocation", &getComponentLocation_params);

			SceneComponent_K2_AddLocalOffset addLocalOffset_params{};
			addLocalOffset_params.bSweep = false;
			addLocalOffset_params.bTeleport = true;
			addLocalOffset_params.DeltaLocation = glm::fvec3(0.0f, 0.0f, maxDuckOffset);

			playerHead->call_function(L"K2_AddLocalOffset", &addLocalOffset_params);
		}

		// Update the previous crouch state
		wasDucking = isDucking;

		//// Update the current offset
		//if (isDucking) {
		//	// Smoothly increase the offset toward -maxDuckOffset
		//	if (currentDuckOffset > initialCameraYoffset - maxDuckOffset) {
		//		currentDuckOffset -= duckSpeed;
		//	}
		//	else {
		//		currentDuckOffset = initialCameraYoffset - maxDuckOffset;
		//	}
		//}
		//else {
		//	// Smoothly reset the offset back to 0
		//	if (currentDuckOffset < initialCameraYoffset) {
		//		currentDuckOffset += duckSpeed;
		//	}
		//	else {
		//		currentDuckOffset = initialCameraYoffset; // Ensure it doesn't overshoot
		//	}
		//}

		//if (currentDuckOffset != lastWrittenOffset) {
		//	*(reinterpret_cast<float*>(cameraYoffsetAddressUEVR)) = currentDuckOffset;
		//	lastWrittenOffset = currentDuckOffset;
		//}


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
				point1Offsets = {2.82819, -2.52103, 9.92684};
				point2Offsets = {21.7272, -3.89487, 12.9088};
				break;
			case 23: // PistolSilenced
				point1Offsets = { 2.80735, -2.52308, 9.9193 };
				point2Offsets = { 17.3316, -3.5591 +0.1, 12.2129 +0.6 };
				break;
			case 24: // DesertEagle
				point1Offsets = { 7.06492 , -2.25853 , 11.9386 + 0.5 };
				point2Offsets = { 33.5914, -1.46079 - 0.5, 11.9439 - 0.5};
				break;
			case 25: // Shotgun
				point1Offsets = { 31.3429, -0.670153, 15.2663 };
				point2Offsets = { 73.6795 , 4.2357 -1 , 22.2237 -2 };
				break;
			case 26: // Sawnoff
				point1Offsets = { 21.2896, -2.13098 , 13.0224 };
				point2Offsets = { 55.8867 , -2.10406 -1, 16.3934 -2  };
				break;
			case 27: // Spas12
				point1Offsets = { 51.9659 , 1.30133, 19.5475 };
				point2Offsets = { 70.459 , 3.20646 , 22.5404  };
				break;
			case 28: // MicroUzi
				point1Offsets = { -0.267532, -2.19868 , 10.2951  };
				point2Offsets = { 12.9468 , -0.996034, 11.293  };
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
				point1Offsets = { 5.85945 , -1.78476 , 15.1271   };
				point2Offsets = { 60.0434  , 2.99539-1 , 16.4006-1.5  };
				break;
			case 32: // Tec9
				point1Offsets = { 1.1631 , -3.60654, 11.7162  };
				point2Offsets = { 24.9241 , -3.60654, 13.9038-1.5 };
				break;
			case 33: //Rifle cuntgun
				point1Offsets = { 7.92837 , -3.48911 , 11.4936 };
				point2Offsets = { 71.2598, 4.09339 - 0.75, 20.9391 - 1.5 }; //additional offsets required. Crosshair offset is probably different for that weapon
				break;
			case 34: // Sniper
				point1Offsets = { 3.00373 , -3.05089 , 10.5162  };
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
				point2Offsets = { 76.7885, 0.537026 , 31.6837  };
				break;
			case 38: // Minigun
				point1Offsets = { 48.1025 , -2.9978 , 14.3878  };
				point2Offsets = { 86.6453 , 0.429413 , 35.9644  };
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
				point1Offsets = {0.0f , 0.0f, 0.0f};
				point2Offsets = {0.0f , 0.0f, 0.0f};
				socketAvailable = false;
				break;
			}

			glm::fvec3 point1Position = {0.0f , 0.0f, 0.0f};
			glm::fvec3 point2Position = {0.0f , 0.0f, 0.0f};
			glm::fvec3 aimingDirection = {0.0f , 0.0f, 0.0f};

			if (socketAvailable)
			{
				struct {
					const struct API::FName& InSocketName = API::FName(L"gunflash");
					glm::fvec3 Location;
				} socketLocation_params;

				weaponMesh->call_function(L"GetSocketLocation", &socketLocation_params);
				//API::get()->log_info("ForwardVector : x = %f, y = %f, z = %f", forwardVector_params.ForwardVector.x,  forwardVector_params.ForwardVector.y, forwardVector_params.ForwardVector.z);

				//Check if the return value is ok, if not, reset the UObject
				if (glm::length(socketLocation_params.Location - actualPlayerPositionUE) > 200)
				{
					FetchRequiredUObjects();
					API::get()->log_info("bad values retrieved, refetching UObject");
					return;
				}

				point1Position = CalculateAimingReferencePoints(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point1Offsets);
				point2Position = CalculateAimingReferencePoints(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point2Offsets);

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

				point2Position = CalculateAimingReferencePoints(point2Position, projectedToFloorVector, yawUp, yawRight, crosshairOffset);

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
			

			

			// Apply new values to memory
			*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = point1Position.x * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = point1Position.z * 0.01f;


			*(reinterpret_cast<float*>(aimVectorAddresses[0])) = aimingDirection.x;
			*(reinterpret_cast<float*>(aimVectorAddresses[1])) = -aimingDirection.y;
			*(reinterpret_cast<float*>(aimVectorAddresses[2])) = aimingDirection.z;
		}
		else
		{
			API::get()->log_info("%s", "mesh not found");
		}
	}

	void UpdateWeaponMeshOnChange() {
		int actualWeaponIndex = *(reinterpret_cast<int*>(equippedWeaponAddress));
        if (equippedWeaponIndex != actualWeaponIndex) {
            UpdateActualWeaponMesh();
            equippedWeaponIndex = actualWeaponIndex;
        }
    }

	void FetchRequiredUObjects()
	{
		playerController = API::get()->get_player_controller(0);
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		const auto& playerCharacter = children.data[3];
		playerHead = playerCharacter->get_property<API::UObject*>(L"head");
		API::get()->log_info("%ls", playerHead->get_full_name().c_str());
        UpdateActualWeaponMesh();
	}

	void UpdateActualWeaponMesh()
	{
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		weapon = children.data[4];
		weaponMesh = weapon->get_property<API::UObject*>(L"WeaponMesh");
		API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());
		
		//struct {
		//	bool absoluteLocation = true;
		//	bool absoluteRotation = true;
		//	bool absoluteScale = true;
		//} setAbsolute_params;
		//weaponMesh->call_function(L"SetAbsolute", &setAbsolute_params);

		//struct {
		//	uint32_t materialIndex = -1;
		//	float flashAmount = 0.00001f;
		//} setFlashAmount_params;
		//weapon->call_function(L"SetEffect", &setFlashAmount_params);
	}

	glm::fvec3 CalculateAimingReferencePoints(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
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
        for (auto& address : cameraMatrixAddresses) address += baseAddressGameEXE;
        for (auto& address : aimVectorAddresses) address += baseAddressGameEXE;
        for (auto& address : cameraPositionAddresses) address += baseAddressGameEXE;

		fpsCamInitializedAddress += baseAddressGameEXE;
        equippedWeaponAddress += baseAddressGameEXE;
        characterHeadingAddress += baseAddressGameEXE;
        characterIsInCarAddress += baseAddressGameEXE;
        characterIsDuckingAddress += baseAddressGameEXE;
    }

	void ResolveGunFlashSocketMemoryAddresses()
	{
		gunFlashSocketRotationAddresses[0] = FindDMAAddy(baseAddressGameEXE + baseGunFlashSocketRotationAddress, gunFlashSocketOffsets);
		gunFlashSocketRotationAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x4;
		gunFlashSocketRotationAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x8;

		gunFlashSocketPositionAddresses[0] = gunFlashSocketRotationAddresses[0] + 0x40;
		gunFlashSocketPositionAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x44;
		gunFlashSocketPositionAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x48;
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
	WorldSpace                               = 0,
	ComponentSpace                           = 1,
	EBoneSpaces_MAX                          = 2,
};



#pragma pack(push, 1) // Disable padding
	struct FTransform {
		glm::fquat Rotation;
		glm::fvec3 Location;
		uint8_t Padding[4];
		glm::fvec3 Scale3D;
	};
#pragma pack(pop)

#pragma pack(push, 1) // Disable padding
	struct SceneComponent_GetBoneTransformByName
	{
		API::FName BoneName;
		EBoneSpaces BoneSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1) // Disable padding
	struct SceneComponent_GetSocketTransform {
		API::FName InSocketName;
		uint8_t ERelativeTransformSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1) // Disable padding
	struct SceneComponent_K2_AddLocalOffset final
{
public:
	glm::fvec3 DeltaLocation;                                     // 0x0000(0x000C)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	bool bSweep;                                            // 0x000C(0x0001)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8_t Pad_D[3];                                        // 0x000D(0x0003)(Fixing Size After Last Property [ Dumper-7 ])
	uint8_t Padding[0x8C];                                  // 0x0010(0x008C)(Parm, OutParm, IsPlainOldData, NoDestructor, ContainsInstancedReference, NativeAccessSpecifierPublic)
	bool bTeleport;                                         // 0x009C(0x0001)(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	uint8_t Pad_9D[3];                                       // 0x009D(0x0003)(Fixing Struct Size After Last Property [ Dumper-7 ])
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