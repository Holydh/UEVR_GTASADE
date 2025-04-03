#include "Utilities.h"

uevr::API::UObject* Utilities::KismetMathLibrary = nullptr;
uevr::API::UObject* Utilities::KismetSystemLibrary = nullptr;
uevr::API::UObject* Utilities::AssetRegistryHelper = nullptr;
uevr::API::UObject* Utilities::AssetRegistry = nullptr;

void Utilities::InitHelperClasses()
{
	static auto kismetMathLibrary_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/Engine.KismetMathLibrary");
	KismetMathLibrary = kismetMathLibrary_c->get_class_default_object();
	static auto kismetSystemLibrary_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/Engine.KismetSystemLibrary");
	KismetSystemLibrary = kismetSystemLibrary_c->get_class_default_object();
	static auto assetRegistryHelper_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/AssetRegistry.AssetRegistryHelpers");
	AssetRegistryHelper = assetRegistryHelper_c->get_class_default_object();
	static auto assetRegistry_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/AssetRegistry.AssetRegistry");
	AssetRegistry = assetRegistry_c->get_class_default_object();
	uevr::API::get()->log_info("% ls", AssetRegistryHelper->get_full_name().c_str());
	uevr::API::get()->log_info("% ls", AssetRegistry->get_full_name().c_str());
}

glm::fvec3 Utilities::OffsetLocalPositionFromWorld(glm::fvec3 worldPosition, glm::fvec3 forwardVector, glm::fvec3 upVector, glm::fvec3 rightVector, glm::fvec3 offsets)
{
	// Apply the offsets along the local axes
	glm::fvec3 offset = (forwardVector * offsets.x) + (rightVector * offsets.y) + (upVector * offsets.z);

	// Calculate the new position
	glm::fvec3 pointWorldPosition = worldPosition + offset;

	return pointWorldPosition;
}