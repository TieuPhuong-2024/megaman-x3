#include "PlayerState.h"
#include "../Framework/IComponent.h"

PlayerState::PlayerState(CPlayer *player) : _player(player)
{
	_movement = (Movement *)_player->getComponent("Movement");
	_gravity = (Gravity *)_player->getComponent("Gravity");
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

void PlayerState::setState(std::unique_ptr<PlayerState> newState, float transitionTime)
{
	_player->setState(std::move(newState));
}

CPlayer *PlayerState::getPlayer()
{
	return _player;
}
