#include "DebugDraw.h"
#include "ICollidable.h"
#include "Viewport.h"
#include <cmath>

// Only compile in debug builds
#ifdef _DEBUG

DebugDraw* DebugDraw::_instance = nullptr;

DebugDraw::DebugDraw()
    : _device(nullptr), _line(nullptr), _font(nullptr), _enabled(true), _vertexBuffer(nullptr)
{
}

DebugDraw::~DebugDraw()
{
    if (_line) _line->Release();
    if (_font) _font->Release();
    if (_vertexBuffer) _vertexBuffer->Release();
}

DebugDraw* DebugDraw::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = new DebugDraw();
    }
    return _instance;
}

void DebugDraw::release()
{
    SAFE_DELETE(_instance);
}

void DebugDraw::init(LPDIRECT3DDEVICE9 device)
{
    _device = device;

    // Create line interface
    D3DXCreateLine(device, &_line);

    // Create font
    D3DXCreateFont(device, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                   L"Arial", &_font);

    // Create vertex buffer for filled shapes
    device->CreateVertexBuffer(sizeof(Vertex) * 4, D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
                               D3DPOOL_MANAGED, &_vertexBuffer, NULL);
}

void DebugDraw::begin()
{
    if (!_enabled) return;

    // Set render states for 2D drawing
    _device->SetRenderState(D3DRS_LIGHTING, FALSE);
    _device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    _device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void DebugDraw::end()
{
    if (!_enabled) return;

    // Reset render states
    _device->SetRenderState(D3DRS_LIGHTING, TRUE);
    _device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void DebugDraw::present()
{
    // Nothing special needed here
}

void DebugDraw::drawLine(GVector2 start, GVector2 end, D3DXCOLOR color, float thickness)
{
    if (!_enabled || !_line) return;

    D3DXVECTOR2 points[2] = { D3DXVECTOR2(start.x, start.y), D3DXVECTOR2(end.x, end.y) };
    _line->SetWidth(thickness);
    _line->Draw(points, 2, color);
}

void DebugDraw::drawRect(RECT rect, D3DXCOLOR color, float thickness)
{
    drawRect((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom, color, thickness);
}

void DebugDraw::drawRect(float left, float top, float right, float bottom, D3DXCOLOR color, float thickness)
{
    if (!_enabled) return;

    // Draw four lines
    drawLine(GVector2(left, top), GVector2(right, top), color, thickness);
    drawLine(GVector2(right, top), GVector2(right, bottom), color, thickness);
    drawLine(GVector2(right, bottom), GVector2(left, bottom), color, thickness);
    drawLine(GVector2(left, bottom), GVector2(left, top), color, thickness);
}

void DebugDraw::drawFilledRect(RECT rect, D3DXCOLOR color)
{
    if (!_enabled || !_vertexBuffer) return;

    Vertex vertices[4] = {
        { (float)rect.left, (float)rect.top, 0.0f, 1.0f, color },
        { (float)rect.right, (float)rect.top, 0.0f, 1.0f, color },
        { (float)rect.left, (float)rect.bottom, 0.0f, 1.0f, color },
        { (float)rect.right, (float)rect.bottom, 0.0f, 1.0f, color }
    };

    void* pVertices;
    _vertexBuffer->Lock(0, sizeof(vertices), &pVertices, 0);
    memcpy(pVertices, vertices, sizeof(vertices));
    _vertexBuffer->Unlock();

    _device->SetStreamSource(0, _vertexBuffer, 0, sizeof(Vertex));
    _device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    _device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void DebugDraw::drawCircle(GVector2 center, float radius, D3DXCOLOR color, int segments, float thickness)
{
    if (!_enabled) return;

    std::vector<D3DXVECTOR2> points;
    points.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * D3DX_PI * i / segments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        points.push_back(D3DXVECTOR2(x, y));
    }

    if (_line)
    {
        _line->SetWidth(thickness);
        _line->Draw(&points[0], points.size(), color);
    }
}

void DebugDraw::drawText(std::string text, GVector2 position, D3DXCOLOR color, int fontSize)
{
    if (!_enabled || !_font) return;

    RECT rect = { (LONG)position.x, (LONG)position.y, 0, 0 };
    _font->DrawTextA(NULL, text.c_str(), -1, &rect, DT_NOCLIP, color);
}

void DebugDraw::drawBoundingBox(const RECT& bbox, D3DXCOLOR color)
{
    drawRect(bbox, color);
}

void DebugDraw::drawBoundingBox(const RECT& bbox, Viewport* viewport, D3DXCOLOR color)
{
    if (!viewport) return;

    // Transform bounding box from world space to screen space
    GVector3 topLeft = viewport->getPositionInViewport(&GVector3((float)bbox.left, (float)bbox.top, 0));
    GVector3 bottomRight = viewport->getPositionInViewport(&GVector3((float)bbox.right, (float)bbox.bottom, 0));

    // Draw transformed rect
    drawRect((float)topLeft.x, (float)topLeft.y, (float)bottomRight.x, (float)bottomRight.y, color);
}

void DebugDraw::drawCollisionBox(ICollidable* obj, D3DXCOLOR color)
{
    if (!obj) return;
    RECT bbox = obj->getBoundingBox();
    drawRect(bbox, color);
}

void DebugDraw::drawCollisionBox(ICollidable* obj, Viewport* viewport, D3DXCOLOR color)
{
    if (!obj || !viewport) return;
    
    RECT bbox = obj->getBoundingBox();
    
    // Transform bounding box from world space to screen space
    GVector3 topLeft = viewport->getPositionInViewport(&GVector3((float)bbox.left, (float)bbox.top, 0));
    GVector3 bottomRight = viewport->getPositionInViewport(&GVector3((float)bbox.right, (float)bbox.bottom, 0));
    
    // Draw transformed rect
    drawRect((float)topLeft.x, (float)topLeft.y, (float)bottomRight.x, (float)bottomRight.y, color);
}

#endif // _DEBUG