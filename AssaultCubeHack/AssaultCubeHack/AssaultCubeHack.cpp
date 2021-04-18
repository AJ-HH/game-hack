#include "stdafx.h"
#include <iostream>
#include <vector>
#include <Windows.h>
#include "proc.h"
#include "mem.h"

int GetEntityAtCrossHair(HANDLE hProc);

int main() {
	HANDLE hProcess = 0;
	uintptr_t moduleBase, localPlayerPtr, healthAddr, ammoAddr, grenadeAddr, pistolMaxAmmoAddr, ARMaxAmmoAddr, isAttackAddr;
	moduleBase = localPlayerPtr = healthAddr = ammoAddr = grenadeAddr = pistolMaxAmmoAddr = ARMaxAmmoAddr = isAttackAddr = 0;
	bool bHealthArmor, bAmmo, bRecoil, bFireRate, bAttack;
	bHealthArmor = bAmmo = bRecoil = bFireRate = bAttack = false;
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
		isAttackAddr = FindDMAAddy(hProcess, localPlayerPtr, { 0x224 });
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
				//mem::NopEx((BYTE*)(moduleBase + 0x63786), 10, hProcess);
				mem::PatchEx((BYTE*)(moduleBase + 0x62020), (BYTE*)"\xc2\x08\x00", 3, hProcess);
			}
			else {
				// Revert nop bytes with prior code
				//mem::PatchEx((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2", 10, hProcess);
				mem::PatchEx((BYTE*)(moduleBase + 0x62020), (BYTE*)"\x55\x8B\xEC", 3, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
			bFireRate = !bFireRate;

			if (bFireRate) {
				// Turn off code to set fire rate delay
				mem::NopEx((BYTE*)(moduleBase + 0x637E4), 2, hProcess);
			}
			else {
				// Revert nop bytes with prior code
				mem::PatchEx((BYTE*)(moduleBase + 0x637E4), (BYTE*)"\x89\x0A", 2, hProcess);
			}
		}
		if (GetAsyncKeyState(VK_NUMPAD6) & 1) {
			// I would call something like if (GetEntityAtCrossHair(hProcess)) to see if there is a valid entity, but alas, it keeps on crashing
			bAttack = !bAttack;

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
		if (bAttack) {
			int one = 1;
			mem::PatchEx((BYTE*)isAttackAddr, (BYTE*)&one, sizeof(one), hProcess);
		}
		Sleep(10);
	}

	std::cout << "Process not found, press enter to exit\n";
	getchar();
	return 0;
}

// Code modifed from CallFunc4SC in https://github.com/ELTraxo/EFCT_Invoker/blob/62a2f9ea5782fc4cc0fd684d0e524d611638e19d/EFCT_Invoker/EFCT_Invoker.cpp#L43
#define pl(s) printf("%s\n", s)

struct _Args
{
	int a = 0; //arg 1
	int b = 0; //arg 2
	int c = 0; //return
};

// Should insert shellcode, executing the function that detects whether an entity is present, but the exit code thread is 0, meaning it failed.
int GetEntityAtCrossHair(HANDLE hProc)
{
	pl("Calling Function with shellcode.\n");

	//buffer for our shellcode
	DWORD dwBufferSize = 0x2000;

	//allocate space for our shellcode. MEM_COMMIT ensures that the memory contents are zero
	void* pMemory = VirtualAllocEx(hProc, 0, dwBufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);	
	if (!pMemory)
	{
		pl("Failed to allocate memory.");
		return -1;
	}


	constexpr DWORD dwCodeSize = 0x200;

	char shellcode[dwCodeSize] = {
		0xB8, 0x00, 0x46, 0x07, 0xC0,	//mov eax, 0	  // move address of target function into eax
		0xFF, 0xD0,						//call eax		  // call function
		0xC3,							//ret			  // return
		0x90							//nop			  // just for alignment.
	};




	if (!WriteProcessMemory(hProc, pMemory, 0, 0, nullptr))
	{
		pl("Failed to write arguments to process.");
		VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);
		return -1;
	}

	pl("Writing code to process...");

	void* pCode = (void*)((uintptr_t)pMemory);
	if (!WriteProcessMemory(hProc, pCode, shellcode, dwCodeSize, nullptr))
	{
		pl("Failed to write code to process.");
		VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);
		return -1;
	}

	// Create thread with shell code and run it
	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)pCode, 0, 0, 0);
	if (!hThread)
	{
		pl("Failed to create thread.");
		VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);
		return -1;
	}

	WaitForSingleObject(hThread, INFINITE);

	//Read return data
	DWORD dwExit = 0;
	GetExitCodeThread(hThread, &dwExit);
	printf("Function returned: %i\n", dwExit);
	VirtualFreeEx(hProc, pMemory, dwBufferSize, MEM_RELEASE);

	return -1;
}

//char shellcode[dwCodeSize] = {
//	0x8B, 0x0D, 0x74, 0x9B, 0x50, 0x00,			// mov ecx,[00509B74]
//	0x8A, 0x81, 0x83, 0x00, 0x00, 0x00,			// mov al,[ecx+00000083]
//	0x83, 0xEC, 0x08,							// sub esp,08 
//	0x57,										// push edi
//	0x84, 0xC0,									// test al,al
//	0x74, 0x19,									// je 004607ED
//	0x3C, 0x02,									// cmp al,02
//	0x75, 0x0E,									// jne 004607E6
//	0xA1, 0xF4, 0xF4, 0x50, 0x00,				// mov eax,[0050F4F4]
//	0x83, 0xB8, 0x38, 0x03, 0x00, 0x00, 0x01,	// cmp dword ptr [eax+00000338],01
//	0x74, 0x07,									// je 004607ED
//	0x33, 0xC0,									// xor eax,eax
//	0x5F,										// pop edi
//	0x83, 0xC4, 0x08,							// add esp,08
//	0xC3,										// ret 
//	0x6A, 0x00,									// push 00
//	0x8D, 0x54, 0x24, 0x08,						// lea edx,[esp+08]
//	0x52,										// push edx
//	0x51,										// push ecx
//	0x83, 0xC1, 0x04,							// add ecx,04
//	0x68, 0x00, 0xA4, 0x50, 0x00,				// push 0050A400
//	0x51,										// push ecx
//	0x8D, 0x7C, 0x24, 0x1C,						// lea edi,[esp+1C]
//	0xE8, 0x69, 0xFE, 0xFF, 0xFF,				// call 00460670
//	0x83, 0xC4, 0x14,							// add esp,14
//	0x5F,										// pop edi
//	0x83, 0xC4, 0x08,							// add esp,08
//	0xC3										// ret 
//};
////79