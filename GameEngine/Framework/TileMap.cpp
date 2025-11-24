#include "TileMap.h"
#include "../trace.h"

TileMap::TileMap()
{
	_checkPoint = -1;
	_frameWidth = 0;
	_frameHeight = 0;
}

TileMap::~TileMap()
{
	GAMELOG("~TileMap");
}

void TileMap::release()
{
}

void TileMap::draw(LPD3DXSPRITE spriteHandle, Viewport *viewport)
{
	RECT screenRectEx =
		{
			(LONG)viewport->getPositionWorld().x,
			(LONG)viewport->getPositionWorld().y,
			(LONG)(viewport->getPositionWorld().x + viewport->getWidth()),
			(LONG)(viewport->getPositionWorld().y - viewport->getHeight())};

	int colBegin = max(screenRectEx.left / _frameWidth, 0);
	int colEnd = min(screenRectEx.right / _frameWidth + 1, _mapSize.x);
	int rowBegin = _mapSize.y - min(screenRectEx.top / _frameHeight + 1, _mapSize.y);
	int rowEnd = _mapSize.y - max(screenRectEx.bottom / _frameHeight, 0);

	GVector2 pos;

	for (int col = colBegin; col < colEnd; col++)
	{
		for (int row = rowBegin; row < rowEnd; row++)
		{
			pos.x = col * _frameWidth;
			pos.y = (_mapSize.y - row - 1) * _frameHeight;
			this->_tileSet->draw(spriteHandle, this->_mapIndex[row][col], pos, viewport);
		}
	}
}

GVector2 TileMap::getWorldSize()
{
	GVector2 result;
	result.x = this->_mapSize.x * this->_frameWidth;
	result.y = this->_mapSize.y * this->_frameHeight;
	return result;
}

void TileMap::setCheckpoint(int checkPoint)
{
	_checkPoint = checkPoint;
}

void TileMap::setColor(D3DXCOLOR color)
{
	_tileSet->setColor(color);
}

std::unique_ptr<TileMap> TileMap::LoadFromFile(const string &path, eID spriteId)
{
	xml_document doc;

	xml_parse_result result = doc.load_file(path.data(), parse_default | parse_pi);
	if (result == false)
	{
		return nullptr;
	}
	auto tileMap = std::make_unique<TileMap>();

	xml_node map = doc.child("map");
	if (map == NULL)
		return nullptr;

	xml_node properties = map.child("properties");
	if (properties != NULL)
	{
		tileMap->setCheckpoint(properties.child("property").attribute("value").as_int() * 2);
	}

	xml_node tileset = map.child("tileset");
	tileMap->_tileSet = make_unique<TileSet>(spriteId);
	tileMap->_tileSet->loadListTiles(tileset);

	xml_node layer = map.child("layer");
	tileMap->_mapSize.x = layer.attribute("width").as_int();
	tileMap->_mapSize.y = layer.attribute("height").as_int();

	tileMap->_mapIndex.assign(tileMap->_mapSize.y, vector<int>(tileMap->_mapSize.x, 0));

	tileMap->getElementMatrixIndex(layer);

	// Set frame dimensions BEFORE loading walls (walls need these for coordinate conversion)
	tileMap->_frameWidth = tileMap->_tileSet->getSprite()->getFrameWidth();
	tileMap->_frameHeight = tileMap->_tileSet->getSprite()->getFrameHeight();

	tileMap->loadWalls(map);

	return tileMap;
}

void TileMap::getElementMatrixIndex(xml_node &node)
{
	auto elements = node.child("data").children();
	auto row = 0, col = 0;

	for (auto element : elements)
	{
		this->_mapIndex[row][col] = element.attribute("gid").as_int();
		col++;
		if (col >= this->_mapSize.x)
		{
			col = 0;
			row++;
		}
	}
}

void TileMap::loadWalls(xml_node &mapNode)
{
	xml_node objectgroup = mapNode.find_child_by_attribute("objectgroup", "name", "Wall");
	if (objectgroup)
	{
		// Get map height for coordinate conversion
		float mapHeight = _mapSize.y * _frameHeight;

		for (xml_node object : objectgroup.children("object"))
		{
			auto wall = std::make_unique<CWall>();
			wall->SetId(object.attribute("id").as_int());

			// TMX uses screen coordinates (Y down from top-left)
			float tmxX = object.attribute("x").as_float();
			float tmxY = object.attribute("y").as_float();
			float width = object.attribute("width").as_float() * 2;
			float height = object.attribute("height").as_float() * 2;

			// Convert from TMX screen coords to world coords
			// TMX: Y is top-left, measured down from top
			// World: Y is bottom-left, measured up from bottom
			// TMX bottom = tmxY + height, World bottom = mapHeight - (tmxY + height)
			float worldX = tmxX * 2;
			float worldY = mapHeight - tmxY * 2 - height;

			printLog("[TileMap] Wall ID=%d TMX(x=%.1f,y=%.1f,w=%.1f,h=%.1f) -> World(x=%.1f,y=%.1f) mapH=%.0f\n",
					 wall->GetId(), tmxX, tmxY, width, height, worldX, worldY, mapHeight);

			wall->SetX(worldX);
			wall->SetY(worldY);
			wall->SetWidth(width);
			wall->SetHeight(height);
			_walls.push_back(std::move(wall));
		}
	}
}

int TileMap::worldHeight()
{
	return _frameHeight * _mapSize.y;
}

int TileMap::worldWidth()
{
	return _frameWidth * _mapSize.x;
}

int TileMap::getCheckpoint()
{
	return _checkPoint;
}
