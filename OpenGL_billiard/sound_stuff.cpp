/* sound_stuff.c
**
**    code for sound sample data
**    Copyright (C) 2001  Florian Berger
**    Email: harpin_floh@yahoo.de, florian.berger@jk.uni-linz.ac.at
**
**    Updated Version foobillard++ started at 12/2010
**    Copyright (C) 2010 - 2013 Holger Schaekel (foobillardplus@go4more.de)
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License Version 2 as
**    published by the Free Software Foundation;
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software
**    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
*/
#include "sound_stuff.h"
#include "sound_raw_data.h"
#include "utils.h"

#include <windows.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

/***********************************************************************/

TSound ball_ball_snd;
TSound ball_wall_snd;
TSound ball_cue_snd;

int options_use_sound = 1;	/* 使用什么声音，0为无法使用声音 */

// Mix_Chunk actually holds the noise information.

Mix_Chunk* ball_hole;
Mix_Chunk* wave_shuffle;
Mix_Chunk* wave_oneball;
Mix_Chunk* wave_outball;
Mix_Chunk* cue_sound;
Mix_Chunk* wall_sound;
Mix_Chunk* ball_sound;

/***********************************************************************/

/* Internal Routines */

void init_sound(void);
void exit_sound(void);
Mix_Chunk* loadsound(char* filename);
void PlayNoise(Mix_Chunk* chunkdata, int volume);
void create_cue_sound(short int** data, int* len, int length, float randmul, float maxrand, float sinfloat, float expfloat);
void create_wall_sound(short int** data, int* len);

/***********************************************************************
 *                             load sounds                             *
 ***********************************************************************/

Mix_Chunk* loadsound(const char* filename, unsigned char* fileb, long long file_size) {

	Mix_Chunk* chunkname;
	// 从内存中创建 SDL_RWops 结构
	SDL_RWops* rw = SDL_RWFromConstMem(fileb, file_size);
	if (!(chunkname = Mix_LoadWAV_RW(rw, 1))) {
		ErrorMsg("Initializing %s failed. No sound in game.", filename);
		options_use_sound = 0;
	}
	return (chunkname);
}

/***********************************************************************
 *                       Initialize the sound system                   *
 ***********************************************************************/

void init_sound(void)
{

	/* ball-ball sounds from samuele catuzzi's kbilliards - thanx */
	ball_ball_snd.len = raw_size_ball_ball;
	ball_ball_snd.data = reinterpret_cast<short int*>(raw_ball_ball);

	// Manipulate the ballsounds a little bit (for better sound)
	for (int i = 0; i < ball_ball_snd.len / 2 / 2; i++) {
		ball_ball_snd.data[(i) * 2] *= exp(-(float)i / (float)(ball_ball_snd.len / 2 / 4));
		ball_ball_snd.data[(i) * 2 + 1] *= exp(-(float)i / (float)(ball_ball_snd.len / 2 / 4));
	}
	for (int i = 0; i < ball_ball_snd.len / 2 / 2 - 1; i++) {
		ball_ball_snd.data[i * 2] = ball_ball_snd.data[i * 2] * 0.7 + ball_ball_snd.data[(i + 1) * 2] * 0.3;
		ball_ball_snd.data[i * 2 + 1] = ball_ball_snd.data[i * 2 + 1] * 0.7 + ball_ball_snd.data[(i + 1) * 2 + 1] * 0.3;
	}

	create_wall_sound(&ball_wall_snd.data, &ball_wall_snd.len);
	create_cue_sound(&ball_cue_snd.data, &ball_cue_snd.len, 2640, 0.6, 0.4, 20.0, 220.0); //length 2*2*3*220

	if (Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
		ErrorMsg("Unable to open audio!", NULL);
		options_use_sound = 0;
	}
	else {
		Mix_AllocateChannels(20); // max. 20 Channels
		Mix_Volume(-1, MIX_MAX_VOLUME);

		// Extended Init for Version higher then SDL 1.2.10
#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
		 // load support for the MP3, OGG music formats
		int flags = MIX_INIT_OGG | MIX_INIT_MP3;
		int initted = Mix_Init(flags);
		if ((initted & flags) != flags) {
			fprintf(stderr, "Mix_Init: Failed to init both ogg and mp3 support!\nCheck only for ogg.\n");
			fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
			flags = MIX_INIT_OGG; // check only for ogg
			initted = Mix_Init(flags);
			if ((initted & flags) != flags) {
				fprintf(stderr, "Mix_Init: Failed to init required ogg support!\n");
				fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
				options_use_music = 0;
			}
		}
#endif
		/* Actually loads up the sounds */
		ball_hole = loadsound("ballinhole.wav", raw_ballinhole, raw_size_ballinhole);
		wave_shuffle = loadsound("shuffleballs.wav", raw_shuffleballs, raw_size_shuffleballs);
		wave_oneball = loadsound("oneballontable.wav", raw_oneballontable, raw_size_oneballontable);
		wave_outball = loadsound("balloutoftable.wav", raw_balloutoftable, raw_size_balloutoftable);
		if (!(ball_sound = Mix_QuickLoad_RAW((Uint8*)ball_ball_snd.data, ball_ball_snd.len))) {
			ErrorMsg("Initializing internal ball-sound failed. No sound in game", NULL);
			options_use_sound = 0;
		}
		if (!(wall_sound = Mix_QuickLoad_RAW((Uint8*)ball_wall_snd.data, ball_wall_snd.len))) {
			ErrorMsg("Initializing internal wall-sound failed. No sound in game", NULL);
			options_use_sound = 0;
		}
		if (!(cue_sound = Mix_QuickLoad_RAW((Uint8*)ball_cue_snd.data, ball_cue_snd.len))) {
			ErrorMsg("Initializing internal cue-sound failed. No sound in game", NULL);
			options_use_sound = 0;
		}
	}
}

