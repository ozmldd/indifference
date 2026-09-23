#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>
#include "animation.h"

enum class PlayerState
{
    idle, running, jumping, dead
};
enum class BulletState
{
    moving, colliding, inactive
};

enum class EnemyState
{
    roaming, damaged, dead
};
struct PlayerData
{
    PlayerState state;
    timer weaponTimer;
    timer invincibilityTimer;
    int healthPoints;

    PlayerData() : weaponTimer(0.35f), invincibilityTimer(0.5f),  healthPoints(100)
    {
        invincibilityTimer.reset();
        state = PlayerState::idle;
    }
};

struct LevelData{};
struct EnemyData{
    EnemyState state;
    timer damagedTimer;
    int healthPoints;


    EnemyData() : state(EnemyState::roaming), damagedTimer(0.5f)
    {
        healthPoints = 100;
    }
};
struct BulletData
{
    BulletState state;
    BulletData() : state(BulletState :: moving){

    }
}; 

union ObjectData{
    PlayerData player;
    LevelData level;
    EnemyData enemy;
    BulletData bullet;
};

enum class ObjectType{
    player, level, enemy, bullet
 };

 struct GameObject{
    ObjectType type;
    ObjectData data;
    glm::vec2 position, velocity, acceleration;
    float direction;
    float maxSpeedX;
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture *texture;
    bool dynamic;
    bool grounded;
    SDL_FRect collider;
    timer flashTimer;
    bool shouldFlash;
    int spriteFrame;


    GameObject() : data{ .level = LevelData() } , collider { 0 }, flashTimer(0.05f)
    {
        type = ObjectType::level;
        direction = 1;
        maxSpeedX = 0;
        position = velocity = acceleration = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
        dynamic = false;
        grounded = false;
        shouldFlash = false;
        spriteFrame = 1;
    }
 };
