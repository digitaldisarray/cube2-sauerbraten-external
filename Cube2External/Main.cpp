#include <iostream>
#include <Windows.h>

#include <GL/glew.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "Helpers.h"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main() {
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

    bool bMenuVisible = true;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Get input
        glfwPollEvents();

        if (GetAsyncKeyState(VK_INSERT) & 1) {
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

        // draw imgui stuff here
        if (bMenuVisible) {

            ImGui::Begin("x86Cheats - Cube 2: Sauerbraten");
            

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