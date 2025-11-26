#include "PlayerState.h"
#include "../Framework/IComponent.h"

PlayerState::PlayerState(CPlayer *player) : _player(player)
{
	_movement = (Movement *)_player->GetComponent("Movement");
	_gravity = (Gravity *)_player->GetComponent("Gravity");
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

void PlayerState::ChangeState(std::unique_ptr<PlayerState> newState, float transitionTime)
{
	_player->ChangeState(std::move(newState));
}

CPlayer *PlayerState::getPlayer()
{
	return _player;
}
