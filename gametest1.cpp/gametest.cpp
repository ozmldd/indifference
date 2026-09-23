#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <format>

#include "gameobject.h"

using namespace std;

struct SDLState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    MIX_Mixer *mixer;
    int width, height, logW, logH;
    const bool *keys;
    bool fullscreen;

    SDLState() : keys(SDL_GetKeyboardState(nullptr))
    {
        fullscreen = false;
    }
};


const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTERS = 1;
const int MAP_ROWS = 5;
const int MAP_COLS = 65;
const int TILE_SIZE = 32;
const float JUMP_FORCE = -200.0f;
const float LEVEL_END_X = 64 * TILE_SIZE;

struct GameState
{
    std::array<std::vector<GameObject>, 2 > layers;
    std::vector<GameObject> bullets;
    int playerIndex;
    SDL_FRect mapViewport;
    float bg2Scroll, bg3Scroll, bg4Scroll;
    bool debugMode;
    float deathTimer;
    bool playerWon;
    float runTime;

    GameState(const SDLState &state){
        playerIndex = -1; 
        mapViewport = SDL_FRect{
            .x = 0, .y = 0,
            .w = static_cast<float>(state.logW),
            .h = static_cast<float>(state.logH)
        };
        bg2Scroll = bg3Scroll = bg4Scroll = 0;
        debugMode = false;
        deathTimer = 0;
        playerWon = false;
        runTime = 0;
    }
    GameObject &player() { return layers[LAYER_IDX_CHARACTERS][playerIndex]; }
};

//holds all the resources needed for the game
struct Resources{
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    const int ANIM_PLAYER_DEAD = 2;
    std::vector <Animation> playerAnims;
    const int ANIM_BULLET_MOVING = 0;
    const int ANIM_BULLET_HIT = 1;
    std::vector <Animation> bulletAnims;
    const int ANIM_ENEMY = 0;
    const int ANIM_ENEMY_HIT = 1;
    const int ANIM_ENEMY_DEAD = 2;
    std::vector<Animation> enemyAnims;
    //declaring textures
    std::vector<SDL_Texture *> textures;
    SDL_Texture *textureIdle, *textureRun, *textureGrass, *textureGrassBlock, *textureDirtSurface,
     *textureDirtBlock, *textureBg1, *textureBg2, *textureBg3, *textureBg4, *textureBullet, *textureBulletHit,
     *textureEnemy, *textureEnemyHit, *textureEnemyDead, *textureDead;
    //declaring Audio
    std::vector<MIX_Audio *> audioClips;
    MIX_Audio *audioMusic, *audioShoot, *audioEnemyDead;
    MIX_Track *trackMusic, *trackShoot, *trackEnemyDead;

