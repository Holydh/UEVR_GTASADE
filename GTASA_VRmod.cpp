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
    //uintptr_t cameraMatrixAddresses[16] = {
    //    0x53E2C00, 0x53E2C04, 0x53E2C08, 0x53E2C0C, 
    //    0x53E2C10, 0x53E2C14, 0x53E2C18, 0x53E2C1C, 
    //    0x53E2C20, 0x53E2C24, 0x53E2C28, 0x53E2C2C,
    //    0x53E2C30, 0x53E2C34, 0x53E2C38, 0x53E2C3C
    //};

    uintptr_t AimVectorAddresses[3]
    {
        0x53E2668, 0x53E266C, 0x53E2670
    };

    float cameraMatrixValues[12] = { 0.0f }; 

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
        for (auto& address : AimVectorAddresses) {
            address += baseAddress;
        }

        oss << "Base address: 0x" << std::hex << AimVectorAddresses[10];
        std::string cameraMatrix10AddressStr = oss.str();

        // Log the last address
        API::get()->log_info(cameraMatrix10AddressStr.c_str());
    }

    void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
        PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
        
        // Allocate the transformation matrix
        UEVR_Vector3f hmdPosition{};
        UEVR_Quaternionf hmdRotation{};

        // Get the transformation matrix for the HMD
        API::get()->param()->vr->get_pose(API::get()->param()->vr->get_hmd_index(), &hmdPosition, &hmdRotation);

        // Calculate the forward vector
        UEVR_Vector3f forwardVector;
        forwardVector.x = 2.0f * (hmdRotation.x * hmdRotation.z + hmdRotation.w * hmdRotation.y);
        forwardVector.y = 2.0f * (hmdRotation.y * hmdRotation.z - hmdRotation.w * hmdRotation.x);
        forwardVector.z = 1.0f - 2.0f * (hmdRotation.x * hmdRotation.x + hmdRotation.y * hmdRotation.y);

        *(reinterpret_cast<float*>(AimVectorAddresses[0])) = -forwardVector.x;
        *(reinterpret_cast<float*>(AimVectorAddresses[1])) = forwardVector.z;
        *(reinterpret_cast<float*>(AimVectorAddresses[2])) = -forwardVector.y;

        // Write rotation matrix to memory
 /*       for (int i = 0; i < 12; ++i) {
            *(reinterpret_cast<float*>(cameraMatrixAddresses[i])) = cameraMatrixValues[i];
        }*/

        // Optional: Log some matrix values
		API::get()->log_info("Hmd rotations values -> matrix0: %f, matrix1: %f, matrix2: %f", cameraMatrixValues[0], cameraMatrixValues[1], cameraMatrixValues[2]);
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
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASA_VRmod> g_plugin{new GTASA_VRmod()};