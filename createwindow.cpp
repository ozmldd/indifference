#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <string>

bool init();
bool loadMedia();
void close();

    constexpr int kScreenWidth{ 640 };
    constexpr int kScreenHeight{ 480 };
    
    SDL_Window* gWindow{ nullptr };
    SDL_Surface* gScreenSurface{ nullptr };
    SDL_Surface* gHelloWorld{ nullptr };

int main(){

    //Final exit code
    int exitCode {0};

    //initializing
    if( init() == false ){
        SDL_Log("Unable to initialize program!\n");
        exitCode = 1;
    }
    else{
        //load media
        if(loadMedia() == false ){
            SDL_Log("Unable to load media!\n");
            exitCode = 2;
        }
        else{
            // the quit flag
            bool quit { false };

            // the event data
            SDL_Event e;
            SDL_zero ( e );
    
    //the main loop
    while( quit == false ){
        //get event data
        while ( SDL_PollEvent ( &e ) == true ){
            //if event is quit type
            if ( e.type == SDL_EVENT_QUIT){
                // end main loop
                quit = true;
            }
        }
    //Fill the surface white
    SDL_FillSurfaceRect ( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 0xFF, 0xFF, 0xFF) );
    
    //Render image on screen
    SDL_BlitSurface ( gHelloWorld, nullptr, gScreenSurface, nullptr);

    //update surface
    SDL_UpdateWindowSurface (gWindow);
    }
        }
    }
    //clean up
    close();


    return exitCode;
}

bool init(){
    bool success{ true };

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not be initialized! SDL error: %s\n", SDL_GetError());
        success = false;
    } else {
        gWindow = SDL_CreateWindow("SDL3 Tutorial: Hello SDL3", kScreenWidth, kScreenHeight, 0);
        if (gWindow == nullptr) {
            SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
            success = false;
        } else {
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }

    return success;
}

bool loadMedia(){
    //file loading flag
    bool success { true };
    // load image
    std::string imagePath { "C:\\Users\\erick\\Downloads\\hello-sdl3.bmp" };
    gHelloWorld = SDL_LoadBMP( imagePath.c_str() );
    if( gHelloWorld == nullptr){
        SDL_Log( "Using fallback surface because %s was not found.\n", imagePath.c_str() );
        gHelloWorld = SDL_CreateSurface(200, 100, SDL_PIXELFORMAT_RGB24);
        if( gHelloWorld == nullptr){
            SDL_Log( "Unable to create fallback surface! SDL Error : %s\n", SDL_GetError() );
            success = false;
        }
        else{
            SDL_FillSurfaceRect( gHelloWorld, nullptr, SDL_MapSurfaceRGB( gHelloWorld, 0x00, 0x80, 0x80 ) );
        }
    }
    return success;
}

void close(){
    //clean up surface
    SDL_DestroySurface( gHelloWorld );
    gHelloWorld = nullptr;

    // destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = nullptr;
    gScreenSurface = nullptr;

    // Quit SDL subsystems
    SDL_Quit();

}