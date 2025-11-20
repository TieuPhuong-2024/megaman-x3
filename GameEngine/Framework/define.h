#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x800 
#endif

#pragma warning(disable: 4018)
#pragma warning(disable: 4066)
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#pragma warning(disable: 4715)
#pragma warning(disable: 4800)

#ifndef __GAMEUIT_FRAMEWORK__
#define __GAMEUIT_FRAMEWORK__

#define _USE_MATH_DEFINES

#include <d3d9.h>		
#include <d3dx9.h>	
#include <dinput.h>		
#include <dsound.h>		
#include <windows.h>
#include <exception>
#include <math.h>
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::vector;
using std::make_pair;

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 450/2
#define SCALE_FACTOR 1.0f

#define COLOR_WHITE D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f)				// màu trắnng
#define COLOR_KEY D3DXCOLOR(1.0f, 0.0f, 1.0f, 1.0f)				

#define ITEM_DESTROY_TIME 3000

#define MAX_OBJECTS 4
#define MAX_LEVELS 32

enum class eID
{
	NONE,
	XMAN,
	MAP_STAGE_21,
	MAP_STAGE_22,
	MAP_STAGE_23,
	MAP_STAGE_24,
	MAP_STAGE_MEGAMAN
};

enum class eStatus
{
	NONE = 1 << 0,
	STAND = 1 << 1,
	RUN = 1 << 2,
	JUMP = 1 << 3,
	FALL = 1 << 4,
	CLING = 1 << 5,
	DASH = 1 << 6,
	KICK = 1 << 7,
	SHOOT = 1 << 8,
	STAND_SHOOT = STAND | SHOOT,
	RUN_SHOOT = RUN | SHOOT,
	JUMP_SHOOT = JUMP | SHOOT,
	FALL_SHOOT = FALL | SHOOT,
	CLING_SHOOT = CLING | SHOOT,
	DASH_SHOOT = DASH | SHOOT,
	KICK_SHOOT = KICK | SHOOT,
};

enum class eDirection
{
	NONE = 0,
	TOP = 1,
	BOTTOM = 2,
	LEFT = 4,
	RIGHT = 8,
	TOP_BOTTOM = (TOP | BOTTOM),
	ALL = (TOP | BOTTOM | LEFT | RIGHT),
};

enum class eSoundId
{
	NONE,
	SHOOT,
	JUMP,
	FALL
};

typedef D3DXVECTOR3 GVector3;
typedef D3DXVECTOR2 GVector2;
#define VECTOR2ZERO GVector2(0.0f, 0.0f)
#define VECTOR2ONE  GVector2(1.0f, 1.0f)

#define EVENT_SOURCE [event_source(native)]
#define EVENT_RECEIVER [event_receiver(native)]

#ifndef SAFE_DELETE

#define SAFE_DELETE(p) \
if(p) \
{\
	delete (p); \
	p = nullptr; \
} \

#endif // !SAFE_DELETE


#endif // !__GAMEUIT_FRAMEWORK__
