//https://guidedhacking.com and c5
#pragma once
#include <windows.h>
#include <stdio.h>
#include <gl\GL.h>

struct vec3 { float x, y, z; };

namespace GL
{
    class Font
    {
    public:
        bool bBuilt = false;
        unsigned int base;
        HDC hdc = nullptr;
        int height;
        int width;

        void Build(int height);
        void Print(float x, float y, int r, int g, int b, const char* format, ...);

        //center on X and Y axes
        vec3 centerText(float x, float y, float width, float height, float textWidth, float textHeight);
        //center on X axis only
        float centerText(float x, float width, float textWidth);
    };
};