#include "CameraController.h"


void CameraController::ProcessCameraMatrix(float delta) {
	if (settingsManager->debugMod) uevr::API::get()->log_info("UpdatePlayerPositions()");

	// Update the camera position based on the head's socket location
	struct {
		uevr::API::FName InSocketName = uevr::API::FName(L"head");
		glm::fvec3 Location;
	} socketLocation_params;

	playerManager->playerHead->call_function(L"GetSocketLocation", &socketLocation_params);


	struct {
		glm::fvec3 ForwardVector;
	} forwardVector_params;

	struct {
		glm::fvec3 UpVector;
	} upVector_params;

	struct {
		glm::fvec3 RightVector;
	} rightVector_params;

	playerManager->playerHead->call_function(L"GetForwardVector", &forwardVector_params);
	playerManager->playerHead->call_function(L"GetUpVector", &upVector_params);
	playerManager->playerHead->call_function(L"GetRightVector", &rightVector_params);


	// Camera Matrix rotation ---------------------------------------------------
	UEVR_Vector2f rightJoystick{};
	uevr::API::get()->param()->vr->get_joystick_axis(uevr::API::get()->param()->vr->get_right_joystick_source(), &rightJoystick);

	// Create a full rotation matrix from the head's forward, up, and right vectors
	glm::mat4 headRotationMatrix = glm::mat4(1.0f);

	headRotationMatrix[0] = glm::vec4(-forwardVector_params.ForwardVector.x, forwardVector_params.ForwardVector.y, -forwardVector_params.ForwardVector.z, 0.0f);
	headRotationMatrix[1] = glm::vec4(-rightVector_params.RightVector.x, rightVector_params.RightVector.y, -rightVector_params.RightVector.z, 0.0f); // Right vector
	headRotationMatrix[2] = glm::vec4(upVector_params.UpVector.x, -upVector_params.UpVector.y, upVector_params.UpVector.z, 0.0f);      // Up vector 

	float joystickYaw = 0.0f;
	//float joystickPitch = 0.0f;

	if (playerManager->isInVehicle && !playerManager->wasInVehicle)
	{
		accumulatedJoystickRotation = glm::mat4(1.0f);
	}
	if ((!playerManager->isInVehicle && playerManager->wasInVehicle) || (!playerManager->isInVehicle && camResetRequested) || (cameraModeWas == 46 && cameraModeIs != 46))
	{
		accumulatedJoystickRotation = glm::mat4(1.0f);
		baseHeadRotation = headRotationMatrix;
	}

	// Calculate the delta rotation matrix. 
	glm::mat4 deltaRotationMatrix = playerManager->isInVehicle && cameraModeIs != 55 ? glm::inverse(accumulatedJoystickRotation) * headRotationMatrix : glm::inverse(accumulatedJoystickRotation) * baseHeadRotation;

	// Apply joystick input to adjust the local yaw rotation
	const float DEADZONE = 0.1f;
	if (abs(rightJoystick.x) > DEADZONE) {
		joystickYaw = -rightJoystick.x * delta * settingsManager->xAxisSensitivity;
		//joystickPitch = rightJoystick.y * delta * settingsManager->xAxisSensitivity;
	}

	// Convert joystick yaw to radians
	float yawRadians = joystickYaw * (M_PI / 180.0f);
	//float pitchRadians = joystickPitch * (M_PI / 180.0f);

	// Extract the right vector from the current rotation matrix
	//glm::vec3 currentRightVector = glm::vec3(
	//	cameraMatrixValues[0],
	//	cameraMatrixValues[1],
	//	cameraMatrixValues[2]
	//);

	// Create rotation matrices using the current right vector for pitch
	glm::mat4 joystickYawRotation = glm::rotate(glm::mat4(1.0f), yawRadians, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around world Y-axis
	//glm::mat4 joystickPitchRotation = glm::rotate(glm::mat4(1.0f), pitchRadians, currentRightVector); // Rotate around local right vector

	// Combine the delta rotation with the joystick rotations
	accumulatedJoystickRotation = /*cameraModeIs == 46
		? accumulatedJoystickRotation * joystickYawRotation * joystickPitchRotation
		: */accumulatedJoystickRotation * joystickYawRotation;

	// Combine the delta rotation with the joystick yaw rotation
	glm::mat4 totalDeltaRotation = accumulatedJoystickRotation * deltaRotationMatrix;

	// Combine the head rotation matrix with the joystick rotations
	glm::mat4 finalRotationMatrix = accumulatedJoystickRotation * totalDeltaRotation;

	// Copy the modified matrix back to cameraMatrixValues
	for (int i = 0; i < 16; ++i) {
		cameraMatrixValues[i] = finalRotationMatrix[i / 4][i % 4];
	}

	UpdateCameraMatrix();

	//If player loads a save or after a cinematic, reset the camera to the camera heading direction
	if (camResetRequested) {
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[0])) = -1;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[5])) = 1;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[10])) = 1;
	}

	//letting the original code manage ingame camera position (not the uevr one) fixes the aim in car issue but 
	// also keeps the original audio listener position. Attempt to mitigate it by disabling the overwrite only when shooting in car.

	if (cameraModeIs == 55 || !playerManager->isInVehicle || !playerManager->shootFromCarInput)
	{
		glm::fvec3 offsetedPosition = Utilities::OffsetLocalPositionFromWorld(socketLocation_params.Location, forwardVector_params.ForwardVector, upVector_params.UpVector, rightVector_params.RightVector, glm::fvec3(49.5, 0.0, 0.0));

		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[12])) = offsetedPosition.x * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[13])) = -offsetedPosition.y * 0.01f;
		*(reinterpret_cast<float*>(memoryManager->cameraMatrixAddresses[14])) = offsetedPosition.z * 0.01f;
	}


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

