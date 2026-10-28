#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>
#include <format>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "gameobject.h"

                    #define SDL_BUTTON_LEFT     1
                    #define SDL_BUTTON_MIDDLE   2
                    #define SDL_BUTTON_RIGHT    3
                    #define SDL_BUTTON_X1       4
                    #define SDL_BUTTON_X2       5

                    #define SDL_BUTTON_MASK(X)  (1u << ((X)-1))
                    #define SDL_BUTTON_LMASK    SDL_BUTTON_MASK(SDL_BUTTON_LEFT)
                    #define SDL_BUTTON_MMASK    SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)
                    #define SDL_BUTTON_RMASK    SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)
                    #define SDL_BUTTON_X1MASK   SDL_BUTTON_MASK(SDL_BUTTON_X1)
                    #define SDL_BUTTON_X2MASK   SDL_BUTTON_MASK(SDL_BUTTON_X2)

using namespace std;

struct SDLState
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width, height, logW, logH;
    const bool *keys;
    bool fullscreen;

    SDLState() : keys(SDL_GetKeyboardState(nullptr))
    {
        fullscreen = false;
    }
};
//local variables
const size_t LAYER_IDX_LEVEL = 0;
const size_t LAYER_IDX_CHARACTER = 1;
const int MAP_ROWS = 20;
const int MAP_COLS = 40;
const int TILE_SIZE = 32;
bool touchingNPC = false;
bool movementKeys = true;
int dialogLine = 0;
int touchingNPCId = -1;
int activeNPCId = -1;
const float SKIP_DELAY = 5.0f;
bool title = true;
const float FLASH_DURATION = 0.15f;
bool gameFin = false;
bool response = false;
std::string credits [6]
{
    "Thanks For Playing!",
    "Huge ShoutOut to Constref he was a huge help with his youtube tutorial",
    "and helping me learn SDL3, follow his Youtube and GitHub",
    "gdfgdfgh",
    "gdjgkljdflgjf",
    "jfgkldjg",
};
bool CSppl;

struct GameState
{
    std::array<std::vector<GameObject>, 2 >layers;
    std::vector<GameObject> backgroundTiles;
    std::vector<Button> menuButtons;
    std::vector<ButtonGame> gameButtons;
    std::vector<Slider> slider;
    std::vector<Handle> handles;
    std::vector<std::string> credits;
    std::vector<GameObject> creditA;
    int PlayerIndex;
    bool debugMode;
    bool flashEnabled;
    int levelNum;
    bool dragging;
    float creditsElapsed;
    float creditsY = 340;
    float warningElapsed;
    float timeFlashed = 0.0f;
    bool flash = false;
    float nextFlash = 0.0f;
    bool gameInProgress = false;
    bool saved = false;
    string read;
    SDL_FRect mapViewport;
    gameMode mode;
    bool sprinting = false;
    bool switchLvl = false;
    float Lvltransition = 0.0f;
    bool lvlswitching = false;
    bool lvlMapSwitched = false;
    float introCS = 0.0f;
    int dialogLimiter = 0;
    std::array<bool, 3> patientsTalked{};
    bool choice1 = false;
    bool choice2 = false;
    bool choice3 = false;
    float choiceElapsed = 0.0f;
    float lvl3IntroWait = 0.0f;
    float dialogSkip2 = 0.0f;
    float dialogSkip3 = 0.0f;
    float lvl3Wait = 0.0f;

    GameState(const SDLState &state)
    {
        levelNum = 1;
        PlayerIndex = -1;
        debugMode = false; 
        dragging = false;
        mode = gameMode::warning;
        creditsElapsed = 0.0f;

        mapViewport = SDL_FRect
        {
            .x = 0, .y = 0, 
            .w = static_cast<float>(state.logW), .h = static_cast<float>(state.logH)
        };
        
        
    }
    

    GameObject &player() { return layers[LAYER_IDX_CHARACTER][PlayerIndex]; }
};

struct Resources
{
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    std::vector<Animation> playerAnims;
    std::vector<Animation> AgentAnims;
    const int ANIM_AGENT_WALK = 0;

    std::vector<SDL_Texture *> textures;
    SDL_Texture *textureIdle, *textureRun, *textureAgentIdle, *textureWood, *textureWoodBaseN, *textureWoodBaseS, *textureWoodBaseW, *textureWoodBaseE, *textureWoodFloor, *textureIdk, *textureEvent, *textureNPC, *textureButton, *textureBg2
    , *textureBg1, *textureSlider, *textureHandle, *textureMenu, *textureTransition, *textureStoneFloor, *textureWindowWatch, *textureCoolerWindow;
    
    //music stuffff
    std::vector<MIX_Audio *> audioClips;
    MIX_Audio *audioMusic;
    MIX_Track *trackMusic;

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

    SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &filepath)
    {
    //load texture
        SDL_Texture *texture = IMG_LoadTexture(renderer, filepath.c_str());
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        textures.push_back(texture);
        return texture;
    }

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
    
    void load(SDLState &state)
    {
        playerAnims.resize(5);
        playerAnims[ANIM_PLAYER_IDLE] = Animation(1, 1.50f);
        playerAnims[ANIM_PLAYER_RUN] = Animation(4, .80f);
        AgentAnims.resize(3);
        AgentAnims[ANIM_AGENT_WALK]= Animation(1, 1.0f);

        textureIdle = loadTexture(state.renderer, "assets/texture/idle.png");
        textureRun = loadTexture(state.renderer, "assets/texture/walking.png");
        textureAgentIdle = loadTexture(state.renderer, "assets/texture/NPC.png");
        textureWood = loadTexture(state.renderer, "assets/texture/wood.png");
        textureWoodBaseS = loadTexture(state.renderer, "assets/texture/base woodS.png");
        textureWoodBaseN = loadTexture(state.renderer, "assets/texture/base woodN.png");
        textureWoodBaseE = loadTexture(state.renderer, "assets/texture/base woodE.png");
        textureWoodBaseW = loadTexture(state.renderer, "assets/texture/base woodW.png");
        textureWoodFloor = loadTexture(state.renderer, "assets/texture/test.png");
        textureStoneFloor = loadTexture(state.renderer, "assets/texture/stoneFloor.png");
        textureIdk = loadTexture(state.renderer, "assets/texture/idk.png");
        textureEvent = loadTexture(state.renderer, "assets/texture/event.block.png");
        textureNPC = loadTexture(state.renderer, "assets/texture/NPC.png");
        textureButton = loadTexture(state.renderer, "assets/texture/button.png");
        textureBg1 = loadTexture(state.renderer, "assets/texture/bg1.png");
        textureBg2 = loadTexture(state.renderer, "assets/texture/bg2.png");
        textureSlider = loadTexture(state.renderer, "assets/texture/slider.png");
        textureHandle = loadTexture(state.renderer, "assets/texture/handle.png");
        textureMenu = loadTexture(state.renderer, "assets/texture/menu.png");
        textureTransition = loadTexture(state.renderer, "assets/texture/transition.png");
        textureWindowWatch = loadTexture(state.renderer, "assets/texture/window.png");
        textureCoolerWindow = loadTexture(state.renderer, "assets/texture/cooler window.png");

        //this is to link music to the slider 
        //MIX_SetTrackGain(res.trackMusic, slider.value);
        //this is for music later
    }
    void unload()
    {
        for (SDL_Texture *texture : textures){
            SDL_DestroyTexture(texture);
        }
        MIX_DestroyTrack(trackMusic);
        for (MIX_Audio *audio : audioClips){
            MIX_DestroyAudio(audio);
        }
    }
};

