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
#include <windows.h>
#include <objbase.h>




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
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    MIX_Mixer *mixer = nullptr;
    int width, height, logW, logH;
    const bool *keys;
    bool fullscreen;
    bool mixerInitialized = false;

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
const float SKIP_DELAY = 8.0f;
bool title = true;
const float FLASH_DURATION = 0.15f;
constexpr float LEVEL_TRANSITION_FADE_DURATION = 0.25f;
bool gameFin = false;
bool response = false;
std::array<std::string, 15> credits
{
    "Thanks For Playing!",
    "Huge ShoutOut to Constref he was a huge help with his youtube tutorial",
    "and helping me learn SDL3, follow his Youtube and GitHub",
    "Shoutout Diego (itsmista_dlg) for the art on the MC and baseline for the NPCs",
    "Another Huge shoutout to ceeBee(ceebee_do_thingies) for the crazy art on the",
    "intro cutscene, came in clutch last second.",
    "Shoutout Bro Code for teaching me c++ great video and helped alot.",
    "I really hope you enjoyed this game took way too long, (91 hours to be exact)",
    "I did all the coding in c++ and SDL3 (with a little help with AI sorry)",
    "and I hope you enjoyed the game even though it's a bit short",
    "Coder/developer: Chris(ozmld)",
    "Game Director/designer: Diego",
    "Artist: CeeBee and Diego",
    "Tile and Map Designer: Chris",
    "Dialog: Chris",
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
    float warningElapsed = 0.0f;
    float timeFlashed = 0.0f;
    bool flash = false;
    float nextFlash = 0.0f;
    bool gameInProgress = false;
    bool saved = false;
    bool startNewGame = false;
    string read;
    std::string playerName;
    float sliderDragOffsetX = 0.0f;
    SDL_FRect mapViewport;
    float mapScale = 1.0f;
    gameMode mode;
    bool sprinting = false;
    bool switchLvl = false;
    float Lvltransition = 0.0f;
    bool lvlswitching = false;
    bool lvlMapSwitched = false;
    bool introCutsceneFlashing = false;
    float introCutsceneFlashElapsed = 0.0f;
    float introCS = 0.0f;
    float introCutsceneElapsed = 0.0f;
    bool restartRequested = false;
    int dialogLimiter = 0;
    std::array<bool, 12> patientsTalked{};
    float choiceElapsed = 0.0f;
    float lvl3IntroWait = 0.0f;
    float dialogSkip2 = 0.0f;
    float dialogSkip3 = 0.0f;
    float dialogSkip4 = 0.0f;
    float dialogSkip5 = 0.0f;
    float dialogSkip6 = 0.0f;
    float dialogSkip7 = 0.0f;
    float dialogSkip8 = 0.0f;
    float dialogSkip9 = 0.0f;
    float lvl3Wait = 0.0f;
    float lvl5Wait = 0.0f;
    float lvl7Wait = 0.0f;
    float lvl9Wait = 0.0f;
    float introCutscenetime = 0.0f;

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
    SDL_Texture *texturePlayerIdleNorth, *texturePlayerIdleSouth, *texturePlayerIdleEast, *texturePlayerIdleWest;
    SDL_Texture *texturePlayerRunNorth, *texturePlayerRunSouth;
    SDL_Texture *textureButtonBack, *textureButtonNext, *textureButtonMenu, *textureButtonOptions, *textureButtonSave, *textureButtonStart;
    SDL_Texture *textureRun, *textureAgentUp, *textureAgentIdle, *textureAgentIdleEast, *textureWood, *textureWoodBaseN, *textureWoodBaseS, *textureWoodBaseW, *textureWoodBaseE, *textureWoodFloor, *textureIdk, *textureEvent, *textureNPC, *textureButton, *textureBg2
    , *textureBg1, *textureSlider, *textureHandle, *textureMenu, *textureTransition, *textureStoneFloor, *textureWindowWatch, *textureCoolerWindow, *textureStoneWallA, *textureStoneWallC, *texturedoor
    ,*textureBed, *texturewallMirror, *textureIntroCutscene, *textureFlash, *textureNPC1, *textureNPC2, *textureNPC3, *textureNPC4,*textureNPC5,
    *textureNPC6, *textureNPC7, *textureNPC8, *textureNPC9, *textureNPC10, *textureNPC11, *textureNPC12, *texturedoorThing;
    
    //music stuffff
    std::vector<MIX_Audio *> audioClips;
    MIX_Audio *audioMenuMusic = nullptr;
    MIX_Track *trackMenuMusic = nullptr;
    bool menuMusicReady = false;

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
        AgentAnims[ANIM_AGENT_WALK]= Animation(4, 0.8f);

        texturePlayerIdleNorth = loadTexture(state.renderer, "assets/texture/idleN.png");
        texturePlayerIdleSouth = loadTexture(state.renderer, "assets/texture/idleS.png");
        texturePlayerIdleEast = loadTexture(state.renderer, "assets/texture/idleE.png");
        texturePlayerIdleWest = loadTexture(state.renderer, "assets/texture/idleW.png");
        texturePlayerRunNorth = loadTexture(state.renderer, "assets/texture/runningN.png");
        texturePlayerRunSouth = loadTexture(state.renderer, "assets/texture/runningS.png");
        textureButtonBack = loadTexture(state.renderer, "assets/texture/btn_back.png");
        textureButtonNext = loadTexture(state.renderer, "assets/texture/btn_next.png");
        textureButtonMenu = loadTexture(state.renderer, "assets/texture/btn_menu.png");
        textureButtonOptions = loadTexture(state.renderer, "assets/texture/btn_options.png");
        textureButtonSave = loadTexture(state.renderer, "assets/texture/btn_save.png");
        textureButtonStart = loadTexture(state.renderer, "assets/texture/btn_start.png");
        textureRun = loadTexture(state.renderer, "assets/texture/walking.png");
        textureAgentUp = loadTexture(state.renderer, "assets/texture/agent_up.png");
        textureAgentIdle = loadTexture(state.renderer, "assets/texture/agent_idle_up.png");
        textureAgentIdleEast = loadTexture(state.renderer, "assets/texture/agent_idle_east.png");
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
        textureStoneWallA = loadTexture(state.renderer, "assets/texture/stoneWallA.png");
        textureStoneWallC = loadTexture(state.renderer, "assets/texture/stoneWallC.png");
        texturedoor = loadTexture(state.renderer, "assets/texture/door.png");
        textureBed = loadTexture(state.renderer, "assets/texture/bed.png");
        texturewallMirror = loadTexture(state.renderer, "assets/texture/wallMirror.png");
        textureIntroCutscene = loadTexture(state.renderer, "assets/texture/introCutscene.jpg");
        textureFlash = loadTexture(state.renderer, "assets/texture/flash.png");
        textureNPC1 = loadTexture(state.renderer, "assets/texture/NPC1.png");
        textureNPC2 = loadTexture(state.renderer, "assets/texture/NPC2.png");
        textureNPC3 = loadTexture(state.renderer, "assets/texture/NPC3.png");
        textureNPC4 = loadTexture(state.renderer, "assets/texture/NPC4.png");
        textureNPC5 = loadTexture(state.renderer, "assets/texture/NPC5.png");
        textureNPC6 = loadTexture(state.renderer, "assets/texture/NPC6.png");
        textureNPC7 = loadTexture(state.renderer, "assets/texture/NPC7.png");
        textureNPC8 = loadTexture(state.renderer, "assets/texture/NPC8.png");
        textureNPC9 = loadTexture(state.renderer, "assets/texture/NPC9.png");
        textureNPC10 = loadTexture(state.renderer, "assets/texture/NPC10.png");
        textureNPC11 = loadTexture(state.renderer, "assets/texture/NPC11.png");        
        textureNPC12 = loadTexture(state.renderer, "assets/texture/NPC12.png");
        texturedoorThing = loadTexture(state.renderer, "assets/texture/door thing.png");
        if (state.mixer)
        {
            audioMenuMusic = loadAudio(state.mixer, "assets/audio/Menu.wav");
            trackMenuMusic = MIX_CreateTrack(state.mixer);
            if (!trackMenuMusic)
            {
                SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                    "Failed to create menu music track: %s", SDL_GetError());
            }
            else if (audioMenuMusic
                && MIX_SetTrackAudio(trackMenuMusic, audioMenuMusic))
            {
                menuMusicReady = true;
            }
            else
            {
                SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
                    "Failed to configure menu music: %s", SDL_GetError());
            }
        }
        else
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO,
                "Menu music is disabled because no audio device is available.");
        }

    }
    void unload()
    {
        for (SDL_Texture *texture : textures){
            SDL_DestroyTexture(texture);
        }
        MIX_DestroyTrack(trackMenuMusic);
        for (MIX_Audio *audio : audioClips){
            MIX_DestroyAudio(audio);
        }
    }
};

