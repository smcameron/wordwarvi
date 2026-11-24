/*
    (C) Copyright 2007,2008, Stephen M. Cameron.

    This file is part of wordwarvi.

    wordwarvi is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    wordwarvi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with wordwarvi; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */

#ifndef WWVIAUDIO_STUBS_ONLY

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <SDL.h>

#define WWVIAUDIO_DEFINE_GLOBALS
#include "wwviaudio.h"
#undef WWVIAUDIO_DEFINE_GLOBALS

#include "ogg_to_pcm.h"

#define FRAMES_PER_BUFFER  (1024)

static SDL_AudioDeviceID audio_device = 0;
static int sdl_audio_initialized = 0;
static int audio_paused = 0;
static int music_playing = 1;
static int sound_working = 0;
static int nomusic = 0;
static int sound_effects_on = 1;
static int sound_device = -1; /* default sound device for SDL2 audio. */
static unsigned int max_concurrent_sounds = 0;
static int max_sound_clips = 0;

static void lock_audio(void)
{
	if (audio_device != 0)
		SDL_LockAudioDevice(audio_device);
}

static void unlock_audio(void)
{
	if (audio_device != 0)
		SDL_UnlockAudioDevice(audio_device);
}

/* Pause all audio output, output silence. */
void wwviaudio_pause_audio(void)
{
	lock_audio();
	audio_paused = 1;
	unlock_audio();
}

/* Resume playing audio previously paused. */
void wwviaudio_resume_audio(void)
{
	lock_audio();
	audio_paused = 0;
	unlock_audio();
}

/* Silence the music channel */
void wwviaudio_silence_music(void)
{
	lock_audio();
	music_playing = 0;
	unlock_audio();
}

/* Resume volume on the music channel. */
void wwviaudio_resume_music(void)
{
	lock_audio();
	music_playing = 1;
	unlock_audio();
}

void wwviaudio_toggle_music(void)
{
	lock_audio();
	music_playing = !music_playing;
	unlock_audio();
}

/* Silence the sound effects. */
void wwviaudio_silence_sound_effects(void)
{
	lock_audio();
	sound_effects_on = 0;
	unlock_audio();
}

/* Resume volume on the sound effects. */
void wwviaudio_resume_sound_effects(void)
{
	lock_audio();
	sound_effects_on = 1;
	unlock_audio();
}

void wwviaudio_toggle_sound_effects(void)
{
	lock_audio();
	sound_effects_on = !sound_effects_on;
	unlock_audio();
}

void wwviaudio_set_nomusic(void)
{
	nomusic = 1;
}

static struct sound_clip {
	int active;
	int nsamples;
	int pos;
	int16_t *sample;
} *clip = NULL;

static struct sound_clip *audio_queue = NULL;

int wwviaudio_read_ogg_clip(int clipnum, char *filename)
{
	uint64_t nframes = 0;
	char filebuf[PATH_MAX + 1];
	struct stat statbuf;
	int sample_rate;
	int nchannels;
	int nsamples;
	int16_t *pcm = NULL;
	int16_t *old_sample;
	unsigned int i;
	int rc;

	if (clip == NULL || clipnum >= max_sound_clips || clipnum < 0)
		return -1;

#ifndef __WIN32__
	snprintf(filebuf, PATH_MAX, "%s/%s", DATADIR, filename);
	rc = stat(filebuf, &statbuf);
	if (rc != 0) {
		strncpy(filebuf, filename, PATH_MAX);
		filebuf[PATH_MAX] = '\0';
		rc = stat(filebuf, &statbuf);
		if (rc != 0) {
			fprintf(stderr, "stat('%s') failed.\n", filebuf);
			return -1;
		}
	}
#else
	/* use relative path on win32 */
	snprintf(filebuf, PATH_MAX, "%s", filename);
#endif
	rc = ogg_to_pcm(filebuf, &pcm, &sample_rate, &nchannels, &nframes);
	if (pcm == NULL) {
		printf("Can't get memory for sound data for %" PRIu64 " frames in %s\n",
			nframes, filebuf);
		return -1;
	}

	if (rc != 0) {
		fprintf(stderr, "Error: ogg_to_pcm('%s') failed.\n",
			filebuf);
		free(pcm);
		return -1;
	}

	nsamples = (int) nframes;
	if (nsamples < 0)
		nsamples = 0;

	lock_audio();
	old_sample = clip[clipnum].sample;
	if (old_sample != NULL && audio_queue != NULL) {
		for (i = 0; i < max_concurrent_sounds; i++) {
			if (audio_queue[i].sample == old_sample) {
				audio_queue[i].active = 0;
				audio_queue[i].sample = NULL;
				audio_queue[i].nsamples = 0;
				audio_queue[i].pos = 0;
			}
		}
	}
	clip[clipnum].sample = pcm;
	clip[clipnum].nsamples = nsamples;
	unlock_audio();

	free(old_sample);

	return 0;
}

