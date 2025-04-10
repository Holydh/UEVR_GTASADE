#include "SettingsManager.h"

void SettingsManager::InitSettingsManager()
{
	GetAllConfigFilePaths();
	uevr::API::get()->log_info("%s", uevrConfigFilePath.c_str());
	UpdateUevrSettings();
	UpdatePluginSettings();
	CacheSettings();
}

void SettingsManager::UpdateUevrSettings()
{
	if (debugMod) uevr::API::get()->log_info("UpdateUevrSettings()");

	xAxisSensitivity = SettingsManager::GetFloatValueFromFile(uevrConfigFilePath, "VR_AimSpeed", 125.0f) * 10; //*10 because the base UEVR setting is too low as is 
	autoDecoupledPitchDuringCutscenes = SettingsManager::GetBoolValueFromFile(uevrConfigFilePath, "AutoDecoupledPitchDuringCutscenes", true);
	autoPitchAndLerpForFlight = SettingsManager::GetBoolValueFromFile(uevrConfigFilePath, "AutoPitchAndLerpSettingsForFlight", true);
	decoupledPitch = SettingsManager::GetBoolValueFromFile(uevrConfigFilePath, "VR_DecoupledPitch", true);
	lerpPitch = SettingsManager::GetBoolValueFromFile(uevrConfigFilePath, "VR_LerpCameraPitch", true);
	lerpRoll = SettingsManager::GetBoolValueFromFile(uevrConfigFilePath, "VR_LerpCameraRoll", true);
	uevr::API::get()->log_info("UEVR Settings Updated");
}

void SettingsManager::UpdatePluginSettings()
{
	if (debugMod) uevr::API::get()->log_info("UpdatePluginSettings()");
	autoDecoupledPitchDuringCutscenes = SettingsManager::GetBoolValueFromFile(pluginConfigFilePath, "AutoDecoupledPitchDuringCutscenes", true);
	autoPitchAndLerpForFlight = SettingsManager::GetBoolValueFromFile(pluginConfigFilePath, "AutoPitchAndLerpSettingsForFlight", true);
	uevr::API::get()->log_info("Plugin Settings Updated");
}

void SettingsManager::SetPitchAndLerpSettingsForFlight(bool enable)
{
	SetBoolValueToFile(uevrConfigFilePath, "VR_DecoupledPitch", enable ? storedDecoupledPitch : false);
	SetBoolValueToFile(uevrConfigFilePath, "VR_LerpCameraPitch", enable ? storedLerpPitch : false);
	SetBoolValueToFile(uevrConfigFilePath, "VR_LerpCameraRoll", enable ? storedLerpRoll : false);
	uevr::API::VR::reload_config();
}

void SettingsManager::CacheSettings()
{
	storedDecoupledPitch = decoupledPitch;
	storedLerpPitch = lerpPitch;
	storedLerpRoll = lerpRoll;
}

void SettingsManager::UpdateSettingsIfModified()
{
	CheckSettingsModificationAndUpdate(uevrConfigFilePath, true);
	CheckSettingsModificationAndUpdate(pluginConfigFilePath, false);
}

bool SettingsManager::CheckSettingsModificationAndUpdate(const std::string& filePath, bool uevr)
{
	if (debugMod) uevr::API::get()->log_info("UpdateSettingsIfModified()");

	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		uevr::API::get()->log_info("%s", "Failed to open config.txt file");
		return false;
	}

	FILETIME currentWriteTime;
	if (GetFileTime(hFile, NULL, NULL, &currentWriteTime))
	{
		if (CompareFileTime(uevr ? &uevrLastWriteTime : &pluginLastWriteTime, &currentWriteTime) != 0)
		{
			if (uevr)
			{
				uevrLastWriteTime = currentWriteTime;  // Update last write time
				UpdateUevrSettings();
			}
			else
			{
				pluginLastWriteTime = currentWriteTime; 
				UpdatePluginSettings();
			}
			CloseHandle(hFile);
			return true;  // File has been modified
		}
	}

	CloseHandle(hFile);
	return false;  // No change
}

void SettingsManager::SetBoolValueToFile(const std::string& filePath, const std::string& key, float value)
{
	if (debugMod) uevr::API::get()->log_info("SetBoolValueToFile()");

	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		uevr::API::get()->log_info("Failed to open config.txt file for reading");
		return;
	}

	DWORD bytesRead;
	char buffer[1024];
	std::string fileContents;

	while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		fileContents.append(buffer);
	}
	CloseHandle(hFile);

	size_t pos = fileContents.find(key);
	if (pos != std::string::npos)
	{
		size_t equalPos = fileContents.find('=', pos);
		if (equalPos != std::string::npos)
		{
			size_t endOfLine = fileContents.find_first_of("\r\n", equalPos);
			std::string before = fileContents.substr(0, equalPos + 1);
			std::string after = (endOfLine != std::string::npos) ? fileContents.substr(endOfLine) : "";

			// Replace value
			std::string newContents = before + (value ? "true" : "false") + after;
			
			// Write it back
			HANDLE hWriteFile = CreateFileA(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hWriteFile != INVALID_HANDLE_VALUE)
			{
				DWORD bytesWritten;
				WriteFile(hWriteFile, newContents.c_str(), static_cast<DWORD>(newContents.size()), &bytesWritten, NULL);
				CloseHandle(hWriteFile);
				uevr::API::get()->log_info("Updated %s to %s", key.c_str(), value ? "true" : "false");
			}
			else
			{
				uevr::API::get()->log_info("Failed to open config.txt file for writing");
			}
			return;
		}
	}
}

