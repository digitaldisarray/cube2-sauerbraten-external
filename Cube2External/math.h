#pragma once

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
    void multiply(Vec3 vector) {
        x *= vector.x;
        y *= vector.y;
        z *= vector.z;
    };
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

struct Matrix4D {
    float Matrix[16];
};

/*
class Vector3
{
public:

    float x, y, z;

    Vector3() {};
    Vector3(const float x, const float y, const float z) : x(x), y(y), z(z) {}
    Vector3 operator + (const Vector3& rhs) const { return Vector3(x + rhs.x, y + rhs.y, z + rhs.z); }
    Vector3 operator - (const Vector3& rhs) const { return Vector3(x - rhs.x, y - rhs.y, z - rhs.z); }
    Vector3 operator * (const float& rhs) const { return Vector3(x * rhs, y * rhs, z * rhs); }
    Vector3 operator / (const float& rhs) const { return Vector3(x / rhs, y / rhs, z / rhs); }
    Vector3& operator += (const Vector3& rhs) { return *this = *this + rhs; }
    Vector3& operator -= (const Vector3& rhs) { return *this = *this - rhs; }
    Vector3& operator *= (const float& rhs) { return *this = *this * rhs; }
    Vector3& operator /= (const float& rhs) { return *this = *this / rhs; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    Vector3 Normalize() const { return *this * (1 / Length()); }
    float Distance(const Vector3& rhs) const { return (*this - rhs).Length(); }
};
*/

// When rendering with float coordinates, OpenGL uses -1 to 1 for x and y axis
void ConvertToRange(Vec2& point, int windowWidth, int windowHeight) {
    point.x /= windowWidth;
    point.x *= 2.f;
    point.x -= 1.f;

    point.y /= windowHeight;
    point.y *= 2.f;
    point.y -= 1.f;
}

bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight) {
    // Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
    Vec4 clipCoords;
    clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
    clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
    clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

    if (clipCoords.w < 0.1f)
        return false;

    // Perspective division, dividing by clip.w = Normalized Device Coordinates
    Vec2 NDC;
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;

    // Transform to window coordinates
    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

    return true;
}


// OpenGL world to screen with -1 to 1 coordinates
bool WorldToScreenOld(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight) {
    Vec3 clip;
    clip.z = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];
    if (clip.z < 0.1f)
        return false;

    clip.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
    clip.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];

    Vec2 ndc;
    ndc.x = clip.x / clip.z;
    ndc.y = clip.y / clip.z;

    screen.x = (windowWidth / 2 * ndc.x) + (ndc.x + windowWidth / 2);
    screen.y = (windowHeight / 2 * ndc.y) + (ndc.y + windowHeight / 2);

    // Convert to -1.0 to 1.0 range, only used for OpenGL rendering
    ConvertToRange(screen, windowWidth, windowHeight);

    return true;
}
