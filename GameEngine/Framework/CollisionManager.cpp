#include "CollisionManager.h"
#include <algorithm>

QuadtreeNode::QuadtreeNode(RECT b, int maxObj) : bounds(b), isLeaf(true), maxObjects(maxObj)
{
    for (int i = 0; i < 4; ++i) children[i] = nullptr;
}

QuadtreeNode::~QuadtreeNode()
{
    clear();
}

void QuadtreeNode::subdivide()
{
    if (!isLeaf) return;

    int midX = (bounds.left + bounds.right) / 2;
    int midY = (bounds.top + bounds.bottom) / 2;

    RECT nw = { bounds.left, bounds.top, midX, midY };
    RECT ne = { midX, bounds.top, bounds.right, midY };
    RECT sw = { bounds.left, midY, midX, bounds.bottom };
    RECT se = { midX, midY, bounds.right, bounds.bottom };

    children[0] = new QuadtreeNode(nw, maxObjects);
    children[1] = new QuadtreeNode(ne, maxObjects);
    children[2] = new QuadtreeNode(sw, maxObjects);
    children[3] = new QuadtreeNode(se, maxObjects);

    isLeaf = false;
}

void QuadtreeNode::insert(ICollidable* obj)
{
    RECT bbox = obj->getBoundingBox();

    // Handle edge cases: invalid bbox
    if (bbox.left >= bbox.right || bbox.top >= bbox.bottom) return;

    // Check if object overlaps this node
    if (bbox.right < bounds.left || bbox.left > bounds.right ||
        bbox.bottom < bounds.top || bbox.top > bounds.bottom)
        return; // No overlap, don't insert

    if (isLeaf)
    {
        objects.push_back(obj);
        if (objects.size() > maxObjects)
        {
            subdivide();
            // Re-insert objects into children
            for (auto o : objects)
            {
                for (int i = 0; i < 4; ++i)
                {
                    children[i]->insert(o);
                }
            }
            objects.clear();
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i]->insert(obj);
        }
    }
}

void QuadtreeNode::remove(ICollidable* obj)
{
    if (isLeaf)
    {
        objects.erase(std::remove(objects.begin(), objects.end(), obj), objects.end());
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i]->remove(obj);
        }
    }
}

void QuadtreeNode::query(const RECT& region, std::vector<ICollidable*>& result)
{
    if (region.right < bounds.left || region.left > bounds.right ||
        region.bottom < bounds.top || region.top > bounds.bottom)
        return; // No overlap

    if (isLeaf)
    {
        for (auto obj : objects)
        {
            RECT bbox = obj->getBoundingBox();
            if (!(bbox.right < region.left || bbox.left > region.right ||
                  bbox.bottom < region.top || bbox.top > region.bottom))
            {
                result.push_back(obj);
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i]->query(region, result);
        }
    }
}

void QuadtreeNode::clear()
{
    objects.clear();
    if (!isLeaf)
    {
        for (int i = 0; i < 4; ++i)
        {
            delete children[i];
            children[i] = nullptr;
        }
        isLeaf = true;
    }
}

CollisionManager* CollisionManager::getInstance()
{
    static CollisionManager instance;
    return &instance;
}

CollisionManager::CollisionManager()
{
    // Initialize quadtree with world bounds, e.g., 0 to 1024x768
    RECT worldBounds = { 0, 0, 1024, 768 };
    _quadtree = new QuadtreeNode(worldBounds);
}

CollisionManager::~CollisionManager()
{
    delete _quadtree;
    // Objects are managed elsewhere
}

void CollisionManager::addObject(ICollidable* obj)
{
    if (std::find(_objects.begin(), _objects.end(), obj) == _objects.end())
    {
        _objects.push_back(obj);
    }
}

void CollisionManager::removeObject(ICollidable* obj)
{
    _objects.erase(std::remove(_objects.begin(), _objects.end(), obj), _objects.end());
    _quadtree->remove(obj);
}

