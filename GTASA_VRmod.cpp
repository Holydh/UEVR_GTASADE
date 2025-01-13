#include <memory>

#include "uevr/Plugin.hpp"
#include <windows.h>
#include <iostream>

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
	uintptr_t baseAddress = 0;

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

	uintptr_t baseGunFlashSocketRotationAddress = 0x5415348;
	std::vector<unsigned int> gunFlashSocketOffsets = { 0x30, 0x700, 0x1A0, 0x10, 0x190 };

	uintptr_t gunFlashSocketPositionAddresses[3] = {
	};
	uintptr_t gunFlashSocketRotationAddresses[3] = {
		
	};


	float cameraMatrixValues[16] = { 0.0f };
	float yawOffsetDegrees = 0.0f;

	float xAxisSensitivity = 125.0f;

public:
	GTASA_VRmod() = default;

	void on_dllmain() override {}

	void on_initialize() override {
		// Logs to the appdata UnrealVRMod log.txt file
		API::get()->log_error("%s %s", "Hello", "error");
		API::get()->log_warn("%s %s", "Hello", "warning");
		API::get()->log_info("%s %s", "Hello", "info");

		HMODULE hModule = GetModuleHandle(nullptr); // nullptr gets the base module (the game EXE)

		if (hModule == nullptr) {
			API::get()->log_info("Failed to get the base address of the game executable.");
			return;
		}

		baseAddress = reinterpret_cast<uintptr_t>(hModule);

		// Convert the base address to a hexadecimal string
		std::ostringstream oss;
		oss << "Base address: 0x" << std::hex << baseAddress;
		std::string baseAddressStr = oss.str();

		// Log the base address
		API::get()->log_info(baseAddressStr.c_str());

		// Adjust camera matrix addresses to account for base address
		for (auto& address : cameraMatrixAddresses) {
			address += baseAddress;
		}

		for (auto& address : aimVectorAddresses) {
			address += baseAddress;
		}

		for (auto& address : cameraPositionAddresses) {
			address += baseAddress;
		}

		oss << "aimVectorAddresse : 0x" << std::hex << aimVectorAddresses[2];
		std::string aimVectorAddresseAddressStr = oss.str();

		// Log the last address
		API::get()->log_info(aimVectorAddresseAddressStr.c_str());



		//get the flashgun addresses

		gunFlashSocketRotationAddresses[0] = FindDMAAddy(baseAddress + baseGunFlashSocketRotationAddress, gunFlashSocketOffsets);
		gunFlashSocketRotationAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x4;
		gunFlashSocketRotationAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x8;

		gunFlashSocketPositionAddresses[0] = gunFlashSocketRotationAddresses[0] + 0x40;
		gunFlashSocketPositionAddresses[1] = gunFlashSocketRotationAddresses[0] + 0x44;
		gunFlashSocketPositionAddresses[2] = gunFlashSocketRotationAddresses[0] + 0x48;

		oss << "gunFlashSocketRotationAddresses: 0x" << std::hex << gunFlashSocketRotationAddresses[0];
		std::string gunFlashSocketRotationAddressesStr = oss.str();

		// Log the base address
		API::get()->log_info(gunFlashSocketRotationAddressesStr.c_str());
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

		// Camera Matrix Yaw movements :
		UEVR_Vector2f rightJoystick{};
		API::get()->param()->vr->get_joystick_axis(API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);

		//API::get()->log_info("Joystick Input X: %f", rightJoystick.x);

		const float DEADZONE = 0.1f;
		if (abs(rightJoystick.x) > DEADZONE) {
			yawOffsetDegrees = rightJoystick.x * delta * xAxisSensitivity;
		}
		else
			yawOffsetDegrees = 0.0f;

		float yawOffsetRadians = yawOffsetDegrees * 0.0174533f;
		float cosYaw = std::cos(yawOffsetRadians);
		float sinYaw = std::sin(yawOffsetRadians);

		// Create a yaw rotation matrix
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

		// To uncomment to reapply matrix
		for (int i = 0; i < 12; ++i) {
			*(reinterpret_cast<float*>(cameraMatrixAddresses[i])) = cameraMatrixValues[i];
		}
		

		//End of camera matrix yaw movements
		
		//Place ingame camera at shoot position
		for (int i = 0; i < 3; ++i) {
			float newPos = *(reinterpret_cast<float*>(gunFlashSocketPositionAddresses[i])) * 0.01f;
			API::get()->log_info("cameraPositions : %f", newPos);
			if (i == 1)
				*(reinterpret_cast<float*>(cameraPositionAddresses[i])) = -newPos;
			else
				*(reinterpret_cast<float*>(cameraPositionAddresses[i])) = newPos;
		}
		
		//*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = 2497.5f;
		//*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -1708.590942f;
		//*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = 1015.262451;


	/*	for (int i = 0; i < 3; ++i) {
			*(reinterpret_cast<float*>(aimVectorAddresses[i])) = *(reinterpret_cast<float*>(gunFlashSocketRotationAddresses[0]));
		}*/

	}

	void on_post_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);


	}

	

	struct Vec3 {
		float x, y, z;

		// Normalize the vector
		void normalize() {
			float magnitude = std::sqrt(x * x + y * y + z * z);
			if (magnitude > 0.0f) {
				x /= magnitude;
				y /= magnitude;
				z /= magnitude;
			}
		}
	};

	Vec3 CalculateAimingVector(uintptr_t gunFlashSocketRotationAddresses[3]) {
		// Fetch Euler angles (assuming they're stored as floats)
		float pitch = *(reinterpret_cast<float*>(gunFlashSocketRotationAddresses[0]));
		float yaw = *(reinterpret_cast<float*>(gunFlashSocketRotationAddresses[1]));

		// Convert from degrees to radians
		pitch *= 3.14159265359f / 180.0f;
		yaw *= 3.14159265359f / 180.0f;

		// Compute forward vector
		Vec3 forwardVector;
		forwardVector.x = std::cos(pitch) * std::sin(yaw);
		forwardVector.y = std::sin(pitch);
		forwardVector.z = std::cos(pitch) * std::cos(yaw);

		// Normalize the vector
		forwardVector.normalize();

		return forwardVector;
	}

	uintptr_t FindDMAAddy(uintptr_t baseAddress, const std::vector<unsigned int>& offsets) {
    uintptr_t addr = baseAddress;

    for (size_t i = 0; i < offsets.size(); ++i) {
        if (addr == 0) {
            // If at any point the address is invalid, return 0
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

	// Optional: Log some matrix values
 // API::get()->log_info("Updated rotation matrix values -> matrix0: %f, matrix1: %f, matrix2: %f", cameraMatrixValues[0], cameraMatrixValues[1], cameraMatrixValues[2]);
	void on_pre_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
		PLUGIN_LOG_ONCE("Pre Slate Draw Window");

	}

	void on_post_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
		PLUGIN_LOG_ONCE("Post Slate Draw Window");

	}
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASA_VRmod> g_plugin{ new GTASA_VRmod() };