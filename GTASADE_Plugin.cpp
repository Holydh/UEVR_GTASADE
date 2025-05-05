#include "uevr/Plugin.hpp"
#include "uevr/API.hpp"
#include "MemoryManager.h"
#include "SettingsManager.h"
#include "CameraController.h"
#include "PlayerManager.h"
#include "WeaponManager.h"
#include "Utilities.h"
//#include <chrono>

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
		ManagePluginState(false);
	}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");
		settingsManager.InitSettingsManager();
		memoryManager.InitMemoryManager();
		Utilities::InitHelperClasses();
		weaponManager.HideBulletTrace();
	}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		/*auto start = std::chrono::high_resolution_clock::now();*/

		FetchRequiredValuesFromMemory();
		playerManager.FetchPlayerUObjects();
		if (!cameraController.underwaterViewFixed && playerManager.isInControl)
			cameraController.FixUnderwaterView(true);

		ManagePluginState(true);

		// Main VR functions :
		if (pluginStateApplied != VRdisabled)
		{
			weaponManager.UpdateActualWeaponMesh();
			if (settingsManager.debugMod) uevr::API::get()->log_info("equippedWeaponIndex");
			
			if (!playerManager.weaponWheelEnabled)
			{
				cameraController.ProcessCameraMatrix(delta);
				cameraController.ProcessHookedHeadPosition(delta);
				weaponManager.UpdateShootingState(!weaponManager.firstWeaponShotDone);
				weaponManager.ProcessAiming(!weaponManager.firstWeaponShotDone);
			}

			weaponManager.ProcessWeaponHandling(delta);
			weaponManager.ProcessWeaponVisibility();
		}
		SendStatesToLua();
		settingsManager.UpdateSettingsIfModifiedByPlayer();
		UpdatePreviousStates();

		//auto end = std::chrono::high_resolution_clock::now();
		//auto duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		//API::get()->log_info("execution time : %lld micro seconds", duration_ms.count());
		//Last test average = 85,150537634409 micro seconds
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

	void ManagePluginState(bool enableVR)
	{
		if (settingsManager.debugMod) API::get()->log_info("ManagePluginState");

		// We need to fetch the weapon one last time after player lost control so the plugin can correctly reset the weapon position for cutscenes.
		if (!playerManager.isInControl && playerManager.wasInControl)
			weaponManager.UpdateActualWeaponMesh();

		bool viewRequiresDisabledVR = playerManager.isInControl && 
			(!playerManager.isInVehicle && cameraController.currentOnFootCameraMode != CameraController::OnFootCameraMode::Close) ||
			(playerManager.isInVehicle && cameraController.currentVehicleCameraMode != CameraController::VehicleCameraMode::Close);

		if (pluginStateApplied != VRdisabled && (!playerManager.isInControl || viewRequiresDisabledVR || !enableVR))
		{
			ApplyVRdisabledState();
			return;
		}

		if (viewRequiresDisabledVR || !enableVR)
			return;

		if (pluginStateApplied != OnFoot && playerManager.isInControl && (!playerManager.isInVehicle || cameraController.currentCameraMode == CameraController::AimWeaponFromCar) && 
			cameraController.currentCameraMode != CameraController::Camera)
			ApplyBaseState();

		// Toggles the game's original instructions when going in or out of a vehicle if there's no scripted event with AimWeaponFromCar camera.
		// Then sets UEVR settings according to the vehicle type
		if (pluginStateApplied != Driving && playerManager.isInControl && playerManager.isInVehicle && cameraController.currentCameraMode != CameraController::AimWeaponFromCar)
		{
			ApplyBaseState();
			ApplyDrivingState();
		}
		
		// Toggles the game's original instructions for the camera weapon controls
		if (pluginStateApplied != CameraWeapon && cameraController.currentCameraMode == CameraController::Camera)
			ApplyCameraWeaponState();
	}

	void FetchRequiredValuesFromMemory()
	{
		if (settingsManager.debugMod) API::get()->log_info("FetchRequiredValuesFromMemory");
		playerManager.isInControl = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInControlAddress)) == 0;
		playerManager.isInVehicle = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInVehicleAddress)) > 0;
		playerManager.vehicleType = *(reinterpret_cast<PlayerManager::VehicleType*>(memoryManager.vehicleTypeAddress));
		playerManager.shootFromCarInput = *(reinterpret_cast<int*>(memoryManager.playerShootFromCarInputAddress)) == 3;
		playerManager.weaponWheelEnabled = *(reinterpret_cast<int*>(memoryManager.weaponWheelDisplayedAddress)) > 30;
		cameraController.currentCameraMode = *(reinterpret_cast<CameraController::CameraMode*>(memoryManager.cameraModeAddress));
		cameraController.currentOnFootCameraMode = *(reinterpret_cast<CameraController::OnFootCameraMode*>(memoryManager.onFootCameraModeAddress));
		cameraController.currentVehicleCameraMode = *(reinterpret_cast<CameraController::VehicleCameraMode*>(memoryManager.vehicleCameraModeAddress));
		//cameraController.isCutscenePlaying = *(reinterpret_cast<uint8_t*>(memoryManager.cutscenePlayingAddress)) > 0;
	}

	void UpdatePreviousStates()
	{
		if (settingsManager.debugMod) API::get()->log_info("UpdatePreviousStates");

		playerManager.wasInControl = playerManager.isInControl;
		playerManager.wasInVehicle = playerManager.isInVehicle;
		playerManager.previousVehicleType = playerManager.vehicleType;
		cameraController.previousCameraMode = cameraController.currentCameraMode;
		cameraController.previousOnFootCameraMode = cameraController.currentOnFootCameraMode;
		cameraController.previousVehicleCameraMode = cameraController.currentVehicleCameraMode;
		//cameraController.wasCutscenePlaying = cameraController.isCutscenePlaying;
		weaponManager.previousWeaponEquipped = weaponManager.currentWeaponEquipped;
	}

	enum PluginState {
		Uninitialized = 0,
		VRdisabled = 1,
		OnFoot = 2,
		Driving = 3,
		CameraWeapon = 4
	};

	PluginState pluginStateApplied = Uninitialized;

	void ApplyBaseState()
	{
		cameraController.camResetRequested = true;
		memoryManager.ToggleAllMemoryInstructions(false);
		memoryManager.InstallBreakpoints();
		uevr::API::UObjectHook::set_disabled(false);
		weaponManager.ResetShootingState();
		settingsManager.ApplyCameraSettings(SettingsManager::OnFoot);
		pluginStateApplied = OnFoot;
		if (settingsManager.debugMod) API::get()->log_info("pluginStateApplied = OnFoot");
	}

	void ApplyDrivingState()
	{
		memoryManager.RestoreVehicleRelatedMemoryInstructions();
		switch (playerManager.vehicleType)
		{
		case PlayerManager::Plane:
		case PlayerManager::Helicopter:
			settingsManager.ApplyCameraSettings(SettingsManager::Flying);
			break;
		case PlayerManager::CarOrBoat:
			settingsManager.ApplyCameraSettings(SettingsManager::DrivingCar);
			break;
		case PlayerManager::Bike:
			settingsManager.ApplyCameraSettings(SettingsManager::DrivingBike);
			break;
		}
		weaponManager.UnhookAndRepositionWeapon();
		pluginStateApplied = Driving;
		/*if (settingsManager.debugMod) */API::get()->log_error("pluginStateApplied = Driving");
	}

	void ApplyCameraWeaponState()
	{
		memoryManager.ToggleAllMemoryInstructions(true);
		pluginStateApplied = CameraWeapon;
		API::get()->log_error("pluginStateApplied = Camera");
	}

	void ApplyVRdisabledState()
	{
		memoryManager.RemoveBreakpoints();
		memoryManager.ToggleAllMemoryInstructions(true);
		cameraController.FixUnderwaterView(false);
		uevr::API::UObjectHook::set_disabled(true);
		playerManager.RepositionUnhookedUobjects();
		weaponManager.UnhookAndRepositionWeapon();
		settingsManager.ApplyCameraSettings(SettingsManager::Cutscene);
		pluginStateApplied = VRdisabled;
		API::get()->log_error("pluginStateApplied = NoControls");
	}

	void SendStatesToLua()
	{
		if (playerManager.vehicleType != playerManager.previousVehicleType)
			API::get()->dispatch_lua_event("playerState", playerManager.VehicleTypeToString(playerManager.vehicleType));
		if (cameraController.previousOnFootCameraMode != cameraController.currentOnFootCameraMode)
			API::get()->dispatch_lua_event("onFootCameraMode", cameraController.VehicleCameraModeToString(cameraController.currentVehicleCameraMode));
		if (cameraController.previousVehicleCameraMode != cameraController.currentVehicleCameraMode)
			API::get()->dispatch_lua_event("vehicleCameraMode", cameraController.VehicleCameraModeToString(cameraController.currentVehicleCameraMode));
	}
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<GTASADE_Plugin> g_plugin{ new GTASADE_Plugin() };