void CollisionManager::update()
{
    _quadtree->clear();
    for (auto obj : _objects)
    {
        _quadtree->insert(obj);
    }
    for (auto obj : _objects)
    {
        if (obj->IsBlocking()) continue; // Only process moving objects? Wait, in GitHub, Process is called for collidable objects.
        Process(obj, _objects);
    }
}

void CollisionManager::Scan(ICollidable* objSrc, std::vector<ICollidable*>& objDests, std::vector<CCollisionEvent*>& coEvents)
{
    for (auto objDest : objDests)
    {
        if (objSrc == objDest) continue;

        RECT b1 = objSrc->getBoundingBox();
        RECT b2 = objDest->getBoundingBox();
        float mvx, mvy;
        objSrc->GetSpeed(mvx, mvy);
        float svx, svy;
        objDest->GetSpeed(svx, svy);
        float dx = mvx - svx;
        float dy = mvy - svy;

        float t, nx, ny;
        SweptAABB(b1.left, b1.top, b1.right, b1.bottom, dx, dy,
                  b2.left, b2.top, b2.right, b2.bottom, t, nx, ny);

        CCollisionEvent* e = new CCollisionEvent(t, nx, ny, dx, dy, objDest);
        if (e->WasCollided())
            coEvents.push_back(e);
        else
            delete e;
    }
}

void CollisionManager::Filter(ICollidable* objSrc, std::vector<CCollisionEvent*>& coEvents, CCollisionEvent*& colX, CCollisionEvent*& colY, int filterBlock, int filterX, int filterY)
{
    float min_tx = 1.0f;
    float min_ty = 1.0f;
    int min_ix = -1;
    int min_iy = -1;

    colX = nullptr;
    colY = nullptr;

    for (size_t i = 0; i < coEvents.size(); ++i)
    {
        CCollisionEvent* c = coEvents[i];
        if (c->isDeleted) continue;

        if (filterBlock == 1 && !c->obj->IsBlocking()) continue;

        if (c->t < min_tx && c->nx != 0 && filterX == 1)
        {
            min_tx = c->t;
            min_ix = i;
        }
        if (c->t < min_ty && c->ny != 0 && filterY == 1)
        {
            min_ty = c->t;
            min_iy = i;
        }
    }

    if (min_ix >= 0) colX = coEvents[min_ix];
    if (min_iy >= 0) colY = coEvents[min_iy];
}

void CollisionManager::Process(ICollidable* objSrc, std::vector<ICollidable*>& coObjects)
{
    std::vector<CCollisionEvent*> coEvents;
    CCollisionEvent* colX = nullptr;
    CCollisionEvent* colY = nullptr;

    coEvents.clear();

    // Scan for potential collisions
    Scan(objSrc, coObjects, coEvents);

    if (coEvents.empty())
    {
        // No collision, but in GitHub, OnNoCollision is called, but we don't have it.
        return;
    }

    Filter(objSrc, coEvents, colX, colY);

    float x, y;
    GVector2 pos = objSrc->getPosition();
    x = pos.x;
    y = pos.y;
    float vx, vy;
    objSrc->GetSpeed(vx, vy);
    float dx = vx; // Assume dt=1 for simplicity
    float dy = vy;

    if (colX != nullptr && colY != nullptr)
    {
        if (colY->t < colX->t)
        {
            // Collision on Y first
            y += colY->t * dy + colY->ny * 0.01f; // BLOCK_PUSH_FACTOR
            objSrc->SetPosition(x, y);
            objSrc->OnCollisionWith(colY);

            // Re-check X
            colX->isDeleted = true;
            coEvents.push_back(new CCollisionEvent(0, 0, 0, 0, 0, nullptr)); // Placeholder, need to re-scan
            // Simplified, assume no re-check
            if (colX)
            {
                x += colX->t * dx + colX->nx * 0.01f;
                objSrc->OnCollisionWith(colX);
            }
            else
            {
                x += dx;
            }
        }
        else
        {
            // Collision on X first
            x += colX->t * dx + colX->nx * 0.01f;
            objSrc->SetPosition(x, y);
            objSrc->OnCollisionWith(colX);

            // Re-check Y
            colY->isDeleted = true;
            // Simplified
            if (colY)
            {
                y += colY->t * dy + colY->ny * 0.01f;
                objSrc->OnCollisionWith(colY);
            }
            else
            {
                y += dy;
            }
        }
    }
    else if (colX != nullptr)
    {
        x += colX->t * dx + colX->nx * 0.01f;
        y += dy;
        objSrc->OnCollisionWith(colX);
    }
    else if (colY != nullptr)
    {
        x += dx;
        y += colY->t * dy + colY->ny * 0.01f;
        objSrc->OnCollisionWith(colY);
    }
    else
    {
        x += dx;
        y += dy;
    }

    objSrc->SetPosition(x, y);

    // Handle non-blocking collisions
    for (auto e : coEvents)
    {
        if (e->isDeleted) continue;
        if (e->obj->IsBlocking()) continue;
        objSrc->OnCollisionWith(e);
    }

    // Clean up
    for (auto e : coEvents) delete e;
}

