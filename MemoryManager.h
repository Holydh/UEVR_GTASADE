#pragma once
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <windows.h>
#include <array>

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
public:
    uintptr_t baseAddressGameEXE;
	std::array<uintptr_t, 16> cameraMatrixAddresses = {
		0x53E2C00, 0x53E2C04, 0x53E2C08, 0x53E2C0C,
		0x53E2C10, 0x53E2C14, 0x53E2C18, 0x53E2C1C,
		0x53E2C20, 0x53E2C24, 0x53E2C28, 0x53E2C2C,
		0x53E2C30, 0x53E2C34, 0x53E2C38, 0x53E2C3C
	};
	std::array<uintptr_t, 3> aimForwardVectorAddresses // x, y, z
	{
		0x53E2668, 0x53E266C, 0x53E2670
	};
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

	uintptr_t weaponWheelOpenAddress = 0x507C580;

	//without Cleo delete
	uintptr_t playerHasControl = 0x53E8840;
	uintptr_t cameraModeAddress = 0x53E2580;
	uintptr_t characterIsInCarAddress = 0x51B39D4;

	//borrowed empty addresses
	uintptr_t currentCrouchOffsetAddress = 0x53DACDA;


	//Cleo set address
	//uintptr_t characterIsDuckingAddress = 0x53DAD11;
	//uintptr_t characterIsGettingInACarAddress = 0x53DAD01;
	

	//uintptr_t equippedWeaponAddress = 0x53DACC7;

	//Without Cleo
	static uintptr_t crouchInstructionAddress;
	static uintptr_t shootInstructionAddress;

	uintptr_t GetModuleBaseAddress(LPCTSTR moduleName);

	
	static std::atomic<bool> playerIsCrouching;
	static std::atomic<bool> playerIsShooting;

	static LONG WINAPI CrouchExceptionHandler(EXCEPTION_POINTERS* pException);
	static LONG WINAPI ShootExceptionHandler(EXCEPTION_POINTERS* pException);

	void HookCrouchFunction();
	void HookShootFunction();

	void ResetCrouchStatus();
	void ResetShootStatus();

    void AdjustAddresses();
	void NopVehicleRelatedMemoryInstructions();
	void RestoreVehicleRelatedMemoryInstructions();
	void ToggleAllMemoryInstructions(bool restoreInstructions);

	//void GetAllBytes();
	//void WriteBytesToIniFile(const char* header, const std::vector<std::pair<uintptr_t, size_t>>& addresses);
};

#endif // MEMORYMANAGER_H