struct saveStuff
{
    std::string lvl;
    std::string pState;
    std::string pPosX;
    std::string pPosY;
};

struct holder
{
    //BUTTON things
    void buttonStuff(const Resources &res, GameState &gs)
    {
    Button startButton;
    startButton.rect = SDL_FRect
    {
        .x = 240, .y = 150,
        .w = 200, .h = 60,
    };
    startButton.texture = res.textureButton;
    startButton.ButtonId = 0;
    gs.menuButtons.push_back(startButton);

    Button menuButtonn;
    menuButtonn.rect = SDL_FRect
    {
        .x = 240, .y = 250,
        .w = 200, .h = 60
    };
    menuButtonn.texture = res.textureButton;
    menuButtonn.ButtonId = 1;
    gs.menuButtons.push_back(menuButtonn);

    //ingame option button
    Button optionButton;
    optionButton.rect = SDL_FRect
    {
        .x = 0, .y = 60,
        .w = 100, .h = 60
    };
    optionButton.texture = res.textureButton;
    optionButton.ButtonId = 10;
    gs.menuButtons.push_back(optionButton);

    //In-game Menu
    ButtonGame menuIG;
    menuIG.rectt = SDL_FRect
    {
        .x = 0, .y = 20,
        .w = 30, .h = 40,
    };
    menuIG.texture = res.textureMenu;
    menuIG.ButtonId = 20; //use accounting trick to number In-game and Out of game
    gs.gameButtons.push_back(menuIG);

    //go back to game
    ButtonGame menuBack;
    menuBack.rectt = SDL_FRect
    {
        .x = 210, .y = 100,
        .w = 200, .h = 40,
    };
    menuBack.texture = res.textureButton;
    menuBack.ButtonId = 21;
    gs.gameButtons.push_back(menuBack);

    ButtonGame BTM; //back to menu
    BTM.rectt = SDL_FRect
    {
        .x = 210, .y = 160,
        .w = 200, .h = 40,
    };
    BTM.texture = res.textureButton;
    BTM.ButtonId = 22;
    gs.gameButtons.push_back(BTM);

    ButtonGame saveBtn; // save the game
    saveBtn.rectt = SDL_FRect
    {
        .x = 210, .y = 220,
        .w = 200, .h = 40,
    };
    saveBtn.texture = res.textureButton;
    saveBtn.ButtonId = 23;
    gs.gameButtons.push_back(saveBtn);

    Slider slider;
    slider.texture = res.textureSlider;
    gs.slider.push_back(slider);

    Handle handle;
    handle.texture = res.textureHandle;
    handle.handler.x = slider.slide.x + slider.value * (slider.slide.w - handle.handler.w);
    gs.handles.push_back(handle);
 
    }
};

void cleanup(SDLState &state);
bool initialize(SDLState &state);
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime);
void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime);
void createTiles(const SDLState &state, GameState &gs, const Resources &res,
    bool cutsceneCharactersOnly = false);
void checkCollision(const SDLState &state, GameState &gs, Resources &res,
    GameObject &a, GameObject &b, float deltaTime, bool xAxis);
void collisionResponse (const SDLState &state, GameState &gs, Resources &res, 
    const SDL_FRect &rectA, const SDL_FRect &rectB, const SDL_FRect &rectC,
    GameObject &objA, GameObject &objB, float deltaTime, bool xAxis);
void clearMap(GameState &gs);
void dialog(const SDLState &state, GameState &gs, const Resources &res);
bool isButtonClicked(const Button &btn, float mouseX, float mouseY);
void Credits(const SDLState &state, GameState &gs, float scrollSpeed, float deltaTime, const Resources &res);
void InGameMenu(const SDLState &state, GameState &gs, const Resources &res, GameObject &obj);
void resetGame(const SDLState &state, GameState &gs, const Resources &res);
void warning(const SDLState &state, GameState &gs, float deltaTime);
void save(const SDLState &state, GameState &gs);
void readSave(const SDLState &staet, GameState &gs, saveStuff &stf);
bool checkForSave(const SDLState &state, GameState &gs);
void introCutscene(GameState &gs, float deltaTime);
bool introNPCsBesidePlayer(GameState &gs);
bool doneTalking(const GameState &gs);
void playerChoice(SDLState &state, GameState &gs,float deltaTime);
void charSpeech(SDLState &state, GameState &gs,float deltaTime);

