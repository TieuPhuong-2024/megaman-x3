
#include "TileSet.h"
#include "Tile.h"
#include "SpriteManager.h"
#include "../trace.h"

TileSet::TileSet(eID spriteId)
{
	auto sp = SpriteManager::getInstance().getSprite(spriteId);
	this->_tileImage = sp;
	this->_heighttile = sp->getFrameHeight();
	this->_widthtile = sp->getFrameWidth();
	_columns = 0;
	_rows = 0;
}

TileSet::~TileSet()
{
	GAMELOG("~TileSet");
}

void TileSet::draw(LPD3DXSPRITE spriteHandle, int id, GVector2 position, Viewport *viewport)
{
	for (auto &tile : _listTiles)
	{
		if (tile->getId() == id)
		{
			tile->draw(spriteHandle, position, viewport);
			break;
		}
	}
}

void TileSet::loadListTiles(pugi::xml_node &tileset)
{
	auto image = tileset.child("image");
	auto imageWidth = image.attribute("width").as_int();

	auto firstTileId = tileset.attribute("firstgid").as_int();
	auto tileWidth = tileset.attribute("tilewidth").as_int();
	auto tileHeight = tileset.attribute("tileheight").as_int();
	auto tileCount = tileset.attribute("tilecount").as_int();

	Tile *tile = nullptr;
	RECT srcRECT = {0, 0, tileWidth, tileHeight};

	while (tileCount--)
	{
		this->_listTiles.push_back(std::unique_ptr<Tile>(new Tile(this->_tileImage.get(), srcRECT, firstTileId)));
		firstTileId++;
		if (srcRECT.right + tileWidth > imageWidth)
		{
			srcRECT.left = 0;
			srcRECT.top += tileHeight;
		}
		else
			srcRECT.left += tileWidth;
		srcRECT.right = srcRECT.left + tileWidth;
		srcRECT.bottom = srcRECT.top + tileHeight;
	}
}

int TileSet::getWidthtile() const
{
	return _widthtile;
}

void TileSet::setColor(D3DXCOLOR color)
{
	_tileImage->setColor(color);
}

void TileSet::setWidthtile(const int &value)
{
	this->_widthtile = value;
}

int TileSet::getHeighttile() const
{
	return this->_heighttile;
}

void TileSet::setHeighttile(const int &value)
{
	this->_heighttile = value;
}

Sprite *TileSet::getSprite()
{
	return this->_tileImage.get();
}