//Fix the camera not following the crouch animation
void CameraController::ProcessHookedHeadPosition()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("ProcessHookedHeadPosition()");

	if (playerManager->isInVehicle || cameraModeIs == 15)
	{
		Utilities::SceneComponent_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
		setRelativeLocation_params.bSweep = false;
		setRelativeLocation_params.bTeleport = true;
		setRelativeLocation_params.NewLocation = glm::fvec3(0.0f, 0.0f, 69.0f);
		playerManager->playerHead->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);
		return;
	}

	if (cameraModeIs == 46)
	{
		Utilities::SceneComponent_K2_SetWorldOrRelativeLocation setWorldLocation_params{};
		setWorldLocation_params.bSweep = false;
		setWorldLocation_params.bTeleport = true;
		setWorldLocation_params.NewLocation = glm::fvec3(cameraPositionUE.x, cameraPositionUE.y, cameraPositionUE.z);
		playerManager->playerHead->call_function(L"K2_SetWorldLocation", &setWorldLocation_params);
		return;
	}

	Utilities::SceneComponent_K2_SetWorldOrRelativeLocation setWorldLocation_params{};
	setWorldLocation_params.bSweep = false;
	setWorldLocation_params.bTeleport = true;
	setWorldLocation_params.NewLocation = playerManager->actualPlayerHeadPositionUE;
	playerManager->playerHead->call_function(L"K2_SetWorldLocation", &setWorldLocation_params);
}

	void CameraController::FixUnderwaterView(bool enableFix)
	{
		if (settingsManager->debugMod) uevr::API::get()->log_info("FixUnderwaterView()");
		//API::get()->log_info("fixing underwater");
		auto underwaterMaterial = uevr::API::get()->find_uobject(L"MaterialInstanceConstant /Game/Common/Materials/VGD/Instances/MI_Underwater_VGD.MI_Underwater_VGD");
		//API::get()->log_info("underwaterMaterial : %ls", underwaterMaterial->get_full_name().c_str());
		underwaterMaterial->set_bool_property(L"bHasStaticPermutationResource", enableFix);
		waterViewFixed = true;
	/*	const auto& scalarParameter = underwaterMaterial->get_property<API::TArray<API::UObject*>>(L"ScalarParameterValues");
		const auto& test = scalarParameter.data[0];
		API::get()->log_info("scalarParameter : %i", test->get_class());*/
		//test->
		/*API::get()->log_info("scalarParameter : %ls", scalarParameter.data[0].c_str());*/
		//MaterialInstanceConstant /Game/Common/Materials/VGD/Instances/MI_Underwater_VGD.MI_Underwater_VGD
		//Class /Script/Engine.MaterialInstanceConstant
	}