/***********************************************************************
 *                    plays sound with volume set                      *
 ***********************************************************************/
void PlayNoise(Mix_Chunk* chunkdata, int volume)
{
	if (options_use_sound) {
		if (chunkdata != NULL) {
			//fprintf(stderr,"Volume: %i\n",volume);
			Mix_VolumeChunk(chunkdata, volume);
			Mix_PlayChannel(-1, chunkdata, 0);
		}
	}
}


/***********************************************************************
 *                      close the sound system                         *
 ***********************************************************************/
void exit_sound(void)
{
	Mix_CloseAudio();
	/* ### TODO ### really needed, if mixer closes ??
	#if SDL_MIXER_MAJOR_VERSION > 2 || (SDL_MIXER_MINOR_VERSION == 2 && SDL_MIXER_PATCHLEVEL > 9)
		   Mix_Quit();
	#endif
	*/
}

/***********************************************************************
 *      Generates the sound for the cue. There is no file for it!      *
 ***********************************************************************/

void create_cue_sound(short int** data, int* len, int length, float randmul, float maxrand, float sinfloat, float expfloat)
{
	int i;
	*len = length;
	*data = (short int*)malloc(*len);
	for (i = 0; i < (*len) / 2 / 2; i++) {
		(*data)[i * 2] = 32000.0 * (randmul * (float)rand() / (float)RAND_MAX + (maxrand)*sin((float)i / sinfloat * 2.0 * M_PI)) * exp(-(float)i / expfloat);
		(*data)[i * 2 + 1] = (*data)[i * 2];
	}
	for (i = 1; i < (*len) / 2 / 2; i++) {
		(*data)[i * 2] = 0.5 * (*data)[i * 2] + 0.5 * (*data)[(i - 1) * 2];
		(*data)[i * 2 + 1] = (*data)[i * 2];
	}
	for (i = 30; i < (*len) / 2 / 2; i++) {
		(*data)[i * 2] = 0.7 * (*data)[i * 2] + 0.3 * (*data)[(i - 30) * 2];
		(*data)[i * 2 + 1] = (*data)[i * 2];
	}
}

/***********************************************************************
*   Generates the sound for the ball-wall. There is no file for it!    *
************************************************************************/

void create_wall_sound(short int** data, int* len)
{
	int i;
	create_cue_sound(data, len, 5584, 0.1, 0.9, 220.0, 465.0); // length 2*2*3*465+2*2
	for (i = 0; i < (*len) / 2 / 2; i++) {
		(*data)[i * 2] *= exp(-(float)(i) / 40.0 * (float)(i) / 40.0);
		(*data)[i * 2 + 1] *= exp(-(float)(i) / 40.0 * (float)(i) / 40.0);
	}
}
