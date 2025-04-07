#include "PlayerManager.h"

void PlayerManager::FetchPlayerUObjects()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("FetchPlayerUObjects()");
	playerController = uevr::API::get()->get_player_controller(0);
	if (playerController == nullptr)
		return;

	static auto gta_playerActor_c = uevr::API::get()->find_uobject<uevr::API::UClass>(L"Class /Script/GTABase.GTAPlayerActor");
	const auto& children = playerController->get_property<uevr::API::TArray<uevr::API::UObject*>>(L"Children");

	for (auto child : children) {
		if (child->is_a(gta_playerActor_c)) {
			auto playerActor = child;
			playerHead = playerActor->get_property<uevr::API::UObject*>(L"head");
			//API::get()->log_info("playerHead : %ls", playerHead->get_full_name().c_str());
			break;
		}
	}
}

void PlayerManager::DisablePlayerUObjectsHook()
{
	if (settingsManager->debugMod) uevr::API::get()->log_info("DisablePlayerUObjectsHook()");

	if (playerHead == nullptr)
		return;
	//Reset head position during cutscene
	Utilities::Parameter_K2_SetWorldOrRelativeLocation setRelativeLocation_params{};
	setRelativeLocation_params.bSweep = false;
	setRelativeLocation_params.bTeleport = true;
	setRelativeLocation_params.NewLocation = glm::fvec3(0.0f, 0.0f, 0.0f);
	playerHead->call_function(L"K2_SetRelativeLocation", &setRelativeLocation_params);
}