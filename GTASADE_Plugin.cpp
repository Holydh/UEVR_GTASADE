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
		
		//Set up the memory addresses
		memoryManager.baseAddressGameEXE = memoryManager.GetModuleBaseAddress(nullptr);
		memoryManager.AdjustAddresses();

		Utilities::InitHelperClasses();
		weaponManager.HideBulletTrace();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		settingsManager.UpdateSettingsIfModified(settingsManager.configFilePath);

		//Fetch various states from memory
		playerManager.isInControl = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInControlAddress)) == 0;
		playerManager.isInVehicle = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInVehicleAddress)) > 0;
		playerManager.shootFromCarInput = *(reinterpret_cast<int*>(memoryManager.playerShootFromCarInputAddress)) == 3;
		playerManager.FetchPlayerUObjects();
		bool weaponWheelDisplayed = *(reinterpret_cast<int*>(memoryManager.weaponWheelDisplayedAddress)) > 30;
		cameraController.currentCameraMode = *(reinterpret_cast<CameraController::CameraMode*>(memoryManager.cameraModeAddress));
		MemoryManager::cameraMode = cameraController.currentCameraMode;

		if (!cameraController.waterViewFixed && playerManager.isInControl)
			cameraController.FixUnderwaterView(true);
		

		// Handles the cutscenes and various points in which the camera should be freed from VR controls.
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

		// Toggles the game's original instructions when going in and out of a vehicle.
		if (playerManager.isInControl && ((playerManager.isInVehicle && !playerManager.wasInVehicle) || 
			(playerManager.isInVehicle && cameraController.currentCameraMode != CameraController::AimWeaponFromCar && 
				cameraController.previousCameraMode ==  CameraController::AimWeaponFromCar )))
		{
			memoryManager.RestoreVehicleRelatedMemoryInstructions();
		}
		if (playerManager.isInControl && ((!playerManager.isInVehicle && playerManager.wasInVehicle) || 
			(playerManager.isInVehicle && cameraController.currentCameraMode == CameraController::AimWeaponFromCar && 
				cameraController.previousCameraMode !=  CameraController::AimWeaponFromCar )))
		{
			memoryManager.NopVehicleRelatedMemoryInstructions();
		}
	
		// VR mod processing :
		if (playerManager.isInControl)
		{
			weaponManager.UpdateActualWeaponMesh();
			if (settingsManager.debugMod) uevr::API::get()->log_info("equippedWeaponIndex");
			
			if (!weaponWheelDisplayed)
			{
				cameraController.ProcessCameraMatrix(delta);
				cameraController.ProcessHookedHeadPosition(delta);
				weaponManager.UpdateShootingState();
				weaponManager.UpdateAimingVectors();
			}

			weaponManager.WeaponHandling(delta);
		}
		weaponManager.HandleWeaponVisibility();

		// Keep previous states
		playerManager.wasInControl = playerManager.isInControl;
		playerManager.wasInVehicle = playerManager.isInVehicle;
		cameraController.previousCameraMode = cameraController.currentCameraMode;
		weaponManager.previousWeaponEquipped = weaponManager.currentWeaponEquipped;
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

	void ToggleAllUObjectHooks(bool enable)
	{
		if (enable)
			uevr::API::UObjectHook::set_disabled(false);
		else
		{
			uevr::API::UObjectHook::set_disabled(true);

			playerManager.DisablePlayerUObjectsHook();
			weaponManager.UnhookWeaponAndReposition();
		}
	}
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASADE_Plugin> g_plugin{ new GTASADE_Plugin() };