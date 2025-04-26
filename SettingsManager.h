#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include "uevr/API.hpp"

class SettingsManager {
private:
	bool CheckSettingsModificationAndUpdate(const std::string& filePath, bool uevr);
	std::string GetConfigFilePath(bool uevr);
	std::string uevrSettingsFileName = "config.txt";
	std::string pluginSettingsFileName = "UEVR_GTASADE_config.txt";

	std::string uevrConfigFilePath;
	FILETIME uevrLastWriteTime;
	std::string pluginConfigFilePath;
	FILETIME pluginLastWriteTime;

	//Would need some rework if lots of config values to read. Now it opens the config.txt file each time we call these :
	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	bool GetBoolValueFromFile(const std::string& filePath, const std::string& key, bool defaultValue);
	void SetBoolValueToFile(const std::string& filePath, const std::string& key, bool defaultValue);
	void SetIntValueToFile(const std::string& filePath, const std::string& key, int value);

	void UpdateUevrSettings();
	void UpdatePluginSettings();

	bool decoupledPitch = false;
	bool lerpPitch = false;
	bool lerpRoll = false;

public:
	bool debugMod = false;

	bool leftHandedMode = false;
	bool autoPitchAndLerpForFlight = false;
	bool autoDecoupledPitchDuringCutscenes = false;
	bool autoOrientationMode = false;
	bool storedDecoupledPitch = false;
	bool storedLerpPitch = false;
	bool storedLerpRoll = false;

	float xAxisSensitivity = 125.0f;
	float joystickDeadzone = 0.1f;

	void InitSettingsManager();
	void GetAllConfigFilePaths();
	void UpdateSettingsIfModified();

	void CacheSettings(); // For situations where we need to modify UEVR config temporarily
	void SetPitchAndLerpSettingsForFlight(bool enable);
	void SetOrientationMethod(bool inVehicle);
};