void updateVolumeSlider(GameState &gs, Resources &res, float mouseX)
{
    Slider &slider = gs.slider[0];
    Handle &handle = gs.handles[0];
    const float sliderStart = slider.slide.x - 25.0f;
    const float handleTravel = slider.slide.w - handle.handler.w + 50.0f;
    if (handleTravel <= 0.0f)
    {
        slider.value = 0.0f;
        handle.handler.x = sliderStart;
    }
    else
    {
        handle.handler.x = SDL_clamp(
            mouseX - gs.sliderDragOffsetX,
            sliderStart,
            sliderStart + handleTravel);
        slider.value = (handle.handler.x - sliderStart) / handleTravel;
    }

    if (res.menuMusicReady && !MIX_SetTrackGain(res.trackMenuMusic, slider.value))
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to set menu music volume: %s", SDL_GetError());
    }
}

void updateMenuMusic(const GameState &gs, const Resources &res)
{
    if (!res.menuMusicReady)
    {
        return;
    }

    const bool inMenu = gs.mode == gameMode::title || gs.mode == gameMode::optionMenu;
    if (inMenu && !MIX_TrackPlaying(res.trackMenuMusic))
    {
        if (!MIX_PlayTrack(res.trackMenuMusic, 0))
        {
            SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play menu music: %s", SDL_GetError());
        }
        else if (!MIX_SetTrackLoops(res.trackMenuMusic, -1))
        {
            SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to loop menu music: %s", SDL_GetError());
        }
    }
    else if (!inMenu && MIX_TrackPlaying(res.trackMenuMusic))
    {
        if (!MIX_StopTrack(res.trackMenuMusic, 0))
        {
            SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to stop menu music: %s", SDL_GetError());
        }
    }
}

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
        .x = 290, .y = 150,
        .w = 100, .h = 60,
    };
    startButton.texture = res.textureButtonStart;
    startButton.ButtonId = 0;
    gs.menuButtons.push_back(startButton);

    Button menuButtonn;
    menuButtonn.rect = SDL_FRect
    {
        .x = 290, .y = 250,
        .w = 100, .h = 60
    };
    menuButtonn.texture = res.textureButtonOptions;
    menuButtonn.ButtonId = 1;
    gs.menuButtons.push_back(menuButtonn);

    //ingame option button
    Button optionButton;
    optionButton.rect = SDL_FRect
    {
        .x = 0, .y = 60,
        .w = 100, .h = 60
    };
    optionButton.texture = res.textureButtonBack;
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
        .x = 260, .y = 100,
        .w = 100, .h = 60,
    };
    menuBack.texture = res.textureButtonBack;
    menuBack.ButtonId = 21;
    gs.gameButtons.push_back(menuBack);

    ButtonGame BTM; //back to menu
    BTM.rectt = SDL_FRect
    {
        .x = 260, .y = 160,
        .w = 100, .h = 60,
    };
    BTM.texture = res.textureButtonMenu;
    BTM.ButtonId = 22;
    gs.gameButtons.push_back(BTM);

    ButtonGame saveBtn; // save the game
    saveBtn.rectt = SDL_FRect
    {
        .x = 260, .y = 220,
        .w = 100, .h = 60,
    };
    saveBtn.texture = res.textureButtonSave;
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

struct choiceHolder
{
    bool choice1 = false;
    bool choice2 = false;
    bool choice3 = false;
    bool choice4 = false;
    bool choice5 = false;
    bool choice6 = false;
    bool choice7 = false;
    bool choice8 = false;
    bool choice9 = false;
    bool choice10 = false;
    bool choice11 = false;
    bool choice12 = false;
    bool choice13 = false;
    bool choice14 = false;
};

choiceHolder ch;

void appendPlayerName(std::string &name, const char *text)
{
    constexpr size_t maxCharacters = 24;
    size_t characterCount = 0;
    for (unsigned char character : name)
    {
        if ((character & 0xC0) != 0x80)
        {
            ++characterCount;
        }
    }

    const auto *input = reinterpret_cast<const unsigned char *>(text);
    while (*input && characterCount < maxCharacters)
    {
        size_t characterBytes = 1;
        if ((*input & 0xE0) == 0xC0) characterBytes = 2;
        else if ((*input & 0xF0) == 0xE0) characterBytes = 3;
        else if ((*input & 0xF8) == 0xF0) characterBytes = 4;

        for (size_t i = 1; i < characterBytes; ++i)
        {
            if (input[i] == '\0')
            {
                characterBytes = i;
                break;
            }
        }
        name.append(reinterpret_cast<const char *>(input), characterBytes);
        input += characterBytes;
        ++characterCount;
    }
}

