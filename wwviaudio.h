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

GLOBAL int wwviaudio_initialize_portaudio();
GLOBAL void wwviaudio_stop_portaudio();
GLOBAL void wwviaudio_set_nomusic();
GLOBAL int wwviaudio_read_ogg_clip(int clipnum, char *filename);

GLOBAL void wwviaudio_pause_audio();
GLOBAL void wwviaudio_resume_audio();

GLOBAL void wwviaudio_silence_music();
GLOBAL void wwviaudio_resume_music();
GLOBAL void wwviaudio_silence_sound_effects();
GLOBAL void wwviaudio_resume_sound_effects();
GLOBAL void wwviaudio_toggle_sound_effects();

GLOBAL int wwviaudio_play_music(int which_sound);
GLOBAL void wwviaudio_cancel_music();
GLOBAL void wwviaudio_toggle_music();
GLOBAL int wwviaudio_add_sound(int which_sound);
GLOBAL void wwviaudio_add_sound_low_priority(int which_sound);
GLOBAL void wwviaudio_cancel_sound(int queue_entry);
GLOBAL void wwviaudio_cancel_all_sounds();

GLOBAL int wwviaudio_set_sound_device(int device);

#endif
