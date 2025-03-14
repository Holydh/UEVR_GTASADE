#include "WeaponManager.h"

void WeaponManager::UpdateActualWeaponMesh()
	{
		//static auto gta_weapon_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/GTABase.GTAWeapon");
		//static auto gta_BPweapon_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"BlueprintGeneratedClass /Game/SanAndreas/GameData/Blueprints/BP_GTASA_Weapon.BP_GTASA_Weapon_C");
		static auto gta_BPplayerCharacter_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"BlueprintGeneratedClass /Game/SanAndreas/Characters/Player/BP_player_character.BP_Player_Character_C");
		static auto gta_StaticMeshComponent_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/Engine.StaticMeshComponent");
		//API::get()->log_info("gta_BPweapon_c = %ls", gta_BPweapon_c->get_full_name().c_str());

	
		const auto& playerControllerChildren = playerManager->playerController->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"Children");
		//API::get()->log_info("children = %ls", children.data[4]->get_full_name().c_str());

		if (playerControllerChildren.data[3]->is_a(gta_BPplayerCharacter_c))
			torso = playerControllerChildren.data[3]->get_property<uevr::API::UObject*>(L"torso");
		else
		{
			uevr::API::get()->log_info("gta_BPplayerCharacter not found.");
			torso = nullptr;
			equippedWeaponIndex = 0;
			weaponMesh = nullptr;
			weaponStaticMesh = nullptr;
			return;
		}
		const auto& torsoChildren = torso->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"AttachChildren");
		if (torsoChildren.count == 0)
		{
			equippedWeaponIndex = 0;
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
			equippedWeaponIndex = 0;
			weaponMesh = nullptr;
			weaponStaticMesh = nullptr;
			return;
		}
	

		if (!playerManager->characterIsInVehicle || cameraController->cameraMode == 55)
		{
			auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
			glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);
			UEVR_Quaternionf defaultWeaponRotationQuat_UEVR = { defaultWeaponRotationQuat.w , defaultWeaponRotationQuat.x, defaultWeaponRotationQuat.y, defaultWeaponRotationQuat.z };
			motionState->set_rotation_offset(&defaultWeaponRotationQuat_UEVR);
			motionState->set_hand(1);
			motionState->set_permanent(true);
		}
		if ((playerManager->characterIsInVehicle && !playerManager->characterWasInVehicle) && cameraController->cameraMode != 55)
		{
			uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);
		}


		//for (auto child : playerControllerChildren){
		//	//API::get()->log_info("child = %ls", child->get_full_name().c_str());
		//	if (child->is_a(gta_weapon_c) && child == playerControllerChildren.data[4]) {
		//		weapon = child;
		//		weaponMesh = weapon->get_property<uevr::API::UObject*>(L"WeaponMesh");
		//		/*API::get()->log_info("%ls", weaponMesh->get_full_name().c_str());*/
		//		weaponStaticMesh = weaponMesh->get_property<uevr::API::UObject*>(L"StaticMesh");
		//		/*API::get()->log_info("%ls", weaponStaticMesh->get_full_name().c_str());*/

		//		if (!playerManager->characterIsInVehicle || cameraController->cameraMode == 55)
		//		{
		//			auto motionState = uevr::API::UObjectHook::get_or_add_motion_controller_state(weaponMesh);
		//			glm::fquat defaultWeaponRotationQuat = glm::fquat(defaultWeaponRotationEuler);
		//			UEVR_Quaternionf defaultWeaponRotationQuat_UEVR = { defaultWeaponRotationQuat.w , defaultWeaponRotationQuat.x, defaultWeaponRotationQuat.y, defaultWeaponRotationQuat.z };
		//			motionState->set_rotation_offset(&defaultWeaponRotationQuat_UEVR);
		//			motionState->set_hand(1);
		//			motionState->set_permanent(true);
		//		}
		//		if ((playerManager->characterIsInVehicle && !playerManager->characterWasInVehicle) && cameraController->cameraMode != 55)
		//		{
		//			uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);
		//		}
		//		break;
		//	}
		//	else
		//	{
		//		weaponMesh = nullptr;
		//		weaponStaticMesh = nullptr;
		//	}
		//}


		//if (weaponMesh == nullptr)
		//{
		//	equippedWeaponIndex = 0;
		//	return;
		//}
		//
		std::wstring weaponName = weaponStaticMesh->get_full_name();
		
		// Extract only the weapon name from the full path
		size_t lastDot = weaponName.find_last_of(L'.');
		if (lastDot != std::wstring::npos) {
			weaponName = weaponName.substr(lastDot + 1);
		}
		/*API::get()->log_info("%ls", weaponName.c_str());*/
		// Look up the weapon in the map
		auto it = weaponNameToIndex.find(weaponName);
		if (it != weaponNameToIndex.end()) {
			equippedWeaponIndex = it->second;
		}
		/*API::get()->log_info("%i", equippedWeaponIndex);*/
	}

