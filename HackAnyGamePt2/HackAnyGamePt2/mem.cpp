#include "mem.h"
#include "stdafx.h"
#include <Windows.h>

// Function to patch a specific area of memory 
void mem::PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess) {
	DWORD oldProtect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldProtect); // Changes the protection of a region of pages to execute, read write
	WriteProcessMemory(hProcess, dst, src, size, nullptr);						// Patches the bytes
	VirtualProtectEx(hProcess, dst, size, oldProtect, &oldProtect);				// Changes the protections back to previous
}

// Function to nop instructions
void mem::NopEx(BYTE* dst, unsigned int size, HANDLE hProcess) {
	BYTE* nopArray = new BYTE[size];
	memset(nopArray, 0x90, size);

	PatchEx(dst, nopArray, size, hProcess);
	delete[] nopArray;
}