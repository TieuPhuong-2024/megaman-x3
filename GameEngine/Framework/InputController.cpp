#include "InputController.h"
#include "../Observer.h"

InputController::InputController()
{
	_input = nullptr;
	_keyboard = nullptr;
	ZeroMemory(_keyBuffer, 256);
}

InputController::~InputController()
{
	if (_input != nullptr)
		this->_input->Release();
	if (_keyboard != nullptr)
		this->_keyboard->Release();
}

void InputController::release()
{
}

bool InputController::init(HWND hWnd, HINSTANCE hInstance)
{
	this->_hWnd = hWnd;
	HRESULT rs = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void **>(&_input),
		nullptr);
	if (rs != DI_OK)
		return false;

	rs = _input->CreateDevice(GUID_SysKeyboard, reinterpret_cast<LPDIRECTINPUTDEVICEW *>(&_keyboard), nullptr);
	if (rs != DI_OK)
		return false;

	rs = _keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (rs != DI_OK)
		return false;

	rs = _keyboard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (rs != DI_OK)
		return false;

	// Set Property for keyboard buffer.
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.diph.dwObj = 0;
	dipdw.dwData = KEYBOARD_BUFFER_SIZE;

	rs = _keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
	if (rs != DI_OK)
		return false;

	rs = _keyboard->Acquire();
	if (rs != DI_OK)
		return false;

	return true;
}

void InputController::update()
{
	for (int i = 0; i < 256; i++)
	{
		_previousKeyBuffer[i] = ((_keyBuffer[i] & 0x80) > 0);
	}

	// collect state of all of keys.
	_keyboard->GetDeviceState(sizeof(this->_keyBuffer), _keyBuffer);
	if (isKeyDown(DIK_ESCAPE))
	{
		PostMessage(_hWnd, WM_QUIT, 0, 0);
	}

	DWORD dw = KEYBOARD_BUFFER_SIZE;
	HRESULT rs = _keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), _keyEvents, &dw, 0);

	for (DWORD i = 0; i < dw; i++) // not use for each statement.
	{
		int keycode = _keyEvents[i].dwOfs;
		int keyState = _keyEvents[i].dwData;
		if ((keyState & 0x80) > 0)
		{
			KeyEventArg *arg = new KeyEventArg(keycode);
			NotifyKeyPress(arg);
			delete arg;
		}
		else
		{
			KeyEventArg *arg = new KeyEventArg(keycode);
			NotifyKeyRelease(arg);
			delete arg;
		}
	}
}

int InputController::isKeyDown(int keycode)
{
	return ((_keyBuffer[keycode] & 0x80) > 0);
}

int InputController::isKeyUp(int nKeyCode)
{
	return !isKeyDown(nKeyCode);
}

bool InputController::isKeyPressed(int keycode)
{
	// Don't use
	return isKeyDown(keycode) && !(_previousKeyBuffer[keycode]);
}

bool InputController::isKeyRelease(int keycode)
{
	// Don't use
	return !isKeyDown(keycode) && (_previousKeyBuffer[keycode]);
}

void InputController::Attach(IObserver *observer)
{
	auto it = std::find(_observers.begin(), _observers.end(), observer);
	if (it == _observers.end())
		_observers.push_back(observer);
}

void InputController::Detach(IObserver *observer)
{
	// Remove observer
	auto it = std::find(_observers.begin(), _observers.end(), observer);
	if (it == _observers.end())
		return;
	_observers.erase(it);
}

void InputController::NotifyKeyPress(KeyEventArg *e)
{
	for (auto ob : _observers)
	{
		ob->eventKeyDown(e);
	}
}

void InputController::NotifyKeyRelease(KeyEventArg *e)
{
	for (auto ob : _observers)
	{
		auto keyRelease = (IObserverKeyUp *)ob;
		keyRelease->eventKeyUp(e);
	}
}
