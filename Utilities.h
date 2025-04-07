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
		float pitch;
		float yaw;
		float roll;
	};
#pragma pack(pop)

	struct ParameterSingleBool
	{
		bool boolValue = false;
	};

	struct ParameterSingleVector3
	{
		glm::fvec3 vec3Value{};
	};

	struct ParameterGetSocketLocation
	{
		uevr::API::FName inSocketName;
		glm::fvec3 outLocation;
	};

	struct ParameterDetachFromParent
	{
		bool maintainWorldPosition;
		bool callModify;
	};

	struct ParameterFindLookAtRotation
	{
		glm::fvec3 start;
		glm::fvec3 target;
		Utilities::FRotator outRotation;
	};

//Parameters taken from Dumper7 dump of the game. Padding allows to align with the size Unreal Engine expects for each parameters.
#pragma pack(push, 1)
	struct Parameter_K2_SetWorldOrRelativeLocation final
	{
	public:
		glm::fvec3 newLocation;
		bool bSweep;
		uint8_t pad_D[3];
		uint8_t padding[0x8C];
		bool bTeleport;
		uint8_t pad_9D[3];
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct Parameter_K2_SetWorldOrRelativeRotation final
	{
	public:
		FRotator newRotation;
		bool bSweep;
		uint8_t pad_D[3];
		uint8_t padding[0x8C];
		bool bTeleport;
		uint8_t pad_9D[3];
	};
#pragma pack(pop)
};