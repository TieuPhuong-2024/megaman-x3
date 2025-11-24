#include "StageManager.h"

TileMap* StageManager::getTileMap(eID id)
{
	// Check if the TileMap is already loaded
    auto it = _tileMaps.find(id);
    if (it != _tileMaps.end()) {
        return it->second.get();
    }
    
    // Load the TileMap if not found
    auto tileMap = TileMap::LoadFromFile(_resourcePath[id], id);
    TileMap* result = tileMap.get();
    _tileMaps[id] = std::move(tileMap);
    return result;
}

void StageManager::addStage(eID id, const string &path)
{
	_resourcePath.insert(make_pair(id, path));
}

std::string StageManager::getStagePath(eID id)
{
    auto it = _resourcePath.find(id);
    return (it != _resourcePath.end()) ? it->second : "";
}

void StageManager::clearCache()
{
	_tileMaps.clear();
}