/* This routine is called by the SDL2 audio thread when audio is needed. */
static void sdl_audio_callback(__attribute__ ((unused)) void *userdata,
	Uint8 *stream, int len)
{
	unsigned int i, j;
	unsigned int frames;
	int sample;
	float *out;
	float output;

	if (len <= 0)
		return;

	frames = (unsigned int) len / (unsigned int) sizeof(float);
	out = (float *) (void *) stream;

	if (audio_paused) {
		/* output silence when paused and
		 * don't advance any sound slot pointers
		 */
		memset(stream, 0, (size_t) len);
		return;
	}

	for (i = 0; i < frames; i++) {
		output = 0.0f;
		for (j = 0; j < max_concurrent_sounds; j++) {
			if (!audio_queue[j].active ||
				audio_queue[j].sample == NULL)
				continue;
			sample = (int) i + audio_queue[j].pos;
			if (sample >= audio_queue[j].nsamples) {
				audio_queue[j].active = 0;
				continue;
			}
			if (j != WWVIAUDIO_MUSIC_SLOT && sound_effects_on)
				output += (float) audio_queue[j].sample[sample] * 0.5f / (float) (INT16_MAX);
			else if (j == WWVIAUDIO_MUSIC_SLOT && music_playing)
				output += (float) audio_queue[j].sample[sample] / (float) (INT16_MAX);
		}
		output *= 0.5f;
		if (output > 1.0f)
			output = 1.0f;
		else if (output < -1.0f)
			output = -1.0f;
		*out++ = output;
	}
	for (i = 0; i < max_concurrent_sounds; i++) {
		if (!audio_queue[i].active)
			continue;
		audio_queue[i].pos += (int) frames;
		if (audio_queue[i].pos >= audio_queue[i].nsamples)
			audio_queue[i].active = 0;
	}
}

