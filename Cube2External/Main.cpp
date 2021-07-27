#include <iostream>
#include <Windows.h>

#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "proc.h"
#include "dataTypes.h"
#include "Helpers.h"
#include "offsets.h"
#include "gltext.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main() {

    // get procid of the target process
    DWORD procId = GetProcId(L"sauerbraten.exe");
    std::cout << "PID: " << procId << std::endl;

    if (!procId) {
        std::cout << "sauerbraten.exe not found" << std::endl;
        return 0;
    }

    // Get the base address of the game
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"sauerbraten.exe");

    // Open handle to process
    HANDLE hProcess = 0;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

    localPlayer.dmaHealth = FindDMAAddy(hProcess, (moduleBase + localPlayer.base), { 0x0, localPlayer.health });
    std::cout << "among us: 0x" << std::hex << localPlayer.dmaHealth << std::endl;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor)
        return 0;
    int monitorWidth = glfwGetVideoMode(primaryMonitor)->width;
    int monitorHeight = glfwGetVideoMode(primaryMonitor)->height;

    // Make the window floating, non resizable, maximized, and transparent
    glfwWindowHint(GLFW_FLOATING, true);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_MAXIMIZED, true);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);


    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(monitorWidth, monitorHeight, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;

    // Remove title bar
    glfwSetWindowAttrib(window, GLFW_DECORATED, false);

    // Move the window up (titlebar causes window to "spawn" slightly low
    glfwSetWindowMonitor(window, NULL, 0, 0, monitorWidth, monitorHeight + 1, 0);
    // the +1 to monitorHeight stops windows from thinking that this overlay should be in "fullscreen" mode which would break transparency 

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;


    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Temporary setting vars
    bool bMenuVisible = true;
    bool bESP = true;
    bool bTracers = true;
    bool bTeamCheck = true;
    bool bGodMode = false;
    
    // GL Font setup
    GL::Font font;
    font.Build(12); // TODO: make scale to screen resolution

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Get input
        glfwPollEvents();

        if (GetAsyncKeyState(VK_RSHIFT) & 1) {
            bMenuVisible = !bMenuVisible;
            if (bMenuVisible)
                showMenu(window);
            else
                hideMenu(window);
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        glClear(GL_COLOR_BUFFER_BIT);
        
        
        int inMenu = 0;
        ReadProcessMemory(hProcess, (BYTE*)moduleBase + general.menu, &inMenu, sizeof(inMenu), nullptr);
        // If we are in a game
        if (inMenu == 0) {
            int numPlayers = 0;
            ReadProcessMemory(hProcess, (BYTE*)(moduleBase + general.numPlayers), &numPlayers, sizeof(numPlayers), nullptr);
            
            if (bGodMode) {
                int amount = 333;
                // Real health
                WriteProcessMemory(hProcess, (LPVOID)localPlayer.dmaHealth, &amount, sizeof(amount), nullptr);

                // Display value (todo)
            }

            // If other players exist
            if (numPlayers > 1) {
                Matrix4D projectionMatrix;
                ReadProcessMemory(hProcess, (BYTE*)(moduleBase + general.projMatrix), &projectionMatrix, sizeof(projectionMatrix), nullptr);


                // Go to each player
                for (int i = 1; i < numPlayers; i++) {
                    uintptr_t healthAddr = FindDMAAddy(hProcess, (moduleBase + general.playerList), { (unsigned int)(i * 8), entity.health });
                    int health = 0;
                    ReadProcessMemory(hProcess, (BYTE*)healthAddr, &health, sizeof(health), nullptr);

                    // Skip dead entities
                    if (health <= 0) {
                        continue;
                    }

                    // Read position
                    uintptr_t posAddr = FindDMAAddy(hProcess, (moduleBase + general.playerList), { (unsigned int)(i * 8), entity.position });
                    Vec3 position;
                    ReadProcessMemory(hProcess, (BYTE*)(posAddr), &position, sizeof(position), nullptr);

                    Vec3 topPosition = position;
                    topPosition.Z += 4;

                    Vec3 bottomPosition = position;
                    bottomPosition.Z -= 14;

                    if (bTracers) {
                        Vec2 screenCoords;
                        if (!WorldToScreen(topPosition, screenCoords, projectionMatrix.Matrix, 1920, 1080)) {
                            continue;
                        }

                        glBegin(GL_LINES);
                        glVertex2f(0.f, -1.f);
                        glVertex2f(screenCoords.X, screenCoords.Y);
                        glEnd();


                        if (!WorldToScreen(bottomPosition, screenCoords, projectionMatrix.Matrix, 1920, 1080)) {
                            continue;
                        }

                        glBegin(GL_LINES);
                        glVertex2f(0.f, -1.f);
                        glVertex2f(screenCoords.X, screenCoords.Y);
                        glEnd();
                    }

                    if (bESP) {

                        Vec2 screenCoords;
                        if (!WorldToScreen(topPosition, screenCoords, projectionMatrix.Matrix, 1920, 1080)) {
                            continue;
                        }

                        // Read entity name (max length is 15 chars)
                        uintptr_t nameAddr = FindDMAAddy(hProcess, (moduleBase + general.playerList), { (unsigned int)(i * 8), entity.name });
                        char name[15];
                        ReadProcessMemory(hProcess, (BYTE*)nameAddr, &name, sizeof(name), nullptr);


                        font.Print(screenCoords.X, screenCoords.Y, 255, 255, 255, name);
                    }

                }
            }
        }

        // draw imgui stuff here
        if (bMenuVisible) {

            ImGui::Begin("x86Cheats - Cube 2: Sauerbraten");
            

            ImGui::Checkbox("Tracers", &bTracers);
            ImGui::Checkbox("God Mode (Offline Only)", &bGodMode);


            if (ImGui::Button("Exit")) {
                return 0;
            }

            ImGui::End();

        }
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}