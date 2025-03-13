#include "SettingsManager.h"
#include <string>
#include <windows.h>
#include "uevr/API.hpp"

void SettingsManager::UpdateSettings()
{
	xAxisSensitivity = SettingsManager::GetFloatValueFromFile(configFilePath, "VR_AimSpeed", 125.0f) * 10;
}

bool SettingsManager::UpdateSettingsIfModified(const std::string& filePath)
{
	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		uevr::API::get()->log_info("%s", "Failed to open config.txt file");
		return false;
	}

	FILETIME currentWriteTime;
	if (GetFileTime(hFile, NULL, NULL, &currentWriteTime))
	{
		if (CompareFileTime(&lastWriteTime, &currentWriteTime) != 0)
		{
			lastWriteTime = currentWriteTime;  // Update last write time
			UpdateSettings();
			CloseHandle(hFile);
			return true;  // File has been modified
		}
	}

	CloseHandle(hFile);
	return false;  // No change
}

float SettingsManager::GetFloatValueFromFile(const std::string& filePath, const std::string& key, float defaultValue)
{
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
				uevr::API::get()->log_info("%s", "%s", "Error: Invalid float value for key: ", key.c_str());
				return defaultValue;
			}
		}
	}

	return defaultValue;  // Return default if the key is not found
}

std::string GetDLLDirectory()
{
	char path[MAX_PATH];
	HMODULE hModule = GetModuleHandleA("VRmod.dll"); // Get handle to the loaded DLL

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

	return "Unknown";
}

std::string SettingsManager::GetConfigFilePath()
{
	std::string fullPath = GetDLLDirectory();

	// Remove "SanAndreas\plugins\VRmod.dll" part, leaving "SanAndreas"
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

	return fullPath + "config.txt"; // Append "config.txt"
}