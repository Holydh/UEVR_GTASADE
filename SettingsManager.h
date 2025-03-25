#pragma once
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H
#include <iostream>
#include <windows.h>
#include <string>
#include "uevr/API.hpp"

class SettingsManager {
private:


public:
	bool debugMod = false;

	std::string configFilePath;
	FILETIME lastWriteTime;

	float xAxisSensitivity = 125.0f;

	std::string GetConfigFilePath();
	bool UpdateSettingsIfModified(const std::string& filePath);
	void UpdateSettings();
	float GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue);
};

#endif