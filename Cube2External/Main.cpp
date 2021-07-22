#include <iostream>
#include "proc.h"


// Local player offsets
struct LocalPlayer {
    const unsigned int base = 0x003472D0;
    const unsigned int health = 0x340;
    uintptr_t dmaHealth = 0x0;
}localPlayer;


int main() {
    //Get ProcId of the target process
    DWORD procId = GetProcId(L"sauerbraten.exe");
    std::cout << "PID: " << procId << std::endl;

    if (!procId) {
        std::cout << "sauerbraten.exe not found" << std::endl;
        return 0;
    }

    // get the base address of the game
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"sauerbraten.exe"); // 7FF6768C0000 - sauerbraten.exe
    std::cout << "Module Base: 0x" << std::hex << moduleBase << std::endl;
    
    // Open handle to process
    HANDLE hProcess = 0;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);
    std::cout << "hProcess: " << hProcess << std::endl;

    localPlayer.dmaHealth = FindDMAAddy(hProcess, (moduleBase + localPlayer.base), { 0x0, localPlayer.health });

    std::cout << "0x" << std::hex << localPlayer.dmaHealth << std::endl;

    //Read health value
    int health = 0;
    BOOL status = ReadProcessMemory(hProcess, (BYTE*)localPlayer.dmaHealth, &health, sizeof(health), nullptr);
    std::cout << "Status: " << status << std::endl;
    std::cout << "Health: " << std::dec << health << std::endl;
}

