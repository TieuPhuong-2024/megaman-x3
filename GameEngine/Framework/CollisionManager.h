#pragma once
#include "ICollidable.h"
#include <vector>
#include <set>

struct CCollisionEvent
{
    float t, nx, ny;
    float dx, dy; // Relative movement
    ICollidable* obj;
    bool isDeleted;

    CCollisionEvent(float t, float nx, float ny, float dx, float dy, ICollidable* obj)
        : t(t), nx(nx), ny(ny), dx(dx), dy(dy), obj(obj), isDeleted(false) {}

    bool WasCollided() { return t >= 0.0f && t <= 1.0f; }
};

struct QuadtreeNode
{
    RECT bounds;
    std::vector<ICollidable*> objects;
    QuadtreeNode* children[4]; // 0: NW, 1: NE, 2: SW, 3: SE
    bool isLeaf;
    int maxObjects;

    QuadtreeNode(RECT b, int maxObj = 4);
    ~QuadtreeNode();

    void subdivide();
    void insert(ICollidable* obj);
    void remove(ICollidable* obj);
    void query(const RECT& region, std::vector<ICollidable*>& result);
    void clear();
};

class CollisionManager
{
public:
    static CollisionManager* getInstance();

    void addObject(ICollidable* obj);
    void removeObject(ICollidable* obj);
    void update();

private:
    CollisionManager();
    ~CollisionManager();

    void Scan(ICollidable* objSrc, std::vector<ICollidable*>& objDests, std::vector<CCollisionEvent*>& coEvents);
    void Filter(ICollidable* objSrc, std::vector<CCollisionEvent*>& coEvents, CCollisionEvent*& colX, CCollisionEvent*& colY, int filterBlock = 1, int filterX = 1, int filterY = 1);
    void Process(ICollidable* objSrc, std::vector<ICollidable*>& coObjects);
    void SweptAABB(float ml, float mt, float mr, float mb, float dx, float dy, float sl, float st, float sr, float sb, float& t, float& nx, float& ny);
    bool aabbOverlap(const RECT& a, const RECT& b) const;

    std::vector<ICollidable*> _objects;
    QuadtreeNode* _quadtree;
    std::set<std::pair<ICollidable*, ICollidable*>> _collisionPairs;  // to avoid duplicate checks
};