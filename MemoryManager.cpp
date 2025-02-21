﻿#include "MemoryManager.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>

DWORD PID;



std::vector<MemoryBlock> matrixInstructionsRotationAddresses = {
{0x111DE7E, 7, 0xf30f118b300800}, 
{0x111DE85, 1, 0x00},
{0x111DECC, 7, 0xf30f118b340800}, 
{0x111DED3, 1, 0x00},
{0x111DED9, 7, 0xf30f11b3380800}, 
{0x111DEE0, 1, 0x00},
{0x111DE5C, 7, 0xf3440f11834008}, 
{0x111DE63, 1, 0x00}, 
{0x111DE64, 1, 0x00},
{0x111DE3B, 7, 0xf30f11b3440800}, 
{0x111DE42, 1, 0x00},				  
{0x111DE68, 7, 0xf30f11bb480800}, 
{0x111DE6F, 1, 0x00},				  
{0x111DE75, 7, 0xf3440f11a35008}, 
{0x111DE7C, 1, 0x00}, 
{0x111DE7D, 1, 0x00},
{0x111DE8F, 7, 0xf3440f118b5408}, 
{0x111DE96, 1, 0x00}, 
{0x111DE97, 1, 0x00},
{0x111DE98, 7, 0xf3440f119b5808}, 
{0x111DE9F, 1, 0x00}, 
{0x111DEA0, 1, 0x00}
};
std::vector<MemoryBlock> matrixInstructionsPositionAddresses = {
	{0x111DEA5, 7, 0xf3440f11b36008},	
	{0x111DEAC, 1, 0x00},
	{0x111DEAD, 1, 0x00},
	{0x111DF57, 7, 0xf30f1183600800},
	{0x111DF5E, 1, 0x00},
	{0x111DEB3, 7, 0xf30f1183640800},
	{0x111DEBA, 1, 0x00},
	{0x111DF72, 7, 0xf30f1183640800},
	{0x111DF79, 1, 0x00},
	{0x111DEBE, 7, 0xf3440f11bb6808},
	{0x111DEC5, 1, 0x00},
	{0x111DEC6, 1, 0x00},
	{0x111DF8D, 7, 0xf30f1183680800},
	{0x111DF94, 1, 0x00},
};
std::vector<MemoryBlock> ingameCameraPositionInstructionsAddresses = {
	{0x1109F20, 3, 0xf20f11},
	{0x1109F23, 1, 0x06},
	{0x1109F96, 3, 0xf30f11},
	{0x1109F99, 1, 0x06},
	{0x110A28E, 3, 0xf30f11},
	{0x110A291, 1, 0x06},
	{0x11255AB, 3, 0xf30f11},
	{0x11255AE, 1, 0x03},
	{0x11070E2, 3, 0xf30f11},
	{0x11070E5, 1, 0x03},
	{0x110A3BD, 3, 0xf30f11},
	{0x110A3C0, 1, 0x06},
	{0x11080C6, 7, 0xf20f1106894608},
	{0x1109F24, 3, 0x894608},
	{0x1109FBC, 5, 0xf30f114608},
	{0x110A252, 5, 0xf3440f1146},
	{0x110A257, 1, 0x08},
	{0x110A2C0, 5, 0xf30f114608},
	{0x11255B4, 5, 0xf30f114b08},
	{0x11070FF, 5, 0xf30f114308},
	{0x110A3DD, 5, 0xf30f114608},
	{0x1108165, 5, 0xf3440f115e},
	{0x110816A, 1, 0x08},
	{0x1109FA4, 5, 0xf30f114604},
	{0x110A29C, 5, 0xf30f114604},
	{0x11255B3, 5, 0x04f30f114b},
	{0x11070F0, 5, 0xf30f114304},
	{0x110A3CB, 5, 0xf30f114604}
};
std::vector<MemoryBlock> aimingForwardVectorInstructionsAddresses = {
	{0x11090E8, 5, 0xf2410f1107},
	{0xAE0410, 5, 0xf3440f1101},
	{0x1109EA5, 5, 0xf2410f1107},
	{0x1105AAC, 7, 0xf20f1189100100},
	{0x1105AB3, 1, 0x00},
	{0x1107E3B, 7, 0xf20f1187100100},
	{0x1107E42, 1, 0x00},
	{0x1108E75, 5, 0xf2410f1107},
	{0xAE0406, 5, 0xf30f117104},
	{0x11090ED, 3, 0x418947},
	{0x11090F0, 1, 0x08},
	{0xAE040B, 5, 0xf30f117908},
	{0x1109EAA, 3, 0x418947},
	{0x1109EAD, 1, 0x08},
	{0x1105AC9, 5, 0x8981180100},
	{0x1105ACE, 1, 0x00},
	{0x1107E43, 5, 0x8987180100},
	{0x1107E48, 1, 0x00},
	{0x1108E7A, 3, 0x418947},
	{0x1108E7D, 1, 0x08}
};
std::vector<MemoryBlock> aimingUpVectorInstructionsAddresses = {
	{0x1105840, 5, 0xf20f118134}, 
	{0x1105845, 3, 0x010000},
	{0x1105A00, 5, 0xf20f118234}, 
	{0x1105A05, 3, 0x010000},
	{0x1105854, 5, 0x89813c0100}, 
	{0x1105859, 1, 0x00},
	{0x1105A08, 5, 0x89823c0100}, 
	{0x1105A0D, 1, 0x00},
};
std::vector<MemoryBlock> rocketLauncherAimingVectorInstructionsAddresses = {
	{0x110E71D, 5, 0x8987180100},
	{0x110E722, 1, 0x00},
	{0x110E70B, 7, 0xf20f1187100100},
	{0x110E712, 1, 0x00}
};
std::vector<MemoryBlock> sniperAimingVectorInstructionsAddresses = {
	{0x110E19E, 5, 0x8987180100},	
	{0x110E1A3, 1, 0x00},
	{0x110E196, 7, 0xf20f1187100100},	
	{0x110E19D, 1, 0x00}
};
std::vector<MemoryBlock> carAimingVectorInstructionsAddresses = {
	{0x110BB78, 3, 0x418945},	
	{0x110BB7B, 1, 0x08},
	{0x110C5A4, 3, 0x418945},	
	{0x110C5A7, 1, 0x08},
	{0x110C59E, 5, 0xf2410f1145},	
	{0x110C5A3, 1, 0x00},
	{0x110BB68, 5, 0xf2410f114d},	
	{0x110BB6D, 1, 0x00},
	{0x110CE81, 3, 0x894208},
	{0x110CE7A, 3, 0xf20f11}, 
	{0x110CE7D, 1, 0x02}
};