bool SettingsManager::GetBoolValueFromFile(const std::string& filePath, const std::string& key, float defaultValue)
{
	if (debugMod) uevr::API::get()->log_info("GetBoolValueFromFile()");

	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		uevr::API::get()->log_info("%s", "Failed to open config.txt file");
		return defaultValue;
	}

	DWORD bytesRead;
	char buffer[1024];  // Buffer to read the file content
	std::string fileContents;

	// Read the file into memory
	while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0'; // Null terminate the string
		fileContents.append(buffer);
	}
	CloseHandle(hFile);

	// Look for the key in the file contents
	size_t pos = fileContents.find(key);
	if (pos != std::string::npos)
	{
		size_t equalPos = fileContents.find('=', pos);
		if (equalPos != std::string::npos)
		{
			// Find the end of the line after the '='
			size_t endOfLine = fileContents.find_first_of("\r\n", equalPos);
			std::string valueStr = fileContents.substr(equalPos + 1, endOfLine - (equalPos + 1));

			// Trim whitespace
			valueStr.erase(0, valueStr.find_first_not_of(" \t\n\r"));
			valueStr.erase(valueStr.find_last_not_of(" \t\n\r") + 1);

			// Convert to lowercase
			std::transform(valueStr.begin(), valueStr.end(), valueStr.begin(), ::tolower);

			uevr::API::get()->log_info("Extracted value: %s", valueStr.c_str());

			if (valueStr == "true") return true;
			if (valueStr == "false") return false;

			uevr::API::get()->log_info("Error: Invalid bool value for key: %s", key.c_str());
		}
	}

	return defaultValue;  // Return default if the key is not found or invalid
}

float SettingsManager::GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue)
{
	if (debugMod) uevr::API::get()->log_info("GetFloatValueFromFile()");

	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		uevr::API::get()->log_info("%s", "Failed to open config.txt file");
		return defaultValue;
	}

	DWORD bytesRead;
	char buffer[1024];  // Buffer to read the file content
	std::string fileContents;

	// Read the file into memory
	while (ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		buffer[bytesRead] = '\0'; // Null terminate the string
		fileContents.append(buffer);
	}
	CloseHandle(hFile);

	// Look for the key in the file contents
	size_t pos = fileContents.find(key);
	if (pos != std::string::npos)
	{
		size_t equalPos = fileContents.find('=', pos);
		if (equalPos != std::string::npos)
		{
			std::string valueStr = fileContents.substr(equalPos + 1);
			try
			{
				return std::stof(valueStr); // Convert the string to float
			}
			catch (const std::invalid_argument&)
			{
				uevr::API::get()->log_info("Error: Invalid float value for key: %s", key.c_str());
				return defaultValue;
			}
		}
	}

	return defaultValue;  // Return default if the key is not found
}

std::string GetDLLDirectory()
{
	char path[MAX_PATH];
	HMODULE hModule = GetModuleHandleA("UEVR_GTASADE.dll"); // Get handle to the loaded DLL

	if (hModule)
	{
		GetModuleFileNameA(hModule, path, MAX_PATH); // Get full DLL path
		std::string fullPath = path;

		// Remove the DLL filename to get the directory
		size_t pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos)
		{
			return fullPath.substr(0, pos + 1); // Keep the trailing slash
		}
	}
	else
		uevr::API::get()->log_info("Failed to get module handle for UEVR_GTASADE.dll");

	return "Unknown";
}

void SettingsManager::GetAllConfigFilePaths()
{
	uevrConfigFilePath = GetConfigFilePath(true);
	pluginConfigFilePath = GetConfigFilePath(false);
}

std::string SettingsManager::GetConfigFilePath(bool uevr)
{
	if (debugMod) uevr::API::get()->log_info("GetConfigFilePath()");

	std::string fullPath = GetDLLDirectory();

	// Remove "SanAndreas\plugins\UEVR_GTASADE.dll" part, leaving "SanAndreas"
	size_t pos = fullPath.find_last_of("\\/");
	if (pos != std::string::npos)
	{
		fullPath = fullPath.substr(0, pos); // Remove "\plugins"
		pos = fullPath.find_last_of("\\/");
		if (pos != std::string::npos)
		{
			fullPath = fullPath.substr(0, pos + 1); // Keep "SanAndreas\"
		}
	}

	return fullPath + (uevr ? uevrSettingsFileName : pluginSettingsFileName); // Append "config.txt" or "UEVR_GTASADE_config.txt"
}