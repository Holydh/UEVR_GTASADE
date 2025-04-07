#include "CameraController.h"


void CameraController::ProcessCameraMatrix(float delta) {
	if (settingsManager->debugMod) uevr::API::get()->log_info("ProcessCameraMatrix()");

	// Update the camera position based on the head's socket location
	Utilities::ParameterSocketLocation socketLocation_params{};
	socketLocation_params.InSocketName = uevr::API::FName(L"head");
	playerManager->playerHead->call_function(L"GetSocketLocation", &socketLocation_params);

	Utilities::ParameterGLMfvec3 forwardVector_params{};
	Utilities::ParameterGLMfvec3 upVector_params{};
	Utilities::ParameterGLMfvec3 rightVector_params{};
	playerManager->playerHead->call_function(L"GetForwardVector", &forwardVector_params);
	playerManager->playerHead->call_function(L"GetUpVector", &upVector_params);
	playerManager->playerHead->call_function(L"GetRightVector", &rightVector_params);


	// Camera Matrix rotation ---------------------------------------------------
	UEVR_Vector2f rightJoystick{};
	uevr::API::get()->param()->vr->get_joystick_axis(uevr::API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);

	// Create a rotation matrix from the head's forward, up, and right vectors
	glm::mat4 headRotationMatrix = glm::mat4(1.0f);

	headRotationMatrix[0] = glm::vec4(-forwardVector_params.returnedValue.x, forwardVector_params.returnedValue.y, -forwardVector_params.returnedValue.z, 0.0f);
	headRotationMatrix[1] = glm::vec4(-rightVector_params.returnedValue.x, rightVector_params.returnedValue.y, -rightVector_params.returnedValue.z, 0.0f); // Right vector
	headRotationMatrix[2] = glm::vec4(upVector_params.returnedValue.x, -upVector_params.returnedValue.y, upVector_params.returnedValue.z, 0.0f);      // Up vector 

	float joystickYaw = 0.0f;

	// Reset the accumulated joystick rotation when going in or out of a vehicle
	if (playerManager->isInVehicle && !playerManager->wasInVehicle)
	{
		accumulatedJoystickRotation = glm::mat4(1.0f);
	}
	if ((!playerManager->isInVehicle && playerManager->wasInVehicle) || (!playerManager->isInVehicle && camResetRequested) || (cameraModeWas == 46 && cameraModeIs != 46))
	{
		accumulatedJoystickRotation = glm::mat4(1.0f);
		baseHeadRotation = headRotationMatrix;
	}

	// Calculate the delta rotation matrix, basically the rotation of the vehicle we're driving if any. 
	glm::mat4 deltaRotationMatrix = playerManager->isInVehicle && cameraModeIs != 55 ? glm::inverse(accumulatedJoystickRotation) * headRotationMatrix : glm::inverse(accumulatedJoystickRotation) * baseHeadRotation;

	// Joystick input to adjust the camera yaw rotation
	const float DEADZONE = 0.1f;
	if (abs(rightJoystick.x) > DEADZONE) {
		joystickYaw = -rightJoystick.x * delta * settingsManager->xAxisSensitivity;
	}

	// Convert joystick yaw to radians
	float yawRadians = joystickYaw * (M_PI / 180.0f);

	// Create rotation matrice
	glm::mat4 joystickYawRotation = glm::rotate(glm::mat4(1.0f), yawRadians, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around world Y-axis

	// Combine the accumulatedJoystickRotation with the joystick rotations
	accumulatedJoystickRotation = accumulatedJoystickRotation * joystickYawRotation;

	// Combine the accumulatedJoystickRotation with the delta rotation so camera follows the car when driving
	glm::mat4 totalDeltaRotation = accumulatedJoystickRotation * deltaRotationMatrix;

	// Combine the head rotation matrix with the joystick rotations
	glm::mat4 finalRotationMatrix = accumulatedJoystickRotation * totalDeltaRotation;

	// Copy the modified matrix back to cameraMatrixValues
	for (int i = 0; i < 16; ++i) {
		cameraMatrixValues[i] = finalRotationMatrix[i / 4][i % 4];
	}

	UpdateCameraMatrix();

	//Runs if player loads a save or after a cinematic, resets the camera to the camera heading direction
	if (camResetRequested) {
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[0])) = -1;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[5])) = 1;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[10])) = 1;
	}

	// Letting the original code manage ingame camera position (not the uevr one) fixes the aim in car issue but 
	// also keeps the original audio listener position. Attempt to mitigate it by disabling the overwrite only when shooting in car.
	if (cameraModeIs == 55 || !playerManager->isInVehicle || !playerManager->shootFromCarInput)
	{
		glm::fvec3 offsetedPosition = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.returnedValue, upVector_params.returnedValue, rightVector_params.returnedValue, glm::fvec3(49.5, 0.0, 0.0));

		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[12])) = offsetedPosition.x * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[13])) = -offsetedPosition.y * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[14])) = offsetedPosition.z * 0.01f;
	}

	// Update some vars. The game's source code doesn't use the Unreal Engine unit scale. 
	// GTA SA original unit scale = UE Scale / 100.
	playerManager->actualPlayerPositionUE = socketLocation_params.Location;
	playerManager->actualPlayerHeadPositionUE = glm::fvec3(playerManager->actualPlayerPositionUE.x, playerManager->actualPlayerPositionUE.y, *(reinterpret_cast<float*>(memoryManager->playerHeadPositionAddresses[2])) * 100);
	cameraPositionUE = glm::fvec3(*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[0])) * 100,
		-*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[1])) * 100,
		*(reinterpret_cast<float*>(memoryManager->cameraPositionAddresses[2])) * 100);

	rightVectorUE = glm::fvec3(*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[0])),
		-*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[1])),
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[2])));

	upVectorUE = glm::fvec3(*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[8])),
		-*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[9])),
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[10])));
}

