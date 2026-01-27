#include "Player.h"
#include "../Framework/Animation.h"
#include "../Framework/IComponent.h"
#include "../Framework/SpriteManager.h"
#include "../Framework/InputController.h"
#include "../States/PlayerState.h"

Player::Player(GVector2 position)
    : BaseObject(), _position(position), _scale(VECTOR2ONE), _facingDirection(eDirection::RIGHT)
{
    this->SetID((int)eID::XMAN);
}

Player::~Player()
{
    release();
}

void Player::init()
{
    initComponents();
    initAnimations();
    
    // Start with Standing state
    setState(new Standing(this));
}

void Player::initComponents()
{
    // Get the sprite sheet from SpriteManager
    auto& spriteManager = SpriteManager::getInstance();
    auto xmanSprite = spriteManager.getSprite(eID::XMAN);
    
    if (!xmanSprite)
    {
        // Error handling - sprite not loaded
        return;
    }
    
    // Initialize Movement component
    _movement = std::make_unique<Movement>(
        GVector2(0.0f, 0.0f),  // Acceleration
        GVector2(0.0f, 0.0f), // Initial velocity
        xmanSprite.get()
    );
    
    // Initialize Gravity component
    _gravity = std::make_unique<Gravity>(
        GVector2(0.0f, 1000.0f), // Gravity force
        _movement.get()
    );
    
    // Initialize Animation component
    _animation = std::make_unique<Animation>(
        xmanSprite.get(),
        0.1f,  // Animation time between frames
        true,  // Loop animation
        10     // Expected number of frames
    );
}

void Player::initAnimations()
{
    // Map eStatus to animation frame names from XML
    _animationMap[eStatus::STAND] = "standing";
    _animationMap[eStatus::RUN] = "running";
    _animationMap[eStatus::JUMP] = "jumping";
    _animationMap[eStatus::FALL] = "falling";
}

void Player::setState(PlayerState* newState)
{
    if (!newState) return;
    
    if (_currentState)
    {
        _currentState->onExit();
    }
    
    _currentState.reset(newState);
    _currentState->onEnter();
    
    // Update animation based on new state
    setAnimation(_currentState->getStateStatus());
}

eStatus Player::getCurrentStatus() const
{
    if (_currentState)
    {
        return _currentState->getStateStatus();
    }
    return eStatus::NONE;
}

void Player::setAnimation(eStatus status)
{
    if (!_animation) return;
    
    auto it = _animationMap.find(status);
    if (it != _animationMap.end())
    {
        string animationName = it->second;
        
        // Clear current animation frames
        // Note: Animation class needs a clearFrames method or similar
        
        // Add appropriate frames based on status
        switch (status)
        {
        case eStatus::STAND:
            _animation->addFrameRect(eID::XMAN, "standing_1", "standing_2", nullptr);
            _animation->setTimeAnimate(0.15f);
            break;
            
        case eStatus::RUN:
            _animation->addFrameRect(eID::XMAN, 
                "running_1", "running_2", "running_3", "running_4", "running_5", 
                "running_6", "running_7", "running_8", "running_9", nullptr);
            _animation->setTimeAnimate(0.08f);
            break;
            
        case eStatus::JUMP:
            _animation->addFrameRect(eID::XMAN, "jumping_1", "jumping_2", "jumping_3", nullptr);
            _animation->setTimeAnimate(0.1f);
            _animation->setLoop(false);
            break;
            
        case eStatus::FALL:
            _animation->addFrameRect(eID::XMAN, "falling_1", "falling_2", "falling_3", "falling_4", nullptr);
            _animation->setTimeAnimate(0.12f);
            break;
            
        default:
            break;
        }
        
        _animation->restart();
    }
}

void Player::update(float dt)
{
    // Update components
    if (_movement)
        _movement->update(dt);
    
    if (_gravity)
        _gravity->update(dt);
    
    if (_animation)
        _animation->update(dt);
    
    // Update current state
    if (_currentState)
        _currentState->update(dt);
    
    // Update sprite position
    if (_animation)
    {
        auto sprite = _animation->getSpriteSheet();
        if (sprite)
        {
            // Handle facing direction
            if (_facingDirection == eDirection::LEFT)
            {
                sprite->setScale(GVector2(-_scale.x, _scale.y));
            }
            else
            {
                sprite->setScale(GVector2(_scale.x, _scale.y));
            }
        }
    }
}

void Player::draw(LPD3DXSPRITE spriteHandle, Viewport* viewport)
{
    if (_animation)
    {
        _animation->draw(spriteHandle, viewport);
    }
}

void Player::release()
{
    _movement.reset();
    _gravity.reset();
    _animation.reset();
    _currentState.reset();
}

RECT Player::GetBoundingBox() const
{
    if (_animation && _animation->getSpriteSheet())
    {
        auto sprite = _animation->getSpriteSheet();
        auto currentRect = _animation->getCurrentRect();
        
        // Get sprite dimensions and calculate bounding box
        float width = (float)(currentRect.right - currentRect.left);
        float height = (float)(currentRect.bottom - currentRect.top);
        
        RECT boundingBox;
        boundingBox.left = (long)(_position.x - width / 2);
        boundingBox.right = (long)(_position.x + width / 2);
        boundingBox.top = (long)(_position.y - height / 2);
        boundingBox.bottom = (long)(_position.y + height / 2);
        
        return boundingBox;
    }
    
    // Default bounding box if no sprite
    RECT boundingBox;
    boundingBox.left = (long)_position.x - 16;
    boundingBox.right = (long)_position.x + 16;
    boundingBox.top = (long)_position.y - 24;
    boundingBox.bottom = (long)_position.y + 24;
    
    return boundingBox;
}

GVector2 Player::GetVelocity() const
{
    if (_movement)
    {
        return _movement->getVelocity();
    }
    return VECTOR2ZERO;
}