#include "Scene.h"

Scene::Scene()
{
	_viewport = std::make_unique<Viewport>();
}

Scene::~Scene()
{
}

Viewport *Scene::getViewport()
{
	return _viewport.get();
}