int main (int argc, char *argv[])
{   
    SDLState state;
    state.width = 1600;
    state.height = 900;
    state.logW = 640;
    state.logH = 360;

    if(!initialize(state))
    {
        return 1;
    }

    // load textures
    Resources res;
    res.load(state);

    //setup game data
    GameState gs(state);

    GameObject obj;

    saveStuff stf;
    if(checkForSave(state, gs) == true)
    {
        readSave(state,gs, stf);
        try
        {
            gs.levelNum = std::stoi(stf.lvl);
        }
        catch (const std::exception &e)
        {
            std::cout << "stoi failed on [" << stf.lvl << "]: " << e.what() << '\n';
        }
    }

    createTiles(state, gs, res);
    uint64_t prevTime = SDL_GetTicks();

    holder hol;
    hol.buttonStuff(res, gs);
  
    SDL_srand(0);

    int og = gs.player().position.x - TILE_SIZE;
    if(checkForSave(state, gs) == true)
    {
        readSave(state,gs, stf);
        try
        {
            gs.player().position.x = std::stoi(stf.pPosX);       
        }
        catch (const std::exception &e)
        {
            std::cout << "stoi failed on [" << stf.pPosX << "]: " << e.what() << '\n';
        }
    
    }
    if(checkForSave(state,gs) == true)
    {
        readSave(state, gs, stf);
        try
        {
            gs.player().position.y = std::stoi(stf.pPosY);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    //start the game loop
    bool running = true;
    float wait = 0.0f;
    float dialogSkip = 0.0f;

    while(running)
    {
        uint64_t nowTime = SDL_GetTicks();
        float deltaTime = (nowTime - prevTime) / 1000.0f;
        SDL_Event event { 0 };
        while (SDL_PollEvent(&event))
        {


            SDL_ConvertEventToRenderCoordinates(state.renderer, &event);
            switch(event.type)
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
                }
                case SDL_EVENT_KEY_DOWN:
                {
                    if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing &&
                    gs.player().data.player.state == PlayerState::dialog)
                    {
                        dialogLine++;
                        if ((dialogLine == 1 && gs.dialogLimiter <= 3)
                            || (dialogLine == 2 && gs.dialogLimiter <= 6 && gs.dialogLimiter > 3))
                        {
                            if (activeNPCId >= 2 && activeNPCId <= 4)
                            {
                                gs.patientsTalked[activeNPCId - 2] = true;
                            }
                            dialogLine = 0;
                            activeNPCId = -1;
                            gs.player().data.player.state = PlayerState::idle;
                            movementKeys = true;
                        }
                    }
                    if(event.key.scancode == SDL_SCANCODE_S && gs.mode == gameMode::credits &&
                        gs.creditsElapsed >= SKIP_DELAY)
                    {
                        gs.creditsY = 340;
                        gs.mode = gameMode::title;
                        movementKeys = true;
                    }
                    if (gs.mode == gameMode::warning && gs.warningElapsed >= 3.0f)
                    {
                        gs.mode = gameMode::title;
                    }
                    if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing
                        && gs.levelNum == 3 && !response && gs.lvl3IntroWait >= 2.0f)
                    {
                        response = true;
                        gs.dialogSkip2 = 0.0f;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !gs.choice1 && !gs.choice2 && !gs.choice3)
                    {
                        gs.choice1 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !gs.choice1 && !gs.choice2 && !gs.choice3)
                    {
                        gs.choice2 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_3 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !gs.choice1 && !gs.choice2 && !gs.choice3)
                    {
                        gs.choice3 = true;
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
                case SDL_EVENT_MOUSE_MOTION:
                {
                    if(gs.mode == gameMode::optionMenu || gs.mode == gameMode::paused)
                    {
                        //volume slider for ingame and settings
                        if (gs.dragging)
                        {
                            Slider &s = gs.slider[0];
                            float value = ((event.motion.x+20) - (s.slide.x+20)) / s.slide.w;
                            s.value = SDL_clamp(value, 0.0f, 1.0f);

                            Handle &h = gs.handles[0];
                            h.handler.x = -25+s.slide.x + s.value * (s.slide.w - h.handler.w + 50);
                        }
                    }
                    break;
                }
                
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_FPoint click { event.button.x, event.button.y };

                    if(gs.mode == gameMode::optionMenu || gs.mode == gameMode::paused)
                    {
                        //slider things
                        for (Handle &h : gs.handles)
                        {
                            if (SDL_PointInRectFloat(&click, &h.handler))
                            {
                                gs.dragging = true;
                            }
                        }
                    }
                    //im the for loop that checks if it's pressed
                    if(gs.mode == gameMode::optionMenu || gs.mode == gameMode::title)
                    {
                    for(Button &btn :gs.menuButtons)
                    {
                        
                        if(SDL_PointInRectFloat(&click, &btn.rect))
                        {
                            //start game button
                            if(btn.ButtonId == 0)
                            {
                                if(gs.gameInProgress)
                                {
                                    gs.mode = gameMode::playing;
                                }
                                else
                                {
                                    if(gs.saved == false || gameFin)
                                    {
                                        resetGame(state, gs, res);
                                        gameFin = false;
                                    }
                                gs.gameInProgress = true;
                                gs.mode = gameMode::playing;
                                }
                            }
                            else if(btn.ButtonId == 1)
                            {
                                gs.mode = gameMode::optionMenu;

                            }
                            else if (btn.ButtonId == 10)
                            {
                                gs.mode = gameMode::title;
                            }

                        }
                    }
                    }
                    if(gs.mode == gameMode::playing || gs.mode == gameMode::paused)
                    {
                    for (ButtonGame &btn : gs.gameButtons)
                    {
                        if(SDL_PointInRectFloat(&click, &btn.rectt))
                        {
                            if(btn.ButtonId == 20 && gs.mode == gameMode::playing)
                            {
                                gs.mode = gameMode::paused;
                            }
                            else if(btn.ButtonId == 21 && gs.mode == gameMode::paused)
                            {
                                gs.mode = gameMode::playing;
                            }
                            else if (btn.ButtonId == 22 && gs.mode == gameMode::paused)
                            {   
                                gs.mode = gameMode::title;
                                movementKeys = true;
                                gs.dragging = false;
                                
                            }
                            else if (btn.ButtonId == 23 && gs.mode == gameMode::paused)
                            {
                                save(state, gs);
                            }
                        }
                    }
                    }
                }                
                break;
                }
                case SDL_EVENT_MOUSE_BUTTON_UP:
                {
                    gs.dragging = false;
                    break;
                }
            }

        }

        //event loop ended !                                                                                                            //

        //other things started                                                                                                          //
        
        //perform drawing commands
        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(state.renderer); 

        //playing game
        if(gs.mode == gameMode::playing)
        {                
            if (gs.player().data.player.state == PlayerState::choice)
            {
                movementKeys = false;
            }

            touchingNPC = false;
            touchingNPCId = -1;
            //update all objects
            for(auto &layer : gs.layers)
            {
                for (GameObject &obj : layer)
                {
                    update(state, gs, res, obj, deltaTime);
                    //update the animation
                    if(obj.currentAnimation != -1)
                    {
                        obj.animations[obj.currentAnimation].step(deltaTime);
                    }
                }
            }

            //change maps probably could be better idk 
            if(!gs.lvlswitching && gs.player().data.player.state == PlayerState::switchlvl)
            {
                gs.lvlswitching = true;
                gs.Lvltransition = 0.0f;
                gs.lvlMapSwitched = false;
                movementKeys = false;
            }
            if(gs.lvlswitching)
            {
                gs.Lvltransition += deltaTime;
                constexpr float fadeDuration = 0.25f;

                if(!gs.lvlMapSwitched && gs.Lvltransition >= fadeDuration)
                {
                    clearMap(gs);
                    createTiles(state, gs, res);
                    gs.lvlMapSwitched = true;
                }

                if(gs.Lvltransition >= fadeDuration * 2.0f)
                {
                    gs.lvlswitching = false;
                    gs.Lvltransition = 0.0f;
                    gs.lvlMapSwitched = false;
                    if(gs.player().data.player.state == PlayerState::switchlvl)
                    {
                        gs.player().data.player.state = PlayerState::idle;
                    }
                    movementKeys = true;
                }
            }

            if(gs.flash)
            {
                if(gs.timeFlashed > 0)
                {
                    gs.timeFlashed -= deltaTime;
                }
                else
                {
                    gs.nextFlash -= deltaTime;
                    if(gs.nextFlash <= 0)
                    {
                        gs.timeFlashed = FLASH_DURATION;

                        float base = SDL_max(60.0f - (gs.levelNum * 8.0f), 25.0f);
                        float variation = base * 0.3f;
                        gs.nextFlash = base - variation + (SDL_randf() * variation * 2.0f);
                    }
                }
            }

            if(gs.levelNum == 3)
            {
                if(doneTalking(gs))
                {
                    gs.lvl3Wait += deltaTime;
                    if(gs.lvl3Wait > 3.0f)
                    {
                        gs.player().data.player.state = PlayerState::choice;
                    }
                }
            }
            
            if(gs.levelNum == 2)
            {
                movementKeys = false;
                wait += deltaTime;
            }
            else
            {
                wait = 0.0f;
            }
            //calculate viewport position
            if(gs.levelNum > 1)
            {
            
            gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
            gs.mapViewport.y = (gs.player().position.y + TILE_SIZE / 2) - gs.mapViewport.h / 2;
            }
            else
            {
            gs.mapViewport.x = SDL_clamp(130, -280,200);
            gs.mapViewport.y = SDL_clamp(-320,-400,-300);
            }
            //draw background tiles
            for(GameObject &obj : gs.backgroundTiles)
            {
                SDL_FRect dst
                {
                    .x = obj.position.x - gs.mapViewport.x, 
                    .y = obj.position.y - gs.mapViewport.y,
                    .w = static_cast<float>(obj.texture->w),
                    .h = static_cast<float>(obj.texture->h)
                };
                SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
            }

            //draw all objects
            for(auto &layer : gs.layers)
            {
                for (GameObject &obj : layer)
                {
                    drawObject(state, gs, obj, deltaTime);
                }
            }
            for(ButtonGame &btn : gs.gameButtons)
            {
                if(btn.ButtonId <= 20)
                {
                    SDL_RenderTexture(state.renderer, btn.texture, nullptr, &btn.rectt);
                }
            }

            //flashesesees 
            if(gs.timeFlashed > 0)
            {
                SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(state.renderer, NULL);
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
                SDL_RenderDebugText(state.renderer, 350, 350, "STOPPPPPPPPPP");

            }
            if(gs.player().data.player.state == PlayerState::cutscene)
            {
                if(gs.levelNum == 1)
                {
                    gs.player().currentAnimation = res.ANIM_PLAYER_IDLE;
                    if (!CSppl)
                    {
                        CSppl = true;
                        createTiles(state, gs, res, true);
                    }
                    
                    if (introNPCsBesidePlayer(gs))
                    {
                        gs.player().data.player.state = PlayerState::switchlvl;
                        movementKeys = true;
                    }
                    else
                    {
                        introCutscene(gs, deltaTime);
                    }
                }
            }
        // dialog thingy its working for me so 
            if (gs.player().data.player.state == PlayerState::dialog)
            {
                movementKeys = false;
                dialog(state, gs, res);
            }

            if (gs.levelNum == 2 && wait > 2.0f)
            {
                dialogSkip += deltaTime;
                if(dialogSkip < 3.0f)
                {
                    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                    SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                    SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                    SDL_RenderRect(state.renderer, &levelTwoTextBox);
                    SDL_RenderDebugText(state.renderer, 155, 250, "???");
                    SDL_RenderDebugText(state.renderer, 155, 270, "What do you think?");
                }
                else if (dialogSkip > 3.0f && dialogSkip < 4.0f)
                {
                    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                    SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                    SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                    SDL_RenderRect(state.renderer, &levelTwoTextBox);
                    SDL_RenderDebugText(state.renderer, 155, 250, "???");
                    SDL_RenderDebugText(state.renderer, 155, 270, "He can work");
                }
                else
                {

                    gs.introCS += deltaTime;
                    if(gs.introCS < 1.0f)
                    {
                    gs.player().currentAnimation = res.ANIM_PLAYER_IDLE;
                    if (!CSppl && gs.levelNum == 1)
                    {
                        CSppl = true;
                        createTiles(state, gs, res, true);
                    }
                    
                    introCutscene(gs, deltaTime);
                    }
                    else if (gs.introCS > 1.0f)
                    {
                        gs.player().data.player.state = PlayerState::switchlvl;

                    }
                }
            }
            //Level-three dialogue
            if (gs.levelNum == 3
                && gs.player().data.player.state != PlayerState::choice
                && gs.player().data.player.state != PlayerState::switchlvl)
            {
                movementKeys = false;
                if (!response)
                {
                    gs.lvl3IntroWait += deltaTime;
                    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                    SDL_FRect textBox { 143, 240, 350, 100 };
                    SDL_RenderFillRect(state.renderer, &textBox);
                    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                    SDL_RenderRect(state.renderer, &textBox);
                    SDL_RenderDebugText(state.renderer, 155, 250, "???");
                    SDL_RenderDebugText(state.renderer, 155, 270, "Patient zero: Can you hear me?");
                    if (gs.lvl3IntroWait >= 2.0f)
                    {
                        SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
                    }
                }
                else
                {
                    gs.dialogSkip2 += deltaTime;
                    if (gs.dialogSkip2 < 4.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect textBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &textBox);
                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &textBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Good we need you to do something for us,");
                        SDL_RenderDebugText(state.renderer, 155, 290, "than you can return home");
                    }
                    else if (gs.dialogSkip2 < 7.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect textBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &textBox);
                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &textBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Do you see those people over there?");
                        SDL_RenderDebugText(state.renderer, 155, 290, "Find out the one with your same illness");
                        SDL_RenderDebugText(state.renderer, 155, 310, "by talking to them and goog luck.");
                    }
                    else
                    {
                        movementKeys = true;
                        if(gs.player().data.player.state == PlayerState::dialog)
                        {
                        movementKeys = false;
                        }
                    }
                }
            }
            else
            {
                gs.lvl3IntroWait = 0.0f;
                response = false;
            }

            if (gs.player().data.player.state == PlayerState::choice)
            {
                movementKeys = false;
                playerChoice(state, gs, deltaTime);
            }
            if(gs.levelNum == 4)
            {
                charSpeech(state, gs, deltaTime);
            }

            if (gs.player().data.player.state == PlayerState::ended)
            {
                gs.mode = gameMode::credits;
                gs.creditsY = 340; 
                gs.creditsElapsed = 0.0f;
            }
        }

        //menu screen stuff
        else if (gs.mode == gameMode::optionMenu)
        {

            Button btn;
            
            SDL_RenderTexture(state.renderer, res.textureBg2, nullptr, nullptr);
            SDL_RenderTexture(state.renderer, res.textureButton, nullptr, &btn.rect);

            for(Slider &s : gs.slider)
            {
                SDL_RenderTexture(state.renderer, res.textureSlider, nullptr, &s.slide);
            }
            for(Handle &h : gs.handles)
            {
                
                SDL_RenderTexture(state.renderer, res.textureHandle, nullptr, &h.handler);
            }
            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 150, 287, std::format("Volume").c_str());
            
            SDL_RenderDebugText(state.renderer, 470, 287, std::format(
               "{}%", static_cast<int>(gs.slider[0].value * 100)).c_str());
        }
        //title screen thing
        else if(gs.mode == gameMode::title)
        {

            //draw background for title screen
            SDL_RenderTexture(state.renderer, res.textureBg1, nullptr, nullptr);

            Button btn;

            //draw button
            for (Button &btn : gs.menuButtons)
                {
                    if(btn.ButtonId < 10)
                    {
                    SDL_RenderTexture(state.renderer, btn.texture, nullptr, &btn.rect);   
                    }
                }
            
        }
        //game paused
        else if (gs.mode == gameMode::paused)
        {

            //draw background tiles
            for(GameObject &obj : gs.backgroundTiles)
            {
                SDL_FRect dst
                {
                    .x = obj.position.x - gs.mapViewport.x, 
                    .y = obj.position.y - gs.mapViewport.y,
                    .w = static_cast<float>(obj.texture->w),
                    .h = static_cast<float>(obj.texture->h)
                };
                SDL_RenderTexture(state.renderer, obj.texture, nullptr, &dst);
            }

            //draw all objects
            for(auto &layer : gs.layers)
            {
                for (GameObject &obj : layer)
                {
                    drawObject(state, gs, obj, deltaTime);
                }
            }
            GameObject obj;
            InGameMenu(state, gs, res, obj);

                
                    for(Slider &s : gs.slider)
                    {
                        SDL_RenderTexture(state.renderer, res.textureSlider, nullptr, &s.slide);
                    }
                    for(Handle &h : gs.handles)
                    {
                        SDL_RenderTexture(state.renderer, res.textureHandle, nullptr, &h.handler);
                    }
                    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                SDL_RenderDebugText(state.renderer, 153, 287, std::format("Volume").c_str());
                
                SDL_RenderDebugText(state.renderer, 450, 287, std::format(
                "{}%", static_cast<int>(gs.slider[0].value * 100)).c_str());
                

            for(ButtonGame &btn : gs.gameButtons)
                {
                    if(btn.ButtonId <= 23)
                    {
                        SDL_RenderTexture(state.renderer, btn.texture, nullptr, &btn.rectt);
                    }
                }

        }
        //end game
        else if (gs.mode == gameMode::credits)
        {
            Credits(state, gs, 40.0f, deltaTime, res);
            gs.gameInProgress = false;
            gameFin = true;
        }
        else if (gs.mode == gameMode::warning)
        {
            warning(state, gs, deltaTime);
        }


        //debugging 
        if(gs.debugMode)
        {
            //debug info
            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 5, 5, std::format("State: {}, playerindex:{}, movementKeys:{}, level: {}",
            static_cast<int>( gs.player().data.player.state), gs.PlayerIndex, movementKeys, gs.levelNum).c_str());
        }

        if(gs.lvlswitching)
        {
            constexpr float fadeDuration = 0.25f;
            const float fadeProgress = gs.Lvltransition < fadeDuration
                ? gs.Lvltransition / fadeDuration
                : 1.0f - ((gs.Lvltransition - fadeDuration) / fadeDuration);
            const Uint8 alpha = static_cast<Uint8>(SDL_clamp(fadeProgress, 0.0f, 1.0f) * 255.0f);

            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, alpha);
            SDL_RenderFillRect(state.renderer, nullptr);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
        }

        // swap buffers and present
        SDL_RenderPresent(state.renderer);
        prevTime = nowTime;
    }
    res.unload();
    cleanup(state);
    return 0;
}