//Retrieves original bytes to manually set them as a variables in the code. std::vector<std::pair<uintptr_t, size_t>>
//void MemoryManager::GetAllBytes()
//{
//	WriteBytesToIniFile("aimingUpVectorInstructionsAddresses",aimingUpVectorInstructionsAddresses);
//}
//void MemoryManager::WriteBytesToIniFile(const char* header, const std::vector<std::pair<uintptr_t, size_t>>& addresses) {
//    // Open the file in append mode
//    std::ofstream file("originalBytes.ini", std::ios::app);
//    if (!file.is_open()) {
//        std::cerr << "Failed to open file: originalBytes.ini\n";
//        return;
//    }
//
//    // Write the header
//    file << "[" << header << "]\n";
//
//    for (const auto& [address, size] : addresses) {
//        // Allocate a buffer to hold the bytes
//        std::vector<uint8_t> bytes(size);
//
//        // Read the bytes from memory
//        if (ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(address + baseAddressGameEXE), bytes.data(), size, nullptr)) {
//            // Write the address and size to the file
//            file << "0x" << std::hex << address << ", " << size << ", 0x";
//
//            // Write the bytes in contiguous hexadecimal format
//            for (size_t i = 0; i < size; ++i) {
//                file << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
//            }
//            file << "\n";
//        } else {
//            std::cerr << "Failed to read memory at address: 0x" << std::hex << address << "\n";
//        }
//    }
//
//    file.close();
//    std::cout << "Bytes appended to originalBytes.ini under header: " << header << "\n";
//}

