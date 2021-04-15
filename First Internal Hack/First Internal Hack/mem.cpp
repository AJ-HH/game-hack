#include "stdafx.h"
#include <Windows.h>
#include "mem.h"

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


// DMA - dynamic memory allocation
// Takes a handle to a process, a pointer to the base address in the process and an array of unsigned ints which are our offsets
uintptr_t mem::FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);  // Copies the data in the specified address range from the specific process' address space
		addr += offsets[i];												// Dereferencing pointer
	}
	return addr;
}





///////////////////////////////////////
// Internal versions of above hacks ///
///////////////////////////////////////


// Function to patch a specific area of memory 
void mem::Patch(BYTE* dst, BYTE* src, unsigned int size) {
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect); // Changes the protection of a region of pages to execute, read write
	memcpy(dst, src, size);											// Patches the bytes
	VirtualProtect(dst, size, oldProtect, &oldProtect);				// Changes the protections back to previous
}

// Function to nop instructions
void mem::Nop(BYTE* dst, unsigned int size) {
	DWORD oldProtect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldProtect); // Changes the protection of a region of pages to execute, read write
	memset(dst, 0x90, size);											// Patches the bytes
	VirtualProtect(dst, size, oldProtect, &oldProtect);				// Changes the protections back to previous
}


// DMA - dynamic memory allocation
// Takes a handle to a process, a pointer to the base address in the process and an array of unsigned ints which are our offsets
uintptr_t mem::FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		addr = *(uintptr_t*)addr;
		addr += offsets[i];												// Dereferencing pointer
	}
	return addr;
}

