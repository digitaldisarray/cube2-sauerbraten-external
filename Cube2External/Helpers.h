#pragma once

void showMenu(GLFWwindow* window) {
	std::cout << "showing menu" << std::endl;
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, false);
}

void hideMenu(GLFWwindow* window) {
	std::cout << "hiding menu" << std::endl;
	glfwSetWindowAttrib(window, GLFW_MOUSE_PASSTHROUGH, true);
}

// When rendering with float coordinates, OpenGL uses -1 to 1 for x and y axis
void ConvertToRange(Vec2& point, int windowWidth, int windowHeight) {
    point.X /= windowWidth;
    point.X *= 2.f;
    point.X -= 1.f;

    point.Y /= windowHeight;
    point.Y *= 2.f;
    point.Y -= 1.f;
}

bool WorldToScreenOld(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight)
{
    Vec3 clip;
    clip.Z = pos.X * matrix[3] + pos.Y * matrix[7] + pos.Z * matrix[11] + matrix[15];
    if (clip.Z < 0.1f)
        return false;

    clip.X = pos.X * matrix[0] + pos.Y * matrix[4] + pos.Z * matrix[8] + matrix[12];
    clip.Y = pos.X * matrix[1] + pos.Y * matrix[5] + pos.Z * matrix[9] + matrix[13];

    Vec2 ndc;
    ndc.X = clip.X / clip.Z;
    ndc.Y = clip.Y / clip.Z;

    screen.X = (windowWidth / 2 * ndc.X) + (ndc.X + windowWidth / 2);
    screen.Y = (windowHeight / 2 * ndc.Y) + (ndc.Y + windowHeight / 2);

    // Convert to -1.0 to 1.0 range, only used for OpenGL rendering
    // ConvertToRange(screen, windowWidth, windowHeight);

    return true;
}

bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight)
{
    // Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
    Vec4 clipCoords;
    clipCoords.X = pos.X * matrix[0] + pos.Y * matrix[4] + pos.Z * matrix[8] + matrix[12];
    clipCoords.Y = pos.X * matrix[1] + pos.Y * matrix[5] + pos.Z * matrix[9] + matrix[13];
    clipCoords.Z = pos.X * matrix[2] + pos.Y * matrix[6] + pos.Z * matrix[10] + matrix[14];
    clipCoords.W = pos.X * matrix[3] + pos.Y * matrix[7] + pos.Z * matrix[11] + matrix[15];

    if (clipCoords.W < 0.1f)
        return false;

    // perspective division, dividing by clip.W = Normalized Device Coordinates
    Vec2 NDC;
    NDC.X = clipCoords.X / clipCoords.W;
    NDC.Y = clipCoords.Y / clipCoords.W;

    // Transform to window coordinates
    screen.X = (windowWidth / 2 * NDC.X) + (NDC.X + windowWidth / 2);
    screen.Y = -(windowHeight / 2 * NDC.Y) + (NDC.Y + windowHeight / 2);

    // Convert to -1.0 to 1.0 range, only used for OpenGL rendering
    // ConvertToRange(screen, windowWidth, windowHeight);

    return true;
}

