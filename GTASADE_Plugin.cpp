#include "uevr/Plugin.hpp"
#include "MemoryManager.h"
#include "SettingsManager.h"
#include "CameraController.h"
#include "PlayerManager.h"
#include "WeaponManager.h"
#include "Utilities.h"
//#include <thread>
//#include <atomic> 
//#include <chrono>
//#include <iostream>

using namespace uevr;

#define PLUGIN_LOG_ONCE(...) {\
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }}

//std::atomic<bool> resetMatrix(false);
//std::atomic<bool> stopThread(false);
//std::condition_variable cv;
//std::mutex cv_m;

class GTASADE_Plugin : public uevr::Plugin {
private:
	MemoryManager memoryManager;
	SettingsManager settingsManager;
	CameraController cameraController;
	PlayerManager playerManager;
	WeaponManager weaponManager;

	//std::unique_ptr<std::thread> waitThread;


public:
	GTASADE_Plugin() : cameraController(&memoryManager, &settingsManager, &playerManager),
        weaponManager(&playerManager, &cameraController, &memoryManager, &settingsManager),
		playerManager(&settingsManager),
		memoryManager(&settingsManager){}

	void on_dllmain_attach() override {}

	void on_dllmain_detach() override {
		memoryManager.RemoveBreakpoints();
		memoryManager.RemoveExceptionHandler();
		memoryManager.RestoreAllMemoryInstructions(true);
		cameraController.FixUnderwaterView(false);
		ToggleAllUObjectHooks(false);
		
		
        //// Signal thread to stop
        //{
        //    std::lock_guard<std::mutex> lk(cv_m);
        //    stopThread.store(true, std::memory_order_release);
        //    resetMatrix.store(true, std::memory_order_release);
        //}
        //cv.notify_all();

        //// Wait for thread to finish with timeout
        //if (waitThread && waitThread->joinable()) {
        //    if (waitThread->get_id() != std::this_thread::get_id()) {
        //        // Implement join with timeout
        //        auto start = std::chrono::steady_clock::now();
        //        while (waitThread->joinable()) {
        //            if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(500)) {
        //                API::get()->log_error("Thread failed to terminate in time - detaching");
        //                waitThread->detach();
        //                break;
        //            }
        //            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        //        }
        //        if (waitThread->joinable()) {
        //            waitThread->join();
        //        }
        //    } else {
        //        waitThread->detach();
        //    }
        //}
        //waitThread.reset();
	}

	void on_initialize() override {
		API::get()->log_info("%s", "VR cpp mod initializing");
		settingsManager.configFilePath = settingsManager.GetConfigFilePath();
		API::get()->log_info("%s", settingsManager.configFilePath.c_str());
		

		memoryManager.baseAddressGameEXE = memoryManager.GetModuleBaseAddress(nullptr);
		memoryManager.AdjustAddresses();

		//waitThread = std::make_unique<std::thread>([this]() {
		//	this->WaitAndUpdateTheCameraMatrix();
		//	});
	}

	//void WaitAndUpdateTheCameraMatrix() {
	//	std::unique_lock<std::mutex> lk(cv_m);
	//	while (!stopThread.load(std::memory_order_acquire)) {
	//		cv.wait_for(lk, std::chrono::milliseconds(9), [this]() {
	//			return resetMatrix.load(std::memory_order_acquire) ||
	//				stopThread.load(std::memory_order_acquire);
	//			});

	//		if (resetMatrix.load(std::memory_order_acquire)) {
	//			lk.unlock();
	//			cameraController.UpdateCameraMatrix();
	//			lk.lock();
	//			resetMatrix.store(false, std::memory_order_release);
	//		}
	//	}
	//}

	void on_pre_engine_tick(API::UGameEngine* engine, float delta) override {
		PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
		//MemoryManager::isShootingCamera == false;

		//API::get()->log_error("MemoryManager::cameraMatrixAddresses[0] : 0x%llx", MemoryManager::cameraMatrixAddresses[1]);
		//API::get()->log_info(" MemoryManager::matrixAimCalculatedValues[3] : %f", MemoryManager::matrixAimCalculatedValues[3]);
		settingsManager.UpdateSettingsIfModified(settingsManager.configFilePath);

		playerManager.isInControl = *(reinterpret_cast<uint8_t*>(memoryManager.playerIsInControlAddress)) == 0;

		if (!cameraController.waterViewFixed && playerManager.isInControl)
			cameraController.FixUnderwaterView(true);

		
	/*	API::get()->log_info("playerIsInControl = %i",playerIsInControl);*/
		//Debug
		//if (GetAsyncKeyState(VK_UP)) fpsCamInitialized = true;
		//if (GetAsyncKeyState(VK_DOWN)) fpsCamInitialized = false;

		bool weaponWheelDisplayed = *(reinterpret_cast<int*>(memoryManager.weaponWheelDisplayedAddress)) > 30;
		/*characterIsGettingInACar = *(reinterpret_cast<byte*>(memoryManager.characterIsGettingInACarAddress)) > 0;*/
		
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
			memoryManager.RestoreAllMemoryInstructions(false);
			memoryManager.InstallBreakpoints();
			ToggleAllUObjectHooks(true);
		}
		else
			cameraController.camResetRequested = false;

		if (!playerManager.isInControl && playerManager.wasInControl)
		{
			if (settingsManager.debugMod) API::get()->log_info("player NOT InControl");
			memoryManager.RestoreAllMemoryInstructions(true);
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

		//{
		//	std::lock_guard<std::mutex> lk(cv_m);
		//	resetMatrix.store(true, std::memory_order_release);
		//}
		//cv.notify_one();
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

	//void on_pre_calculate_stereo_view_offset(UEVR_StereoRenderingDeviceHandle, int view_index, float world_to_meters,
	//	UEVR_Vector3f* position, UEVR_Rotatorf* rotation, bool is_double)
	//{
	//	API::get()->log_error("3 - On pre calculate");
	//	
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