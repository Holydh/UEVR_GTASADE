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

	glm::fvec3 crosshairOffset = {0.0f, -1.0f, 2.0f};

public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
        API::get()->log_info("%s", "VR cpp mod initializing");

        playerController = API::get()->get_player_controller(0);
        UpdateActualWeaponMesh();

        baseAddressGameEXE = GetModuleBaseAddress(nullptr);
        baseAddressUEVR = GetModuleBaseAddress(TEXT("UEVRBackend.dll"));
        cameraYoffsetAddressUEVR = FindDMAAddy(baseAddressUEVR + baseCameraYoffsetAddressUEVR, cameraY_UEVROffsets);

        AdjustAddresses();
        ResolveGunFlashSocketMemoryAddresses();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);

		UpdateCameraMatrix(delta);
		UpdateWeaponMeshOnChange();
		UpdateAimingVectors();
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

	void UpdateAimingVectors()
	{
		if (weaponMesh != nullptr) {

			struct {
				const struct API::FName& InSocketName = API::FName(L"gunflash");
				glm::fvec3 Location;
			} socketLocation_params;

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
			weaponMesh->call_function(L"GetSocketLocation", &socketLocation_params);

			glm::fvec3 point1Offsets = { 0.0f, 0.0f, 0.0f };
			glm::fvec3 point2Offsets = { 0.0f, 0.0f, 0.0f };

			//select weapon offsets
			switch (equippedWeaponIndex)
			{
			case 22: //Pistol
				point1Offsets = {2.82819, -2.52103, 9.92684};
				point2Offsets = {21.7272, -3.89487, 12.9088};
				break;
			case 30: //AK47
				point1Offsets = {2.82819, -2.52103, 9.92684};
				point2Offsets = {21.7272, -3.89487, 12.9088};
				break;

			default:
				point1Offsets = {2.82819, -2.52103, 9.92684};
				point2Offsets = {21.7272, -3.89487, 12.9088};
				break;
			}

			glm::fvec3 point1Position = CalculateAimingReferencePoints(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point1Offsets);
			glm::fvec3 point2Position = CalculateAimingReferencePoints(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point2Offsets);

			glm::fvec3 aimingDirection = glm::normalize(point2Position - point1Position);


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


			// Apply new values to memory
			*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = point1Position.x * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
			*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = point1Position.z * 0.01f;

			*(reinterpret_cast<float*>(aimVectorAddresses[0])) = aimingDirection.x;
			*(reinterpret_cast<float*>(aimVectorAddresses[1])) = -aimingDirection.y;
			*(reinterpret_cast<float*>(aimVectorAddresses[2])) = aimingDirection.z;
		}
	}

	void UpdateWeaponMeshOnChange() {
        if (equippedWeaponIndex != *(reinterpret_cast<int*>(equippedWeaponAddress))) {
            UpdateActualWeaponMesh();
            equippedWeaponIndex = *(reinterpret_cast<int*>(equippedWeaponAddress));
        }
    }

	void UpdateActualWeaponMesh()
	{
		const auto& children = playerController->get_property<API::TArray<API::UObject*>>(L"Children");
		weapon = children.data[4];
		weaponMesh = weapon->get_property<API::UObject*>(L"WeaponMesh");
		API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());
	}

	glm::fvec3 CalculateAimingReferencePoints(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
	{
		// Apply the offsets along the local axes
		glm::fvec3 offset = (forwardVector * offsets.x) + (rightVector * offsets.y) + (upVector * offsets.z);

		// Calculate the new position
		glm::fvec3 pointWorldPosition = worldPosition + offset;

		return pointWorldPosition;
	}

	void UpdateCameraMatrix(float delta)
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

        equippedWeaponAddress += baseAddressGameEXE;
        characterHeadingAddress += baseAddressGameEXE;
        characterIsInCarAddress += baseAddressGameEXE;
        characterIsCrouchingAddress += baseAddressGameEXE;
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