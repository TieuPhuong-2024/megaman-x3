#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <vector>
#include <memory>
#include "define.h"
#include "Scene.h"

class SceneManager
{
public:
	~SceneManager() = default;
	static SceneManager &getInstance()
	{
		static SceneManager instance;
		return instance;
	}

	bool init();
	void updateInput(float dt);
	void update(float dt);
	void draw(LPD3DXSPRITE spriteHandle);
	void release();

	void addScene(std::unique_ptr<Scene> scene);
	void removeScene();
	void replaceScene(std::unique_ptr<Scene> scene);
	void clearScenes();
	Scene *getCurrentScene();

private:
	SceneManager() = default;
	SceneManager(const SceneManager &) = delete;
	void operator=(const SceneManager &) = delete;

	std::vector<std::unique_ptr<Scene>> _scenes;
};

#endif // !__SCENE_MANAGER_H__