uintptr_t MemoryManager::crouchInstructionAddress = 0x1368515;
uintptr_t MemoryManager::shootInstructionAddress = 0x13EE170;

bool MemoryManager::isCrouching = false;
bool MemoryManager::isShooting = false;

// Struct for each breakpoint
struct BreakpointInfo {
    void* address;
    bool* flag;  // Pointer to the boolean variable to update
};

// Global breakpoints
BreakpointInfo breakpoints[4];  // DR0-DR3, we'll use DR0 and DR1

bool MemoryManager::SetHardwareBreakpoint(HANDLE hThread, int index, void* address, bool* flag) {
    if (index < 0 || index > 3) return false;  // DR0-DR3 are valid

    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if (!GetThreadContext(hThread, &ctx)) return false;

    // Assign the correct debug register (DR0 - DR3)
    switch (index) {
        case 0: ctx.Dr0 = (DWORD64)address; break;
        case 1: ctx.Dr1 = (DWORD64)address; break;
        case 2: ctx.Dr2 = (DWORD64)address; break;
        case 3: ctx.Dr3 = (DWORD64)address; break;
        default: return false;
    }

    // Enable the corresponding debug control bits
    ctx.Dr7 |= (1ULL << (index * 2)); // Enable breakpoint (L0, L1, L2, L3)

    if (!SetThreadContext(hThread, &ctx)) return false;

    // Store the breakpoint information
    breakpoints[index] = { address, flag };

    return true;
}

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* pException) {
    if (pException->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {  
        uintptr_t instructionAddress = (uintptr_t)pException->ExceptionRecord->ExceptionAddress;

        if (instructionAddress == MemoryManager::crouchInstructionAddress) {
            MemoryManager::isCrouching = true;
        } 
        else if (instructionAddress == MemoryManager::shootInstructionAddress) {
            MemoryManager::isShooting = true;
        }

		// Set Resume Flag (RF) to prevent infinite breakpoint triggering
        pException->ContextRecord->EFlags |= (1 << 16);  // Set RF bit in EFLAGS

        // Move execution to the next instruction to avoid freezing
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH; // Let other handlers process it if it's not our breakpoint
}

void MemoryManager::InstallBreakpoints() {
    HANDLE hThread = GetCurrentThread();

    // Set the breakpoints
    SetHardwareBreakpoint(hThread, 0, (void*)MemoryManager::crouchInstructionAddress, &MemoryManager::isCrouching);
    SetHardwareBreakpoint(hThread, 1, (void*)MemoryManager::shootInstructionAddress, &MemoryManager::isShooting);

    // Install exception handler
    exceptionHandlerHandle = AddVectoredExceptionHandler(1, ExceptionHandler);
}

void MemoryManager::RemoveBreakpoints() {
 // Clear hardware breakpoints
    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    HANDLE hThread = GetCurrentThread();  // Only applies to current thread (set for all if needed)
    GetThreadContext(hThread, &ctx);

    // Remove breakpoints by clearing DR0, DR1, and their control bits
    ctx.Dr0 = 0;
    ctx.Dr1 = 0;
    ctx.Dr7 &= ~(1 << 0); // Clear enable bit for DR0
    ctx.Dr7 &= ~(1 << 2); // Clear enable bit for DR1

    SetThreadContext(hThread, &ctx);

    // Remove the exception handler
    if (exceptionHandlerHandle) {
        RemoveVectoredExceptionHandler(exceptionHandlerHandle);
        exceptionHandlerHandle = nullptr;  // Prevent accidental double removal
    }
}

void MemoryManager::RemoveExceptionHandler() {
    static PVOID handler = nullptr;  // Store handler pointer globally
    if (handler) {
        RemoveVectoredExceptionHandler(handler);
        handler = nullptr;
    }
}

// Function to NOP a batch of addresses
void NopMemory(const std::vector<MemoryBlock>& memoryBlocks) {
	for (const auto& [address, size, bytes] : memoryBlocks) {
		DWORD oldProtect;
		VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);

		for (size_t i = 0; i < size; ++i) {
			uintptr_t currentAddr = address + i;
			*reinterpret_cast<uint8_t*>(currentAddr) = 0x90; // Write NOP
		}

		VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
	}
}

// Function to restore original bytes for a batch of addresses
void RestoreMemory(const std::vector<MemoryBlock>& memoryBlocks) {
	for (const auto& block : memoryBlocks) {
		DWORD oldProtect;
		VirtualProtect((LPVOID)block.address, block.size, PAGE_EXECUTE_READWRITE, &oldProtect);

		for (size_t i = 0; i < block.size; ++i) {
            *reinterpret_cast<uint8_t*>(block.address + i) = block.bytes[i];
        }

		VirtualProtect((LPVOID)block.address, block.size, oldProtect, &oldProtect);
	}
}

// Public method to print the original bytes
//void MemoryManager::PrintOriginalBytes() const {
//    for (const auto& [offset, originalByte] : originalBytes) {
//        std::cout << "Offset: 0x" << std::hex << offset
//                  << ", Value: 0x" << static_cast<int>(originalByte.value) << "\n";
//    }
//}

uintptr_t MemoryManager::GetModuleBaseAddress(LPCTSTR moduleName) {
	HMODULE hModule = GetModuleHandle(moduleName);
	if (hModule == nullptr) {
		//uevr::API::get()->log_info("Failed to get the base address of the module.");
		return 0;
	}
	return reinterpret_cast<uintptr_t>(hModule);
}

void MemoryManager::AdjustAddresses() {
		//for (auto& [key, value] : originalBytes) { value.address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : matrixInstructionsRotationAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : matrixInstructionsPositionAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : ingameCameraPositionInstructionsAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : aimingForwardVectorInstructionsAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : aimingUpVectorInstructionsAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : rocketLauncherAimingVectorInstructionsAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : sniperAimingVectorInstructionsAddresses) { address += baseAddressGameEXE; }
	for (auto& [address, size, bytes] : carAimingVectorInstructionsAddresses) { address += baseAddressGameEXE; }

	for (auto& address : cameraMatrixAddresses) address += baseAddressGameEXE;
	for (auto& address : aimForwardVectorAddresses) address += baseAddressGameEXE;
	for (auto& address : aimUpVectorAddresses) address += baseAddressGameEXE;
	for (auto& address : cameraPositionAddresses) address += baseAddressGameEXE;
	for (auto& address : playerHeadPositionAddresses) address += baseAddressGameEXE;

	playerHasControl += baseAddressGameEXE;
	shootInstructionAddress += baseAddressGameEXE;
	currentCrouchOffsetAddress += baseAddressGameEXE;
	characterIsInCarAddress += baseAddressGameEXE;
	weaponWheelOpenAddress += baseAddressGameEXE;

	cameraModeAddress += baseAddressGameEXE;

	crouchInstructionAddress += baseAddressGameEXE;
}

void MemoryManager::NopVehicleRelatedMemoryInstructions()
{
	NopMemory(matrixInstructionsPositionAddresses);
	NopMemory(ingameCameraPositionInstructionsAddresses);
	NopMemory(aimingForwardVectorInstructionsAddresses);
	NopMemory(rocketLauncherAimingVectorInstructionsAddresses);
	NopMemory(sniperAimingVectorInstructionsAddresses);
};

void MemoryManager::RestoreVehicleRelatedMemoryInstructions()
{
	RestoreMemory(matrixInstructionsPositionAddresses);
	RestoreMemory(ingameCameraPositionInstructionsAddresses);
	RestoreMemory(aimingForwardVectorInstructionsAddresses);
	RestoreMemory(rocketLauncherAimingVectorInstructionsAddresses);
	RestoreMemory(sniperAimingVectorInstructionsAddresses);
}

void MemoryManager::ToggleAllMemoryInstructions(bool restoreInstructions)
{
	if (!restoreInstructions)
	{
		NopMemory(matrixInstructionsRotationAddresses);
		NopMemory(matrixInstructionsPositionAddresses);
		NopMemory(ingameCameraPositionInstructionsAddresses);
		NopMemory(aimingForwardVectorInstructionsAddresses);
		NopMemory(rocketLauncherAimingVectorInstructionsAddresses);
		NopMemory(sniperAimingVectorInstructionsAddresses);
		NopMemory(carAimingVectorInstructionsAddresses);
	}
	if (restoreInstructions)
	{
		RestoreMemory(matrixInstructionsRotationAddresses);
		RestoreMemory(matrixInstructionsPositionAddresses);
		RestoreMemory(ingameCameraPositionInstructionsAddresses);
		RestoreMemory(aimingForwardVectorInstructionsAddresses);
		RestoreMemory(rocketLauncherAimingVectorInstructionsAddresses);
		RestoreMemory(sniperAimingVectorInstructionsAddresses);
		RestoreMemory(carAimingVectorInstructionsAddresses);
	}
}

// Function to adjust addresses and store original bytes
//void AdjustAddresses(std::vector<std::pair<uintptr_t, size_t>>& addresses) const {
//	auto processAddresses = [&](std::vector<std::pair<uintptr_t, size_t>>& addresses) {
//		for (auto& [address, size] : addresses) {
//			uintptr_t offset = address;
//			uintptr_t finalAddress = address + baseAddressGameEXE; // Calculate final address
//			address = finalAddress; // Update the address in the vector

//			DWORD oldProtect;
//			VirtualProtect((LPVOID)finalAddress, size, PAGE_EXECUTE_READWRITE, &oldProtect);

//			for (size_t i = 0; i < size; ++i) {
//				uintptr_t currentAddr = finalAddress + i;
//				if (originalBytes.find(offset) == originalBytes.end()) {
//					originalBytes[offset] = { offset, *reinterpret_cast<uint8_t*>(currentAddr) };
//				}
//			}

//			VirtualProtect((LPVOID)finalAddress, size, oldProtect, &oldProtect);
//		}
//		};

//	// Process each address vector
//	processAddresses(matrixInstructionsRotationAddresses);
//	processAddresses(matrixInstructionsPositionAddresses);
//	processAddresses(ingameCameraPositionInstructionsAddresses);
//	processAddresses(aimingForwardVectorInstructionsAddresses);
//	processAddresses(aimingUpVectorInstructionsAddresses);
//	processAddresses(rocketLauncherAimingVectorInstructionsAddresses);
//	processAddresses(sniperAimingVectorInstructionsAddresses);
//	processAddresses(carAimingVectorInstructionsAddresses);

//	for (auto& address : cameraMatrixAddresses) address += baseAddressGameEXE;
//	for (auto& address : aimForwardVectorAddresses) address += baseAddressGameEXE;
//	for (auto& address : aimUpVectorAddresses) address += baseAddressGameEXE;
//	for (auto& address : cameraPositionAddresses) address += baseAddressGameEXE;

//	fpsCamInitializedAddress += baseAddressGameEXE;
//	equippedWeaponAddress += baseAddressGameEXE;
//	characterHeadingAddress += baseAddressGameEXE;
//	characterIsInCarAddress += baseAddressGameEXE;
//	characterIsGettingInACarAddress += baseAddressGameEXE;
//	characterIsShootingAddress += baseAddressGameEXE;
//	characterIsDuckingAddress += baseAddressGameEXE;
//	currentDuckOffsetAddress += baseAddressGameEXE;
//	weaponWheelOpenAddress += baseAddressGameEXE;
//	cameraModeAddress += baseAddressGameEXE;
//}