void WeaponManager::UpdateAimingVectors()
{
	if (weaponMesh != nullptr) {
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

		glm::fvec3 point1Offsets = { 0.0f, 0.0f, 0.0f };
		glm::fvec3 point2Offsets = { 0.0f, 0.0f, 0.0f };
		bool socketAvailable = true;
		bool sprayWeapon = false;
		bool meleeWeapon = false;

		//mesh alignement weapon offsets
		switch (equippedWeaponIndex)
		{
			//case 0:  // Unarmed
			//case 1:  // BrassKnuckles
			//case 2:  // GolfClub
			//case 3:  // NightStick
			//case 4:  // Knife
			//case 5:  // BaseballBat
			//case 6:  // Shovel
			//case 7:  // PoolCue
			//case 8:  // Katana
			//case 9:  // Chainsaw
			//case 10: // Dildo1
			//case 11: // Dildo2
			//case 12: // Vibe1
			//case 13: // Vibe2
			//case 14: // Flowers
			//case 15: // Cane
			//case 16: // Grenade
			//case 17: // Teargas
			//case 18: // Molotov
		case 22: //Pistol colt 45
			point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 + 0.2 };
			break;
		case 23: // PistolSilenced
			point1Offsets = { 2.80735, -2.52308, 9.9193 };
			point2Offsets = { 17.3316, -3.5591 + 0.1, 12.2129 + 0.6 };
			break;
		case 24: // DesertEagle
			point1Offsets = { 7.06492 , -2.25853 , 11.9386 + 0.5 };
			point2Offsets = { 33.5914, -1.46079 - 0.5, 11.9439 - 0.5 };
			break;
		case 25: // Shotgun
			point1Offsets = { 31.3429, -0.670153, 15.2663 };
			point2Offsets = { 73.6795 , 4.2357 - 1 , 22.2237 - 2 };
			break;
		case 26: // Sawnoff
			point1Offsets = { 21.2896, -2.13098 , 13.0224 };
			point2Offsets = { 55.8867 , -2.10406 - 1, 16.3934 - 2 };
			break;
		case 27: // Spas12
			point1Offsets = { 51.9659 , 1.30133, 19.5475 };
			point2Offsets = { 70.459 , 3.20646 , 22.5404 };
			break;
		case 28: // MicroUzi
			point1Offsets = { -0.267532, -2.19868 , 10.2951 };
			point2Offsets = { 12.9468 , -0.996034 + 0.4, 11.293 + 0.9 };
			break;
		case 29: // Mp5
			point1Offsets = { 6.8924, -1.74509 , 19.3761 };
			point2Offsets = { 21.3778 , 0.000536 + 0.2, 21.2535 + 1 };
			break;
		case 30: //AK47
			point1Offsets = { 3.8416 , -2.83908, 14.3539 };
			point2Offsets = { 36.3719, 0.193737 - 0.2, 16.1544 - 0.2 };
			break;
		case 31: // M4
			point1Offsets = { 5.85945 , -1.78476 , 15.1271 };
			point2Offsets = { 60.0434  , 2.99539 - 1 , 16.4006 - 1.5 };
			break;
		case 32: // Tec9
			point1Offsets = { 1.1631 , -3.60654, 11.7162 };
			point2Offsets = { 24.9241 , -3.60654, 13.9038 - 1 };
			break;
		case 33: //Rifle cuntgun
			point1Offsets = { 7.92837 , -3.48911 , 11.4936 };
			point2Offsets = { 71.2598, 4.09339 - 0.75, 20.9391 - 1.5 }; //additional offsets required. Crosshair offset is probably different for that weapon
			break;
		case 34: // Sniper
			point1Offsets = { 3.00373 , -3.05089 , 10.5162 };
			point2Offsets = { 76.0552 , 4.39762, 17.8463 };
			break;
		case 35: // RocketLauncher
			//point1Offsets = { 2.41748 , -3.88386 , 14.4056 };
			//point2Offsets = { 29.0589, -3.88386, 14.4056 };
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			socketAvailable = false;
			break;
		case 36: // RocketLauncherHeatSeek
			//point1Offsets = { -57.665 , -3.74195 , 20.2618 };
			//point2Offsets = { 34.8035, -3.52085 , 20.1928 };
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			socketAvailable = false;
			break;
		case 37: // Flamethrower
			point1Offsets = { 48.0165 , -1.65182 , 16.1683 };
			point2Offsets = { 76.7885, 0.537026 , 31.6837 };
			sprayWeapon = true;
			break;
		case 38: // Minigun
			point1Offsets = { 48.1025 , -2.9978 , 14.3878 };
			point2Offsets = { 86.6453 , 0.429413 - 0.5 , 35.9644 - 0.5 };
			break;
			//case 39: // Satchel
			//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
			//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
			//	break;
			//case 40: // Detonator
			//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
			//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
			//	break;
		case 41: // SprayCan
			/*point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 };*/
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			sprayWeapon = true;
			socketAvailable = false;
			break;
		case 42: // Extinguisher
			/*point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 };*/
			point1Offsets = { 0.0f , 0.0f, 0.0f };
			point2Offsets = { 0.0f , 0.0f, 0.0f };
			sprayWeapon = true;
			socketAvailable = false;
			break;
		case 43: // Camera
			point1Offsets = { 2.82819, -2.52103, 9.92684 };
			point2Offsets = { 21.7272, -3.89487, 12.9088 };
			socketAvailable = false;
			break;
			//case 44: // NightVision
			//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
			//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
			//	break;
			//case 45: // Infrared
			//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
			//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
			//	break;
			//case 46: // Parachute
			//	point1Offsets = { 2.82819, -2.52103, 9.92684 };
			//	point2Offsets = { 21.7272, -3.89487, 12.9088 };
			//	break;

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
			struct {
				const struct uevr::API::FName& InSocketName = uevr::API::FName(L"gunflash");
				glm::fvec3 Location;
			} socketLocation_params;

			weaponMesh->call_function(L"GetSocketLocation", &socketLocation_params);
			//API::get()->log_info("ForwardVector : x = %f, y = %f, z = %f", forwardVector_params.ForwardVector.x,  forwardVector_params.ForwardVector.y, forwardVector_params.ForwardVector.z);

			point1Position = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point1Offsets);
			point2Position = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, point2Offsets);

			aimingDirection = glm::normalize(point2Position - point1Position);

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
			struct {
				glm::fvec3 Location;
			} componentToWorld_params;
			weaponMesh->call_function(L"K2_GetComponentLocation", &componentToWorld_params);
			point1Position = componentToWorld_params.Location;
			aimingDirection = forwardVector_params.ForwardVector;
		}



		if (playerManager->characterIsInVehicle && cameraController->cameraMode != 55 || meleeWeapon)
		{
			// Apply new values to memory - This messes up the aiming vector
			//*(reinterpret_cast<float*>(cameraPositionAddresses[0])) = actualPlayerPositionUE.x * 0.01f;;
			//*(reinterpret_cast<float*>(cameraPositionAddresses[1])) = -actualPlayerPositionUE.y * 0.01f;;
			//*(reinterpret_cast<float*>(cameraPositionAddresses[2])) = actualPlayerPositionUE.z * 0.01f;;


			////forward vector
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = cameraController->cameraMatrixValues[4];
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = cameraController->cameraMatrixValues[5];
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = cameraController->cameraMatrixValues[6];

			////forward vector
			//*(reinterpret_cast<float*>(aimUpVectorAddresses[0])) = cameraMatrixValues[8];
			//*(reinterpret_cast<float*>(aimUpVectorAddresses[1])) = cameraMatrixValues[9];
			//*(reinterpret_cast<float*>(aimUpVectorAddresses[2])) = cameraMatrixValues[10];
		}
		else
		{
			//Apply new values to memory
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) = point1Position.x * 0.01f;
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) = -point1Position.y * 0.01f;
			*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) = point1Position.z * 0.01f;

			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = aimingDirection.x;
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = -aimingDirection.y;
			*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = aimingDirection.z;
		}
		if (sprayWeapon)
		{
			*(reinterpret_cast<float*>(memoryManager->xAxisSpraysAimAddress)) = aimingDirection.z;
		}
	}
	else //unarmed
	{
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) = playerManager->actualPlayerPositionUE.x * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) = -playerManager->actualPlayerPositionUE.y * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) = playerManager->actualPlayerPositionUE.z * 0.01f;

		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[0])) = cameraController->cameraMatrixValues[4];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[1])) = cameraController->cameraMatrixValues[5];
		*(reinterpret_cast<float*>(memoryManager->aimForwardVectorAddresses[2])) = cameraController->cameraMatrixValues[6];
	}
}

