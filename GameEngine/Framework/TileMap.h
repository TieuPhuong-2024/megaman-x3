#ifndef __MAP_H__
#define __MAP_H__
#include "TileSet.h"
#include <vector>
#include <memory>
#include "../Object/CWall.h"
using namespace std;

class TileMap
{
public:
	void draw(LPD3DXSPRITE, Viewport *);
	void release();

	static std::unique_ptr<TileMap> LoadFromFile(const string &path, eID spriteId);

	GVector2 getWorldSize();

	TileMap();
	~TileMap();
	int worldWidth();
	int worldHeight();
	int getCheckpoint();

	void setCheckpoint(int checkPoint);

	void setColor(D3DXCOLOR color);

	std::vector<std::unique_ptr<CWall>> GetWalls() { return std::move(_walls); }

private:
	int _frameWidth;
	int _frameHeight;

	int _checkPoint;

	// Kích thước map tính theo số ô Tile.
	GVector2 _mapSize;

	vector<vector<int>> _mapIndex;

	unique_ptr<TileSet> _tileSet;

	std::vector<std::unique_ptr<CWall>> _walls;

	void getElementMatrixIndex(xml_node &node);
	void loadWalls(xml_node &mapNode);
};

#endif // !__MAP_H__
