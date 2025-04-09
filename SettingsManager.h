#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include "uevr/API.hpp"

class SettingsManager {
private:


public:
	bool debugMod = false;

	bool autoPitchAndLerpForFlight = false;
	bool autoDecoupledPitchDuringCutscenes = false;
	bool decoupledPitch = false;
	bool storedDecoupledPitch = false;
	bool lerpPitch = false;
	bool storedLerpPitch = false;
	bool lerpRoll = false;
	bool storedLerpRoll = false;

	std::string uevrConfigFilePath;
	FILETIME uevrLastWriteTime;
	std::string pluginConfigFilePath;
	FILETIME pluginLastWriteTime;

	float xAxisSensitivity = 125.0f;

	std::string GetConfigFilePath(bool uevr);
	bool UpdateSettingsIfModified(const std::string& filePath, bool uevr);
	void UpdateUevrSettings();
	void UpdatePluginSettings();

	void CacheSettings(); // For situations where we need to modify UEVR config temporarily
	void SetPitchAndLerpSettingsForFlight(bool enable);

	//Would need some rework if lots of config values to read. Now it opens the config.txt file each time we call these :
	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	bool GetBoolValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	void SetBoolValueToFile(const std::string& filePath, const std::string& key, float defaultValue);
};