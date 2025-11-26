#pragma once
#include <Windows.h>
#include <vector>
#include <algorithm>

using namespace std;

class ICollidable;
typedef ICollidable *LPCOLLIDABLE;

struct CCollisionEvent;
typedef CCollisionEvent *LPCOLLISIONEVENT;

struct CCollisionEvent
{
    LPCOLLIDABLE src_obj; // source object: the object from which to calculate collision
    LPCOLLIDABLE obj;     // the target object
    float t, nx, ny;
    float dx, dy; // *RELATIVE* movement distance between this object and obj
    bool isDeleted;

    CCollisionEvent(float t, float nx, float ny, float dx = 0, float dy = 0,
                    LPCOLLIDABLE obj = NULL, LPCOLLIDABLE src_obj = NULL)
    {
        this->t = t;
        this->nx = nx;
        this->ny = ny;
        this->dx = dx;
        this->dy = dy;
        this->obj = obj;
        this->src_obj = src_obj;
        this->isDeleted = false;
    }

    int WasCollided();

    static bool compare(const LPCOLLISIONEVENT &a, LPCOLLISIONEVENT &b)
    {
        return a->t < b->t;
    }
};

class CCollision
{
    static CCollision *__instance;

public:
    static void SweptAABB(
        float ml, // move left
        float mt, // move top
        float mr, // move right
        float mb, // move bottom
        float dx, // movement delta x
        float dy, // movement delta y
        float sl, // static left
        float st, // static top
        float sr, // static right
        float sb, // static bottom
        float &t,
        float &nx,
        float &ny);

    LPCOLLISIONEVENT SweptAABB(
        LPCOLLIDABLE objSrc,
        DWORD dt,
        LPCOLLIDABLE objDest);

    void Scan(
        LPCOLLIDABLE objSrc,
        DWORD dt,
        vector<LPCOLLIDABLE> *objDests,
        vector<LPCOLLISIONEVENT> &coEvents);

    void Filter(
        LPCOLLIDABLE objSrc,
        vector<LPCOLLISIONEVENT> &coEvents,
        LPCOLLISIONEVENT &colX,
        LPCOLLISIONEVENT &colY,
        int filterBlock = 1, // 1 = only filter block collisions, 0 = filter all collisions
        int filterX = 1,     // 1 = process events on X-axis, 0 = skip events on X
        int filterY = 1);    // 1 = process events on Y-axis, 0 = skip events on Y

    void Process(LPCOLLIDABLE objSrc, DWORD dt, vector<LPCOLLIDABLE> *coObjects);

    static CCollision *GetInstance();
};
