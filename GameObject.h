#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

struct Metro {
    float x, y, width;
    bool active;
    Metro(float _x, float _y, float _w) : x(_x), y(_y), width(_w), active(true) {}
};

struct Obstacle {
    float x, y, width, height;
    bool isFlying;
    bool active;
    Obstacle(float _x, float _y, float _w, float _h, bool flying) 
        : x(_x), y(_y), width(_w), height(_h), isFlying(flying), active(true) {}
};

struct Coin {
    float x, y, size;
    bool collected;
    Coin(float _x, float _y) : x(_x), y(_y), size(20), collected(false) {}
};

#endif