void cleanup(SDLState &state)
{
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    SDL_Quit();
}
bool initialize(SDLState &state)
{
    bool success = true;

    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "ERROR INTIALIZING SDL", nullptr);
        success = false;
    }
    //create the window
    state.window = SDL_CreateWindow("Indifference", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if(!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "ERROR CREATING WINDOW", nullptr);
        cleanup(state);
        success = false;
    }

    //create the renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if(!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "ERROR CREATING RENDERER", nullptr);
        cleanup(state);
        success = false;
    }
    SDL_SetRenderVSync(state.renderer, 1);

    //configure presenatation

    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_OVERSCAN);
    return success;
}
void drawObject(const SDLState &state, GameState &gs, GameObject &obj, float deltaTime)
{
        const float frameSize = 50; 
        const float spriteSize = 32;
        float srcX = obj.currentAnimation != -1
            ? obj.animations[obj.currentAnimation].currentFrame() * frameSize : 0.0f;
        //player drawing
        SDL_FRect src
        {
            .x = srcX,
            .y = 0,
            .w = frameSize,
            .h = frameSize,
        };
        //rest of the thing drawing
        SDL_FRect dst
        {
            .x = obj.position.x - gs.mapViewport.x,
            .y = obj.position.y - gs.mapViewport.y,
            .w = spriteSize,
            .h = spriteSize
        };

        SDL_FlipMode flipMode = obj.direction == 1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);

        //hitboxes
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

