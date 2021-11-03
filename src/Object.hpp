#pragma once

#include <map>
#include <SDL_mixer.h>
#include <SDL.h>

#include "Particle.hpp"
#include "Animation.hpp"
#include "MediaManager.hpp"
#include "Config.hpp"
#include "Wave.hpp"

using namespace std;

class Object:public Particle{
  Config *cfg;
  SDL_Renderer *ren;
  MediaManager *media;

  map<string,Animation *> animations;
  map<string, Mix_Chunk *> sounds;
  
protected:
  Animation *a;
  SDL_Rect dest;
  Waves *waves;

public:
  Object(MediaManager *newMedia, SDL_Renderer *newRen, Waves *newWaves, Config *newCfg, string objType,
  double newx=0.0, double newy=0.0,
  double newv=0.0, int newtheta=0,
  double newax=0.0, double neway=0.0,
  double newdamp=0.0):Particle(newx, newy, newv, newtheta, newax, neway, newdamp) {

    ren=newRen;
    media=newMedia;
    cfg=newCfg;
    waves=newWaves;
    
    dest.w = stoi((*cfg)["width"]);
    dest.h = stoi((*cfg)["height"]);

    vector<string> newAnimations = cfg->getMany("animations");
		for(auto anim: newAnimations){
			animations[anim] = new Animation();
			animations[anim]->readAnimation(media, anim);
		}

		a=animations[(*cfg)["defaultAnimation"]];
		
		vector<string> newSounds = cfg->getMany("sounds");
		for(auto sound: newSounds){
			sounds[sound] = new Mix_Chunk();
			sounds[sound] = media->readSound(sound);
    }

    if (objType == "floor") y=480/2+1;
  }

  SDL_Rect *getDest() { return &dest; }

  void setAnimation(Animation *newA) { a=newA; }

  void update(double dt) {
    a->update(dt);
    dest.x = x;
    dest.y = y;

    SDL_RenderCopy(ren, a->getTexture(), a->getFrame(), &dest);
  }

  ~Object() {
    for (auto a:animations) delete a.second;
    for (auto s:sounds) delete s.second;
  }
};