#include "uevr/Plugin.hpp"
#include "uevr/API.hpp"
#include "MemoryManager.h"
#include "SettingsManager.h"
#include "CameraController.h"
#include "PlayerManager.h"
#include "WeaponManager.h"
#include "Utilities.h"

using namespace uevr;

#define PLUGIN_LOG_ONCE(...) {\
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }}

class GTASADE_Plugin : public uevr::Plugin {
private:
	MemoryManager memoryManager;
	SettingsManager settingsManager;
	CameraController cameraController;
	PlayerManager playerManager;
	WeaponManager weaponManager;


public:
	GTASADE_Plugin() : cameraController(&memoryManager, &settingsManager, &playerManager),
        weaponManager(&playerManager, &cameraController, &memoryManager, &settingsManager),
		playerManager(&settingsManager),
		memoryManager(&settingsManager){}

	void on_dllmain() override {}

	void on_dllmain_detach() override {
		memoryManager.RemoveBreakpoints();
		memoryManager.RemoveExceptionHandler();
		memoryManager.ToggleAllMemoryInstructions(true);
		cameraController.FixUnderwaterView(false);
		ToggleAllUObjectHooks(false);
	}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");
		settingsManager.configFilePath = settingsManager.GetConfigFilePath();
		API::get()->log_info("%s", settingsManager.configFilePath.c_str());
		

		memoryManager.baseAddressGameEXE = memoryManager.GetModuleBaseAddress(nullptr);
		memoryManager.AdjustAddresses();

		Utilities::InitHelperClasses();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		settingsManager.UpdateSettingsIfModified(settingsManager.configFilePath);

		playerManager.isInControl = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInControlAddress)) == 0;

		if (!cameraController.waterViewFixed && playerManager.isInControl)
			cameraController.FixUnderwaterView(true);

		
	/*	API::get()->log_info("playerIsInControl = %i",playerIsInControl);*/
		//Debug
		//if (GetAsyncKeyState(VK_UP)) fpsCamInitialized = true;
		//if (GetAsyncKeyState(VK_DOWN)) fpsCamInitialized = false;

		bool weaponWheelDisplayed = *(reinterpret_cast<int*>(memoryManager.weaponWheelDisplayedAddress)) > 30;
		
		playerManager.isInVehicle = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInVehicleAddress)) > 0;
		//API::get()->log_info("characterIsInCar = %i", characterIsInCar);
		cameraController.cameraModeIs = *(reinterpret_cast<int*>(memoryManager.cameraModeAddress));
		MemoryManager::cameraMode = cameraController.cameraModeIs;
		//API::get()->log_info("cameraController.cameraModeIs = %i", cameraController.cameraModeIs);

		playerManager.shootFromCarInput = *(reinterpret_cast<int*>(memoryManager.playerShootFromCarInputAddress)) == 3;
		//API::get()->log_info("playerShootFromCarInput = %i", playerShootFromCarInput);
		playerManager.isShooting = weaponManager.equippedWeaponIndex == weaponManager.previousEquippedWeaponIndex ? memoryManager.isShooting : false;
		memoryManager.isShooting = false;

		playerManager.FetchPlayerUObjects();
		
		if (playerManager.isInControl && !playerManager.wasInControl)
		{
			if (settingsManager.debugMod) API::get()->log_info("playerIsInControl");
			cameraController.camResetRequested = true;
			memoryManager.ToggleAllMemoryInstructions(false);
			memoryManager.InstallBreakpoints();
			ToggleAllUObjectHooks(true);
		}
		else
			cameraController.camResetRequested = false;

		if (!playerManager.isInControl && playerManager.wasInControl)
		{
			if (settingsManager.debugMod) API::get()->log_info("player NOT InControl");
			memoryManager.ToggleAllMemoryInstructions(true);
			memoryManager.RemoveBreakpoints();
			memoryManager.RemoveExceptionHandler();
			ToggleAllUObjectHooks(false);
		}

		if (playerManager.isInControl && ((playerManager.isInVehicle && !playerManager.wasInVehicle) || (playerManager.isInVehicle && cameraController.cameraModeIs != 55 && cameraController.cameraModeWas == 55)))
		{
			memoryManager.RestoreVehicleRelatedMemoryInstructions();
		}

		if (playerManager.isInControl && ((!playerManager.isInVehicle && playerManager.wasInVehicle) || (playerManager.isInVehicle && cameraController.cameraModeIs == 55 && cameraController.cameraModeWas != 55)))
		{
			memoryManager.NopVehicleRelatedMemoryInstructions();
		}
	
		if (playerManager.isInControl)
		{
			weaponManager.UpdateActualWeaponMesh();
			
			if (!weaponWheelDisplayed)
			{
				cameraController.ProcessCameraMatrix(delta);
				cameraController.ProcessHookedHeadPosition();
				weaponManager.UpdateAimingVectors();
			}

			weaponManager.WeaponHandling(delta);
		}

		weaponManager.HandleWeaponVisibility();

		playerManager.wasInControl = playerManager.isInControl;
		playerManager.wasInVehicle = playerManager.isInVehicle;
		cameraController.cameraModeWas = cameraController.cameraModeIs;
		weaponManager.previousEquippedWeaponIndex = weaponManager.equippedWeaponIndex;
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

	//void on_custom_event(const char* event_name, const char* event_data) override
	//{
	//	if (event_name == "OnHmdComponentCreated")
	//	{
	//		API::get()->log_info("OnHmdComponentCreated, event_data = %s", event_data);
	//		std::wstring w_event_data = std::wstring(event_data, event_data + strlen(event_data));
	//		playerManager.playerHmd = uevr::API::get()->find_uobject<uevr::API::UObject>(w_event_data);
	//		if (playerManager.playerHmd != nullptr)
	//			API::get()->log_info("playerHmd = %ws", playerManager.playerHmd->get_full_name().c_str());
	//	}
	//}

	//void on_pre_calculate_stereo_view_offset(UEVR_StereoRenderingDeviceHandle, int view_index, float world_to_meters,
	//	UEVR_Vector3f* position, UEVR_Rotatorf* rotation, bool is_double)
	//{
	//	rotation->pitch = 0.0f;
	//	rotation->yaw = -90.0f;
	//	rotation->roll = 0.0f;

	//	cameraController.hm
	//}

	void ToggleAllUObjectHooks(bool enable)
	{
		if (enable)
			uevr::API::UObjectHook::set_disabled(false);
		else
		{
			//uevr::API::UObjectHook::remove_all_motion_controller_states();
			uevr::API::UObjectHook::set_disabled(true);

			playerManager.DisablePlayerUObjectsHook();
			weaponManager.ResetWeaponMeshPosAndRot();
		}
	}

};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASADE_Plugin> g_plugin{ new GTASADE_Plugin() };