void update(const SDLState &state, GameState &gs, Resources &res, GameObject &obj, float deltaTime)
{
    float currentDirection = 0;
    if(obj.type == ObjectType::player)
    {
        float Ydirection = 0;

        //check all X collisions before checking all Y collisions
        if (state.keys[SDL_SCANCODE_LEFT] && movementKeys ==true)
        {

            if(gs.sprinting == true)
            {
                obj.position.x += -96 *deltaTime;
                currentDirection += -1;
            }
            else
            {
            currentDirection += -1;
            obj.position.x += -64 * deltaTime;
            }

        }
        if(state.keys[SDL_SCANCODE_RIGHT] && movementKeys ==true)
        {
            
            if(gs.sprinting == true)
            {
                obj.position.x += 96 *deltaTime;
                currentDirection += 1;
            }
            else
            {
            currentDirection += 1;
            obj.position.x += 64 * deltaTime;
            }

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

        if(currentDirection)
        {
            obj.direction = currentDirection;
        }

        switch(obj.data.player.state)
        {
            case PlayerState::idle:
            {
                if(currentDirection)
                {
                    obj.data.player.state = PlayerState::running;
                    obj.texture = res.textureRun;
                    obj.currentAnimation = res.ANIM_PLAYER_RUN;
                }
                break;
            }
            case PlayerState::running:
            {
                if(!currentDirection)
                {
                    obj.data.player.state = PlayerState::idle;
                    obj.texture = res.textureIdle;
                    obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                }
                break;
            }
            case PlayerState::dialog:
            {
                obj.texture = res.textureIdle;
                obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                break;
            }
        }

        // ---- Y axis: now move, then resolve Y collisions ----
        if (state.keys[SDL_SCANCODE_UP] && movementKeys == true)
        {

            if(gs.sprinting == true)
            {
                obj.position.y += -96 *deltaTime;
                Ydirection += 1;
            }
            else
            {
            obj.position.y += -64 *deltaTime;
            Ydirection += 1;
            }
        }
        if (state.keys[SDL_SCANCODE_DOWN] && movementKeys == true)
        {

            if(gs.sprinting == true)
            {
                obj.position.y += 96 *deltaTime;
                Ydirection += -1;

            }
            else
            {
                obj.position.y += 64 * deltaTime;
                Ydirection += -1;
            }
        }

        if(state.keys[SDL_SCANCODE_Z])
        {
            if(touchingNPC == true)
            {
                activeNPCId = touchingNPCId;
                dialogLine = 0;
                obj.data.player.state = PlayerState::dialog;
            }
        }


        if (Ydirection)
        {
            obj.direction = Ydirection;
        }
        switch(obj.data.player.state)
        {
            case PlayerState::idle:
            {
                if(Ydirection)
                {
                    obj.data.player.state = PlayerState::walking;
                    obj.texture = res.textureRun;
                    obj.currentAnimation = res.ANIM_PLAYER_RUN;

                }
                break;
            }
            case PlayerState::walking:
            {
                if(!Ydirection)
                {
                    obj.data.player.state = PlayerState::idle;
                    obj.texture = res.textureIdle;
                    obj.currentAnimation = res.ANIM_PLAYER_IDLE;
                }
                break;
            }
        }
        //sprinting mechanic
        if(state.keys[SDL_SCANCODE_LSHIFT] && gs.mode == gameMode::playing && movementKeys == true)
        {
            gs.sprinting = true;
        }
        else
        {
            gs.sprinting = false;
        }


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
                    }
                }
            }
        }
}

        else if(obj.type == ObjectType::NPC)
        {
            if(obj.data.NPC.state == NPCState::following)
            {
                const GameObject &player = gs.player();
                const glm::vec2 npcCenter = obj.position + glm::vec2(
                    obj.collider.x + obj.collider.w / 2.0f,
                    obj.collider.y + obj.collider.h / 2.0f);
                const glm::vec2 playerCenter = player.position + glm::vec2(
                    player.collider.x + player.collider.w / 2.0f,
                    player.collider.y + player.collider.h / 2.0f);
                const float side = obj.data.NPC.NPCId == 0 ? -1.0f : 1.0f;
                const float sideOffset =
                    player.collider.w / 2.0f + obj.collider.w / 2.0f + 6.0f;
                const glm::vec2 target = playerCenter + glm::vec2(side * sideOffset, 0.0f);
                const glm::vec2 targetDir = target - npcCenter;
                constexpr float followStopDistance = 4.0f;
                const float distance = glm::length(targetDir);

                if (distance > followStopDistance)
                {
                    obj.velocity = glm::normalize(targetDir) * obj.maxSpeedX;
                }
                else
                {
                    obj.velocity = glm::vec2(0);
                    obj.data.NPC.state = NPCState::standing;
                }
            }
            else
            {
                obj.velocity = glm::vec2(0);
            }

            obj.position.x += obj.velocity.x * deltaTime;
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

            obj.position.y += obj.velocity.y * deltaTime;
            for (auto &layer : gs.layers)
            {
                for (GameObject &objB : layer)
                {
                    if (&obj != &objB)
                    {
                        checkCollision(state, gs, res, obj, objB, deltaTime, false);
                    }
                }
            }
        }
        else
        {
            obj.velocity += currentDirection * obj.acceleration * deltaTime;
            if (std::abs(obj.velocity.x) > obj.maxSpeedX)
            {
                obj.velocity.x = (obj.velocity.x > 0 ? 1.0f : -1.0f) * obj.maxSpeedX;
            }
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
    }
    else
    {
        float centerA = rectA.y + rectA.h / 2.0f;
        float centerB = rectB.y + rectB.h / 2.0f;
        if (centerA < centerB)
        {
            objA.position.y -= rectC.h;
        }
        else                   
        { 
            objA.position.y += rectC.h; 
        }

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
                case ObjectType::NPC:
                {             
                    genericResponse();
                    touchingNPC = true;
                    touchingNPCId = objB.data.NPC.NPCId;
                    break;
                }
                case ObjectType::event:
                {
                    gs.player().data.player.state = PlayerState::cutscene;
                    break;
                }
                case ObjectType::switchlvl:
                {
                    gs.player().data.player.state = PlayerState::switchlvl;
                    break;
                }
                case ObjectType::end:
                {
                    gs.player().data.player.state = PlayerState::ended;
                    break;
                }
                }
            }
        
        else if (objA.type == ObjectType::NPC
            && (objB.type == ObjectType::level || objB.type == ObjectType::player))
        {
            genericResponse();
            if (xAxis)
            {
                objA.velocity.x = 0;
            }
            else
            {
                objA.velocity.y = 0;
            }
        }
    }