    static const std::string &assetPath(const std::string &relativePath)
    {
        static const std::string basePath = []() -> std::string {
            const char *path = SDL_GetBasePath();
            std::string result = path ? path : "./";
            return result;
        }();
        static std::string full;
        full = basePath + relativePath;
        return full;
    }
    //make the format to load the textures 
    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepath)
    {
        SDL_Texture* texture = IMG_LoadTexture(renderer, assetPath(filepath).c_str());
        if (!texture)
        {
            std::cout << "Failed to load texture " << filepath << ": " << SDL_GetError() << std::endl;
        }
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        textures.push_back(texture);
        return texture;
    }
    //same thing but with audio
    MIX_Audio *loadAudio(MIX_Mixer *mixer, const std::string &filepath)
    {
        MIX_Audio *audio = MIX_LoadAudio(mixer, assetPath(filepath).c_str(), false);
        if (!audio)
        {
            std::cout << "Failed to load audio " << filepath << ": " << SDL_GetError() << std::endl;
        }
        audioClips.push_back(audio);
        return audio;
    }

    //loading textures and audio
    void load(SDLState &state){
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(1, 1.6f);
        playerAnims[ANIM_PLAYER_RUN] = Animation(6, 0.5f);
        playerAnims[ANIM_PLAYER_DEAD] = Animation(6, 1.0f);
        bulletAnims.resize(2);
        bulletAnims[ANIM_BULLET_MOVING] = Animation(8, 0.4f);
        bulletAnims[ANIM_BULLET_HIT] = Animation(1, 0.8f);
        enemyAnims.resize(3);
        enemyAnims[ANIM_ENEMY]= Animation(6, 1.0f);
        enemyAnims[ANIM_ENEMY_HIT] = Animation(6, 0.45f);
        enemyAnims[ANIM_ENEMY_DEAD] = Animation(20, 1.5f);

        textureIdle = loadTexture(state.renderer, "assets/textures/deer_run.png");
        textureRun = loadTexture(state.renderer, "assets/textures/deer_run.png");
        textureGrass = loadTexture(state.renderer, "assets/textures/grass.png");
        textureGrassBlock = loadTexture(state.renderer, "assets/textures/grass_block.png");
        textureDirtSurface = loadTexture(state.renderer, "assets/textures/dirt_surface.png");
        textureDirtBlock = loadTexture(state.renderer, "assets/textures/dirt_block.png");
        textureBg1 = loadTexture(state.renderer, "assets/textures/bg_back_trees.png");
        textureBg2 = loadTexture(state.renderer, "assets/textures/bg_front_trees.png");
        textureBg3 = loadTexture(state.renderer, "assets/textures/bg_lights.png");
        textureBg4 = loadTexture(state.renderer, "assets/textures/bg_middle_trees.png");
        textureBullet = loadTexture(state.renderer, "assets/textures/bullet_move.png");
        textureBulletHit = loadTexture(state.renderer, "assets/textures/bullet_hit.png");
        textureEnemy = loadTexture(state.renderer, "assets/textures/enemy.png");
        textureEnemyHit = loadTexture(state.renderer, "assets/textures/enemy_hit.png");
        textureEnemyDead = loadTexture(state.renderer, "assets/textures/enemy_dead.png");
        textureDead = loadTexture(state.renderer, "assets/textures/deer_death.png");
        audioMusic = loadAudio(state.mixer, "assets/audio/forest_music.wav");
        audioShoot = loadAudio(state.mixer, "assets/audio/archer_shoot.flac");
        audioEnemyDead = loadAudio(state.mixer,"assets/audio/mutantdie.wav");


        trackMusic = MIX_CreateTrack(state.mixer);
        MIX_SetTrackAudio(trackMusic, audioMusic);
        MIX_SetTrackGain(trackMusic, 0.1f); // lower bg music volume (0.0 - 1.0)

        trackShoot = MIX_CreateTrack(state.mixer);
        MIX_SetTrackAudio(trackShoot, audioShoot);
        MIX_SetTrackGain(trackShoot, 0.1f);

        trackEnemyDead = MIX_CreateTrack(state.mixer);
        MIX_SetTrackAudio(trackEnemyDead, audioEnemyDead);
        
    }

    void unload()
    {
        for (SDL_Texture *texture : textures){
            SDL_DestroyTexture(texture);
        }
        MIX_DestroyTrack(trackMusic);
        MIX_DestroyTrack(trackShoot);
        MIX_DestroyTrack(trackEnemyDead);
        for (MIX_Audio *audio : audioClips){
            MIX_DestroyAudio(audio);
        }
    }

};
bool initialize(SDLState &state);
void cleanup(SDLState &state);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float width, float height, float deltaTime, float scale = 1.0f);
void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime);
void createTiles(const SDLState &state, GameState &gs, const Resources &res);
void checkCollision(const SDLState &state, GameState &gs, Resources &res, GameObject &a, GameObject &b, float deltaTime, bool xAxis);
void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture,
    float xVelocity, float &scrollPos, float scrollFactor, float deltaTime);
void drawDeathScreen(const SDLState &state, GameState &gs);
void resetGame(const SDLState &state, GameState &gs, const Resources &res);
void collisionResponse (const SDLState &state, GameState &gs, Resources &res, 
    const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC,
    GameObject &objA, GameObject &objB, float deltaTime, bool xAxis);
