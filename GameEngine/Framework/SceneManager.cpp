#include "SceneManager.h"

bool SceneManager::init()
{
	if (!_scenes.empty())
		_scenes.back()->init();

	return true;
}

void SceneManager::updateInput(float dt)
{
	if (!_scenes.empty())
		_scenes.back()->updateInput(dt);
}

void SceneManager::update(float dt)
{
	if (!_scenes.empty())
		_scenes.back()->update(dt);
}

void SceneManager::draw(LPD3DXSPRITE spriteHandle)
{
	if (!_scenes.empty())
		_scenes.back()->draw(spriteHandle);
}

void SceneManager::release()
{
	if (!_scenes.empty())
	{
		_scenes.back()->release();
	}
}

void SceneManager::addScene(std::unique_ptr<Scene> scene)
{
	_scenes.push_back(std::move(scene));
	_scenes.back()->init();
}

void SceneManager::removeScene()
{
	if (!_scenes.empty())
	{
		_scenes.back()->release();
		_scenes.pop_back();
	}
}

void SceneManager::replaceScene(std::unique_ptr<Scene> scene)
{
	this->removeScene();
	this->addScene(std::move(scene));
}

void SceneManager::clearScenes()
{
	while (!_scenes.empty())
	{
		_scenes.back()->release();
		_scenes.pop_back();
	}
}

Scene *SceneManager::getCurrentScene()
{
	if (!_scenes.empty())
		return _scenes.back().get();

	return nullptr;
}
