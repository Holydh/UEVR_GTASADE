#pragma once
#include <unordered_map>
#include <unordered_set>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "uevr/API.hpp"
#include "PlayerManager.h"
#include "CameraController.h"
#include "MemoryManager.h"
#include "Utilities.h"
#include "SettingsManager.h"


class WeaponManager {
private:
	PlayerManager* const playerManager;
	CameraController* const cameraController;
	MemoryManager* const memoryManager;
	SettingsManager* const settingsManager;

public:
	WeaponManager(PlayerManager* pm, CameraController* cc, MemoryManager* mm, SettingsManager* sm) : playerManager(pm), cameraController(cc), memoryManager(mm), settingsManager(sm) {};

	//Shoot detection
	bool isShooting = false;
	int shootParticleCount = 0;
	uevr::API::UObject* lastParticleShot = nullptr;

	//Weapon infos
	uevr::API::UObject* weapon = nullptr;
	uevr::API::UObject* weaponMesh = nullptr;
	uevr::API::UObject* torso = nullptr;
	uevr::API::UObject* weaponStaticMesh = nullptr;
	const std::unordered_map<std::wstring, int> weaponNameToIndex = {
		{L"SM_unarmed", 0},           // Unarmed
		{L"SM_brassknuckle", 1},    // BrassKnuckles
		{L"SM_golfclub", 2},         // GolfClub
		{L"SM_nightstick", 3},       // NightStick
		{L"SM_knifecur", 4},            // Knife
		{L"SM_bat", 5},      // BaseballBat
		{L"SM_shovel", 6},           // Shovel
		{L"SM_poolcue", 7},          // PoolCue
		{L"SM_katana", 8},           // Katana
		{L"SM_chnsaw", 9},         // Chainsaw
		{L"SM_gun_dildo1", 10},          // Dildo1
		{L"SM_gun_dildo2", 11},          // Dildo2
		{L"SM_gun_vibe1", 12},           // Vibe1
		{L"SM_gun_vibe2", 13},           // Vibe2
		{L"SM_flowera", 14},         // Flowers
		{L"SM_gun_cane", 15},            // Cane
		{L"SM_grenade", 16},         // Grenade
		{L"SM_teargas", 17},         // Teargas
		{L"SM_molotov", 18},         // Molotov
		{L"SM_colt45", 22},          // Pistol Colt 45
		{L"SM_silenced", 23},        // Silenced Pistol
		{L"SM_desert_eagle", 24},     // Desert Eagle
		{L"SM_chromegun", 25},         // Shotgun
		{L"SM_sawnoff", 26},         // Sawnoff Shotgun
		{L"SM_shotgspa", 27},          // Spas12
		{L"SM_micro_uzi", 28},             // MicroUzi
		{L"SM_mp5lng", 29},             // MP5
		{L"SM_ak47", 30},            // AK47
		{L"SM_m4", 31},              // M4
		{L"SM_tec9", 32},            // Tec9
		{L"SM_cuntgun", 33},         // Rifle (Cuntgun)
		{L"SM_sniper", 34},          // Sniper Rifle
		{L"SM_rocketla", 35},  // RocketLauncher
		{L"SM_heatseek", 36},// RocketLauncherHeatSeek
		{L"SM_flame", 37},    // Flamethrower
		{L"SM_minigun", 38},         // Minigun
		{L"SM_satchel", 39},         // Satchel
		{L"SM_detonator", 40},       // Detonator
		{L"SM_spraycan", 41},        // SprayCan
		{L"SM_fire_ex", 42},    // Extinguisher
		{L"SM_camera", 43},          // Camera
		{L"SM_nvgoggles", 44},     // NightVision
		{L"SM_irgoggles", 45},        // Infrared
		{L"SM_gun_para", 46}        // Parachute
	};
	enum WeaponType {
		Unarmed = 0,
		BrassKnuckles = 1,
		GolfClub = 2,
		NightStick = 3,
		Knife = 4,
		BaseballBat = 5,
		Shovel = 6,
		PoolCue = 7,
		Katana = 8,
		Chainsaw = 9,
		Dildo1 = 10,
		Dildo2 = 11,
		Vibe1 = 12,
		Vibe2 = 13,
		Flowers = 14,
		Cane = 15,
		Grenade = 16,
		Teargas = 17,
		Molotov = 18,
		Pistol = 22,
		PistolSilenced = 23,
		DesertEagle = 24,
		Shotgun = 25,
		Sawnoff = 26,
		Spas12 = 27,
		MicroUzi = 28,
		Mp5 = 29,
		Ak47 = 30,
		M4 = 31,
		Tec9 = 32,
		Rifle = 33,
		Sniper = 34,
		RocketLauncher = 35,
		RocketLauncherHs = 36,
		Flamethrower = 37,
		Minigun = 38,
		Satchel = 39,
		Detonator = 40,
		SprayCan = 41,
		Extinguisher = 42,
		Camera = 43,
		NightVision = 44,
		Infrared = 45,
		Parachute = 46
	};
	WeaponType currentWeaponEquipped = Unarmed;
	WeaponType previousWeaponEquipped = Unarmed;

	//aiming
	glm::fvec3 crosshairOffset = { 0.0f, -1.0f, 2.0f };
	glm::fvec3 calculatedAimForward = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 calculatedAimPosition = { 0.0f, 0.0f, 0.0f };
	std::unordered_set<int> aimingCamModes = {5, 7, 8, 9, 34, 39, 40, 41, 42, 45, 51, 52, 53, 55, 65};

	//recoil
	glm::fvec3 defaultWeaponRotationEuler = { 0.4f, 0.0f, 0.0f };
	glm::fvec3 defaultWeaponPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilPosition = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 currentWeaponRecoilRotationEuler = { 0.0f, 0.0f, 0.0f };
	float recoilPositionRecoverySpeed = 10.0f;
	float recoilRotationRecoverySpeed = 8.0f;
	
	void UpdateActualWeaponMesh();
	void HideBulletTrace();
	void UpdateShootingState();
	void UpdateAimingVectors();
	void HandleWeaponVisibility();
	void WeaponHandling(float delta);
	void UnhookWeaponAndReposition();
	void HandleCameraWeaponAiming();
};