void WeaponManager::FixWeaponVisibility()
{
	bool hideWeapon = false;
	switch (equippedWeaponIndex)
	{
		//case 0:  // Unarmed
		//case 1:  // BrassKnuckles
		//case 2:  // GolfClub
		//case 3:  // NightStick
		//case 4:  // Knife
		//case 5:  // BaseballBat
		//case 6:  // Shovel
		//case 7:  // PoolCue
		//case 8:  // Katana
		//case 9:  // Chainsaw
		//case 10: // Dildo1
		//case 11: // Dildo2
		//case 12: // Vibe1
		//case 13: // Vibe2
		//case 14: // Flowers
		//case 15: // Cane
		//case 16: // Grenade
		//case 17: // Teargas
		//case 18: // Molotov
		//case 22: //Pistol colt 45
		//case 23: // PistolSilenced
		//case 24: // DesertEagle
		//case 25: // Shotgun
		//case 26: // Sawnoff
		//case 27: // Spas12
		//case 28: // MicroUzi
		//case 29: // Mp5
		//case 30: //AK47
		//case 31: // M4
		//case 32: // Tec9
		//case 33: //Rifle cuntgun
		//case 34: // Sniper
		//case 35: // RocketLauncher
		//case 36: // RocketLauncherHeatSeek
		//case 37: // Flamethrower
		//case 38: // Minigun
		//case 39: // Satchel
		//case 40: // Detonator
		//case 41: // SprayCan
		//case 42: // Extinguisher
		//case 43: // Camera
		case 44: // NightVision
			hideWeapon = true;
			break;
		case 45: // Infrared
			hideWeapon = true;
			break;
		case 46: // Parachute
			hideWeapon = true;
			break;

	default:
		hideWeapon = false;
		break;
	}
	struct {
		bool ownerNoSee = false;
	} setOwnerNoSee_params;
	setOwnerNoSee_params.ownerNoSee = hideWeapon;
	weaponMesh->call_function(L"SetOwnerNoSee", &setOwnerNoSee_params);
	weaponMesh->set_bool_property(L"bVisible", !hideWeapon);
}