void CameraController::UpdateCameraMatrix()
{	
	if (settingsManager->debugMod) uevr::API::get()->log_info("UpdateCameraMatrix()");

	// Required for the camera weapon controls (to take photos ingame)
	if (cameraModeIs != 46 && cameraModeWas == 46)
	{
		memoryManager->ToggleAllMemoryInstructions(false);
	}
	if (cameraModeIs == 46 && cameraModeWas != 46)
		memoryManager->ToggleAllMemoryInstructions(true);
	if (cameraModeIs == 46)
	{
		return;
	}

	// Write the modified matrix back to memory
	for (int i = 0; i < 12; ++i) {
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[i])) = cameraMatrixValues[i];
	}
}

//Handles the VR camera height. Allows it to follow the crouch animation or to be correctly positioned in different camera mods.
void CameraController::ProcessHookedHeadPosition(float delta)
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("ProcessHookedHeadPosition()");

	if (cameraModeIs != 15 && cameraModeWas == 15 )
		keepCameraHeight = true ;
	uevr::API::get()->log_info("keepCameraHeight = %i", keepCameraHeight);

	//Workaround : Forces the VR camera height when player is in his garage.
	if (playerManager->isInVehicle || cameraModeIs == 15 || keepCameraHeight)
	{
		Utilities::Parameter_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
		setRelativeLocation_params.bSweep = false;
		setRelativeLocation_params.bTeleport = true;
		setRelativeLocation_params.NewLocation = playerManager->defaultPlayerHeadLocalPositionUE;
		playerManager->playerHead->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);
		
		keepCameraHeightTimer += keepCameraHeight ? delta : 0.0f;
		if (keepCameraHeightTimer >= keepCameraHeightTime)
		{
			keepCameraHeight = false;
			keepCameraHeightTimer = 0.0f;
		}
		return;
	}

	uevr::API::get()->log_info("keepCameraHeightTimer = %f", keepCameraHeightTimer);

	//Fixes the VR camera height when player handles the camera weapon.
	if (cameraModeIs == 46)
	{
		Utilities::Parameter_K2_SetWorldOrRelativeLocation setWorldLocation_params{};
		setWorldLocation_params.bSweep = false;
		setWorldLocation_params.bTeleport = true;
		setWorldLocation_params.NewLocation = cameraPositionUE;
		playerManager->playerHead->call_function(L"K2_SetWorldLocation", &setWorldLocation_params);
		return;
	}

	Utilities::Parameter_K2_SetWorldOrRelativeLocation setWorldLocation_params{};
	setWorldLocation_params.bSweep = false;
	setWorldLocation_params.bTeleport = true;
	setWorldLocation_params.NewLocation = playerManager->actualPlayerHeadPositionUE;
	playerManager->playerHead->call_function(L"K2_SetWorldLocation", &setWorldLocation_params);
}

void CameraController::FixUnderwaterView(bool enableFix)
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("FixUnderwaterView()");
	//API::get()->log_info("fixing underwater");
	static auto underwaterMaterial = uevr::API::get()->find_uobject(L"MaterialInstanceConstant /Game/Common/Materials/VGD/Instances/MI_Underwater_VGD.MI_Underwater_VGD");
	//API::get()->log_info("underwaterMaterial : %ls", underwaterMaterial->get_full_name().c_str());
	underwaterMaterial->set_bool_property(L"bHasStaticPermutationResource", enableFix);
	waterViewFixed = true;
}
