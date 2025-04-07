#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H
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

	struct ParameterGLMfvec3
	{
		glm::fvec3 returnedValue;
	};

	struct ParameterSocketLocation
	{
		uevr::API::FName InSocketName;
		glm::fvec3 Location;
	};

	struct FQuat {
		float x;
		float y;
		float z;
		float w;
	};

	struct FVector {
		float x, y, z;
	};

	enum class EBoneSpaces
	{
		WorldSpace = 0,
		ComponentSpace = 1,
		EBoneSpaces_MAX = 2,
	};



#pragma pack(push, 1)
	struct FTransform {
		glm::fquat Rotation;
		glm::fvec3 Location;
		uint8_t Padding[4];
		glm::fvec3 Scale3D;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct FRotator {
		float Pitch;
		float Yaw;
		float Roll;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_GetBoneTransformByName
	{
		uevr::API::FName BoneName;
		EBoneSpaces BoneSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_GetSocketTransform {
		uevr::API::FName InSocketName;
		uint8_t ERelativeTransformSpace;
		uint8_t Padding[7];
		struct FTransform Transform;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct SceneComponent_K2_AddLocalOffset final
	{
	public:
		glm::fvec3 DeltaLocation;
		bool bSweep;
		uint8_t Pad_D[3];
		uint8_t Padding[0x8C];
		bool bTeleport;
		uint8_t Pad_9D[3];
	};
#pragma pack(pop)

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
	struct SceneComponent_K2_SetWorldOrRelativeRotation final
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

#endif