#include "WeaponManager.h"

void WeaponManager::UpdateActualWeaponMesh()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("UpdateActualWeaponMesh()");

	if (cameraController->currentCameraMode == CameraController::Camera  && cameraController->previousCameraMode == CameraController::Camera )
		return;

	//static auto gta_weapon_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/GTABase.GTAWeapon");
	//static auto gta_BPweapon_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"BlueprintGeneratedClass /Game/SanAndreas/GameData/Blueprints/BP_GTASA_Weapon.BP_GTASA_Weapon_C");
	static auto gta_BPplayerCharacter_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"BlueprintGeneratedClass /Game/SanAndreas/Characters/Player/BP_player_character.BP_Player_Character_C");
	static auto gta_StaticMeshComponent_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/Engine.StaticMeshComponent");
	//API::get()->log_info("gta_BPweapon_c = %ls", gta_BPweapon_c->get_full_name().c_str());

	const auto& playerControllerChildren = playerManager->playerController->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"Children");
	//API::get()->log_info("children = %ls", children.data[4]->get_full_name().c_str());
	const uevr::API::UObject* gta_BPplayerCharacter = nullptr;

	for (auto child : playerControllerChildren) {
		if (gta_BPplayerCharacter == nullptr && child->is_a(gta_BPplayerCharacter_c)) {
			gta_BPplayerCharacter = child;
		}
	}
	if (gta_BPplayerCharacter != nullptr)
		torso = gta_BPplayerCharacter->get_property<uevr::API::UObject*>(L"torso");
	else
	{
		uevr::API::get()->log_info("gta_BPplayerCharacter not found.");
		torso = nullptr;
		currentWeaponEquipped = Unarmed;
		weaponMesh = nullptr;
		weaponStaticMesh = nullptr;
		return;
	}
	const auto& torsoChildren = torso->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"AttachChildren");
	if (torsoChildren.count == 0)
	{
		currentWeaponEquipped = Unarmed;
		weaponMesh = nullptr;
		weaponStaticMesh = nullptr;
		return;
	}
	const auto& weaponRootChildren = torsoChildren.data[0]->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"AttachChildren");

	if (weaponRootChildren.data[0]->is_a(gta_StaticMeshComponent_c))
	{
		weaponMesh = weaponRootChildren.data[0];
		weaponStaticMesh = weaponMesh->get_property<uevr::API::UObject*>(L"StaticMesh");
	}
	else
	{
		currentWeaponEquipped = Unarmed;
		weaponMesh = nullptr;
		weaponStaticMesh = nullptr;
		return;
	}


	if (cameraController->currentCameraMode != CameraController::Camera  && (!playerManager->isInVehicle || 
		cameraController->currentCameraMode == CameraController::AimWeaponFromCar ))
	{
		auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
		glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);
		UEVR_Quaternionf defaultWeaponRotationQuat_UEVR = { defaultWeaponRotationQuat.w , defaultWeaponRotationQuat.x, defaultWeaponRotationQuat.y, defaultWeaponRotationQuat.z };
		motionState->set_rotation_offset(&defaultWeaponRotationQuat_UEVR);
		motionState->set_hand(1);
		motionState->set_permanent(true);
	}
	if ((cameraController->currentCameraMode != CameraController::Camera  && playerManager->isInVehicle && !playerManager->wasInVehicle) && 
		cameraController->currentCameraMode != CameraController::AimWeaponFromCar)
	{
		uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);
	}

	std::wstring weaponName = weaponStaticMesh->get_full_name();

	// Extract only the weapon name from the full path
	size_t lastDot = weaponName.find_last_of(L'.');
	if (lastDot != std::wstring::npos) {
		weaponName = weaponName.substr(lastDot + 1);
	}

	// Look up the weapon in the map
	auto it = weaponNameToIndex.find(weaponName);
	if (it != weaponNameToIndex.end()) {
		currentWeaponEquipped = static_cast<WeaponType>(it->second);
	}
}

void WeaponManager::HideBulletTrace()
{
	static auto bp_water_base_c = uevr::API::get()->find_uobject<uevr::API::UObject>(L"BP_Water_Base_C /Game/SanAndreas/Maps/SAWorld/SAWorld.SAWorld.PersistentLevel.BP_Water_Base_4");
	static auto bulletTraceProceduralMeshComponent = bp_water_base_c->get_property<uevr::API::UObject*>(L"BulletTrace");
	bulletTraceProceduralMeshComponent->set_bool_property(L"bVisible", false);
}

