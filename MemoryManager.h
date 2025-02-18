#pragma once
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <unordered_map>
#include <cstdint>
#include <iostream>
#include <windows.h>


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
    uintptr_t cameraMatrixAddresses[16]{};
	uintptr_t aimForwardVectorAddresses[3] // x, y, z
	{};
	uintptr_t aimUpVectorAddresses[3] // x, y, z
	{};

	uintptr_t cameraPositionAddresses[3] // x, y, z
	{};

	uintptr_t weaponWheelOpenAddress;

	//borrowed empty addresses
	uintptr_t fpsCamInitializedAddress;
	uintptr_t equippedWeaponAddress;
	uintptr_t characterIsDuckingAddress;
	uintptr_t currentDuckOffsetAddress;
	uintptr_t characterHeadingAddress;
	uintptr_t characterIsInCarAddress;
	uintptr_t characterIsGettingInACarAddress;

	uintptr_t characterIsShootingAddress;
	uintptr_t cameraModeAddress;

	uintptr_t GetModuleBaseAddress(LPCTSTR moduleName);

    void AdjustAddresses();
	void NopVehicleRelatedMemoryInstructions();
	void RestoreVehicleRelatedMemoryInstructions();
	void ToggleAllMemoryInstructions(bool restoreInstructions);

	//void GetAllBytes();
	//void WriteBytesToIniFile(const char* header, const std::vector<std::pair<uintptr_t, size_t>>& addresses);
};

#endif // MEMORYMANAGER_H