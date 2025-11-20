#ifndef __STAGEMANAGER_H__
#define __STAGEMANAGER_H__

#include <map>
#include <fstream>
#include "define.h"
#include "Sprite.h"
#include "TileMap.h"

class StageManager
{
public:
	static StageManager* getInstance();

	static void release();

	TileMap* getTileMap(eID id);

	static TileMap* getCurrentTileMap();

	//list<BaseObject*>* getListObject(eID id);

	map<eID, string>* getListStage();

	~StageManager(void);

private:
	StageManager(void);
	
	static StageManager* _instance;
	static TileMap* _tileMap;

	map<eID, string> _resourcePath;

};


#endif // !__SPRITEMANAGER_H__
