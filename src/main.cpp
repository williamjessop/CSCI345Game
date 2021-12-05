#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <map>
#include <math.h>
#include <string>
#include <SDL_mutex.h>


#include "Exception.hpp"
#include "MediaManager.hpp"
#include "Game.hpp"
#include "Particle.hpp"
#include "Animation.hpp"
#include "Wave.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Config.hpp"
#include "Tile.hpp"
#include "Map.hpp"

using namespace std;

class MyGame:public Game {
	Waves *waves;

	Config *playerConf;
	Player *player;

	Map *level;
	string currentLevel;

	Mix_Chunk *backgroundMusic;

	Animation *tvStatic;
	SDL_Rect *staticDest;

	public:
	MyGame(Config &gameConf):Game(gameConf["name"], stoi(gameConf["screenW"]), stoi(gameConf["screenH"])) {
		backgroundMusic = media->readSound(gameConf["backgroundMusic"]);

		waves = new Waves(ren);

		currentLevel="level1";
		level = new Map(media, ren, waves, NULL);
		level->initMap(currentLevel);

		playerConf = new Config("player");
		player = new Player(media, ren, waves, playerConf, level->getStartX(), level->getStartY()+32);

		Mix_PlayChannel(-1,backgroundMusic,-1);

		//This block is for initing the static effect
		tvStatic = new Animation(100);
		tvStatic->readAnimation(media, "static");

		staticDest = new SDL_Rect();
		staticDest->x = 0;
		staticDest->x = 0;
		staticDest->w = stoi(gameConf["screenW"]);
		staticDest->h = stoi(gameConf["screenH"]);

		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
	}

	void levelChange(string levelName) {
		Map *oldLevel = level;

		Map *newLevel = new Map(media, ren, waves, NULL);
		newLevel->initMap(levelName);
		level = newLevel;
		player->setX(level->getStartX());
		player->setY(level->getStartY()-32);

		delete oldLevel;
		currentLevel = levelName;
	}

	void update(double dt) {
		level->update(dt, player);
		player->update(dt);

		tvStatic->update(dt);
	}

	void render(){
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tvStatic->getTexture(), tvStatic->getFrame(), staticDest);

		level->render(player);
		  
		SDL_RenderPresent(ren);
	}

	void handleKeyUp(SDL_Event keyEvent) {
		if (keyEvent.key.keysym.sym==SDLK_a || keyEvent.key.keysym.sym==SDLK_LEFT ||
			keyEvent.key.keysym.sym==SDLK_d || keyEvent.key.keysym.sym==SDLK_RIGHT
		){
			player->stopMoving();
		}
		if (keyEvent.key.keysym.sym==SDLK_e) { player->setClap(false); }
	}

	void handleKeyDown(SDL_Event keyEvent) {
		if(!player->isMoving()){
			if(keyEvent.key.keysym.sym==SDLK_a || keyEvent.key.keysym.sym==SDLK_LEFT)
				player->moveLeft();
			else if(keyEvent.key.keysym.sym==SDLK_d || keyEvent.key.keysym.sym==SDLK_RIGHT)
				player->moveRight();

			else if(keyEvent.key.keysym.sym==SDLK_SPACE)
				player->jump();
		}

		if(player->isMoving()){ //for jumping while moving
			if(keyEvent.key.keysym.sym==SDLK_SPACE)
				player->jump();
		}
		
		if(keyEvent.key.keysym.sym==SDLK_e)
			player->clap();
		else if(keyEvent.key.keysym.sym==SDLK_1)
			levelChange("level1");
		else if(keyEvent.key.keysym.sym==SDLK_2)
			levelChange("level2");
		// else if(keyEvent.key.keysym.sym==SDLK_3)
		// 	levelChange(3);

	}

	~MyGame() {
		delete player;
		delete waves;	
	}
};

int mainMenu(){
	SDL_Window *window;                    // Declare a pointer

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
	TTF_Init();	
	int w = 640;
	int h = 480;
    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "Main Menu",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        w,                               // width, in pixels
        h,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Renderer *ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Color black = {0, 0, 0, 255};
	SDL_Color white = { 225, 255, 255, 255};
	SDL_Color grey = {120, 120, 120, 255};

	TTF_Font *font = TTF_OpenFont("media/fonts/aovel-sans-rounded-font/AovelSansRounded-rdDL.ttf", 25);
	SDL_Surface *surface = TTF_RenderText_Solid(font, "Start Game", white);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);

	int texW = 0;
	int texH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);

	SDL_Rect dstrect = {w/2 - texW/2, h/2 - texH/2, texW, texH};

	// The window is open: could enter program loop here (see SDL_PollEvent())
	bool done = false;
	while (!done) {
        SDL_Event e;
		
		int x, y;
		Uint32 buttons;

		SDL_PumpEvents();

		buttons = SDL_GetMouseState(&x, &y);

        if (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT){ 
				done=true; 
				break;
			}else if(e.type == SDL_MOUSEBUTTONDOWN){
			 	if (x > w/2 - texW/2 && y > h/2 - texH/2 && x < w/2 + texW/2 && y < h/2 + texH/2){	
					TTF_CloseFont(font);
					SDL_DestroyTexture(texture);
					SDL_FreeSurface(surface);

					// Close and destroy the window
					SDL_DestroyWindow(window);

					// Clean up
					TTF_Quit();
					SDL_Quit();
					return 1;
				}
			}
		}

		// make black background
		SDL_SetRenderDrawColor(ren, black.r, black.g, black.b, black.a);
		SDL_RenderClear(ren);
	
		// handle mouseover
		if (x > w/2 - texW/2 && y > h/2 - texH/2 && x < w/2 + texW/2 && y < h/2 + texH/2){
			// set box color white
			SDL_SetRenderDrawColor(ren, 255,255,255,255);
			surface = TTF_RenderText_Solid(font, "TESTING", black);
			texture = SDL_CreateTextureFromSurface(ren, surface);
		}else{
			// set box color black
			SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
			surface = TTF_RenderText_Solid(font, "TESTING", white);
			texture = SDL_CreateTextureFromSurface(ren, surface);
		}

		SDL_RenderFillRect(ren, &dstrect);
		SDL_RenderCopy(ren, texture, NULL, &dstrect);

		SDL_RenderPresent(ren);
    }

	TTF_CloseFont(font);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(surface);

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
	TTF_Quit();
    SDL_Quit();
    return 0;
}

int main(int argc, char* argv[]) {

	int startGame = mainMenu();
	if(startGame){
		try {
			Config gameConf("game");

			MyGame g(gameConf);

			g.run();
		} catch (Exception e) {
			cerr << e;
		}
	}
    return 0;
}