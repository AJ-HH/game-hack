#include "stdafx.h"
#include <iostream>
#include <vector>
#include <Windows.h>
#include "proc.h"
#include "mem.h"

int main() {
	HANDLE hProcess = 0;
	uintptr_t moduleBase, localPlayerPtr, healthAddr, ammoAddr, grenadeAddr, pistolMaxAmmoAddr, ARMaxAmmoAddr ;
	moduleBase = localPlayerPtr = healthAddr = ammoAddr = grenadeAddr = pistolMaxAmmoAddr = ARMaxAmmoAddr = 0;
	bool bHealthArmor, bAmmo, bRecoil;
	bHealthArmor = bAmmo = bRecoil = false;
	const int refreshValue = 1000;

	DWORD procId = GetProcId(L"ac_client.exe");

	if (procId) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);	// Allows us to read, write, execute process, etc.
		moduleBase = GetModuleBaseAdress(procId, L"ac_client.exe");	// Get module base address of Assault Cube
		localPlayerPtr = moduleBase + 0x10f4f4;
		// Get the dynamic address using the relative offsets
		healthAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0xf8 });	
		ammoAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0xfc });	   
		grenadeAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0x158 });
		ARMaxAmmoAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0x128 });
		pistolMaxAmmoAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0x114 });
	}
	else {
		std::cout << "Process not found, press enter to exit\n";
		getchar();
		return 0;
	}
	DWORD dwExit = 0;

	// GetExitCodeProcess ensures the process is still running
	while (GetExitCodeProcess(hProcess, &dwExit) && dwExit == STILL_ACTIVE) {
		// Different checks for keyboard input
		if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
			bHealthArmor = !bHealthArmor;			// Turn off health and armor decrement
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
			const int grenadeValue = 10;
			mem::PatchEx((BYTE*)grenadeAddr, (BYTE*)&grenadeValue, sizeof(grenadeValue), hProcess);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
			bAmmo = !bAmmo;
			if (bAmmo) {
				// Set max ammo to 1000
				mem::PatchEx((BYTE*)ARMaxAmmoAddr, (BYTE*)&refreshValue, sizeof(refreshValue), hProcess);
				mem::PatchEx((BYTE*)pistolMaxAmmoAddr, (BYTE*)&refreshValue, sizeof(refreshValue), hProcess);
				// ff 06 = inc [esi] (set the ammo to increment)
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x06", 2, hProcess);
			}
			else {
				// ff 0e = dec [esi] (turn off ammo hack and decrement as usual)
				mem::PatchEx((BYTE*)(moduleBase + 0x637e9), (BYTE*)"\xFF\x0E", 2, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
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
		if (bHealthArmor) {

			mem::PatchEx((BYTE*)healthAddr, (BYTE*)&refreshValue, sizeof(refreshValue), hProcess);
			mem::PatchEx((BYTE*)ammoAddr, (BYTE*)&refreshValue, sizeof(refreshValue), hProcess);
		}
		Sleep(10);
	}

	std::cout << "Process not found, press enter to exit\n";
	getchar();
	return 0;
}

