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
	uintptr_t characterIsCrouchingAddress = 0x53DACD2;

	//variables
	float initialCameraYoffset = 0.0f;
	float currentDuckOffset = 0.0f;
	float lastWrittenOffset = 0.0f;
	float maxDuckOffset = 45.0f;  // Maximum offset when crouching
	float duckSpeed = 2.5f;     // Speed per frame adjustment
	bool wasDucking = false;

	float cameraMatrixValues[16] = { 0.0f };
	float newAimingVector[3] = { 0.0f, 0.0f, 0.0f };
	float newCameraPositionVector[3] = { 0.0f, 0.0f, 0.0f };
	float yawOffsetDegrees = 0.0f;
	float xAxisSensitivity = 125.0f;
	float characterHeading = 0.0f;
	float characterHeadingOffset = 0.0f;
	float previousHeading = 0.0f;

	bool characterIsInCar = false;
	int equippedWeaponIndex = 0;
	uevr::API::UObject* playerController;
	uevr::API::UObject* weapon;
	uevr::API::UObject* weaponMesh;

	float pitchOffset = 0.0f;
	float yawOffset = 0.0f;

	float forwardOffset = 0.0f;
	float upOffset = 0.0f;
	float rightOffset = 0.0f;

public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");

		playerController = API::get()->get_player_controller(0);

		UpdateActualWeaponMesh();


		HMODULE hModule = GetModuleHandle(nullptr); // nullptr gets the base module (the game EXE)
		if (hModule == nullptr) {
			API::get()->log_info("Failed to get the base address of the game executable.");
			return;
		}
		baseAddressGameEXE = reinterpret_cast<uintptr_t>(hModule);
		// Convert the base address to a hexadecimal string
		std::ostringstream oss;
		oss << "Base address: 0x" << std::hex << baseAddressGameEXE;
		std::string baseAddressStr = oss.str();
		// Log the base address
		API::get()->log_info(baseAddressStr.c_str());

		hModule = GetModuleHandle(TEXT("UEVRBackend.dll"));
		baseAddressUEVR = reinterpret_cast<uintptr_t>(hModule);
		cameraYoffsetAddressUEVR = FindDMAAddy(baseAddressUEVR + baseCameraYoffsetAddressUEVR, cameraY_UEVROffsets);

		// Adjust camera matrix addresses to account for base address
		for (auto& address : cameraMatrixAddresses) {
			address += baseAddressGameEXE;
		}
		for (auto& address : aimVectorAddresses) {
			address += baseAddressGameEXE;
		}
		for (auto& address : cameraPositionAddresses) {
			address += baseAddressGameEXE;
		}

		oss << "aimVectorAddresse : 0x" << std::hex << aimVectorAddresses[2];
		std::string aimVectorAddresseAddressStr = oss.str();
		API::get()->log_info(aimVectorAddresseAddressStr.c_str());

		//get the flashgun final addresses
		ResolveGunFlashSocketMemoryAddresses();
		oss << "gunFlashSocketRotationAddresses: 0x" << std::hex << gunFlashSocketRotationAddresses[0];
		std::string gunFlashSocketRotationAddressesStr = oss.str();
		API::get()->log_info(gunFlashSocketRotationAddressesStr.c_str());

		equippedWeaponAddress += baseAddressGameEXE;
		characterHeadingAddress += baseAddressGameEXE;
		characterIsInCarAddress += baseAddressGameEXE;
		characterIsCrouchingAddress += baseAddressGameEXE;
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		//UEVR_Vector3f hmdPosition{};
		//UEVR_Quaternionf hmdRotation{};
		//API::get()->param()->vr->get_pose(API::get()->param()->vr->get_hmd_index(), &hmdPosition, &hmdRotation);

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
		float currentHeading = characterIsInCar ? -*(reinterpret_cast<float*>(characterHeadingAddress)) : 0.0f;
		// Calculate heading delta (handles wrap-around at 360 degrees)
		float headingDelta = 0.0f;
		if (characterIsInCar) {
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
		float yawRadians = totalYawDegrees * (M_PI / 180.0f); // More precise than 0.0174533

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
		// Log some matrix values
		// API::get()->log_info("Updated rotation matrix values -> matrix0: %f, matrix1: %f, matrix2: %f", cameraMatrixValues[0], cameraMatrixValues[1], cameraMatrixValues[2]);

		//End of camera matrix yaw movements --------------------------------------------------

		//Place ingame camera at gunflash position
				//Resolve new gunflash sockets address on weapon change
		if (equippedWeaponIndex != *(reinterpret_cast<int*>(equippedWeaponAddress)))
		{
			UpdateActualWeaponMesh();

			//struct {
			//bool absoluteLocation = true;
			//bool absoluteRotation = true;
			//bool absoluteScale = true;
			//} setAbsolute_params;

			//weapon->call_function(L"SetAbsolute", &setAbsolute_params);

			equippedWeaponIndex = *(reinterpret_cast<int*>(equippedWeaponAddress));
		}



		if (weaponMesh != nullptr) {
	/*		SceneComponent_GetSocketTransform socketTransformParams{};
			socketTransformParams.InSocketName = API::FName(L"gunflash");
			socketTransformParams.ERelativeTransformSpace = 0;
			weapon->call_function(L"GetSocketTransform", &socketTransformParams);*/

			struct {
				FTransform Transform;
			} weaponTransform_params;

			struct {
				glm::fvec3 ForwardVector;
			} forwardVector_params;

			struct {
				glm::fvec3 UpVector;
			} upVector_params;

			struct {
				glm::fvec3 RightVector;
			} rightVector_params;

			//SceneComponent_GetSocketQuaternion rotation_params{}; //needs a specific type of parameter with padding or else the game crash. cf sdk dump for padding
			//rotation_params.InSocketName = API::FName(L"gunflash"); // Set the socket name

			//struct {
			//	API::FName InSocketName = API::FName(L"gunflash");
			//	glm::fvec3 Location;
			//} location_params;

			weaponMesh->call_function(L"GetForwardVector", &forwardVector_params);
			weaponMesh->call_function(L"GetUpVector", &upVector_params);
			weaponMesh->call_function(L"GetRightVector", &rightVector_params);
			weapon->call_function(L"GetTransform", &weaponTransform_params);
			//weaponMesh->call_function(L"GetSocketLocation", &location_params);
			//weaponMesh->call_function(L"GetSocketQuaternion", &rotation_params);

			//API::get()->log_info("socket Position : x = %f, y = %f, z = %f", location_params.Location.x, location_params.Location.y, location_params.Location.z);
			//API::get()->log_info("socket Quaternion : x = %f, y = %f, z = %f, w = %f", rotation_params.Rotation.x, rotation_params.Rotation.y, rotation_params.Rotation.z, rotation_params.Rotation.w);
			//glm::vec3 currentsocketRotation = glm::eulerAngles(rotation_params.Rotation);
			//API::get()->log_info("socket Rotation : x = %f, y = %f, z = %f", currentsocketRotation.x, currentsocketRotation.y, currentsocketRotation.z);


			//API::get()->log_info("relative weapon Position : x = %f, y = %f, z = %f", weaponTransform_params.Transform.Location.x, weaponTransform_params.Transform.Location.y, weaponTransform_params.Transform.Location.z);
			//API::get()->log_info("relative weapon Quaternion : x = %f, y = %f, z = %f, w = %f", weaponTransform_params.Transform.Rotation.x, weaponTransform_params.Transform.Rotation.y, weaponTransform_params.Transform.Rotation.z, weaponTransform_params.Transform.Rotation.w);
			//glm::vec3 currentWeaponRotation = glm::eulerAngles(weaponTransform_params.Transform.Rotation);
			//API::get()->log_info("relative weapon Rotation : x = %f, y = %f, z = %f", currentWeaponRotation.x, currentWeaponRotation.y, currentWeaponRotation.z);

			////	API::get()->log_info("new position : %f, %f, %f", location_params.Location.x, location_params.Location.y, location_params.Location.z);
			////API::get()->log_info("new aiming vector : %f, %f, %f, %f", rotation_params.Rotation.x, rotation_params.Rotation.y, rotation_params.Rotation.z, rotation_params.Rotation.w);
			//API::get()->log_info("mesh Forward Vector : x = %f, y = %f, z = %f", forwardVector_params.ForwardVector.x, forwardVector_params.ForwardVector.y, forwardVector_params.ForwardVector.z);


			//glm::fvec3 aimingVector = CalculateAimingVector(rotation_params.Rotation);
			
			glm::fvec3 point1Position = CalculateAimingReferencePoints(weaponTransform_params.Transform.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, 2.82819, -2.52103, 9.92684);
			glm::fvec3 point2Position = CalculateAimingReferencePoints(weaponTransform_params.Transform.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, 21.7272, -3.89487, 12.9088);


			glm::fvec3 aimingDirection = glm::normalize(point2Position - point1Position);


			glm::fvec3 projectedToFloorVector = glm::fvec3(aimingDirection.x, aimingDirection.y, 0.0);
			// Safeguard: Normalize projectedToFloorVector only if valid
			if (glm::length(projectedToFloorVector) > 0.0f) {
				projectedToFloorVector = glm::normalize(projectedToFloorVector);
			} else {
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


			point2Position = CalculateAimingReferencePoints(point2Position, projectedToFloorVector, yawUp, yawRight, 0.0, -1.0, 2.0);

			// Safeguard: Recalculate aiming direction and normalize
			aimingDirection = point2Position - point1Position;
			if (glm::length(aimingDirection) > 0.0f) {
				aimingDirection = glm::normalize(aimingDirection);
			}
			else {
				aimingDirection = glm::fvec3(1.0f, 0.0f, 0.0f); // Fallback vector
			}


			

			if (GetAsyncKeyState(VK_UP)) forwardOffset += 0.1f;
			if (GetAsyncKeyState(VK_DOWN)) forwardOffset -= 0.1f;
			if (GetAsyncKeyState(VK_NUMPAD0)) upOffset -= 0.1f; 
			if (GetAsyncKeyState(VK_NUMPAD1)) upOffset += 0.1f;
			if (GetAsyncKeyState(VK_LEFT)) rightOffset -= 0.1f; 
			if (GetAsyncKeyState(VK_RIGHT)) rightOffset += 0.1f;

			//dla merde, a refaire
			//glm::fvec3 newWorldPosition = CalculateAimingReferencePoints(location_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, forwardOffset, rightOffset, upOffset);



			// Apply new values to memory
			*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = point1Position.x * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = point1Position.z * 0.01f;

			*(reinterpret_cast<float*>(aimVectorAddresses[0])) = aimingDirection.x;
			*(reinterpret_cast<float*>(aimVectorAddresses[1])) = -aimingDirection.y;
			*(reinterpret_cast<float*>(aimVectorAddresses[2])) = aimingDirection.z;

		/*	API::get()->log_info("new position : %f, %f, %f", *(reinterpret_cast<float*>(cameraPositionAddresses[0])), *(reinterpret_cast<float*>(cameraPositionAddresses[1])), *(reinterpret_cast<float*>(cameraPositionAddresses[2])));
			API::get()->log_info("new aiming vector : %f, %f, %f", *(reinterpret_cast<float*>(aimVectorAddresses[0])), *(reinterpret_cast<float*>(aimVectorAddresses[1])), *(reinterpret_cast<float*>(aimVectorAddresses[2])));*/

		}
		//Ducking -----------------------------
		// Check if the player is crouching
		//bool isDucking = *(reinterpret_cast<int*>(characterIsCrouchingAddress)) > 0;
		// 
		//// Log the current crouch state for debugging
		////API::get()->log_info("Is Crouching: %d", isDucking);

		//if (isDucking && !wasDucking) {
		//	// Player just started crouching, capture the initial camera offset
		//	currentDuckOffset = initialCameraYoffset = *(reinterpret_cast<float*>(cameraYoffsetAddressUEVR));
		//}

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

		//// Update the previous crouch state
		//wasDucking = isDucking;

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

	void UpdateActualWeaponMesh()
	{
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		weapon = children.data[4];
		weaponMesh = weapon->get_property<API::UObject*>(L"WeaponMesh");
		API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());
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

	glm::fvec3 ApplyYawAlignedOffset(glm::fvec3 forwardVector,
		glm::fvec3 originalAimDirection,
		float offsetForward,
		float offsetRight,
		float offsetUp)
	{
		// Step 1: Calculate the yaw-only forward vector (ignore pitch)
		glm::fvec3 yawForward = glm::normalize(glm::fvec3(forwardVector.x, 0.0f, forwardVector.z));

		// Step 2: Recompute the yaw-aligned local axes (right and up)
		glm::fvec3 yawRight = glm::normalize(glm::cross(glm::fvec3(0.0f, -1.0f, 0.0f), yawForward));
		glm::fvec3 yawUp = glm::normalize(glm::cross(yawForward, yawRight));

		// Step 3: Calculate the offset in the yaw-aligned space
		glm::fvec3 offset = (yawForward * offsetForward) + (yawRight * offsetRight) + (yawUp * offsetUp);

		// Step 4: Add the offset to the original aiming direction
		glm::fvec3 adjustedAimingDirection = glm::normalize(originalAimDirection + offset);

		return adjustedAimingDirection;
	}

	glm::fvec3 CalculateAimingReferencePoints(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, float offsetForward, float offsetRight, float offsetUp)
	{
		// Apply the offsets along the local axes
		glm::fvec3 offset = (forwardVector * offsetForward) + (rightVector * offsetRight) + (upVector * offsetUp);

		// Calculate the new position
		glm::fvec3 pointWorldPosition = worldPosition + offset;

		return pointWorldPosition;
	}

	glm::fvec3 CalculateAimingVector(glm::fquat gunflashSocketQuaternion)
	{
		//quick 
		//if (GetAsyncKeyState(VK_UP)) pitchOffset += 0.1f;
		//if (GetAsyncKeyState(VK_DOWN)) pitchOffset -= 0.1f;
		//if (GetAsyncKeyState(VK_LEFT)) yawOffset -= 0.1f; // Numpad 4 for left
		//if (GetAsyncKeyState(VK_RIGHT)) yawOffset += 0.1f; // Numpad 6 for right

		//API::get()->log_info("weapon Index : %i", equippedWeaponIndex);
		//API::get()->log_info("pitchOffset : %f", pitchOffset);
		//API::get()->log_info("yawOffset : %f", yawOffset);

		//switch (equippedWeaponIndex)
		//{
		//case 22: // Pistol
		//	pitchOffset = 73.099457f
		//	yawOffset = 2.899999f;
		//	break;
		//case 33: // Rifle
		//	pitchOffset = 5.0f;
		//	yawOffset = 5.0f;
		//	break;
		//default:
		//	pitchOffset = 5.0f;
		//	yawOffset = 5.0f;
		//	break;
		//}

		// Base forward vector (Y is forward in your coordinate system)
		glm::fvec3 baseForward = glm::fvec3(1.0f, 0.0f, 0.0f);
		//glm::fvec3 forward = glm::normalize(gunflashSocketQuaternion * baseForward);
		//glm::fvec3 baseUp = glm::fvec3(0.0f, 0.0f, 1.0f);
		//glm::fvec3 up = glm::normalize(gunflashSocketQuaternion * baseUp);

		// Corrected rotation in radians (from Blender notes)
		glm::vec3 desiredRotationRadians(0.0f, 0.156467f /**0.5*/, 0.072526f /**2*/); // X, Y, Z in radians

		glm::vec3 cameraCrosshairOffset(0.0f, -0.09f /**0.5*/, 0.06f /**2*/); // X, Y, Z in radians

		// Step 1: Convert gunflashSocketQuaternion to Euler angles (local rotation)
		glm::vec3 currentRotationRadians = glm::eulerAngles(gunflashSocketQuaternion);
		API::get()->log_info("currentRotationRadians : x = %f, y = %f, z = %f", currentRotationRadians.x, currentRotationRadians.y, currentRotationRadians.z);

		// Step 2: Calculate the correction needed
		glm::vec3 correctionRadians = currentRotationRadians - desiredRotationRadians - cameraCrosshairOffset;
		API::get()->log_info("correctionRadians : x = %f, y = %f, z = %f", correctionRadians.x, correctionRadians.y, correctionRadians.z);

		// Step 3: Create a quaternion from the corrected angles
		glm::fquat correctedQuaternion = glm::quat(correctionRadians);

		// Calculate the aiming direction
		glm::fvec3 aimingVector = glm::normalize(correctedQuaternion * baseForward);
		//API::get()->log_info("aim vector gunsocket : x = %f, y = %f, z = %f", testaimingVector.x, testaimingVector.y, testaimingVector.z);
		//API::get()->log_info("corrected aim vector gunsocket : x = %f, y = %f, z = %f", aimingVector.x, aimingVector.y, aimingVector.z);

		//// Step 1: Create Quaternions for the pitch and yaw offsets
		//glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchOffset), glm::fvec3(1.0f, 0.0f, 0.0f)); // Pitch around X-axis
		//glm::quat yawQuat = glm::angleAxis(glm::radians(yawOffset), glm::fvec3(0.0f, 1.0f, 0.0f));   // Yaw around Y-axis

		// Step 2: Apply the offsets to the forward vector
		//glm::fvec3 adjustedForward = glm::normalize(yawQuat * pitchQuat * glm::normalize(aimingVector));
		return aimingVector;
	}



	struct FQuat {
    float x;
    float y;
    float z;
	float w;
	};

	struct FVector {
		float x, y, z;

		//// Normalize the vector
		//void normalize() {
		//	float magnitude = std::sqrt(x * x + y * y + z * z);
		//	if (magnitude > 0.0f) {
		//		x /= magnitude;
		//		y /= magnitude;
		//		z /= magnitude;
		//	}
		//}
	};

	//	struct FTransform
	//{
	//	struct FQuat Rotation;
	//	struct FVector Translation;
	//	struct FVector Scale3D;
	//};


#pragma pack(push, 1) // Disable padding
	struct FTransform {
		glm::fquat Rotation;
		glm::fvec3 Location;
		uint8_t Padding[4];
		glm::fvec3 Scale3D;
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

//	enum class ERelativeTransformSpace : uint8
//{
//	RTS_World                                = 0,
//	RTS_Actor                                = 1,
//	RTS_Component                            = 2,
//	RTS_ParentBoneSpace                      = 3,
//	RTS_MAX                                  = 4,
//};

#pragma pack(push, 1) // Disable padding
	struct SceneComponent_GetSocketQuaternion {
		API::FName InSocketName;    // Offset 0x0, 8 bytes
		uint8_t Padding[8];              // Offset 0x8, 8 bytes
		glm::fquat Rotation;          // Offset 0x10, 16 bytes
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

	void printMatrix(UEVR_Matrix4x4f matrix) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				API::get()->log_info("%f ", matrix.m[i * 4 + j]);
			}
			API::get()->log_info("\n");
		}
	}
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASA_VRmod> g_plugin{ new GTASA_VRmod() };