void WeaponManager::UpdateShootingState()
{
	if (!weaponMesh)
		return;

	const auto& childrenParticle = weaponMesh->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"AttachChildren");

	if (childrenParticle.count <= 0)
		return;

	auto actualParticleShot = childrenParticle.data[childrenParticle.count-1];
	if (actualParticleShot != lastParticleShot)
	{
		lastParticleShot = actualParticleShot;
		isShooting = true;
	}
}

void WeaponManager::ProcessAiming()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("UpdateAimingVectors()");

	if (cameraController->currentCameraMode == CameraController::Camera)
	{
		cameraController->forwardVectorUE = glm::fvec3(
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])),
			-*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])),
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2]))
		);
		return;
	}
	
	// If not aiming, synchronise the aiming vector with the camera matrix (prevents the radar from following the gun orientation)
	if (!playerManager->isInVehicle && camModsRequiringAimHandling.find((int)cameraController->currentCameraMode) == camModsRequiringAimHandling.end()) //check if the current camera mode is in the aiming cam, if not, return
	{
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) = cameraController->cameraMatrixValues[12];
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) = cameraController->cameraMatrixValues[13];
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) = cameraController->cameraMatrixValues[14];

		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = cameraController->cameraMatrixValues[4];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = cameraController->cameraMatrixValues[5];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = cameraController->cameraMatrixValues[6];
		return;
	}


	if (weaponMesh != nullptr) {
		Utilities::ParameterSingleVector3 forwardVector_params;
		Utilities::ParameterSingleVector3 upVector_params;
		Utilities::ParameterSingleVector3 rightVector_params;
		weaponMesh->call_function(L"GetForwardVector", &forwardVector_params);
		weaponMesh->call_function(L"GetUpVector", &upVector_params);
		weaponMesh->call_function(L"GetRightVector", &rightVector_params);

		glm::fvec3 point1Offsets = { 0.0f, 0.0f, 0.0f };
		glm::fvec3 point2Offsets = { 0.0f, 0.0f, 0.0f };
		bool socketAvailable = true;
		bool sprayWeapon = false;
		bool meleeWeapon = false;

		//mesh alignement weapon offsets
		switch (currentWeaponEquipped)
		{
			// Offsets taken from the game's 3D models in Blender by taking 2 points aligned with the barrel. Units is centimeters. 
			// Y axis is inversed in Blender.
			// We then add some slight offsets manually depending on the aiming tests done ingame.
		case Pistol :
			point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 + 0.2 };
			break;
		case PistolSilenced :
			point1Offsets = { 2.80735, -2.52308, 9.9193 };
			point2Offsets = { 17.3316, -3.5591 + 0.1, 12.2129 + 0.6 };
			break;
		case DesertEagle :
			point1Offsets = { 7.06492 , -2.25853 , 11.9386 + 0.5 };
			point2Offsets = { 33.5914, -1.46079 - 0.5, 11.9439 - 0.5 };
			break;
		case Shotgun :
			point1Offsets = { 31.3429, -0.670153, 15.2663 };
			point2Offsets = { 73.6795 , 4.2357 - 1 , 22.2237 - 2 };
			break;
		case Sawnoff :
			point1Offsets = { 21.2896, -2.13098 , 13.0224 };
			point2Offsets = { 55.8867 , -2.10406 - 1, 16.3934 - 2 };
			break;
		case Spas12 :
			point1Offsets = { 51.9659 , 1.30133, 19.5475 };
			point2Offsets = { 70.459 , 3.20646 , 22.5404 };
			break;
		case MicroUzi:
			point1Offsets = { -0.267532, -2.19868 , 10.2951 };
			point2Offsets = { 12.9468 , -0.996034 + 0.4, 11.293 + 0.9 };
			break;
		case Mp5:
			point1Offsets = { 6.8924, -1.74509 , 19.3761 };
			point2Offsets = { 21.3778 , 0.000536 + 0.2, 21.2535 + 1 };
			break;
		case Ak47:
			point1Offsets = { 3.8416 , -2.83908, 14.3539 };
			point2Offsets = { 36.3719, 0.193737 - 0.2, 16.1544 - 0.2 };
			break;
		case M4:
			point1Offsets = { 5.85945 , -1.78476 , 15.1271 };
			point2Offsets = { 60.0434  , 2.99539 - 1 , 16.4006 - 1.5 };
			break;
		case Tec9:
			point1Offsets = { 1.1631 , -3.60654, 11.7162 };
			point2Offsets = { 24.9241 , -3.60654, 13.9038 - 1 };
			break;
		case Rifle : //"cuntgun"
			point1Offsets = { 7.92837 , -3.48911 , 11.4936 };
			point2Offsets = { 71.2598, 4.09339 - 0.75, 20.9391 - 1.5 };
			break;
		case Sniper :
			point1Offsets = { 5.94806 , -2.75068, 13.2024 };
			point2Offsets = { 30.6871 , -0.22823 - 0.025, 15.6848 };
			socketAvailable = false;
			break;
		case RocketLauncher :
			//point1Offsets = { 2.41748 , -3.88386 , 14.4056 };
			//point2Offsets = { 29.0589, -3.88386, 14.4056 };
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			socketAvailable = false;
			break;
		case RocketLauncherHs : // RocketLauncherHeatSeek
			//point1Offsets = { -57.665 , -3.74195 , 20.2618 };
			//point2Offsets = { 34.8035, -3.52085 , 20.1928 };
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			socketAvailable = false;
			break;
		case Flamethrower :
			point1Offsets = { 48.0165 , -1.65182 , 16.1683 };
			point2Offsets = { 76.7885, 0.537026 , 31.6837 };
			sprayWeapon = true;
			break;
		case Minigun :
			point1Offsets = { 48.1025 , -2.9978 , 14.3878 };
			point2Offsets = { 86.6453 , 0.429413 - 0.5 , 35.9644 - 0.5 };
			break;
		case SprayCan:
			/*point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 };*/
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			sprayWeapon = true;
			socketAvailable = false;
			break;
		case Extinguisher:
			/*point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 };*/
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			sprayWeapon = true;
			socketAvailable = false;
			break;
		case Camera:
			point1Offsets = { 13.8476, -11.6162, 1.72577 };
			point2Offsets = { 27.6432, -11.6162, 2.84382 };
			socketAvailable = false;
			break;

		default:
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			socketAvailable = false;
			meleeWeapon = true;
			break;
		}

		glm::fvec3 point1Position = { 0.0f , 0.0f, 0.0f };
		glm::fvec3 point2Position = { 0.0f , 0.0f, 0.0f };
		glm::fvec3 aimingDirection = { 0.0f , 0.0f, 0.0f };

		if (socketAvailable && !meleeWeapon)
		{
			Utilities::ParameterGetSocketLocation socketLocation_params;
			socketLocation_params.inSocketName = uevr::API::FName(L"gunflash");
			weaponMesh->call_function(L"GetSocketLocation", &socketLocation_params);

			point1Position = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.outLocation, forwardVector_params.vec3Value, upVector_params.vec3Value, rightVector_params.vec3Value, point1Offsets);
			point2Position = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.outLocation, forwardVector_params.vec3Value, upVector_params.vec3Value, rightVector_params.vec3Value, point2Offsets);

			aimingDirection = glm::normalize(point2Position - point1Position);

			//The code below compensate for the over the shoulder camera offset. This game's crosshair is not centered.
			//The game calculate it's aiming vector with the camera direction. We move and align this camera with the gun mesh
			//so the game can still use the camera values to calculate the shoot traces.
			//Hacky but functional, if someday this game gets proper reverse engineered we should be able to do better.
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

			point2Position = Utilities::OffsetLocalPositionFromWorld(point2Position, projectedToFloorVector, yawUp, yawRight, crosshairOffset);

			// Safeguard: Recalculate aiming direction and normalize
			aimingDirection = point2Position - point1Position;
			if (glm::length(aimingDirection) > 0.0f) {
				aimingDirection = glm::normalize(aimingDirection);
			}
			else {
				aimingDirection = glm::fvec3(1.0f, 0.0f, 0.0f); // Fallback vector
			}
		}
		else
		{
			Utilities::ParameterSingleVector3 componentToWorld_params;
			weaponMesh->call_function(L"K2_GetComponentLocation", &componentToWorld_params);
			
			if (glm::length(point1Offsets) > 0.0f && glm::length(point2Offsets) > 0.0f)
			{
				point1Position = Utilities::OffsetLocalPositionFromWorld(componentToWorld_params.vec3Value, forwardVector_params.vec3Value, upVector_params.vec3Value, rightVector_params.vec3Value, point1Offsets);
				point2Position = Utilities::OffsetLocalPositionFromWorld(componentToWorld_params.vec3Value, forwardVector_params.vec3Value, upVector_params.vec3Value, rightVector_params.vec3Value, point2Offsets);
				aimingDirection = glm::normalize(point2Position - point1Position);
			}
			else
			{
				point1Position = componentToWorld_params.vec3Value;
				aimingDirection = forwardVector_params.vec3Value;
			}
		}

		calculatedAimForward = {aimingDirection.x, -aimingDirection.y, aimingDirection.z};
		calculatedAimPosition = { point1Position.x * 0.01f, -point1Position.y * 0.01f, point1Position.z * 0.01f};

		// If in vehicle, just use the forwardVector of the camera matrix
		if (playerManager->isInVehicle && cameraController->currentCameraMode != CameraController::AimWeaponFromCar || meleeWeapon)
		{
			//uevr::API::get()->log_info("UpdateAimingVectors isInVehicle");
			//forward vector
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = cameraController->cameraMatrixValues[4];
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = cameraController->cameraMatrixValues[5];
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = cameraController->cameraMatrixValues[6];
		}
		else
		{
			//Apply new values to memory
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) = calculatedAimPosition.x;
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) = calculatedAimPosition.y;
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) = calculatedAimPosition.z;

			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = calculatedAimForward.x;
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = calculatedAimForward.y;
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = calculatedAimForward.z;
		}
		//Fix the up/down aiming for spray weapons
		if (sprayWeapon)
		{
			*(reinterpret_cast<float*>(memoryManager->xAxisSpraysAimAddress)) = aimingDirection.z;
		}
	}
	else //if player unarmed
	{
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) = playerManager->actualPlayerPositionUE.x * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) = -playerManager->actualPlayerPositionUE.y * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) = playerManager->actualPlayerPositionUE.z * 0.01f;

		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = cameraController->cameraMatrixValues[4];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = cameraController->cameraMatrixValues[5];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = cameraController->cameraMatrixValues[6];
	}

	cameraController->forwardVectorUE = glm::fvec3(
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])),
			-*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])),
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2]))
		);
}

