#include "PlayerState.h"
#include "../Object/Player.h"
#include "../Framework/InputController.h"

PlayerState::PlayerState(Player* player)
    : _player(player)
{
}

// Standing State Implementation
Standing::Standing(Player* player)
    : PlayerState(player), _idleTimer(0.0f)
{
}

void Standing::onEnter()
{
    _idleTimer = 0.0f;
    // Reset velocity when standing
    auto movement = _player->getMovement();
    if (movement)
    {
        movement->setVelocity(VECTOR2ZERO);
    }
}

void Standing::update(float dt)
{
    _idleTimer += dt;
    
    auto input = InputController::getInstance();
    auto movement = _player->getMovement();
    
    if (!movement) return;
    
    // Check for left/right movement input
    if (input->isKeyDown(DIK_LEFT) || input->isKeyDown(DIK_RIGHT))
    {
        _player->setState(new Running(_player));
        return;
    }
    
    // Check for jump input
    if (input->isKeyDown(DIK_X) || input->isKeyDown(DIK_SPACE))
    {
        _player->setState(new Jumping(_player));
        return;
    }
}

void Standing::onExit()
{
    // Cleanup if needed
}

eStatus Standing::getStateStatus()
{
    return eStatus::STAND;
}

// Running State Implementation
Running::Running(Player* player)
    : PlayerState(player), _runSpeed(200.0f)
{
}

void Running::onEnter()
{
    // Initialize running state
}

void Running::update(float dt)
{
    auto input = InputController::getInstance();
    auto movement = _player->getMovement();
    auto gravity = _player->getGravity();
    
    if (!movement) return;
    
    // Handle horizontal movement
    GVector2 velocity = movement->getVelocity();
    velocity.x = 0.0f;
    
    if (input->isKeyDown(DIK_LEFT))
    {
        velocity.x = -_runSpeed;
        _player->setFacingDirection(eDirection::LEFT);
    }
    else if (input->isKeyDown(DIK_RIGHT))
    {
        velocity.x = _runSpeed;
        _player->setFacingDirection(eDirection::RIGHT);
    }
    else
    {
        // No horizontal input, return to standing
        _player->setState(new Standing(_player));
        return;
    }
    
    movement->setVelocity(velocity);
    
    // Check for jump input
    if (input->isKeyDown(DIK_X) || input->isKeyDown(DIK_SPACE))
    {
        _player->setState(new Jumping(_player));
        return;
    }
    
    // Check if player is falling (not on ground)
    if (gravity && gravity->getStatus() == eGravityStatus::FALLING__DOWN)
    {
        _player->setState(new Falling(_player));
        return;
    }
}

void Running::onExit()
{
    // Cleanup if needed
}

eStatus Running::getStateStatus()
{
    return eStatus::RUN;
}

// Jumping State Implementation
Jumping::Jumping(Player* player)
    : PlayerState(player), _jumpVelocity(-400.0f), _gravity(1000.0f), _hasReachedPeak(false)
{
}

void Jumping::onEnter()
{
    _hasReachedPeak = false;
    
    auto movement = _player->getMovement();
    if (movement)
    {
        GVector2 velocity = movement->getVelocity();
        velocity.y = _jumpVelocity; // Negative for upward movement
        movement->setVelocity(velocity);
    }
    
    // Set gravity to falling state
    auto gravity = _player->getGravity();
    if (gravity)
    {
        gravity->setStatus(eGravityStatus::FALLING__DOWN);
    }
}

void Jumping::update(float dt)
{
    auto movement = _player->getMovement();
    auto input = InputController::getInstance();
    
    if (!movement) return;
    
    // Handle horizontal movement during jump
    GVector2 velocity = movement->getVelocity();
    
    if (input->isKeyDown(DIK_LEFT))
    {
        velocity.x = -150.0f; // Reduced air control
        _player->setFacingDirection(eDirection::LEFT);
    }
    else if (input->isKeyDown(DIK_RIGHT))
    {
        velocity.x = 150.0f; // Reduced air control
        _player->setFacingDirection(eDirection::RIGHT);
    }
    else
    {
        velocity.x = 0.0f;
    }
    
    movement->setVelocity(velocity);
    
    // Check if we've reached the peak of the jump
    if (velocity.y >= 0 && !_hasReachedPeak)
    {
        _hasReachedPeak = true;
    }
    
    // Transition to falling when moving downward
    if (velocity.y > 50.0f)
    {
        _player->setState(new Falling(_player));
        return;
    }
}

void Jumping::onExit()
{
    // Cleanup if needed
}

eStatus Jumping::getStateStatus()
{
    return eStatus::JUMP;
}

// Falling State Implementation
Falling::Falling(Player* player)
    : PlayerState(player), _gravity(1000.0f)
{
}

void Falling::onEnter()
{
    // Set gravity to ensure we're falling
    auto gravity = _player->getGravity();
    if (gravity)
    {
        gravity->setStatus(eGravityStatus::FALLING__DOWN);
    }
}

void Falling::update(float dt)
{
    auto movement = _player->getMovement();
    auto gravity = _player->getGravity();
    auto input = InputController::getInstance();
    
    if (!movement) return;
    
    // Handle horizontal movement during fall
    GVector2 velocity = movement->getVelocity();
    
    if (input->isKeyDown(DIK_LEFT))
    {
        velocity.x = -150.0f; // Reduced air control
        _player->setFacingDirection(eDirection::LEFT);
    }
    else if (input->isKeyDown(DIK_RIGHT))
    {
        velocity.x = 150.0f; // Reduced air control
        _player->setFacingDirection(eDirection::RIGHT);
    }
    else
    {
        velocity.x = 0.0f;
    }
    
    movement->setVelocity(velocity);
    
    // Check if we've landed (gravity status indicates on ground)
    if (gravity && gravity->getStatus() == eGravityStatus::SHALLOWED)
    {
        _player->setState(new Standing(_player));
        return;
    }
    
    // Check for wall cling if moving horizontally and against wall
    // (This would require collision detection implementation)
}

void Falling::onExit()
{
    // Cleanup if needed
}

eStatus Falling::getStateStatus()
{
    return eStatus::FALL;
}