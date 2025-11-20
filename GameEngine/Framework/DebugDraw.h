#ifndef __DEBUG_DRAW_H__
#define __DEBUG_DRAW_H__

#include "define.h"
#include <string>
#include <vector>

class DebugDraw
{
public:
    static DebugDraw* getInstance();
    static void release();

    void init(LPDIRECT3DDEVICE9 device);
    void begin();
    void end();
    void present();

    // Drawing methods
    void drawLine(GVector2 start, GVector2 end, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), float thickness = 1.0f);
    void drawRect(RECT rect, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), float thickness = 1.0f);
    void drawRect(float left, float top, float right, float bottom, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), float thickness = 1.0f);
    void drawFilledRect(RECT rect, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
    void drawCircle(GVector2 center, float radius, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), int segments = 16, float thickness = 1.0f);
    void drawText(std::string text, GVector2 position, D3DXCOLOR color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), int fontSize = 12);

    // Convenience methods for game objects
    void drawBoundingBox(const RECT& bbox, D3DXCOLOR color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
    void drawBoundingBox(const RECT &bbox, class Viewport *viewport, D3DXCOLOR color);
    void drawCollisionBox(class ICollidable *obj, D3DXCOLOR color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));
    void drawCollisionBox(class ICollidable* obj, class Viewport* viewport, D3DXCOLOR color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));

    // Toggle debug drawing
    void setEnabled(bool enabled) { _enabled = enabled; }
    bool isEnabled() const { return _enabled; }
    void toggle() { _enabled = !_enabled; }

private:
    DebugDraw();
    ~DebugDraw();

    static DebugDraw* _instance;
    LPDIRECT3DDEVICE9 _device;
    ID3DXLine* _line;
    ID3DXFont* _font;
    bool _enabled;

    // Vertex buffer for filled shapes
    LPDIRECT3DVERTEXBUFFER9 _vertexBuffer;

    struct Vertex
    {
        float x, y, z, rhw;
        D3DXCOLOR color;
    };
};

#endif