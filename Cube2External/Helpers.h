#pragma once

void showMenu(GLFWwindow* window) {
	std::cout << "showing menu" << std::endl;
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false);
}

void hideMenu(GLFWwindow* window) {
	std::cout << "hiding menu" << std::endl;
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, true);
}