#include <memory>

#include "glm/glm.hpp"
#include <glm/gtc/quaternion.hpp>
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
	uevr::API::UObject* fQuatStruct;


public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");

		playerController = API::get()->get_player_controller(0);
		fQuatStruct = API::get()->find_uobject(L"ScriptStruct /Script/CoreUObject.Quat");

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
			equippedWeaponIndex = *(reinterpret_cast<int*>(equippedWeaponAddress));
		}

		

		if (weapon != nullptr) {
			struct {
				const struct API::FName& InSocketName = API::FName(L"gunflash");
				glm::fvec3 location;
			} location_params;

			SceneComponent_GetSocketQuaternion rotation_params{}; //needs a specific type of parameter with padding or else the game crash. cf sdk dump for padding
			rotation_params.InSocketName = API::FName(L"gunflash"); // Set the socket name

			weapon->call_function(L"GetSocketLocation", &location_params);
			weapon->call_function(L"GetSocketQuaternion", &rotation_params);


			//API::get()->log_info("new position : %f, %f, %f",
			//	location_params.location.x, location_params.location.y, location_params.location.z);
			API::get()->log_info("new quaternion : %lf, %lf, %lf, %lf", 
				rotation_params.ReturnValue.w, rotation_params.ReturnValue.x, rotation_params.ReturnValue.y, rotation_params.ReturnValue.z);


			glm::fvec3 aimingVector = CalculateAimingVector(rotation_params.ReturnValue);

			// Apply new values to memory
			*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = location_params.location.x * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -location_params.location.y * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = location_params.location.z * 0.01f;

			/*		*(reinterpret_cast<float*>(aimVectorAddresses[0])) = aimingVector.x;
					*(reinterpret_cast<float*>(aimVectorAddresses[1])) = aimingVector.y;
					*(reinterpret_cast<float*>(aimVectorAddresses[2])) = aimingVector.z;*/

					//API::get()->log_info("new aiming vector : %f, %f, %f", *(reinterpret_cast<float*>(cameraPositionAddresses[0])), *(reinterpret_cast<float*>(cameraPositionAddresses[1])), *(reinterpret_cast<float*>(cameraPositionAddresses[2])));
					//API::get()->log_info("new aiming vector : %f, %f, %f", *(reinterpret_cast<float*>(aimVectorAddresses[0])), *(reinterpret_cast<float*>(aimVectorAddresses[1])), *(reinterpret_cast<float*>(aimVectorAddresses[2])));
		}
		//Ducking -----------------------------
		// Check if the player is crouching
		bool isDucking = *(reinterpret_cast<int*>(characterIsCrouchingAddress)) > 0;

		// Log the current crouch state for debugging
		//API::get()->log_info("Is Crouching: %d", isDucking);

		if (isDucking && !wasDucking) {
			// Player just started crouching, capture the initial camera offset
			currentDuckOffset = initialCameraYoffset = *(reinterpret_cast<float*>(cameraYoffsetAddressUEVR));
		}

		// Update the current offset
		if (isDucking) {
			// Smoothly increase the offset toward -maxDuckOffset
			if (currentDuckOffset > initialCameraYoffset - maxDuckOffset) {
				currentDuckOffset -= duckSpeed;
			}
			else {
				currentDuckOffset = initialCameraYoffset - maxDuckOffset;
			}
		}
		else {
			// Smoothly reset the offset back to 0
			if (currentDuckOffset < initialCameraYoffset) {
				currentDuckOffset += duckSpeed;
			}
			else {
				currentDuckOffset = initialCameraYoffset; // Ensure it doesn't overshoot
			}
		}

		if (currentDuckOffset != lastWrittenOffset) {
			*(reinterpret_cast<float*>(cameraYoffsetAddressUEVR)) = currentDuckOffset;
			lastWrittenOffset = currentDuckOffset;
		}

		// Update the previous crouch state
		wasDucking = isDucking;

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
		weapon = children.data[4]->get_property<API::UObject*>(L"WeaponMesh");
		API::get()->log_info("%ls", weapon->get_full_name().c_str());
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

	glm::fvec3 CalculateAimingVector(glm::fvec3 gunflashSocketRotation)
	{
		float pitch = gunflashSocketRotation.x;
		float yaw = gunflashSocketRotation.y;

		switch (equippedWeaponIndex) //aim offsets per weapons
		{
		case 22: //Pistol
			pitch += 4.0f;
			yaw += 82.0f;
			break;
		case 33: //Rifle
			pitch += 5.0f;
			yaw += 95.0f;
			break;
		default:
			pitch += 5.0f;
			yaw += 95.0f;
			break;
		}

		pitch = glm::radians(pitch);
		yaw = glm::radians(yaw);;

		// Compute forward vector
		glm::fvec3 aimingVector;
		aimingVector.x = std::cos(pitch) * std::sin(yaw); // Left/Right
		aimingVector.y = std::cos(pitch) * std::cos(yaw); // Forward/Backward
		aimingVector.z = std::sin(pitch);                // Up/Down

		aimingVector = glm::normalize(aimingVector);
		return aimingVector;
	}



	//struct FQuat {
 //   double W;
 //   double X;
 //   double Y;
 //   double Z;
	//};

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
		struct SceneComponent_GetSocketQuaternion {
			API::FName InSocketName;    // Offset 0x0, 8 bytes
			uint8_t Padding[8];              // Offset 0x8, 8 bytes
			glm::fquat ReturnValue;          // Offset 0x10, 16 bytes
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