void WeaponManager::ProcessWeaponVisibility()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("HandleWeaponVisibility()");

	if (weaponMesh == nullptr)
		return;

	bool hideWeapon = false;
	switch (currentWeaponEquipped)
	{
	case NightVision:
		hideWeapon = true;
		break;
	case Infrared:
		hideWeapon = true;
		break;
	case Parachute:
		hideWeapon = true;
		break;
	default:
		hideWeapon = false;
		break;
	}

	Utilities::ParameterSingleBool setOwnerNoSee_params;
	setOwnerNoSee_params.boolValue = playerManager->isInControl ? hideWeapon : false; //Enable visibility when in cutscenes
	weaponMesh->call_function(L"SetOwnerNoSee", &setOwnerNoSee_params);
	weaponMesh->set_bool_property(L"bVisible", !setOwnerNoSee_params.boolValue);
}

void WeaponManager::ProcessWeaponHandling(float delta)
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("WeaponHandling()");

	//Forces the weapon to get back in it's original position when entering a vehicle. No 6dof possible in cars for now, devs removed the 
	// "free aiming in car" cheat that was in the original game. Would require more reverse engineering.
	if (playerManager->isInVehicle && !playerManager->wasInVehicle)
	{
		UpdateActualWeaponMesh();
		UnhookAndRepositionWeapon();
	}

	if (!playerManager->isInVehicle && playerManager->wasInVehicle)
	{
		UpdateActualWeaponMesh();
	}

	if (weaponMesh == nullptr || (playerManager->isInVehicle && cameraController->currentCameraMode != CameraController::AimWeaponFromCar)) //check a shooting on car scenario before deleting
		return;

	bool shootDetectionFromMemory = false;

	glm::fvec3 positionRecoilForce = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 rotationRecoilForceEuler = { 0.0f, 0.0f, 0.0f };
	switch (currentWeaponEquipped)
	{
	case Pistol:
		positionRecoilForce = { 0.0f, -0.005f, -0.5f };
		rotationRecoilForceEuler = { -0.08f, 0.0f, 0.0f };
		break;
	case PistolSilenced:
		positionRecoilForce = { 0.0f, -0.005f, -0.5f };
		rotationRecoilForceEuler = { -0.05f, 0.0f, 0.0f };
		break;
	case DesertEagle:
		positionRecoilForce = { 0.0f, -0.005f, -2.5f };
		rotationRecoilForceEuler = { -0.15f, 0.0f, 0.0f };
		break;
	case Shotgun:
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case Sawnoff:
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case Spas12:
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case MicroUzi:
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case Mp5:
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case Ak47:
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case M4:
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case Tec9:
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case Rifle: // "cuntgun"
		positionRecoilForce = { 0.0f, -0.005f, -2.0f };
		rotationRecoilForceEuler = { -0.02f, 0.0f, 0.0f };
		shootDetectionFromMemory = true;
		break;
	case Sniper:
		positionRecoilForce = { 0.0f, -0.005f, -2.0f };
		rotationRecoilForceEuler = { -0.02f, 0.0f, 0.0f };
		shootDetectionFromMemory = true;
		break;
	case RocketLauncher:
		positionRecoilForce = { 0.0f, -0.005f, -3.0f };
		rotationRecoilForceEuler = { -0.02f, 0.0f, 0.0f };
		shootDetectionFromMemory = true;
		break;
	case RocketLauncherHs : // RocketLauncherHeatSeek
		positionRecoilForce = { 0.0f, -0.005f, -3.0f };
		rotationRecoilForceEuler = { -0.02f, 0.0f, 0.0f };
		shootDetectionFromMemory = true;
		break;
	case Flamethrower:
		positionRecoilForce = { -0.0f, 0.5f, -0.5f };
		rotationRecoilForceEuler = { -0.0f, 0.0f, 0.0f };
		break;
	case Minigun:	
		//positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		//rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		positionRecoilForce = { 0.0f, -0.005f, -1.5f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case Detonator:
		return;
	case SprayCan:
		return;
	case Extinguisher:
		return;
	case Camera:
		HandleCameraWeaponAiming();
		return;

	default:

		UnhookAndRepositionWeapon();
		return;
	}

	if (shootDetectionFromMemory)
		isShooting = currentWeaponEquipped == previousWeaponEquipped ? memoryManager->isShooting : false;
	

	//Apply Recoil
	auto motionState = uevr::API::UObjectHook::get_motion_controller_state(weaponMesh);
	
	if (isShooting)
	{
		// use the UEVR uobject attached offset : 
		currentWeaponRecoilPosition += positionRecoilForce;
		currentWeaponRecoilRotationEuler += rotationRecoilForceEuler;
		UEVR_Vector3f recoilPosition = { currentWeaponRecoilPosition.x, currentWeaponRecoilPosition.y, currentWeaponRecoilPosition.z };
		glm::fquat weaponRecoilRotationQuat = glm::fquat(currentWeaponRecoilRotationEuler);
		UEVR_Quaternionf recoilRotation = { weaponRecoilRotationQuat.w, weaponRecoilRotationQuat.x, weaponRecoilRotationQuat.y, weaponRecoilRotationQuat.z };
		motionState->set_location_offset(&recoilPosition);
		motionState->set_rotation_offset(&recoilRotation);
		motionState->set_permanent(true);
	}
	else
	{
		// Smoothly return position to base
		currentWeaponRecoilPosition = glm::mix(currentWeaponRecoilPosition, defaultWeaponPosition, delta * recoilPositionRecoverySpeed);
		UEVR_Vector3f recoveredPositionFromRecoil = { currentWeaponRecoilPosition.x, currentWeaponRecoilPosition.y, currentWeaponRecoilPosition.z };
		motionState->set_location_offset(&recoveredPositionFromRecoil);

		// Convert Euler angles (radians) to quaternions for smooth rotation recovery
		glm::fquat weaponRecoilRotationQuat = glm::fquat(currentWeaponRecoilRotationEuler);
		glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);

		// Smoothly interpolate between current rotation and default rotation
		glm::fquat smoothedWeaponRotationQuat = glm::slerp(weaponRecoilRotationQuat, defaultWeaponRotationQuat, delta * recoilRotationRecoverySpeed);
		currentWeaponRecoilRotationEuler = glm::eulerAngles(smoothedWeaponRotationQuat);
		UEVR_Quaternionf recoveredRotationFromRecoil = { smoothedWeaponRotationQuat.w, smoothedWeaponRotationQuat.x, smoothedWeaponRotationQuat.y, smoothedWeaponRotationQuat.z };
		motionState->set_rotation_offset(&recoveredRotationFromRecoil);
	}
	memoryManager->isShooting = false;
	isShooting = false;
}

