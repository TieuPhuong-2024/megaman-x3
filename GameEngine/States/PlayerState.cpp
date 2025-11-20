#include "PlayerState.h"
#include "../Framework/IComponent.h"

CPlayer* PlayerState::_player = nullptr;

PlayerState::PlayerState()
{
	_movement = (Movement*)_player->getComponent("Movement");
	_gravity = (Gravity*)_player->getComponent("Gravity");
}

PlayerState::~PlayerState()
{
}

void PlayerState::update(float deltaTime)
{
}

void PlayerState::updateInput(float deltaTime)
{
}

void PlayerState::setState(PlayerState* newState, float transitionTime)
{
	_player->setState(newState, transitionTime);
}

void PlayerState::setPlayer(CPlayer* player)
{
	_player = player;
}

CPlayer* PlayerState::getPlayer()
{
	return _player;
}
