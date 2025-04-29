#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include "uevr/API.hpp"

class SettingsManager {
public:
	bool debugMod = false;

	bool leftHandedMode = false;

	float xAxisSensitivity = 125.0f;
	float joystickDeadzone = 0.1f;

	enum CameraModeSettings {
		Cutscene = 0,
		OnFoot = 1,
		DrivingCar = 2,
		DrivingBike = 3,
		Flying = 4
	};

	void InitSettingsManager();
	void GetAllConfigFilePaths();
	void UpdateSettingsIfModifiedByPlayer();
	void ApplyCameraSettings(CameraModeSettings cameraModeSettings);

private:
	bool CheckSettingsModificationAndUpdate(const std::string& filePath, bool uevr);
	std::string GetConfigFilePath(bool uevr);
	std::string uevrSettingsFileName = "config.txt";
	std::string pluginSettingsFileName = "UEVR_GTASADE_config.txt";

	std::string uevrConfigFilePath;
	FILETIME uevrLastWriteTime;
	bool uevrConfigWroteByPlugin = false;
	std::string pluginConfigFilePath;
	FILETIME pluginLastWriteTime;

	//Would need some rework if lots of config values to read. Now it opens the config.txt file each time we call these :
	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	bool GetBoolValueFromFile(const std::string& filePath, const std::string& key, bool defaultValue);
	void SetBoolValueToFile(const std::string& filePath, const std::string& key, bool defaultValue);
	void SetIntValueToFile(const std::string& filePath, const std::string& key, int value);

	void FetchUevrSettings(bool writeToPlugin);
	void FetchPluginSettings();
	void WriteChangedSettingsToPluginConfigFile();

	bool uevr_DecoupledPitch = false;
	bool uevr_LerpPitch = false;
	bool uevr_LerpRoll = false;
	bool uevr_LerpYaw = false;

	bool onFoot_DecoupledPitch = false;
	bool onFoot_LerpPitch = false;
	bool onFoot_LerpRoll = false;
	bool onFoot_LerpYaw = false;

	bool drivingCar_DecoupledPitch = false;
	bool drivingCar_LerpPitch = false;
	bool drivingCar_LerpRoll = false;
	bool drivingCar_LerpYaw = false;
		 
	bool drivingBike_DecoupledPitch = false;
	bool drivingBike_LerpPitch = false;
	bool drivingBike_LerpRoll = false;
	bool drivingBike_LerpYaw = false;

	bool flying_DecoupledPitch = false;
	bool flying_LerpPitch = false;
	bool flying_LerpRoll = false;
	bool flying_LerpYaw = false;

	CameraModeSettings cameraModeSettings = CameraModeSettings::Cutscene;
};