void removeLastPlayerNameCharacter(std::string &name)
{
    if (name.empty())
    {
        return;
    }

    size_t lastCharacter = name.size() - 1;
    while (lastCharacter > 0
        && (static_cast<unsigned char>(name[lastCharacter]) & 0xC0) == 0x80)
    {
        --lastCharacter;
    }
    name.erase(lastCharacter);
}

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
                    if (gs.mode == gameMode::controls
                        && (event.key.scancode == SDL_SCANCODE_RETURN
                            || event.key.scancode == SDL_SCANCODE_KP_ENTER
                            || event.key.scancode == SDL_SCANCODE_SPACE))
                    {
                        gs.mode = gameMode::nameEntry;
                        SDL_StartTextInput(state.window);
                    }
                    else if (gs.mode == gameMode::nameEntry
                        && event.key.scancode == SDL_SCANCODE_BACKSPACE)
                    {
                        removeLastPlayerNameCharacter(gs.playerName);
                    }
                    else if (gs.mode == gameMode::nameEntry
                        && (event.key.scancode == SDL_SCANCODE_RETURN
                            || event.key.scancode == SDL_SCANCODE_KP_ENTER)
                        && !gs.playerName.empty())
                    {
                        SDL_StopTextInput(state.window);
                        if (gs.startNewGame)
                        {
                            resetGame(state, gs, res);
                            gs.startNewGame = false;
                        }
                        gs.gameInProgress = true;
                        gs.mode = gameMode::playing;
                    }

                    if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing &&
                    gs.player().data.player.state == PlayerState::dialog)
                    {
                        dialogLine++;
                        if ((dialogLine == 1 && gs.dialogLimiter <= 3)
                            || (dialogLine == 2 && gs.dialogLimiter <= 6 && gs.dialogLimiter > 3))
                        {
                            if (activeNPCId >= 2 && activeNPCId <= 13)
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
                    //********************/
                    //level 3
                    //********************/
                    if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing
                        && gs.levelNum == 3 && !response && gs.lvl3IntroWait >= 2.0f)
                    {
                        response = true;
                        gs.dialogSkip2 = 0.0f;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice1 && !ch.choice2 && !ch.choice3)
                    {
                        ch.choice1 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice1 && !ch.choice2 && !ch.choice3)
                    {
                        ch.choice2 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_3 && gs.mode == gameMode::playing
                    && gs.levelNum == 3 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice1 && !ch.choice2 && !ch.choice3)
                    {
                        ch.choice3 = true;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 4 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice13 && !ch.choice14)
                    {
                        ch.choice13 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 4 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice13 && !ch.choice14)
                    {
                        ch.choice14 = true;
                    }
                    //********************/
                    //level 5
                    //********************/
                        if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing
                        && gs.levelNum == 5 && !response)
                    {
                        response = true;
                        gs.dialogSkip2 = 0.0f;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 5 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice4 && !ch.choice5 && !ch.choice6)
                    {
                        ch.choice4 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 5 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice4 && !ch.choice5 && !ch.choice6)
                    {
                        ch.choice5 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_3 && gs.mode == gameMode::playing
                    && gs.levelNum == 5 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice4 && !ch.choice5 && !ch.choice6)
                    {
                        ch.choice6 = true;
                    }
                    //********************/
                    //level 7
                    //********************/
                        if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing
                        && gs.levelNum == 7 && !response)
                    {
                        response = true;
                        gs.dialogSkip2 = 0.0f;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 7 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice7 && !ch.choice8 && !ch.choice9)
                    {
                        ch.choice7 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 7 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice7 && !ch.choice8 && !ch.choice9)
                    {
                        ch.choice8 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_3 && gs.mode == gameMode::playing
                    && gs.levelNum == 7 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice7 && !ch.choice8 && !ch.choice9)
                    {
                        ch.choice9 = true;
                    }
                    //********************/
                    //level 9
                    //********************/
                    if (event.key.scancode == SDL_SCANCODE_W && gs.mode == gameMode::playing
                        && gs.levelNum == 9 && !response)
                    {
                        response = true;
                        gs.dialogSkip2 = 0.0f;
                    }
                    if(event.key.scancode == SDL_SCANCODE_1 && gs.mode == gameMode::playing
                    && gs.levelNum == 9 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice10 && !ch.choice11 && !ch.choice12)
                    {
                        ch.choice10 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_2 && gs.mode == gameMode::playing
                    && gs.levelNum == 9 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice10 && !ch.choice11 && !ch.choice12)
                    {
                        ch.choice11 = true;
                    }
                    else if(event.key.scancode == SDL_SCANCODE_3 && gs.mode == gameMode::playing
                    && gs.levelNum == 9 && gs.player().data.player.state == PlayerState::choice
                    && !ch.choice10 && !ch.choice11 && !ch.choice12)
                    {
                        ch.choice12 = true;
                    }
                    //end

                    break;
                }
                case SDL_EVENT_TEXT_INPUT:
                {
                    if (gs.mode == gameMode::nameEntry)
                    {
                        appendPlayerName(gs.playerName, event.text.text);
                    }
                    break;
                }
                case SDL_EVENT_KEY_UP:
                {
                    if(event.key.scancode == SDL_SCANCODE_F){
                        gs.debugMode = !gs.debugMode;
                    }
                    if (event.key.scancode == SDL_SCANCODE_F11)
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
                        if (gs.dragging)
                        {
                            updateVolumeSlider(gs, res, event.motion.x);
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
                        Slider &slider = gs.slider[0];
                        Handle &handle = gs.handles[0];
                        if (SDL_PointInRectFloat(&click, &handle.handler))
                        {
                            gs.dragging = true;
                            gs.sliderDragOffsetX = click.x - handle.handler.x;
                        }
                        else if (SDL_PointInRectFloat(&click, &slider.slide))
                        {
                            gs.dragging = true;
                            gs.sliderDragOffsetX = handle.handler.w / 2.0f;
                            updateVolumeSlider(gs, res, click.x);
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
                                    gs.startNewGame = !gs.saved || gameFin;
                                    gameFin = false;
                                    gs.playerName.clear();
                                    gs.mode = gameMode::controls;
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
                    else if (gs.mode == gameMode::controls)
                    {
                        const SDL_FRect continueButton { 270, 275, 100, 60 };
                        if (SDL_PointInRectFloat(&click, &continueButton))
                        {
                            gs.mode = gameMode::nameEntry;
                            SDL_StartTextInput(state.window);
                        }
                    }
                    else if (gs.mode == gameMode::nameEntry)
                    {
                        const SDL_FRect startButton { 270, 275, 100, 60 };
                        if (SDL_PointInRectFloat(&click, &startButton)
                            && !gs.playerName.empty())
                        {
                            SDL_StopTextInput(state.window);
                            if (gs.startNewGame)
                            {
                                resetGame(state, gs, res);
                                gs.startNewGame = false;
                            }
                            gs.gameInProgress = true;
                            gs.mode = gameMode::playing;
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
                    if (gs.mode == gameMode::playing && gs.levelNum == 4
                        && gs.player().data.player.state == PlayerState::choice
                        && !ch.choice13 && !ch.choice14)
                    {
                        const SDL_FRect yesChoice { 195, 308, 90, 32 };
                        const SDL_FRect noChoice { 315, 308, 90, 32 };

                        if (SDL_PointInRectFloat(&click, &yesChoice))
                        {
                            ch.choice13 = true;
                        }
                        else if (SDL_PointInRectFloat(&click, &noChoice))
                        {
                            ch.choice14 = true;
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

        updateMenuMusic(gs, res);

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

                if(!gs.lvlMapSwitched && gs.Lvltransition >= LEVEL_TRANSITION_FADE_DURATION)
                {
                    clearMap(gs);
                    createTiles(state, gs, res);
                    gs.lvlMapSwitched = true;
                }

                if(gs.Lvltransition >= LEVEL_TRANSITION_FADE_DURATION * 2.0f)
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

            if(gs.introCutsceneFlashing)
            {
                gs.introCutsceneFlashElapsed += deltaTime;
                if(gs.introCutsceneFlashElapsed >= LEVEL_TRANSITION_FADE_DURATION * 2.0f)
                {
                    gs.introCutsceneFlashing = false;
                    gs.introCutsceneFlashElapsed = 0.0f;
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

                        float base = SDL_max((60.0f - (gs.levelNum * 8.0f)) * 0.4f, 10.0f);
                        float variation = base * 0.8f;
                        gs.nextFlash = base - variation + (SDL_randf() * variation * 2.0f);
                    }
                }
            }

            if(gs.levelNum == 3 && !gs.lvlswitching)
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
            else if (gs.levelNum == 5 && !gs.lvlswitching)
            {
                if(doneTalking(gs))
                {
                    gs.lvl5Wait += deltaTime;
                    if(gs.lvl5Wait > 3.0f)
                    {
                        gs.player().data.player.state = PlayerState::choice;
                    }
                }
            }
            else if (gs.levelNum == 7 && !gs.lvlswitching)
            {
                if(doneTalking(gs))
                {
                    gs.lvl7Wait += deltaTime;
                    if(gs.lvl7Wait > 3.0f)
                    {
                        gs.player().data.player.state = PlayerState::choice;
                    }
                }
            }
            else if (gs.levelNum == 9 && !gs.lvlswitching)
            {
                if(doneTalking(gs))
                {
                    gs.lvl9Wait += deltaTime;
                    if(gs.lvl9Wait > 3.0f)
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
            const bool choiceLevel =
                gs.levelNum == 3 || gs.levelNum == 5 ||
                gs.levelNum == 7 || gs.levelNum == 9;
            gs.mapScale = 1.25f;
            const float roomLeft = (choiceLevel ? 9.0f : 10.0f) * TILE_SIZE;
            const float roomWidth = (choiceLevel ? 11.0f : 8.0f) * TILE_SIZE;
            const float roomTop = static_cast<float>(
                state.logH - (choiceLevel || gs.levelNum == 1 ? MAP_ROWS : MAP_ROWS - 1) * TILE_SIZE);
            const float roomHeight = (choiceLevel || gs.levelNum == 1 ? 9.0f : 7.0f) * TILE_SIZE;
            gs.mapViewport.x = roomLeft + roomWidth / 2.0f -
                state.logW / (2.0f * gs.mapScale);
            gs.mapViewport.y = roomTop + roomHeight / 2.0f -
                state.logH / (2.0f * gs.mapScale) +
                (gs.levelNum == 1 ? TILE_SIZE / 2.0f : 0.0f);
            SDL_SetRenderScale(state.renderer, gs.mapScale, gs.mapScale);
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
            SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);
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
                const SDL_FRect flashRect
                {
                    .x = 0,
                    .y = 0,
                    .w = static_cast<float>(state.logW),
                    .h = static_cast<float>(state.logH)
                };
                SDL_RenderTexture(state.renderer, res.textureFlash, nullptr, &flashRect);
            }

            if(gs.levelNum >= 7 && gs.levelNum < 9)
            {
                ch.choice3 = false;
                ch.choice4 = false;
                ch.choice5 = false;
                ch.choice6 = false;
            }
            else if(gs.levelNum >= 9)
            {
                ch.choice6 = false;
                ch.choice7 = false;
                ch.choice8 = false;
            }


            if(gs.player().data.player.state == PlayerState::introCutscene)
            {
                gs.introCutscenetime += deltaTime;
                movementKeys = false;
                if(gs.introCutscenetime >= 3.5f)
                {
                    gs.player().data.player.state = PlayerState::cutscene;
                }

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
                    
                    gs.introCutsceneElapsed += deltaTime;
                    introCutscene(gs, deltaTime);
                    if (gs.introCutsceneElapsed >= 3.0f)
                    {
                        gs.player().data.player.state = PlayerState::switchlvl;
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
                        SDL_RenderDebugText(state.renderer, 405, 325, "Yes(W) >>");
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
                        SDL_RenderDebugText(state.renderer, 155, 290, "Find out the one who's been infected,");
                        SDL_RenderDebugText(state.renderer, 155, 310, "they should have a red mark on their face.");
                    }
                    else if (gs.dialogSkip2 < 10.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect textBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &textBox);
                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &textBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Talk to them to find any other info");
                        SDL_RenderDebugText(state.renderer, 155, 290, "that might be helpful and good luck!");
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
            switch(gs.levelNum)
            {
                default:
                {break;}
                case 4:
                {
                    charSpeech(state, gs, deltaTime);
                    break;
                }
                case 5:
                {
                    charSpeech(state, gs, deltaTime);
                    break;
                }
                case 6:
                {
                    charSpeech(state,gs,deltaTime);
                    break;
                }
                case 7:
                {
                    charSpeech(state,gs,deltaTime);
                    break;
                }
                case 8:
                {
                    charSpeech(state,gs,deltaTime);
                    break;
                }
                case 9:
                {
                    charSpeech(state, gs, deltaTime);
                    break;
                }
                case 10://special case
                {
                    charSpeech(state,gs,deltaTime);
                    break;
                }
            }

            if (gs.restartRequested)
            {
                resetGame(state, gs, res);
                wait = 0.0f;
                dialogSkip = 0.0f;
            }

            if (gs.player().data.player.state == PlayerState::dialog)
            {
                movementKeys = false;
            }

            if (gs.player().data.player.state == PlayerState::ended)
            {
                gs.mode = gameMode::credits;
                gs.creditsY = 340; 
                gs.creditsElapsed = 0.0f;
            }

            if (gs.levelNum == 1
                && gs.player().data.player.state == PlayerState::introCutscene)
            {
                const float scale = SDL_min(
                    static_cast<float>(state.logW) / res.textureIntroCutscene->w,
                    static_cast<float>(state.logH) / res.textureIntroCutscene->h);
                const float width = res.textureIntroCutscene->w * scale;
                const float height = res.textureIntroCutscene->h * scale;
                const SDL_FRect introCutsceneRect
                {
                    .x = (state.logW - width) / 2.0f,
                    .y = (state.logH - height) / 2.0f,
                    .w = width,
                    .h = height
                };
                SDL_RenderTexture(state.renderer, res.textureIntroCutscene, nullptr, &introCutsceneRect);
            }
        }

        //menu screen stuff
        else if (gs.mode == gameMode::optionMenu)
        {

            Button btn;
            
            SDL_RenderTexture(state.renderer, res.textureBg2, nullptr, nullptr);
            SDL_RenderTexture(state.renderer, res.textureButtonBack, nullptr, &btn.rect);

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
        else if (gs.mode == gameMode::controls)
        {
            SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
            SDL_RenderClear(state.renderer);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state.renderer, 70, 70, 70, 235);
            const SDL_FRect panel { 110, 40, 420, 290 };
            SDL_RenderFillRect(state.renderer, &panel);
            SDL_SetRenderDrawColor(state.renderer, 155, 155, 155, 255);
            SDL_RenderRect(state.renderer, &panel);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 260, 90, "CONTROLS");
            SDL_RenderDebugText(state.renderer, 175, 145, "Arrow keys - Move");
            SDL_RenderDebugText(state.renderer, 175, 175, "Z - Interact with characters");
            SDL_RenderDebugText(state.renderer, 175, 205, "W - Continue dialogue");
            SDL_RenderDebugText(state.renderer, 175, 235, "Hold Shift - Sprint");

            const SDL_FRect continueButton { 270, 275, 100, 60 };
            SDL_RenderTexture(state.renderer, res.textureButtonNext, nullptr, &continueButton);
        }
        else if (gs.mode == gameMode::nameEntry)
        {
            SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
            SDL_RenderClear(state.renderer);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state.renderer, 70, 70, 70, 235);
            const SDL_FRect panel { 110, 40, 420, 290 };
            SDL_RenderFillRect(state.renderer, &panel);
            SDL_SetRenderDrawColor(state.renderer, 155, 155, 155, 255);
            SDL_RenderRect(state.renderer, &panel);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);

            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 235, 90, "WHAT IS YOUR NAME?");
            SDL_RenderDebugText(state.renderer, 175, 150, "Enter your name, then press Enter.");

            SDL_SetRenderDrawColor(state.renderer, 115, 115, 115, 255);
            const SDL_FRect nameInputBox { 165, 175, 310, 40 };
            SDL_RenderFillRect(state.renderer, &nameInputBox);
            SDL_SetRenderDrawColor(state.renderer, 190, 190, 190, 255);
            SDL_RenderRect(state.renderer, &nameInputBox);
            SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
            SDL_RenderDebugText(state.renderer, 175, 190, gs.playerName.c_str());

            const SDL_FRect startButton { 270, 275, 100, 60 };
            SDL_RenderTexture(state.renderer, res.textureButtonStart, nullptr, &startButton);
        }
        //game paused
        else if (gs.mode == gameMode::paused)
        {

            //draw background tiles
            SDL_SetRenderScale(state.renderer, gs.mapScale, gs.mapScale);
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
            SDL_SetRenderScale(state.renderer, 1.0f, 1.0f);
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
            const float fadeProgress = gs.Lvltransition < LEVEL_TRANSITION_FADE_DURATION
                ? gs.Lvltransition / LEVEL_TRANSITION_FADE_DURATION
                : 1.0f - ((gs.Lvltransition - LEVEL_TRANSITION_FADE_DURATION) / LEVEL_TRANSITION_FADE_DURATION);
            const Uint8 alpha = static_cast<Uint8>(SDL_clamp(fadeProgress, 0.0f, 1.0f) * 255.0f);

            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, alpha);
            SDL_RenderFillRect(state.renderer, nullptr);
            SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
        }
        else if(gs.introCutsceneFlashing)
        {
            const float fadeProgress = gs.introCutsceneFlashElapsed < LEVEL_TRANSITION_FADE_DURATION
                ? gs.introCutsceneFlashElapsed / LEVEL_TRANSITION_FADE_DURATION
                : 1.0f - ((gs.introCutsceneFlashElapsed - LEVEL_TRANSITION_FADE_DURATION) / LEVEL_TRANSITION_FADE_DURATION);
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
    if (state.mixer)
    {
        MIX_DestroyMixer(state.mixer);
        state.mixer = nullptr;
    }
    if (state.mixerInitialized)
    {
        MIX_Quit();
        state.mixerInitialized = false;
    }
    SDL_DestroyRenderer(state.renderer);
    state.renderer = nullptr;
    SDL_DestroyWindow(state.window);
    state.window = nullptr;
    SDL_Quit();
}
bool initialize(SDLState &state)
{
    if(!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", SDL_GetError(), nullptr);
        return false;
    }
    if (!MIX_Init())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", SDL_GetError(), nullptr);
        cleanup(state);
        return false;
    }
    state.mixerInitialized = true;
    state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (state.mixer)
    {
        SDL_Log("Audio output initialized with SDL driver '%s'.",
            SDL_GetCurrentAudioDriver());
    }

    if (!state.mixer)
    {
        const std::string firstError = SDL_GetError();
        SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO,
            "Default audio output failed: %s. Trying other available drivers.",
            firstError.c_str());

        const char *requestedDriver = SDL_GetHint(SDL_HINT_AUDIO_DRIVER);
        const int driverCount = SDL_GetNumAudioDrivers();
        for (int i = 0; i < driverCount && !state.mixer; ++i)
        {
            const char *driver = SDL_GetAudioDriver(i);
            if (!driver || SDL_strcmp(driver, "disk") == 0
                || SDL_strcmp(driver, "dummy") == 0
                || (requestedDriver && SDL_strcmp(driver, requestedDriver) == 0))
            {
                continue;
            }

            if (!SDL_SetHintWithPriority(
                    SDL_HINT_AUDIO_DRIVER, driver, SDL_HINT_OVERRIDE))
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO,
                    "Could not select SDL audio driver '%s'.", driver);
                continue;
            }

            SDL_QuitSubSystem(SDL_INIT_AUDIO);
            state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            if (state.mixer)
            {
                SDL_Log("Audio output initialized with fallback SDL driver '%s'.",
                    SDL_GetCurrentAudioDriver());
            }
        }
    }

    if (!state.mixer)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO,
            "No audio output is available. The game will continue without music.");
    }
    //create the window
    state.window = SDL_CreateWindow("Indifference", state.width, state.height, SDL_WINDOW_RESIZABLE);
    if(!state.window)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", SDL_GetError(), nullptr);
        cleanup(state);
        return false;
    }

    //create the renderer
    state.renderer = SDL_CreateRenderer(state.window, nullptr);
    if(!state.renderer)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", SDL_GetError(), nullptr);
        cleanup(state);
        return false;
    }
    SDL_SetRenderVSync(state.renderer, 1);

    //configure presenatation

    SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_OVERSCAN);
    return true;
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

        SDL_FlipMode flipMode = SDL_FLIP_NONE;
        if(obj.type == ObjectType::player)
        {
            const bool isMoving = obj.data.player.state == PlayerState::running
                || obj.data.player.state == PlayerState::walking;
            if(isMoving && obj.data.player.facing == PlayerFacing::east)
            {
                flipMode = SDL_FLIP_HORIZONTAL;
            }
        }
        else if(obj.direction == 1)
        {
            flipMode = SDL_FLIP_HORIZONTAL;
        }

        SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);

        //hitboxes
        if (gs.debugMode)
        {
                SDL_FRect rectA
                {
                .x = obj.position.x + obj.collider.x - gs.mapViewport.x, 
                .y = obj.position.y + obj.collider.y - gs.mapViewport.y,
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

        if(Ydirection > 0)
        {
            obj.data.player.facing = PlayerFacing::north;
        }
        else if(Ydirection < 0)
        {
            obj.data.player.facing = PlayerFacing::south;
        }
        else if(currentDirection > 0)
        {
            obj.data.player.facing = PlayerFacing::east;
        }
        else if(currentDirection < 0)
        {
            obj.data.player.facing = PlayerFacing::west;
        }

        const bool moving = currentDirection != 0 || Ydirection != 0;
        if(obj.data.player.state == PlayerState::idle
            || obj.data.player.state == PlayerState::running
            || obj.data.player.state == PlayerState::walking)
        {
            if(moving)
            {
                obj.data.player.state = Ydirection ? PlayerState::walking : PlayerState::running;
            }
            else
            {
                obj.data.player.state = PlayerState::idle;
            }
        }

        obj.currentAnimation = obj.data.player.state == PlayerState::running
            || obj.data.player.state == PlayerState::walking
            ? res.ANIM_PLAYER_RUN
            : res.ANIM_PLAYER_IDLE;

        if(obj.currentAnimation == res.ANIM_PLAYER_RUN)
        {
            switch(obj.data.player.facing)
            {
                case PlayerFacing::north: obj.texture = res.texturePlayerRunNorth; break;
                case PlayerFacing::south: obj.texture = res.texturePlayerRunSouth; break;
                case PlayerFacing::east:
                case PlayerFacing::west: obj.texture = res.textureRun; break;
            }
        }
        else
        {
            switch(obj.data.player.facing)
            {
                case PlayerFacing::north: obj.texture = res.texturePlayerIdleNorth; break;
                case PlayerFacing::south: obj.texture = res.texturePlayerIdleSouth; break;
                case PlayerFacing::east: obj.texture = res.texturePlayerIdleEast; break;
                case PlayerFacing::west: obj.texture = res.texturePlayerIdleWest; break;
            }
        }
        //sprinting mechanic
        if(state.keys[SDL_SCANCODE_LSHIFT] || state.keys[SDL_SCANCODE_RSHIFT] && gs.mode == gameMode::playing && movementKeys == true)
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

            if (obj.data.NPC.NPCId <= 1)
            {
                const bool isFollowing = obj.data.NPC.state == NPCState::following
                    && glm::length(obj.velocity) > 0.1f;
                obj.currentAnimation = isFollowing ? res.ANIM_AGENT_WALK : -1;
                if (isFollowing)
                {
                    obj.texture = res.textureAgentUp;
                }
                else
                {
                    obj.texture = gs.levelNum > 1 || obj.data.NPC.NPCId == 1
                        ? res.textureAgentIdleEast : res.textureAgentIdle;
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
                    if (gs.player().data.player.state != PlayerState::switchlvl
                        && gs.player().data.player.state != PlayerState::introCutscene
                        && gs.player().data.player.state != PlayerState::cutscene
                        && !gs.lvlswitching)
                    {
                        gs.introCutscenetime = 0.0f;
                        gs.introCutsceneElapsed = 0.0f;
                        gs.introCutsceneFlashing = true;
                        gs.introCutsceneFlashElapsed = 0.0f;
                        gs.player().data.player.state = PlayerState::introCutscene;
                    }
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 4, 4, 4, 4, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 35, 3, 3, 3, 3, 34, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 3, 3, 3, 3, 3, 3, 37, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
    {//make the tiles look good
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 8, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 8, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
     short level5[MAP_ROWS][MAP_COLS]=//choosing
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        7, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10,10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        short level5ppl[MAP_ROWS][MAP_COLS]=//choosing change this to ppl
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        short level7ppl[MAP_ROWS][MAP_COLS]=//room 
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
        short level9ppl[MAP_ROWS][MAP_COLS]=//choosing change this to ppl
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
    short level10[MAP_ROWS][MAP_COLS]=//room
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 8, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 7, 7, 7, 7, 7, 7, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
                case 10://stone wallA
                {
                    GameObject o = createObject(r, c, res.textureStoneWallA, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 11://stone wallC
                {
                    GameObject o = createObject(r, c, res.textureStoneWallC, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 12://door
                {
                    GameObject o = createObject(r, c, res.texturedoor, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 13: //player
                {
                    //create our player
                    GameObject player = createObject(r, c, res.texturePlayerIdleSouth, ObjectType::player);
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
                case 14: //Agent A
                {
                    SDL_Texture *idleTexture = gs.levelNum > 1
                        ? res.textureAgentIdleEast : res.textureAgentIdle;
                    GameObject NPC = createObject(r, c, idleTexture, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 0;
                    NPC.animations = res.AgentAnims;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 15: //Agent B
                {
                    GameObject NPC = createObject(r, c, res.textureAgentIdleEast, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 1;
                    NPC.animations = res.AgentAnims;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 16:// Subject 1
                {
                    GameObject NPC = createObject(r, c, res.textureNPC1, ObjectType::NPC);
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
                case 17: // Subject 2
                {
                    GameObject NPC = createObject(r, c, res.textureNPC2, ObjectType::NPC);
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
                case 18: //subject 3
                {
                    GameObject NPC = createObject(r, c, res.textureNPC3, ObjectType::NPC);
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
                

                //level 5


                case 19: // subject 4
                {
                    GameObject NPC = createObject(r, c, res.textureNPC4, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 5;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 20: //subject 5
                {
                    GameObject NPC = createObject(r, c, res.textureNPC5, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 6;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 21://subject 6
                {
                    GameObject NPC = createObject(r, c, res.textureNPC6, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 7;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }


                //level 7


                case 22://subject 7
                {
                    GameObject NPC = createObject(r, c, res.textureNPC7, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 8;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 23://subject 8
                {
                    GameObject NPC = createObject(r, c, res.textureNPC8, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 9;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 24://subject 9
                {
                    GameObject NPC = createObject(r, c, res.textureNPC9, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 10;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }


                //level 9


                case 25://subject 10
                {
                    GameObject NPC = createObject(r, c, res.textureNPC10, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 11;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 26://subject 11
                {
                    GameObject NPC = createObject(r, c, res.textureNPC11, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 12;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }
                case 27://subject 12
                {
                    GameObject NPC = createObject(r, c, res.textureNPC12, ObjectType::NPC);
                    NPC.data.NPC = NPCData();
                    NPC.data.NPC.NPCId = 13;
                    NPC.maxSpeedX = 50;
                    NPC.collider = 
                    {
                        .x = 11, .y = 6,
                        .w = 12, .h = 26,
                    };
                    gs.layers[LAYER_IDX_CHARACTER].push_back(NPC);
                    break;
                }


                //break;;;;;;;;;;;;;;;;;


                case 32: //switch level
                {
                    //switch lvl
                    GameObject o = createObject(r,c,res.texturedoor, ObjectType::switchlvl);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 33:
                {
                    //end game event block
                    GameObject o = createObject(r, c, res.texturedoor, ObjectType::end);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 34:
                {
                    //cutscene event block
                    GameObject o = createObject(r,c,res.textureWoodFloor, ObjectType::event);
                    o.collider = { .x = TILE_SIZE - 8, .y = 0, .w = 8, .h = TILE_SIZE };
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 35://sick bed
                {
                    GameObject o = createObject(r, c, res.textureBed, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 36://lvl 1 mirror
                {
                    GameObject o = createObject(r, c, res.texturewallMirror, ObjectType::level);
                    gs.layers[LAYER_IDX_LEVEL].push_back(o);
                    break;
                }
                case 37://intro door
                {
                    GameObject o = createObject(r, c, res.texturedoorThing, ObjectType::level);
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
            case 5://choice
            {
                loadMap(level3);
                loadMap(level5ppl);
                break;
            }
            case 6://room
            {
                loadMap(level4);
                loadMap(level4ppl);
                break;
            }
            case 7://choice
            {
                loadMap(level3);
                loadMap(level7ppl);
                break;
            }
            case 8://room
            {
                loadMap(level4);
                loadMap(level4ppl);
                break;
            }
            case 9://choice
            {
                loadMap(level3);
                loadMap(level9ppl);
                break;
            }
            case 10://last
            {
                loadMap(level10);
                loadMap(level1ppl);
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

    if (gs.levelNum > 10) { gs.levelNum = 1;} //set it to the first level instead of crashing the game
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
    std::string characterName[2] = {
                                    "NPC",
                                    "NPC2",
                                    };
    // limit of characters is 43
    std::string characterSpeech[][2] = {
    {
        //0
        //agent A lvl4
        "Nice job finding him, We dealt with him.",
        "He's dealt with, now move it kid."
    },
    {
        //1
        //agent B lvl4
        "Guy had it coming, belching in the street",
        "Atleast he isn't sick anymore"
    },
    {
        //2
        //agent A lvl6
        "This time was rough wasn't it? Guy you"

    },
    {
        //3
        //agent B lvl6
        "Never gets easier does it kid? My advice,"
    },
    {
        //4
        //agent A lvl8
        "Jackass mendax always got us doing the",
        "talk to B, I don't talk to the higher ups"
    },
    {
        //5
        //agent B lvl8
        "That guy Mendax is a psycho, I'm not even",
        "Mendax is the psycho on the intercom"
    },
    {
        //6
        "I swear I don't have the virus please help",
  
    },
    {
        //7
        "I don't got nothing, now let me out!",
    },
    {
        //8
        "Listen kid, you don't know what you're",
    },
    {
        //9 lvl5
        "I hate you lapdogs, just doing what you're"
    },
    {
        //10 lvl5
        "Virus? I ain't got no virus, I don't have"
    },
    {
        //11 lvl5 
        "Where the hell am I? Who are you? Let me"
    },
    {
        //12 lvl7
        "Let me see my kids, I miss them. I don't"
    },
    {
        //13 lvl7
        "I just got the cold man *cough* don't kill"
    },
    {
        //14 lvl 7
        "Aye man, *cough* I don't got anything"
    },
    {
        //15 lvl 9
        gs.playerName,
        "They kill them. Baby, there is no virus."
    },
    {
        //16 lvl9
        "Please I don't have anything don't kill me"
    },
    {
        //17 lvl9
        "Don't kill me or anyone, it isn't right!"
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
            if(gs.levelNum == 4)
            {
                gs.dialogLimiter = 4;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent A");
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
            }
            else if (gs.levelNum == 6)
            {
                gs.dialogLimiter = 3;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent A");

                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[2][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 150, 290, "picked tried to attack us, sad way to go.");       
                SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");
            }
            else if(gs.levelNum == 8)
            {
                gs.dialogLimiter = 4;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent A");
                if(dialogLine > 0)
                {
                    SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[4][dialogLine].c_str());
                    SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");
                }
                else
                {
                    SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[4][dialogLine].c_str());
                    SDL_RenderDebugText(state.renderer, 150, 290, "dirty work, ordered the whole batch gone");
                    SDL_RenderDebugText(state.renderer, 325, 325, "Who's Mendax? (W) >>");
                }
            }
            break;
        }
        case 1: //agent B
        {
            if(gs.levelNum == 4)
            {
                gs.dialogLimiter = 4;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent B");
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
            }

            else if (gs.levelNum == 6)
            {
                gs.dialogLimiter = 3;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent B");
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[3][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 150, 290, "don't think much on it, everyone dies.");
                SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");      
            }
            else if(gs.levelNum == 8)
            {
                gs.dialogLimiter = 4;
                SDL_RenderDebugText(state.renderer, 150, 250, "Agent B");
                if(dialogLine > 0)
                {
                    SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[5][dialogLine].c_str());
                    SDL_RenderDebugText(state.renderer, 150, 290, "He's the one bossing you around and");
                    SDL_RenderDebugText(state.renderer, 150, 310, "giving you info on the symptoms");
                    SDL_RenderDebugText(state.renderer, 410, 325, "Ok (W) >>");
                }
                else
                {
                    SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[5][dialogLine].c_str());
                    SDL_RenderDebugText(state.renderer, 150, 290, "sure he want's to beat the virus.");
                    SDL_RenderDebugText(state.renderer, 325, 325, "Who's Mendax? (W) >>");
                }
            }
            break;
        }
        case 2: // patient 1
        {
            gs.dialogLimiter = 2;
            SDL_RenderDebugText(state.renderer, 155, 250, "Job");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[6][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "me. You have to believe me!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 3: // patient 2
        {
            gs.dialogLimiter = 1;
            SDL_RenderDebugText(state.renderer, 155, 250, "Will");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[7][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 4: // patient 3
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 155, 250, "Veritas");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[8][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "doing, these people are crazy. There is");
            SDL_RenderDebugText(state.renderer, 150, 310, "no virus but choose me if you have to.");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 5: // patient 4 (on level 5)
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 155, 250, "Mark");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[9][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "told. I don't have anything, this mark");
            SDL_RenderDebugText(state.renderer, 150, 310, "just a birthmark, so let me go!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");

            break;
        }
        case 6://patient 5 (on level 5)
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 155, 250, "Cam");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[10][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "a rash or anything like those sickos.");
            SDL_RenderDebugText(state.renderer, 150, 310, "And what's up with your face?");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 7://patient 6 (on level 5)
        {
            gs.dialogLimiter = 2;
            SDL_RenderDebugText(state.renderer, 155, 250, "Iram");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[11][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "out yo, I wanna go back home you pig!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 8 ://patient 7 (on level 7)
        {
            gs.dialogLimiter = 2;
            SDL_RenderDebugText(state.renderer, 155, 250, "Mary");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[12][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "care what you do to me, let me see them!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 9://patient 8 (on level 7)
        {
            gs.dialogLimiter = 2;
            SDL_RenderDebugText(state.renderer, 155, 250, "Jake");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[13][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "me man, i can't control it!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 10://patient 9 (on level 7)
        {
            SDL_RenderDebugText(state.renderer, 155, 250, "Lilian");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[14][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "you know I can't do anything about it!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 11://patient 10 (on level 9)
        {
            gs.dialogLimiter = 4;
            SDL_RenderDebugText(state.renderer, 155, 250, "MA");
            if(dialogLine > 0)
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[15][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 150, 290, "you judge these people off of nothing!");
                SDL_RenderDebugText(state.renderer, 405, 325, "Next (W) >>");
            }
            else
            {
                SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[15][dialogLine].c_str());
                SDL_RenderDebugText(state.renderer, 150, 290, "Do you know what happens to the people");
                SDL_RenderDebugText(state.renderer, 150, 310, "you choose?");
                SDL_RenderDebugText(state.renderer, 405, 325, "Next (W) >>");
            }
            break;
        }
        case 12://patient 11(on level 9)
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 155, 250, "Hope");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[16][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "I just want to live!");
            SDL_RenderDebugText(state.renderer, 405, 325, "Next(W) >>");
            break;
        }
        case 13://patient 12 (on level 9)
        {
            gs.dialogLimiter = 3;
            SDL_RenderDebugText(state.renderer, 155, 250, "Simone");
            SDL_RenderDebugText(state.renderer, 150, 270, characterSpeech[17][dialogLine].c_str());
            SDL_RenderDebugText(state.renderer, 150, 290, "Who are you to judge if I live!");
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

    for (size_t s = 0; s < credits.size(); s++)
    {
        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
        SDL_RenderDebugText(state.renderer, (state.logW - credits[s].size() * charW) / 2, gs.creditsY + (s * lineGap), credits[s].c_str());
    }
    if (gs.creditsY + ((credits.size() - 1) * lineGap) + 50 < 0)
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
    gs.lvlswitching = false;
    gs.Lvltransition = 0.0f;
    gs.lvlMapSwitched = false;
    gs.introCutsceneFlashing = false;
    gs.introCutsceneFlashElapsed = 0.0f;
    gs.switchLvl = false;
    gs.introCS = 0.0f;
    gs.introCutsceneElapsed = 0.0f;
    gs.restartRequested = false;
    gs.dialogLimiter = 0;
    gs.choiceElapsed = 0.0f;
    gs.lvl3IntroWait = 0.0f;
    gs.dialogSkip2 = 0.0f;
    gs.dialogSkip3 = 0.0f;
    gs.dialogSkip4 = 0.0f;
    gs.dialogSkip5 = 0.0f;
    gs.dialogSkip6 = 0.0f;
    gs.dialogSkip7 = 0.0f;
    gs.dialogSkip8 = 0.0f;
    gs.dialogSkip9 = 0.0f;
    gs.lvl3Wait = 0.0f;
    gs.lvl5Wait = 0.0f;
    gs.lvl7Wait = 0.0f;
    gs.lvl9Wait = 0.0f;
    gs.patientsTalked.fill(false);
    gs.flash = false;
    gs.timeFlashed = 0.0f;
    gs.nextFlash = 0.0f;
    ch = choiceHolder{};
    CSppl = false;
    response = false;
    dialogLine = 0;
    touchingNPC = false;
    touchingNPCId = -1;
    activeNPCId = -1;
    movementKeys = true;
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
bool doneTalking(const GameState &gs)
{
    if(gs.levelNum == 5)
    {
        return gs.patientsTalked[3] && gs.patientsTalked[4] && gs.patientsTalked [5];
    }
    else if(gs.levelNum == 3)
    {
        return gs.patientsTalked[0] && gs.patientsTalked[1] && gs.patientsTalked[2];
    }
    else if(gs.levelNum == 7)
    {
        return gs.patientsTalked[6] && gs.patientsTalked[7] && gs.patientsTalked[8];
    }
    else if(gs.levelNum == 9)
    {
        return gs.patientsTalked[9] && gs.patientsTalked[10] && gs.patientsTalked[11];
    }
    else
    {
        return false;
    }
}
void playerChoice(SDLState &state, GameState &gs, float deltaTime)
{
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
    SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
    SDL_RenderFillRect(state.renderer, &levelTwoTextBox);
    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    SDL_RenderRect(state.renderer, &levelTwoTextBox);
    SDL_RenderDebugText(state.renderer, 155, 250, "???");

    if (gs.levelNum == 4 && ch.choice13)
    {
        ch.choice13 = false;
        gs.dialogSkip3 = 15.0f;
        gs.player().data.player.state = PlayerState::idle;
    }
    else if (gs.levelNum == 4 && ch.choice14)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Then you are free to leave.");
        gs.choiceElapsed += deltaTime;

        if (gs.choiceElapsed >= 2.0f)
        {
            gs.choiceElapsed = 0.0f;
            gs.restartRequested = true;
        }
    }
    //level 3
    else if (gs.levelNum == 3 && (ch.choice1 || ch.choice2 || ch.choice3))
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Good job, you chose right. We're going to");
        SDL_RenderDebugText(state.renderer, 155, 290, "need your talents");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice1 = false;
            ch.choice2 = false;
            ch.choice3 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    //level 5
    else if(gs.levelNum == 5 && ch.choice4)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Alright we trust in your decision. We'll");
        SDL_RenderDebugText(state.renderer, 155, 290, "Deal with him accordingly");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice4 = true;
            ch.choice5 = false;
            ch.choice6 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 5 && ch.choice5)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "If you say so, I'll give the order, we'll");
        SDL_RenderDebugText(state.renderer, 155, 290, "call you back when we have the next batch.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice4 = false;
            ch.choice5 = true;
            ch.choice6 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 5 && ch.choice6)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Good choice, he was bad mouthing us the");
        SDL_RenderDebugText(state.renderer, 155, 290, "whole way here, glad to get rid of him.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice4 = false;
            ch.choice5 = false;
            ch.choice6 = true;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    // level7
        else if(gs.levelNum == 7 && ch.choice7)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "We trust in your decision. Our Agents will");
        SDL_RenderDebugText(state.renderer, 155, 290, "deal with them. ");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice7 = true;
            ch.choice8 = false;
            ch.choice9 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 7 && ch.choice8)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Good decision, we'll send him back with");
        SDL_RenderDebugText(state.renderer, 155, 290, "our experts and call his family.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice7 = false;
            ch.choice8 = true;
            ch.choice9 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 7 && ch.choice9)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "Great choice, our specialists will deal");
        SDL_RenderDebugText(state.renderer, 155, 290, "with him, he'll be killed.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice7 = false;
            ch.choice8 = true;
            ch.choice9 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    //level 9
        else if(gs.levelNum == 9 && ch.choice10)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "I know it was a tough decision but good");
        SDL_RenderDebugText(state.renderer, 155, 290, "choice. She will be killed for our safety,");
        SDL_RenderDebugText(state.renderer, 155, 310, "you did this for the greater good.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice10 = true;
            ch.choice11 = false;
            ch.choice12 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 9 && ch.choice11)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "You did well, too bad he has to be killed.");
        SDL_RenderDebugText(state.renderer, 155, 290, "Remember you're doing this for the good of");
        SDL_RenderDebugText(state.renderer, 155, 310, "people.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice10 = false;
            ch.choice11 = true;
            ch.choice12 = false;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else if(gs.levelNum == 9 && ch.choice12)
    {
        SDL_RenderDebugText(state.renderer, 155, 270, "We hope you are right, mark on his face is");
        SDL_RenderDebugText(state.renderer, 155, 290, "red as the symptoms says it should be.");
        gs.choiceElapsed += deltaTime;

        if(gs.choiceElapsed >= 3.0f)
        {
            gs.choiceElapsed = 0.0f;
            ch.choice10 = false;
            ch.choice11 = false;
            ch.choice12 = true;
            gs.player().data.player.state = PlayerState::switchlvl;
        }
    }
    else
    {
        if (gs.levelNum == 4)
        {
            SDL_RenderDebugText(state.renderer, 155, 270, "Do you want to work with us?");
            SDL_RenderDebugText(state.renderer, 205, 320, "(1) Yes");
            SDL_RenderDebugText(state.renderer, 325, 320, "(2) No");
        }
        else
        {
            SDL_RenderDebugText(state.renderer, 155, 270, "So who do you think it is? Choose.");
            SDL_RenderDebugText(state.renderer, 150, 320, "(1)");
            SDL_RenderDebugText(state.renderer, 270, 320, "(2)");
            SDL_RenderDebugText(state.renderer, 390, 320, "(3)");
        }
    }
}
void charSpeech(SDLState &state, GameState &gs, float deltaTime)
{
    switch(gs.levelNum)
    {
        default:
        {
            
        }
        case 4:
        {
            movementKeys = false;
            if (gs.player().data.player.state == PlayerState::choice)
            {
                break;
            }

            gs.dialogSkip3 += deltaTime;
            if(gs.dialogSkip3 < 5.0f)
            {
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                SDL_RenderRect(state.renderer, &levelTwoTextBox);
                SDL_RenderDebugText(state.renderer, 155, 250, "???");
                SDL_RenderDebugText(state.renderer, 155, 270, "Thanks for picking him out, we needed");
                SDL_RenderDebugText(state.renderer, 155, 290, "someone with your talent, and experience.");
                SDL_RenderDebugText(state.renderer, 155, 310, "You are patient 0 afterall.");
            }
            else if (gs.dialogSkip3 < 11.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "So do you want to work with us? We'll");
                        SDL_RenderDebugText(state.renderer, 155, 290, "guarantee your freedom and treat you. We");
                        SDL_RenderDebugText(state.renderer, 155, 310, "have the best healthcare afterall.");
                    }
            else if (gs.dialogSkip3 < 15.0f)
                    {
                        gs.player().data.player.state = PlayerState::choice;
                    }
            else if (gs.dialogSkip3 < 19.0f)
            {
                SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                SDL_RenderRect(state.renderer, &levelTwoTextBox);
                SDL_RenderDebugText(state.renderer, 155, 250, "???");
                SDL_RenderDebugText(state.renderer, 155, 270, "Good, just go through the door, right in ");
                SDL_RenderDebugText(state.renderer, 155, 290, "between Agent A and Agent B. If anything");
                SDL_RenderDebugText(state.renderer, 155, 310, "goes wrong, we'll pull you out guranteed.");
            }
            else
            {
                movementKeys = true;
            }
            break;
        }
        case 5://edit this one later
        {
            gs.dialogSkip4 += deltaTime;
            movementKeys = false;
                    if(gs.dialogSkip4 < 4.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Alright just like last time, choose the");
                        SDL_RenderDebugText(state.renderer, 155, 290, "sick one. This time the symptomes evolved");
                        SDL_RenderDebugText(state.renderer, 155, 310, "if they have a red dots, they are sick.");
                    }
                    else 
                    {
                        movementKeys = true;
                    }

                break;
            }
            case 6:
            {
                movementKeys = false;
                gs.dialogSkip5 += deltaTime;
                if(gs.dialogSkip5 < 3.0f)
                {          
                    if(ch.choice4)
                    {                
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Another good job kid, he was hiding it but");
                        SDL_RenderDebugText(state.renderer, 155, 290, "you caught it. We'll make sure you're");
                        SDL_RenderDebugText(state.renderer, 155, 310, "compensated once this is over.");
                    }
                    else if(ch.choice5)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Turns out he wasn't our guy, so we'll send");
                        SDL_RenderDebugText(state.renderer, 155, 290, "you back.");
                    }
                    else if(ch.choice6)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Chose wrong, so we had to go with plan B.");
                        SDL_RenderDebugText(state.renderer, 155, 290, "We'll keep you on board for now though,");
                        SDL_RenderDebugText(state.renderer, 155, 310, "since he had the symptoms we told you.");
                    }
                }
                else 
                {
                    movementKeys = true;
                    if (ch.choice5)
                    {
                        gs.restartRequested = true;
                    }
                }
                break;
            }
            case 7:
            {
                gs.dialogSkip6 += deltaTime;
                movementKeys = false;
                    if(gs.dialogSkip6 < 4.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "You know the drill, be careful though");
                        SDL_RenderDebugText(state.renderer, 155, 290, "these ones are a bit resistant. And the");
                        SDL_RenderDebugText(state.renderer, 155, 310, "the symptomes evolved, lookout for a cough");
                    }
                    else 
                    {
                        movementKeys = true;
                    }

                break;

            }
            case 8:
            {
                movementKeys = false;
                gs.dialogSkip7 += deltaTime;
                if(gs.dialogSkip7 < 3.0f)
                {                   
                    if(ch.choice7)
                    {                
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "No luck, wasn't him similiar disease, but");
                        SDL_RenderDebugText(state.renderer, 155, 290, "not him. We can't afford another mess like");
                        SDL_RenderDebugText(state.renderer, 155, 310, "this one, so get your act together.");
                    }
                    else if(ch.choice8)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Good job, keep this up and we'll beat");
                        SDL_RenderDebugText(state.renderer, 155, 290, "this outbreak in no time, no more");
                        SDL_RenderDebugText(state.renderer, 155, 310, "quarantine.");
                    }
                    else if(ch.choice9)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "???");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Nice job, turns out he had a mutation of");
                        SDL_RenderDebugText(state.renderer, 155, 290, "the disease, could have made the outbreak");
                        SDL_RenderDebugText(state.renderer, 155, 310, "worse, had no symptoms for some odd reason");
                    }
                }
                else
                {
                    movementKeys = true;
                }
                break;
            }
            case 9:
            {
                gs.dialogSkip8 += deltaTime;
                movementKeys = false;
                    if(gs.dialogSkip8 < 4.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "Mendax");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Last batch, guess correctly and we'll let");
                        SDL_RenderDebugText(state.renderer, 155, 290, "you go. A familiar face is waiting for you");
                    }
                    else 
                    {
                        movementKeys = true;
                    }

                break;
            }
            case 10:
            {
                gs.dialogSkip9 += deltaTime;
                movementKeys = false;
                    if(gs.dialogSkip9 < 2.0f)
                    {
                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, gs.playerName.c_str());
                        SDL_RenderDebugText(state.renderer, 155, 270, "What have you done.");
                    }
                    else if(gs.dialogSkip9 > 2.0f && gs.dialogSkip9 < 4.0f)
                    {

                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, gs.playerName.c_str());
                        SDL_RenderDebugText(state.renderer, 155, 270, "They weren't even sick.");
                    }
                    else if(gs.dialogSkip9 > 4.0f && gs.dialogSkip9 < 6.0f)
                    {

                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, gs.playerName.c_str());
                        SDL_RenderDebugText(state.renderer, 155, 270, "That guy was right, who was I to judge if");
                        SDL_RenderDebugText(state.renderer, 155, 290, "someone can live or die.");
                    }
                    else if(gs.dialogSkip9 > 6.0f && gs.dialogSkip9 < 8.0f)
                    {

                        SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 220);
                        SDL_FRect levelTwoTextBox { 143, 240, 350, 100 };
                        SDL_RenderFillRect(state.renderer, &levelTwoTextBox);

                        SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
                        SDL_RenderRect(state.renderer, &levelTwoTextBox);
                        SDL_RenderDebugText(state.renderer, 155, 250, "Mendax");
                        SDL_RenderDebugText(state.renderer, 155, 270, "Government stopped funding the project so");
                        SDL_RenderDebugText(state.renderer, 155, 290, "you're free to go. You did well kid.");
                    }
                    else 
                    {
                        movementKeys = true;
                    }
                break;
            }
        }
    }