int wwviaudio_initialize_audio(int maximum_concurrent_sounds, int maximum_sound_clips)
{
	SDL_AudioSpec want, have;
	const char *device_name = NULL;
	int device_count;
	int i;

	if (maximum_concurrent_sounds < 0 || maximum_sound_clips < 0)
		return -1;

	max_concurrent_sounds = (unsigned int) maximum_concurrent_sounds;
	max_sound_clips = maximum_sound_clips;

	audio_queue = malloc(max_concurrent_sounds * sizeof(audio_queue[0]));
	clip = malloc((size_t) max_sound_clips * sizeof(clip[0]));
	if (audio_queue == NULL || clip == NULL)
		goto error;

	memset(audio_queue, 0, sizeof(audio_queue[0]) * max_concurrent_sounds);
	memset(clip, 0, sizeof(clip[0]) * (size_t) max_sound_clips);

	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "Unable to initialize SDL audio: %s\n", SDL_GetError());
		goto error;
	}
	sdl_audio_initialized = 1;

	device_count = SDL_GetNumAudioDevices(0);
	if (device_count >= 0) {
		printf("SDL2 reports %d audio output devices.\n", device_count);
		for (i = 0; i < device_count; i++)
			printf("  Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}

	if (sound_device != -1) {
		if (sound_device < 0 || device_count <= 0 || sound_device >= device_count)
			fprintf(stderr, "wordwarvi:  Invalid sound device "
				"%d specified, ignoring.\n", sound_device);
		else {
			device_name = SDL_GetAudioDeviceName(sound_device, 0);
			printf("Using sound device %d: %s\n", sound_device,
				device_name ? device_name : "(default)");
		}
	}

	SDL_zero(want);
	want.freq = WWVIAUDIO_SAMPLE_RATE;
	want.format = AUDIO_F32SYS;
	want.channels = 1;
	want.samples = FRAMES_PER_BUFFER;
	want.callback = sdl_audio_callback;
	want.userdata = NULL;

	audio_device = SDL_OpenAudioDevice(device_name, 0, &want, &have, 0);
	if (audio_device == 0) {
		fprintf(stderr, "Unable to open SDL audio device: %s\n", SDL_GetError());
		goto error;
	}

	sound_working = 1;
	SDL_PauseAudioDevice(audio_device, 0);
	return 0;

error:
	wwviaudio_stop_audio();
	return -1;
}

void wwviaudio_stop_audio(void)
{
	int i;

	sound_working = 0;

	if (audio_device != 0) {
		SDL_CloseAudioDevice(audio_device);
		audio_device = 0;
	}
	if (sdl_audio_initialized) {
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		sdl_audio_initialized = 0;
	}
	if (audio_queue) {
		free(audio_queue);
		audio_queue = NULL;
		max_concurrent_sounds = 0;
	}
	if (clip) {
		for (i = 0; i < max_sound_clips; i++) {
			if (clip[i].sample)
				free(clip[i].sample);
		}
		free(clip);
		clip = NULL;
		max_sound_clips = 0;
	}
}

static int wwviaudio_add_sound_to_slot(int which_sound, int which_slot)
{
	unsigned int i;

	if (!sound_working)
		return 0;

	if (which_sound < 0 || which_sound >= max_sound_clips)
		return -1;

	if (nomusic && which_slot == WWVIAUDIO_MUSIC_SLOT)
		return 0;

	if (which_slot != WWVIAUDIO_ANY_SLOT) {
		if (which_slot < 0 || (unsigned int) which_slot >= max_concurrent_sounds)
			return -1;
		lock_audio();
		audio_queue[which_slot].active = 0;
		audio_queue[which_slot].pos = 0;
		audio_queue[which_slot].sample = clip[which_sound].sample;
		audio_queue[which_slot].nsamples = clip[which_sound].nsamples;
		audio_queue[which_slot].active = 1;
		unlock_audio();
		return which_slot;
	}

	lock_audio();
	for (i = 1; i < max_concurrent_sounds; i++) {
		if (audio_queue[i].active == 0) {
			audio_queue[i].nsamples = clip[which_sound].nsamples;
			audio_queue[i].pos = 0;
			audio_queue[i].sample = clip[which_sound].sample;
			audio_queue[i].active = 1;
			break;
		}
	}
	unlock_audio();
	return (i >= max_concurrent_sounds) ? -1 : (int) i;
}

int wwviaudio_add_sound(int which_sound)
{
	return wwviaudio_add_sound_to_slot(which_sound, WWVIAUDIO_ANY_SLOT);
}

int wwviaudio_play_music(int which_sound)
{
	return wwviaudio_add_sound_to_slot(which_sound, WWVIAUDIO_MUSIC_SLOT);
}

void wwviaudio_add_sound_low_priority(int which_sound)
{
	/* adds a sound if there are at least 5 empty sound slots. */
	unsigned int i;
	int empty_slots = 0;
	int last_slot;

	if (!sound_working)
		return;
	if (which_sound < 0 || which_sound >= max_sound_clips)
		return;

	lock_audio();
	last_slot = -1;
	for (i = 1; i < max_concurrent_sounds; i++) {
		if (audio_queue[i].active == 0) {
			last_slot = (int) i;
			empty_slots++;
			if (empty_slots >= 5)
				break;
		}
	}

	if (empty_slots < 5 || last_slot < 0) {
		unlock_audio();
		return;
	}

	i = (unsigned int) last_slot;
	if (audio_queue[i].active == 0) {
		audio_queue[i].nsamples = clip[which_sound].nsamples;
		audio_queue[i].pos = 0;
		audio_queue[i].sample = clip[which_sound].sample;
		audio_queue[i].active = 1;
	}
	unlock_audio();
}

void wwviaudio_cancel_sound(int queue_entry)
{
	if (!sound_working)
		return;
	if (queue_entry < 0 || (unsigned int) queue_entry >= max_concurrent_sounds)
		return;
	lock_audio();
	audio_queue[queue_entry].active = 0;
	unlock_audio();
}

void wwviaudio_cancel_music(void)
{
	wwviaudio_cancel_sound(WWVIAUDIO_MUSIC_SLOT);
}

void wwviaudio_cancel_all_sounds(void)
{
	unsigned int i;
	if (!sound_working)
		return;
	lock_audio();
	for (i = 0; i < max_concurrent_sounds; i++)
		audio_queue[i].active = 0;
	unlock_audio();
}

int wwviaudio_set_sound_device(int device)
{
	sound_device = device;
	return 0;
}

#else /* stubs only... */

#define WWVIAUDIO_DEFINE_GLOBALS
#include "wwviaudio.h"
#undef WWVIAUDIO_DEFINE_GLOBALS

int wwviaudio_initialize_audio(__attribute__ ((unused)) int maximum_concurrent_sounds,
	__attribute__ ((unused)) int maximum_sound_clips) { return 0; }
void wwviaudio_stop_audio(void) { return; }
void wwviaudio_set_nomusic(void) { return; }
int wwviaudio_read_ogg_clip(__attribute__ ((unused)) int clipnum,
	__attribute__ ((unused)) char *filename) { return 0; }

void wwviaudio_pause_audio(void) { return; }
void wwviaudio_resume_audio(void) { return; }

void wwviaudio_silence_music(void) { return; }
void wwviaudio_resume_music(void) { return; }
void wwviaudio_silence_sound_effects(void) { return; }
void wwviaudio_resume_sound_effects(void) { return; }
void wwviaudio_toggle_sound_effects(void) { return; }

int wwviaudio_play_music(__attribute__ ((unused)) int which_sound) { return 0; }
void wwviaudio_cancel_music(void) { return; }
void wwviaudio_toggle_music(void) { return; }
int wwviaudio_add_sound(__attribute__ ((unused)) int which_sound) { return 0; }
void wwviaudio_add_sound_low_priority(__attribute__ ((unused)) int which_sound) { return; }
void wwviaudio_cancel_sound(__attribute__ ((unused)) int queue_entry) { return; }
void wwviaudio_cancel_all_sounds(void) { return; }
int wwviaudio_set_sound_device(__attribute__ ((unused)) int device) { return 0; }

#endif
