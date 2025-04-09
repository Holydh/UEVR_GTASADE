#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include "uevr/API.hpp"

class SettingsManager {
private:


public:
	bool debugMod = false;

	bool decoupledPitch = false;
	bool storedDecoupledPitch = false;
	bool lerpPitch = false;
	bool storedLerpPitch = false;
	bool lerpRoll = false;
	bool storedLerpRoll = false;

	std::string configFilePath;
	FILETIME lastWriteTime;

	float xAxisSensitivity = 125.0f;

	std::string GetConfigFilePath();
	bool UpdateSettingsIfModified(const std::string& filePath);
	void UpdateSettings();

	void CacheSettings(); // For situations where we need to modify UEVR config temporarily
	void SetPitchAndLerpSettingsForFlight(bool enable);

	//Would need some rework if lots of config values to read. Now it opens the config.txt file each time we call these :
	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	bool GetBoolValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
	void SetBoolValueToFile(const std::string& filePath, const std::string& key, float defaultValue);
};