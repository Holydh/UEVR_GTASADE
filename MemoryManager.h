#pragma once
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

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
	SettingsManager* settingsManager;
	void* exceptionHandlerHandle = nullptr;  // Store the handler so we can remove it later

public:
	MemoryManager(SettingsManager* sm) : settingsManager(sm) {};

	uintptr_t baseAddressGameEXE = NULL;

	static std::array<uintptr_t, 16> cameraMatrixAddresses;

	std::array<uintptr_t, 3> aimForwardVectorAddresses // x, y, z
	{
		0x53E2668, 0x53E266C, 0x53E2670
	};
	uintptr_t xAxisSpraysAimAddress = 0x53E2558;

	std::array<uintptr_t, 3> aimUpVectorAddresses // x, y, z
	{
		0x53E268C, 0x53E2690, 0x53E2694
	};

	std::array<uintptr_t, 3> cameraPositionAddresses // x, y, z
	{
		0x53E2674, 0x53E2678, 0x53E267C
	};

	std::array<uintptr_t, 3> playerHeadPositionAddresses // x, y, z
	{
		0x58013D8, 0x58013DC, 0x58013E0
	};

	static std::array<float, 12> matrixAimCalculatedValues;
	static int cameraMode;

	uintptr_t weaponWheelDisplayedAddress = 0x507C580;

	//without Cleo delete
	uintptr_t playerIsInControlAddress = 0x53E8840;
	uintptr_t cameraModeAddress = 0x53E2580;
	uintptr_t playerIsInVehicleAddress = 0x51B39D4;

	//Cleo set address
	//uintptr_t characterIsDuckingAddress = 0x53DAD11;
	//uintptr_t characterIsGettingInACarAddress = 0x53DAD01;
	//uintptr_t equippedWeaponAddress = 0x53DACC7;
	static uintptr_t shootInstructionAddress;
	//static uintptr_t cameraShootInstructionAddress;
	uintptr_t playerShootFromCarInputAddress = 0x50251A8;


	uintptr_t GetModuleBaseAddress(LPCTSTR moduleName);

	//void ResetShootStatus();

    void AdjustAddresses();
	void NopVehicleRelatedMemoryInstructions();
	void RestoreVehicleRelatedMemoryInstructions();
	void ToggleAllMemoryInstructions(bool enableOriginalInstructions);
	
	static bool isShooting;
	//static bool isShootingCamera;

	void InstallBreakpoints();
	bool SetHardwareBreakpoint(HANDLE hThread, int index, void* address, bool* flag);
	void RemoveBreakpoints();
	void RemoveExceptionHandler();

	//void GetAllBytes();
	//void WriteBytesToIniFile(const char* header, const std::vector<std::pair<uintptr_t, size_t>>& addresses);
};

#endif // MEMORYMANAGER_H