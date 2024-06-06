#pragma once

#define PLAY_NOISE(data,volume) PlayNoise(data,volume);

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

typedef struct {
    short int* data;
    int len;  /* length in bytes */
} TSound;

extern TSound ball_ball_snd;
extern TSound ball_wall_snd;
extern TSound ball_cue_snd;

/* Mix_Chunk actually holds the noise information. */

extern Mix_Chunk *ball_hole;
extern Mix_Chunk *wave_shuffle;
extern Mix_Chunk *wave_oneball;
extern Mix_Chunk *wave_outball;
extern Mix_Chunk *cue_sound;
extern Mix_Chunk *wall_sound;
extern Mix_Chunk *ball_sound;

void init_sound();
void exit_sound();
void PlayNoise(Mix_Chunk *chunkdata, int volume);
