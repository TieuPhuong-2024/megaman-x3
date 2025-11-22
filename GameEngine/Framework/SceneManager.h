#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <vector>
#include "define.h"
#include "Scene.h"


class SceneManager
{
public:
	~SceneManager() = default;
	static SceneManager& getInstance() {
		static SceneManager instance;
		return instance;
	}

	bool init();
	void updateInput(float dt);
	void update(float dt);
	void draw(LPD3DXSPRITE spriteHandle);
	void release();

	void addScene(Scene* scene);
	void removeScene();
	void replaceScene(Scene* scene);
	void clearScenes();
	Scene* getCurrentScene();

private:
	SceneManager() = default;
	SceneManager(const SceneManager&) = delete;
	void operator=(const SceneManager&) = delete;

	std::vector<Scene*> _scenes;
};

#endif // !__SCENE_MANAGER_H__
