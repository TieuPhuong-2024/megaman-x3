#include "InputController.h"
#include "Event.h"

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
	// collect state of all of keys.
	_keyboard->GetDeviceState(sizeof(this->_keyBuffer), _keyBuffer);

	// Update key states
    updateKey(DIK_LEFT, isKeyDown(DIK_LEFT));
    updateKey(DIK_RIGHT, isKeyDown(DIK_RIGHT));
    updateKey(DIK_X, isKeyDown(DIK_X));
    updateKey(DIK_C, isKeyDown(DIK_C));
    updateKey(DIK_Z, isKeyDown(DIK_Z));

	if (isKeyDown(DIK_ESCAPE))
	{
		PostMessage(_hWnd, WM_QUIT, 0, 0);
	}
}

int InputController::isKeyDown(int keycode)
{
	return ((_keyBuffer[keycode] & 0x80) > 0);
}

InputData InputController::getInputData()
{
	_inputData.left = _keyStates[DIK_LEFT];
    _inputData.right = _keyStates[DIK_RIGHT];
    _inputData.jump = _keyStates[DIK_X];
    _inputData.dash = _keyStates[DIK_C];
    _inputData.shoot = _keyStates[DIK_Z];
    return _inputData;
}

void InputController::updateKey(int keycode, bool isDown)
{
	KeyState& state = _keyStates[keycode];
    if (isDown) {
        state = (state == NONE || state == RELEASED) ? PRESSED : HELD;
    } else {
        state = (state == PRESSED || state == HELD) ? RELEASED : NONE;
    }
}
