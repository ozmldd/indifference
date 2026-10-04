#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "animation.h"
#include <SDL3/SDL.h>

enum class PlayerState
{
    idle, running, cutscene, walking, dialog, ended, switchlvl
};

enum class NPCState
{
    dialog, following, standing,
};
enum class creditState
{
    inactive, active,
};

enum class gameMode
{
    warning, title, optionMenu, playing, paused, credits, flash
};

struct playerData
{
    PlayerState state;
    playerData()
    {
        state = PlayerState::idle;
    }
};
struct credittData
{
    creditState state;
    credittData() : state(creditState::active)
    {

    }
};

struct EventData
{

};

struct LevelData
{

};
struct NPCData
{
    NPCState state;
    int NPCId;
    NPCData() : state(NPCState::standing), NPCId(0) {}
};

struct Button
{
    SDL_FRect rect
    {
        .x = 0, .y = 60,
        .w = 100, .h = 60,
    };
    SDL_Texture *texture;

    int ButtonId;
    Button() : ButtonId(0){}

};
struct ButtonGame
{
    SDL_FRect rectt 
    {
        .x = 0, .y = 40,
        .w = 20, .h = 40,
    };
    SDL_Texture *texture;

    int ButtonId;
    ButtonGame() : ButtonId(0){}
};

struct Slider
{
    SDL_FRect slide
    {
        .x = 200, .y = 265,
        .w = 250, .h = 80
    };
    SDL_Texture *texture;
    float value = 1.0f;
};

struct Handle
{   
    SDL_FRect handler
    {
        .x = 195, .y = 275.5,
        .w = 100, .h = 50
    };
    SDL_Texture *texture;

};

union ObjectData
{
    playerData player;
    LevelData level;
    NPCData NPC;
    EventData event;
};

enum class ObjectType
{
    player, level, NPC, event, end, creditB
};

struct GameObject
{
    ObjectType type;
    ObjectData data;
    glm::vec2 position;
    float maxSpeedX;
    float direction;
    std::vector<Animation> animations;
    int currentAnimation;
    SDL_Texture *texture;
    SDL_FRect collider;
    int spriteFrame;


    GameObject() : data{.level = LevelData()}
    {
        type = ObjectType::level;
        maxSpeedX = 0;
        direction = 1;
        position = glm::vec2(0);
        currentAnimation = -1;
        texture = nullptr;
        spriteFrame = 1;
    };

};