void winGame(const SDLState &state, GameState &gs, const Resources &res, GameObject &obj);
void winTimer(const SDLState &state, GameState &gs);

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDLState state;
    state.width = 1600;
    state.height = 900;
    state.logW = 480;
    state.logH = 320;

    if (!initialize(state)) {
        SDL_Log("Failed to initialize SDL");
        return 1;
    }
    //load game assets
    Resources res;
    res.load(state);

    //start background music, looping forever
    SDL_PropertiesID musicOptions = SDL_CreateProperties();
    SDL_SetNumberProperty(musicOptions, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    MIX_PlayTrack(res.trackMusic, musicOptions);
    SDL_DestroyProperties(musicOptions);

    //setup game data
    GameState gs(state);
    createTiles(state, gs, res);


    uint64_t prevTime = SDL_GetTicks();

    //start game loop
    bool running = true;
    while (running)
     {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f;

        SDL_Event event { 0 };
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
             case SDL_EVENT_QUIT:
             {
                running = false;
                break;
             }
             case SDL_EVENT_WINDOW_RESIZED:
            {
                state.width = event.window.data1;
                state.height = event.window.data2;
                break;
            } ;
                case SDL_EVENT_KEY_DOWN:
                {
                    if (event.key.scancode == SDL_SCANCODE_R &&
                    gs.player().data.player.state == PlayerState::dead || event.key.scancode == SDL_SCANCODE_R && gs.playerWon)
                    {
                    resetGame(state, gs, res);
                    }

                break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    if(event.key.scancode == SDL_SCANCODE_F){
                        gs.debugMode = !gs.debugMode;
                    }
                    else if (event.key.scancode == SDL_SCANCODE_F11)
                    {
                        state.fullscreen = !state.fullscreen;
                        SDL_SetWindowFullscreen(state.window, state.fullscreen);

                    }
                    break;
                }

            }
        }

        //update all objects 
        for (auto &layer : gs.layers)
        {
            for (GameObject &obj : layer)
            {
                update(state, gs, res, obj, deltaTime);

            }
        }
        //update bullets
        for(GameObject &bullet : gs.bullets)
        {
                update(state, gs, res, bullet, deltaTime);

        }
        if (!gs.playerWon && gs.player().data.player.state != PlayerState::dead)
        {
            gs.runTime += deltaTime;
        }

        //calculate viewport position
        gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;

        //don't let the players see the gap on the left
        const float levelWidth = MAP_COLS * TILE_SIZE;
        gs.mapViewport.x = SDL_clamp(gs.mapViewport.x, 0.0f, levelWidth - gs.mapViewport.w);

        //drawing commands
        SDL_SetRenderDrawColor(state.renderer, 10, 50, 10, 255);
        SDL_RenderClear(state.renderer);

        //draw background
        SDL_RenderTexture(state.renderer, res.textureBg1, nullptr, nullptr);
        drawParalaxBackground(state.renderer, res.textureBg4, gs.player().velocity.x, gs.bg4Scroll, 0.1f, deltaTime);
        drawParalaxBackground(state.renderer, res.textureBg3, gs.player().velocity.x, gs.bg3Scroll, 0.2f, deltaTime);
        drawParalaxBackground(state.renderer, res.textureBg2, gs.player().velocity.x, gs.bg2Scroll, 0.4f, deltaTime);

        //draw all objects
        for (auto &layer : gs.layers){
            for (GameObject &obj : layer){
                drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime ,1.0f);
            }
        }

        // draw bullets
        for (GameObject &bullet : gs.bullets)
        {
            if (bullet.data.bullet.state != BulletState::inactive)
            {
                drawObject(state, gs, bullet, 48.0f, 48.0f, deltaTime, 0.3f);
            }


        
        }
        //draw healthpoints 

        SDL_SetRenderDrawColor(state.renderer, 11, 218, 81, 255);
        SDL_RenderDebugText(state.renderer, 425, 5, std::format("HP: {}", static_cast<int> (gs.player().data.player.healthPoints)).c_str());
        //death screen
        if (gs.player().data.player.state == PlayerState::dead)
        {
        gs.deathTimer += deltaTime;
        if (gs.deathTimer > 1.0f)
            {
                drawDeathScreen(state, gs);
            }
        }
        if(gs.playerWon)
        {
            winGame(state, gs, res, gs.player());
            winTimer(state, gs);
        }

        if(gs.debugMode)
        {
            //display some debug info
            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 5, 5, std::format("State: {}, B: {}, G: {}, playerindex:{}",
            static_cast<int>( gs.player().data.player.state), gs.bullets.size(), gs.player().grounded, gs.playerIndex).c_str());

        }

        //swap buffers and present
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }
    res.unload();
    cleanup(state);
    return 0;
}

bool initialize(SDLState &state)
{
    bool initSuccess = true;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        initSuccess = false;
    }
    if (!MIX_Init()) {
        SDL_Log("Unable to initialize SDL_mixer: %s", SDL_GetError());
        initSuccess = false;
    }
    state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!state.mixer) {
        SDL_Log("Unable to create audio mixer: %s", SDL_GetError());
        initSuccess = false;
    }
    //create window

    state.window = SDL_CreateWindow("SDL3 Demo", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if (!state.window) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        cleanup(state);
        initSuccess = false;
    }
    //create the renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if (!state.renderer) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        cleanup(state);
        initSuccess = false;
    }
    SDL_SetRenderVSync(state.renderer, 1);

    //configure presentation
    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_STRETCH);
    return initSuccess;
}
void cleanup(SDLState &state) {
    MIX_DestroyMixer(state.mixer);
    MIX_Quit();
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float width, float height, float deltaTime, float scale)
{
        
        float srcX = obj.currentAnimation != -1
            ? obj.animations[obj.currentAnimation].currentFrame() * width : (obj.spriteFrame - 1) * width;
        //hitboxes for player
        SDL_FRect src;

        src.x = srcX;
        src.y = (obj.type == ObjectType::player) ? 96 : 0;
        src.w = width;
        src.h = height;

        SDL_FRect dst{
            .x = obj.position.x - gs.mapViewport.x,
            .y = obj.position.y,
            .w = width * scale,
            .h = height * scale
        };
        //flip to direction that is pressed
        SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        if(!obj.shouldFlash)
        {
            SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
        }
        else
        {
            //flash object with a redish tint
            SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
            SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
            SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

            if (obj.flashTimer.step(deltaTime))
            {
                obj.shouldFlash = false;
            }
        }

        //hitbox
        if (gs.debugMode)
        {
                SDL_FRect rectA
                {
                .x = obj.position.x + obj.collider.x - gs.mapViewport.x, 
                .y = obj.position.y + obj.collider.y,
                .w = obj.collider.w, 
                .h = obj.collider.h
            };
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
            SDL_RenderFillRect(state.renderer, &rectA);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
        }
}

