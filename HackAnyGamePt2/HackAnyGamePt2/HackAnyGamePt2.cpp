// HackAnyGamePt2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <Windows.h>
#include "proc.h"
#include "mem.h"

// Main function based on this video https://www.youtube.com/watch?v=UMt1daXknes&list=PLt9cUwGw6CYHKBH5OoR8M2ELGlNlrgBKl&index=15
// Creates an external hack that toggles using 1,2,3 and insert
int main() {
	HANDLE hProcess = 0;
	uintptr_t moduleBase = 0, localPlayerPtr = 0, healthAddr = 0;
	bool bHealth = false, bAmmo = false, bRecoil = false;
	const int newValue = 1000;

	DWORD procId = GetProcId(L"ac_client.exe");

	if (procId) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);	// Allows us to read, write, execute process, etc.
		moduleBase = GetModuleBaseAdress(procId, L"ac_client.exe");	// Get module base address of Assault Cube
		localPlayerPtr = moduleBase + 0x10f4f4;
		healthAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0xf8 });	// Get the actual dynamic address
	}
	else {
		std::cout << "Process not found, press enter to exit\n";
		getchar();
		return 0;
	}
	DWORD dwExit = 0;
	
	// GetExitCodeProcess ensures the process is still running
	while (GetExitCodeProcess(hProcess, &dwExit) && dwExit == STILL_ACTIVE) {
		// Different checks if you press numpad 1, 2, 3. 
		if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
			bHealth = !bHealth;			// Turn off health decrement
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
			bAmmo = !bAmmo;
			if (bAmmo) {
				// ff 06 = inc [esi] (set the ammo to increment)
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x06", 2, hProcess);
			}
			else {
				// ff 0e = dec [esi] (turn off ammo hack and decrement as usual)
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x0E", 2, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
			bRecoil = !bRecoil;

			if (bRecoil) {
				// Turn off code to recoil weapon
				mem::NopEx((BYTE*)(moduleBase + 0x63786), 10, hProcess);
			}
			else {
				// Revert nop bytes with prior code
				mem::PatchEx((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2", 10, hProcess);
			}
		}
		// Turn off hack
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			return 0;
		}

		// Continous write or freeze
		if (bHealth) {
			mem::PatchEx((BYTE*)healthAddr, (BYTE*)&newValue, sizeof(newValue), hProcess);
		}
		Sleep(10);
	}

	std::cout << "Process not found, press enter to exit\n";
	getchar();
	return 0;
}

//// Program aims to read and change the current ammo value, based on Assault Cube's offsets found in Cheat Engine
//int main()
//{
//    // Get ProcId of the target process, in this case, Assault Cube
//    // L macro is put in front to convert string literal/char array into unicode
//    DWORD procId = GetProcId(L"ac_client.exe");
//
//    // Get module base address
//    uintptr_t moduleBase = GetModuleBaseAdress(procId, L"ac_client.exe");
//
//    // Get Handle to process
//    HANDLE hProcess = 0;
//    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);   // Allows us to read, write, execute process, etc.
//
//    // Resolve base address of the pointer chain
//    uintptr_t dynamicPtrBaseAddr = moduleBase + 0x10f4f4;
//
//    // Resolve our ammo pointer chain
//    std::vector<unsigned int> ammoOffsets = { 0x374, 0x14, 0x0 };
//    uintptr_t ammoAddr = FindDMAAddy(hProcess, dynamicPtrBaseAddr, ammoOffsets);
//
//    // Read Ammo value
//    int ammoValue = 0;
//    ReadProcessMemory(hProcess, (BYTE*)ammoAddr, &ammoValue, sizeof(ammoValue), nullptr);
//
//    // Write to it
//    int newAmmo = 1000;
//    WriteProcessMemory(hProcess, (BYTE*)ammoAddr, &newAmmo, sizeof(newAmmo), nullptr);  
//    
//    // We use BYTE * because it is just an unsigned char that reads nicely
//
//
//    return 0;
//}
