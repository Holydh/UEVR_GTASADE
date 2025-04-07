#pragma once
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_FORCE_QUAT_DATA_XYZW
#include "uevr/API.hpp"

class Utilities {
private:


public:
	static glm::fvec3 OffsetLocalPositionFromWorld(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets);

	static uevr::API::UObject* KismetMathLibrary;

	static void InitHelperClasses();

	#pragma pack(push, 1)
	struct FRotator {
		float Pitch;
		float Yaw;
		float Roll;
	};
#pragma pack(pop)

	struct ParameterGLMfvec3
	{
		glm::fvec3 returnedValue;
	};

	struct ParameterSocketLocation
	{
		uevr::API::FName InSocketName;
		glm::fvec3 Location;
	};

#pragma pack(push, 1)
	struct Parameter_K2_SetWorldOrRelativeLocation final
	{
	public:
		glm::fvec3 NewLocation;
		bool bSweep;
		uint8_t Pad_D[3];
		uint8_t Padding[0x8C];
		bool bTeleport;
		uint8_t Pad_9D[3];
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct Parameter_K2_SetWorldOrRelativeRotation final
	{
	public:
		FRotator NewRotation;
		bool bSweep;
		uint8_t Pad_D[3];
		uint8_t Padding[0x8C];
		bool bTeleport;
		uint8_t Pad_9D[3];
	};
#pragma pack(pop)
};