void WeaponManager::HandleCameraWeaponAiming()
{
	//The camera aiming vector doesn't works like every weapons. Instead it's based on the camera matrix (the original game's one, not the UEVR).
	//So the aiming has to be manually controlled with joystick.

	if (settingsManager->debugMod) uevr::API::get()->log_info("HandleCameraWeaponAiming()");

	// detaching the weaponMesh from motion controls
	if (cameraController->currentCameraMode == CameraController::Camera  && cameraController->previousCameraMode != CameraController::Camera )
	{
		uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);

		Utilities::ParameterDetachFromParent detachFromParent_params;
		detachFromParent_params.maintainWorldPosition = true;
		detachFromParent_params.callModify = false;
		weaponMesh->call_function(L"DetachFromParent", &detachFromParent_params);
	}

	if (cameraController->currentCameraMode != CameraController::Camera  && cameraController->previousCameraMode == CameraController::Camera )
	{
		uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
	}

	// Manually sets the camera Mesh to a position in front of the player.
	if (cameraController->currentCameraMode == CameraController::Camera)
	{
		glm::fvec3 cameraOffsetsPoint1 = { 13.8476, -11.6162, -1.72577 };
		glm::fvec3 cameraOffsetsPoint2 = { 27.6432, -11.6162, -2.84382 };

		// Convert local offsets into world positions
		glm::fvec3 worldOffset1 = (cameraController->forwardVectorUE * cameraOffsetsPoint1.x) + ( cameraController->rightVectorUE * cameraOffsetsPoint1.y) + (cameraController->upVectorUE* cameraOffsetsPoint1.z);
		glm::fvec3 worldOffset2 = (cameraController->forwardVectorUE * cameraOffsetsPoint2.x) + ( cameraController->rightVectorUE * cameraOffsetsPoint2.y) + (cameraController->upVectorUE * cameraOffsetsPoint2.z);

		glm::fvec3 weaponPoint1 = cameraController->cameraPositionUE + (cameraController->forwardVectorUE * 35.0f) + worldOffset1;
		glm::fvec3 weaponPoint2 = cameraController->cameraPositionUE + (cameraController->forwardVectorUE * 35.0f) + worldOffset2;

		// Apply position to weaponMesh
		Utilities::Parameter_K2_SetWorldOrRelativeLocation setWorldLocation_params{};
		setWorldLocation_params.bSweep = false;
		setWorldLocation_params.bTeleport = true;
		setWorldLocation_params.newLocation = weaponPoint1;
		weaponMesh->call_function(L"K2_SetWorldLocation", &setWorldLocation_params);

		// FindLookAtRotation from Point1 to Point2
		Utilities::ParameterFindLookAtRotation lookAtRotationParams;
		lookAtRotationParams.start = weaponPoint1;
		lookAtRotationParams.target = weaponPoint2;
		Utilities::KismetMathLibrary->call_function(L"FindLookAtRotation", &lookAtRotationParams);

		// Apply rotation to weaponMesh
		Utilities::Parameter_K2_SetWorldOrRelativeRotation setWorldRotation_params{};
		setWorldRotation_params.bSweep = false;
		setWorldRotation_params.bTeleport = true;
		setWorldRotation_params.newRotation = lookAtRotationParams.outRotation;
		weaponMesh->call_function(L"K2_SetWorldRotation", &setWorldRotation_params);
	}
}

void WeaponManager::UnhookAndRepositionWeapon()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("DisableMeleeWeaponsUObjectHooks()");

	if (weaponMesh == nullptr)
		return;
	uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);

	//Reset weapon position and rotation for melee weapons
	Utilities::Parameter_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
	setRelativeLocation_params.bSweep = false;
	setRelativeLocation_params.bTeleport = true;
	setRelativeLocation_params.newLocation = glm::fvec3(0.0f, 0.0f, 0.0f);
	weaponMesh->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);

	Utilities::Parameter_K2_SetWorldOrRelativeRotation setRelativeRotation_params{};
	setRelativeRotation_params.bSweep = false;
	setRelativeRotation_params.bTeleport = true;
	setRelativeRotation_params.newRotation = { 0.0f, 0.0f, 0.0f };
	weaponMesh->call_function(L"K2_SetRelativeRotation", &setRelativeLocation_params);
}