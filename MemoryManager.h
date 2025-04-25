#pragma once

#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <windows.h>
#include <array>

#include "SettingsManager.h"

// Define the OriginalByte struct
struct OriginalByte {
    uintptr_t address; // The memory address (offset)
    uint8_t value;     // The original byte value
};

struct MemoryBlock {
    uintptr_t address;
    size_t size;
    std::vector<uint8_t> bytes; // Stores the block of bytes

	// Constructor to initialize from address, size, and a contiguous hexadecimal string
	MemoryBlock(uintptr_t addr, size_t sz, uint64_t hexValue)
		: address(addr), size(sz) {
		// Convert the hexValue into a vector of bytes
		for (size_t i = 0; i < size; ++i) {
			uint8_t byte = static_cast<uint8_t>((hexValue >> (8 * (size - 1 - i))) & 0xFF);
			bytes.push_back(byte);
		}
	}
};

// MemoryManager class
class MemoryManager {
private:
	SettingsManager* const settingsManager;

	uintptr_t GetModuleBaseAddress(LPCTSTR moduleName);
    void AdjustAddresses();
	uintptr_t baseAddressGameEXE = NULL;
	void* exceptionHandlerHandle = nullptr;  // Store the handler so we can remove it later

public:
	MemoryManager(SettingsManager* sm) : settingsManager(sm) {};
	static std::array<uintptr_t, 16> cameraMatrixAddresses;

	std::array<uintptr_t, 3> aimForwardVectorAddresses 	{ 0x53E2668, 0x53E266C, 0x53E2670 }; // x, y, z
	uintptr_t xAxisSpraysAimAddress = 0x53E2558;
	std::array<uintptr_t, 3> cameraPositionAddresses { 0x53E2674, 0x53E2678, 0x53E267C }; // x, y, z
	std::array<uintptr_t, 3> playerHeadPositionAddresses { 0x58013D8, 0x58013DC, 0x58013E0 }; // x, y, z
	std::array<uintptr_t, 3> playerPositionAddresses { 0x5067948, 0x506794C, 0x5067950 }; // x, y, z

	static uintptr_t playerShootInstructionAddress;
	static uintptr_t playerShootCam45InstructionAddress;

	uintptr_t cameraModeAddress = 0x53E2580;
	uintptr_t playerIsInControlAddress = 0x53E8840;
	uintptr_t playerIsInVehicleAddress = 0x51B39D4;
	uintptr_t vehicleTypeAddress = 0x5031278;
	uintptr_t playerShootFromCarInputAddress = 0x50251A8;
	uintptr_t weaponWheelDisplayedAddress = 0x507C580;
	//uintptr_t cutscenePlayingAddress = 0x53E254C;

	void InitMemoryManager();
	void ToggleAllMemoryInstructions(bool enableOriginalInstructions);
	void ToggleHeliCanonCameraModMemoryInstructions(bool enableOriginalInstructions);
	void NopVehicleRelatedMemoryInstructions();
	void RestoreVehicleRelatedMemoryInstructions();
	bool vehicleRelatedMemoryInstructionsNoped = true;

	static bool isShooting;

	void InstallBreakpoints();
	bool SetHardwareBreakpoint(HANDLE hThread, int index, void* address, bool* flag);
	void RemoveBreakpoints();
	void RemoveExceptionHandler();

	//void GetAllBytes();
	//void WriteBytesToIniFile(const char* header, const std::vector<std::pair<uintptr_t, size_t>>& addresses);
};