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

    //Camera Matrix addresses :
    uintptr_t cameraMatrixAddresses[16] = {
        0x53E2C00, 0x53E2C04, 0x53E2C08, 0x53E2C0C, 
        0x53E2C10, 0x53E2C14, 0x53E2C18, 0x53E2C1C, 
        0x53E2C20, 0x53E2C24, 0x53E2C28, 0x53E2C2C,
        0x53E2C30, 0x53E2C34, 0x53E2C38, 0x53E2C3C
    };

    uintptr_t AimVectorAddresses[3]
    {
        0x53E2668, 0x53E266C, 0x53E2670
    };

    float cameraMatrixValues[16] = { 0.0f }; 
    float yawOffsetDegrees = 0.0f;

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

        for (auto& address : AimVectorAddresses) {
            address += baseAddress;
        }

        oss << "Base address: 0x" << std::hex << AimVectorAddresses[2];
        std::string cameraMatrix10AddressStr = oss.str();

        // Log the last address
        API::get()->log_info(cameraMatrix10AddressStr.c_str());
    }

    void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
        PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);

    }

    void on_post_engine_tick(API::UGameEngine* engine, float delta) override {
        PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);
        
		for (int i = 0; i < 16; ++i) {
			cameraMatrixValues[i] = *(reinterpret_cast<float*>(cameraMatrixAddresses[i]));
		}

		// Allocate the transformation matrix
		UEVR_Vector3f hmdPosition{};
		UEVR_Quaternionf hmdRotation{};
		UEVR_Vector2f rightJoystick{};

		// Get the transformation matrix for the HMD
		API::get()->param()->vr->get_pose(API::get()->param()->vr->get_hmd_index(), &hmdPosition, &hmdRotation);
		API::get()->param()->vr->get_joystick_axis(API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);

        API::get()->log_info("Joystick Input X: %f", rightJoystick.x);

		    // Sensitivity factor for rotation
    const float rotationSpeed = 45.0f; // Degrees per second
    float yawDelta = rightJoystick.x * rotationSpeed * delta; // Horizontal rotation
    float yawRadians = yawDelta * 0.0174533f; // Convert degrees to radians

    // Extract current forward and right vectors from the camera matrix
    UEVR_Vector3f forward{
        cameraMatrixValues[8],
        cameraMatrixValues[9],
        cameraMatrixValues[10]
    };
    UEVR_Vector3f right{
        cameraMatrixValues[4],
        cameraMatrixValues[5],
        cameraMatrixValues[6]
    };

    // Create a quaternion from the current rotation
    UEVR_Quaternionf currentRotation = matrixToQuaternion(cameraMatrixValues);

    // Create the yaw quaternion
    UEVR_Quaternionf yawRotation = quaternionFromEuler(0.0f, yawRadians, 0.0f);

    // Apply the yaw rotation to the current rotation
    UEVR_Quaternionf newRotation = shortestPathQuaternion(currentRotation, yawRotation);

    // Convert the new quaternion back to a rotation matrix
    quaternionToMatrix(newRotation, cameraMatrixValues);

    // Write rotation matrix to memory
    for (int i = 0; i < 12; ++i) {
        *(reinterpret_cast<float*>(cameraMatrixAddresses[i])) = cameraMatrixValues[i];
    }

    // Optional: Log some matrix values
    // API::get()->log_info("Updated rotation matrix values -> matrix0: %f, matrix1: %f, matrix2: %f", cameraMatrixValues[0], cameraMatrixValues[1], cameraMatrixValues[2]);
}
    UEVR_Quaternionf shortestPathQuaternion(const UEVR_Quaternionf& q1, const UEVR_Quaternionf& q2) {
    UEVR_Quaternionf q2Copy = q2;
    // If the dot product is negative, negate the second quaternion to take the shortest path
    float dot = q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
    if (dot < 0.0f) {
        q2Copy.w = -q2.w;
        q2Copy.x = -q2.x;
        q2Copy.y = -q2.y;
        q2Copy.z = -q2.z;
    }
    return quaternionMultiply(q1, q2Copy);
}

	UEVR_Quaternionf quaternionMultiply(const UEVR_Quaternionf& q1, const UEVR_Quaternionf& q2) {
		UEVR_Quaternionf result;
		result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
		result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
		result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
		return result;
	}

	UEVR_Quaternionf matrixToQuaternion(const float* matrix) {
		// Convert rotation matrix to quaternion
		UEVR_Quaternionf q;
		float trace = matrix[0] + matrix[5] + matrix[10];

		if (trace > 0.0f) {
			float s = 0.5f / sqrt(trace + 1.0f);
			q.w = 0.25f / s;
			q.x = (matrix[9] - matrix[6]) * s;
			q.y = (matrix[2] - matrix[8]) * s;
			q.z = (matrix[4] - matrix[1]) * s;
		}
		else {
			if (matrix[0] > matrix[5] && matrix[0] > matrix[10]) {
				float s = 2.0f * sqrt(1.0f + matrix[0] - matrix[5] - matrix[10]);
				q.w = (matrix[9] - matrix[6]) / s;
				q.x = 0.25f * s;
				q.y = (matrix[1] + matrix[4]) / s;
				q.z = (matrix[2] + matrix[8]) / s;
			}
			else if (matrix[5] > matrix[10]) {
				float s = 2.0f * sqrt(1.0f + matrix[5] - matrix[0] - matrix[10]);
				q.w = (matrix[2] - matrix[8]) / s;
				q.x = (matrix[1] + matrix[4]) / s;
				q.y = 0.25f * s;
				q.z = (matrix[6] + matrix[9]) / s;
			}
			else {
				float s = 2.0f * sqrt(1.0f + matrix[10] - matrix[0] - matrix[5]);
				q.w = (matrix[4] - matrix[1]) / s;
				q.x = (matrix[2] + matrix[8]) / s;
				q.y = (matrix[6] + matrix[9]) / s;
				q.z = 0.25f * s;
			}
		}
		return q;
	}

	UEVR_Quaternionf quaternionFromEuler(float roll, float pitch, float yaw) {
		// Convert Euler angles to quaternion
		float cy = cos(yaw * 0.5f);
		float sy = sin(yaw * 0.5f);
		float cp = cos(pitch * 0.5f);
		float sp = sin(pitch * 0.5f);
		float cr = cos(roll * 0.5f);
		float sr = sin(roll * 0.5f);

		UEVR_Quaternionf q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;
		return q;
	}

	void quaternionToMatrix(const UEVR_Quaternionf& q, float* matrix) {
		// Convert quaternion to rotation matrix
		matrix[0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
		matrix[1] = 2.0f * (q.x * q.y - q.z * q.w);
		matrix[2] = 2.0f * (q.x * q.z + q.y * q.w);

		matrix[4] = 2.0f * (q.x * q.y + q.z * q.w);
		matrix[5] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
		matrix[6] = 2.0f * (q.y * q.z - q.x * q.w);

		matrix[8] = 2.0f * (q.x * q.z - q.y * q.w);
		matrix[9] = 2.0f * (q.y * q.z + q.x * q.w);
		matrix[10] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	}

    void on_pre_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Pre Slate Draw Window");

    }

    void on_post_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Post Slate Draw Window");

    }
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASA_VRmod> g_plugin{new GTASA_VRmod()};