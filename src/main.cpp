#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#include <vector>
#include <map>
#include <math.h>

#include "Exception.hpp"
#include "MediaManager.hpp"
#include "Game.hpp"
#include "Particle.hpp"
#include "Animation.hpp"
#include "Wave.hpp"
#include "Player.hpp"
#include "Entity.hpp"

using namespace std;

class MyGame:public Game {
	Waves *waves;
	Player *player;
	vector<Entity *>entities;

	Mix_Chunk *backgroundMusic;

	int w,h;

	Animation *tvStatic;
	SDL_Rect *staticDest;

	void initPlayer(){
		//this list of actions could be setup in a config file
		map<string,Animation *> playerAnimations;
		map<string,Mix_Chunk *> playerSounds;

		playerAnimations["walkRight"] = new Animation();
		playerAnimations["walkRight"]->readAnimation(media, "walkRight");

		playerAnimations["walkLeft"] = new Animation();
		playerAnimations["walkLeft"]->readAnimation(media, "walkLeft");

		playerAnimations["idle"] = new Animation();
		playerAnimations["idle"]->readAnimation(media, "idle");

		playerSounds["footstep"] = new Mix_Chunk();
		playerSounds["footstep"]=media->readSound("footstep");

		playerSounds["clap"] = new Mix_Chunk();
		playerSounds["clap"]=media->readSound("clap");

		player = new Player(ren, playerAnimations, playerAnimations["idle"], waves, playerSounds, 100, (480/2)-64);
	}

	void spawnEntity(int num=1, string type="") {
		map<string,Animation *> animations;
		map<string,Mix_Chunk *> sounds;

		animations["walkRight"] = new Animation();
		animations["walkRight"]->readAnimation(media, "walkRight");

		animations["walkLeft"] = new Animation();
		animations["walkLeft"]->readAnimation(media, "walkLeft");

		animations["idle"] = new Animation();
		animations["idle"]->readAnimation(media, "idle");

		sounds["footstep"] = new Mix_Chunk();
		sounds["footstep"]=media->readSound("footstep");

		sounds["clap"] = new Mix_Chunk();
		sounds["clap"]=media->readSound("clap");

		for (int i=0; i<num; i++) entities.push_back(new Entity(ren, animations, animations["idle"], waves, sounds, type, 300+i*50, (480/2)-64));
	}

	public:
	MyGame(int newW=640, int newH=480):Game("Echos", newW, newH) {
		w = newW;
		h = newH;

		//This block is for initing the static effect
		tvStatic = new Animation();
		tvStatic->readAnimation(media, "static");
		SDL_Texture *staticTexture = tvStatic->getTexture();
		SDL_SetTextureAlphaMod(staticTexture, 100);

		staticDest = new SDL_Rect();
		staticDest->x = 0;
		staticDest->x = 0;
		staticDest->w = w;
		staticDest->h = h;

		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

		waves = new Waves(ren);
		backgroundMusic = media->readSound("backgroundMusic");
		initPlayer();
		spawnEntity(2);
		Mix_PlayChannel(-1,backgroundMusic,-1);
	}

	void update(double dt) {
		vector<int> locations;
		for (int i=0; i<entities.size(); i++) {
			SDL_bool collision = SDL_HasIntersection(entities[i]->getDest(),player->getDest());
			if (collision) {
				cout << "Entity has died :c" << endl;
				entities[i]->killed();
				locations.push_back(i-locations.size());
			}
		}
		for (auto i:locations)
			entities.erase(entities.begin()+i);
		
		SDL_RenderClear(ren);

		waves->updateWaves(dt);

		player->update(dt);

		for (auto e:entities) e->update(dt);

		tvStatic->update(dt);
		SDL_RenderCopy(ren, tvStatic->getTexture(), tvStatic->getFrame(), staticDest);
		 
		SDL_RenderPresent(ren);
	}

	void handleKeyUp(SDL_Event keyEvent) {
		if (keyEvent.key.keysym.sym==SDLK_a || keyEvent.key.keysym.sym==SDLK_LEFT ||
			keyEvent.key.keysym.sym==SDLK_d || keyEvent.key.keysym.sym==SDLK_RIGHT
		){
			player->stopMoving();
		}	
	}

	void handleKeyDown(SDL_Event keyEvent) {
		if(!player->isMoving()){
			if(keyEvent.key.keysym.sym==SDLK_a || keyEvent.key.keysym.sym==SDLK_LEFT) {
				player->walkLeft();
			}
			else if(keyEvent.key.keysym.sym==SDLK_d || keyEvent.key.keysym.sym==SDLK_RIGHT) {
				player->walkRight();
			}
		}
		
		if(keyEvent.key.keysym.sym==SDLK_e)
			player->clap();
	}

	~MyGame() {
		delete player;
		delete waves;
		for (auto e:entities) delete e;
	}
};

int main(int argc, char* argv[]) {
    try {
		MyGame g;
		g.run();
	} catch (Exception e) {
		cerr << e;
	}
    return 0;
}