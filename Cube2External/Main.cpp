#include <iostream>
#include "proc.h"
#include <GLFW/glfw3.h>


// Local player offsets
struct LocalPlayer {
    const unsigned int base = 0x003472D0;
    const unsigned int health = 0x340;
    const unsigned int xPos = 0x394;
    const unsigned int yPos = 0x398;
    const unsigned int zPos = 0x39C;
    uintptr_t dmaHealth = 0x0;
    uintptr_t dmaXPos = 0x0;
    uintptr_t dmaYPos = 0x0;
    uintptr_t dmaZPos = 0x0;
}localPlayer;

struct General {
    const unsigned int players = 0x346C90;
    const unsigned int numPlayers = players + 0xC;
    const unsigned int menu = 0x345C50;
}general;

int main(void) {
    GLFWwindow* window;

    // Initialize the library 
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "Cube 2 Cheat Overlay", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }



    // Make the window's context current 
    glfwMakeContextCurrent(window);

    // Get ProcId of the target process
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



    // Loop until the user closes the window 
    while (!glfwWindowShouldClose(window))
    {
        int inMenu = 0;
        ReadProcessMemory(hProcess, (BYTE*)moduleBase + general.menu, &inMenu, sizeof(inMenu), nullptr);
        if (inMenu == 0) {

            int health = 0;
            ReadProcessMemory(hProcess, (BYTE*)localPlayer.dmaHealth, &health, sizeof(health), nullptr);
            std::cout << "\rHealth: " << std::dec << health;

        }


        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

