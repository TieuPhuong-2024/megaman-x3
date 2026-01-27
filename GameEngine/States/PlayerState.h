#pragma once
#include "../Framework/define.h"
#include "../Framework/IComponent.h"
#include "../Framework/InputController.h"

class Player;

class PlayerState
{
public:
    PlayerState(Player* player);
    virtual ~PlayerState() = default;

    virtual void update(float dt) = 0;
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual eStatus getStateStatus() = 0;

protected:
    Player* _player;
};

class Standing : public PlayerState
{
public:
    Standing(Player* player);
    void update(float dt) override;
    void onEnter() override;
    void onExit() override;
    eStatus getStateStatus() override;

private:
    float _idleTimer;
};

class Running : public PlayerState
{
public:
    Running(Player* player);
    void update(float dt) override;
    void onEnter() override;
    void onExit() override;
    eStatus getStateStatus() override;

private:
    float _runSpeed;
};

class Jumping : public PlayerState
{
public:
    Jumping(Player* player);
    void update(float dt) override;
    void onEnter() override;
    void onExit() override;
    eStatus getStateStatus() override;

private:
    float _jumpVelocity;
    float _gravity;
    bool _hasReachedPeak;
};

class Falling : public PlayerState
{
public:
    Falling(Player* player);
    void update(float dt) override;
    void onEnter() override;
    void onExit() override;
    eStatus getStateStatus() override;

private:
    float _gravity;
};