void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime){
    // once the player is dead, freeze the rest of the world
    if (obj.type != ObjectType::player &&
        gs.player().data.player.state == PlayerState::dead)
    {
       return;
    }

    //update the animation
    if (obj.currentAnimation != -1)
    {
        obj.animations[obj.currentAnimation].step(deltaTime);
    }
    //death animation stuff
        if (obj.type == ObjectType::player && obj.data.player.state == PlayerState::dead)
    {
        if (obj.currentAnimation != -1 && obj.animations[obj.currentAnimation].isDone())
        {
            //stop animating and freeze on the last frame
            obj.currentAnimation = -1;
            obj.spriteFrame = 6; 
        }
        return;
    }

    if (obj.dynamic && !obj.grounded)
    {
        //apply gravity
        obj.velocity += glm::vec2(0,500) * deltaTime;

    }
    float currentDirection = 0;
    //handle key inputs
    if (obj.type == ObjectType::player)
    {
        if (state.keys[SDL_SCANCODE_A]){
            currentDirection += -1;
        }
        if (state.keys[SDL_SCANCODE_D]){
            currentDirection += 1;
        }
        if (state.keys[SDL_SCANCODE_SPACE] && obj.grounded)
        {
            obj.grounded = false;
            obj.velocity.y = JUMP_FORCE;
            obj.data.player.state = PlayerState::jumping;
        }

        timer &weaponTimer = obj.data.player.weaponTimer;
        weaponTimer.step(deltaTime);
        obj.data.player.invincibilityTimer.step(deltaTime);
        const auto handleShooting = [&state, &gs, &res, &obj, &weaponTimer]()
        {
            if(state.keys[SDL_SCANCODE_S])
            {
                if (weaponTimer.isTimeout())
                {
                    weaponTimer.reset();
                    MIX_PlayTrack(res.trackShoot, 0);

                    //spawn some bullets
                    GameObject bullet;
                    bullet.data.bullet = BulletData();
                    bullet.type = ObjectType::bullet;
                    bullet.direction = gs.player().direction;
                    bullet.texture = res.textureBullet;
                    bullet.currentAnimation = res.ANIM_BULLET_MOVING;
                    float hitboxScale = 0.3f;

                    //hit boxes for bullets
                    bullet.collider = SDL_FRect
                    {
                        .x = 1,
                        .y = 1,
                        .w = (48.0f * hitboxScale) - 3,
                        .h = (48.0f * hitboxScale) - 3
                    };
                    const int yVariation = 30;
                    const float yVelocity = SDL_rand(yVariation) - yVariation / 2.0f;
                    bullet.velocity = glm::vec2
                    (
                        600.0f * obj.direction,
                        0
                    );
                bullet.maxSpeedX = 1000.0f;
                bullet.animations = res.bulletAnims;

                //adjust bullet start position
                const float left = 0;
                const float right = 24;
                const float t = (obj.direction + 1) / 2.0f; //results in 0 or 2 
                const float xOffset = left + right * t;

                bullet.position = glm::vec2(
                    obj.position.x + xOffset,
                    obj.position.y + 5
                );
                //look for an active bullet slot
                bool foundInactive = false;
                for (int i = 0; i < gs.bullets.size() && !foundInactive; i++)
                {
                    if(gs.bullets[i].data.bullet.state == BulletState::inactive)
                    {
                        foundInactive = true;
                        gs.bullets[i] = bullet;
                    }
                }
                //if no inactive bullets are found
                if (!foundInactive)
                {
                gs.bullets.push_back(bullet);
                }

            }
        }
        };

        switch (obj.data.player.state)
        {
            case PlayerState::idle:
            {
                if (currentDirection)
                {
                    obj.data.player.state = PlayerState::running;

                }
                else
                {
                    //decelerate
                    if(obj.velocity.x){
                        const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
                        float amount = factor * obj.acceleration.x * deltaTime;
                        if(std::abs(obj.velocity.x) < std::abs(amount))
                        {
                            obj.velocity.x = 0;
                        }
                        else
                        {
                            obj.velocity.x += amount;
                        }
                    }
                }

                handleShooting();

                obj.texture = res.textureIdle;
                obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                break;
            }
            case PlayerState::running:
            {
                handleShooting();
                if (!currentDirection){
                    obj.data.player.state = PlayerState::idle;

                }
                obj.texture = res.textureRun;
                obj.currentAnimation = res.ANIM_PLAYER_RUN; 
                break;
            }
            case PlayerState::jumping:
            {
                handleShooting();
                obj.texture = res.textureRun;
                obj.currentAnimation = res.ANIM_PLAYER_RUN; 
                break;
            }
        }
    }
    else if (obj.type == ObjectType::bullet)
    {
        switch(obj.data.bullet.state)
        {
            case BulletState::moving:
            {
            if(obj.position.x - gs.mapViewport.x < 0 || // check the left side of the screen for bullets 
                obj.position.x - gs.mapViewport.x > state.logW || // check the right side of the screen for bullets
                obj.position.y - gs.mapViewport.y < 0 || // check up
                obj.position.y - gs.mapViewport.y > state.logH) // check down
            {
                obj.data.bullet.state = BulletState::inactive;
            }
            break;
        }
        case BulletState::colliding:
        {
            if(obj.animations[obj.currentAnimation].isDone())
            {
                obj.data.bullet.state = BulletState::inactive;
            }
            break;
        }
    }
}
        else if (obj.type == ObjectType::enemy)
        {
            EnemyData &d = obj.data.enemy;
            switch(d.state)
            {
                case EnemyState::roaming:
                {
                    glm::vec2 playerDir = gs.player().position - obj.position;
                    if (glm::length(playerDir) < 100)
                    {
                        currentDirection = playerDir.x < 0 ? -1 : 1;
                        obj.acceleration = glm::vec2(30, 0);
                    }
                    else
                    {
                        obj.acceleration = glm::vec2(0);
                        obj.velocity.x = 0;
                    }
                    break;
                }
                case EnemyState::damaged:
                {
                    if(obj.data.enemy.damagedTimer.step(deltaTime))
                    {
                        obj.data.enemy.state = EnemyState::roaming;
                        obj.texture = res.textureEnemy;
                        obj.currentAnimation = res.ANIM_ENEMY;

                    }
                    break;
                }
            case EnemyState::dead:
            {
                obj.velocity.x = 0;
                if (obj.currentAnimation != -1 && obj.animations[obj.currentAnimation].isDone())
                {
                    MIX_PlayTrack(res.trackEnemyDead, 0);
                    //remove animation and set to last frame
                    obj.currentAnimation = -1;
                    obj.spriteFrame = 18;
                }
                break;
            }
            }
        }
        if (currentDirection)
        {
            obj.direction = currentDirection;
        }
        //add accelaration to velocity
        obj.velocity += currentDirection * obj.acceleration * deltaTime;
        if (std::abs(obj.velocity.x) > obj.maxSpeedX)
        {
            obj.velocity.x = (obj.velocity.x > 0 ? 1.0f : -1.0f) * obj.maxSpeedX;
        }
        // X-axis: move, then resolve horizontal collisions
       obj.position.x += obj.velocity.x * deltaTime;

        if (obj.type == ObjectType::player)
        {
            const float maxX = MAP_COLS * TILE_SIZE - TILE_SIZE;
            if (obj.position.x < 0)         { obj.position.x = 0;    obj.velocity.x = 0; }
            else if (obj.position.x > maxX) { obj.position.x = maxX; obj.velocity.x = 0; }
        }

        for (auto &layer : gs.layers)
        {
            for (GameObject &objB : layer)
            {
                if (&obj != &objB)
                {
                    checkCollision(state, gs, res, obj, objB, deltaTime, true);
                }
            }
        }

        //Y axis: move, then resolve vertical collisions + ground sensor
        obj.position.y += obj.velocity.y * deltaTime;

        bool foundGround = false;
        for (auto &layer : gs.layers)
        {
            for (GameObject &objB : layer)
            {
                if (&obj != &objB)
                {
                    checkCollision(state, gs, res, obj, objB, deltaTime, false);

                    if (objB.type == ObjectType::level)
                    {
                        SDL_FRect sensor {
                            .x = obj.position.x + obj.collider.x,
                            .y = obj.position.y + obj.collider.y + obj.collider.h,
                            .w = obj.collider.w,
                            .h = 1
                        };
                        SDL_FRect rectB
                        {
                            .x = objB.position.x + objB.collider.x,
                            .y = objB.position.y + objB.collider.y,
                            .w = objB.collider.w,
                            .h = objB.collider.h
                        };
                        SDL_FRect rectC { 0 };
                        if (SDL_GetRectIntersectionFloat(&sensor, &rectB, &rectC))
                        {
                            foundGround = true;
                        }
                    }
                }
            }
        }
        foundGround = foundGround && obj.velocity.y >= 0;

        if(obj.grounded != foundGround)
        {
            //switching grounded state
            obj.grounded = foundGround;
            if(obj.type == ObjectType::player && obj.grounded){
                if(currentDirection != 0)
                {
                    obj.data.player.state = PlayerState::running;
                }
                else
                {
                    obj.data.player.state = PlayerState::idle;
                }
            }

        }
            // fell out of the world
        if (obj.type == ObjectType::player &&
            obj.data.player.state != PlayerState::dead &&
            obj.position.y > state.logH + TILE_SIZE)
        {
            obj.data.player.healthPoints = 0;
            obj.texture = res.textureDead;
            obj.currentAnimation = res.ANIM_PLAYER_DEAD;
            obj.data.player.state = PlayerState::dead;
            obj.velocity = glm::vec2(0, 0);
        }
        //if the player won
        if(obj.type == ObjectType::player && obj.data.player.state != PlayerState::dead && obj.position.x >= LEVEL_END_X)
        {
            gs.playerWon = true;
            obj.velocity = glm::vec2(0,0);
            obj.currentAnimation = -1;
            obj.spriteFrame = 6;
            winGame(state, gs, res, obj);
            
        }

}
void collisionResponse (const SDLState &state, GameState &gs, Resources &res, 
    const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC,
    GameObject &objA, GameObject &objB, float deltaTime, bool xAxis)
    {
        const auto genericResponse = [&]()
{
    if (xAxis)
    {
        float centerA = rectA.x + rectA.w / 2.0f;
        float centerB = rectB.x + rectB.w / 2.0f;
        if (centerA < centerB)
        {
            objA.position.x -= rectC.w;
        }
        else
        {
            objA.position.x += rectC.w; 
        }
        objA.velocity.x = 0;
    }
    else
    {
        float centerA = rectA.y + rectA.h / 2.0f;
        float centerB = rectB.y + rectB.h / 2.0f;
        if (centerA < centerB)
        {
            objA.position.y -= rectC.h;
        }
        else                   { objA.position.y += rectC.h; }
        objA.velocity.y = 0;
    }
};
        //object we are checking
        if(objA.type == ObjectType::player)
        {
            //object it is colliding with
            switch(objB.type)
            {
                case ObjectType::level:
                {
                    genericResponse();
                    break;
                }
                case ObjectType::enemy:
                {
                    if(objB.data.enemy.state != EnemyState::dead && objA.data.player.invincibilityTimer.isTimeout())
                    {
                        //bounce off enemies
                        objA.velocity = glm::vec2(100,0) * - objA.direction;
                        objA.data.player.healthPoints -= 20;
                        objA.data.player.invincibilityTimer.reset();

                        if (objA.data.player.healthPoints <= 0)
                        {
                            objA.spriteFrame = 18;
                            objA.texture = res.textureDead;
                            objA.currentAnimation = res.ANIM_PLAYER_DEAD;
                            objA.data.player.state = PlayerState::dead;
                            objA.velocity = glm::vec2(0,0);
                        }

                        
                    }
                    break;
                }
            }
        }
        else if (objA.type == ObjectType::bullet)
        {
            bool passthrough = false; 
            switch(objA.data.bullet.state)
            {
                case BulletState::moving:
                {
                    switch(objB.type)
                    {
                        case ObjectType::level:
                        {
                            break;
                        }
                        case ObjectType::enemy:
                        {
                            EnemyData &d = objB.data.enemy;
                            if (d.state != EnemyState::dead)
                            {
                            objB.direction = objA.direction;
                            objB.shouldFlash = true;
                            objB.flashTimer.reset();
                            objB.texture = res.textureEnemyHit;
                            objB.currentAnimation = res.ANIM_ENEMY_HIT;
                            d.state = EnemyState::damaged;

                            //damage the enemy and flag dead if needed
                            d.healthPoints -= 25;
                            if(d.healthPoints <=0)
                            {
                                d.state = EnemyState::dead;
                                objB.texture = res.textureEnemyDead;
                                objB.currentAnimation = res.ANIM_ENEMY_DEAD;
                            }
                        }
                        else 
                        {
                            //don't collide with dead enemys
                            passthrough = true;

                        }

                            break;
                        }

                    }
                    if(!passthrough)
                    {
                    genericResponse();
                    objA.velocity *= 0;
                    objA.data.bullet.state = BulletState::colliding;
                    objA.texture = res.textureBulletHit;
                    objA.currentAnimation = res.ANIM_BULLET_HIT;
                    }

                }
            }
        }
        else if (objA.type == ObjectType::enemy)
        {
            if (objB.type == ObjectType::level)
            {
                genericResponse();
            }

        }
    }
