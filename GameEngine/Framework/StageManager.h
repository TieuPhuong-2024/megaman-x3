#ifndef __STAGEMANAGER_H__
#define __STAGEMANAGER_H__

#include <map>
#include <fstream>
#include "define.h"
#include "Sprite.h"
#include "TileMap.h"
#include <memory>
#include <string>

class StageManager
{
public:
	~StageManager() = default;
	static StageManager& getInstance()
	{
		static StageManager instance;
		return instance;
	}

	TileMap* getTileMap(eID id);
	void addStage(eID id, const string& path);
	std::string getStagePath(eID id);

    void clearCache();

private:
	StageManager() = default;
	std::map<eID, std::unique_ptr<TileMap>> _tileMaps;
	std::map<eID, string> _resourcePath;
};


#endif // !__SPRITEMANAGER_H__
