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
#ifndef _WWVIAUDIO_H_
#define _WWVIAUDIO_H_

#ifdef WWVIAUDIO_DEFINE_GLOBALS
#define GLOBAL
#else
#define GLOBAL extern
#endif

#define MAX_CONCURRENT_SOUNDS (26)
#define NCLIPS 56
#define WWVIAUDIO_MUSIC_SLOT (0)
#define WWVIAUDIO_SAMPLE_RATE   (44100)
#define WWVIAUDIO_ANY_SLOT (-1)

/*
 *             Configuration functions.
 */
/* Disables the music channel.  Meant to be called prior to 
 * wwviaudio_initialize_portaudio */
GLOBAL void wwviaudio_set_nomusic();

/* Set the audio device number to the given value */
/* This is meant to be called prior to calling */
/* wwviaudio_initialize_portaudio.  If you don't use */
/* this function, portaudio's default device will be */
/* used.  This function provides a way to use a */
/* device other than the default */
GLOBAL int wwviaudio_set_sound_device(int device);
/* Initialize portaudio and start the audio engine. */
/* 0 is returned on success, -1 otherwise. */
GLOBAL int wwviaudio_initialize_portaudio();
/* Stop portaudio and the audio engine. */
GLOBAL void wwviaudio_stop_portaudio();

/*
 *             Audio data functions
 */
/* Read and decode an ogg vorbis audio file into a numbered buffer */
/* The clipnum parameter is used later with wwviaudio_play_music and */
/* wwviaudio_add_sound.  0 is returned on success, -1 otherwise. */
/* Audio files should be 44100Hz, MONO. */
GLOBAL int wwviaudio_read_ogg_clip(int clipnum, char *filename);

/*
 *             Global sound control functions.
 */
/* Suspend all audio playback.  Silence is output. */
GLOBAL void wwviaudio_pause_audio();
/* Resume all previously playing audio from whence it was previusly paused. */
GLOBAL void wwviaudio_resume_audio();

/*
 *             Music channel related functions 
 */
/* Begin playing a numbered buffer into the mix on the music channel */
/* The channel number of the music channel is returned. */
GLOBAL int wwviaudio_play_music(int which_sound);
/* Output silence on the music channel (pointer still advances though.) */
GLOBAL void wwviaudio_silence_music();
/* Unsilence the music channel */
GLOBAL void wwviaudio_resume_music();
/* Silence or unsilence the music channe. */ 
GLOBAL void wwviaudio_toggle_music();
/* Stop playing the playing buffer from the music channel */
GLOBAL void wwviaudio_cancel_music();

/*
 *             Sound effect (not music) related functions
 */
/* Begin playing a sound on a non-music channel.  The channel is returned. */
GLOBAL int wwviaudio_add_sound(int which_sound);
/* Begin playing a sound on a non-music channel.  The channel is returned. */
/* If fewer than five channels are open, the sound is not played, and -1 */
/* is returned. */
GLOBAL void wwviaudio_add_sound_low_priority(int which_sound);
/* Silence all channels but the music channel (pointers still advance though) */
GLOBAL void wwviaudio_silence_sound_effects();
/* Unsilence all channels but the music channel */
GLOBAL void wwviaudio_resume_sound_effects();
/* Either silence or unsilence all but the music channel */
GLOBAL void wwviaudio_toggle_sound_effects();
/* Stop playing the playing buffer from the given channel */
GLOBAL void wwviaudio_cancel_sound(int queue_entry);


/* Stop playing the playing buffer from all channels */
GLOBAL void wwviaudio_cancel_all_sounds();


#endif
