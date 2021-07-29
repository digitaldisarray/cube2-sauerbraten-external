#pragma once

// Local player offsets
struct LocalPlayer {
    const unsigned int base = 0x003472D0;
    const unsigned int camBase = 0x002A2560;
    const unsigned int health = 0x340;
    const unsigned int xPos = 0x394;
    const unsigned int yPos = 0x398;
    const unsigned int zPos = 0x39C;
    const unsigned int viewAngles = 0x3C;
    uintptr_t dmaHealth = 0x0;
    uintptr_t dmaXPos = 0x0;
    uintptr_t dmaYPos = 0x0;
    uintptr_t dmaZPos = 0x0;
}localPlayer;


struct Entity {
    const unsigned int position = 0x30; // 0x30 is x, 0x34 is y, 0x38 is z
    const unsigned int alive = 0x77;
    const unsigned int health = 0x178;
    const unsigned int armor = 0x180;
    const unsigned int name = 0x274;
    const unsigned int team = 0x378;
}entity;

struct General {
    const unsigned int playerList = 0x346C90;
    const unsigned int numPlayers = playerList + 0xC;
    const unsigned int menu = 0x345C50;
    const unsigned int projMatrix = 0x32D040;
}general;