void checkCollision(const SDLState &state, GameState &gs, Resources &res,
    GameObject &a, GameObject &b, float deltaTime, bool xAxis)
{
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
        // NOTE: this only holds if the other axis's position hasn't moved
        // since it was last resolved
        const float EPSILON = 0.05f;
        if (xAxis  && rectC.h < EPSILON) { return; }
        if (!xAxis && rectC.w < EPSILON) { return; }
        // found interception, respond 
        collisionResponse(state, gs, res, rectA, rectB, rectC, a, b, deltaTime, xAxis);
    }
}
void createTiles(const SDLState &state, GameState &gs, const Resources &res,
    bool cutsceneCharactersOnly)
{
    
    short level1[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 22, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 4, 4, 4, 4, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    short level2[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 21, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 20, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    short level1ppl[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level2ppl[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level3ppl[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    short level3[MAP_ROWS][MAP_COLS]=//choosing
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
 
    short level1CS[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level4[MAP_ROWS][MAP_COLS]=//room
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 20, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level4ppl[MAP_ROWS][MAP_COLS]=
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level5[MAP_ROWS][MAP_COLS]=//choosing change this to ppl
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level6[MAP_ROWS][MAP_COLS]=//room 
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 21, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 20, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level7[MAP_ROWS][MAP_COLS]=//choosing change this to ppl
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level8[MAP_ROWS][MAP_COLS]=//room
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 21, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 20, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level9[MAP_ROWS][MAP_COLS]=//choosing change this to ppl 
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
        short level10[MAP_ROWS][MAP_COLS]=//cutscene
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 8, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 21, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 20, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 7, 7, 7, 7, 7, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    
    const auto loadMap = [&state, &gs, &res](short layer[MAP_ROWS][MAP_COLS])
    {
        const auto createObject = [&state] (int r, int c, SDL_Texture *texture, ObjectType type)
    {
        GameObject o;
        o.type = type;
        o.position = glm::vec2(c * TILE_SIZE, state.logH - (MAP_ROWS - r) * TILE_SIZE);
        o.texture = texture;
        o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE};
        return o;
    };

    for(int r = 0; r < MAP_ROWS; r++)
    {
        for(int c = 0; c < MAP_COLS; c++)
        {
            switch(layer[r][c])
            {
                case 1: //Wood wall
                {
                GameObject o = createObject(r, c, res.textureWood, ObjectType::level);
                gs.layers[LAYER_IDX_LEVEL].push_back(o);
                break;
                }
                case 2://wood base S
                {
                GameObject o = createObject(r, c, res.textureWoodBaseS, ObjectType::level);
                gs.layers[LAYER_IDX_LEVEL].push_back(o);
                break;
                }
                case 3:// wood floor
                {
                    GameObject o = createObject(r,c,res.textureWoodFloor, ObjectType::level);
                    gs.backgroundTiles.push_back(o);
                    break;
                }
                case 4: // wood base N
                {
                    GameObject o = createObject(r, c, res.textureWoodBaseN, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 5: // wood base E
                {
                    GameObject o = createObject(r, c, res.textureWoodBaseE, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 6: //wood base W
                {
                    GameObject o = createObject(r, c, res.textureWoodBaseW, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 7://stone floor
                {
                    GameObject o = createObject(r,c,res.textureStoneFloor, ObjectType::level);
                    gs.backgroundTiles.push_back(o);
                    break;
                }
                case 8: // window
                {
                    GameObject o = createObject(r, c, res.textureWindowWatch, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 9://cooler window
                {
                    GameObject o = createObject(r, c, res.textureCoolerWindow, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 10:
                {

                }
                case 11: //player
                {
                //create our player
                GameObject player = createObject(r, c, res.textureIdle, ObjectType::player);
                player.data.player = playerData();
                player.animations = res.playerAnims;
                player.currentAnimation = res.ANIM_PLAYER_IDLE;
                player.maxSpeedX = 100;
                player.collider = 
                {
                    .x = 11, .y = 4,
                    .w = 12, .h = 26,
                };
                gs.layers[LAYER_IDX_CHARACTER].push_back(player);
                gs.PlayerIndex = gs.layers[LAYER_IDX_CHARACTER].size() - 1;
                break;
                }
                case 12: //Agent A
                {
                    GameObject NPC = createObject(r, c, res.textureNPC, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 0;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 13: //Agent B
                {
                    GameObject NPC = createObject(r, c, res.textureNPC, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 1;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 14:// Subject 1
                {
                    GameObject NPC = createObject(r, c, res.textureNPC, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 2;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 15: // Subject 2
                {
                    GameObject NPC = createObject(r, c, res.textureNPC, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 3;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 16: //subject 3
                {
                    GameObject NPC = createObject(r, c, res.textureNPC, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 4;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 20: //switch level
                {
                    //switch lvl
                    GameObject o = createObject(r,c,res.textureEvent, ObjectType::switchlvl);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 21:
                {
                    //end game event block
                    GameObject o = createObject(r, c, res.textureEvent, ObjectType::end);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 22:
                {
                    //cutscene event block
                    GameObject o = createObject(r,c,res.textureWoodFloor, ObjectType::event);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }

                
            }
        }
    }

    };
    if (cutsceneCharactersOnly)
    {
        if (gs.levelNum == 1)
        {
            loadMap(level1CS);
        }
    }
    else
    {
        //switch the level if the level num changes
        switch(gs.levelNum)
        {
            default:
            {
                loadMap(level1ppl);
                loadMap(level1);
                break;
            }
            case 1:
            {
                loadMap(level1ppl);
                loadMap(level1);
                if(CSppl == true)
                {
                    loadMap(level1CS);
                }
                break;
            }
            case 2:
            {
                loadMap(level2ppl);
                loadMap(level2);
                break;
            }
            case 3:
            {
                loadMap(level3ppl);
                loadMap(level3);
                break;
            }
            case 4://room
            {
                loadMap(level4);
                loadMap(level4ppl);
                break;
            }
            case 5:
            {
                loadMap(level5);
                loadMap(level3ppl);
                break;
            }
            case 6:
            {
                loadMap(level6);
                loadMap(level4ppl);
                break;
            }
            case 7:
            {
                loadMap(level5);
                loadMap(level3ppl);
                break;
            }
            case 8:
            {
                loadMap(level6);
                loadMap(level4ppl);
                break;
            }
            case 9:
            {
                loadMap(level5);
                loadMap(level3ppl);
                break;
            }
            case 10://last
            {
                loadMap(level6);
                loadMap(level4ppl);
                break;
            }
        }
    }

    //debugging stufffff
    assert(gs.PlayerIndex != -1);
    if (gs.PlayerIndex != -1)
    {

    }
    if(gs.levelNum >= 6)
    {
        gs.flash = true;
    }
}

//clear the map for the next level
void clearMap(GameState &gs)
{
    for (auto &layer : gs.layers) { layer.clear(); }
    //reset to a starting position and change the level
    gs.backgroundTiles.clear();
    gs.PlayerIndex = -1;
    gs.mapViewport.x = 0;

    gs.levelNum ++;

    if (gs.levelNum > 6) { gs.levelNum = 1;} //set it to the first level instead of crashing the game
}

void dialog(const SDLState &state, GameState &gs, const Resources &res)
{
    movementKeys = false;
    int npcId = activeNPCId;

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_FRect dck
    {
        .x = 143, .y = 240,
        .w = 350, .h = 100,
    };

    //empty text box
    SDL_RenderFillRect(state.renderer, &dck);
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    SDL_RenderRect(state.renderer, &dck);

    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);

    //keep seperate from drawing box thingy 
    std::string characterName[5] = {
                                    "NPC",
                                    "NPC2",
                                    "Patient 1",
                                    "Patient 2",
                                    "Patient 3"
                                    };
    // limit of characters is 43
    std::string characterSpeech[][5] = {
    {
        "Nice job finding him, We dealt with him.",
        "He's dealt with, now move it kid."
    },
    {
        "Guy had it coming, belching in the street",
        "Atleast he isn't sick anymore"
    },
    {
        "I swear I don't have the virus please help",
  
    },
    {
        "I don't got nothing, now let me out!",
    },
    {
        "Listen kid, you don't know what you're",
   
    },
    };
    switch(npcId)
    {
        default:
        {
            break;
        }
        case 0://agent A
        {
            gs.dialogLimiter = 4;
            SDL_RenderDebugText(state.renderer, 150, 250, "Agent 1");
            if(dialogLine > 0)
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[0][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");
            }
            else
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[0][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 295, 325, "What do you mean? (W) >>");
            }
            break;
        }
        case 1: //agent B
        {
            gs.dialogLimiter = 4;
            SDL_RenderDebugText(state.renderer, 150, 250, "Agent 2");
            if(dialogLine > 0)
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[1][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");
            }
            else
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[1][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 375, 325, "Coming? (W) >>");
            }
            break;
        }
        case 2: // patient 1
        {
            gs.dialogLimiter = 2;
            SDL_RenderDebugText(state.renderer, 450, 250, "NPC2");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[2][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "me. You have to believe me!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 3: // patient 2
        {
            gs.dialogLimiter = 1;
            SDL_RenderDebugText(state.renderer, 450, 250, "NPC2");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[3][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 4: // patient 3
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 450, 250, "NPC2");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[4][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "doing, these people are crazy. There is");
            SDL_RenderDebugText(state.renderer, 150, 310, "no virus but choose me if you have to.");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
    }

}
bool isButtonClicked(const Button &btn, float mouseX, float mouseY)
{
    SDL_FPoint point { mouseX, mouseY };
    return SDL_PointInRectFloat(&point, &btn.rect);
}
void Credits(const SDLState &state, GameState &gs, float scrollSpeed, float deltaTime, const Resources &res)
{    

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(state.renderer, NULL);
    int lineGap = 40;
    int charW = 8;
    gs.creditsY -= scrollSpeed * deltaTime;
    gs.creditsElapsed += deltaTime;
    movementKeys = false;

    for(int s = 0; s < 6; s++)
    {
        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(state.renderer, (state.logW - credits[s].size() * charW) / 2, gs.creditsY + (s * lineGap), credits[s].c_str());
    }
    if (gs.creditsY + (5 * lineGap) + 50 < 0)
    {
        gs.creditsY = 340;
        movementKeys = true;
        gs.mode = gameMode::title;
    }
    
    if (gs.creditsElapsed >= SKIP_DELAY)
    {
        SDL_RenderDebugText(state.renderer, 10, state.logH - 20, "Press S to skip");
    }

}
void InGameMenu(const SDLState &state, GameState &gs, const Resources &res, GameObject &obj)
{
    obj.currentAnimation = -1;
    obj.spriteFrame = 6;
    
    const float charW = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 160);
    SDL_FRect full{
        140, 40, static_cast<float>((state.logW / 2) + 20),
                          static_cast<float>((state.logH / 1.8) + 80) };
    SDL_RenderFillRect(state.renderer, &full);
    SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

    const char *titlet = "Paused";
    const float titleScale = 3.0f;
    SDL_SetRenderScale(state.renderer, titleScale, titleScale);
    float tx = ((state.logW / titleScale) - SDL_strlen(titlet) * charW) / 2.0f;
    float ty = (state.logH / titleScale) / 2.0f - charW * 2.0f - 25;
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(state.renderer, tx, ty, titlet);
    SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);

}
void resetGame(const SDLState &state, GameState &gs, const Resources &res)
{
    for (auto &layer : gs.layers) { layer.clear(); }
    gs.backgroundTiles.clear();
    gs.PlayerIndex = -1;
    gs.levelNum = 1;
    gs.mapViewport.x = 0;
    gs.mapViewport.y = 0;
    gs.choice1 = false;
    gs.choice2 = false;
    gs.choice3 = false;
    gs.choiceElapsed = 0.0f;
    gs.patientsTalked.fill(false);
    createTiles(state, gs, res);
}
void warning(const SDLState &state, GameState &gs, float deltaTime)
{
    gs.warningElapsed += deltaTime;
    const float charW = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_FRect full { 0, 0, (float)state.logW, (float)state.logH };
    SDL_RenderFillRect(state.renderer, &full);

    const char *lines[] = {
        "PHOTOSENSITIVITY WARNING",
        "",
        "This game contains flashing lights",
        "and patterns that may trigger seizures",
        "in people with photosensitive epilepsy.",
        "",
        "Player discretion is advised."
    };

    SDL_SetRenderDrawColor(state.renderer, 230, 230, 230, 255);
    float y = 90.0f;
    for (const char *line : lines)
    {
        float x = (state.logW - SDL_strlen(line) * charW) / 2.0f;
        SDL_RenderDebugText(state.renderer, x, y, line);
        y += 16.0f;
    }

    if (gs.warningElapsed >= 3.0f && SDL_fmodf(gs.warningElapsed, 1.0f) < 0.6f)
    {
        const char *prompt = "Press any key to continue";
        float px = (state.logW - SDL_strlen(prompt) * charW) / 2.0f;
        SDL_RenderDebugText(state.renderer, px, state.logH - 40.0f, prompt);
    }
}
void save(const SDLState &state, GameState &gs)
{
    
    ofstream saveFile("saveData.txt");
    saveFile << gs.levelNum << '\n' << static_cast<float>(gs.player().position.x) << '\n' << static_cast<float>(gs.player().position.y) << '\n';
    saveFile.close();
    if(!saveFile)
    {
        std::cout<<"this shit aint work";
    }
    else 
    {
        
        char *prefPath = SDL_GetPrefPath("D.O.C Studios", "Indifference");
        std::string savePath = std::string(prefPath) + "save.txt";
        SDL_free(prefPath);

        std::ofstream out(savePath);
    }
}
void readSave(const SDLState &state, GameState &gs, saveStuff &stf)
{

    ifstream reading("saveData.txt");
    if (!reading)
    {
        std::cout << "couldn't open file\n";
        return; 
    }

    std::getline(reading, stf.lvl);
    std::getline(reading, stf.pPosX);
    std::getline(reading, stf.pPosY);
    reading.close();

}
bool checkForSave(const SDLState &state, GameState &gs)
{

    if(bool exists = std::filesystem::exists("saveData.txt"))
    {
        gs.saved = true;
        return true;
    }
    else 
    {
        return false;
    }
}
void introCutscene(GameState &gs, float deltaTime)
{     
    movementKeys = false;
    for (GameObject &character : gs.layers[LAYER_IDX_CHARACTER])
    {
        if (character.type == ObjectType::NPC
            && (character.data.NPC.NPCId == 0 || character.data.NPC.NPCId == 1))
        {
            character.data.NPC.state = NPCState::following;
        }
    }
}
bool introNPCsBesidePlayer(GameState &gs)
{
    const GameObject &player = gs.player();
    const glm::vec2 playerCenter = player.position + glm::vec2(
        player.collider.x + player.collider.w / 2.0f,
        player.collider.y + player.collider.h / 2.0f);
    std::array<bool, 2> foundNPCs{};

    for (const GameObject &character : gs.layers[LAYER_IDX_CHARACTER])
    {
        if (character.type != ObjectType::NPC
            || (character.data.NPC.NPCId != 0 && character.data.NPC.NPCId != 1))
        {
            continue;
        }

        const glm::vec2 npcCenter = character.position + glm::vec2(
            character.collider.x + character.collider.w / 2.0f,
            character.collider.y + character.collider.h / 2.0f);
        const float side = character.data.NPC.NPCId == 0 ? -1.0f : 1.0f;
        const float sideOffset =
            player.collider.w / 2.0f + character.collider.w / 2.0f + 6.0f;
        const glm::vec2 target = playerCenter + glm::vec2(side * sideOffset, 0.0f);
        constexpr float followStopDistance = 4.0f;

        if (glm::length(target - npcCenter) > followStopDistance)
        {
            return false;
        }
        foundNPCs[character.data.NPC.NPCId] = true;
    }

    return foundNPCs[0] && foundNPCs[1];
}
bool doneTalking(const GameState &gs)
{
    return gs.patientsTalked[0] && gs.patientsTalked[1] && gs.patientsTalked[2];
}
void playerChoice(SDLState &state, GameState &gs, float deltaTime)
{
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
    SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
    SDL_RenderFillRect(state.renderer, &levelTwoTextBox);
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    SDL_RenderRect(state.renderer, &levelTwoTextBox);
    SDL_RenderDebugText(state.renderer, 155, 250, "???");

    if (gs.choice1 || gs.choice2 || gs.choice3)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Good job, you chose right. We're going to");
        SDL_RenderDebugText(state.renderer, 155, 290, "need your talents");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            gs.choice1 = false;
            gs.choice2 = false;
            gs.choice3 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "So who do you think it is? Choose.");
        SDL_RenderDebugText(state.renderer, 150, 320, "(1)");
        SDL_RenderDebugText(state.renderer, 270, 320, "(2)");
        SDL_RenderDebugText(state.renderer, 390, 320, "(3)");
    }
}
void charSpeech(SDLState &state, GameState &gs, float deltaTime)
{
    switch(gs.levelNum)
    {
        case 1:
        {
            break;
        }
        case 2:
        {
            break;
        }
        case 3:
        {
            
            break;
        }

        case 4:
        {
            gs.dialogSkip3 += deltaTime;
            movementKeys = false;
            if(gs.dialogSkip3 < 5.0f)
            {
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                SDL_RenderRect(state.renderer, &levelTwoTextBox);
                SDL_RenderDebugText(state.renderer, 155, 250, "???");
                SDL_RenderDebugText(state.renderer, 155, 270, "Thanks for picking him out, we needed");
                SDL_RenderDebugText(state.renderer, 155, 290, "someone who can find who has the virus");
            }
            else if (gs.dialogSkip3 > 5.0f && gs.dialogSkip3 < 11.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Oh and we'll need you for a little more.");
                        SDL_RenderDebugText(state.renderer, 155, 290, "We need you to pick out some more people");
                        SDL_RenderDebugText(state.renderer, 155, 310, "with the disease we trust in your decision");
                    }
            else if (gs.dialogSkip3 > 11.0f && gs.dialogSkip3 < 15.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Just go through the door, right in ");
                        SDL_RenderDebugText(state.renderer, 155, 290, "between Agent A and Agent B. Don't be");
                        SDL_RenderDebugText(state.renderer, 155, 310, "scared, they're here for your safety.");
                    }
            else
                    {
                        movementKeys = true;
                    }
            break;
        }
        case 5://edit this one later
        {
            if(response == false)
                {
                    gs.lvl3IntroWait += deltaTime;
                    movementKeys = false;
                    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                    SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                    SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                    SDL_RenderRect(state.renderer, &levelTwoTextBox);
                    SDL_RenderDebugText(state.renderer, 155, 250, "???");
                    SDL_RenderDebugText(state.renderer, 155, 270, "Patient zero: Can you hear me?");
                    if(gs.lvl3IntroWait > 2.0f)
                    {
                        SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
                    }
                }
                else if (response == true)
                {
                    gs.dialogSkip2 += deltaTime;
                    movementKeys = false;
                    if(gs.dialogSkip2 < 3.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Alright just like last time,");
                        SDL_RenderDebugText(state.renderer, 155, 290, "choose the sick one.");
                    }
                    else if (gs.dialogSkip2 > 3.0f && gs.dialogSkip2 < 6.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Do you see those people over there?");
                        SDL_RenderDebugText(state.renderer, 155, 290, "Find out the one with your same illness");
                        SDL_RenderDebugText(state.renderer, 155, 310, "Talk to them if you need.");
                    }
                    else
                    {
                        movementKeys = true;
                    }
                }
                break;
            }
            case 6:
            {
                break;
            }
            case 7:
            {
                break;
            }
            case 8:
            {
                break;
            }
            case 9:
            {
                break;
            }
            case 10:
            {
                break;
            }
        }
    }