void checkCollision(const SDLState &state, GameState &gs, Resources &res,
    GameObject &a, GameObject &b, float deltaTime, bool xAxis)
{
    //hitboxes
    SDL_FRect rectA{
        .x = a.position.x + a.collider.x, 
        .y = a.position.y + a.collider.y,
        .w = a.collider.w, 
        .h = a.collider.h
    };
    SDL_FRect rectB{
        .x = b.position.x + b.collider.x, 
        .y = b.position.y + b.collider.y,
        .w = b.collider.w, 
        .h = b.collider.h
    };

    SDL_FRect rectC { 0 };

    if (SDL_GetRectIntersectionFloat(&rectA, &rectB, &rectC))
    {
         // A paper-thin overlap on the axis we're NOT resolving means we're
        // resting against a surface, not moving into it. Ignore it.
        const float EPSILON = 0.05f;
        if (xAxis  && rectC.h < EPSILON) { return; }
        if (!xAxis && rectC.w < EPSILON) { return; }
        // found interception, respond 
        collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime, xAxis);
    }
}
void createTiles(const SDLState &state, GameState &gs, const Resources &res)
{

    short map[MAP_ROWS][MAP_COLS] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 5, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 3, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 2, 1, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 5, 0, 0, 0, 6, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 5, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
        2, 2, 2, 1, 1, 1, 2, 0, 0, 3, 0, 0, 0, 0, 3, 0, 0, 0, 3, 3, 3, 5, 0, 0, 5, 0, 5, 0, 0, 3, 0, 0, 1, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 3, 3, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 6, 0, 0, 0, 0, 0, 0, 6, 0, 6, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1,
    };

    const auto createObject = [&state](int r, int c, SDL_Texture *texture, ObjectType type)
    {
        GameObject o;
        o.type = type;
        o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
        o.texture = texture;
        o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE};
        return o;

    };
    for (int r = 0; r < MAP_ROWS; r++)
    {
        for (int c = 0; c <MAP_COLS; c++)
        {
            switch (map[r][c])
            {
                case 1: //grass block
                {
                    GameObject o = createObject(r, c, res.textureGrassBlock, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 2: //grass
                {
                    GameObject o = createObject(r, c, res.textureGrass, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 3: // enemy
                {
                    GameObject o = createObject(r, c, res.textureEnemy, ObjectType::enemy);
                    o.data.enemy = EnemyData();
                    o.currentAnimation = res.ANIM_ENEMY;
                    o.animations = res.enemyAnims;
                    o.collider = SDL_FRect
                    {
                        .x = 2, .y = 4,
                        .w = 30, .h = 28
                    };
                    o.maxSpeedX = 15;
                    o.dynamic = true;
                    gs.layers[LAYER_IDX_CHARACTERS].push_back(o);
                    break;
                }
                case 4:
                {
                    //create our player
                    GameObject player = createObject(r, c, res.textureIdle, ObjectType::player);
                    player.data.player = PlayerData();
                    player.animations = res.playerAnims;
                    player.currentAnimation = res.ANIM_PLAYER_IDLE;
                    player.acceleration = glm::vec2(300, 0);
                    player.maxSpeedX = 100;
                    player.dynamic = true;
                    player.collider = {
                        .x = 5.5, .y = 4,
                        .w = 21.5, .h = 24
                    };
                    gs.layers[LAYER_IDX_CHARACTERS].push_back(player);
                    gs.playerIndex = gs.layers[LAYER_IDX_CHARACTERS].size() - 1;

                    break;
                }
                case 5: //dirtblock
                {
                    GameObject o = createObject(r, c, res.textureDirtBlock, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 6://dirt surface
                {
                    GameObject o = createObject(r, c, res.textureDirtSurface, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
            }
        }
    }
    assert(gs.playerIndex != -1);
}
void drawParalaxBackground(SDL_Renderer *renderer, SDL_Texture *texture,
    float xVelocity, float &scrollPos, float scrollFactor, float deltaTime)
{
    //draw moving background
    const float scale = 2.0f;
    float scaledW = texture->w * scale;
    float scaledH = texture->h *scale;

    scrollPos -= xVelocity *scrollFactor * deltaTime;
    scrollPos = SDL_fmodf(scrollPos,scaledW);

    if (scrollPos > 0)
    {
        scrollPos -= scaledW;
    }

    SDL_FRect dst
    {
        .x = scrollPos, 
        .y = 0,
        .w = scaledW * 3.0f,
        .h = scaledH
    };

    SDL_RenderTextureTiled(renderer, texture, nullptr, scale, &dst);
}
void drawDeathScreen(const SDLState &state, GameState &gs)
{
    const float charW = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE; // 8px

    //need for later in game opening
    // fade the level out over half a second, stopping at ~70% black
    float fade = SDL_min((gs.deathTimer - 1.0f) / 0.5f, 1.0f);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0,
        static_cast<Uint8>(180 * SDL_max(fade, 0.0f)));
    SDL_FRect full{ 0, 0, static_cast<float>(state.logW),
                          static_cast<float>(state.logH) };
    SDL_RenderFillRect(state.renderer, &full);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

    // title, drawn at 3x via render scale
    const char *title = "YOU DIED";
    const float titleScale = 3.0f;
    SDL_SetRenderScale(state.renderer, titleScale, titleScale);
    float tx = ((state.logW / titleScale) - SDL_strlen(title) * charW) / 2.0f;
    float ty = (state.logH / titleScale) / 2.0f - charW * 2.0f;
    SDL_SetRenderDrawColor(state.renderer, 190, 30, 30, 255);
    SDL_RenderDebugText(state.renderer, tx, ty, title);
    SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);

    //need for later too
    // blinking prompt
    if (SDL_fmodf(gs.deathTimer, 1.0f) < 0.6f)
    {
        const char *prompt = "PRESS R TO RESTART";
        float px = (state.logW - SDL_strlen(prompt) * charW) / 2.0f;
        SDL_SetRenderDrawColor(state.renderer, 230, 230, 230, 255);
        SDL_RenderDebugText(state.renderer, px, state.logH / 2.0f + 24.0f, prompt);
    }
}
void resetGame(const SDLState &state, GameState &gs, const Resources &res)
{
    for (auto &layer : gs.layers) { layer.clear(); }
    gs.bullets.clear();
    gs.playerIndex = -1;
    gs.mapViewport.x = 0;
    gs.bg2Scroll = gs.bg3Scroll = gs.bg4Scroll = 0;
    gs.deathTimer = 0;
    gs.playerWon = false;
    gs.runTime = 0;
    createTiles(state, gs, res);
}
void winGame(const SDLState &state, GameState &gs, const Resources &res, GameObject &obj)
{
            obj.velocity = glm::vec2(0,0);
            obj.currentAnimation = -1;
            obj.spriteFrame = 6;
const float charW = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 160);
    SDL_FRect full{ 0, 0, static_cast<float>(state.logW),
                          static_cast<float>(state.logH) };
    SDL_RenderFillRect(state.renderer, &full);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

    const char *title = "YOU WIN!";
    const float titleScale = 3.0f;
    SDL_SetRenderScale(state.renderer, titleScale, titleScale);
    float tx = ((state.logW / titleScale) - SDL_strlen(title) * charW) / 2.0f;
    float ty = (state.logH / titleScale) / 2.0f - charW * 2.0f;
    SDL_SetRenderDrawColor(state.renderer, 240, 210, 60, 255);
    SDL_RenderDebugText(state.renderer, tx, ty, title);
    SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);

    const char *prompt = "PRESS R TO PLAY AGAIN";
    float px = (state.logW - SDL_strlen(prompt) * charW) / 2.0f;
    SDL_SetRenderDrawColor(state.renderer, 230, 230, 230, 255);
    SDL_RenderDebugText(state.renderer, px, state.logH / 2.0f + 24.0f, prompt);
}
void winTimer(const SDLState &state, GameState &gs)
{
    const float charW = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    int hundredths = static_cast<int>(gs.runTime * 100.0f);
    std::string text = std::format("TIME  {}:{:02}.{:02}",
        hundredths / 6000, (hundredths / 100) % 60, hundredths % 100);

    float tx = (state.logW - text.size() * charW) / 2.0f;
    SDL_SetRenderDrawColor(state.renderer, 230, 230, 230, 255);
    SDL_RenderDebugText(state.renderer, tx, state.logH / 2.0f + 40.0f, text.c_str());
}