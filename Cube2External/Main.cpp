#include <iostream>
#include <Windows.h>

#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "proc.h"
#include "math.h"
#include "offsets.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void SetForeground(HWND window) {
    // Set up a generic keyboard event
    INPUT keyInput;
    keyInput.type = INPUT_KEYBOARD;
    keyInput.ki.wScan = 0; // Hardware scan code for key
    keyInput.ki.time = 0;
    keyInput.ki.dwExtraInfo = 0;

    // Set focus to the hWnd (sending Alt allows to bypass limitation)
    keyInput.ki.wVk = VK_MENU;
    keyInput.ki.dwFlags = 0;   // 0 for key press
    SendInput(1, &keyInput, sizeof(INPUT));

    // Set focus to cheat overlay
    SetForegroundWindow(window);

    keyInput.ki.wVk = VK_MENU;
    keyInput.ki.dwFlags = KEYEVENTF_KEYUP;  // Key release
    SendInput(1, &keyInput, sizeof(INPUT));
}

int main() {
    // Get process ID
    DWORD procId = GetProcId(L"sauerbraten.exe");
    std::cout << "PID: " << procId << std::endl;
    if (!procId) {
        std::cout << "sauerbraten.exe not found" << std::endl;
        return 0;
    }

    // Get the base address of the game and open a handle to the process
    uintptr_t moduleBase = GetModuleBaseAddress(procId, L"sauerbraten.exe");
    HANDLE hProcess = 0;
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

    localPlayer.dmaHealth = FindDMAAddy(hProcess, (moduleBase + localPlayer.base), { 0x0, localPlayer.health });

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor)
        return 0;
    int monitorWidth = glfwGetVideoMode(primaryMonitor)->width;
    int monitorHeight = glfwGetVideoMode(primaryMonitor)->height;
    Vec2 center;
    center.x = .5f * monitorWidth;
    center.y = .5f * monitorHeight;

    // Make the window floating, non resizable, maximized, and transparent
    glfwWindowHint(GLFW_FLOATING, true);
    glfwWindowHint(GLFW_RESIZABLE, false);
    glfwWindowHint(GLFW_MAXIMIZED, true);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, true);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(monitorWidth, monitorHeight, "Overlay", NULL, NULL);
    if (window == NULL)
        return 1;

    // Get handle to overlay window
    HWND overlayHandle = glfwGetWin32Window(window);

    // Get handle to sauerbraten
    HWND sauerbratenHandle = FindWindowA(NULL, "Cube 2: Sauerbraten");

    // Hide console
    ShowWindow(GetConsoleWindow(), SW_HIDE);

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

    ImFont* font = io.Fonts->AddFontDefault();
    IM_ASSERT(font != NULL);

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Temporary setting vars
    bool bMenuVisible = true;
    bool bAimbot = false;
    bool bESP = true;
    bool bTracers = true;
    bool bTeamCheck = true;
    bool bGodMode = false;
    bool bShowConsole = false;

    // The enemy closest to the center of the screen
    Vec3 closestEnemy;
    float smallestDist = 9999999.f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Get input
        glfwPollEvents();
        
        // Open/close menu
        if (GetAsyncKeyState(VK_RSHIFT) & 1) {
            bMenuVisible = !bMenuVisible;
            if (bMenuVisible) {
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false); // Show menu

                SetForeground(overlayHandle);
            }
            else {
                glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, true); // Hide menu

                SetForeground(sauerbratenHandle);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
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

                    Vec2 screenPos;
                    if (!WorldToScreen(position, screenPos, projectionMatrix.Matrix, 1920, 1080)) {
                        continue;
                    }

                    // Find the closest enemy to the center of the screen
                    int dX = std::abs(center.x - screenPos.x);
                    int dY = std::abs(center.y - screenPos.y);
                    int dist = std::sqrt(dX * dX + dY * dY);
                    if (i == 1) {
                        smallestDist = dist;
                        closestEnemy = position;
                    }
                    else {
                        if (dist < smallestDist) {
                            smallestDist = dist;
                            closestEnemy = position;
                        }
                    }

                    Vec3 bottomPosition = position;
                    bottomPosition.z -= 14;

                    if (bTracers) {
                        Vec2 bottomPos;
                        WorldToScreen(bottomPosition, bottomPos, projectionMatrix.Matrix, 1920, 1080);
                        ImGui::GetBackgroundDrawList()->AddLine(ImVec2(990.f, 1080.f), ImVec2(bottomPos.x, bottomPos.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
                    }

                    if (bESP) {
                        // Read entity name (max length is 15 chars)
                        uintptr_t nameAddr = FindDMAAddy(hProcess, (moduleBase + general.playerList), { (unsigned int)(i * 8), entity.name });
                        char name[15];
                        ReadProcessMemory(hProcess, (BYTE*)nameAddr, &name, sizeof(name), nullptr);

                        ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenPos.x, screenPos.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)), std::string(name).c_str());
                    }
                }

                if (GetAsyncKeyState(0x51) & 1) {
                    bAimbot = !bAimbot;
                }

                // TODO: FIX: aimbot cant run unless world to screen works on at least one enemy
                if (bAimbot) {
                    
                    //uintptr_t positionAddr = FindDMAAddy(hProcess, (moduleBase + localPlayer.base), { 0x0, localPlayer.xPos });
                    uintptr_t positionAddr = FindDMAAddy(hProcess, (moduleBase + general.playerList), { 0x0, entity.position });
                    Vec3 localPos;
                    ReadProcessMemory(hProcess, (BYTE*)positionAddr, &localPos, sizeof(localPos), nullptr);

                    // math
                    Vec2 angle;
                    angle.x = -std::atan2(closestEnemy.x - localPos.x, closestEnemy.y - localPos.y) / 3.141593 * 180.f;

                    Vec3 d;
                    d.x = localPos.x - closestEnemy.x;
                    d.y = localPos.y - closestEnemy.y;
                    d.z = localPos.z - closestEnemy.z;
                    float distance = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);

                    std::cout << "x: " << d.x << "\n";


                    angle.y = std::asin((closestEnemy.z - localPos.z) / distance) * (57.2957795131); // 57 is 180/pi
                    // OLD: Vec2 angle = CalcAngle(closestEnemy, localPos);

                    // Normalize/Clamp angles
                    if (angle.x < 0.f) {
                        angle.x += 360.f;
                    }

                    if (angle.x > 360.f) {
                        angle.x -= 360.f;
                    }

                    if (angle.y < -90.f) {
                        angle.y = -90.f;
                    }

                    if (angle.y > 90.f) {
                        angle.y = 90.f;
                    }

                    // write angles
                    uintptr_t viewAnglesAddr = FindDMAAddy(hProcess, (moduleBase + localPlayer.camBase), { localPlayer.viewAngles });
                    WriteProcessMemory(hProcess, (LPVOID)viewAnglesAddr, &angle, sizeof(angle), nullptr);

                    Vec2 crossPos;
                    WorldToScreen(closestEnemy, crossPos, projectionMatrix.Matrix, 1920, 1080);
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(crossPos.x - 8, crossPos.y), ImVec2(crossPos.x + 8, crossPos.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
                    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(crossPos.x, crossPos.y - 8), ImVec2(crossPos.x, crossPos.y + 8), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
                }

                // Reset smallest dist and closest enemy
                smallestDist = 9999999.f;

            }
        }

        // draw imgui stuff here
        if (bMenuVisible) {

            ImGui::Begin("x86Cheats - Cube 2: Sauerbraten");
            
            ImGui::Checkbox("Aimbot", &bAimbot);
            ImGui::Checkbox("ESP", &bESP);
            ImGui::Checkbox("Tracers", &bTracers);
            ImGui::Checkbox("God Mode (Offline Only)", &bGodMode);


            if (ImGui::Button("Toggle Console")) {
                bShowConsole = !bShowConsole;

                if (bShowConsole)
                    ShowWindow(GetConsoleWindow(), SW_SHOW);
                else
                    ShowWindow(GetConsoleWindow(), SW_HIDE);
            }

            if (ImGui::Button("Quit")) {
                return 0;
            }

            ImGui::End();
        }
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
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