void WeaponManager::WeaponHandling(float delta)
{
	if (playerManager->characterIsInVehicle && !playerManager->characterWasInVehicle)
	{
		UpdateActualWeaponMesh();
		ResetWeaponMeshPosAndRot();
	}

	if (!playerManager->characterIsInVehicle && playerManager->characterWasInVehicle)
	{
		UpdateActualWeaponMesh();
	}

	if ((playerManager->characterIsInVehicle && cameraController->cameraMode != 55)) //check a shooting on car scenario before deleting
		return;

	glm::fvec3 positionRecoilForce = { 0.0f, 0.0f, 0.0f };
	glm::fvec3 rotationRecoilForceEuler = { 0.0f, 0.0f, 0.0f };
	switch (equippedWeaponIndex)
	{
		//case 0:  // Unarmed
		//case 1:  // BrassKnuckles
		//case 2:  // GolfClub
		//case 3:  // NightStick
		//case 4:  // Knife
		//case 5:  // BaseballBat
		//case 6:  // Shovel
		//case 7:  // PoolCue
		//case 8:  // Katana
		//case 9:  // Chainsaw
		//case 10: // Dildo1
		//case 11: // Dildo2
		//case 12: // Vibe1
		//case 13: // Vibe2
		//case 14: // Flowers
		//case 15: // Cane
		//case 16: // Grenade
		//case 17: // Teargas
		//case 18: // Molotov
	case 22: //Pistol colt 45
		positionRecoilForce = { 0.0f, -0.005f, -0.5f };
		rotationRecoilForceEuler = { -0.08f, 0.0f, 0.0f };
		break;
	case 23: // PistolSilenced
		positionRecoilForce = { 0.0f, -0.005f, -0.5f };
		rotationRecoilForceEuler = { -0.05f, 0.0f, 0.0f };
		break;
	case 24: // DesertEagle
		positionRecoilForce = { 0.0f, -0.005f, -2.5f };
		rotationRecoilForceEuler = { -0.15f, 0.0f, 0.0f };
		break;
	case 25: // Shotgun
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case 26: // Sawnoff
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case 27: // Spas12
		positionRecoilForce = { 0.0f, -0.005f, -5.0f };
		rotationRecoilForceEuler = { -0.3f, 0.0f, 0.0f };
		break;
	case 28: // MicroUzi
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 29: // Mp5
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 30: //AK47
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 31: // M4
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 32: // Tec9
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 33: //Rifle cuntgun
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 34: // Sniper
		positionRecoilForce = { 0.0f, -0.005f, -1.0f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 35: // RocketLauncher
		positionRecoilForce = { 0.0f, -0.005f, -1.5f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 36: // RocketLauncherHeatSeek
		positionRecoilForce = { 0.0f, -0.005f, -1.5f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
	case 37: // Flamethrower
		positionRecoilForce = { 0.0f, -0.00001f, -0.0001f };
		rotationRecoilForceEuler = { -0.0001f, 0.0f, 0.0f };
		break;
	case 38: // Minigun
		positionRecoilForce = { 0.0f, -0.005f, -1.5f };
		rotationRecoilForceEuler = { -0.01f, 0.0f, 0.0f };
		break;
		//case 39: // Satchel
	case 40: // Detonator
		return;
	case 41: // SprayCan
		return;
	case 42: // Extinguisher
		return;
		//case 43: // Camera
		//case 44: // NightVision
		//case 45: // Infrared
		//case 46: // Parachute

	default:
		DisableMeleeWeaponsUObjectHooks();
		return;
	}

	auto motionState = uevr::API::UObjectHook::get_motion_controller_state(weaponMesh);


	if (playerManager->isShooting)
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

}

void WeaponManager::DisableMeleeWeaponsUObjectHooks()
{
	if (weaponMesh == nullptr)
		return;
	uevr::API::UObjectHook::remove_motion_controller_state(weaponMesh);

	//Reset weapon position and rotation for melee weapons
	Utilities::SceneComponent_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
	setRelativeLocation_params.bSweep = false;
	setRelativeLocation_params.bTeleport = true;
	setRelativeLocation_params.NewLocation = glm::fvec3(0.0f, 0.0f, 0.0f);
	weaponMesh->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);

	Utilities::SceneComponent_K2_SetWorldOrRelativeRotation setRelativeRotation_params{};
	setRelativeRotation_params.bSweep = false;
	setRelativeRotation_params.bTeleport = true;
	setRelativeRotation_params.NewRotation = { 0.0f, 0.0f, 0.0f };
	weaponMesh->call_function(L"K2_SetRelativeRotation", &setRelativeLocation_params);
}

void WeaponManager::ResetWeaponMeshPosAndRot()
{
	if (weaponMesh == nullptr)
		return;
	Utilities::SceneComponent_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
	setRelativeLocation_params.bSweep = false;
	setRelativeLocation_params.bTeleport = true;
	setRelativeLocation_params.NewLocation = glm::fvec3(0.0f, 0.0f, 0.0f);
	weaponMesh->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);

	Utilities::SceneComponent_K2_SetWorldOrRelativeRotation setRelativeRotation_params{};
	setRelativeRotation_params.bSweep = false;
	setRelativeRotation_params.bTeleport = true;
	setRelativeRotation_params.NewRotation = { 0.0f, 0.0f, 0.0f };
	weaponMesh->call_function(L"K2_SetRelativeRotation", &setRelativeLocation_params);
}

	//switch (equippedWeaponIndex)
	//{
	//	//case 0:  // Unarmed
	//	//case 1:  // BrassKnuckles
	//	//case 2:  // GolfClub
	//	//case 3:  // NightStick
	//	//case 4:  // Knife
	//	//case 5:  // BaseballBat
	//	//case 6:  // Shovel
	//	//case 7:  // PoolCue
	//	//case 8:  // Katana
	//	//case 9:  // Chainsaw
	//	//case 10: // Dildo1
	//	//case 11: // Dildo2
	//	//case 12: // Vibe1
	//	//case 13: // Vibe2
	//	//case 14: // Flowers
	//	//case 15: // Cane
	//	//case 16: // Grenade
	//	//case 17: // Teargas
	//	//case 18: // Molotov
	//	//case 22: //Pistol colt 45
	//	//case 23: // PistolSilenced
	//	//case 24: // DesertEagle
	//	//case 25: // Shotgun
	//	//case 26: // Sawnoff
	//	//case 27: // Spas12
	//	//case 28: // MicroUzi
	//	//case 29: // Mp5
	//	//case 30: //AK47
	//	//case 31: // M4
	//	//case 32: // Tec9
	//	//case 33: //Rifle cuntgun
	//	//case 34: // Sniper
	//	//case 35: // RocketLauncher
	//	//case 36: // RocketLauncherHeatSeek
	//	//case 37: // Flamethrower
	//	//case 38: // Minigun
	//	//case 39: // Satchel
	//	//case 40: // Detonator
	//	//case 41: // SprayCan
	//	//case 42: // Extinguisher
	//	//case 43: // Camera
	//	//case 44: // NightVision
	//	//case 45: // Infrared
	//	//case 46: // Parachute

	//default:
	//}