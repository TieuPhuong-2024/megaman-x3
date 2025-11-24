#ifndef __INPUT_CTRL_H__
#define __INPUT_CTRL_H__

#pragma warning(disable : 4467)

#include "define.h"
#include "Graphics.h"
#include "Event.h"
#include <vector>
#include <list>

#define KEYBOARD_BUFFER_SIZE 1024

using pGInput = LPDIRECTINPUT;
using pGKeyboard = LPDIRECTINPUTDEVICE8;

class InputController
{
public:
	~InputController();
	static InputController &getInstance()
	{
		static InputController instance;
		return instance;
	}

	static void release();

	bool init(HWND, HINSTANCE); // should be called in game::init()
	void update();				// should be called in main game loop
	int isKeyDown(int keycode); // check if keycode is down.
	int isKeyUp(int nKeyCode);
	bool isKeyPressed(int keycode);
	bool isKeyRelease(int keycode);

private:
	InputController();

	pGInput _input;
	pGKeyboard _keyboard;
	BYTE _keyBuffer[256]; // BYTE = unsigned char
	HWND _hWnd;			  // reference to another place. Don't release!!!
	DIDEVICEOBJECTDATA _keyEvents[KEYBOARD_BUFFER_SIZE];

	bool _previousKeyBuffer[256];
};

using pInputController = InputController *;

#endif // !__INPUT_CTRL_H__
