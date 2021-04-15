#include "proc.h"
#include "stdafx.h"
#include <Windows.h>
// Copied this video for the code, but the comments are for my own notes
// https://www.youtube.com/watch?v=wiX5LmdD5yk&list=PLt9cUwGw6CYHKBH5OoR8M2ELGlNlrgBKl&index=16


// Gets the process associated with procName
// Notes: DWORD is the type of processID in Windows API
DWORD GetProcId(const wchar_t* procName) {
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot takes a snapshot of all processes
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 procEntry;
		// Must set the size of the structure before using it.
		procEntry.dwSize = sizeof(procEntry);

		// Grabs first process in snapshot and stores in procentry
		if (Process32First(hSnap, &procEntry)) {
			do {
				// Check if the process in procEntry is the same as the procName we pass in; strcmp
				if (!_wcsicmp(procEntry.szExeFile, procName)) {
					procId = procEntry.th32ParentProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry)); // Otherwise get the next process and check it
		}
	}
	CloseHandle(hSnap);
	return procId;
}

// Gets the module base address using the process ID and module name
uintptr_t GetModuleBaseAdress(DWORD procId, const wchar_t* modName) {
	uintptr_t modBaseAddr = 0;
	// Both flags necessary to get both 32 bit and 64 bit modules
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		// Again, must set the size of the structure before using it.
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry)) {
			do {
				if (!_wcsicmp(modEntry.szModule, modName)) {
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr; // Need to cast it as it is defined as a BYTE *
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}
