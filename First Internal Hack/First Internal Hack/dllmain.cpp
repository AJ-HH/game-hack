// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "mem.h"
#include "proc.h"

// dll allows one file on disc to be used across multiple processes. Previously files were statically linked
DWORD WINAPI HackThread(HMODULE hModule) {
    // Create console
    AllocConsole();                                     
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);                  // CONOUT is the current console output of the process and gives us write output

    
    //get module base
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    bool bHealth = false, bAmmo = false, bRecoil = false;

    //hack loop
    while (true) {
        // Get Key input
        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }
        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            bHealth = !bHealth;
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            bAmmo = !bAmmo;
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            bRecoil = !bRecoil;
            if (bRecoil) {
                mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
            }
            else {
                // write back original instructions
                mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10);
            }

        }
        //continuous write/freeze
        uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);
        if (localPlayerPtr) {
            if (bHealth) {
                *(int*)(localPlayerPtr + 0xf8) = 1000;          // Deferencing player ptr and offset, casting to int * and then deferencing again.
                                                                // Have access to the whole memory, so no longer have to call WriteProcessMemory
            }
            if (bAmmo) {
                uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 });
                int* ammo = (int*)ammoAddr;
                *ammo = 1000;

                //or just use this
                // *(int*)mem::FindDMAAddy(moduleBase + 0x10F4F4, { 0x374, 0x14, 0x0 }) = 1000;
            }
        }
        Sleep(5);
    }



    //cleanup
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        // We only want to create a thread and let it return true
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));     // LPTHREAD_SART_ROUTINE - pointer to a DWORD WINAPI
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