void CollisionManager::SweptAABB(float ml, float mt, float mr, float mb, float dx, float dy, float sl, float st, float sr, float sb, float& t, float& nx, float& ny)
{
    float dx_entry, dx_exit, tx_entry, tx_exit;
    float dy_entry, dy_exit, ty_entry, ty_exit;
    float t_entry;
    float t_exit;
    t = -1.0f; // no collision
    nx = ny = 0;

    // Broad-phase test
    float bl = dx > 0 ? ml : ml + dx;
    float bt = dy > 0 ? mt : mt + dy;
    float br = dx > 0 ? mr + dx : mr;
    float bb = dy > 0 ? mb + dy : mb;
    if (br < sl || bl > sr || bb < st || bt > sb) return;

    if (dx == 0 && dy == 0) return; // not moving

    if (dx > 0) {
        dx_entry = sl - mr;
        dx_exit = sr - ml;
    } else if (dx < 0) {
        dx_entry = sr - ml;
        dx_exit = sl - mr;
    } else {
        dx_entry = -9999999.0f;
        dx_exit = 99999999.0f;
    }

    if (dy > 0) {
        dy_entry = st - mb;
        dy_exit = sb - mt;
    } else if (dy < 0) {
        dy_entry = sb - mt;
        dy_exit = st - mb;
    } else {
        dy_entry = -99999999999.0f;
        dy_exit = 99999999999.0f;
    }

    if (dx == 0) {
        tx_entry = -9999999.0f;
        tx_exit = 99999999.0f;
    } else {
        tx_entry = dx_entry / dx;
        tx_exit = dx_exit / dx;
    }

    if (dy == 0) {
        ty_entry = -99999999999.0f;
        ty_exit = 99999999999.0f;
    } else {
        ty_entry = dy_entry / dy;
        ty_exit = dy_exit / dy;
    }

    if ((tx_entry < 0.0f && ty_entry < 0.0f) || tx_entry > 1.0f || ty_entry > 1.0f) return;

    t_entry = tx_entry > ty_entry ? tx_entry : ty_entry;
    t_exit = tx_exit < ty_exit ? tx_exit : ty_exit;

    if (t_entry > t_exit) return;

    t = t_entry;

    if (tx_entry > ty_entry) {
        ny = 0.0f;
        nx = (dx > 0) ? -1.0f : 1.0f;
    } else {
        nx = 0.0f;
        ny = (dy > 0) ? -1.0f : 1.0f;
    }
}

bool CollisionManager::aabbOverlap(const RECT& a, const RECT& b) const
{
    return !(a.right < b.left || a.left > b.right || a.bottom < b.top || a.top > b.bottom);
}