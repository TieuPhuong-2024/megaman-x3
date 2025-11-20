#include "StageManager.h"

StageManager* StageManager::_instance = nullptr;
TileMap* StageManager::_tileMap = nullptr;

StageManager::StageManager(void)
{
}

StageManager::~StageManager(void)
{
}

StageManager* StageManager::getInstance()
{
	if (_instance == nullptr)
		_instance = new StageManager();
	return _instance;
}

//list<BaseObject*>* StageManager::getListObject(eID id)
//{
//	return GetListObjectFromFile(_resourcePath[id]);
//}

map<eID, string>* StageManager::getListStage()
{
	return &_resourcePath;
}

TileMap* StageManager::getTileMap(eID id)
{
	_tileMap = TileMap::LoadFromFile(_resourcePath[id], id);
	return _tileMap;
}

void StageManager::release()
{
}

TileMap* StageManager::getCurrentTileMap()
{
	return  _tileMap;
}
