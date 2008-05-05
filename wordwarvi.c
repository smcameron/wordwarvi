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
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <gdk/gdkkeysyms.h>
#include <stdint.h>
#include <math.h>

#define GNU_SOURCE
#include <getopt.h>

#ifdef WITHAUDIOSUPPORT
/* For Audio stuff... */
#include "portaudio.h"
#include "ogg_to_pcm.h"
#endif

#include "joystick.h"
#include "version.h"
#include "stamp.h"

#ifndef DATADIR
#define DATADIR ""
#endif

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (1024)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif
#define TWOPI (M_PI * 2.0)
#define NCLIPS (48)
#define MAX_CONCURRENT_SOUNDS (26)

/* define sound clip constants, and dedicated sound queue slots. */
int sound_device = -1; /* default sound device for port audio. */
int add_sound(int which_sound, int which_slot);
#define ANY_SLOT (-1)
#define MUSIC_SLOT (0)
#define PLAYER_LASER_SOUND 0
#define BOMB_IMPACT_SOUND 1
#define ROCKET_LAUNCH_SOUND 2
#define FLAK_FIRE_SOUND 3
#define LARGE_EXPLOSION_SOUND 4
#define ROCKET_EXPLOSION_SOUND 5
#define LASER_EXPLOSION_SOUND 6
#define GROUND_SMACK_SOUND 7
#define INSERT_COIN_SOUND 8
#define MUSIC_SOUND 9
#define SAM_LAUNCH_SOUND 10 
#define THUNDER_SOUND 11 
#define INTERMISSION_MUSIC_SOUND 12
#define MISSILE_LOCK_SIREN_SOUND 13
#define CARDOOR_SOUND 14
#define OWMYSPINE_SOUND 15
#define WOOHOO_SOUND 16
#define HELPDOWNHERE_SOUND 17
#define CRONSHOT 18
#define HELPUPHERE_SOUND 19
#define ABDUCTED_SOUND 20 
#define CLANG_SOUND 21
#define SCREAM_SOUND 22
#define BODYSLAM_SOUND 23
#define USETHESOURCE_SOUND 24
#define OOF_SOUND 25
#define METALBANG1 26
#define METALBANG2 27
#define METALBANG3 28
#define METALBANG4 29
#define METALBANG5 30 
#define METALBANG6 31
#define METALBANG7 32
#define METALBANG8 33
#define STONEBANG1 34
#define STONEBANG2 35
#define STONEBANG3 36
#define STONEBANG4 37
#define STONEBANG5 38
#define STONEBANG6 39
#define STONEBANG7 40
#define STONEBANG8 41
#define CORROSIVE_SOUND 42
#define VOLCANO_ERUPTION 43
#define IT_BURNS 44
#define ZZZT_SOUND 45

/* ...End of audio stuff */

#define NHUMANOIDS 4 /* number of vi .swp files, initially */
#define MAXHUMANS 10 /* maxiumum number possible of vi .swp files */
#define RADAR_HEIGHT 60  /* height, in pixels, of radar screen */
#define RADAR_LEFT_MARGIN 10 /* space to leave, in pixels left/right of radar screen to window edge. */
#define RADAR_RIGHT_MARGIN 160 /* space to leave, in pixels left/right of radar screen to window edge. */
#define	RADAR_YMARGIN 10 /* space to leave, in pixels from bottom edge of radar to bottom of window */
#define MAX_RADAR_NOISE 2000 /* maximum number of noise pixels placed by radar jammers, increasing */
			     /* this may adversely affect performance, as this number of x's are */
			     /* drawn EVERY FRAME when in close proximity to jammer. */

#define FRAME_RATE_HZ 30	/* target frame rate at which gtk callback fires by default */
int frame_rate_hz = FRAME_RATE_HZ; /* Actual frame rate, user adjustable. */
#define TERRAIN_LENGTH 1000	/* length, in number of line segments, of terrain */
#define SCREEN_WIDTH 800        /* window width, in pixels */
#define SCREEN_HEIGHT 600       /* window height, in pixels */
#define WORLDWIDTH (SCREEN_WIDTH * 40)  /* width of world, 40 screens wide. */
#define KERNEL_Y_BOUNDARY (-1000) /* basically, an arbitrary altitude limit on movement of player. */

#define VOLCANO_XFRACTION (15)	 /* volcano x pos is 1/VOLCANO_XFRACTION of the way across the world. */

#define LARGE_SCALE_ROUGHNESS (0.04)   /* limits roughness, on large scale, of fractal terrain algorithm */
#define SMALL_SCALE_ROUGHNESS (0.09)   /* limits roughtness, on small scale, of fractal terrain algorithm */
#define MAXOBJS 8500		       /* arbitrary, maximum number of objects in game, ~4000 are commonly seen. */


					/* object allocation algorithm uses a bit per object to indicate */
					/* free/allocated... how many 32 bit blocks do we need?  NBITBLOCKS. */
#define NBITBLOCKS ((MAXOBJS >> 5) + 1)  /* 5, 2^5 = 32, 32 bits per int. */


#define LASER_BOLT_DAMAGE 5 		/* damage done by flak guns to player */
#define PLAYER_LASER_DAMAGE 20		/* damage done by player's laser (to blimps, clipper ships */

#define NFLAK 10			/* Number of flak guns (laser turrets) */
#define NKGUNS 30			/* Number of Kernel defense guns (laser turrets) */
#define NROCKETS 20 			/* Number of rockets sprinkled into the terrain */ 
#define LAUNCH_DIST 1200			/* How close player can get in x dimension before rocket launches */
#define MAX_ROCKET_SPEED -32		/* max vertical speed of rocket */
#define SAM_LAUNCH_DIST 400		/* How close player can get in x deminsion before SAM might launch */
#define GDB_LAUNCH_DIST 700		/* How close player can get in x deminsion before GDB might launch */
#define SAM_LAUNCH_CHANCE 15 		/* chances out of 100 if in ranch, missile will be launched */
#define PLAYER_SPEED 8 			/* max X player speed, in pixels per frame */
#define MAX_VX 15			/* Hmm... another max X player speed?  a bug, I think. */
#define MAX_VY 25			/* Max player y speed, pixels per frame. */
#define LASER_FIRE_CHANCE 20		/* INITIAL chance/1000 that flak guns (laser turrets) will fire if in range */
#define LASERLEAD (11)			/* How many pixels left/right to lead the player in aiming flak guns */	
#define LASER_SPEED 50			/* Speed of player's laser beams, pixels/frame */
#define LASER_PROXIMITY 300
#define LASER_Y_PROXIMITY 8
#define BOMB_PROXIMITY 10000 /* square root of 30000, how close bomb has to be to be considered a "hit", squared. */
#define BOMB_X_PROXIMITY 100 /* X proximity to hit, for bombs which impact the ground. */

#define CRON_SHOT_CHANCE 40  /* chance out of 100 that cron jobs take a shot if in range, and timer % 10 == 0 */
#define CRON_SHOT_DIST_SQR (300*300)  /* 300 pixels dist for cron to consider player "in range" */
#define CRON_DX_THRESHOLD 14  /* dist must be <= this many pixels for cron to consider himself to have */
#define CRON_DY_THRESHOLD 14  /* reached his destination. */
#define CRON_MAX_VX 8 	      /* max x and y velocities of cron jobs */
#define CRON_MAX_VY 5 

#define GDB_DX_THRESHOLD 25  /* dist must be <= this many pixels for gdb to consider himself to have */
#define GDB_DY_THRESHOLD 25  /* reached it's destination.  It's large, so they won't just sit on the */
			      /* player's face. */
#define GDB_MAX_VX 13 	      /* max x and y velocities of GDBs. */
#define GDB_MAX_VY 13 


#define NBUILDINGS 15 		/* initial number of buildings on the terrain */
#define NBRIDGES 2		/* max initial number of bridges in terrain (less, if no valleys) */
#define MAXBUILDING_WIDTH 9	/* max building width, in terrain line segments */
#define NFUELTANKS 20		/* Initial number of fuel tanks sprinkled around the terrain */
#define FUELTANK_CAPACITY 30	/* How many hit points a fuel tank contains */
#define REFUEL_RATE 1 		/* lower numbers = faster, used as modulo of timer */
#define REFILL_RATE (frame_rate_hz * 3) /* lower numbers == faster, used as modulo of timer */
#define NJAMMERS 1		/* Initial number of radar jammers sprinkled through the terrain */
#define NCRON 15 		/* Initial number of cron jobs sprinkeled through the terrain */
#define NSHIPS 1		/* Initial number of clipper ships sprinkeled through the terrain */
#define NGDBS 3 		/* Initial number of gdbs sprinkeled through the terrain */
#define NOCTOPI 0 		/* Initial number of octopi sprinkled through the terrain */
#define NTENTACLES 2 		/* Initial number of tentacles sprinkled through the terrain */
#define NSAMS 3			/* Initial number of SAM stations sprinkled through the terrain */
#define BOMB_SPEED 10		/* differential x velocity of bomb as it leaves player's ship */
#define NBOMBS 100		/* Number of bombs allocated to player at beginning of levels */

#define MAX_ALT 100		/* for "attract mode", max altitude above ground player flies. */
#define MIN_ALT 50		/* for "attract mode", min altitude above ground player flies. */

#define MAXHEALTH 100		/* Max, and initial health value of player */
#define RADAR_FRITZ_HEALTH 30   /* If player's health drops below this, the radar goes on the fritz */
#define NAIRSHIPS 1		/* Initial number of blimps sprinkled through the terrain */
#define NBALLOONS 2 		/* Initial number of balloons sprinkled through the terrain */
#define NWORMS 2		/* Initial number of worms sprinkled through the terrain */
#define MAX_WORM_VX 8
#define MAX_WORM_VY MAX_WORM_VX 

#define MAX_BALLOON_HEIGHT 300  /* these two control blimps, balloons, and clipper ships */
#define MIN_BALLOON_HEIGHT 50   /* limiting their altitude to a range above the ground (pixels) */

#define MAX_MISSILE_VELOCITY 19 /* Max x and y missile/harpoon velocities */
#define MISSILE_DAMAGE 20	/* amount of damage missile/harpoon inflict on player */
#define MISSILE_PROXIMITY 10	/* x and y proximity for missiles to be considered a hit (pixels) */
#define MISSILE_FIRE_PERIOD (frame_rate_hz * 1)	/* time which must elapse between firings of missiles */
#define WORM_DAMAGE 5

#define BULLET_SPEED 25		/* max x/y velocities of bullets (cron jobs shoot) */
#define BULLET_DAMAGE 20	/* amount of damage bullets inflict on player */
#define BULLET_PROXIMITY 10	/* x and y proximity for bullet to be considered a hit (pixels) */
#define BULLET_LEAD_TIME 15	/* pixels per unit of velocity to lead player while aiming bullets */

#define HUMANOID_PICKUP_SCORE 1000 /* score for picking up a .swp file */
#define HUMANOID_DIST 15	   /* proximity to pick up a .swp file */

#define MAX_TENTACLE_SEGS 40	   /* max number of segments of a tentacle */
#define MAX_SEG_ANGLE 60	   /* max angle a tentacle may bend with the one next to it */
				/* macro to compute initial tentacle angles */
#define TENTACLE_RANGE(t) (randomn(t.upper_angle - t.lower_angle) + t.lower_angle)

/* Scoring stuff */

#define ROCKET_SCORE 200	/* score for killing a rocket */
#define BRIDGE_SCORE 10 	/* score for moving a piece of a bridge (zillions of them, so should be small) */
#define FLAK_SCORE 250		/* score for killing a flak gun (laser turret) */
#define OCTOPUS_SCORE 800 	/* score for killing an octopus */
#define SAM_SCORE 400		/* score for killing a SAM station. */
#define GDB_SCORE 400		/* score for killing a GDB */
#define CRON_SCORE 400		/* score for killing a cron job */
#define TENTACLE_SCORE 100

#define GROUND_OOPS 64000	/* There are some functions which, given an x value, */
				/* return the corresponding y value for the terrain */
				/* at that x value.  For some values of x, there is */
				/* no y.  The value returned in that case is GROUND_OOPS */
				/* The functions are: find_ground_level() and */
				/* approximate_horizon() */

/* some globals... maybe should be in game_state */
int game_pause = 0;		/* is game paused? */
int attract_mode = 0;		/* is game in attract mode */
int credits = 0;		/* how many quarters have been put in, but not played? */
// int toggle = 0;		
//
int total_radar_noise;		/* count of how much radar noise has been drawn in a given frame */
				/* This is to limit things, so if there are 10 radar jammers in */
				/* close range, we don't draw 10x the amount of noise necessary */
				/* to obscure things. If the radar is totally obscured, obscuring */
				/* it 10 more times is wasting cpu. */

int timer = 0;			/* monotonically increases by 1 with every frame */
int next_timer = 0;		/* when timer reaches this value, timer_expired() is called */
int timer_event = 0;		/* timer_expired() switches on this value... */ 

/* timer_event values. */
#define BLINK_EVENT 1
#define READY_EVENT 2
#define SET_EVENT 3
#define GO_EVENT 4
#define BLANK_EVENT 5
#define GAME_OVER_EVENT 6
#define BLANK_GAME_OVER_1_EVENT 7
#define INSERT_COIN_EVENT 8
#define BLANK_GAME_OVER_2_EVENT 9
#define GAME_ENDED_EVENT 10
#define GAME_ENDED_EVENT_2 11
#define CREDITS1_EVENT 12
#define CREDITS2_EVENT 13
#define INTRO1_EVENT 14
#define INTRO2_EVENT 15
#define START_INTERMISSION_EVENT 16
#define END_INTERMISSION_EVENT 17
#define KEYS1_EVENT 18
#define KEYS2_EVENT 19

int sound_working = 0;
int brightsparks = 0;		/* controls preference for how to draw sparks */
int nframes = 0;		/* count of total frames drawn, used for calculating actual frame rate */
struct timeval start_time, end_time;		/* start and end time of game, for calc'ing frame rate */

unsigned int free_obj_bitmap[NBITBLOCKS] = {0}; /* bitmaps for object allocater free/allocated status */
int jsfd = -1;

#define NCOLORS 10		/* number of "cardinal" colors */
#define NSPARKCOLORS 25 	/* 25 shades from yellow to red for the sparks */
#define NRAINBOWSTEPS (16)
#define NRAINBOWCOLORS (NRAINBOWSTEPS*3)

GdkColor huex[NCOLORS + NSPARKCOLORS + NRAINBOWCOLORS]; /* all the colors we have to work with are in here */
GdkColor *sparkcolor;		/* a pointer into the huex[] array where the spark colors begin */
GdkColor *rainbow_color;	/* a pointer into the huex[] array where the rainbow colors begin */

/* cardinal color indexes into huex array */
#define WHITE 0
#define BLUE 1
#define BLACK 2
#define GREEN 3
#define YELLOW 4
#define RED 5
#define ORANGE 6
#define CYAN 7
#define MAGENTA 8
#define DARKGREEN 9

int planet_color[] = {
	RED, GREEN, YELLOW, ORANGE, MAGENTA, CYAN
};


/* Object types, just arbitrary constants used to uniquely id object types */
#define OBJ_TYPE_AIRSHIP 'a'
#define OBJ_TYPE_BOMB 'p'
#define OBJ_TYPE_BALLOON 'B'
#define OBJ_TYPE_BUILDING 'b'
#define OBJ_TYPE_CHAFF 'c'
#define OBJ_TYPE_CRON 'C'
#define OBJ_TYPE_FUEL 'f'
#define OBJ_TYPE_SHIP 'w'
#define OBJ_TYPE_GUN 'g'
#define OBJ_TYPE_HUMAN 'h'
#define OBJ_TYPE_LASER 'L'
#define OBJ_TYPE_MISSILE 'm'
#define OBJ_TYPE_HARPOON 'H'
#define OBJ_TYPE_ROCKET 'r'
#define OBJ_TYPE_SOCKET 'x'
#define OBJ_TYPE_SAM_STATION 'S'
#define OBJ_TYPE_SPARK 's'
#define OBJ_TYPE_BRIDGE 'T'
#define OBJ_TYPE_SYMBOL 'z'
#define OBJ_TYPE_GDB 'd'
#define OBJ_TYPE_OCTOPUS 'o'
#define OBJ_TYPE_TENTACLE 'j'
#define OBJ_TYPE_FLOATING_MESSAGE 'M'
#define OBJ_TYPE_BULLET '>'
#define OBJ_TYPE_PLAYER '1'
#define OBJ_TYPE_DEBRIS 'D'
#define OBJ_TYPE_JAMMER 'J'
#define OBJ_TYPE_VOLCANO 'v'
#define OBJ_TYPE_WORM 'W'
#define OBJ_TYPE_KGUN 'k'
#define OBJ_TYPE_TRUSS 't'

int current_level = 0;		/* current level of the game, starts at zero */
struct level_parameters_t {
	int random_seed;	/* so the games always have the same terrain, setup, etc. */

	/* numbers of various objects on a given level */
	int nrockets;
	int nbridges;
	int nflak;
	int nfueltanks;
	int njammers;
	int ncron;
	int nships;
	int ngdbs;
	int noctopi;
	int ntentacles;
	int nsams;
	int nhumanoids;
	int nbuildings;
	int nbombs;
	int nairships;
	int nworms;
	int nkguns;

	/* how often flak guns (laser turrets) fire. */
	int laser_fire_chance;

	/* how the terrain looks.  Starts out smooth, later levels more mountainous */
	double large_scale_roughness;
	double small_scale_roughness;
	int ground_color;
} level = {
	/* initial values */
	31415927, /* constant, so the games are all the same from play to play. */
	NROCKETS,
	NBRIDGES,
	NFLAK,
	NFUELTANKS,
	NJAMMERS,
	NCRON,
	NSHIPS,
	NGDBS,
	NOCTOPI,
	NTENTACLES,
	NSAMS,
	NHUMANOIDS,
	NBUILDINGS,
	NBOMBS,
	NAIRSHIPS,
	NWORMS,
	NKGUNS,
	LASER_FIRE_CHANCE,
	LARGE_SCALE_ROUGHNESS,
	SMALL_SCALE_ROUGHNESS,
	0,
};

/**** LETTERS and stuff *****/

/* special values to do with drawing shapes. */
#define LINE_BREAK (-999)
#define COLOR_CHANGE (-1000) /* note, color change can ONLY follow LINE_BREAK */

typedef unsigned char stroke_t;

/**************************

	Here's how this works, there's a 3x7 grid. on which the
	letters are drawn.  You list the sequence of strokes.
	Use 21 to lift the pen, 99 to mark the end.

	Inspired by Hofstadters' Creative Concepts and Creative Analogies.

                               letter 'A'    can be compactly represented by:

		0   1   2      *   *   *    { 6, 8, 14, 12, 9, 11, 99 }

                3   4   5      *   *   *

                6   7   8      *---*---*
                                       |
                9  10  11      *---*---*
                               |       |
               12  13  14      *---*---*

               15  16  17      *   *   *

               18  19  20      *   *   *
The grid numbers can be decoded into (x,y) coords like:
	x = ((n % 3) * xscale);
	y = (x/3) * yscale;     // truncating division.

	(not sure the above actually works)

	instead, use decode_glyph[] to get x and y values -- see below
***************************/
stroke_t glyph_Z[] = { 0, 2, 12, 14, 99 };
stroke_t glyph_Y[] = { 0, 7, 2, 21, 7, 13, 99 };
stroke_t glyph_X[] = { 0, 14, 21, 12, 2, 99 };
stroke_t glyph_W[] = { 0, 12, 10, 14, 2, 21, 10, 1, 99 };
stroke_t glyph_V[] = { 0, 13, 2, 99 };
stroke_t glyph_U[] = { 0, 9, 13, 11, 2, 99 };
stroke_t glyph_T[] = { 13, 1, 21, 0, 2, 99 };
stroke_t glyph_S[] = { 9, 13, 11, 3, 1, 5, 99 };
stroke_t glyph_R[] = { 12, 0, 1, 5, 7, 6, 21, 7, 14, 99 };
stroke_t glyph_Q[] = { 13, 9, 3, 1, 5, 11, 13, 21, 10, 14, 99 };
stroke_t glyph_P[] = { 12, 0, 1, 5, 7, 6, 99 };
stroke_t glyph_O[] = { 13, 9, 3, 1, 5, 11, 13, 99 };
stroke_t glyph_N[] = { 12, 0, 14, 2, 99 };
stroke_t glyph_M[] = { 12, 0, 4, 2, 14, 99 };
stroke_t glyph_L[] = { 0, 12, 14, 99};
stroke_t glyph_K[] = { 0, 12, 21, 6, 7, 11, 14, 21, 7, 5, 2, 99};
stroke_t glyph_J[] = { 9, 13, 11, 2, 99}; 
stroke_t glyph_I[] = { 12, 14, 21, 13, 1, 21, 0, 2, 99 }; 
stroke_t glyph_H[] = { 0, 12, 21, 2, 14, 21, 6, 8, 99 };
stroke_t glyph_G[] = { 7, 8, 11, 13, 9, 3, 1, 5, 99 };
stroke_t glyph_F[] = { 12, 0, 2, 21, 8, 7, 99 };
stroke_t glyph_E[] = { 14, 12, 0, 2, 21, 6, 7, 99 };
stroke_t glyph_D[] = { 12, 13, 11, 5, 1, 0, 12, 99 };
stroke_t glyph_C[] = { 11, 13, 9, 3, 1, 5, 99 };
stroke_t glyph_B[] = { 0, 12, 13, 11, 5, 1, 0, 21, 6, 8, 99 };
stroke_t glyph_A[] = { 12, 3, 0, 5, 14, 21, 8, 6, 99 };
stroke_t glyph_slash[] = { 12, 2, 99 };
stroke_t glyph_backslash[] = { 0, 14, 99 };
stroke_t glyph_pipe[] = { 1, 13, 99 };
stroke_t glyph_que[] = { 13, 10, 21, 7, 5, 2, 0, 3, 99 };
stroke_t glyph_bang[] = { 10, 13, 21, 1, 7, 99};
stroke_t glyph_colon[] = { 6, 7, 21, 12, 13, 99 };
stroke_t glyph_leftparen[] = { 2, 4, 10, 14,99 };
stroke_t glyph_rightparen[] = { 0, 4, 10, 12,99 };
stroke_t glyph_space[] = { 99 };
stroke_t glyph_plus[] = { 6, 8, 21, 7, 13, 99 };
stroke_t glyph_dash[] = { 6, 8, 99 };
stroke_t glyph_0[] = { 12, 0, 2, 14, 12, 2, 99 };
stroke_t glyph_9[] = { 8, 6, 0, 2, 14, 99 };
stroke_t glyph_8[] = { 0, 2, 14, 12, 0, 21, 6, 8, 99 };
stroke_t glyph_7[] = { 0, 2, 12, 99 };
stroke_t glyph_6[] = { 2, 0, 12, 14, 8, 6, 99 };
stroke_t glyph_5[] = { 2, 0, 6, 8, 14, 12, 99 };
stroke_t glyph_4[] = { 0, 6, 8, 21, 2, 14, 99 };
stroke_t glyph_3[] = { 0, 2, 14, 12, 21, 6, 8, 99 };
stroke_t glyph_2[] = { 0, 2, 5, 9, 12, 14, 99 };
stroke_t glyph_1[] = { 1, 13, 99 };
stroke_t glyph_comma[] = { 13, 16, 99 };
stroke_t glyph_period[] = { 12, 13, 99 };
stroke_t glyph_z[] = { 6, 8, 12, 14, 99 };
stroke_t glyph_y[] = { 6, 12, 14, 21, 8, 17, 19, 18,  99};
stroke_t glyph_x[] = { 12, 8, 21, 6, 14, 99 };
stroke_t glyph_w[] = { 6, 12, 14, 8, 21, 7, 13, 99 };
stroke_t glyph_v[] = { 6, 13, 8, 99 };
stroke_t glyph_u[] = { 6, 12, 14, 8, 99 };
stroke_t glyph_t[] = { 14, 13, 4, 21, 6, 8, 99 };
stroke_t glyph_s[] = { 8, 6, 9, 11, 14,12, 99 };

stroke_t glyph_a[] = { 6, 8, 14, 12, 9, 11, 99 };
stroke_t glyph_b[] = { 0, 12, 14, 8, 6, 99 };
stroke_t glyph_c[] = { 8, 6, 12, 14, 99 };
stroke_t glyph_d[] = { 8, 6, 12, 14, 2, 99 };
stroke_t glyph_e[] = { 9, 11, 8, 6, 12, 14, 99 };
stroke_t glyph_f[] = { 13, 1, 2, 21, 6, 8, 99 };
stroke_t glyph_g[] = { 11, 12, 6, 8, 20, 18, 99 };
stroke_t glyph_h[] = { 0, 12, 21, 6, 8, 14, 99 };
stroke_t glyph_i[] = { 13, 7, 21, 4, 2, 99 };
stroke_t glyph_j[] = { 18, 16, 7, 21, 4, 2, 99 };
stroke_t glyph_k[] = { 0, 12, 21, 6, 7, 11, 14, 21, 7, 5, 99 };
stroke_t glyph_l[] = { 1, 13, 99 };
stroke_t glyph_m[] = { 12, 6, 8, 14, 21, 7, 13, 99 };
stroke_t glyph_n[] = { 12, 6, 7, 13, 99 };
stroke_t glyph_o[] = { 12, 6, 8, 14, 12, 99 };
stroke_t glyph_p[] = { 18, 6, 8, 14, 12, 99 };
stroke_t glyph_q[] = { 14, 12, 6, 8, 20, 99 };
stroke_t glyph_r[] = { 12, 6, 21, 9,7,8,99 };



struct target_t; /* linked list of possible target objects to */
		 /* avoid searching through vast majority of */
		 /* non-targetable objects (e.g., sparks) */

struct my_point_t {
	int x,y;
};

/* x and y offsets for decoding stroke_t's, above */
struct my_point_t decode_glyph[] = {
	{ 0, -4 },
	{ 1, -4 },
	{ 2, -4 },
	{ 0, -3 },
	{ 1, -3 },
	{ 2, -3 },
	{ 0, -2 },
	{ 1, -2 },
	{ 2, -2 },
	{ 0, -1 },
	{ 1, -1 },
	{ 2, -1 },
	{ 0, -0 },
	{ 1, -0 },
	{ 2, -0 },
	{ 0, 1 },
	{ 1, 1 },
	{ 2, 1 },
	{ 0, 2 },
	{ 1, 2 },
	{ 2, 2 },
	{ 0, 3 },
	{ 1, 3 },
	{ 2, 3 },
};
/**** end of LETTERS and stuff */

/* vxy_2_dxy is a 2d array which translates a vx and vy velocity vextor
 * into an x,y vector in the same direction but a fixed magnitude.  The array
 * caches precomputed values.  This is used for drawing the missiles.  If the
 * missiles are travelling at vx, vy, then this array allows us to compute x,y
 * offsets to draw the missile pointing in the right direction with just a 
 * table lookup.  See init_vxy_2_dxy() function, below. */
#define MAX_VELOCITY_TO_COMPUTE 31 
#define V_MAGNITUDE (20.0)
struct my_point_t vxy_2_dxy[MAX_VELOCITY_TO_COMPUTE+1][MAX_VELOCITY_TO_COMPUTE+1];

/* these hold cached values of sine and cosine functions for 0-360 degress */
double sine[361];
double cosine[361];

/* Below, arrays of points telling how to draw the various objects in */
/* the game. */

struct my_point_t kgun_points[] = {

	{ -10, -25 },
	{ -10, -5 },
	{ 10, -5 },
	{ 10, -25 },
	{ -10, -5 },
	{ -7,  5 },
	{ 7, 5 },
	{ 10, -5 },
	{ -10, -25 },
};

struct my_point_t inverted_kgun_points[] = {

	{ -10, 25 },
	{ -10, 5 },
	{ 10, 5 },
	{ 10, 25 },
	{ -10, 5 },
	{ -7,  -5 },
	{ 7, -5 },
	{ 10, 5 },
	{ -10, 25 },
};

struct my_point_t octopus_points[] = {
	{ -7, 0 },
	{ 7, 0 },
	{ 10, -5 },
	{ 15, -25 },
	{ 10, -30 },
	{ -10, -30 },
	{ -15, -25 },
	{ -10, -5 },
	{ -7, 0 },
	{ LINE_BREAK, LINE_BREAK },
	{ -8, -10 },
	{ -4, -7 },
	{ LINE_BREAK, LINE_BREAK },
	{ 8, -10 },
	{ 4, -7 }
};

struct my_point_t bullet_points[] = {
	{ -4, 0 },
	{ 0, -4 },
	{ 4, 0 },
	{ 0, 4 },
	{ -4, 0 },
};

struct my_point_t gdb_points_left[] = {
	{ 10, 0 },
	{ 5, 0 },
	{ -15, 5 },
	{ -20, 0 },
	{ -20, -5 },
	{ 0, -15, },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, GREEN },

	{ -20, -5 },
	{ -18, -12 },
	{ -10, -16 },
	{ -3, -15 },
	{ 0, -15, },

	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, CYAN },
	{ 0, -15, },
	{ 0, 15 },
	{ -5, 20 },
	{ -10, 20 },
	{ LINE_BREAK, LINE_BREAK },
	{ 15, -22 },
	{ 0, -15 },
	{ 0, 0 },
	{ 15, 0  },
	{ 15, -22  },
	{ 15, -45  },
	{ 18, -45  },
	{ 18, -22  },
	{ 18, 0  },
	{ 33, 0  },
	{ 33, -15  },
	{ 18, -22  },
};

struct my_point_t gdb_points_right[] = {
	{ 10, 0 },
	{ 5, 0 },
	{ -15, 5 },
	{ -20, 0 },
	{ -20, -5 },
	{ 0, -15, },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, GREEN },

	{ -20, -5 },
	{ -18, -12 },
	{ -10, -16 },
	{ -3, -15 },
	{ 0, -15, },

	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, CYAN },
	{ 0, -15, },
	{ 0, 15 },
	{ -5, 20 },
	{ -10, 20 },
	{ LINE_BREAK, LINE_BREAK },
	{ 15, -22 },
	{ 0, -15 },
	{ 0, 0 },
	{ 15, 0  },
	{ 15, -22  },
	{ 15, -45  },
	{ 18, -45  },
	{ 18, -22  },
	{ 18, 0  },
	{ 33, 0  },
	{ 33, -15  },
	{ 18, -22  },
};

struct my_point_t humanoid_points[] = {
	{ -1, 0 }, /* waist */
	{  1, 0 },
	{  4, -6 }, /* shoulders */
	{  -4, -6 },
	{ -1, 0 }, 
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, CYAN },
	{ -1, 0 }, /* left leg */
	{ -3, 4 },
	{ -3, 7 },
	{ LINE_BREAK, LINE_BREAK },
	{ 1, 0 }, /* right leg */
	{ 3, 4 },
	{ 3, 7 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, MAGENTA },
	{ 4, -6, }, /* right arm */
	{ 6, -3 },
	{ 6, 1 }, 
	{ LINE_BREAK, LINE_BREAK },
	{ -4, -6, }, /* left arm */
	{ -6, -3 },
	{ -6, 1 }, 
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, ORANGE },
	{ -1, -7 }, /* head */
	{ -1, -9 },
	{ 1, -9, },
	{ 1, -7 },
};

struct my_point_t SAM_station_points[] = {
	{ -5, 0 },   /* Bottom base */
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ -5, 0 },   /* Bottom base */
	{ -5, -10 },
	{ 5, 0 },
	{ 5, -10 },
	{ -5, 0 },
	{ 5, 0 }, 	
	{ LINE_BREAK, LINE_BREAK },
	{ -5, -10 },   /* middle base */
	{ -5, -20 },
	{ 5, -10 },
	{ 5, -20 },
	{ -5, -10 },
	{ 5, -10 }, 	
	{ LINE_BREAK, LINE_BREAK },
	{ -5, -20 },   /* Top base */
	{ -5, -30 },
	{ 5, -20 },
	{ 5, -30 },
	{ -5, -20 },
	{ 5, -20 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ -5, -30 },
	{ 5, -30 }, 	
	{ LINE_BREAK, LINE_BREAK },
	{ -3, -30 }, /* Base of radar */
	{ 0, -35, },
	{ 3, -30 },
	{ LINE_BREAK, LINE_BREAK },
	{ 0, -35, }, /* Radar dish */
	{ 10, -45 },
	{ 13, -59 },
	{ LINE_BREAK, LINE_BREAK },
	{ 0, -35, }, /* Radar dish */
	{ -10, -25, },
	{ -25, -22, },
	{ 13, -59 },
	{ LINE_BREAK, LINE_BREAK },
	{ 0, -35, }, /* Radar dish */
	{ -15, -50, }, /* Radar dish */
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ 20, 0 }, /* Little building */
	{ 20, -50 },
	{ 30, -50 },
	{ 30, 0 },

};

struct my_point_t airship_points[] = {
	{ -70, -50 }, /* tip of nose */
	{ -60, -60 },
	{ -50, -65 },
	{ -40, -67 },
	{ -30, -70 }, /* top left */
	{  30, -70 }, /* top right */
	{  40, -67 },
	{  50, -65 },
	{  60, -60 },
	{  70, -50 },
	{  LINE_BREAK, LINE_BREAK },
	/* Now the same shape, but with y displacement to make bottom of blimp */
	{ -70, -50 }, /* tip of nose */
	{ -60, -40 },
	{ -50, -35 },
	{ -40, -33 },
	{ -30, -30 }, /* bottom left */
	{  30, -30 }, /* bottom right */
	{  40, -33 },
	{  50, -35 },
	{  60, -40 },
	{  70, -50 }, /* back point */
	{  LINE_BREAK, LINE_BREAK },
	{  60, -60 }, /* top tail */
	{  70, -70 },
	{  90, -70 },
	{  90, -60 },
	{  70, -50 }, /* back point */
	{  90, -40 },
	{  90, -30 },
	{  70, -30 },
	{  60, -40 },
	{  LINE_BREAK, LINE_BREAK },  /* central tail fin */
	{  60, -50 } ,
	{  90, -50 },
	{  LINE_BREAK, LINE_BREAK },  /* Gondola */
	{  -10, -30 },
	{    -5, -20 },
	{  10, -20 },
	{  10, -30 },
};

struct my_point_t balloon_points[] = {
	{ 0, -100 },
	{ -20, -98 }, 
	{ -35, -90 },
	{ -45, -80 },
	{ -47, -70 },
	{ -47, -60 },
	{ -40, -50 },
	{ -8, -20 },
	{ -8, -10 },
	{  8, -10 },
	{  8, -20 },
	{  40, -50 }, 
	{  47, -60 },
	{  47, -70 },
	{  45, -80 },
	{  35, -90 },
	{  20, -98 }, 
	{ 0, -100 },
	{  LINE_BREAK, LINE_BREAK },  /* Gondola strings */
	{ -8, -10 },
	{ -5, 5, },
	{ 5, 5, },
	{ 8, -10 },
	{  LINE_BREAK, LINE_BREAK },  /* Gondola */
	{ -5, 5,},
	{ -5, 0,},
	{  5, 0,},
	{  5, 5,},
};


struct my_point_t spark_points[] = {
	{ -1,-1 },
	{ -1, 1},
	{ 1, 1 },
	{ 1, -1 },
	{ -1, -1 },
#if 0
	{ 0, 0 },
	{ 0, 10 },
	{ 10, 10 },
	{ 10, 0 },
	{ 0, 0 },
#endif
};

struct my_point_t cron_points[] = {
	{ -10, -15 },	
	{ 10, -15 },	
	{ 15, -8 },	
	{ 10, 0 },	
	{ -10, 0 },	
	{ -15, -8 },	
	{ -10, -15 },	
	{ LINE_BREAK, LINE_BREAK },
	{ 7, 0 },
	{ 7, 17 },
	{ 10, 30 },
	{ 7, 37 },
	{ LINE_BREAK, LINE_BREAK },
	{ -7, 0 },
	{ -7, 17 },
	{ -10, 30 },
	{ -7, 37 },
	{ LINE_BREAK, LINE_BREAK },
	{ 0, 0 },
	{ 0, 30 },
	{ LINE_BREAK, LINE_BREAK },
	{ 7, -15 },
	{ 7, -25 },
	{ LINE_BREAK, LINE_BREAK },
	{ 9, -15 },
	{ 9, -35 },
};

struct my_point_t jammer_points[] = {
	{ -10, 0 },
	{ 10, 0 },
	{ 10, -10 },
	{ 5, -10 },
	{ 3, -15 },
	{ -3, -15 },
	{ -5, -10 },
	{ -10, -10 },
	{ -10, 0 },
};

struct my_point_t fuel_points[] = {
	{ -30, -15 },

	{ -25, -18 }, /* top far curve */
	{ -20, -19 },
	{  20, -19 },
	{  25, -18 },

	{ 30, -15 },

	{  25, -12 }, /* top near curve */
	{  20, -11 },
	{ -20, -11 },
	{ -25, -12 },
	{ -30, -15 },

	{ LINE_BREAK, LINE_BREAK },

	{ 30, -15 },
	{ 30, 30 },
	
	{ 25, 33 }, /* bottom curve */
	{ 20, 34 },
	{ -20, 34 },
	{ -25, 33 },

	{ -30, 30 },
	{ -30, -15 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ -25, 25 },
	{ -25, -5 },
	{ -15, -5 },
	{ LINE_BREAK, LINE_BREAK },
	{ -25, 15 },
	{ -15, 15 },
	{ LINE_BREAK, LINE_BREAK },
	{ -10, -5 },
	{ -10, 25 },
	{ 0, 25 },
	{ 0, -5 },
	{ LINE_BREAK, LINE_BREAK },
	{ 15, -5 },
	{ 5, -5 },
	{ 5, 25 },
	{ 15, 25 },
	{ LINE_BREAK, LINE_BREAK },
	{ 5, 15 },
	{ 10, 15 },
	{ LINE_BREAK, LINE_BREAK },
	{ 18, -5 },
	{ 18, 25 },
	{ 25, 25 },
};

struct my_point_t right_laser_beam_points[] = {
	{ -100, 0 },
	{ -80, 0 },
	{ LINE_BREAK, LINE_BREAK },
	{ -75, 0 },
	{ -50, 0 },
	{ LINE_BREAK, LINE_BREAK },
	{ -45, 0 },
	{ 0, 0 },
};

struct my_point_t player_ship_points[] = {
	{ 9, 0 }, /* front of hatch */
	{ 0,0 },
	{ -3, -6 }, /* top of hatch */
	{ -12, -12 },
	{ -18, -12 },
	{ -15, -4 }, /* bottom of tail */
	{ -3, -4 }, /* bottom of tail */
	{ -15, -4 }, /* bottom of tail */
	{ -15, 3 }, /* back top of wing */
	{ 0, 3 }, /* back top of wing */
	{ -15, 3 }, /* back top of wing */
	{ -18, 9 }, /* back bottom of wing */
	{ -12, 9 }, /* back front of wing */
	{ 0, 3 },
	{ -6, 6 },
	{ 20, 4 },
	{ 24, 2 }, /* tip of nose */
	{ 9, 0 }, /* front of hatch */
	{ -3, -6 }, /* top of hatch */ 
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ 20, 2 },
	{ -17, 2 }, /* red stripe */
#if 0
	{ LINE_BREAK, LINE_BREAK }, /* Just for testing */
	{ -30, -20 },
	{ 30, -20 },
	{ 30, 20 },
	{ -30, 20 },
	{ -30, -20 },
#endif
};

struct my_point_t socket_points[] = {
	{ 9, 0 }, /* front of hatch */
	{ -3, -6 }, /* top of hatch */
	{ -12, -12 },
	{ -18, -12 },
	{ -15, -4 }, /* bottom of tail */
	// { -3, -4 }, /* bottom of tail */
	// { -15, -4 }, /* bottom of tail */
	{ -15, 3 }, /* back top of wing */
	//{ 0, 3 }, /* back top of wing */
	//{ -15, 3 }, /* back top of wing */
	{ -18, 9 }, /* back bottom of wing */
	{ -12, 9 }, /* back front of wing */
	// { LINE_BREAK, LINE_BREAK },
	// { 0, 3 },
	{ -6, 6 },
	{ 20, 4 },
	{ 24, 2 }, /* tip of nose */
	{ 9, 0 }, /* front of hatch */
	{ LINE_BREAK, LINE_BREAK },
	{ -30, -25 },
	{ 30, -25 },
	{ 30, 25 },
	{ -30, 25 },
	{ -30, -25 },
	// { -3, -6 }, /* top of hatch */ 
#if 0
	{ LINE_BREAK, LINE_BREAK }, /* Just for testing */
	{ -30, -20 },
	{ 30, -20 },
	{ 30, 20 },
	{ -30, 20 },
	{ -30, -20 },
#endif
};

struct my_point_t left_player_ship_points[] = {
	/* Data is reversed algorithmically */
	{ 9, 0 }, /* front of hatch */
	{ 0,0 },
	{ -3, -6 }, /* top of hatch */
	{ -12, -12 },
	{ -18, -12 },
	{ -15, -4 }, /* bottom of tail */
	{ -3, -4 }, /* bottom of tail */
	{ -15, -4 }, /* bottom of tail */
	{ -15, 3 }, /* back top of wing */
	{ 0, 3 }, /* back top of wing */
	{ -15, 3 }, /* back top of wing */
	{ -18, 9 }, /* back bottom of wing */
	{ -12, 9 }, /* back front of wing */
	{ 0, 3 },
	{ -6, 6 },
	{ 20, 4 },
	{ 24, 2 }, /* tip of nose */
	{ 9, 0 }, /* front of hatch */
	{ -3, -6 }, /* top of hatch */ 
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ 20, 2 },
	{ -17, 2 }, /* red stripe */
	
#if 0
	{ LINE_BREAK, LINE_BREAK }, /* Just for testing */
	{ -30, -20 },
	{ 30, -20 },
	{ 30, 20 },
	{ -30, 20 },
	{ -30, -20 },
#endif
};

struct my_point_t bomb_points[] = {
	{ -2, -6 },
	{ 2, -6 },
	{ 2, -4 },
	{ 0, -3 },
	{ -2, -4 },
	{ -2, -6 },
	{ LINE_BREAK, LINE_BREAK },
	{ 0, -3 },
	{ 2, -1 },
	{ 2, 8 },
	{ 0, 9 },
	{ -2, 8 },
	{ -2, -1 },
	{ 0, -3 },
};

struct my_point_t rocket_points[] = {
	{ -2, 3 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ -2, 3 },
	{ -4, 7 },
	{ -2, 7 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ -2, 7 },
	{ -2, -14 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ -2, -14 },
	{ 0, -20 },
	{ 2, -14 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ 2, -14 },
	{ 2, 7},
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ 2, 7},
	{ 4, 7},
	{ 2, 3}, 
};

struct my_point_t bridge_points[] = {  /* square with an x through it, 8x8 */
	{ -4, -4 },
	{ -4, 4 },
	{ 4, 4 },
	{ -4, -4 },
	{ 4, -4 },
	{ -4, 4 },
	{ LINE_BREAK, LINE_BREAK },
	{ 4, -4 },
	{ 4, 4 },
};

struct my_point_t ships_hull_points[] = {
	/* bowsprit */
	{ -60, -14 },
	{ -15, -10 },
	{  -15, -5 },
	{   21, -5 },
	{   25, -13 },
	{  60, -15 },
	{  55, 0 },
	{  51, -1 },
	{  51, 20 },
	{  -12, 15 },
	{  -27, 0 },
	{  -30, -7 },
	{  -60, -14 }, 	
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, GREEN }, /* wine bottle */
	{ -135, -120 }, 
	{ 110, -120 }, 
	{ 110, 23 }, 
	{ -135, 23 }, 
	{ -160, 5 }, 
	{ -175, -25 }, 
	{ -300, -25 }, 
	{ -300, -65 }, 
	{ -175, -65 }, 
	{ -160, -102 }, 
	{ -135, -120 }, 
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, ORANGE },
	{ 51, 15 },
	{ -12, 15 }, /* keel */
	{ LINE_BREAK, LINE_BREAK },
	{ 5, -5 },
	{ 8, -110 }, /* main mast */
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, RED },
	{ 8, -110 }, /* main mast */
	{ 0, -108 }, /* flag */
	{ 8, -105 },
	
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, ORANGE },
	{ 39, -14 }, /* aft mast */
	{ 41, -95 },
	{ LINE_BREAK, LINE_BREAK },
	{ -22, -12 }, /* fore mast */
	{ -20, -83 },
	{ LINE_BREAK, LINE_BREAK },
	{ COLOR_CHANGE, WHITE },
	{ -20, -83 },
	{ -60, -14 }, /* fore sail */
	{ -30, -25 },
	{ -20, -83 },
	{ 8, -100 }, /* lines on top */
	{ 41, -90 },
	{ 60, -70 },
	{ LINE_BREAK, LINE_BREAK },
	{ 39, -20 }, /* aft boom */
	{ 75, -20 },
	{ LINE_BREAK, LINE_BREAK },
	{ 39, -60 }, /* upper aft boom */
	{ 60, -70 },
	{ 75, -20 },
	{ LINE_BREAK, LINE_BREAK },
	{ -30, -75 }, /* foretopsail */
	{ -14, -75 },
	{ -17, -68 },
	{ -14, -58 },
	{ -27, -61 },
	{ -30, -58 },
	{ -32, -65 },
	{ -30, -75 },  
	{ LINE_BREAK, LINE_BREAK },
	{ -30, -58 }, /* foremidsail */
	{ -14, -58 },
	{ -14, -49 },
	{ -10, -40 },
	{ -24, -45 },
	{ -35, -40 },
	{ -36, -47 },
	{ -30, -58 },
	{ LINE_BREAK, LINE_BREAK },
	{ -35, -40 }, /* forelowersail */
	{ -10, -40 },
	{ -10, -26 },
	{ -5, -12 },
	{ -25, -17 },
	{ -39, -12 },
	{ -40, -22 },
	{ -35, -40 },
	{ LINE_BREAK, LINE_BREAK },
	{ -5, -100 }, /* main top sail */
	{  20, -100 },
	{  20, -95 },
	{  25, -80 },
	{  20, -83 },
	{  -6, -83 },
	{ -10, -80 },
	{ -10, -85 },
	{ -5, -100 },
	{ LINE_BREAK, LINE_BREAK },
	{  25, -80 }, /* 2nd main sail */
	{ -10, -80 },
	{ -15, -63 },
	{ -15,  -57 },
	{ -10,  -60 },
	{  25,  -60 },
	{  30,  -57 },
	{  25,  -75 },
	{  25, -80 },
	{ LINE_BREAK, LINE_BREAK },
	{ -15,  -57 }, /* 3rd main sail */
	{  30,  -57 },
	{  30,  -50 },
	{  33,  -37 },
	{  28,  -40 },
	{  -13,  -40 },
	{  -18,  -37 },
	{  -18,  -43 },
	{ -15,  -57 },
	{ LINE_BREAK, LINE_BREAK },
	{  33,  -37 }, /* 4th main sail */
	{  -18,  -37 },
	{  -20,  -18 },
	{  -20,  -13 },
	{  -15,  -17 },
	{  30,  -17 },
	{  35,  -13 },
	{  33,  -32 },
	{  33,  -37 },
	{ LINE_BREAK, LINE_BREAK },
	{  28,  -90 }, /* aft top sail */
	{  52,  -90 },
	{  52,  -85 },
	{  55,  -70 },
	{  50,  -73 },
	{  30,  -73 },
	{  25,  -70 },
	{  25,  -75 },
	{  28,  -90 },
	{ LINE_BREAK, LINE_BREAK },
	{  55,  -70 },
	{  25,  -70 },
	{  22,  -55 },
	{  22,  -50 },
	{  25,  -53 },
	{  52,  -53 },
	{  57,  -50 },
	{  55,  -65 },
	{  55,  -70 },
	{ LINE_BREAK, LINE_BREAK },
	{  22,  -50 },
	{  57,  -50 },
	{  57,  -45 },
	{  60,  -30 },
	{  55,  -33 },
	{  25,  -33 },
	{  20,  -30 },
	{  20,  -35 },
	{  22,  -50 },
	
};

/* These debris things will get filled in procedurally */
/* these drawings are used when something explodes, to add */
/* flying debris objects into the explosion. See make_debris_forms().  */
#define NDEBRIS_FORMS 25
struct my_point_t *debris_point[NDEBRIS_FORMS] = { 0 };
struct my_vect_obj *debris_vect[NDEBRIS_FORMS] = { 0 };

/* Just a bunch of random snippets of lisp code for the emacs */
/* blimps to spew out into core memory space as "memory leaks" */
/* wish I could find some humorous lisp code, but, */
/* who the hell would even recognize it? */
char randomlisp[] = "(let ((arg (if (consp (car args)) (caar args) (car args)))"
	"(fcs (if (consp (car args)) (cdr (car args)) nil)))"
	"(cond ((= chr ?s)"
	"(if addr (concat (car addr) \" <\" (cdr addr) \">\")"
	"(or (and (boundp 'report-xemacs-bug-beta-address)"
	"report-xemacs-bug-beta-address)"
	"(if (eq (car-safe calc-last-kill) (car kill-ring-yank-pointer))"
	"(cdr calc-last-kill)"
	"(if (stringp (car kill-ring-yank-pointer))"
	"(list 'extern"
	"(semantic-bovinate-from-nonterminal-full" /* semantic-bovinate? */
	" (car (nth 2 vals)) (cdr (nth 2 vals)) 'extern-c-contents)" ")))"
	"   (if value (list (car value) (cdr value))))"
	":value-to-external (lambda (widget value)"
	"(if value (append (list (car value)) (cadr value)))))";

/* Just a grouping of arrays of points with the number of points in the array */
struct my_vect_obj {
	int npoints;
	struct my_point_t *p;	
};

/* contains instructions on how to draw all the objects */
struct my_vect_obj player_vect;
struct my_vect_obj left_player_vect;
struct my_vect_obj rocket_vect;
struct my_vect_obj spark_vect;
struct my_vect_obj right_laser_vect;
struct my_vect_obj fuel_vect;
struct my_vect_obj jammer_vect;
struct my_vect_obj cron_vect;
struct my_vect_obj ship_vect;
struct my_vect_obj bomb_vect;
struct my_vect_obj bridge_vect;
struct my_vect_obj kgun_vect;
struct my_vect_obj inverted_kgun_vect;
struct my_vect_obj airship_vect;
struct my_vect_obj balloon_vect;
struct my_vect_obj SAM_station_vect;
struct my_vect_obj humanoid_vect;
struct my_vect_obj socket_vect;
struct my_vect_obj bullet_vect;
struct my_vect_obj gdb_vect_right;
struct my_vect_obj gdb_vect_left;
struct my_vect_obj octopus_vect;
struct my_vect_obj sail_segment;
struct my_vect_obj ships_hull;

/* There are 4 home-made "fonts" in the game, all the same "typeface", but 
 * different sizes */
struct my_vect_obj **gamefont[4];
/* indexes into the gamefont array */
#define BIG_FONT 0
#define SMALL_FONT 1
#define TINY_FONT 2
#define NANO_FONT 3

/* sizes of the fonts... in arbitrary units */
#define BIG_FONT_SCALE 14 
#define SMALL_FONT_SCALE 5 
#define TINY_FONT_SCALE 3 
#define NANO_FONT_SCALE 2 

/* spacing of letters between the fonts, pixels */
#define BIG_LETTER_SPACING (10)
#define SMALL_LETTER_SPACING (5)
#define TINY_LETTER_SPACING (3)
#define NANO_LETTER_SPACING (2)

/* max text lines that fit on the screen, used in attract mode */
#define MAXTEXTLINES 20
/* State of the game text printing system for the attract mode */
int current_color = WHITE;
int current_font = BIG_FONT;
int cursorx = 0;
int cursory = 0;
int livecursorx = 0;
int livecursory = 0;

/* for getting at the font scales and letter spacings, given  only font numbers */
int font_scale[] = { BIG_FONT_SCALE, SMALL_FONT_SCALE, TINY_FONT_SCALE, NANO_FONT_SCALE };
int letter_spacing[] = { BIG_LETTER_SPACING, SMALL_LETTER_SPACING, TINY_LETTER_SPACING, NANO_LETTER_SPACING };

/* more stuff for the attract mode's idea of how the "text screen" works. */
int ntextlines = 0;
struct text_line_t {
	int x, y, font;
	char string[80];
} textline[20];

#define FINAL_MSG1 "Where is your"
#define FINAL_MSG2 "editor now???"

/* text line entries are just fixed... */
#define GAME_OVER 1
#define CREDITS 0

typedef void line_drawing_function(GdkDrawable *drawable,
	 GdkGC *gc, gint x1, gint y1, gint x2, gint y2);

typedef void rectangle_drawing_function(GdkDrawable *drawable,
	GdkGC *gc, gboolean filled, gint x, gint y, gint width, gint height);

line_drawing_function *current_draw_line = gdk_draw_line;
rectangle_drawing_function *current_draw_rectangle = gdk_draw_rectangle;

/* I can switch out the line drawing function with these macros */
/* in case I come across something faster than gdk_draw_line */
#define DEFAULT_LINE_STYLE current_draw_line
#define DEFAULT_RECTANGLE_STYLE current_draw_rectangle

// this is neat, but too much of a performance hit... esp.
// for the radar noise.
#define wwvi_draw_line DEFAULT_LINE_STYLE
#define wwvi_draw_rectangle DEFAULT_RECTANGLE_STYLE

// #define wwvi_draw_line gdk_draw_line
// #define wwvi_draw_rectangle gdk_draw_rectangle

float xscale_screen;
float yscale_screen;
int real_screen_width;
int real_screen_height;

void scaled_line(GdkDrawable *drawable,
	GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
	gdk_draw_line(drawable, gc, x1*xscale_screen, y1*yscale_screen,
		x2*xscale_screen, y2*yscale_screen);
}

void scaled_rectangle(GdkDrawable *drawable,
	GdkGC *gc, gboolean filled, gint x, gint y, gint width, gint height)
{
	gdk_draw_rectangle(drawable, gc, filled, x*xscale_screen, y*yscale_screen,
		width*xscale_screen, height*yscale_screen);
}

void crazy_line(GdkDrawable *drawable,
	GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
	int angle;

	int x3,y3, x4,y4;
#if 1
	angle = timer % 360;

	x3 = ((x1 - (SCREEN_WIDTH/2)) * sine[angle]) + SCREEN_WIDTH/2;
	x4 = ((x2 - (SCREEN_WIDTH/2)) * sine[angle]) + SCREEN_WIDTH/2;
	y3 = ((y1 - (SCREEN_HEIGHT/2)) * cosine[angle]) + SCREEN_HEIGHT/2;
	y4 = ((y2 - (SCREEN_HEIGHT/2)) * cosine[angle]) + SCREEN_HEIGHT/2;
	gdk_draw_line(drawable, gc, x3, y3, x4, y4);	
#else
	double size;
	size = (timer % 300) + 100;
	size = size / 100.0;
	x3 = ((x1 - (SCREEN_WIDTH/2)) / size) + SCREEN_WIDTH/2;
	x4 = ((x2 - (SCREEN_WIDTH/2)) / size) + SCREEN_WIDTH/2;
	y3 = ((y1 - (SCREEN_HEIGHT/2)) / size) + SCREEN_HEIGHT/2;
	y4 = ((y2 - (SCREEN_HEIGHT/2)) / size) + SCREEN_HEIGHT/2;
	gdk_draw_line(drawable, gc, x3, y3, x4, y4);	
#endif
	// gdk_draw_line(drawable, gc, x1, y1, x2, y2);	

}

void crazy_rectangle(GdkDrawable *drawable,
	GdkGC *gc, gboolean filled, gint x, gint y, gint width, gint height)
{
	int x3, y3, x4, y4;

	x3 = x;
	y3 = y;
	x4 = x + width;
	y4 = y + height;

	crazy_line(drawable, gc, x3, y3, x4, y3);
	crazy_line(drawable, gc, x3, y4, x4, y4);
	crazy_line(drawable, gc, x3, y3, x3, y4);
	crazy_line(drawable, gc, x4, y3, x4, y4);
}

void init_vxy_2_dxy()
{
	int x, y;
	double dx, dy, angle;

	/* Given a velocity, (vx, vy), precompute offsets */
	/* for a fixed magnitude */

	for (x=0;x<=MAX_VELOCITY_TO_COMPUTE;x++) {
		for (y=0;y<=MAX_VELOCITY_TO_COMPUTE;y++) {
			if (x == 0) {
				vxy_2_dxy[x][y].y = 
					((double) -y / (double) MAX_VELOCITY_TO_COMPUTE) * V_MAGNITUDE;
				vxy_2_dxy[x][y].x = 0;
				continue;
			}
			dx = x;
			dy = y;
			angle = atan(dy/dx);
			dx = cos(angle) * V_MAGNITUDE;
			dy = -sin(angle) * V_MAGNITUDE;
			vxy_2_dxy[x][y].x = (int) dx;
			vxy_2_dxy[x][y].y = (int) dy;
		}
	}
}

/* These two functions use the vxy_2_dxy array, but correct for negative x or y */
static inline int dx_from_vxy(int vx, int vy) 
{
	if ((abs(vx) > MAX_VELOCITY_TO_COMPUTE) || (abs(vy) > MAX_VELOCITY_TO_COMPUTE))
		return 0;
	return (vx < 0) ?  -vxy_2_dxy[abs(vx)][abs(vy)].x : vxy_2_dxy[abs(vx)][abs(vy)].x;
}

static inline int dy_from_vxy(int vx, int vy) 
{
	if ((abs(vx) > MAX_VELOCITY_TO_COMPUTE) || (abs(vy) > MAX_VELOCITY_TO_COMPUTE))
		return 0;
	return (vy < 0) ?  -vxy_2_dxy[abs(vx)][abs(vy)].y : vxy_2_dxy[abs(vx)][abs(vy)].y;
}


/* attract mode text screen related functions */
void set_font(int fontnumber)
{
	current_font = fontnumber;
}

void gotoxy(int x, int y)
{
	cursorx = (x+1) * font_scale[current_font]*3;
	cursory = (y+1) * font_scale[current_font]*7;
}

void cleartext()
{
	ntextlines = 0;
}

void gameprint(char *s)
{
	int n;

	/* printf("Printing '%s'\n", s); */
	n = ntextlines;
	if (n>=MAXTEXTLINES)
		n = 0;
	textline[n].x = cursorx;
	textline[n].y = cursory;
	textline[n].font = current_font;
	strcpy(textline[n].string, s);
	ntextlines++;
	if (ntextlines >=MAXTEXTLINES)
		ntextlines = MAXTEXTLINES-1;
}

int current_font_scale = BIG_FONT_SCALE;

/* every object in the game is a game_obj_t, defined below... */
struct game_obj_t;

/* some function pointers which game_obj_t's may have */
typedef void obj_move_func(struct game_obj_t *o);		/* moves and object, called once per frame */
typedef void obj_draw_func(struct game_obj_t *o, GtkWidget *w); /* draws object, called 1/frame, if onscreen */
typedef void obj_destroy_func(struct game_obj_t *o);		/* called when an object is killed */

/* Various object type specific data structures are defined below. */
/* these are all put into a union in the game_obj_t structure. */

struct extra_player_data {  /* these are used when drawing the player zooming into the game */
	int count;	    /* at the beginning of levels. */
	int count2;
};

struct harpoon_data {		
	struct game_obj_t *gdb;	/* which gdb this harpoon came from */
};

struct octopus_data {
	int awake;			/* is octopus awake? */
	int tx, ty;			/* target x and y to which he's moving, in game coords */
	struct game_obj_t *tentacle[8];	/* the octopus's tentacles are separate objs. */
};

struct gdb_data {
	int awake;			/* is gdb awake? */
	int tx, ty;			/* target x and y, game coords */
	// struct game_obj_t *tentacle[8];
};

struct cron_data {
	int beam_speed;			/* speed at which beam is sweeping across ground. */
	int beam_pos;			/* x coord, in game coords of beam */
	int tmp_ty_offset;		/* if destination is a long way off, ty may be offset randomly */
					/* this keeps cron job from just sitting at a constant altitude */
					/* which would make him an easy target */
	int tx, ty;			/* destination x,y, in game coords */
	int eyepos;			/* the cron job has a little eye, this traks the eye's movement */
	struct game_obj_t *myhuman;	/* the vi .swp file which the cron job is cleaning up (abducting) */

	int pissed_off_timer;
	int state;			/* initially, SEEKING_HUMAN, then when he finds a human */
					/* he carries him to the volcano, then seeks another human. */
					/* occasionally he'll get pissed off at the player and chase */
					/* him for awhile (until pissed_off_timer gets to zero. */
					/* then, depending if he's carrying a human, back to seeking, */
					/* or back to the volcano. */
#define CRON_STATE_SEEKING_HUMAN 1
#define CRON_STATE_CARRYING_HUMAN_TO_VOLCANO 2
#define CRON_STATE_PISSED_OFF 3
};

struct tentacle_seg_data {
	int angle;			/* angle of this tentacle segment relative to previous one, in degrees */
	int length;			/* length of tentacle seg, pixels */
	int angular_v;			/* angular velocity, degrees/frame */
	int dest_angle;			/* target angle towards which tentacle seg is moving. */
};

struct tentacle_data {
	struct game_obj_t *attached_to;	/* the octopus's head */
	int angle;			/* angle of attachment of first segment */
	int nsegs;			/* how many segments? */
	int upper_angle, lower_angle;	/* limits on attachment angle (keeps tentacles mostly pointing down) */
	struct tentacle_seg_data *seg;	/* array of tentacle segments */
};

struct floating_message_data { 		/* for those floating text messages, like "Help!", "Woohoo!", and scores */
	int font;
	unsigned char msg[21];
};

struct human_data {
	struct game_obj_t *abductor;	/* cron job which has abducted him */
	int picked_up;			/* is he currently picked up? */
	int on_ground;			/* is he on the ground? */
	int human_number;		/* there are a number of humans, he must know which one he is. */
	int seat_number;
};

struct jammer_data {			/* these are used in the drawing of radar jammers */
	int width;			/* the spinning antenna uses these values. */
	int direction;			/* expanding or contracting */
};

struct fuel_data {
	int level;			/* how much fuel is in the fuel tank? */
};

struct airship_data {
	int bannerline;			/* blimps have scrolling text on the side */
					/* bannerline keeps track of where in the */
					/* script of text that scrolls by we are. */
	int pressure;			/* shooting at the blimps increases the */
					/* pressure, makes the blimp shoot out more */
					/* leaking lisp code, and makes it fatter */
};

struct worm_data {
#define NWORM_TIME_UNITS 5 
	struct game_obj_t *parent;	/* next worm segment up towards the head */
	struct game_obj_t *child;	/* next worm segment back towards the tail */
	int x[NWORM_TIME_UNITS];	/* list of this segments position for the last */
	int y[NWORM_TIME_UNITS];	/* few time units... x[i], y[i] is current pos */
	int i;				/* current index into x,y arrays */
	int tx, ty;			/* short term destination x,y */
	int ltx, lty;			/* long term destination x,y */
	int coloroffset;
};

struct debris_data {
	int debris_type;		/* controls what sounds debris makes when */
#define DEBRIS_TYPE_STONE 0		/* it bounces. */
#define DEBRIS_TYPE_METAL 1
};

struct truss_data {
	struct game_obj_t *above, *below;
};

struct kgun_data {
	int invert;	/* 1 means hanging upside down, -1, means mounted on ground. */
	int size;	/* between 0 and 31, 31 being the biggest. */
	int velocity_factor; /* smaller guns shoot slower, unless corrected by velocity factor */
};

union type_specific_data {		/* union of all the typs specific data */
	struct harpoon_data harpoon;
	struct gdb_data gdb;
	struct octopus_data octopus;
	struct extra_player_data epd;
	struct tentacle_data tentacle;
	struct floating_message_data floating_message;
	struct cron_data cron;
	struct human_data human;
	struct jammer_data jammer;
	struct fuel_data fuel;
	struct airship_data airship;
	struct debris_data debris;
	struct worm_data worm;
	struct truss_data truss;
	struct kgun_data kgun;
};

struct game_obj_t {
	int number;			/* offset into the go[] (game object) array */
	obj_move_func *move; 
	obj_draw_func *draw;
	obj_destroy_func *destroy;
	struct my_vect_obj *v;		/* drawing instructions */
	struct target_t *target;	/* keeps track of himself in the target list */
	int x, y;			/* current position, in game coords */
	int vx, vy;			/* velocity */
	int color;			/* initial color */
	int alive;			/* alive?  Or dead? */
	int otype;			/* object type */
	struct game_obj_t *bullseye;	/* point to object this object is chasing */
	int last_xi;			/* the last x index into the terrain array which */
					/* corresponds to the segment directly underneath */
					/* this object -- used for detecting when an object */
					/* smacks into the ground. */
	int counter;			/* a counter various object types used for var. purposes */
	union type_specific_data tsd;	/* the Type Specific Data for this object */
	int missile_timer;		/* to keep missiles from firing excessively rapidly */
	int radar_image;		/* Does this object show up on radar? */ 
};

struct target_t {			/* doubly linked list of objects which may be hit */
	struct game_obj_t *o;		/* saves us from searching through all objects. */ 
	struct target_t *prev, *next;
} *target_head = NULL;

struct terrain_t {			/* x,y points of the ground, in game coords */
	int npoints;
	int x[TERRAIN_LENGTH];
	int y[TERRAIN_LENGTH];
	int slope[TERRAIN_LENGTH];
} terrain;

struct game_state_t {
	int x;				/* x position of left edge of screen, in game coords */
	int y;				/* y position of top of screen, in game coords */
	// int last_x1, last_x2;	/* not used... */
	int vx;				/* x velocity of viewport.  Usually the same as player's vx */
	int vy;				/* y velocity of viewport.  Usually the same as player's vy */
	int lives;			/* How many lives the player has left in this game. */
	int nobjs;			/* max number of objects in the game */
	int direction;			/* direction player is facing. 1 = left, -1 = right. */
	int health;			/* Think of this as the player's hit points. */
	int score;			/* Player's score */
	int prev_score;			/* Used to detect changes in score for drawing routine efficiency. */
	int nbombs;			/* number of bombs in the player's possession */
	int prev_bombs;			/* Used to detect changes in bombs for drawing routine efficiency */
	int humanoids;			/* Number of humanoids the player has picked up */
	int gdbs_killed;		/* etc.*/
	int crons_killed;
	int guns_killed;
	int sams_killed;
	int missiles_killed;
	int octos_killed;
	int emacs_killed;
	int rockets_killed;
	int missile_locked;		/* Indicates whether a missile is after the player, for alarm sound */
	int corrosive_atmosphere;
	struct timeval start_time, 	/* Used to calculate how long player took to finish a level */
		finish_time;
	struct game_obj_t go[MAXOBJS];	/* all the objects in the game are in this array. */
	int cmd_multiplier;		/* If you prefix keystrokes with a number... like editing in vi. */
	int music_on;			/* Whether music is turned on or off */
	int sound_effects_on;		/* Whether sound effects are turned on or off */
	int radar_state;		/* Whether the radar is booting up, fritzed, or operational */
#define     RADAR_RUNNING (0)
#define     RADAR_FRITZED (-1)
#define     RADAR_BOOTUP (5 * frame_rate_hz) /* How long it takes the radar to boot up. */
	int nextbombtime;
	int nextlasertime;
	int nextlasercolor;
	int nextchafftime;

} game_state = { 0, 0, 0, 0, PLAYER_SPEED, 0, 0 };

struct game_obj_t * human[MAXHUMANS];	/* Keep a special array of just the humans so we can scan it quickly */

struct game_obj_t *volcano_obj;
struct game_obj_t *player = &game_state.go[0];	/* The player is object zero. */
int lasthuman = 0;				/* debug code... for 'n' key. */

GdkGC *gc = NULL;		/* our graphics context. */
GtkWidget *main_da;		/* main drawing area. */
gint timer_tag;			/* for our gtk 30 times per second timer function */
int next_quarter_time = -1;	/* Used to limit rate at which quarters can be put in. */
int next_thunder_time = -1;

#define NSTARS 150
struct star_t {
	int x; 
	int y;
	int last_xi;
	int bright;
} star[NSTARS];

static inline int randomn(int n);
int want_starfield = 1;
void init_stars()
{
	int i;

	for (i=0;i<NSTARS;i++) {
		star[i].x = randomn(SCREEN_WIDTH);
		star[i].y = randomn(SCREEN_HEIGHT);
		star[i].bright = randomn(100) & 0x01;
		star[i].last_xi = -1;
	}
}

int ground_level(int x, int *xi, int *slope);
int interpolate(int x, int x1, int y1, int x2, int y2);


/* given world coords, x, and y, and pointer to last index into the
 * terrain array which we were above, return an "approximate horizon"
 * which is to say, return a y value which is either the true y value
 * of the ground directly below the given x value, or, if it is 
 * faster to compute, give a value which is > y if tf the horizon is
 * below y, or < y, if the horizon is above y. 
 *
 * Useful for instance to know if stars are above or below the horizon
 * to know if they should be drawn. 
 *
 * Also, the value of *last_xi is updated with the terrain index 
 * corresponding to the x value given.  If this is saved, and reused 
 * between calls, then an expensive search is avoided.
 *
 */
int approximate_horizon(int x, int y, int *last_xi) 
{
	/* optimized way to find the ground level (y value) at an object's */
	/* x position.  Each object tracks the terrain segment it was last at. */
	/* This means, the linear search of the terrain array is mostly eliminated */

	int xi1, xi2, i;
	xi1 = *last_xi; /* try the terrain segment we used last time. */
	xi2 = xi1 + 1;
	int slope;

	if (xi1 < 0 || xi2 >= TERRAIN_LENGTH) /* Do we have a hint of the last one? No? */
		return ground_level(x, last_xi, &slope); /* do it the hard way. */

	/* Is the last terrain segment the correct one? */
	if (terrain.x[xi1] <= x && terrain.x[xi2] >= x) {
	
		/* if both endpoints are below us, then no need ti find the exact point */
		/* below us, it is sufficient to know that it must be _somewhere_ */
		/* below us, and not above us. */
		if (terrain.y[xi1] > y && terrain.y[xi2] > y)
			return y + 10;
		/* similarly if both endpoints above us... */
		if (terrain.y[xi1] < y && terrain.y[xi2] < y)
			return y - 10;

		/* vast majority will hit one of above 2 cases, but close to the ground... */
		return interpolate(x, terrain.x[xi1], terrain.y[xi1],  /* do it the hard way. */
				terrain.x[xi2], terrain.y[xi2]);
	}

	/* The last one wasn't correct.  Have to search. */
	if (terrain.x[xi1] > x) {
		/* The correct one is to the left.  Search left. */
		for (i=xi1-1;i>=0;i--) {
			if (x >= terrain.x[i] && x < terrain.x[i+1]) {

				/* remember i for next time so we don't have to search. */
				*last_xi = i;

				/* Two trivial cases...  ground is trivially below us? */
				if (terrain.y[i] > y && terrain.y[i+1] > y)
					return y + 10;
				/* or trivially above us? */
				if (terrain.y[i] < y && terrain.y[i+1] < y) 
					return y - 10;
				/* Close to the ground, so, ok, do it the hard way. */
				return interpolate(x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	} else if (terrain.x[xi2] < x) { /* Correct one is to the right. */
		for (i=xi1+1;i<=TERRAIN_LENGTH-10;i++) { /* search to the right. */
			if (x >= terrain.x[i] && x < terrain.x[i+1]) {

				/* remember i for next time so we don't have to search. */
				*last_xi = i;

				/* Two trivial cases...  ground is trivially below us? */
				if (terrain.y[i] > y && terrain.y[i+1] > y)
					return y + 10;
				/* or trivially above us? */
				if (terrain.y[i] < y && terrain.y[i+1] < y) 
					return y - 10;
				/* Close to the ground, so, ok, do it the hard way. */
				return interpolate(x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	}

	/* What?  there *is* no correct answer.  Object must have fallen off */
	/* the edge of the planet. (it happens sometimes -- though that is a bug. ) */
	*last_xi = -1;
	return GROUND_OOPS;
}
	
void draw_stars(GtkWidget *w)
{
	int i;
	int worldx, worldy;
	int gl;

	gdk_gc_set_foreground(gc, &huex[WHITE]);
	for (i=0;i<NSTARS;i++) {
		if (randomn(100) < 3)
			continue;
		worldx = game_state.x + star[i].x;
		worldy = star[i].y + game_state.y - (SCREEN_HEIGHT/2); 

		gl = approximate_horizon(worldx, worldy, &star[i].last_xi);
		if (worldy < gl) {
			if (star[i].bright && randomn(100) > 10) {
				wwvi_draw_line(w->window, gc, star[i].x, star[i].y-1, star[i].x+1, star[i].y-1);
			}
			wwvi_draw_line(w->window, gc, star[i].x, star[i].y, star[i].x+1, star[i].y);
		}
	}
}


/* add an object to the list of targets... */
struct target_t *add_target(struct game_obj_t *o)
{
	struct target_t *t;

	t = malloc(sizeof(struct target_t));
	if (t == NULL) {
		printf("add target failed.\n");
		return NULL;
	}

	t->o = o;
	t->prev = NULL;
	if (target_head == NULL) { 
		target_head = t;
		t->next = NULL;
	} else {
		t->next = target_head;
		target_head->prev = t;
		target_head = t;
	}
	return target_head;
}

/* for debugging... */
void print_target_list()
{
	struct target_t *t;
	printf("Targetlist:\n");
	for (t=target_head; t != NULL;t=t->next) {
		printf("%c: %d,%d\n", t->o->otype, t->o->x, t->o->y);
	}
	printf("end of list.\n");
}

/* remove an object from the target list */
struct target_t *remove_target(struct target_t *t)
{

	struct target_t *next;
	if (!t)
		return NULL;

	next = t->next;
	if (t == target_head)
		target_head = t->next;
	if (t->next)
		t->next->prev = t->prev;
	if (t->prev)
		t->prev->next = t->next;
	t->next = NULL;
	t->prev = NULL;
	free(t);
	return next;
}

/* get a random number between 0 and n-1... fast and loose algorithm.  */
static inline int randomn(int n)
{
	/* return (int) (((random() + 0.0) / (RAND_MAX + 0.0)) * (n + 0.0)); */
	/* floating point divide?  No. */
	return ((random() & 0x0000ffff) * n) >> 16;
}

#define min(a,b) ((a) > (b) ? (b) : (a))

/* get a random number between a and b. */
static inline int randomab(int a, int b)
{
	/* int x, y;
	if (a > b) {
		x = b;
		y = a;
	} else {
		x = a;
		y = b;
	}
	return (int) (((random() + 0.0) / (RAND_MAX + 0.0)) * (y - x + 0.0)) + x; */
	int n;
	n = abs(a - b);
	return (((random() & 0x0000ffff) * n) >> 16) + min(a,b);
}

void explode(int x, int y, int ivx, int ivy, int v, int nsparks, int time);
static inline void kill_object(struct game_obj_t *o);
static inline void age_object(struct game_obj_t *o);


void move_laserbolt(struct game_obj_t *o)
{
	int dy;
	if (!o->alive)
		return;
	dy = (o->y - player->y);
	if (dy < -1000) { /* if laser bolt is very far away, just get forget about it. */
		kill_object(o);
		o->destroy(o);
		return;
	}
	if (abs(dy) < 9 && abs(player->x - o->x) < 15) { /* hit the player? */
		explode(o->x, o->y, o->vx, 1, 70, 20, 20);
		add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);
		game_state.health -= LASER_BOLT_DAMAGE;
		kill_object(o);
		o->destroy(o);
	}
	o->x += o->vx;
	o->y += o->vy;
	age_object(o);
}

void fuel_move(struct game_obj_t *o)
{
	int xdist, ydist;

	xdist = abs(player->x - (o->x + 20));
	ydist = abs(player->y - o->y);
	if (xdist <= HUMANOID_DIST*3 && 
		ydist <= HUMANOID_DIST*5 &&		/* player close enough? */
		(timer % REFUEL_RATE) == 0 && 		/* control refuel rate... */
		o->tsd.fuel.level > 0 && 		/* there is some fuel to dispense? */
		game_state.health < MAXHEALTH ) {	/* player's tank not full? */
			o->tsd.fuel.level--;		/* ...give player some fuel. */
			game_state.health++;
	} else 
		/* tanks refill at one unit per 3 secs */
		if (o->tsd.fuel.level < FUELTANK_CAPACITY && 
			(timer % REFILL_RATE == 0))
			o->tsd.fuel.level++;
}

/* Given target o, and shooter, sx,sy, and laserbolt velocity v, */
/* Compute correct *vx, *vy */
static void aim_vx_vy(struct game_obj_t *target, 
		struct game_obj_t *shooter,
		int v, int leadtime, 
		int *vx, int *vy)
{
	int dx, dy;
	dx = target->x + leadtime * target->vx - shooter->x;
	dy = target->y + leadtime * target->vy - shooter->y;

	/* whichever is farther, x or y, make that vx or vy be the max */
	/* then calculate the other one by similar triangles. */

	if (abs(dx) >= abs(dy)) {
		if (dx < 0) {
			*vx = -v;
			*vy = (*vx * dy) / dx;
		} else {
			if (dx > 0) {
				*vx = v;
				*vy = (*vx * dy) / dx;
			} else {
				/* shooter and target are the same. */
				*vx = 0;
				*vy = 0;
			}
		}
	} else {
		if (dy < 0) {
			*vy = -v;
			*vx = (*vy * dx) / dy;
		} else {
			if (dy > 0) {
				*vy = v;
				*vx = (*vy * dx) / dy;
			} else {
				*vy = 0;
				*vx = 0;
			}
		}
	}
}

static void add_laserbolt(int x, int y, int vx, int vy, int color, int time);
void kgun_move(struct game_obj_t *o)
{
	int xdist, ydist;
	int dx, dy;
	int x1, y1;
	int xi, yi;
	int adx, ady;
	int vx, vy;
	int crossbeam_x1, crossbeam_y, crossbeam_x2, xoffset;
	int guntipx, guntipy;
	int chance;
	int size = o->tsd.kgun.size;
	int velocityfactor = o->tsd.kgun.velocity_factor;
	int invert = o->tsd.kgun.invert;


	xdist = abs(o->x - player->x); /* in range? */
	ydist = abs(o->y - player->y);

	if (xdist < 600)
		chance = 100 + xdist;
	else 
		chance = 1000;

	if (xdist < SCREEN_WIDTH && ydist < SCREEN_HEIGHT-200 && 
		randomn(chance) < level.laser_fire_chance) {
		/* we're going to fire the laser... */
		add_sound(FLAK_FIRE_SOUND, ANY_SLOT);

		/* Find x,y dist to player... */
		dx = player->x+LASERLEAD*player->vx - o->x;
		dy = player->y+LASERLEAD*player->vy - o->y;

		adx = abs(dx);
		ady = abs(dy);

#if 0
		/* whichever is farther, x or y, make that vx or vy be the max */
		/* then calculate the other one by similar triangles. */
		if (dy <= 0) {
			if (player->x+player->vx*LASERLEAD < o->x)
				bx = -20;
			else
				bx = 20;
			by = 0;
		} else if (dx == 0) {
			bx = -0;
			by = 20;
		} else if (abs(dx) > abs(dy)) {
			if (player->x+player->vx*LASERLEAD < o->x)
				bx = -20;
			else
				bx = 20;
			by = abs((20*dy)/dx);
		} else {
			by = 20;
			/* if (player->x < o->x)
				bx = -20;
			else
				bx = 20; */
			bx = (20*dx)/dy;
		}
		x1 = o->x-5;
		y1 = o->y+5;  
#endif

		x1 = o->x-5;
		y1 = o->y+(5*invert);  
		crossbeam_y = y1 + (20*invert);

		/* First, scale the larger of dx,dy to 32, and the other by */
		/* a proportional amount (by similar triangles).  This is in */
		/* order to index into the vxy_2_dxy array. */
		
		/* Calculate indices into vxy_2_dxy array, xi, and yi */
		if (adx > ady) {
			xi = size;
			yi = (size * ady) / adx;
		} else {
			if (ady > adx) {
				yi = size;
				xi = (size * adx) / ady;
			} else { /* abs(dx) == abs(dy) */
				xi = size;
				yi = size;
			}
		}

		/* notice, using y component for x offset... */
		crossbeam_x1 = vxy_2_dxy[xi][yi].y;
		crossbeam_x2 = -crossbeam_x1;

		crossbeam_x1 += o->x;
		crossbeam_x2 += o->x;

		xoffset = crossbeam_x2 - crossbeam_x1;

		// guntipx = dx_from_vxy(xi, yi);
		// guntipy = dy_from_vxy(xi, yi);

		guntipx = vxy_2_dxy[xi][yi].x;
		guntipy = vxy_2_dxy[xi][yi].y;

		if (dy > 20)
			guntipy = -guntipy;
		if (dx < 0)
			guntipx = -guntipx;

		vx = guntipx * velocityfactor;
		vy = guntipy * velocityfactor;

		guntipx = guntipx << 1; /* gun barrel is 2x as long as the butt of the gun. */
		guntipy = guntipy << 1;

		guntipx += crossbeam_x1;
		guntipy += crossbeam_y;
		
		add_laserbolt(guntipx, guntipy, vx, vy, RED, 50);
		add_laserbolt(guntipx+xoffset, guntipy, vx, vy, RED, 50);
	}
}

static void add_missile(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye);
static void add_harpoon(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye,
	struct game_obj_t *gdb);

void move_rocket(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	/* Should probably make a rocket-station, which launches rockets */
	/* instead of just having bare rockets sitting on the ground which */
	/* launch once, blow up, then that's the end of them. */

	/* see if rocket should launch... */
	xdist = abs(o->x - player->x);
	if (xdist < LAUNCH_DIST && o->alive != 2 && randomn(100) < 20) {
		ydist = o->y - player->y;
		if (((xdist<<1) <= ydist && ydist > 0) || o->vy != 0) {
			if (o->vy == 0) { /* only add the sound once. */
				add_sound(ROCKET_LAUNCH_SOUND, ANY_SLOT);
				o->alive = 2; /* launched. */
				o->vy = -6; /* give them a little boost. */
			}
		}
	}
	if (o->alive == 2) {
		if (o->vy > MAX_ROCKET_SPEED - (o->number % 5))
			o->vy--;

		/* let the rockets veer slightly left or right. */
		if (player->x < o->x && player->vx < 0)
			o->vx = -2;
		else if (player->x > o->x && player->vx > 0)
			o->vx = 2;
		else 
			o->vx = 0;

		ydist = o->y - player->y;
		if ((ydist*ydist + xdist*xdist) < 400) { /* hit the player? */
			add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
			explode(o->x, o->y, o->vx, 1, 70, 150, 20);
			kill_object(o);
			game_state.health -= 20;
			remove_target(o->target);
			return;
		}
	}


	/* move the rocket... */
	o->x += o->vx;
	o->y += o->vy;
	if (o->vy != 0)
		explode(o->x, o->y, 0, 9, 8, 7, 13); /* spray out some exhaust */
	if (o->y - player->y < -1000 && o->vy != 0) {
		/* if the rocket is way off the top of the screen, just forget about it. */
		kill_object(o);
		remove_target(o->target);
		o->destroy(o);
	}
}

/* move sam stations. */
void sam_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	xdist = abs(o->x - player->x);
	if (xdist < SAM_LAUNCH_DIST) { /* player in range? */
		ydist = o->y - player->y;
		/* should we launch?  And not too many at once... */
		if (ydist > 0 && randomn(1000) < SAM_LAUNCH_CHANCE && timer >= o->missile_timer) {
			/* launching a missile... */
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_missile(o->x+20, o->y-30, 0, -10, 300, GREEN, player);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
		}
	}
}

void draw_generic(struct game_obj_t *o, GtkWidget *w);
void draw_scaled_generic(struct game_obj_t *o, GtkWidget *w, float xscale, float yscale);

void fuel_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2;
	int xdist, ydist;

	xdist = abs(player->x - o->x);
	ydist = abs(player->y - o->y);
		
	/* draw the fuel in the tank. */
	x1 = o->x - game_state.x - 25;
	x2 = o->x - game_state.x + 25;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2) + 
		30 - (45 * o->tsd.fuel.level / FUELTANK_CAPACITY);
	if (o->tsd.fuel.level > 0) {
		gdk_gc_set_foreground(gc, &huex[DARKGREEN]);
		wwvi_draw_rectangle(w->window, gc, TRUE, x1, y1, 50, o->tsd.fuel.level * 45 / FUELTANK_CAPACITY);
	}

	draw_generic(o, w); /* draw the fuel tank in the usual way... */

	/* if the player is close, draw a refueling hose... */
	if (xdist <= HUMANOID_DIST*3 && ydist <= HUMANOID_DIST*5) {
		x1 = o->x - game_state.x;
		y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);
		x2 = player->x - game_state.x;
		y2 = player->y - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2);
	}
}

/* draw a radar jammer */
void jammer_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x1, x2, y1, y2, x3, x4, x5;
	draw_generic(o, w); /* will set the color too. */

	/* this draws a sort of spinning rectangular gridded radar dish */
	o->tsd.jammer.width += o->tsd.jammer.direction;
	if (o->tsd.jammer.width > 8) {
		o->tsd.jammer.direction = -1;
	} else if (o->tsd.jammer.width < 2) {
		o->tsd.jammer.direction = 1;
	}
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2) - 15;  
	y2 = y1 - 16;
	x5 = (o->x - game_state.x);
	x1 = x5 - o->tsd.jammer.width;
	x2 = x5 + o->tsd.jammer.width;
	x3 = x5 - (o->tsd.jammer.width >> 1);
	x4 = x5 + (o->tsd.jammer.width >> 1);
	wwvi_draw_line(w->window, gc, x1, y1, x2, y1); 
	wwvi_draw_line(w->window, gc, x1, y2, x2, y2); 
	wwvi_draw_line(w->window, gc, x1, y1, x1, y2); 
	wwvi_draw_line(w->window, gc, x2, y1, x2, y2); 
	wwvi_draw_line(w->window, gc, x3, y1, x3, y2); 
	wwvi_draw_line(w->window, gc, x4, y1, x4, y2); 
	wwvi_draw_line(w->window, gc, x5, y1, x5, y2); 
	wwvi_draw_line(w->window, gc, x1, y2+4, x2, y2+4); 
	wwvi_draw_line(w->window, gc, x1, y2+8, x2, y2+8); 
	wwvi_draw_line(w->window, gc, x1, y2+12, x2, y2+12); 
}

void cron_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2, gy, xi;

	draw_generic(o, w);

#if 0
	/* this is debug code */
	dist2 = (o->tsd.cron.tx - o->x)* (o->tsd.cron.tx - o->x) + (o->tsd.cron.ty - o->y)* (o->tsd.cron.ty - o->y);
	if (dist2 < 20000) {
		gdk_gc_set_foreground(gc, &huex[randomn(NCOLORS+NSPARKCOLORS)]);
		x1 = o->x - game_state.x;
		y1 = o->y + 7 - game_state.y + (SCREEN_HEIGHT/2);  
		x2 = o->tsd.cron.tx - game_state.x;
		y2 = o->tsd.cron.ty + 7 - game_state.y + (SCREEN_HEIGHT/2);  
		if (x1 > 0 && x2 > 0)
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
	}
#endif

	/* draw the cron job's eye... */
	x1 = o->x - game_state.x + o->tsd.cron.eyepos;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2) - 5;  
	x2 = x1 + 3;
	gdk_gc_set_foreground(gc, &huex[WHITE]);
	wwvi_draw_line(w->window, gc, x1, y1, x2, y1); /* draw eye */

	/* move the cron job's eye */
	o->tsd.cron.eyepos +=1;
	if (o->tsd.cron.eyepos > 10)
		o->tsd.cron.eyepos = -10;
	

	/* draw the cron job's scanning beam... */
	if (o->tsd.cron.beam_speed != 0) { /* beam on? */
		gdk_gc_set_foreground(gc, &huex[randomn(NCOLORS+NSPARKCOLORS+NRAINBOWCOLORS)]);
		gy = ground_level(o->x + o->tsd.cron.beam_pos, &xi, NULL);
		if (xi != -1) {
			x1 = o->x - game_state.x;
			y1 = o->y + 7 - game_state.y + (SCREEN_HEIGHT/2);  
			x2 = o->x + o->tsd.cron.beam_pos - game_state.x; 
			y2 = gy + (SCREEN_HEIGHT/2) - game_state.y;
			if (x1 > 0 && x2 > 0) /* on screen? */
				wwvi_draw_line(w->window, gc, x1, y1, x2, y2);  /* draw beam. */
			explode(o->x + o->tsd.cron.beam_pos, gy, 0, 1, 20, 3, 10); /* make sparks. */
		}
	}
}

void gdb_draw(struct game_obj_t *o, GtkWidget *w) /* draw a gdb. */
{
	/* make him point left or right, depending which way he's going. */
	if (o->vx < 0)		
		o->v = &gdb_vect_left;
	else if (o->vx > 0)
		o->v = &gdb_vect_right;
	draw_generic(o, w);
}

/* recursive routine to draw a fractal lightning bolt between x1,y1 and x2,y2 */
void draw_lightning( GtkWidget *w, int x1, int y1, int x2, int y2) 
{
	int x3, y3, dx, dy;

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	/* terminal case, x1, y1, and x2, y2 are very close, just draw a line. */
	if (dx < 10 && dy < 10) {
		gdk_gc_set_foreground(gc, &huex[BLUE]);
		if (dx < dy) {
			wwvi_draw_line(w->window, gc, x1-1, y1, x2-1, y2); 
			wwvi_draw_line(w->window, gc, x1+1, y1, x2+1, y2); 
		} else {
			wwvi_draw_line(w->window, gc, x1, y1-1, x2, y2-1); 
			wwvi_draw_line(w->window, gc, x1, y1+1, x2, y2+1); 
		}
		gdk_gc_set_foreground(gc, &huex[WHITE]);
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		return;
	}

	/* find the midpoint between x1,y1, x2, y2.  Call this midpoint x3,y3. */
	if (y2 > y1)
		y3 = (y2 - y1) / 2 + y1;
	else if (y2 < y1)
		y3 = (y1 - y2) / 2 + y2;
	else 
		y3 = y1;
	if (x2 > x1)
		x3 = (x2 - x1) / 2 + x1;
	else if (x2 < x1)
		x3 = (x1 - x2) / 2 + x2;
	else
		x3 = x1;

	/* perturb the midpoint a bit */
	x3 += (int) (0.2 * (randomn(2*dx) - dx)); 
	y3 += (int) (0.2 * (randomn(2*dy) - dy)); 

	/* draw lightning between x1,y1 and x3,y3, and between x3,y3, and x2,y2. */
	draw_lightning(w, x1, y1, x3, y3);	
	draw_lightning(w, x3, y3, x2, y2);	
}


static void xy_draw_string(GtkWidget *w, char *s, int font, int x, int y) ;
void floating_message_draw(struct game_obj_t *o, GtkWidget *w)
{
	/* draw a floating message -- a game object that is text living in game coords. */
	/* this is for bonus scores, "Help!" and "Woohoo!" messages the .swp files "say." */
	gdk_gc_set_foreground(gc, &huex[o->color]);
	xy_draw_string(w, (char *) o->tsd.floating_message.msg, 
		o->tsd.floating_message.font, o->x, o->y) ;
}

void tentacle_draw(struct game_obj_t *o, GtkWidget *w)
{
	int i;
	int x1, y1, x2, y2;
	int angle = 0;
	int thickness = o->tsd.tentacle.nsegs >> 1;

	x2 = 0; /* make compiler happy */
	y2 = 0; /* make compiler happy */

	/* draw each tentacle, accumulating relative angles as you go, */
	/* alternating colors of segments between yellow and blue.  */
	gdk_gc_set_foreground(gc, &huex[o->color]);
	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  
	for (i=0;i<o->tsd.tentacle.nsegs;i++) {
		angle = angle + o->tsd.tentacle.seg[i].angle;
		if (angle < 0)
			angle += 360;
		else if (angle > 360)
			angle -= 360;
		x2 = x1 + cosine[angle] * o->tsd.tentacle.seg[i].length; 
		y2 = y1 -   sine[angle] * o->tsd.tentacle.seg[i].length; 
		if ((i % 2) == 0)
			gdk_gc_set_foreground(gc, &huex[BLUE]);
		else
			gdk_gc_set_foreground(gc, &huex[YELLOW]);
		if (thickness > 0) {
			wwvi_draw_line(w->window, gc, x1-thickness, y1, x2-thickness, y2); 
			wwvi_draw_line(w->window, gc, x1+thickness, y1, x2+thickness, y2); 
			wwvi_draw_line(w->window, gc, x1, y1-thickness, x2, y2-thickness); 
			wwvi_draw_line(w->window, gc, x1, y1+thickness, x2, y2+thickness); 
			if (i & 0x01) 
				thickness--;
		} else 
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		x1 = x2;
		y1 = y2;
	}

	/* Shoot lightning at the player occasionally if he gets too close. */
	if ((timer & 0x01) && 
		randomn(1000) < 45 && 
		abs(o->x - player->x) < 200 && 
		abs(o->y - player->y) < 500) {

		draw_lightning(w, x2, y2, player->x - game_state.x, 
			player->y - game_state.y + (SCREEN_HEIGHT/2));
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		game_state.health -= 1; /* just take off one, the octo's are bad... */
		/* keep thunder from going oof too much */
		if ((next_thunder_time) < timer) {
			next_thunder_time = timer + frame_rate_hz * 2;
			add_sound(THUNDER_SOUND, ANY_SLOT);
		}
	}
}

int find_ground_level(struct game_obj_t *o, int *slope);

void tentacle_move(struct game_obj_t *o) 
{

	struct tentacle_seg_data *t;
	int i, gy;

	/* if we're off screen, bail right away. */
	if (abs(o->x - game_state.x) > SCREEN_WIDTH*1.5)
		return;

	/* loose tentacle? */
	if (o->tsd.tentacle.attached_to == NULL) {	
		o->x += o->vx;
		o->y += o->vy;

		o->vy += 1; /* make loose tentacle fall to the ground. */

		gy = find_ground_level(o, NULL);
		if (o->y >= gy) {
			o->vy = 0;
			o->vx = 0;
			o->y = gy;
		}

		/* keep tentacle from jumping off the edges of the earth. */
		if (o->x < 0) {
			o->x = 0;
			o->vx = 20;
		}
		if (o->x > terrain.x[TERRAIN_LENGTH-1]) {
			o->x = terrain.x[TERRAIN_LENGTH-1];
			o->vx = -20;
		}
	} else {
		/* attached tentacles don't fall, but remain with whatever they're */
		/* attached to, usually an octupus head. */
		o->x = o->tsd.tentacle.attached_to->x;
		o->y = o->tsd.tentacle.attached_to->y;
	}

	for (i=0;i<o->tsd.tentacle.nsegs;i++) { /* for each segment of the tentacle.... */
		int da;

		/* set the angular velocity to move the segment towards it's desired angle. */
		t = &o->tsd.tentacle.seg[i];
		if (i==0)
			t->dest_angle = o->tsd.tentacle.angle;

		da = (t->dest_angle - t->angle);
		if (abs(da) > 10) {
			if (t->angle < t->dest_angle)
				t->angular_v = 3;
			else if (t->angle > t->dest_angle)
				t->angular_v = -3;
		} else {
			if (t->angle < t->dest_angle)
				t->angular_v = 1;
			else if (t->angle > t->dest_angle)
				t->angular_v = -1;
			else
				t->angular_v = 0;
		}

		t->angle += t->angular_v; /* move segment towards desired angle... */

		/* don't hyper-rotate segment... */
		if (i != 0 && t->angle > MAX_SEG_ANGLE)
			t->angle = MAX_SEG_ANGLE;
		else if (i != 0 && t->angle < -MAX_SEG_ANGLE)
			t->angle = -MAX_SEG_ANGLE;
	}

	if (randomn(1000) < 50) { /* every once in awhile... */

		/* change the tentacle's idea of what a desirable angle is. */
		o->tsd.tentacle.angle = TENTACLE_RANGE(o->tsd.tentacle);
		if (o->tsd.tentacle.angle < 0)
			o->tsd.tentacle.angle += 360; 
		if (o->tsd.tentacle.angle > 360)
			o->tsd.tentacle.angle -= 360; 

		/* set the tentacle's segments' desired angles... */
		switch (randomn(11)) {

			/* mostly, set tentacle segment desired angles  */
			/* to random values, but in range... */
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
		case 8:
			for (i=1;i<o->tsd.tentacle.nsegs;i++) {
				t = &o->tsd.tentacle.seg[i];
				t->dest_angle = randomn(2 * MAX_SEG_ANGLE) - MAX_SEG_ANGLE;
			}
			break;
		case 10: if (o->tsd.tentacle.attached_to == NULL) {
				/* sometimes, straighten a tentacle out, */
				/* and make it jump in the direction */
				/* it's pointing... very weird. */
				for (i=1;i<o->tsd.tentacle.nsegs;i++) {
					t = &o->tsd.tentacle.seg[i];
					t->dest_angle = 0;
				}
				o->vx = cosine[o->tsd.tentacle.angle] * 20;
				o->vy = -sine[o->tsd.tentacle.angle] * 20;
			}
			break;
		case 5:
			/* sometimes, make tentacles curl up clockwise... */
			for (i=0;i<o->tsd.tentacle.nsegs;i++) {
				t = &o->tsd.tentacle.seg[i];
				t->angular_v = -1;
			}
			break;
		case 9:
			/* sometimes, make tentacles curl up counterclockwise... */
			for (i=0;i<o->tsd.tentacle.nsegs;i++) {
				t = &o->tsd.tentacle.seg[i];
				t->angular_v = 1;
			}
			break;
		}
	}
}

void octopus_move(struct game_obj_t *o)
{
	int xdist, ydist;
	int dvx, dvy, tx, ty;
	int gy, i;

	if (!o->alive)
		return;

	gy = find_ground_level(o, NULL);
	

	if (o->tsd.octopus.awake) {
		dvx = 0; /* make compiler happy */
		dvy = 0; /* make compiler happy */
		tx = o->tsd.octopus.tx;
		ty = o->tsd.octopus.ty;

		/* compute a "desired" vx, vy which is towards the player. */
		if (o->x < tx && tx - o->x > GDB_DX_THRESHOLD)
			dvx = GDB_MAX_VX;
		else if (o->x < tx)
			dvx = 0;
		else if (o->x > tx && o->x - tx > GDB_DX_THRESHOLD)
			dvx = -GDB_MAX_VX;
		else if (o->x > tx)
			dvx = 0;
		if (o->y < ty && ty - o->y > GDB_DY_THRESHOLD)
			dvy = GDB_MAX_VY;
		else if (o->y < ty)
			dvy = 0;
		else if (o->y > ty && o->y - ty > GDB_DY_THRESHOLD)
			dvy = -GDB_MAX_VY;
		else if (o->y > ty)
			dvy = 0;

		/* if we're not already close enough, sometimes choose a destination near */
		/* the player's location. */
		if (abs(player->x - tx) > GDB_DX_THRESHOLD ||
			abs(player->y - ty) > GDB_DY_THRESHOLD || randomn(100) < 3) {
			o->tsd.octopus.tx = player->x + randomn(300)-150;
			o->tsd.octopus.ty = player->y + randomn(300)-150;
		}

		/* don't sink through the ground... */
		if (o->y > gy - 100 && dvy > -3)
			dvy = -10;	

		/* adjust velocity to make it close to the desired vx,vy computed earlier. */
		if (o->vx < dvx)
			o->vx++;
		else if (o->vx > dvx)
			o->vx--;
		if (o->vy < dvy)
			o->vy++;
		else if (o->vy > dvy)
			o->vy--;

		o->x += o->vx; /* move... */
		o->y += o->vy;

		/* put out some exhaust in the direction opposite what we're moving. */	
		explode(o->x - dvx + randomn(4)-2, o->y - dvy + randomn(4)-2, -dvx, -dvy, 4, 8, 9);
	}

	/* see if we're close to the player, and if so, and we're not awake, then wake up. */	
	xdist = abs(o->x - player->x);
	if (xdist < GDB_LAUNCH_DIST) {
		ydist = o->y - player->y;
#if 1
		if (randomn(1000) < SAM_LAUNCH_CHANCE && timer >= o->missile_timer) {
			// add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			//add_harpoon(o->x+10, o->y, 0, 0, 300, MAGENTA, player, o);
			// o->missile_timer = timer + MISSILE_FIRE_PERIOD;
			if (!o->tsd.octopus.awake) {
				o->tsd.octopus.awake = 1;
				o->tsd.octopus.tx = player->x + randomn(200)-100;
				o->tsd.octopus.ty = player->y + randomn(200)-100;
			}
		}
#endif
	}

	/* move the tentacles along with us. */
	for (i=0;i<8;i++) {
		if (o->tsd.octopus.tentacle[i]) {
			o->tsd.octopus.tentacle[i]->x = o->x;
			o->tsd.octopus.tentacle[i]->y = o->y;
		}
	}


	/* hmm, I forget what this is for.  Seems to say, if octopus smacks into ground, */
	/* blow up and die.... but why? */	
	if (o->y >= gy + 3) {
		kill_object(o);
		explode(o->x, o->y, o->vx, 1, 70, 150, 20);
		o->destroy(o);
	}
}

static void add_floater_message(int x, int y, char *msg);
static void add_bullet(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye);
void cron_move(struct game_obj_t *o)
{
	int xdist, ydist;
	int dvx, dvy, tx, ty;
	int gy, dgy, xi;
	int done, dist2, i;

	if (!o->alive)
		return;

	/* Try to find a human that you want to abduct... */
	if (o->tsd.cron.state == CRON_STATE_SEEKING_HUMAN && 
		(o->tsd.cron.myhuman == NULL) &&
		((timer & 0x0f) == 0)) {

		/* check for unpursued humans. */
		for (i=0;i<level.nhumanoids;i++) {
			if (human[i] == NULL)
				continue;
			if (human[i]->tsd.human.abductor == NULL) {
				int hvx;

				/* If the human is already at the volcano, don't consider him. */
				hvx = abs(human[i]->x - volcano_obj->x);
				if (hvx < 20)
					continue;
			
				/* Found an un-abducted human, not too close to the volcano */	
				/* go after him. */
				o->tsd.cron.myhuman = human[i];
				human[i]->tsd.human.abductor = o;
				human[i]->tsd.human.picked_up = 0;
				o->tsd.cron.tx = human[i]->x;
				o->tsd.cron.ty = human[i]->y;
				break;
			}
		}
	}

	done = 0;
	gy = find_ground_level(o, NULL);

	tx = o->tsd.cron.tx;
	ty = o->tsd.cron.ty;

	/* See how far we still have to travel... */
	xdist = abs(o->x - tx);

	if (xdist < CRON_DX_THRESHOLD * 2)
		/* It's not far, don't screw around, just go there... */
		o->tsd.cron.tmp_ty_offset = 0;
	else {
		/* Still pretty far away, so, sometimes... */
		if (randomn(100) <= 30)
			/* screw around a bit and offset our y dest by a bit. */
			/* This makes us bob up and down a bit, a less easy target. */
			o->tsd.cron.tmp_ty_offset = -randomn(150);
	}

	/* if we made it to our dest, pick a new dest. */
	if (xdist <= CRON_DX_THRESHOLD  && abs(o->y - ty) <= CRON_DY_THRESHOLD ) {
		/* pick a new target location */
		done = 1;


		/* See if we have carried a human to the volcano... */
		if (o->tsd.cron.state == CRON_STATE_CARRYING_HUMAN_TO_VOLCANO &&
			o->tsd.cron.myhuman != NULL &&		/* carrying human? */
			o->tsd.cron.myhuman->tsd.human.picked_up == 1 &&	/* human is being carried? */
			o->tsd.cron.myhuman->tsd.human.abductor == o &&	/* by me? */
			o->tsd.cron.tx == volcano_obj->x) {	/* we're headed to the volcano? */

			/* We have carried a human to the volcano.  Drop him in. */

			o->tsd.cron.myhuman->tsd.human.abductor = NULL; /* human isn't abducted anymore. */
			o->tsd.cron.myhuman->x = o->x;	      /* drop him off _here_ */
			o->tsd.cron.myhuman->y = o->y;
			o->tsd.cron.myhuman->tsd.human.picked_up = 0;   /* he's not picked up. */
			o->tsd.cron.myhuman->tsd.human.on_ground = 0;   /* he's not on the ground. */
			o->tsd.cron.myhuman = NULL;
			o->tsd.cron.state = CRON_STATE_SEEKING_HUMAN; /* start looking for another one. */
			add_sound(SCREAM_SOUND, ANY_SLOT);
		} else {
		
			/* Pick a new random destination... */	
			o->tsd.cron.tx = terrain.x[randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1)];
			tx = o->tsd.cron.tx;
			dgy = ground_level(tx, &xi, NULL);
			o->tsd.cron.ty = dgy-140;
			ty = o->tsd.cron.ty;
			o->tsd.cron.tmp_ty_offset = 0;

			/* If we are seeking a human... */
			if (o->tsd.cron.myhuman != NULL && 
				o->tsd.cron.state == CRON_STATE_SEEKING_HUMAN) {
				/* Check to see if we've reached him. */
				ydist = abs(o->y - o->tsd.cron.myhuman->y); 
				xdist = abs(o->x - o->tsd.cron.myhuman->x); 
				if (o->tsd.cron.myhuman->tsd.human.picked_up == 0 &&
					xdist <= CRON_DX_THRESHOLD && ydist <= CRON_DY_THRESHOLD) {

					/* We got him... pick him up. */
					o->tsd.cron.myhuman->tsd.human.picked_up = 1;
					o->tsd.cron.myhuman->tsd.human.on_ground = 0;
					o->tsd.cron.state = CRON_STATE_CARRYING_HUMAN_TO_VOLCANO;
					o->tsd.cron.tx = volcano_obj->x;
					o->tsd.cron.ty = volcano_obj->y - 150;
					o->tsd.cron.myhuman->tsd.human.seat_number = 0;
					add_sound(ABDUCTED_SOUND, ANY_SLOT);

					/* Take him to the volcano. */
					o->tsd.cron.state = CRON_STATE_CARRYING_HUMAN_TO_VOLCANO;
				}
			}
		}
	}


	/* If it's a ways off to our destination, screw around a little bit */
	/* with the y coord of the destination to make us bob up and down */
	if (xdist > CRON_DX_THRESHOLD) {
		ty += o->tsd.cron.tmp_ty_offset;

		/* avoid the ground */
		if (ty > gy-15 && xdist)
			ty = gy-15;
	}
		else ty = o->tsd.cron.ty;

	
	/* move towards the destination */	
	if (o->x < tx && tx - o->x > CRON_DX_THRESHOLD)
		dvx = CRON_MAX_VX;
	else if (o->x > tx && o->x - tx > CRON_DX_THRESHOLD)
		dvx = -CRON_MAX_VX;
	else
		dvx = 0;
	if (o->y < ty && ty - o->y > CRON_DY_THRESHOLD)
		dvy = CRON_MAX_VY;
	else if (o->y > ty && o->y - ty > CRON_DY_THRESHOLD)
		dvy = -CRON_MAX_VY;
	else
		dvy = 0;


	/* watch out for bug...  This isn't fixed, but don't know how we get in here.. */
	if (done && dvx == 0 && dvy == 0) {
		/* printf("Arg!  Stuck.  tx=%d,ty=%d, x=%d, y=%d\n", 
			tx, ty, o->x, o->y); */
		dvy = -CRON_MAX_VY;
		dvx = -CRON_MAX_VX;
	}

	/* should we take a shot at the player? */
	if (randomn(100) <= CRON_SHOT_CHANCE && timer % 10 == 0) {
		dist2 = (player->x - o->x) * (player->x - o->x) + 
			(player->y - o->y) * (player->y - o->y);
		if (dist2 <= CRON_SHOT_DIST_SQR) {
			int dx, dy, vx, vy;
			/* calculate vx, vy by similar triangles */
			dx = (player->x + player->vx * BULLET_LEAD_TIME) - o->x;
			dy = (player->y + player->vy * BULLET_LEAD_TIME) - o->y;
			if (dx == 0 && dy == 0) {
				vx = 0;
				vy = 0;
			} else if (abs(dx) > abs(dy)) {
				vx = ((dx < 0) ? -1 : 1) * BULLET_SPEED;
				vy = (dy * BULLET_SPEED)/(abs(dx));
			} else {
				vy = ((dy < 0) ? -1 : 1) * BULLET_SPEED;
				vx = (dx * BULLET_SPEED)/(abs(dy));
			}
			// vx += randomn(3)-1;
			// vy += randomn(3)-1;
			explode(o->x, o->y, o->vx, o->vy, 4, 8, 9);
			add_bullet(o->x, o->y, vx, vy, 40, WHITE, player);
			// add_floater_message(o->x, o->y, "Bang!");
			add_sound(CRONSHOT, ANY_SLOT);
		}
	}
	
#if 0
	if (abs(player->x - tx) > CRON_DX_THRESHOLD ||
		abs(player->y - ty) > CRON_DY_THRESHOLD || randomn(100) < 3) {
		o->tsd.gdb.tx = player->x + randomn(300)-150;
		o->tsd.gdb.ty = player->y + randomn(300)-150;
	}
#endif

	/* if (o->y > gy - 100 && dvy > -3)
		dvy = -10;	 */

	/* adjust velocity towards desired velocity. */
	if (o->vx < dvx)
		o->vx++;
	else if (o->vx > dvx)
		o->vx--;
	if (o->vy < dvy)
		o->vy++;
	else if (o->vy > dvy)
		o->vy -= 2;

	o->x += o->vx; /* move... */
	o->y += o->vy;

	o->vy++; /* gravity */

	/* move the beam around */
	if (randomn(100) <= 3) {
		switch (randomn(10)) {
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5: o->tsd.cron.beam_speed = randomn(24) - 12;
				break;
			default:
				o->tsd.cron.beam_speed = 0; /* turn off beam */
		}
	}
	/* don't let the beam get too far to the left or right of the cron job. */
	if (((o->tsd.cron.beam_speed < 0 && o->tsd.cron.beam_pos < -300)) || 
		((o->tsd.cron.beam_speed > 0 && o->tsd.cron.beam_pos > 300)))
		o->tsd.cron.beam_speed = -o->tsd.cron.beam_speed;

	/* move the beam. */
	o->tsd.cron.beam_pos += o->tsd.cron.beam_speed;
		
	// explode(o->x - dvx + randomn(4)-2, o->y - dvy + randomn(4)-2, -dvx, -dvy, 4, 8, 9);
#if 0	
	xdist = abs(o->x - player->x);
	if (xdist < GDB_LAUNCH_DIST) {
		ydist = o->y - player->y;
		if (randomn(1000) < SAM_LAUNCH_CHANCE && timer >= o->missile_timer) {
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_harpoon(o->x+10, o->y, 0, 0, 300, MAGENTA, player, o);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
			if (!o->tsd.gdb.awake) {
				o->tsd.gdb.awake = 1;
				o->tsd.gdb.tx = player->x + randomn(200)-100;
				o->tsd.gdb.ty = player->y + randomn(200)-100;
			}
		}
	}
#endif

#if 0
	if (o->y >= gy + 3) {
		o->alive = 0;
		explode(o->x, o->y, o->vx, 1, 70, 150, 20);
		o->destroy(o);
	}
#endif
}

void gdb_move(struct game_obj_t *o)
{
	int xdist, ydist;
	int dvx, dvy, tx, ty;
	int gy = 32756; /* big number. */

	if (!o->alive)
		return;

	if (o->tsd.gdb.awake) {
		gy = find_ground_level(o, NULL);
	
		dvx = 0; /* make compiler happy */
		dvy = 0; /* make compiler happy */ 	

		/* compute a desired velocity which will move towards target coords */
		tx = o->tsd.gdb.tx;
		ty = o->tsd.gdb.ty;
		if (o->x < tx && tx - o->x > GDB_DX_THRESHOLD)
			dvx = GDB_MAX_VX;
		else if (o->x < tx)
			dvx = 0;
		else if (o->x > tx && o->x - tx > GDB_DX_THRESHOLD)
			dvx = -GDB_MAX_VX;
		else if (o->x > tx)
			dvx = 0;
		if (o->y < ty && ty - o->y > GDB_DY_THRESHOLD)
			dvy = GDB_MAX_VY;
		else if (o->y < ty)
			dvy = 0;
		else if (o->y > ty && o->y - ty > GDB_DY_THRESHOLD)
			dvy = -GDB_MAX_VY;
		else if (o->y > ty)
			dvy = 0;
#if 0
		/* If we aren't close to the destination, every once in awhile, */
		/* change the distination a bit.  This makes it a little less predictable */
		if (abs(player->x - tx) > GDB_DX_THRESHOLD ||
			abs(player->y - ty) > GDB_DY_THRESHOLD || randomn(100) < 3) {
			o->tsd.gdb.tx = player->x + randomn(300)-150;
			o->tsd.gdb.ty = player->y + randomn(300)-150;
		}
#endif

		{
			int angle = ((timer * 3 + o->number*47) % 360);
			tx = player->x + sine[angle] * 250;  
			ty = player->y + cosine[angle] * 250;
	
			if (ty > gy)
				ty -= 100;

			o->tsd.gdb.tx = tx;
			o->tsd.gdb.ty = ty;
		}

		/* avoid the ground. */
		if (o->y > gy - 100 && dvy > -3)
			dvy = -10;	

		/* adjust velocity towards desired velocity. */
		if (o->vx < dvx)
			o->vx++;
		else if (o->vx > dvx)
			o->vx--;
		if (o->vy < dvy)
			o->vy++;
		else if (o->vy > dvy)
			o->vy -= 2;

		o->x += o->vx; /* move... */
		o->y += o->vy;

		o->vy++; /* gravity */

		/* shoot out some exhaust. */	
		explode(o->x - dvx + randomn(4)-2, o->y - dvy + randomn(4)-2, -dvx, -dvy, 4, 8, 9);
	}

	/* launch a missile? */	
	xdist = abs(o->x - player->x);
	if (xdist < GDB_LAUNCH_DIST) {
		ydist = o->y - player->y;
		if (randomn(1000) < SAM_LAUNCH_CHANCE && timer >= o->missile_timer) {
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_harpoon(o->x+10, o->y, 0, 0, 300, MAGENTA, player, o);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
			if (!o->tsd.gdb.awake) {
				/* if we weren't awake when we fired the missile, we are now. */
				o->tsd.gdb.awake = 1;
				o->tsd.gdb.tx = player->x + randomn(200)-100;
				o->tsd.gdb.ty = player->y + randomn(200)-100;
			}
		}
	}

	/* If GDB's smash into the ground... they die. */
	/* Somewhat counteracts their bad-assitude.... */
	if (o->y >= gy + 3) {
		kill_object(o);
		explode(o->x, o->y, o->vx, 1, 70, 150, 20);
		o->destroy(o);
	}
}

void humanoid_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive) {
		printf("Bug... Dead human %d\n", o->tsd.human.human_number);
		return;
	}
	if (o->alive != 1) printf("human%d, alive=%d\n", o->tsd.human.human_number, o->alive);

	/* humans move around with their abductors. */
	if (o->tsd.human.picked_up && o->tsd.human.abductor != NULL) {
		o->x = 10 + o->tsd.human.abductor->x + (o->tsd.human.seat_number * 8) - (game_state.humanoids * 4);
		// o->x = o->tsd.human.abductor->x + 10 * (o->tsd.human.human_number - ((level.nhumanoids >> 1)));
		o->y = o->tsd.human.abductor->y + 30;
	}

	/* not on ground, not picked up -- we're falling... */
	if (o->tsd.human.on_ground == 0 && o->tsd.human.picked_up == 0) {
		int gy;
		/* we got dropped */
		if ((timer & 0x03) == 0)
			o->vy++; /* make them easier to catch. */
		o->y += o->vy;
		o->x += o->vx;
		gy = find_ground_level(o, NULL);
		if (gy == GROUND_OOPS || o->y >= gy) {
			o->y = gy;
			o->tsd.human.on_ground = 1;
			add_sound(BODYSLAM_SOUND, ANY_SLOT);
			if (abs(o->x - volcano_obj->x) > 20)
				add_sound(OOF_SOUND, ANY_SLOT);
			else {
				add_sound(IT_BURNS, ANY_SLOT);
				kill_object(o);
				game_state.score -= 1000;
				return;
			}
		}
	}

	xdist = abs(o->x - player->x);
	ydist = abs(o->y - player->y);
	if (xdist < HUMANOID_DIST && o->tsd.human.picked_up == 0) {
		if (ydist < HUMANOID_DIST) {	/* close enough for pickup? */
			if (o->tsd.human.on_ground == 0 && 
				o->tsd.human.picked_up == 0) { /* midair catch */
				add_floater_message(o->x, o->y + 25, "Nice Catch! +1000!");
				game_state.score += 1000;
			}
			add_sound(CARDOOR_SOUND, ANY_SLOT);
			add_sound(WOOHOO_SOUND, ANY_SLOT);
			add_floater_message(o->x, o->y, "Woohoo!");
			o->x = -1000; /* take him off screen. */
			o->y = -1000;
			o->tsd.human.abductor = player;
			o->tsd.human.picked_up = 1;
			o->tsd.human.seat_number = game_state.humanoids;
			o->tsd.human.on_ground = 0;
			game_state.score += HUMANOID_PICKUP_SCORE;
			game_state.humanoids++;
		} else {
			/* counter tracks if player has left the general vicinity of the human lately. */
			/* keeps human from saying "Help up/down here!" more than once at a time. */
			/* If the player is in the middle of falling, he doesn't say anything. */ 
			/* Too busy screaming. */
			if (o->counter == 0 && (o->tsd.human.on_ground != 0 || 
				o->tsd.human.picked_up != 0)) {
				add_floater_message(o->x, o->y, "Help!");
				if (o->y > player->y)
					add_sound(HELPDOWNHERE_SOUND, ANY_SLOT);
				else
					add_sound(HELPUPHERE_SOUND, ANY_SLOT);
				o->counter = 1;
			}
		}
	} else if (xdist > 1000) 
		o->counter = 0; /* reset counter if player is far enough away. */
}

void advance_level();
void socket_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	/* see if player entered ths socket... */
	xdist = abs(o->x - player->x);
	ydist = abs(o->y - player->y);
	/* HUMANOID_DIST is close enough */
	if (xdist < HUMANOID_DIST && ydist < HUMANOID_DIST 
		&& timer_event != START_INTERMISSION_EVENT) {
		gettimeofday(&game_state.finish_time, NULL);
		add_sound(WOOHOO_SOUND, ANY_SLOT);
		player->tsd.epd.count = 50;
		player->tsd.epd.count2 = 0;
		player->vx = 0;
		player->vy = 0;
		player->x = 0; /* so we won't enter the socket again immediately */
		timer_event = START_INTERMISSION_EVENT;
		next_timer = timer + 1;
		// advance_level();
	}
	if (xdist < SCREEN_WIDTH)
		o->color = randomn(NCOLORS + NSPARKCOLORS + NRAINBOWCOLORS);
}

static inline void free_object(int i)
{
	game_state.go[i].alive = 0;
	free_obj_bitmap[i >> 5] &= ~(1 << (i % 32)); /* clear the proper bit. */
}

static inline void age_object(struct game_obj_t *o)
{
	o->alive--;
	if (o->alive <= 0)
		free_object(o->number);	
}

static inline void kill_object(struct game_obj_t *o)
{
	o->alive = 0;
	free_object(o->number);
}

int find_free_obj();

void laser_move(struct game_obj_t *o);
void laser_draw(struct game_obj_t *o,  GtkWidget *w);
void generic_destroy_func(struct game_obj_t *o);

void player_fire_laser()
{
	int i;
	struct game_obj_t *o, *p;
	int j;
	int y;

	if (timer < game_state.nextlasertime)
		return;
	game_state.nextlasertime = timer + (frame_rate_hz / 12);

	p = &game_state.go[0];

	/* Fire laser... cmd muliplier times. */
	y = p->y - ((game_state.cmd_multiplier-1)/2) * 10;
	for (j=0;j<game_state.cmd_multiplier;j++) {
		i = find_free_obj();
		o = &game_state.go[i];

		if (p != player) {
			printf("p != player!\n");
		} 

		o->last_xi = -1;
		o->x = p->x+(0 * game_state.direction);
		o->y = y;
		y += 10;
		o->vx = p->vx + LASER_SPEED * game_state.direction;
		o->vy = 0;
		o->v = &right_laser_vect;
		o->draw = laser_draw;
		o->move = laser_move;
		o->destroy = generic_destroy_func;
		o->otype = OBJ_TYPE_LASER;
		o->color = game_state.nextlasercolor;
		o->alive = 20;
		o->target = NULL;
	
		game_state.nextlasercolor++;
		if  (game_state.nextlasercolor >= NCOLORS + NSPARKCOLORS + NRAINBOWCOLORS)
			game_state.nextlasercolor = NCOLORS + NSPARKCOLORS;
	}
	game_state.cmd_multiplier = 1;
	add_sound(PLAYER_LASER_SOUND, ANY_SLOT);
}

int interpolate(int x, int x1, int y1, int x2, int y2)
{
	/* return corresponding y on line x1,y1,x2,y2 for value x */
	/*
		(y2 -y1)/(x2 - x1) = (y - y1) / (x - x1)     by similar triangles.
		(x -x1) * (y2 -y1)/(x2 -x1) = y - y1	     a little algebra...
		y = (x - x1) * (y2 - y1) / (x2 -x1) + y1;    I think there's one more step
	                                                     which would optimize this a bit more.
							     but I forget how it goes. 
	*/
	if (x2 == x1)
		return y1;
	else
		return (x - x1) * (y2 - y1) / (x2 -x1) + y1;
}

int ground_level(int x, int *xi, int *slope)
{

/* Find the level (y value) of the ground at position x, 
 * return index into terrain array in *xi.  */
	int deepest, i;

	*xi = -1;
	deepest = GROUND_OOPS;

	/* find the terrain segment spanning x. */
	for (i=0;i<TERRAIN_LENGTH-1;i++) {
		if (x >= terrain.x[i] && x < terrain.x[i+1]) {
			*xi = i; /* tell caller the index, and find the exact y value. */
			deepest = interpolate(x, terrain.x[i], terrain.y[i],
					terrain.x[i+1], terrain.y[i+1]);

			if (slope != NULL) {
				/* calculate the slope of the ground at point of impact. */
				/* we use this later when bouncing things, and making them */
				/* slide downhill. */
				*slope = terrain.slope[i];
			}
			break;
		}
	}
	return deepest;
}

int find_ground_level(struct game_obj_t *o, int *slope)
{

	/* optimized way to find the ground level (y value) at an object's */
	/* x position.  Each object tracks the terrain segment it was last at. */
	/* This means, the linear search of the terrain array is mostly eliminated */

	int xi1, xi2, i;
	xi1 = o->last_xi; /* try the terrain segment we used last time. */
	xi2 = xi1 + 1;

	if (xi1 < 0 || xi2 >= TERRAIN_LENGTH) /* Do we have a last one? No? */
		return ground_level(o->x, &o->last_xi, slope); /* do it the hard way. */

	/* Is the last terrain segment the correct one? */
	if (terrain.x[xi1] <= o->x && terrain.x[xi2] >= o->x) {
		if (slope != NULL) {
			/* calculate the slope of the ground at point of impact. */
			/* we use this later when bouncing things, and making them */
			/* slide downhill. */
			*slope = terrain.slope[xi1];
		}
		return interpolate(o->x, terrain.x[xi1], terrain.y[xi1],  /* do it the easy way. */
				terrain.x[xi2], terrain.y[xi2]);
	}

	/* The last one wasn't correct.  Have to search. */
	/* The correct one is to the left.  Search left. */
	if (terrain.x[xi1] > o->x) {
		for (i=xi1;i>=0;i--) {
			if (o->x >= terrain.x[i] && o->x < terrain.x[i+1]) {
				o->last_xi = i;
				if (slope != NULL) {
					/* calculate the slope of the ground at point of impact. */
					/* we use this later when bouncing things, and making them */
					/* slide downhill. */
					*slope = terrain.slope[i];
				}
				return interpolate(o->x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	} else if (terrain.x[xi2] < o->x) { /* Correct one is to the right. */
		for (i=xi1;i<=TERRAIN_LENGTH-10;i++) { /* search to the right. */
			if (o->x >= terrain.x[i] && o->x < terrain.x[i+1]) {
				o->last_xi = i;
				if (slope != NULL) {
					/* calculate the slope of the ground at point of impact. */
					/* we use this later when bouncing things, and making them */
					/* slide downhill. */
					*slope = terrain.slope[i];
				}
				return interpolate(o->x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	}

	/* What?  there *is* no correct answer.  Object must have fallen off */
	/* the edge of the planet. (it happens sometimes -- though that is a bug. ) */
	o->last_xi = GROUND_OOPS;
	return GROUND_OOPS;
	
}

void generic_destroy_func(struct game_obj_t *o)
{
	/* so far, nothing needs to be done in this. */
	return;
}

void octopus_destroy(struct game_obj_t *o)
{
	int i;

	/* when an octopus is destroyed, his tentacles aren't.  They fall off */
	/* and proceed on their own. */
	for (i=0;i<8;i++) {
		struct game_obj_t *tentacle;
		tentacle = o->tsd.octopus.tentacle[i];
		if (tentacle) {
			tentacle->tsd.tentacle.attached_to = NULL;
			tentacle->vx = randomn(40)-20;
			tentacle->vy = randomn(40)-20;
			tentacle->tsd.tentacle.upper_angle = 170;
			tentacle->tsd.tentacle.lower_angle = 10;
			o->tsd.octopus.tentacle[i] = NULL;
		}
	}
}

void cron_destroy(struct game_obj_t *o)
{
	/* Drop any human the cron job is carrying. */
	if (o->tsd.cron.myhuman != NULL && 
		o->tsd.cron.myhuman->tsd.human.picked_up &&
		o->tsd.cron.myhuman->tsd.human.abductor == o) {
		o->tsd.cron.myhuman->tsd.human.abductor = NULL;
		o->tsd.cron.myhuman->tsd.human.picked_up = 0;
		o->tsd.cron.myhuman->tsd.human.on_ground = 0;
		/* printf("Human released, alive=%d, x=%d, y=%d.\n", o->tsd.cron.myhuman->x, o->tsd.cron.myhuman->y,
			o->tsd.cron.myhuman->alive); */
		o->tsd.cron.myhuman->vx = o->vx; /* <-- this doesn't seem to be working... FIXME */
		o->tsd.cron.myhuman = NULL;
		add_sound(SCREAM_SOUND, ANY_SLOT);
	}
	generic_destroy_func(o);
}

void bridge_move(struct game_obj_t *o);
void no_move(struct game_obj_t *o);
static void add_score_floater(int x, int y, int score);
static void spray_debris(int x, int y, int vx, int vy, int r, struct game_obj_t *victim, int metal);

void bomb_move(struct game_obj_t *o)
{
	struct target_t *t;
	int deepest;
	int dist2;
	int removed;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;
	o->vy++; /* gravity */

	/* Scan the target list to see if we've hit anything. */
	for (t=target_head;t != NULL;) {
		if (t->o == o) { /* bomb_move things can't bomb them selves. */
			t = t-> next;
			continue;
		}
		if (!t->o->alive) {
			t=t->next;
			continue;
		}
		removed = 0;
		switch (t->o->otype) {
			case OBJ_TYPE_TRUSS:
				dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
					(o->y - t->o->y)*(o->y - t->o->y);
				if (dist2 < LASER_PROXIMITY) { /* a hit (LASER_PROXIMITY is already squared.) */
					struct game_obj_t *i;

					/* Cut loose everything below */
					for (i=t->o; i != NULL; i = i->tsd.truss.below) {

						if (i->otype == OBJ_TYPE_TRUSS) {
							if (i->tsd.truss.above) {
								i->tsd.truss.above->tsd.truss.below = NULL;
								i->tsd.truss.above = NULL;
							}
							i->move = bridge_move;
						} else {
							/* make the gun blow up when it lands */
							i->move = bomb_move; 
							i->vx = randomn(6)-3;
							i->vy = randomn(6)-3;
							break;
						}
						i->vx = randomn(6)-3;
						i->vy = randomn(6)-3;
					}
				}
				/* Fall thru */	
			case OBJ_TYPE_ROCKET:
			case OBJ_TYPE_MISSILE:
			case OBJ_TYPE_HARPOON:
			case OBJ_TYPE_CRON:
			case OBJ_TYPE_GDB:
			case OBJ_TYPE_OCTOPUS:
			case OBJ_TYPE_FUEL:
			case OBJ_TYPE_JAMMER:
			case OBJ_TYPE_GUN:
			case OBJ_TYPE_KGUN:
			/* case OBJ_TYPE_BOMB:  no, bomb can't bomb himself... */
			case OBJ_TYPE_SAM_STATION:  {
				/* find distance squared... don't take square root. */
				dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
					(o->y - t->o->y)*(o->y - t->o->y);
				if (dist2 < LASER_PROXIMITY) { /* a hit (LASER_PROXIMITY is already squared.) */
					if (t->o->otype == OBJ_TYPE_ROCKET) {
						game_state.score += ROCKET_SCORE;
						add_score_floater(t->o->x, t->o->y, ROCKET_SCORE);
					} else if (t->o->otype == OBJ_TYPE_SAM_STATION) { 
						game_state.score += SAM_SCORE;
						add_score_floater(t->o->x, t->o->y, SAM_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GDB) { 
						game_state.score += GDB_SCORE;
						add_score_floater(t->o->x, t->o->y, GDB_SCORE);
					} else if (t->o->otype == OBJ_TYPE_KGUN) { 
						game_state.score += FLAK_SCORE;
						add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GUN) { 
						game_state.score += FLAK_SCORE;
						add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
					} else if (t->o->otype == OBJ_TYPE_OCTOPUS) { 
						game_state.score += OCTOPUS_SCORE;
						add_score_floater(t->o->x, t->o->y, OCTOPUS_SCORE);
					} else if (t->o->otype == OBJ_TYPE_CRON) { 
						game_state.score += CRON_SCORE;
						add_score_floater(t->o->x, t->o->y, CRON_SCORE);
					}
					add_sound(BOMB_IMPACT_SOUND, ANY_SLOT);
					explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
					spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 70, t->o, 1);
					// t->o->alive = 0;
					kill_object(t->o);
					t->o->destroy(t->o);
					t = remove_target(t);
					removed = 1;
					kill_object(o);
					// o->alive = 0;
					o->destroy(o);

					/* I think this stuff should be moved into the above similar ifs... */
					if (t->o->otype == OBJ_TYPE_SAM_STATION)
						game_state.sams_killed++;
					else if (t->o->otype == OBJ_TYPE_ROCKET)
						game_state.rockets_killed++;
					else if (t->o->otype == OBJ_TYPE_MISSILE || 
						t->o->otype == OBJ_TYPE_HARPOON)
						game_state.missiles_killed++;
					else if (t->o->otype == OBJ_TYPE_OCTOPUS)
						game_state.octos_killed++;
					else if (t->o->otype == OBJ_TYPE_GDB)
						game_state.gdbs_killed++;
					else if (t->o->otype == OBJ_TYPE_CRON)
						game_state.crons_killed++;
					else if (t->o->otype == OBJ_TYPE_GUN)
						game_state.guns_killed++;
				}
			}
			default:
				break;
		}
		/* Careful.  if we removed a target, then the current one *is* the "next" one. */
		if (!removed)  
			t=t->next;
	}

	/* Detect smashing into the ground */
	deepest = find_ground_level(o, NULL);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		kill_object(o);
		o->destroy(o);
		add_sound(BOMB_IMPACT_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, 1, 90, 150, 20);
		/* find nearby targets */
		for (t=target_head;t != NULL;) {
			if (!t->o->alive) {
				t=t->next;
				continue;
			}
			removed = 0;
			switch (t->o->otype) {
				case OBJ_TYPE_TENTACLE: 
				case OBJ_TYPE_ROCKET:
				case OBJ_TYPE_HARPOON:
				case OBJ_TYPE_MISSILE:
				case OBJ_TYPE_GDB:
				case OBJ_TYPE_CRON:
				case OBJ_TYPE_OCTOPUS:
				case OBJ_TYPE_GUN:
				case OBJ_TYPE_KGUN:
				case OBJ_TYPE_BOMB:
				case OBJ_TYPE_JAMMER:
				case OBJ_TYPE_SAM_STATION:
				case OBJ_TYPE_FUEL: {
					dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
						(o->y - t->o->y)*(o->y - t->o->y);
					if (dist2 < BOMB_PROXIMITY) { /* a hit */
						/* FIXME -- need to adjust kill counts. */
						if (t->o->otype == OBJ_TYPE_ROCKET) {
							game_state.score += ROCKET_SCORE;
							add_score_floater(t->o->x, t->o->y, ROCKET_SCORE);
						} else if (t->o->otype == OBJ_TYPE_SAM_STATION) { 
							game_state.score += SAM_SCORE;
							add_score_floater(t->o->x, t->o->y, SAM_SCORE);
						} else if (t->o->otype == OBJ_TYPE_CRON) { 
							game_state.score += GDB_SCORE;
							add_score_floater(t->o->x, t->o->y, GDB_SCORE);
						} else if (t->o->otype == OBJ_TYPE_KGUN) { 
							game_state.score += FLAK_SCORE;
							add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
						} else if (t->o->otype == OBJ_TYPE_GUN) { 
							game_state.score += FLAK_SCORE;
							add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
						} else if (t->o->otype == OBJ_TYPE_OCTOPUS) { 
							game_state.score += OCTOPUS_SCORE;
							add_score_floater(t->o->x, t->o->y, OCTOPUS_SCORE);
						} else if (t->o->otype == OBJ_TYPE_TENTACLE) {
							game_state.score += TENTACLE_SCORE;
						}
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
						spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 70, t->o, 1);
						kill_object(t->o);
						// t->o->alive = 0;
						t->o->destroy(t->o);
						/* if (t->o->otype == OBJ_TYPE_FUEL) {
							game_state.health += 10;
							if (game_state.health > MAXHEALTH)
								game_state.health = MAXHEALTH;
						} */
						t = remove_target(t);
						removed = 1;
					}
				}
				break;

				/* this is where pieces of bridges hit by bombs get set in motion. */
				case OBJ_TYPE_BRIDGE:
				case OBJ_TYPE_DEBRIS: /* <--- this doesn't seem to work.  FIXME */
					if (abs(o->x - t->o->x) < BOMB_X_PROXIMITY) { /* a hit */
						/* "+=" instead of "=" in case multiple bombs */
						if (t->o->move == no_move) /* only get the points once. */
							game_state.score += BRIDGE_SCORE;
						t->o->vx += ((t->o->x < o->x) ? -1 : 1) * randomn(6);
						t->o->vy += ((t->o->y < o->y) ? -1 : 1) * randomn(6);
						t->o->move = bridge_move;
						t->o->alive = 20;
					}
					break;
				default:
					break;
			}
			/* careful, if target removed, next == current. */
			if (!removed)
				t = t->next;
		}
	}
	/* if bomb exploded, it's dead and gone. */
	if (!o->alive) {
		remove_target(o->target);
		o->target = NULL;
		/* FIXME.  Don't we need to kill_object() here? */
	}
}

// static void corrosive_atmosphere_sound();
void volcano_move(struct game_obj_t *o)
{
	int player_dist;

	player_dist = abs(player->x - o->x);
	if (timer % (frame_rate_hz*2) == 0) {
		if (randomn(100) < 25)  {
			spray_debris(o->x, o->y, 0, -30, 30, o, 0);
			spray_debris(o->x, o->y, 0, -20, 20, o, 0);
			spray_debris(o->x, o->y, 0, -10, 10, o, 0);
			if (player_dist < 1500)
				add_sound(VOLCANO_ERUPTION, ANY_SLOT);
		} else {
			if (randomn(100) < 50) {
				spray_debris(o->x, o->y, 0, -10, 10, o, 0);
				if (player_dist < 1500)
					add_sound(VOLCANO_ERUPTION, ANY_SLOT);
			}
		}
	}
	if (player_dist < 250) {
		game_state.corrosive_atmosphere = 1;
		// corrosive_atmosphere_sound();
		if ((timer & 0x03) == 3) 
			game_state.health--;
	} else
		game_state.corrosive_atmosphere = 0;
}

/* Consider removing the entire concept of chaff from the game. */
/* I think it may harm game play fun, by confusing the missiles */
/* too easily.  At the very least, fix the missiles so they */
/* aren't *permanently* confused by the chaff. */
void chaff_move(struct game_obj_t *o)
{
	int deepest;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;
	o->vy++; /* gravity */
	age_object(o);

	/* air resistance reduces horizontal velocity with time. */
	if (o->vx > 0)
		o->vx--;
	else if (o->vx < 0);
		o->vx++;

	explode(o->x, o->y, 0, 0, 10, 7, 19);	/* throw some sparks.*/
	deepest = find_ground_level(o, NULL); 	/* Detect smashing into the ground */
	if (deepest != GROUND_OOPS && o->y > deepest) {
		kill_object(o);
		o->destroy(o);
	}
	if (!o->alive) {
		remove_target(o->target);
		o->target = NULL;
	}
}

void drop_chaff()
{
	int j, i[3];
	struct game_obj_t *o;
	struct target_t *t;

	if (game_state.nextchafftime > timer)
		return;
	game_state.nextchafftime = timer + (frame_rate_hz);

	/* Throw three pieces of chaff each time... */
	for (j=0;j<3;j++) {
		i[j] = find_free_obj();
		if (i[j] < 0)
			continue;
		o = &game_state.go[i[j]];
		o->last_xi = -1;
		o->x = player->x;
		o->y = player->y;
		o->vx = player->vx + ((j-1) * 7); /* -7, 0, 7 are x velocities */
		o->vy = player->vy + 7;
		o->v = &spark_vect; /* used? */
		o->move = chaff_move;
		o->otype = OBJ_TYPE_CHAFF;
		o->target = add_target(o);
		o->color = ORANGE;
		o->alive = 25;
	}
	/* find any missiles in the vicinity */
	for (t=target_head;t != NULL;t=t->next) {
		if (t->o->otype != OBJ_TYPE_MISSILE)
			continue;
		/* make the missile chase the chaff instead of the player -- sometimes. */
		if (t->o->bullseye == player) {
			j = randomn(3);
			if (j >= 0 && j <= 3 && i[j] > 0 && randomn(100) < 50)
				t->o->bullseye = &game_state.go[i[j]];
		}
	}
	/* FIXME Bug: when (bullseye->alive == 0) some new object will allocate there
	   and become the new target... probably a spark.  The missiles will not 
	   re-aquire the proper target. */
}

void drop_bomb()
{
	int i, j;
	struct game_obj_t *o;

	if (game_state.nextbombtime > timer)
		return;
	game_state.nextbombtime = timer + (frame_rate_hz >> 2);

	/* Player drops cmd_multiplier bombs.  */
	for (j=0;j<game_state.cmd_multiplier;j++) {
		if (game_state.nbombs == 0)
			return;
		game_state.nbombs--;
	
		i = find_free_obj();
		if (i < 0)
			return;

		o = &game_state.go[i];
		o->last_xi = -1;
		o->x = player->x+(5 * game_state.direction);
		o->y = player->y;
		o->vx = player->vx + BOMB_SPEED * game_state.direction + (j*3*game_state.direction);
		o->vy = player->vy;
		o->v = &bomb_vect;
		o->move = bomb_move;
		o->draw = NULL;
		o->destroy = generic_destroy_func;
		o->otype = OBJ_TYPE_BOMB;
		o->target = add_target(o);
		o->color = ORANGE;
		o->alive = 20;
	}
	game_state.cmd_multiplier = 1;
}

void player_draw(struct game_obj_t *o, GtkWidget *w)
{

	int i, countdir;
	double scale, scalefactor;

	if (player->tsd.epd.count == 0) /* normal case, just draw the player normally. */
		draw_generic(o, w);
	else {
		/* this is the insanity that makes the player "zoom" into the game */
		/* at the start of levels. */
		if (player->tsd.epd.count > 0) {
			scale = 1.07;
			scalefactor = 1.07;
			countdir = 1;
		} else {
			scale = 1.07;
			scalefactor = 1.07;
			countdir = -1;
		}

		/* Each frame, we draw a bunch of images of the player, each scaled */
		/* to a different size.  With each frame, the number of images reduces by 1.  */
		/* o->tsd.epd.count2 is this number of images. */
		/* o->tsd.epd.count is the number of frame iterations it takes to reduce */
		/* things down to normal. It's a bit funky, but it works. */
		for (i = 0; i<o->tsd.epd.count2; i++) {
			int j;
			int x1, y1, x2, y2;
			gdk_gc_set_foreground(gc, &huex[o->color]);
			for (j=0;j<o->v->npoints-1;j++) {
				if (o->v->p[j+1].x == LINE_BREAK) /* Break in the line segments. */
					j+=2;
				if (o->v->p[j].x == COLOR_CHANGE) {
					gdk_gc_set_foreground(gc, &huex[o->v->p[j].y]);
					j+=1;
				}
				x1 = o->x + o->v->p[j].x*scale - game_state.x;
				y1 = o->y + o->v->p[j].y*scale - game_state.y + (SCREEN_HEIGHT/2);  
				x2 = o->x + o->v->p[j+1].x*scale - game_state.x; 
				y2 = o->y + o->v->p[j+1].y*scale+(SCREEN_HEIGHT/2) - game_state.y;
				wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
			}
			scale = scale * scalefactor;
		}
		o->tsd.epd.count2 += countdir;
		if (o->tsd.epd.count2 == 0 && countdir == -1)
			o->tsd.epd.count = 0;
		if (o->tsd.epd.count != 0 && o->tsd.epd.count2 == o->tsd.epd.count && countdir == 1)
			o->tsd.epd.count = 0;
	}
}

void add_sound_low_priority(int which_sound);
static void add_stone_sound()
{
	int i;

	i = randomn(8);
	add_sound_low_priority(STONEBANG1 + i);
}

static void add_metal_sound()
{
	int i;

	i = randomn(16);
	add_sound_low_priority(METALBANG1 + i);
}

#if 0
static void corrosive_atmosphere_sound()
{
	static int nexttime = 0;

	/* Only allow this to enter the sound queue once per every 3 seconds */
	if (timer > nexttime) {
		nexttime = timer + frame_rate_hz*10;
		add_sound(CORROSIVE_SOUND, ANY_SLOT);
	}
}
#endif

static void ground_smack_sound()
{
	static int nexttime = 0;

	/* Only allow this to enter the sound queue once per every 3 seconds */
	if (timer > nexttime) {
		nexttime = timer + frame_rate_hz*3;
		add_sound(GROUND_SMACK_SOUND, ANY_SLOT);
		add_sound(OWMYSPINE_SOUND, ANY_SLOT);
	}
}

void no_draw(struct game_obj_t *o, GtkWidget *w);
void move_player(struct game_obj_t *o)
{
	int i;
	int deepest;
	static int was_healthy = 1; /* notice this is static. */
	o->x += o->vx; /* move him... */
	o->y += o->vy;

	if (game_state.health > 0) {
		if (credits > 0 && o->vy > 0) /* damp y velocity. */
			o->vy--;
		if (credits > 0 && o->vy < 0)
			o->vy++;
		was_healthy = 1;
	} else if (was_healthy) { /* has player _just_ died, just this frame? */
		was_healthy = 0;  /* remember he's died, for next time. */

		/* force 10 secs to elapse before another quarter can go in. */
		next_quarter_time = timer + (frame_rate_hz * 10);

		player->move = bridge_move; /* bridge move makes the player fall. */
		explode(player->x, player->y, player->vx, player->vy, 90, 350, 30); /* bunch of sparks. */
		player->draw = no_draw;				/* Make player invisible. */
		spray_debris(o->x, o->y, o->vx, o->vy, 70, o, 1);/* Throw hunks of metal around, */
		add_sound(LARGE_EXPLOSION_SOUND, ANY_SLOT);	/* and make a lot of noise */
		// printf("decrementing lives %d.\n", game_state.lives);
		game_state.lives--;				/* lost a life. */
		strcpy(textline[CREDITS].string, "");
		if (game_state.lives <= 0 || credits <= 0) { /* last life? */
			if (credits > 0) 
				credits--;			/* reduce credits */
			if (credits <= 0) {
				timer_event = GAME_ENDED_EVENT; /* game is over. */
				next_timer = timer + 30;
			} else {				/* game not over... */
				timer_event = READY_EVENT; 	/* back to beginning of level. */
				game_state.lives = 3;		/* 3 more lives, on next credit. */
				next_timer = timer + 60;	/* give them a little breather. */
			}
		} else {					/* same game, no new credit, no new lives */
			next_timer = timer + 30;		/* 30 sec breather */
			timer_event = READY_EVENT;		/* back to beginning of level. */
		}
	} 
	deepest = find_ground_level(player, NULL);

	/* if the player flies too slow, gravity starts pulling him down. */
	if (abs(o->vx) < 5 || game_state.health <= 0) {
		if (deepest != GROUND_OOPS) {
			if (o->y < deepest) 
				o->vy+=1;
			else if (o->vx < 0)
				o->vx += 1;
			else if (o->vx > 0)
				o->vx -= 1;
		}
		
		if (o->vy > MAX_VY)
			o->vy = MAX_VY;
		if (was_healthy)
			explode(o->x-(13 * game_state.direction), o->y, -(7*game_state.direction), 0, 7, 10, 9);
	} else
		if (was_healthy)
			/* make some exhaust */
			explode(o->x-(13 * game_state.direction), o->y, -((abs(o->vx)+7)*game_state.direction), 0, 10, 10, 9);


	/* This stuff is to make the viewport track the player */
	/* if he's facing right, then the viewport puts him at 1/3 */
	/* the way across the screen, if left, then 2/3rds across the */
	/* screen.  If he switches direction, the transition is not abrupt */
	/* instead the velocity of the viewport is adjusted relative to the */
	/* player's velocity so that the viewport and player kind of slide */
	/* relative to one another to get the viewport in the right place. */
	if (game_state.direction == 1) {
		if (player->x - game_state.x > SCREEN_WIDTH/3) {
			/* going off screen to the right... rein back in */
			/* game_state.x = player->x - 2*SCREEN_WIDTH/3; */
			game_state.vx = player->vx + 7;
		} else {
			game_state.x = player->x - SCREEN_WIDTH/3;
			game_state.vx = player->vx;
		}
	} else {
		if (player->x - game_state.x < 2*SCREEN_WIDTH/3) {
		/* going off screen to the left... rein back in */
			game_state.vx = player->vx - 7;
		} else {
			game_state.x = player->x - 2*SCREEN_WIDTH/3;
			game_state.vx = player->vx;
		}
	}

	if (player->y - game_state.y > 2*SCREEN_HEIGHT/6) {
		game_state.vy = player->vy + 7;
	} else if (player->y - game_state.y < -SCREEN_HEIGHT/6) {
		game_state.vy = player->vy - 7;
	} else
		game_state.vy = player->vy;

	/* Detect smashing into the ground */
	deepest = find_ground_level(player, NULL);
	if (deepest != GROUND_OOPS && player->y >= deepest) {
		/* keep player from sinking through the ground. */
		player->y = deepest;
		if (abs(player->vy) > 7) 
			player->vy = -0.65 * abs(player->vy);
		else
			player->vy = 0;
		if (player->vy < -15) {
			player->vy = -15;
		}

		/* if player smacks the ground too hard, sparks, noise, damage ensue. */
		if (abs(player->vx) > 5 || abs(player->vy) > 5) {
			ground_smack_sound();
			explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
			game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
			player->vy = -5;
			player->vx = player->vx >> 1;
		}
	}

	/* Detect smashing into sides and roof, similar to above ground detecting code */
	if (player->y < KERNEL_Y_BOUNDARY) {
		player->y = KERNEL_Y_BOUNDARY + 10;
		if (abs(player->vy) > 7) 
			player->vy = 0.65 * abs(player->vy);
		else
			player->vy = 0;
		player->vx = 0.65 * player->vx;
		if (player->vy > 15)
			player->vy = 15;
		if (abs(player->vx) > 5 || abs(player->vy) > 5) {
			ground_smack_sound();
			explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
			game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		}
	}
	if (player->x < 0) {
		ground_smack_sound();
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		player->x = 20;
		player->vx = 5;
	} else if (player->x > terrain.x[TERRAIN_LENGTH - 1]) {
		ground_smack_sound();	
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		player->x = terrain.x[TERRAIN_LENGTH - 1] - 20;
		player->vx = -5;
	}

	/* Check to see if radar is fritzed, or unfritzed. */
	if (game_state.health > RADAR_FRITZ_HEALTH) {
		if (game_state.radar_state == RADAR_FRITZED) {
			game_state.radar_state = RADAR_BOOTUP;
		}
	} else if (game_state.health <= RADAR_FRITZ_HEALTH) {
		if (game_state.radar_state != RADAR_FRITZED) {
			game_state.radar_state = RADAR_FRITZED;
		}
	}

	/* Autopilot, "attract mode", if credits <= 0 */
	if (credits <= 0) {
		for (i=0;i<TERRAIN_LENGTH;i++) {
			/* adjust player's vy to make him move towards a desired altitude. */
			/* which is MIN_ALT above the section of ground 100 units ahead of */
			/* the player */
			if (terrain.x[i] - player->x > 100 && (terrain.x[i] - player->x) < 300) {
				if (terrain.y[i] - player->y > MAX_ALT) {
					player->vy += 1;
					if (player->vy > 9)
						player->vy = 9;
				} else if (terrain.y[i] - player->y < MIN_ALT) {
					player->vy -= 1;
					if (player->vy < -9)
					player->vy = -9;
				} else if (player->vy > 0) player->vy--;
				else if (player->vy < 0) player->vy++;
				game_state.vy = player->vy;
				break;
			}
			/* keep moving */
			if (player->vx < PLAYER_SPEED)
				player->vx++; 
		}
		if (randomn(40) < 4)	/* fire laser randomly */
			player_fire_laser();
		if (randomn(100) < 2)	/* drop bombs randomly */
			drop_bomb();
	}
	/* End attract mode */
}

void bounce(int *vx, int *vy, int slope, double bouncefactor)
{

	/* When flaming hunks of debris from an explosion hit the ground, they bounce. */

	/* thought about doing the whole "find the normal, calculate a reflected angle */
	/* thing, but then I figured, what's bouncing is flaming hunks of metal wreckage */
	/* a simpler, faster algorithm with a bit of randomness thrown in will work just */
	/* as well.  Might have a bit too much bounciness in this algorithm, may want to */
	/* damp it just a bit. */

	if (slope > 25) {
		*vx = (*vx * bouncefactor) + (randomn(7));  /* bounce a bit to the right */
		*vy = -(*vy * bouncefactor); 
	} else if (slope < -25) {
		*vx = (*vx * bouncefactor) - (randomn(7));  /* bounce a bit to the left */
		*vy = -(*vy * bouncefactor); 
	} else {
		*vy = -(*vy * bouncefactor); 
		*vx = (*vx * bouncefactor) + (randomn(10)-6); /* bounce a bit left or right */
	}
	
	return;	
}

void bridge_move(struct game_obj_t *o) /* move bridge pieces when hit by bomb */
{
	int deepest;
	int slope = 0;

	o->x += o->vx;
	o->y += o->vy;
	o->vy++;		/* gravity */
	if (o->alive >1)	/* don't age all the way down to one, stop at 2. */
		age_object(o);

	/* Detect smashing into the ground */
	deepest = find_ground_level(o, &slope);
	if (deepest == GROUND_OOPS)
		return;

	if (o->otype == OBJ_TYPE_DEBRIS) {
		/* debris doesn't last forever, and disappears when age == 2. */
		if  (o->alive == 2) {
			kill_object(o);	
			remove_target(o->target);
		}
		/* flying debris will throw sparks for 4 seconds or until it hits the ground. */
		if (o->alive > frame_rate_hz*4 || o->y < deepest-2) 
			explode(o->x, o->y, 0, 0, 16, 3, 10);
	}		

	/* hit the ground? */
	if (o->y >= deepest) {
		/* if it's falling pretty fast, then bounce, taking slope into account... */
		if (o->vy > 4) {
			o->y = deepest-2;
			if (o->vy > 12 && o->otype == OBJ_TYPE_DEBRIS && abs(o->x - player->x) < 800) {
				if (o->tsd.debris.debris_type == DEBRIS_TYPE_METAL) /* metal? */
					add_metal_sound();
				else
					add_stone_sound();
			}
			bounce(&o->vx, &o->vy, slope, 0.3);
		} else {
			/* hit the ground, no bounce, stop. */
			o->y = deepest;
			o->vx = 0;
			o->vy = 0;

			/* But slide downhill if it's steep... */
			if (slope > 25 && o->alive > 1) {
				o->vx = 3;
				o->vy += 1;
			} else if (slope < -25 && o->alive > 1) {
				o->vx = -3;
				o->vy += 1;
			}
		}
		if (o->alive == 1) {
			/* FIXME, does this ever execute?  I think maybe it doesn't. */
			o->move = NULL;
			o->radar_image = 0;
		}
	}
}

void no_move(struct game_obj_t *o)
{
	return;
}

void laser_draw(struct game_obj_t *o,  GtkWidget *w)
{
	int x1, y1, x2;
	gdk_gc_set_foreground(gc, &huex[o->color]);
	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  

	if (o->vx > 0)
		x2 = x1 - (15) * (20 - o->alive);
	else
		x2 = x1 + (15) * (20 - o->alive);
	gdk_gc_set_foreground(gc, &huex[WHITE]);
	wwvi_draw_line(w->window, gc, x1, y1, x2, y1);
	gdk_gc_set_foreground(gc, &huex[o->color]);
	wwvi_draw_line(w->window, gc, x1, y1-1, x2, y1-1);
	wwvi_draw_line(w->window, gc, x1, y1+1, x2, y1+1);
}

void laser_move(struct game_obj_t *o)
{
	struct target_t *t;
	int hit;
	int removed;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;

	/* search the list of potential targets and see if we've hit anything. */
	for (t=target_head;t != NULL;) {
		if (!t->o->alive) {
			t = t->next;
			continue;
		}
		removed = 0;
		switch (t->o->otype) {
			case OBJ_TYPE_SHIP:
			case OBJ_TYPE_AIRSHIP:
				if (abs(o->x - t->o->x) < 3*60 &&	/* hit a blimp or clipper ship? */
					o->y - t->o->y <= 0 &&
					o->y - t->o->y > -50*3) {
					if (t->o->tsd.airship.pressure < 40)
						t->o->tsd.airship.pressure += 2;
					explode(o->x, o->y, o->vx/2, 1, 70, 20, 20); /* make sparks */
					kill_object(o);		/* get rid of laser beam object. */
					o->destroy(o);
					t->o->alive -= PLAYER_LASER_DAMAGE;	/* damage enemy */
					if (t->o->alive <= 0) {			/* killed him? */
						game_state.emacs_killed++;
						kill_object(t->o);
						t->o->destroy(t->o);
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
						spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 30, t->o, 1);
						add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);
						t = remove_target(t);
						removed = 1;
					}
				}
				break;
			case OBJ_TYPE_ROCKET:
			case OBJ_TYPE_HARPOON:
			case OBJ_TYPE_GDB:
			case OBJ_TYPE_CRON:
			case OBJ_TYPE_OCTOPUS:
			case OBJ_TYPE_FUEL:
			case OBJ_TYPE_JAMMER:
			case OBJ_TYPE_GUN:
			case OBJ_TYPE_KGUN:
			case OBJ_TYPE_BOMB:
			case OBJ_TYPE_SAM_STATION:
			case OBJ_TYPE_TRUSS:
			case OBJ_TYPE_MISSILE:{

				/* check y value first. */
				hit = 0;
				if (abs(o->y - t->o->y) <= LASER_Y_PROXIMITY) {
					if (o->vx > 0) {
						if (o->x > t->o->x && o->x - o->vx < t->o->x)
							hit=1;
					} else {
						if (o->x < t->o->x && o->x - o->vx > t->o->x)
							hit=1;
					}
				}
				// printf("dist2 = %d\n", dist2);
				if (hit) { /* a hit */
					if (t->o->otype == OBJ_TYPE_TRUSS) {
						struct game_obj_t *i;

						/* Cut loose everything below */
						for (i=t->o; i != NULL; i = i->tsd.truss.below) {
							if (i->otype == OBJ_TYPE_TRUSS) {
								if (i->tsd.truss.above) {
									i->tsd.truss.above->tsd.truss.below = NULL;
									i->tsd.truss.above = NULL;
								}
								i->move = bridge_move;
							} else {
								/* make the gun blow up when it lands */
								i->move = bomb_move; 
								i->vx = randomn(6)-3;
								i->vy = randomn(6)-3;
								break;
							}
							i->vx = randomn(6)-3;
							i->vy = randomn(6)-3;
						}
					}
					if (t->o->otype == OBJ_TYPE_ROCKET) {
						game_state.score += ROCKET_SCORE;
						game_state.rockets_killed++;
						add_score_floater(t->o->x, t->o->y, ROCKET_SCORE);
					} else if (t->o->otype == OBJ_TYPE_MISSILE ||
						t->o->otype == OBJ_TYPE_HARPOON)
						game_state.missiles_killed++;
					else if (t->o->otype == OBJ_TYPE_SAM_STATION) {
						game_state.sams_killed++;
						game_state.score += SAM_SCORE;
						add_score_floater(t->o->x, t->o->y, SAM_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GDB) {
						game_state.gdbs_killed++;
						game_state.score += GDB_SCORE;
						add_score_floater(t->o->x, t->o->y, GDB_SCORE);
					} else if (t->o->otype == OBJ_TYPE_CRON) {
						game_state.crons_killed++;
						game_state.score += CRON_SCORE;
						add_score_floater(t->o->x, t->o->y, CRON_SCORE);
					} else if (t->o->otype == OBJ_TYPE_KGUN) {
						game_state.guns_killed++;
						game_state.score += FLAK_SCORE;
						add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GUN) {
						game_state.guns_killed++;
						game_state.score += FLAK_SCORE;
						add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
					} else if (t->o->otype == OBJ_TYPE_OCTOPUS) {
						game_state.octos_killed++;
						game_state.score += OCTOPUS_SCORE;
						add_score_floater(t->o->x, t->o->y, OCTOPUS_SCORE);
					}
					add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);
					explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
					spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 70, t->o, 1);
					kill_object(t->o);
					/* if (t->o->otype == OBJ_TYPE_FUEL) {
						game_state.health += 10;
						if (game_state.health > MAXHEALTH)
							game_state.health = MAXHEALTH;
					} */
					t->o->destroy(t->o);
					t = remove_target(t);
					removed = 1;
					kill_object(o);
					o->destroy(o);
				}
			}
			break;
			case OBJ_TYPE_WORM:
				/* check y value first. */
				hit = 0;
				if (abs(o->y - t->o->y) <= LASER_Y_PROXIMITY) {
					if (o->vx > 0) {
						if (o->x > t->o->x && o->x - o->vx < t->o->x)
							hit=1;
					} else {
						if (o->x < t->o->x && o->x - o->vx > t->o->x)
							hit=1;
					}
				}
				if (!hit)
					break;

				explode(t->o->x, t->o->y, t->o->vx, 1, 70, 10, 20);
				/* cut the worm in half where it's hit. */
				/* Disconnect the top half: */
				if (t->o->tsd.worm.parent) {
					struct game_obj_t *parent;
					parent = t->o->tsd.worm.parent;

					parent->tsd.worm.child = NULL;
					if (parent->tsd.worm.parent == NULL) { /* just a head? */
						kill_object(parent);
						parent->destroy(parent);
						remove_target(parent->target);
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 25, 20);
						spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 70, t->o, 1);
					}
					t->o->tsd.worm.parent->tsd.worm.child = NULL;
				}
			
				/* Disconnect the bottom half */
				if (t->o->tsd.worm.child) {
					/* make new head become autonomous ... see worm_move() */
					struct game_obj_t *child;

					child = t->o->tsd.worm.child;

					child->tsd.worm.parent = NULL;
					child->tsd.worm.tx = child->x;
					child->tsd.worm.ty = child->y;
					child->tsd.worm.ltx = child->x;
					child->tsd.worm.lty = child->y;

					if (child->tsd.worm.child == NULL) {	
						kill_object(child);
						child->destroy(child);
						remove_target(child->target);
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 25, 20);
						spray_debris(t->o->x, t->o->y, t->o->vx, t->o->vy, 70, t->o, 1);
					}
				}

				/* blow away the segment we just hit. */
				add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);

				kill_object(t->o);

				t->o->destroy(t->o);
				t = remove_target(t);
				removed = 1;
				kill_object(o);
				o->destroy(o);
				break;
			default:
				break;
		}
		/* careful, if we removed something, next == current */
		if (!removed)
			t=t->next;
	}
	if (o->alive)
		age_object(o);
	// if (!o->alive) {
		//remove_target(o->target);
		//o->target = NULL;
	//}
}

void move_obj(struct game_obj_t *o)
{
	o->x += o->vx; /* generic move function */
	o->y += o->vy;
}

void draw_spark(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2;

	/* draw sparks using their velocities as offsets. */
	x2 = o->x - game_state.x; 
	if (x2 < 0 || x2 > SCREEN_WIDTH)
		return;

	x1 = x2 - o->vx;
	if (x1 < 0 || x1 > SCREEN_WIDTH)
		return;

	y2 = o->y + (SCREEN_HEIGHT/2) - game_state.y;
	y1 = y2 - o->vy;
	if (brightsparks) {
		gdk_gc_set_foreground(gc, &huex[WHITE]);
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		gdk_gc_set_foreground(gc, &huex[o->color]);
		if (abs(o->vx) > abs(o->vy)) {
			wwvi_draw_line(w->window, gc, x1, y1-1, x2, y2-1); 
			wwvi_draw_line(w->window, gc, x1, y1+1, x2, y2+1); 
		} else {
			wwvi_draw_line(w->window, gc, x1-1, y1, x2-1, y2); 
			wwvi_draw_line(w->window, gc, x1+1, y1, x2+1, y2); 
		}
	} else {
		gdk_gc_set_foreground(gc, &huex[o->color]);
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
	}
}

void draw_laserbolt(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2, dx, dy;

	/* very much like sparks... */
	x2 = o->x - game_state.x; 
	if (x2 < 0 || x2 > SCREEN_WIDTH)
		return;

	x1 = x2 - o->vx;
	if (x1 < 0 || x1 > SCREEN_WIDTH)
		return;

	y2 = o->y + (SCREEN_HEIGHT/2) - game_state.y;
	y1 = y2 - o->vy;
	dx = abs(x1-x2);
	dy = abs(y1-y2);
	gdk_gc_set_foreground(gc, &huex[WHITE]);
	wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
	gdk_gc_set_foreground(gc, &huex[o->color]);
	if (dx > dy) {
		wwvi_draw_line(w->window, gc, x1, y1+1, x2, y2+1); 
		wwvi_draw_line(w->window, gc, x1, y1-1, x2, y2-1); 
	} else {
		wwvi_draw_line(w->window, gc, x1+1, y1, x2+1, y2); 
		wwvi_draw_line(w->window, gc, x1-1, y1, x2-1, y2); 
	}
}

void draw_missile(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1;
	int dx, dy;

	/* similar to sparks, but with a constant length.  Offsets are precomputed */
	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  
	dx = dx_from_vxy(o->vx, o->vy);
	dy = -dy_from_vxy(o->vx, o->vy);
	gdk_gc_set_foreground(gc, &huex[o->color]);
	if (x1 > 0 && x1+dx*2 > 0)
		wwvi_draw_line(w->window, gc, x1, y1, x1+dx*2, y1+dy*2); 
}

void draw_harpoon(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2;
	int dx, dy;

	/* exactly like missiles, used to have a line back to the gdb which */
	/* fired the missile... but I didn't really like it, so commented it out. */
	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  
	x2 = o->tsd.harpoon.gdb->x - game_state.x;
	y2 = o->tsd.harpoon.gdb->y - game_state.y + (SCREEN_HEIGHT/2);  
	dx = dx_from_vxy(o->vx, o->vy);
	dy = -dy_from_vxy(o->vx, o->vy);
	gdk_gc_set_foreground(gc, &huex[o->color]);
	wwvi_draw_line(w->window, gc, x1, y1, x1+dx, y1+dy); 
	/* if (o->tsd.harpoon.gdb->alive && o->tsd.harpoon.gdb->otype == OBJ_TYPE_GDB) {
		x2 = o->tsd.harpoon.gdb->x - game_state.x;
		y2 = o->tsd.harpoon.gdb->y - game_state.y + (SCREEN_HEIGHT/2);  
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
	} */
}

void move_missile(struct game_obj_t *o)
{
	struct game_obj_t *target_obj;
	int dx, dy, desired_vx, desired_vy;
	int exvx,exvy,deepest;

	/* move one step... */
	o->x += o->vx;
	o->y += o->vy;

	/* detect smashing into the ground. */	
	deepest = find_ground_level(o, NULL);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		kill_object(o);
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	/* missiles don't last forever... when they run out of fuel, they explode. */
	age_object(o);
	if (o->alive <= 0) { 
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	/* Figure out where we're trying to go, dx,dy */
	target_obj = o->bullseye;
	if (target_obj == player) {
		/* this is so we know to sound the alarm. */
		game_state.missile_locked = 1; 
		/* printf("mlock1\n"); */
	}
	dx = target_obj->x + target_obj->vx - o->x;
	dy = target_obj->y + target_obj->vy - o->y;

	/* have we hit the target? */
	if ((abs(dx) < MISSILE_PROXIMITY) && (abs(dy) < MISSILE_PROXIMITY)) {
		/* We've hit the target */
		if (player == o->bullseye)
			game_state.health -= MISSILE_DAMAGE;
		else
			target_obj->alive -= MISSILE_DAMAGE;
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		kill_object(o);
		/* target_obj->alive -= MISSILE_DAMAGE; */
		if (target_obj->alive <= 0)
			kill_object(target_obj);
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		o->destroy(o);
		return;
	}

	/* by similar triangles, find desired vx/vy from dx/dy... */
	if (abs(dx) < abs(dy)) {
		desired_vy = ((dy < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
		if (dy != 0)
			desired_vx = desired_vy * dx / dy;
		else /* shouldn't happen */
			desired_vx = ((dx < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
	} else {
		desired_vx = ((dx < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
		if (dx != 0)
			desired_vy = desired_vx * dy / dx;
		else /* shouldn't happen */
			desired_vy = ((dy < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
	}

	/* Adjust velocity towards desired vx,vy, but, only once every other clock tick */
	if ((timer % 2) == 0) {
		if (o->vx < desired_vx)
			o->vx++;
		else if (o->vx > desired_vx)
			o->vx--;
		if (o->vy < desired_vy)
			o->vy++;
		else if (o->vy > desired_vy)
			o->vy--;
	}

	/* make some exhaust sparks. */
	exvx = -dx_from_vxy(o->vx, o->vy) + randomn(2)-1;
	exvy = dy_from_vxy(o->vx, o->vy) + randomn(2)-1;
	explode(o->x + exvx + randomn(4)-2, o->y + exvy + randomn(4)-2, exvx, exvy, 4, 8, 9);
}

void move_harpoon(struct game_obj_t *o)
{
	struct game_obj_t *target_obj;
	int dx, dy, desired_vx, desired_vy;
	int exvx,exvy,deepest;

	/* this is exactly like the missile code above. */
	/* Was trying to make the gdb's have a different weapon */
	/* but for now, it's just missiles. */

	/* move one step... */
	o->x += o->vx;
	o->y += o->vy;
	
	/* detect smashing into the ground. */	
	deepest = find_ground_level(o, NULL);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		kill_object(o);
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	/* missiles don't last forever... when they run out of fuel, they explode. */
	age_object(o);
	if (o->alive <= 0) { 
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	/* Figure out where we're trying to go, dx,dy */
	target_obj = o->bullseye;
	if (target_obj == player) {
		/* this is so we know to sound the alarm. */
		game_state.missile_locked = 1;
		/* printf("mlock2\n"); */
	}
	dx = target_obj->x + target_obj->vx - o->x;
	dy = target_obj->y + target_obj->vy - o->y;

	/* have we hit the target? */
	if ((abs(dx) < MISSILE_PROXIMITY) && (abs(dy) < MISSILE_PROXIMITY)) {
		/* We've hit the target */
		if (player == o->bullseye)
			game_state.health -= MISSILE_DAMAGE;
		else
			target_obj->alive -= MISSILE_DAMAGE;
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		kill_object(o);
		if (target_obj->alive <= 0)
			kill_object(target_obj);
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		o->destroy(o);
		return;
	}

	/* by similar triangles, find desired vx/vy from dx/dy... */
	if (abs(dx) < abs(dy)) {
		desired_vy = ((dy < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
		if (dy != 0)
			desired_vx = desired_vy * dx / dy;
		else /* shouldn't happen */
			desired_vx = ((dx < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
	} else {
		desired_vx = ((dx < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
		if (dx != 0)
			desired_vy = desired_vx * dy / dx;
		else /* shouldn't happen */
			desired_vy = ((dy < 0) ? -1 : 1 ) * MAX_MISSILE_VELOCITY;
	}

	/* Adjust velocity towards desired vx,vy, */
	/* unlike missiles -- every clock tick. */
	/* This might be a little too ferocious... */
	// if ((timer % 2) == 0) {
		if (o->vx < desired_vx)
			o->vx++;
		else if (o->vx > desired_vx)
			o->vx--;
		if (o->vy < desired_vy)
			o->vy++;
		else if (o->vy > desired_vy)
			o->vy--;
	// }

	/* make some exhaust sparks. */
	exvx = -dx_from_vxy(o->vx, o->vy) + randomn(2)-1;
	exvy = dy_from_vxy(o->vx, o->vy) + randomn(2)-1;
	explode(o->x + exvx + randomn(4)-2, o->y + exvy + randomn(4)-2, exvx, exvy, 4, 8, 9);
}

void move_bullet(struct game_obj_t *o)
{
	struct game_obj_t *target_obj;
	int dx, dy;
	int deepest;

	/* move one step... */
	o->x += o->vx;
	o->y += o->vy;

	/* bullets don't last forever... */
	age_object(o);	
	deepest = find_ground_level(o, NULL);
	if (o->alive <= 0 || (deepest != GROUND_OOPS && o->y > deepest)) {
		kill_object(o);
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	/* Figure out where we were trying to go, dx,dy */
	target_obj = o->bullseye;
	dx = target_obj->x + target_obj->vx - o->x;
	dy = target_obj->y + target_obj->vy - o->y;

	/* did we hit? */
	if ((abs(dx) < BULLET_PROXIMITY) && (abs(dy) < BULLET_PROXIMITY)) {
		/* We've hit the target */
		if (player == o->bullseye)
			game_state.health -= BULLET_DAMAGE;
		else
			target_obj->alive -= BULLET_DAMAGE;
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		kill_object(o);
		if (target_obj->alive <= 0)
			kill_object(target_obj);
		add_sound(CLANG_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 10, 10);
		o->destroy(o);
		return;
	}
}

static struct game_obj_t *add_generic_object(int x, int y, int vx, int vy,
	obj_move_func *move_func,
	obj_draw_func *draw_func,
	int color, 
	struct my_vect_obj *vect, 
	int target,  /* can this object be a target? hit by laser, etc? */
	char otype, 
	int alive)
{
	int j;
	struct game_obj_t *o;

	j = find_free_obj();
	if (j < 0)
		return NULL;
	o = &game_state.go[j];
	o->last_xi = -1; /* don't know, so use -1 at first, this gets fixed up later */
	o->x = x;
	o->y = y;
	o->vx = vx;
	o->vy = vy;
	o->move = move_func;
	o->draw = draw_func;
	o->destroy = generic_destroy_func;
	o->color = color;
	if (target)
		o->target = add_target(o);
	else
		o->target = NULL;
	o->v = vect;
	o->otype = otype;
	o->alive = alive;
	o->bullseye = NULL;
	o->missile_timer = 0;
	o->counter = 0;
	o->radar_image = 0;
	return o;
}

static void add_bullet(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye)
{
	int i;
	struct game_obj_t *o;

	i = find_free_obj();
	if (i<0)
		return;
	o = &game_state.go[i];
	o->x = x;
	o->y = y;
	o->vx = vx;
	o->vy = vy;
	o->move = move_bullet;
	o->draw = NULL;
	o->v = &bullet_vect;
	o->destroy = generic_destroy_func;
	o->bullseye = bullseye;
	o->target = add_target(o);
	o->otype = OBJ_TYPE_BULLET;
	o->color = color;
	o->alive = time;
	o->counter = 0;
}

static void add_missile(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye)
{
	int i;
	struct game_obj_t *o;

	i = find_free_obj();
	if (i<0)
		return;
	o = &game_state.go[i];
	o->last_xi = -1;
	o->x = x;
	o->y = y;
	o->vx = vx;
	o->vy = vy;
	o->move = move_missile;
	o->draw = draw_missile;
	o->destroy = generic_destroy_func;
	o->bullseye = bullseye;
	o->target = add_target(o);
	o->otype = OBJ_TYPE_MISSILE;
	o->color = color;
	o->alive = time;
	o->counter = 0;
}

static void add_harpoon(int x, int y, int vx, int vy, 
	int time, int color, struct game_obj_t *bullseye,
	struct game_obj_t *gdb)
{
	int i;
	struct game_obj_t *o;

	i = find_free_obj();
	if (i<0)
		return;
	o = &game_state.go[i];
	o->last_xi = -1;
	o->x = x;
	o->y = y;
	o->vx = vx;
	o->vy = vy;
	o->move = move_harpoon;
	o->draw = draw_harpoon;
	o->destroy = generic_destroy_func;
	o->bullseye = bullseye;
	o->target = add_target(o);
	o->otype = OBJ_TYPE_HARPOON;
	o->color = color;
	o->alive = time;
	o->counter = 0;
	o->tsd.harpoon.gdb = gdb;
}
void balloon_move(struct game_obj_t *o)
{
	int deepest;

	o->x += o->vx;
	o->y += o->vy;

	if (randomn(1000) < 5) {
		o->vx = randomab(1, 3) - 2;
		o->vy = randomab(1, 3) - 2;
	}

	deepest = find_ground_level(o, NULL);
	if (deepest != GROUND_OOPS && o->y > deepest - MIN_BALLOON_HEIGHT)
		o->vy = -1;
	else if (deepest != GROUND_OOPS && o->y < deepest - MAX_BALLOON_HEIGHT)
		o->vy = 1;
}

void floating_message_move(struct game_obj_t *o)
{

	/* this is a trivial move, if anything else needs this, factor it out */
	if (!o->alive)
		return;
	age_object(o);
	if (o->alive <= 0)
		o->destroy(o);
	o->x += o->vx;
	o->y += o->vy;
}

void symbol_move(struct game_obj_t *o) /* move bridge pieces when hit by bomb */
{
	int vx,vy;
	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;
	if ((timer % 80) == 1) {
		o->vx += randomab(1,4)-2;
		o->vy += randomab(1,4)-2;
	}
	if (abs(o->x - player->x) < 20 && abs(o->y - player->y) < 20) {
		vx = o->vx;
		vy = o->vy;
		o->vx = player->vx/2;
		o->vy = player->vy/2;
		player->vx -= vx/3;
		player->vy -= vy/3;
	}
	if (o->alive>0) {
		age_object(o);
		// if (!o->alive)
		//	remove_target(o->target);
	} else
		o->destroy(o);
}

static void aim_vx_vy(struct game_obj_t *target, 
		struct game_obj_t *shooter,
		int v, int leadtime, 
		int *vx, int *vy);

static void flying_thing_shoot_missile(struct game_obj_t *o)
{
	int gambling, xdist, ydist;

	xdist = abs(o->x - player->x);
	ydist = abs(o->y - player->y);
	if (xdist < SAM_LAUNCH_DIST && ydist < SAM_LAUNCH_DIST) {
		if (xdist < SAM_LAUNCH_DIST/3)
			gambling = 4;
		else if (xdist < SAM_LAUNCH_DIST/2)
			gambling = 2;
		else
			gambling = 0;
		
		if (randomn(2000) < (SAM_LAUNCH_CHANCE+gambling) && timer >= o->missile_timer) {
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_missile(o->x, o->y, 0, 0, 300, RED, player);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
		}

		if (randomn(2000) < (SAM_LAUNCH_CHANCE*2+gambling)) {
			int vx, vy;
			aim_vx_vy(player, o, 28, 10, &vx, &vy);
			add_laserbolt(o->x, o->y, vx, vy, RED, 50);
			add_sound(FLAK_FIRE_SOUND, ANY_SLOT);
		}
	}
}

static void add_symbol(int c, int myfont, int x, int y, int vx, int vy, int time);
void flying_thing_move(struct game_obj_t *o)
{
	int deepest;

	o->x += o->vx;
	o->y += o->vy;

	/* keep it in bounds */
	if (o->x < 150)
		o->vx = 1;
	if (o->x > terrain.x[TERRAIN_LENGTH-1]-150)
		o->vx = -1;

	if (randomn(1000) < 5) {
		o->vx = randomab(1, 3) - 2;
		o->vy = randomab(1, 3) - 2;
	}

	deepest = find_ground_level(o, NULL);
	if (deepest != GROUND_OOPS && o->y > deepest - MIN_BALLOON_HEIGHT)
		o->vy = -1;
	else if (deepest != GROUND_OOPS && o->y < deepest - MAX_BALLOON_HEIGHT)
		o->vy = 1;
}

void airship_leak_lisp(struct game_obj_t *o)
{
	/* The airships have "memory leaks" in which they spew "lisp code" */
	/* out into core memory... */
	if ((timer % 8) && randomn(100) < 5 + (o->tsd.airship.pressure)) {
		if (randomlisp[o->counter] != ' ') /* skip putting out "space" objects. */
			add_symbol(randomlisp[o->counter], TINY_FONT, o->x + 70*3, o->y - 30*3, 
				o->vx+4, 0, 200);
		o->counter--;
		if (o->counter < 0)
			o->counter = strlen(randomlisp)-1;
	}
}

static void abs_xy_draw_string(GtkWidget *w, char *s, int font, int x, int y);

/* this is the text which scrolls up on the side of the blimp. */
char *blimp_message[] = {
"",
"",
"        vi vi vi...",
"",
"   The editor of the",
"",
"       B E A S T !",
"",
"",
"    MMMMMMMMMMMMMMMMMMMMM     ", /* Richard Stallman as ascii art. */
" MMMMMMMMMWX0dxxKNWMMMMMMMMM  ", /* Done from a copyright-free photo */
"MMMMMMMM0:   'xO0kdXMMMMMMMMM ", /* found on wikipedia run through gimp */
"MMMMMMX,   .,lONWMxokWMMMMMMMM", /* to adjust contrast, and then jp2a */
"MMMMM0.   .,:cx0NMWlllNMMMMMMM", /* to convert the jpeg to ascii. */
"MMMMN.     ..:,.;0WXcloKWMMMMM", /* It looks better with light text */
"MMMMO    ... d0clKWMx,dloXMMMM", /* dark background -- hightlight it */
"MMMMO   .....;docOMMW:,;::OMMM", /* with the mouse to see for yourself. */
"MMMX,    ..   .,:dKKd;.::;;xMM",
"MMMK.         .'':cc,'..,,,;lX",
"l:,         ..'':;,;,',.;:,,:;",
"            ..',,,'cc;...,::c:",
"            .......co.... ',:;",
"                 .''.. .  ...;",
"                  .  .     . .",
"",
"       Bow down before",
"        St. IGNUcius,",
"         heathen!!!",
"",
"",
"",
"  Using vi is not a sin,",
"",
"    It is a penance!",
"",
"",
"",
"",
"    G N U   E M A C S",
"",
"        Rules!!!!",
"",
"     vi drools!!!!",
"",
"",
"",
"       XXX  X  X X   X",
"      X   X XX X X   X",
"      X     XX X X   X",
"      X XXX X XX X   X",
"      X   X X XX X   X",
"       XXX  X  X  XXX",
"",
"",
"",
NULL, /* mark the end. */
}; 
void airship_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x, y, i, line;

	if (o->tsd.airship.pressure == 0)
		draw_generic(o, w);
	else
		draw_scaled_generic(o, w, 1.0, 1.0 + 0.01 * o->tsd.airship.pressure);

	/* the -90 and -130 were arrived at empirically, as were the dimensions */
	/* of the text field.  The commented out code below with 0123456... is */
	/* part of the testing done to figure that out. */
	x = o->x - game_state.x - 90;
	y = o->y - game_state.y + (SCREEN_HEIGHT/2) - 130;

	/* 
	for (i=0;i<7;i++) {
		abs_xy_draw_string(w, "012345678901234567890123456789", NANO_FONT, x, y);
		y = y + 15;
	} */

	/* Write out 7 lines of text from the loop, starting at our current position. */
	line = o->tsd.airship.bannerline;
	for (i=0;i<7;i++) {
		if (blimp_message[line] == NULL)
			line = 0;
		abs_xy_draw_string(w, blimp_message[line], NANO_FONT, x, y);
		y = y + 15;
		line++;
	}

	/* advance to the next line 2x per second */
	if ((timer % 15) == 0) {
		o->tsd.airship.bannerline++;
		if (blimp_message[o->tsd.airship.bannerline] == NULL)
			o->tsd.airship.bannerline = 0;
	}

}

void worm_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x, y, x2,y2;
	int color;

	x = o->x - game_state.x;
	y = o->y - game_state.y + (SCREEN_HEIGHT/2);
	if (o->tsd.worm.parent == NULL) {
		/* int x3, y3; */
		gdk_gc_set_foreground(gc, &huex[o->color]);
		wwvi_draw_line(w->window, gc, x-2, y, x, y-2); 
		wwvi_draw_line(w->window, gc, x, y-2, x+2, y); 
		wwvi_draw_line(w->window, gc, x+2, y, x, y+2); 
		wwvi_draw_line(w->window, gc, x, y+2, x-2, y); 
/*
		x3 = o->tsd.worm.tx - game_state.x;
		y3 = o->tsd.worm.ty - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x, y, x3, y3); 
		x3 = o->tsd.worm.ltx - game_state.x;
		y3 = o->tsd.worm.lty - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x, y, x3, y3); 
*/
	} else {
		color = NCOLORS + NSPARKCOLORS + ((timer + o->tsd.worm.coloroffset) % NRAINBOWCOLORS);
		gdk_gc_set_foreground(gc, &huex[color]);
		if (o->tsd.worm.child == NULL) {
			wwvi_draw_line(w->window, gc, x-2, y, x, y-2); 
			wwvi_draw_line(w->window, gc, x, y-2, x+2, y); 
			wwvi_draw_line(w->window, gc, x+2, y, x, y+2); 
			wwvi_draw_line(w->window, gc, x, y+2, x-2, y); 
		}
	}

#if 0
	if (o->tsd.worm.parent == NULL) {
		x2 = o->tsd.worm.tx - game_state.x;
		y2 = o->tsd.worm.ty - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x, y, x2, y2);
		x2 = o->tsd.worm.ltx - game_state.x;
		y2 = o->tsd.worm.lty - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x, y, x2, y2);
	} else {
#endif
	if (o->tsd.worm.parent != NULL) {
		int delta1 = 7;
		int delta2 = 7;

		if (o->tsd.worm.parent->tsd.worm.parent == NULL)
			delta2 = 2;
		if (o->tsd.worm.child == NULL)
			delta1 = 2;

		x2 = o->tsd.worm.parent->x - game_state.x;
		y2 = o->tsd.worm.parent->y - game_state.y + (SCREEN_HEIGHT/2);
		wwvi_draw_line(w->window, gc, x-delta1, y, x2-delta2, y2);
		wwvi_draw_line(w->window, gc, x+delta1, y, x2+delta2, y2);
		wwvi_draw_line(w->window, gc, x, y-delta1, x2, y2-delta2);
		wwvi_draw_line(w->window, gc, x, y+delta1, x2, y2+delta2);
	}
}

void worm_move(struct game_obj_t *o)
{
	int xi, n, gy, pdx, pdy;
	int ldx, ldy, sdx, sdy, dvx, dvy;

	pdx = abs(o->x - player->x);
	pdy = abs(o->y - player->y);
	/* is this the head of a worm? Or just a body segment? */
	if (o->tsd.worm.parent == NULL) {
		/* It's a head...  think about where to go */

		/* long term destination reached?  Pick a new one. */
		ldx = o->tsd.worm.ltx - o->x;
		ldy = o->tsd.worm.lty - o->y;
		if (abs(ldx) < 20 && abs(ldy) < 20) {
                        o->tsd.worm.ltx = terrain.x[randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1)];
                        o->tsd.worm.lty = ground_level(o->tsd.worm.ltx, &xi, NULL) - 800 + randomn(700);

			// printf("New long term dest %d,%d\n", o->tsd.worm.ltx, o->tsd.worm.lty);

			/* trigger short term destination reached code, below. */
			o->tsd.worm.tx = o->x;
			o->tsd.worm.ty = o->y;
			ldx = o->tsd.worm.ltx - o->x;
			ldy = o->tsd.worm.lty - o->y;
		}

		/* short term destination reached?  Pick a new one. */
		sdx = o->tsd.worm.tx - o->x;
		sdy = o->tsd.worm.ty - o->y;
		if (abs(sdx) < 5 && abs(sdy) < 5) {


			/* If player is close, follow him... */
			if (pdx < 170 && pdy < 170) {
				o->tsd.worm.tx = player->x + player->vx * 10 + randomn(30) - 15;
				o->tsd.worm.ty = player->y + player->vy * 10 + randomn(30) - 15;

				/* Touching player?  Zap him. */
				if (pdx < 8 && pdy < 8) {
					add_sound(ZZZT_SOUND, ANY_SLOT);
					explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
					game_state.health -= WORM_DAMAGE;
				}

			} else {
				/* Aim vaguely towards the destination... */
				if (o->x < o->tsd.worm.ltx)
					o->tsd.worm.tx = o->x + randomn(200) - 30; 
				else
					o->tsd.worm.tx = o->x - randomn(200) + 30; 

				if (o->y < o->tsd.worm.lty)
					o->tsd.worm.ty = o->y + randomn(60) - 10; 
				else
					o->tsd.worm.ty = o->y - randomn(60) + 10; 
			}

			/* Avoid the ground. */
			gy = ground_level(o->tsd.worm.tx, &xi, NULL);	
			if (o->tsd.worm.ty >= gy)
				o->tsd.worm.ty = gy - 50 - randomn(200);
			if (o->tsd.worm.ty <= KERNEL_Y_BOUNDARY)
				o->tsd.worm.ty = KERNEL_Y_BOUNDARY + 50 + randomn(200);
			
		}

		/* Calculate desired velocity... */
		sdx = o->tsd.worm.tx - o->x;
		sdy = o->tsd.worm.ty - o->y;
		if (abs(sdx) > abs(sdy)) {
			if (sdx > 0) {
				dvx = MAX_WORM_VX;
				dvy = (sdy * MAX_WORM_VY) / sdx;
			} else if (sdx < 0) {
				dvx = -MAX_WORM_VX;
				dvy = (sdy * MAX_WORM_VY) / abs(sdx);
			} else {
				dvx = 0;
				dvy = 0;
			}
		} else {
			if (sdy > 0) {
				dvy = MAX_WORM_VY;
				dvx = (sdx * MAX_WORM_VX) / sdy;
			} else if (sdy < 0) {
				dvy = -MAX_WORM_VY;
				dvx = (sdx * MAX_WORM_VX) / abs(sdy);
			} else { /* sdy == 0 */
				dvx = 0;
				dvy = 0;
			}
		}

		/* Adjust velocity towards desired velocity dvx, dvy */
		if (o->vx < dvx)
			o->vx++;
		else if (o->vx > dvx)
			o->vx--;
		if (o->vy < dvy)
			o->vy++;
		else if (o->vy > dvy)
			o->vy--;

		/* Store our movements so tail segments can ape them. */
		n = o->tsd.worm.i;

		o->tsd.worm.x[n] = o->x;
		o->tsd.worm.y[n] = o->y;
		o->tsd.worm.i++;
		if (o->tsd.worm.i >= NWORM_TIME_UNITS)
			o->tsd.worm.i = 0;

		/* finally, move */
		o->x += o->vx;
		o->y += o->vy;

	} else {
		/* we're in the tail (body segment), just move as the guy in front of you did. */
		int n;

		/* Store our movements so tail segments can ape them. */
		n = o->tsd.worm.i;
		o->tsd.worm.x[n] = o->x;
		o->tsd.worm.y[n] = o->y;
		o->tsd.worm.i++;
		if (o->tsd.worm.i >= NWORM_TIME_UNITS)
			o->tsd.worm.i = 0;

		/* ape the parents motion... */
		n = o->tsd.worm.parent->tsd.worm.i+1;
		if (n >= NWORM_TIME_UNITS)
			n = 0;

		o->x = o->tsd.worm.parent->tsd.worm.x[n];
		o->y = o->tsd.worm.parent->tsd.worm.y[n];

		/* touching the player?  Zap him. */
		if (pdx < 8 && pdy < 8) {
			add_sound(ZZZT_SOUND, ANY_SLOT);
			explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
			game_state.health -= WORM_DAMAGE;
		}
	}
}

void airship_move(struct game_obj_t *o)
{
	if (!o->alive)
		return;

	flying_thing_move(o);
	flying_thing_shoot_missile(o);
	airship_leak_lisp(o);
	if (o->tsd.airship.pressure > 0)
		o->tsd.airship.pressure--;
	
}

void ship_move(struct game_obj_t *o)
{
	if (!o->alive)
		return;

	flying_thing_move(o);
	/* flying_thing_shoot_missile(o); */
}

/* this is called VERY often.  Don't do anything slow in here. */
void move_spark(struct game_obj_t *o)
{
	// printf("x=%d,y=%d,vx=%d,vy=%d, alive=%d\n", o->x, o->y, o->vx, o->vy, o->alive);
	if (o->alive <= 0) {
		o->draw = NULL;
		o->destroy(o);
		return;
	}
	o->x += o->vx;
	o->y += o->vy;
	age_object(o);
	// printf("x=%d,y=%d,vx=%d,vy=%d, alive=%d\n", o->x, o->y, o->vx, o->vy, o->alive);

	/* Fade the colors. */
	if (o->alive >= NSPARKCOLORS)
		o->color = NCOLORS + NSPARKCOLORS - 1; 
	else if (o->alive < 0) 
		o->color = NCOLORS;
	else
		o->color = NCOLORS + o->alive;

	/* air resistance */
	if (o->vx > 0)
		o->vx--;
	else if (o->vx < 0)
		o->vx++;
	if (o->vy > 0)
		o->vy--;
	else if (o->vy < 0)
		o->vy++;

	/* no gravity on the sparks, I've tried it, I like it better without it. */

	/* If the sparks reach speed of zero due to air resistance, kill them. */
	if (o->vx == 0 && o->vy == 0) {
		kill_object(o);
		o->draw = NULL;
	}

	/* If the sparks are too far away, kill them so we won't have to process them. */
	if (abs(o->y - player->y) > 2000 || o->x > 2000+WORLDWIDTH || o->x < -2000) {
		kill_object(o);
		o->draw = NULL;
	}
}

static void add_spark(int x, int y, int vx, int vy, int time);

/* Make a bunch of sparks going in random directions. 
 * x, and y are where, ivx,ivy are initial vx, vy, v is the
 * radial velocity, think of it as the "power" of the explosion
 * and nsparks is how many sparks to make.  time is how long
 * the sparks are set to live. 
 */
void explode(int x, int y, int ivx, int ivy, int v, int nsparks, int time)
{
	int vx, vy, i;

	for (i=0;i<nsparks;i++) {
		// vx = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (v + 0.0) + (0.0 + ivx));
		// vy = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (v + 0.0) + (0.0 + ivy));
		vx = randomn(v) - (v>>1) + ivx;
		vy = randomn(v) - (v>>1) + ivy;
		add_spark(x, y, vx, vy, time); 
		/* printf("%d,%d, v=%d,%d, time=%d\n", x,y, vx, vy, time); */
	}
}

/* This converts a stroke_t, which is a sort of slightly compressed coding 
 * of how to draw a letter, lifted from Hofstadter's book, and converts it
 * into a set of line segments and breaks, like all the other objects in
 * the game, while also scaling it by some amount.  It is used in making
 * a particular font size
 */
struct my_vect_obj *prerender_glyph(stroke_t g[], int xscale, int yscale)
{
	int i, x, y;
	int npoints = 0;
	struct my_point_t scratch[100];
	struct my_vect_obj *v;

	/* printf("Prerendering glyph..\n"); */

	for (i=0;g[i] != 99;i++) {
		if (g[i] == 21) {
			/* printf("LINE_BREAK\n"); */
			x = LINE_BREAK;
			y = LINE_BREAK;
		} else {
			// x = ((g[i] % 3) * xscale);
			// y = ((g[i]/3)-4) * yscale ;     // truncating division.
			x = decode_glyph[g[i]].x * xscale;
			y = decode_glyph[g[i]].y * yscale;
			/* printf("x=%d, y=%d\n", x,y); */
		}
		scratch[npoints].x = x;
		scratch[npoints].y = y;
		npoints++;
	}

	v = (struct my_vect_obj *) malloc(sizeof(struct my_vect_obj));
	v->npoints = npoints;
	if (npoints != 0) {
		v->p = (struct my_point_t *) malloc(sizeof(struct my_point_t) * npoints);
		memcpy(v->p, scratch, sizeof(struct my_point_t) * npoints);
	} else
		v->p = NULL;
	return v;
}

/* This makes a font, by prerendering all the known glyphs into
 * prescaled sets of line segments that the drawing routines know
 * how to draw.
 */
int make_font(struct my_vect_obj ***font, int xscale, int yscale) 
{
	struct my_vect_obj **v;

	v = malloc(sizeof(**v) * 256);
	if (!v) {
		if (v) free(v);
		return -1;
	}
	memset(v, 0, sizeof(**v) * 256);
	v['A'] = prerender_glyph(glyph_A, xscale, yscale);
	v['B'] = prerender_glyph(glyph_B, xscale, yscale);
	v['C'] = prerender_glyph(glyph_C, xscale, yscale);
	v['D'] = prerender_glyph(glyph_D, xscale, yscale);
	v['E'] = prerender_glyph(glyph_E, xscale, yscale);
	v['F'] = prerender_glyph(glyph_F, xscale, yscale);
	v['G'] = prerender_glyph(glyph_G, xscale, yscale);
	v['H'] = prerender_glyph(glyph_H, xscale, yscale);
	v['I'] = prerender_glyph(glyph_I, xscale, yscale);
	v['J'] = prerender_glyph(glyph_J, xscale, yscale);
	v['K'] = prerender_glyph(glyph_K, xscale, yscale);
	v['L'] = prerender_glyph(glyph_L, xscale, yscale);
	v['M'] = prerender_glyph(glyph_M, xscale, yscale);
	v['N'] = prerender_glyph(glyph_N, xscale, yscale);
	v['O'] = prerender_glyph(glyph_O, xscale, yscale);
	v['P'] = prerender_glyph(glyph_P, xscale, yscale);
	v['Q'] = prerender_glyph(glyph_Q, xscale, yscale);
	v['R'] = prerender_glyph(glyph_R, xscale, yscale);
	v['S'] = prerender_glyph(glyph_S, xscale, yscale);
	v['T'] = prerender_glyph(glyph_T, xscale, yscale);
	v['U'] = prerender_glyph(glyph_U, xscale, yscale);
	v['V'] = prerender_glyph(glyph_V, xscale, yscale);
	v['W'] = prerender_glyph(glyph_W, xscale, yscale);
	v['X'] = prerender_glyph(glyph_X, xscale, yscale);
	v['Y'] = prerender_glyph(glyph_Y, xscale, yscale);
	v['Z'] = prerender_glyph(glyph_Z, xscale, yscale);
	v['!'] = prerender_glyph(glyph_bang, xscale, yscale);
	v['/'] = prerender_glyph(glyph_slash, xscale, yscale);
	v['\\'] = prerender_glyph(glyph_backslash, xscale, yscale);
	v['|'] = prerender_glyph(glyph_pipe, xscale, yscale);
	v['?'] = prerender_glyph(glyph_que, xscale, yscale);
	v[':'] = prerender_glyph(glyph_colon, xscale, yscale);
	v['('] = prerender_glyph(glyph_leftparen, xscale, yscale);
	v[')'] = prerender_glyph(glyph_rightparen, xscale, yscale);
	v['a'] = prerender_glyph(glyph_a, xscale, yscale);
	v[' '] = prerender_glyph(glyph_space, xscale, yscale);
	v['b'] = prerender_glyph(glyph_b, xscale, yscale);
	v['c'] = prerender_glyph(glyph_c, xscale, yscale);
	v['d'] = prerender_glyph(glyph_d, xscale, yscale);
	v['e'] = prerender_glyph(glyph_e, xscale, yscale);
	v['f'] = prerender_glyph(glyph_f, xscale, yscale);
	v['g'] = prerender_glyph(glyph_g, xscale, yscale);
	v['h'] = prerender_glyph(glyph_h, xscale, yscale);
	v['i'] = prerender_glyph(glyph_i, xscale, yscale);
	v['j'] = prerender_glyph(glyph_j, xscale, yscale);
	v['k'] = prerender_glyph(glyph_k, xscale, yscale);
	v['l'] = prerender_glyph(glyph_l, xscale, yscale);
	v['m'] = prerender_glyph(glyph_m, xscale, yscale);
	v['n'] = prerender_glyph(glyph_n, xscale, yscale);
	v['o'] = prerender_glyph(glyph_o, xscale, yscale);
	v['p'] = prerender_glyph(glyph_p, xscale, yscale);
	v['q'] = prerender_glyph(glyph_q, xscale, yscale);
	v['r'] = prerender_glyph(glyph_r, xscale, yscale);
	v['s'] = prerender_glyph(glyph_s, xscale, yscale);
	v['t'] = prerender_glyph(glyph_t, xscale, yscale);
	v['u'] = prerender_glyph(glyph_u, xscale, yscale);
	v['v'] = prerender_glyph(glyph_v, xscale, yscale);
	v['w'] = prerender_glyph(glyph_w, xscale, yscale);
	v['x'] = prerender_glyph(glyph_x, xscale, yscale);
	v['y'] = prerender_glyph(glyph_y, xscale, yscale);
	v['z'] = prerender_glyph(glyph_z, xscale, yscale);
	v['0'] = prerender_glyph(glyph_0, xscale, yscale);
	v['1'] = prerender_glyph(glyph_1, xscale, yscale);
	v['2'] = prerender_glyph(glyph_2, xscale, yscale);
	v['3'] = prerender_glyph(glyph_3, xscale, yscale);
	v['4'] = prerender_glyph(glyph_4, xscale, yscale);
	v['5'] = prerender_glyph(glyph_5, xscale, yscale);
	v['6'] = prerender_glyph(glyph_6, xscale, yscale);
	v['7'] = prerender_glyph(glyph_7, xscale, yscale);
	v['8'] = prerender_glyph(glyph_8, xscale, yscale);
	v['9'] = prerender_glyph(glyph_9, xscale, yscale);
	v['-'] = prerender_glyph(glyph_dash, xscale, yscale);
	v['+'] = prerender_glyph(glyph_plus, xscale, yscale);
	v[','] = prerender_glyph(glyph_comma, xscale, yscale);
	v['.'] = prerender_glyph(glyph_period, xscale, yscale);
	*font = v;
	return 0;
}

void init_object_numbers()
{
	int i;
	/* every object must known its position in the array, because */
	/* it needs to be able to free itself, and that involves knowing */
	/* which bit to clear -- which involves knowing the position in */
	/* the array. */
	for (i=0;i<MAXOBJS;i++)
		game_state.go[i].number = i;
}

/* this is just cramming arrays, and counts into structures... it's kind of
 * stupid.  There are a few times when some mirror images are computed, and
 * some things are scaled, since when I made the lists of points, mostly
 * I just kind of imagined the thing in my head and typed in the numbers,
 * (quite a lot of trial and error and correcting of points in that 
 * process, but after awhile, you get pretty good at it.)  But, sometimes
 * I'd get the scale off, and have to scale things a bit to make them the
 * right size -- big enough or small enough, or long and skinny enough
 * or whatever.
 */
void init_vects()
{
	int i;

/* Stuck this precalculation of sines and cosines in here as I know
 * this gets called exactly once.
 */
	for (i=0;i<=360;i++) {
		sine[i] = sin((double)i * 3.1415927 * 2.0 / 360.0);
		cosine[i] = cos((double)i * 3.1415927 * 2.0 / 360.0);
		// printf("%d, %g, %g\n", i, cosine[i], sine[i]);
	}

	/* memset(&game_state.go[0], 0, sizeof(game_state.go[0])*MAXOBJS); */
	player_vect.p = player_ship_points;
	player_vect.npoints = sizeof(player_ship_points) / sizeof(player_ship_points[0]);

	/* The left player ship oints are a copy of the right player ship points. */
	/* I just duplicated them in the source.  So we have to flip them */
	left_player_vect.p = left_player_ship_points;
	left_player_vect.npoints = sizeof(left_player_ship_points) / sizeof(left_player_ship_points[0]);
	for (i=0;i<left_player_vect.npoints;i++) {
		if (left_player_ship_points[i].x != COLOR_CHANGE &&
			left_player_ship_points[i].x != LINE_BREAK)
			left_player_ship_points[i].x *= -1;
	}

	rocket_vect.p = rocket_points;
	rocket_vect.npoints = sizeof(rocket_points) / sizeof(rocket_points[0]);
	spark_vect.p = spark_points;
	spark_vect.npoints = sizeof(spark_points) / sizeof(spark_points[0]);
	right_laser_vect.p = right_laser_beam_points;
	right_laser_vect.npoints = sizeof(right_laser_beam_points) / sizeof(right_laser_beam_points[0]);
	fuel_vect.p = fuel_points;
	fuel_vect.npoints = sizeof(fuel_points) / sizeof(fuel_points[0]);
	jammer_vect.p = jammer_points;
	jammer_vect.npoints = sizeof(jammer_points) / sizeof(jammer_points[0]);
	cron_vect.p = cron_points;
	cron_vect.npoints = sizeof(cron_points) / sizeof(cron_points[0]);
	ship_vect.p = ships_hull_points;
	ship_vect.npoints = sizeof(ships_hull_points) / sizeof(ships_hull_points[0]);

	/* This scaling was here when I was typing in the ship point numbers just to */
	/* make it easier to see where things were a little off. */
	/* for (i=0;i<ship_vect.npoints;i++) {
		if (ship_vect.p[i].x != LINE_BREAK && ship_vect.p[i].x != COLOR_CHANGE) {
			ship_vect.p[i].x *= 2;
			ship_vect.p[i].y = (ship_vect.p[i].y+20) * 2;
		}
	} */

	octopus_vect.p = octopus_points;
	octopus_vect.npoints = sizeof(octopus_points) / sizeof(octopus_points[0]);
	bullet_vect.p = bullet_points;
	bullet_vect.npoints = sizeof(bullet_points) / sizeof(bullet_points[0]);
	gdb_vect_right.p = gdb_points_right;
	gdb_vect_right.npoints = sizeof(gdb_points_right) / sizeof(gdb_points_right[0]);

	/* left gdb points are mirror image of right points. */
	gdb_vect_left.p = gdb_points_left;
	gdb_vect_left.npoints = sizeof(gdb_points_left) / sizeof(gdb_points_left[0]);
	for (i=0;i<gdb_vect_right.npoints;i++)
		if (gdb_vect_right.p[i].x != LINE_BREAK && gdb_vect_right.p[i].x != COLOR_CHANGE) 
			gdb_vect_right.p[i].x = -gdb_vect_left.p[i].x;

	bomb_vect.p = bomb_points;
	bomb_vect.npoints = sizeof(bomb_points) / sizeof(bomb_points[0]);
	bridge_vect.p = bridge_points;
	bridge_vect.npoints = sizeof(bridge_points) / sizeof(bridge_points[0]);
	kgun_vect.p = kgun_points;
	kgun_vect.npoints = sizeof(kgun_points) / sizeof(kgun_points[0]);
	inverted_kgun_vect.p = inverted_kgun_points;
	inverted_kgun_vect.npoints = sizeof(inverted_kgun_points) / sizeof(inverted_kgun_points[0]);

	/* I had to scale and translate the airship points as I made it too small */
	/* and picked a poor origin. */
	airship_vect.p = airship_points;
	airship_vect.npoints = sizeof(airship_points) / sizeof(airship_points[0]);
	for (i=0;i<airship_vect.npoints;i++) {
		if (airship_vect.p[i].x != LINE_BREAK && airship_vect.p[i].x != COLOR_CHANGE) {
			airship_vect.p[i].x *= 3;
			airship_vect.p[i].y = (airship_vect.p[i].y+20) * 3;
		}
	}

	balloon_vect.p = balloon_points;
	balloon_vect.npoints = sizeof(balloon_points) / sizeof(balloon_points[0]);
	SAM_station_vect.p = SAM_station_points;
	SAM_station_vect.npoints = sizeof(SAM_station_points) / sizeof(SAM_station_points[0]);
	humanoid_vect.p = humanoid_points;
	humanoid_vect.npoints = sizeof(humanoid_points) / sizeof(humanoid_points[0]);
	socket_vect.p = socket_points;
	socket_vect.npoints = sizeof(socket_points) / sizeof(socket_points[0]);

	/* Make the line segment lists from the stroke_t structures. */
	make_font(&gamefont[BIG_FONT], font_scale[BIG_FONT], font_scale[BIG_FONT]);
	make_font(&gamefont[SMALL_FONT], font_scale[SMALL_FONT], font_scale[SMALL_FONT]);
	make_font(&gamefont[TINY_FONT], font_scale[TINY_FONT], font_scale[TINY_FONT]);
	make_font(&gamefont[NANO_FONT], font_scale[NANO_FONT], font_scale[NANO_FONT]);
	set_font(BIG_FONT);
}

void no_draw(struct game_obj_t *o, GtkWidget *w)
{
	return;
}

/* this is what can draw a list of line segments with line
 * breaks and color changes...   This one scales in x, y.
 * It's used for drawing blimps which are overpressurized.
 */
void draw_scaled_generic(struct game_obj_t *o, GtkWidget *w, float xscale, float yscale)
{
	int j;
	int x1, y1, x2, y2;
	gdk_gc_set_foreground(gc, &huex[o->color]);
	x1 = o->x + (o->v->p[0].x * xscale) - game_state.x;
	y1 = o->y + (o->v->p[0].y * yscale) - game_state.y + (SCREEN_HEIGHT/2);  
	for (j=0;j<o->v->npoints-1;j++) {
		if (o->v->p[j+1].x == LINE_BREAK) { /* Break in the line segments. */
			j+=2;
			x1 = o->x + (o->v->p[j].x * xscale) - game_state.x;
			y1 = o->y + (o->v->p[j].y * yscale) - game_state.y + (SCREEN_HEIGHT/2);  
		}
		if (o->v->p[j].x == COLOR_CHANGE) {
			gdk_gc_set_foreground(gc, &huex[o->v->p[j].y]);
			j+=1;
			x1 = o->x + (o->v->p[j].x * xscale) - game_state.x;
			y1 = o->y + (o->v->p[j].y * yscale) - game_state.y + (SCREEN_HEIGHT/2);  
		}
		x2 = o->x + (o->v->p[j+1].x * xscale) - game_state.x; 
		y2 = o->y + (o->v->p[j+1].y * yscale) +(SCREEN_HEIGHT/2) - game_state.y;
		if (x1 > 0 && x2 > 0)
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		x1 = x2;
		y1 = y2;
	}
}

/* this is what can draw a list of line segments with line
 * breaks and color changes...  This gets called quite a lot,
 * so try to make sure it's fast.  There is an inline version
 * of this in draw_objs(), btw. 
 */
void draw_generic(struct game_obj_t *o, GtkWidget *w)
{
	int j;
	int x1, y1, x2, y2;
	gdk_gc_set_foreground(gc, &huex[o->color]);
	x1 = o->x + o->v->p[0].x - game_state.x;
	y1 = o->y + o->v->p[0].y - game_state.y + (SCREEN_HEIGHT/2);  
	for (j=0;j<o->v->npoints-1;j++) {
		if (o->v->p[j+1].x == LINE_BREAK) { /* Break in the line segments. */
			j+=2;
			x1 = o->x + o->v->p[j].x - game_state.x;
			y1 = o->y + o->v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
		}
		if (o->v->p[j].x == COLOR_CHANGE) {
			gdk_gc_set_foreground(gc, &huex[o->v->p[j].y]);
			j+=1;
			x1 = o->x + o->v->p[j].x - game_state.x;
			y1 = o->y + o->v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
		}
		x2 = o->x + o->v->p[j+1].x - game_state.x; 
		y2 = o->y + o->v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y;
		if (x1 > 0 && x2 > 0)
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		x1 = x2;
		y1 = y2;
	}
}

void truss_draw(struct game_obj_t *o, GtkWidget *w)
{
	int x, y, x1, y1, x2, y2;
	gdk_gc_set_foreground(gc, &huex[o->color]);

	x = o->x - game_state.x;
	y = o->y - game_state.y + (SCREEN_HEIGHT/2);  

	x1 = x - 10;
	x2 = x + 10;
	y1 = y - 10;
	y2 = y + 10;

	wwvi_draw_line(w->window, gc, x1, y1, x2, y2);
	wwvi_draw_line(w->window, gc, x1, y2, x2, y1);
	wwvi_draw_line(w->window, gc, x1, y1, x2, y1);
	wwvi_draw_line(w->window, gc, x1, y2, x2, y2);
	wwvi_draw_line(w->window, gc, x1, y1, x1, y2);
	wwvi_draw_line(w->window, gc, x2, y1, x2, y2);
}

void kgun_draw(struct game_obj_t *o, GtkWidget *w)
{
	int dx, dy;
	int x1, y1;
	int xi, yi;
	int adx, ady;
	int crossbeam_x1, crossbeam_y, crossbeam_x2, xoffset;
	int guntipx, guntipy;
	int gunbackx, gunbacky;
	int thickxo, thickyo;
	int size = o->tsd.kgun.size;
	int invert = o->tsd.kgun.invert;

	draw_generic(o, w);
	/* Find x and y dist to player.... */
	dx = player->x+LASERLEAD*player->vx - o->x;
	dy = player->y+LASERLEAD*player->vy - o->y;

	x1 = o->x-5 - game_state.x;
	y1 = o->y+(5*invert) - game_state.y + (SCREEN_HEIGHT/2);  
	crossbeam_y = y1 + (20*invert);

	wwvi_draw_line(w->window, gc, x1, y1, x1+5, crossbeam_y); 
	wwvi_draw_line(w->window, gc, x1+10, y1, x1+6, crossbeam_y);

	/* We're going to draw a swivelling dual gun turret.  Two */
	/* guns mounted on a crossbeam that kind of swivel around */
	/* in a 3-d looking way (but it's fake 3-d, of course.) */
	/* Kind of like this:
	 *
	 *                                         *  *
	 *                                         *  *     
	 *                                          **  
	 *                                          **
	 *                                   ***    **  ***   
	 *                                   ***    **  ***     
	 *                                    ***   **   ***
	 *                                    **************
	 *                                     ***        ***
	 *                                     ***        ***
	 *                                      ***        ***
	 *                                       *          * 
	 *                                        *          * 
	 *                                        *          * 
	 *                                         *          * 
	 *                                         *          * 
	 *
	 */

	/* First, scale the larger of dx,dy to 32, and the other by */
	/* a proportional amount (by similar triangles).  This is in */
	/* order to index into the vxy_2_dxy array. */
	
	adx = abs(dx);
	ady = abs(dy);

	/* Calculate indices into vxy_2_dxy array, xi, and yi */
	if (adx > ady) {
		xi = size;
		yi = (size * ady) / adx;
	} else {
		if (ady > adx) {
			yi = size;
			xi = (size * adx) / ady;
		} else { /* abs(dx) == abs(dy) */
			xi = size;
			yi = size;
		}
	}

	/* notice, using y component for x offset... */
	crossbeam_x1 = vxy_2_dxy[xi][yi].y;
	crossbeam_x2 = -crossbeam_x1;

	crossbeam_x1 += o->x - game_state.x;
	crossbeam_x2 += o->x - game_state.x;

	xoffset = crossbeam_x2 - crossbeam_x1;

	guntipx = vxy_2_dxy[xi][yi].x;
	guntipy = vxy_2_dxy[xi][yi].y;

	if (dy > 20)
		guntipy = -guntipy;
	if (dx < 0)
		guntipx = -guntipx;

	gunbackx = -guntipx;
	gunbacky = -guntipy;

	thickxo = -gunbacky >> 2; /* notice reversed x,y coords here, to get normal (right angle). */
	thickyo = gunbackx >> 2;  /* gun butt is 1/4 as thick as it is long. */

	guntipx = guntipx << 1; /* gun barrel is 2x as long as the butt of the gun. */
	guntipy = guntipy << 1;

	/* guntipy *= invert;
	gunbacky *= invert;
	thickyo *= invert; */

	guntipx += crossbeam_x1;
	gunbackx += crossbeam_x1;
	guntipy += crossbeam_y;
	gunbacky += crossbeam_y;
	
	wwvi_draw_line(w->window, gc, crossbeam_x1, crossbeam_y, crossbeam_x2, crossbeam_y);

	wwvi_draw_line(w->window, gc, gunbackx, gunbacky, guntipx, guntipy);
	wwvi_draw_line(w->window, gc, gunbackx+xoffset, gunbacky, guntipx+xoffset, guntipy);

	wwvi_draw_line(w->window, gc, gunbackx-thickxo, gunbacky-thickyo, 
			gunbackx+thickxo, gunbacky+thickyo);
	wwvi_draw_line(w->window, gc, gunbackx-thickxo+xoffset, gunbacky-thickyo, 
			gunbackx+thickxo+xoffset, gunbacky+thickyo);
	wwvi_draw_line(w->window, gc, crossbeam_x1-thickxo, crossbeam_y-thickyo, 
			crossbeam_x1+thickxo, crossbeam_y+thickyo);
	wwvi_draw_line(w->window, gc, crossbeam_x1-thickxo+xoffset, crossbeam_y-thickyo, 
			crossbeam_x1+thickxo+xoffset, crossbeam_y+thickyo);

	wwvi_draw_line(w->window, gc, gunbackx-thickxo, gunbacky-thickyo, 
			crossbeam_x1-thickxo, crossbeam_y-thickyo);
	wwvi_draw_line(w->window, gc, gunbackx-thickxo+xoffset, gunbacky-thickyo, 
			crossbeam_x1-thickxo+xoffset, crossbeam_y-thickyo);
	wwvi_draw_line(w->window, gc, gunbackx+thickxo, gunbacky+thickyo, 
			crossbeam_x1+thickxo, crossbeam_y+thickyo);
	wwvi_draw_line(w->window, gc, gunbackx+thickxo+xoffset, gunbacky+thickyo, 
			crossbeam_x1+thickxo+xoffset, crossbeam_y+thickyo);
	
}



/* this is an array of random values for use in picking
 * coordinates and colors for noise in the little radar 
 * screen.  We have to precompute and store this because
 * it's like snow on a tv tuned to no channel.  There's
 * a lot of it and it has to be painted FAST, and it's
 * CPU intensive.
 *
 * Why so much, why those values?  Empirically, these values
 * produce believable noise with no discernable pattern.
 * (smaller values had patterns I could see sometimes,
 * not that these values are ideal, but they work.)
 */
#define XRADARN 3313 /* should be 3 unequal primes */
#define YRADARN 223 
#define CRADARN 47

int xradarnoise[XRADARN];
int yradarnoise[YRADARN]; 
int cradarnoise[CRADARN];

void init_radar_noise()
{
	int i;

	for (i=0;i<XRADARN;i++) {
		xradarnoise[i] = randomn(SCREEN_WIDTH - 
			RADAR_LEFT_MARGIN - RADAR_RIGHT_MARGIN) + RADAR_LEFT_MARGIN;
	}
	for (i=0;i<YRADARN;i++) {
		yradarnoise[i] = randomn(RADAR_HEIGHT) + 
			SCREEN_HEIGHT - RADAR_YMARGIN - RADAR_HEIGHT;
	}
	for (i=0;i<CRADARN;i++) {
		cradarnoise[i] = randomn(NCOLORS);
	}
}

void draw_on_radar(GtkWidget *w, struct game_obj_t *o, int y_correction)
{
	int radarx, radary;
	int x1, x2, y1, y2; 

	/* project game coordinates onto the radar coordiantes. */
	radary = SCREEN_HEIGHT - (RADAR_HEIGHT >> 1) - RADAR_YMARGIN + ((o->y * RADAR_HEIGHT) / 1500);
	radary = radary + y_correction;

	/* If object is off top or bottom of radar screen, don't draw it. */ 
	/* However, if it is a radar jammer, don't bail just yet. as we */
	/* still have to draw the noise that it makes, otherwise, the player */
	/* can subvert the jammer just by flying high enough that the jammer */
	/* goes off the bottom of the radar screen. */
	if (radary < SCREEN_HEIGHT - RADAR_HEIGHT - RADAR_YMARGIN 
		&& o->otype != OBJ_TYPE_JAMMER)
		return;
	if (radary > SCREEN_HEIGHT - RADAR_YMARGIN 
		&& o->otype != OBJ_TYPE_JAMMER)
		return;

	radarx = ((SCREEN_WIDTH - (RADAR_LEFT_MARGIN + RADAR_RIGHT_MARGIN)) * o->x) / WORLDWIDTH + RADAR_LEFT_MARGIN;

	/* o->radar image is mostly a boolean 0 or 1, indicating whether something's */
	/* on the radar at all, but I used value 2 to make bigger items on the radar. */
	/* and us it in calculating how to draw the little 'x' */
	x1 = radarx - o->radar_image;
	x2 = radarx + o->radar_image;
	y1 = radary - o->radar_image;
	y2 = radary + o->radar_image;


	/* Don't draw anything for radar jammer's themselves, */
	/* but do draw other stuff.. */	
	if (o->otype != OBJ_TYPE_JAMMER) {
		/* draw a little x.... */	
		gdk_gc_set_foreground(gc, &huex[o->color]);
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		wwvi_draw_line(w->window, gc, x1, y2, x2, y1); 
		if (o->otype == OBJ_TYPE_HUMAN || o->otype == OBJ_TYPE_PLAYER) {
			/* for the player and the humans, make them a little more */
			/* prominent. */
			wwvi_draw_line(w->window, gc, x1, y2, x1, y1); 
			wwvi_draw_line(w->window, gc, x1, y1, x2, y1); 
			wwvi_draw_line(w->window, gc, x2, y1, x2, y2); 
			wwvi_draw_line(w->window, gc, x2, y2, x1, y2); 
			return;
		}
	}

	/* Here's where we put noise on the screen. total_radar_noise */
	/* keeps track of how much noise we've already put on, as there's */
	/* no sense, once the maximum amount of noise has been reached, */
	/* to paint more noise over something already completely obscured */
	/* by noise.  Save the time by skipping, in that case. */
	if (o->otype == OBJ_TYPE_JAMMER && total_radar_noise < MAX_RADAR_NOISE) {
		int xdist, noisecount, i, nx, ny, nc;

		nc = randomn(CRADARN);
		nx = randomn(XRADARN);
		ny = randomn(YRADARN);
		if (nx >= XRADARN) nx = 0;
		if (ny >= YRADARN) ny = 0;
		if (nc >= CRADARN) nc = 0;

		/* based on distance from jammer, figure how much noise to paint. */
		xdist = abs(player->x - o->x);
		noisecount = 2000 - xdist;

		/* paint the noise... */
		for (i=0;i<noisecount;i++) {
			int x1, y1, x2, y2;
			gdk_gc_set_foreground(gc, &huex[cradarnoise[nc]]);
			x1 = xradarnoise[nx]-1;
			x2 = xradarnoise[nx]+1;
			y1 = yradarnoise[ny]-1;
			y2 = yradarnoise[ny]+1;
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2);
			wwvi_draw_line(w->window, gc, x1, y2, x2, y1);
			nx++; if (nx >= XRADARN) nx = 0;
			ny++; if (ny >= YRADARN) ny = 0;
			nc++; if (nc >= CRADARN) nc = 0;
			total_radar_noise++;
			if (total_radar_noise >= MAX_RADAR_NOISE)
				break;
		}
	}
}

/* This gets called frame_rate_hz times every second (normally 30 times a sec) */
/* by main_da_expose().  Don't do anything slow in here. */
void draw_objs(GtkWidget *w)
{
	int i, j, x1, y1, x2, y2, radary;

	/* figure where the center y of the radar screen is */
	radary = SCREEN_HEIGHT - (RADAR_HEIGHT >> 1) - RADAR_YMARGIN + ((player->y * RADAR_HEIGHT) / 1500);
	radary = (SCREEN_HEIGHT - (RADAR_HEIGHT >> 1)) - radary;

	total_radar_noise = 0; /* Each frame gets it's own allotment of noise. */
	for (i=0;i<MAXOBJS;i++) {
		struct my_vect_obj *v = game_state.go[i].v;
		struct game_obj_t *o = &game_state.go[i];

		if (!o->alive)
			continue; /* skip dead things. */

		/* Draw each radar imageable object on the radar screen. */
		if (o->radar_image && game_state.radar_state == RADAR_RUNNING)
			draw_on_radar(w, o, radary);

		/* Don't draw offscreen things. */
		if (o->x < (game_state.x - (SCREEN_WIDTH/3)))
			continue;
		if (o->x > (game_state.x + 4*(SCREEN_WIDTH/3)))
			continue;
		if (o->y < (game_state.y - (SCREEN_HEIGHT)))
			continue;
		if (o->y > (game_state.y + (SCREEN_HEIGHT)))
			continue;

#if 0
		if (o->v == &spark_vect)
			printf("s");
		if (o->v == &player_vect)
			printf("p");
		if (o->v == &rocket_vect)
			printf("r");
#endif

		/* If there's no special drawing function, and the object has */
		/* a list of line segments to draw, draw them.  This is an */
		/* inline version of the draw_generic function. */
		if (o->draw == NULL && o->v != NULL) {
			gdk_gc_set_foreground(gc, &huex[o->color]);
			x1 = o->x + v->p[0].x - game_state.x;
			y1 = o->y + v->p[0].y - game_state.y + (SCREEN_HEIGHT/2);  
			for (j=0;j<v->npoints-1;j++) {
				if (v->p[j+1].x == LINE_BREAK) { /* Break in the line segments. */
					j+=2;
					x1 = o->x + v->p[j].x - game_state.x;
					y1 = o->y + v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
				}
				if (o->v->p[j].x == COLOR_CHANGE) {
					gdk_gc_set_foreground(gc, &huex[o->v->p[j].y]);
					j+=1;
					x1 = o->x + v->p[j].x - game_state.x;
					y1 = o->y + v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
				}
				x2 = o->x + v->p[j+1].x - game_state.x; 
				y2 = o->y + v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y;
				if (x1 > 0 && x2 > 0 && (x1 < SCREEN_WIDTH || x2 < SCREEN_WIDTH))
					wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
				x1 = x2;
				y1 = y2;
			}
		} else if (o->draw != NULL)
			o->draw(o, w); /* Call object's specialized drawing function. */
	}
}

/* This is used by the "attract mode" screen for drawing the letters */
/* livecursorx, and livecursory are coords of a "cursor" which this */
/* routine automatically advances. */
static void draw_letter(GtkWidget *w, struct my_vect_obj **font, unsigned char letter)
{
	int i, x1, y1, x2, y2;

	if (letter == ' ') {
		livecursorx += font_scale[current_font]*2 + letter_spacing[current_font];
		return;
	}
	if (letter == '\n') {
		livecursorx = font_scale[current_font];
		livecursory += font_scale[current_font];
		return;
	}
	if (font[letter] == NULL) {
		return;
	}

	x1 = livecursorx + font[letter]->p[0].x;
	y1 = livecursory + font[letter]->p[0].y;
	for (i=0;i<font[letter]->npoints-1;i++) {
		if (font[letter]->p[i+1].x == LINE_BREAK) {
			i+=2;
			x1 = livecursorx + font[letter]->p[i].x;
			y1 = livecursory + font[letter]->p[i].y;
		}
		x2 = livecursorx + font[letter]->p[i+1].x;
		y2 = livecursory + font[letter]->p[i+1].y;
		wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		//wwvi_draw_line(w->window, gc, x1-1, y1+1, x2-1, y2+1); 
		x1 = x2;
		y1 = y2;
	}
	livecursorx += font_scale[current_font]*2 + letter_spacing[current_font];
}

/* Draws a string for the "attract mode", */
static void draw_string(GtkWidget *w, unsigned char *s) 
{

	unsigned char *i;

	for (i=s;*i;i++)
		draw_letter(w, gamefont[current_font], *i);  
}

/* draws a bunch of strings for the "attract mode." */
static void draw_strings(GtkWidget *w)
{
	int i;

	gdk_gc_set_foreground(gc, &huex[WHITE]); //&huex[current_color]);
	for (i=0;i<ntextlines;i++) {
		/* printf("Drawing '%s' color=%d, x=%d, y=%d\n", 
			textline[i].string, current_color, 
			textline[i].x, textline[i].y); */
		livecursorx = textline[i].x;
		livecursory = textline[i].y;
		set_font(textline[i].font);
		draw_string(w, (unsigned char *) textline[i].string);
	}
}

/* Draws a letter in the given font at an absolute x,y coords on the screen. */
static void abs_xy_draw_letter(GtkWidget *w, struct my_vect_obj **font, 
		unsigned char letter, int x, int y)
{
	int i, x1, y1, x2, y2;

	if (letter == ' ' || letter == '\n' || letter == '\t' || font[letter] == NULL)
		return;

	for (i=0;i<font[letter]->npoints-1;i++) {
		if (font[letter]->p[i+1].x == LINE_BREAK)
			i+=2;
		x1 = x + font[letter]->p[i].x;
		y1 = y + font[letter]->p[i].y;
		x2 = x + font[letter]->p[i+1].x;
		y2 = y + font[letter]->p[i+1].y;
		if (x1 > 0 && x2 > 0)
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
	}
}

/* Used for floating labels in the game. */
/* Draws a string at an absolute x,y position on the screen. */ 
static void abs_xy_draw_string(GtkWidget *w, char *s, int font, int x, int y) 
{

	int i;	
	int deltax = font_scale[font]*2 + letter_spacing[font];

	for (i=0;s[i];i++)
		abs_xy_draw_letter(w, gamefont[font], s[i], x + deltax*i, y);  
}


/* FIXME: what is the difference between thise, and the "abs" versions?  I forget. */
static void xy_draw_letter(GtkWidget *w, struct my_vect_obj **font, 
		unsigned char letter, int x, int y)
{
	int i, x1, y1, x2, y2;

	if (letter == ' ' || letter == '\n' || letter == '\t' || font[letter] == NULL)
		return;

	for (i=0;i<font[letter]->npoints-1;i++) {
		if (font[letter]->p[i+1].x == LINE_BREAK)
			i+=2;
		x1 = x + font[letter]->p[i].x - game_state.x;
		y1 = y + font[letter]->p[i].y - game_state.y + (SCREEN_HEIGHT/2);
		x2 = x + font[letter]->p[i+1].x - game_state.x;
		y2 = y + font[letter]->p[i+1].y - game_state.y + (SCREEN_HEIGHT/2);
		if (x1 > 0 && x2 > 0)
			wwvi_draw_line(w->window, gc, x1, y1, x2, y2); 
		// wwvi_draw_line(w->window, gc, x1-1, y1+1, x2-1, y2+1); 
	}
}

/* FIXME, what is the diff between this and the "abs" version? */
static void xy_draw_string(GtkWidget *w, char *s, int font, int x, int y) 
{

	int i;	
	int deltax = font_scale[font]*2 + letter_spacing[font];

	for (i=0;s[i];i++)
		xy_draw_letter(w, gamefont[font], (unsigned char) s[i], x + deltax*i, y);  
}


static void add_laserbolt(int x, int y, int vx, int vy, int color, int time)
{
	add_generic_object(x, y, vx, vy, move_laserbolt, draw_laserbolt,
		color, &spark_vect, 0, OBJ_TYPE_SPARK, time);
}

static void add_spark(int x, int y, int vx, int vy, int time)
{
	add_generic_object(x, y, vx, vy, move_spark, draw_spark,
		YELLOW, &spark_vect, 0, OBJ_TYPE_SPARK, time);
}

/* Modify *value by a random amount that is a percentage of a difference. */
/* This is used by the recursive routine which makes the ground fractal */
/* to displace the midpoints */
void perturb(int *value, int lower, int upper, double percent)
{
	double perturbation;

	perturbation = percent * (lower - upper) * ((0.0 + random()) / (0.0 + RAND_MAX) - 0.5);
	*value += perturbation;
	return;
}

/* Used to add the lisp code that leaks out of the blimps. */
static void add_symbol(int c, int myfont, int x, int y, int vx, int vy, int time)
{	
	struct my_vect_obj **font = gamefont[myfont];
	if (font[c] != NULL)
		add_generic_object(x, y, vx, vy, symbol_move, NULL,
			WHITE, font[c], 0, OBJ_TYPE_SYMBOL, time);
}

/* Used to add the floating scores, "Woohoo!" and "Help!" messages */
static void add_floating_message(char *msg, int font, int x, int y, int vx, int vy, int time)
{
	struct game_obj_t *o;
	o = add_generic_object(x, y, vx, vy, floating_message_move, 
		floating_message_draw, WHITE, NULL, 0,
		OBJ_TYPE_FLOATING_MESSAGE, time);
	if (o == NULL)
		return;
	strncpy((char *) o->tsd.floating_message.msg, msg, 20);
	o->tsd.floating_message.font = font;
}

static void add_score_floater(int x, int y, int score)
{
	char message[20];
	int vx, vy;
	sprintf(message, "%d", score);
	vx = game_state.go[0].vx * 2;
	vy = -7;
/*
	if (x is offscreen)
		return;
	if (y is offscreen)
		return;
	if (x is mostly to the left)
		vx = -4;
	else
		vx = 4;

	if (y is mostly to the bottom)
		vy = -4; 
	else
		vy = +4;
*/
	add_floating_message(message, SMALL_FONT, x, y, vx, vy, 20);
		
}

static void add_floater_message(int x, int y, char *msg)
{
	add_floating_message(msg, SMALL_FONT, x, y, game_state.go[0].vx * 2, -7, 20);
}

/* recurseive function to generate a terrain map between indexes
 * xi1 and xi2 in the terrain array. */ 
void generate_sub_terrain(struct terrain_t *t, int xi1, int xi2)
{
	int midxi;
	int y1, y2, y3, tmp;
	int x1, x2, x3;


	/* find the middle index, if there is no in between */
	/* index, we're done, it's a terminal case. */
	midxi = (xi2 - xi1) / 2 + xi1;
	if (midxi == xi2 || midxi == xi1)
		return;

	/* Find the midpoint, x3,y3 between the given end points, x1,y1, x2,y2. */
	y1 = t->y[xi1];
	y2 = t->y[xi2];
	if (y1 > y2) {
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	y3 = ((y2 - y1) / 2) + y1;

	x1 = t->x[xi1];
	x2 = t->x[xi2];
	if (x1 > x2) {
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	x3 = ((x2 - x1) / 2) + x1;

	/* if there is a relatively large scale distance between the given */
	/* endpoints, perturb the midpoint by a factor apprpriate to the */
	/* large scale, otherwise by a factor apppropriate for the small scale. */
	if ((x2 - x1) > 1000) {	
		perturb(&x3, x2, x1, level.large_scale_roughness);
		do { 
			perturb(&y3, x2, x1, level.large_scale_roughness);
		} while ((KERNEL_Y_BOUNDARY - y3) > -250);
	} else {
		perturb(&x3, x2, x1, level.small_scale_roughness);
		do { 
			perturb(&y3, x2, x1, level.small_scale_roughness);
		} while ((KERNEL_Y_BOUNDARY - y3) > -250);
	}

	t->x[midxi] = x3;
	t->y[midxi] = y3;
	// printf("gst %d %d\n", x3, y3);

	/* recursively generate more terrain between x1,y1, x3,y3, */
	/* and between x3,y3, and x2,y2 */
	generate_sub_terrain(t, xi1, midxi);
	generate_sub_terrain(t, midxi, xi2);
}

static struct game_obj_t *add_volcano(struct terrain_t *t, int x, int y);
void generate_terrain(struct terrain_t *t)
{
	int volcanox, volcanoi;
	int i, vi1, vi2, vi3, vi4, vi5;

	/* Set up the intial endpoints of the terrain. */
	t->npoints = TERRAIN_LENGTH;
	t->x[0] = 0;
	t->y[0] = 100;
	t->x[t->npoints-1] = WORLDWIDTH;
	t->y[t->npoints-1] = t->y[0];

	// generate_sub_terrain(t, 0, t->npoints-1);
	
	/* put a volcano in... */
	volcanox = WORLDWIDTH / VOLCANO_XFRACTION;
	volcanoi = t->npoints / VOLCANO_XFRACTION;

	vi1 = volcanoi - 30;
	vi2 = volcanoi - 3;
	vi3 = volcanoi;
	vi4 = volcanoi + 3;
	vi5 = volcanoi + 30;

	t->x[vi1] = volcanox - (WORLDWIDTH * 30 / TERRAIN_LENGTH);
	t->x[vi2] = volcanox - (WORLDWIDTH * 5 / TERRAIN_LENGTH);
	t->x[vi3] = volcanox;
	t->x[vi4] = volcanox + (WORLDWIDTH * 5 / TERRAIN_LENGTH);
	t->x[vi5] = volcanox + (WORLDWIDTH * 30 / TERRAIN_LENGTH);
	
	t->y[vi1] = 100;
	t->y[vi2] = -550;
	t->y[vi3] = -500;
	t->y[vi4] = -550;
	t->y[vi5] = 100;

	/* generate the terrain around and including the volcano. */
	generate_sub_terrain(t, 0, vi1);
	generate_sub_terrain(t, vi1, vi2);
	generate_sub_terrain(t, vi2, vi3);
	generate_sub_terrain(t, vi3, vi4);
	generate_sub_terrain(t, vi4, vi5);
	generate_sub_terrain(t, vi5, t->npoints-1);

	/* Put the volcano where he needs to be. */
	volcano_obj = add_volcano(t, volcanox, t->y[vi3]);

	/* calculate slopes */
	for (i=0;i<t->npoints-2;i++)
		t->slope[i] = (100*(t->y[i+1] - t->y[i]) / (t->x[i+1] - t->x[i]) );
	t->slope[t->npoints-1] = 0;
}

/* allocate and create a set of random connected lines near a point
 * to use as the shapes for debris. */
static struct my_vect_obj *init_debris_vect(struct my_vect_obj **v, struct my_point_t **p)
{
	int i, n;

	n = randomab(2,5);
	*v = (struct my_vect_obj *) malloc(sizeof(**v));
	*p = (struct my_point_t *) malloc(sizeof(**p) * n);
	if (!*v || !*p) {
		if (*v)
			free(*v);
		if (*p)
			free (*p);
		return NULL;
	}

	(*v)->p = *p;
	(*v)->npoints = n;
	
	for (i=0;i<n;i++) {
		(*p)[i].x = randomn(20)-10;
		(*p)[i].y = randomn(10)-5;
	}
	return *v;
}

static void make_debris_forms()
{
	int i;

	for (i=0;i<NDEBRIS_FORMS;i++)
		init_debris_vect(&debris_vect[i], &debris_point[i]);
}

static void free_debris_forms()
{
	int i;

	for (i=0;i<NDEBRIS_FORMS;i++) {
		free(debris_vect[i]);
		free(debris_point[i]);
	}
}

/* Spray chunks of debris from a given point x,y, with initial velocity vx,vy
 * and with radiating velocity r.  Used in explosions. */
static void spray_debris(int x, int y, int vx, int vy, int r, struct game_obj_t *victim, int metal)
{
	int i, z; 
	struct game_obj_t *o;

	for (i=0;i<=16;i++) {
		z = find_free_obj();
		if (z < 0)
			return;
		o = &game_state.go[z];
		o->last_xi = -1;
		o->x = x;
		o->y = y;
		o->otype = OBJ_TYPE_DEBRIS;
		o->move = bridge_move;
		o->draw = draw_generic;
		o->destroy = generic_destroy_func;
		o->alive = 5*frame_rate_hz + randomn(frame_rate_hz);	
		o->color = victim->color;
		o->radar_image = 1;
		o->target = add_target(o);
		// o->vx = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (r + 0.0) + (0.0 + vx));
		// o->vy = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (r + 0.0) + (0.0 + vy));
		o->vx = randomn(r) - (r >> 1) + vx;	
		o->vy = randomn(r) - (r >> 1) + vy;	
		o->target = NULL;
		o->tsd.debris.debris_type = metal;
		o->v = debris_vect[randomn(NDEBRIS_FORMS)];
	}
}

/* for picking random locations of objects at level startup. */
static int initial_x_location() 
{
	int x;

	/* don't put things on or near the volcano. */	
	do {
		x = randomn(TERRAIN_LENGTH - 40) + 40;
	} while (abs(volcano_obj->x - terrain.x[x]) < 800);
	return x;
}

static void add_flak_guns(struct terrain_t *t)
{
	int i, xi;
	struct game_obj_t *o;

	for (i=0;i<level.nflak;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi] - 7, 0, 0, 
			kgun_move, kgun_draw, GREEN, &inverted_kgun_vect, 1, OBJ_TYPE_KGUN, 1);
		if (o) {
			o->tsd.kgun.size = 31;
			o->tsd.kgun.velocity_factor = 1;
			o->tsd.kgun.invert = -1;
		}
	}
}

static void add_kernel_guns(struct terrain_t *t)
{
	int i, j, xi;
	int ntrusses;
	struct game_obj_t *o, *p;


	o = NULL;
	p = NULL;
	for (i=0;i<level.nkguns;i++) {
		xi = initial_x_location();
		ntrusses = randomn(9)+3;
		for (j=0;j<ntrusses;j++) {
			o = add_generic_object(t->x[xi], KERNEL_Y_BOUNDARY + 10 + (20*j), 0, 0, 
				NULL, truss_draw, RED, NULL, 1, OBJ_TYPE_TRUSS, 1);
			if (o == NULL) {
				struct game_obj_t *x, *next;

				for (x = p; x != NULL; x = next ) {
					next = x->tsd.truss.above;
					kill_object(x);
				}
				break;
			}
			if (j != 0) {
				o->tsd.truss.above = p;
				p->tsd.truss.below = o;
			} else 
				o->tsd.truss.above = NULL;
			p = o;
			p->tsd.truss.below = NULL;
		}
		if (o == NULL)
			break;
		o = add_generic_object(t->x[xi], p->y + 10 + 25, 0, 0, 
			kgun_move, kgun_draw, RED, &kgun_vect, 1, OBJ_TYPE_KGUN, 1);
		if (o != NULL) {
			o->tsd.kgun.size = 31;	/* biggest size */
			o->tsd.kgun.invert = 1;	/* this means hanging upside down */
			o->tsd.kgun.velocity_factor = 1;	/* normal laser speed */
		}
		if (p != NULL)
			p->tsd.truss.below = o;
	}
}

static void add_rockets(struct terrain_t *t)
{
	int i, xi;
	for (i=0;i<level.nrockets;i++) {
		xi = initial_x_location();
		add_generic_object(t->x[xi], t->y[xi] - 7, 0, 0, 
			move_rocket, NULL, WHITE, &rocket_vect, 1, OBJ_TYPE_ROCKET, 1);
	}
}

/* Insert the points in one list into the middle of another list */
/* This is used by the routines which create the building shapes to */
/* embelish the buildings with various ornamentation. */
static void insert_points(struct my_point_t host_list[], int *nhost, 
		struct my_point_t injection[], int ninject, 
		int injection_point)
{
	/* make room for the injection */
	memmove(&host_list[injection_point + ninject], 
		&host_list[injection_point], 
		(*nhost - injection_point) * sizeof(host_list[0]));

	/* make the injection */
	memcpy(&host_list[injection_point], &injection[0], (ninject * sizeof(injection[0])));
	*nhost += ninject;
}

static void embellish_roof(struct my_point_t *building, int *npoints, int left, int right);
static void indent_roof(struct my_point_t *building, int *npoints, int left, int right);

static struct my_point_t mosquey_roof[] = {
		{ -10, 0 },
		{ -15, -5 },
		{ -17, -10 },
		{ -15, -15 },
		{  -5, -25 },
		{  0, -35 },
		{  5, -25 },
		{ 15, -15 },
		{ 17, -10 },
		{ 15, -5 },
		{ 10, 0 },
	};
static void add_mosquey_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	struct my_point_t p[ sizeof(mosquey_roof)/  sizeof(mosquey_roof[0])];
	int i;
	int width;
	double factor;

	if (building[right].x - building[left].x > 30) {
		/* only looks good on narraw buildings, if it's too wide */
		/* skip the mosquey roof and just indent */
		indent_roof(building, npoints, left, right);
		return;
	}

	memcpy(p, mosquey_roof, sizeof(mosquey_roof));

	width = building[right].x - building[left].x; 
	factor = (0.0 + width) / 20.0;

	for (i=0;i<sizeof(p)/sizeof(p[0]);i++) {
		p[i].x = (p[i].x * factor) + building[left].x + (10 * factor);
		p[i].y = (p[i].y * factor) + building[left].y;
	}
	insert_points(building, npoints, p, 
		sizeof(mosquey_roof) / sizeof(mosquey_roof[0]), right);
	
	return;
}

static void peak_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	struct my_point_t p;
	int height;
	height = randomab(20,140);
	height = (int) (height * (building[right].x-building[left].x) / 100.0);
	if (height > 60)
		height = 60;

	p.x = ((building[right].x - building[left].x) / 2) + building[left].x;
	p.y = building[right].y - height;

	insert_points(building, npoints, &p, 1, right);
	
	return;
}

static void turret_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	struct my_point_t p[8];
	int height, indent, width;
	int old_npoints;

	width = randomab(15,30);
	height = randomab(20,60);
	indent = randomab(0,10);
	height = (int) (height * (building[right].x-building[left].x) / 100.0);
	indent = (int) (indent * (building[right].x-building[left].x) / 100.0);
	width = (int) (width * (building[right].x-building[left].x) / 100.0);
	
	p[0].x = building[left].x - indent;
	p[0].y = building[left].y - indent;

	p[1].x = p[0].x;
	p[1].y = p[0].y - height;

	p[2].x = p[1].x + width;
	p[2].y = p[1].y;

	p[3].x = p[2].x;
	p[3].y = p[0].y;

	p[4].x = building[right].x - width + indent; 
	p[4].y = p[3].y;

	p[5].x = p[4].x; 
	p[5].y = p[4].y - height;

	p[6].x = p[5].x + width; 
	p[6].y = p[5].y;

	p[7].x = p[6].x; 
	p[7].y = p[6].y + height;

	insert_points(building, npoints, p, 8, right);
	old_npoints = *npoints;

	embellish_roof(building, npoints, left+2, left+3);

	embellish_roof(building, npoints, left+6 + *npoints - old_npoints, left+7 + *npoints -old_npoints);
	embellish_roof(building, npoints, left+4 + *npoints - old_npoints, left+5 + *npoints -old_npoints);

	return;
}

static void indent_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	struct my_point_t p[4];
	int indent;
	int height;
	indent = randomab(5,25);
	height = randomab(-20,30);

	height = (int) (height * (building[right].x-building[left].x) / 100.0);
	indent = (int) (indent * (building[right].x-building[left].x) / 100.0);
	
	p[0].x = building[left].x + indent;
	p[0].y = building[left].y;

	p[1].x = p[0].x;
	p[1].y = building[left].y - height;
	p[2].x = building[right].x - indent;
	p[2].y = p[1].y;
	p[3].x = p[2].x;
	p[3].y = building[right].y;

	insert_points(building, npoints, p, 4, right);
	embellish_roof(building, npoints, left+4, left+5);
	embellish_roof(building, npoints, left+2, left+3);

	return;
}

static void addtower_roof(struct my_point_t *building, int *npoints, int left, int right)
{

	return;
}

static void crenellate_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	return;
}


static void embellish_roof(struct my_point_t *building, int *npoints, int left, int right)
{
	int r = randomn(10);

	switch (r) {
		case 1: peak_roof(building, npoints, left, right);
			break;
		case 2:turret_roof(building, npoints, left, right);
			break;
		case 3: add_mosquey_roof(building, npoints, left, right);
			break;
		case 4:
		case 5: indent_roof(building, npoints, left, right);
			break;
		case 8: crenellate_roof(building, npoints, left, right);
			break;
		case 9: addtower_roof(building, npoints, left, right);
			break;
		default:
			break;
	}
}

static void add_window(struct my_point_t *building, int *npoints, 
		int x1, int y1, int x2, int y2)
{
	/* printf("aw: npoints = %p\n", npoints); fflush(stdout); */
	if (*npoints > 1000) {
		/* printf("npoints = %d\n", *npoints); fflush(stdout); */
		return;
	}
	building[*npoints].x = LINE_BREAK; 
	building[*npoints].y = LINE_BREAK; *npoints += 1;
	building[*npoints].x = x1; 
	building[*npoints].y = y1; *npoints += 1;
	building[*npoints].x = x1; 
	building[*npoints].y = y2; *npoints += 1;
	building[*npoints].x = x2; 
	building[*npoints].y = y2; *npoints += 1;
	building[*npoints].x = x2; 
	building[*npoints].y = y1; *npoints += 1;
	building[*npoints].x = x1; 
	building[*npoints].y = y1; *npoints += 1;
}

static void add_windows(struct my_point_t *building, int *npoints, 
		int x1, int y1, int x2, int y2)
{
	int nwindows;
	int xindent, yindent;
	int spacing;
	int width;
	int i;

	/* printf("aws: npoints = %p\n", npoints); fflush(stdout); */
	xindent = randomab(5, 20);
	yindent = randomab(5, 20);
	spacing = randomab(3, 15);
	x2 -= xindent;
	x1 += xindent;
	y1 -= yindent;
	y2 += yindent;
	/* printf("add_windows, %d,%d  %d,%d\n", x1, y1, x2, y2); */

	if (x2 - x1 < 30)
		return;
	if (y1 - y2 < 20)
		return;

	nwindows = randomab(1, 5);
	width = (x2-x1) / nwindows; 
	/* printf("adding %d windows, *npoints = %d\n", nwindows, *npoints); */
	for (i=0;i<nwindows;i++) {
		/* printf("Adding window -> %d, %d, %d, %d, npoints = %d\n",
			x1 + (i*width), y1, x1 + (i+1)*width - spacing, y2, *npoints);
		fflush(stdout); */
		add_window(building, npoints,
			x1 + (i*(width)), y1, x1 + (i+1)*(width) - spacing, y2);
	}
	return;
}

static void embellish_building(struct my_point_t *building, int *npoints)
{
	int x1, y1, x2, y2;

	x1 = building[0].x;
	y1 = building[0].y;
	x2 = building[2].x;
	y2 = building[2].y;
	embellish_roof(building, npoints, 1, 2);
	add_windows(building, npoints, x1, y1, x2, y2);
	return;
}

#if 0
int find_free_obj()
{
	int i;
	for (i=0;i<MAXOBJS;i++)
		if (!game_state.go[i].alive)
			return i;
	return -1;
}
#endif

static inline void clearbit(unsigned int *value, unsigned char bit)
{
	*value &= ~(1 << bit);
}


int find_free_obj()
{
	int i, j, answer;
	unsigned int block;

	/* this might be optimized by find_first_zero_bit, or whatever */
	/* it's called that's in the linux kernel.  But, it's pretty */
	/* fast as is, and this is portable without writing asm code. */
	/* Er, portable, except for assuming an int is 32 bits. */

	for (i=0;i<NBITBLOCKS;i++) {
		if (free_obj_bitmap[i] == 0xffffffff) /* is this block full?  continue. */
			continue;

		/* Not full. There is an empty slot in this block, find it. */
		block = free_obj_bitmap[i];			
		for (j=0;j<32;j++) {
			if (block & 0x01) {	/* is bit j set? */
				block = block >> 1;
				continue;	/* try the next bit. */
			}

			/* Found free bit, bit j.  Set it, marking it non free.  */
			free_obj_bitmap[i] |= (1 << j);
			answer = (i * 32 + j);	/* return the corresponding array index, if in bounds. */
			if (answer < MAXOBJS)
				return answer;
			return -1;
		}
	}
	return -1;
}

static void add_building(struct terrain_t *t, int xi)
{
	int npoints = 0;
	int height;
	int width;
	struct my_point_t scratch[1000];
	struct my_point_t *building;
	struct my_vect_obj *bvec; 
	int objnum;
	struct game_obj_t *o;
	int i, x, y;
	int roofy;

	memset(scratch, 0, sizeof(scratch[0]) * 1000);
	objnum = find_free_obj();
	if (objnum == -1)
		return;

	height = randomab(50, 300); 
	width = randomab(5,MAXBUILDING_WIDTH);
	scratch[0].x = t->x[xi];	
	scratch[0].y = t->y[xi];	

	/* choose the roof level to be based on the highest of the left 
	   or right side of terrain on which the building is set. */
	roofy = t->y[xi] < t->y[xi+width] ? t->y[xi] - height : t->y[xi+width] - height;

	scratch[1].x = scratch[0].x;
	scratch[1].y = roofy;
	scratch[2].x = t->x[xi+width];
	scratch[2].y = roofy;        /* make roof level, even if ground isn't. */
				     /* FIXME< roof level may be _below_ of */
				     /*of the gournd points, not good.. */
	scratch[3].x = scratch[2].x;
	scratch[3].y = t->y[xi+width];
	npoints = 4;

	y = scratch[1].y;
	x = ((scratch[2].x - scratch[0].x) / 2) + scratch[0].x;

	for (i=0;i<npoints;i++) {
		scratch[i].x -= x;
		scratch[i].y -= y;
	}

	embellish_building(scratch, &npoints);

	building = malloc(sizeof(scratch[0]) * npoints);
	bvec = malloc(sizeof(bvec));
	if (building == NULL || bvec == NULL)
		return;

	memcpy(building, scratch, sizeof(scratch[0]) * npoints);
	bvec->p = building;
	bvec->npoints = npoints;

	o = &game_state.go[objnum];
	o->x = x;
	o->y = y;
	o->vx = 0;
	o->vy = 0;
	o->v = bvec;
	o->move = no_move;
	o->otype = OBJ_TYPE_BUILDING;
	o->target = add_target(o);
	o->color = BLUE;
	o->alive = 1;
	o->draw = NULL;
	o->move = NULL;
	o->destroy = generic_destroy_func;
	/* printf("b, x=%d, y=%d\n", x, y); */
}

static void add_buildings(struct terrain_t *t)
{
	int xi, i;

	for (i=0;i<level.nbuildings;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_building(t, xi);
	}
}

static int find_dip(struct terrain_t *t, int n, int *px1, int *px2, int minlength)
{
	/*	Find the nth dip in the terrain, over which we might put a bridge. */

	int i, j, x1, x2, found, highest, lowest;
	/* printf("top of find_dip, n=%d\n", n); */

	found=0;
	x1 = 0;
	x2 = 0; /* make the compiler quit bitching. */
	for (i=0;i<n;i++) {
		for (;x1<TERRAIN_LENGTH-100;x1++) {
			for (x2=x1+5; t->x[x2] - t->x[x1] < minlength; x2++)
				/* do nothing in body of for loop */ 
				;
			highest = 60000; /* highest altitude, lowest y value */
			lowest = -1; /* lowest altitude, highest y value */
			
			for (j=x1+1; j<x2-1;j++) {
				if (t->y[j] < highest)
					highest = t->y[j];
				if (t->y[j] > lowest)
					lowest = t->y[j];
			}

			/* Lowest point between x1,x2 must be at least 100 units deep */
			if (lowest - t->y[x1] < 100 || lowest - t->y[x2] < 100)
				continue; /* not a dip */

			/* highest point between x1,x2 must be lower than both */
			if (t->y[x1] >= highest || t->y[x2] >= highest)
				continue; /* not a dip */
	
			/* printf("found a dip x1=%d, x2=%d.\n", x1, x2); */
			break;
		}
		if (x1 >= TERRAIN_LENGTH-100)
			break;
		found++;
		if (found == n)
			break;
		x1 = x2;
	}
	/* printf("found %d dips.\n", found); */
	if (found == n) {
		*px1 = x1;
		*px2 = x2;
		return 0;
	}
	return -1;
}

static void add_bridge_piece(int x, int y)
{
	add_generic_object(x, y, 0, 0, no_move, NULL, RED, &bridge_vect, 1, OBJ_TYPE_BRIDGE, 1);
}

static void add_bridge_column(struct terrain_t *t, 
	int rx, int ry,  /* real coords */
	int x1, int x2) /* index into terrain */
{
	int i, terminal_y;
	i = x1;

	/* printf("add_bridge_column, rx =%d, ry=%d, x1 = %d, x2=%d\n", rx, ry, x1, x2); */
	while (t->x[i] <= rx && i <= x2)
		i++;
	if (i>x2)  /* we're at the rightmost end of the bridge, that is, done. */
		return;

	terminal_y = interpolate(rx, t->x[i-1], t->y[i-1], t->x[i], t->y[i]);
	/* printf("term_y = %d, intr(%d, %d, %d, %d, %d)\n", 
		terminal_y, rx, t->y[i-1], t->x[i-1], t->y[i], t->x[i]); */

	if (ry > terminal_y) /* shouldn't happen... */
		return;

	do {
		add_bridge_piece(rx, ry);
		ry += 8;
	} while (ry <= terminal_y);
	return;
}

static void add_bridge(struct terrain_t *t, int x1, int x2)
{
	int x, y;
	int rx1, rx2, ry1, ry2;

	ry1 = t->y[x1], 
	rx1 = t->x[x1], 
	ry2 = t->y[x2], 
	rx2 = t->x[x2], 

	/* Make the bridge span */
	x = rx1 - (rx1 % 40);
	while (x < rx2) {
		y = interpolate(x, rx1, ry1, rx2, ry2); 
		add_bridge_piece(x, y);
		if ((x % 40) == 32) {
			add_bridge_column(t, x, y+8, x1, x2);
		}
		x += 8;
	}
}

static void add_bridges(struct terrain_t *t)
{
	int i, n;
	int x1, x2;

	for (i=0;i<level.nbridges;i++) {
		n = find_dip(t, i+1, &x1, &x2, 600);
		if (n != 0) 
			n = find_dip(t, i+1, &x1, &x2, 500);
		if (n != 0)
			n = find_dip(t, i+1, &x1, &x2, 400);
		if (n != 0)
			n = find_dip(t, i+1, &x1, &x2, 300);
		if (n != 0)
			n = find_dip(t, i+1, &x1, &x2, 200);
		if (n == 0)
			add_bridge(t, x1, x2);
		else
			break;
	}
}

static void add_cron(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;
	for (i=0;i<level.ncron;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			cron_move, cron_draw, GREEN, &cron_vect, 1, OBJ_TYPE_CRON, 1);
		if (o != NULL) {
			o->destroy = cron_destroy;
			o->tsd.cron.tx = o->x;
			o->tsd.cron.ty = o->y;
			o->tsd.cron.myhuman = NULL;
			o->tsd.cron.eyepos = 0;
			o->radar_image = 1;
			o->tsd.cron.state = CRON_STATE_SEEKING_HUMAN;
			o->tsd.cron.pissed_off_timer = 0;
		}
	}
}

static struct game_obj_t *add_volcano(struct terrain_t *t, int x, int y)
{
	return add_generic_object(x, y, 0, 0, volcano_move, no_draw, RED, NULL, 0, OBJ_TYPE_VOLCANO, 1);
}

static void add_jammers(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;
	for (i=0;i<level.njammers;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi], 0, 0, 
			no_move, jammer_draw, GREEN, &jammer_vect, 1, OBJ_TYPE_JAMMER, 1);
		if (o) {
			o->tsd.jammer.width = 0;
			o->tsd.jammer.direction = 2;
			o->radar_image = 1;
		}
		
	}
}


static void add_fuel(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;
	for (i=0;i<level.nfueltanks;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			fuel_move, fuel_draw, ORANGE, &fuel_vect, 1, OBJ_TYPE_FUEL, 1);
		if (o) 
			o->tsd.fuel.level = FUELTANK_CAPACITY;
	}
}

static void add_ships(struct terrain_t *t)
{
	struct game_obj_t *o;
	int xi, i;
	for (i=0;i<level.nships;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			ship_move, NULL, ORANGE, &ship_vect, 1, OBJ_TYPE_SHIP, 50*PLAYER_LASER_DAMAGE);
		if (o)
			o->radar_image = 1;
	}
}

static void add_octopi(struct terrain_t *t)
{
	int xi, i, j, k, count;

	count = 0;
	struct game_obj_t *o;
	for (i=0;i<level.noctopi;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-50 - randomn(100), 0, 0, 
			octopus_move, NULL, YELLOW, &octopus_vect, 1, OBJ_TYPE_OCTOPUS, 1);
		if (o != NULL) {
			count++;
			o->destroy = octopus_destroy;
			o->tsd.octopus.awake = 0;
			o->radar_image = 1;

			/* Make the tentacles. */
			for (j=0;j<8;j++) {
				int length = randomn(30) + 9;
				double length_factor = 0.90;
				o->tsd.octopus.tentacle[j] = add_generic_object(o->x, o->y, 0, 0,
					tentacle_move, tentacle_draw, CYAN, NULL, 1, OBJ_TYPE_TENTACLE, 1);
				if (o->tsd.octopus.tentacle[j] != NULL) {
					struct game_obj_t *t = o->tsd.octopus.tentacle[j];
					t->tsd.tentacle.attached_to = o;
					t->tsd.tentacle.upper_angle = 359;
					t->tsd.tentacle.lower_angle = 181;
					t->tsd.tentacle.nsegs = MAX_TENTACLE_SEGS;
					t->tsd.tentacle.seg = (struct tentacle_seg_data *) 
						malloc(MAX_TENTACLE_SEGS * sizeof (t->tsd.tentacle.seg[0]));
					if (t->tsd.tentacle.seg != NULL) {
						t->tsd.tentacle.angle = 0;
						for (k=0;k<MAX_TENTACLE_SEGS;k++) {
							t->tsd.tentacle.seg[k].angle = 
								TENTACLE_RANGE(t->tsd.tentacle);
							t->tsd.tentacle.seg[k].length = length;
							t->tsd.tentacle.seg[k].angular_v = 0;
							t->tsd.tentacle.seg[i].dest_angle = 
								TENTACLE_RANGE(t->tsd.tentacle);
							length = length * length_factor;
							length_factor += 0.01;
							if (length_factor > 0.97) 
								length_factor = 0.97;
							if (length == 1) {
								t->tsd.tentacle.nsegs = k;
								break;
							}
						}
					}
				}
			}
		}
	
	}
}

static void add_gdbs(struct terrain_t *t)
{
	int xi, i; //, j, k, count;

	// count = 0;
	struct game_obj_t *o;
	for (i=0;i<level.ngdbs;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			gdb_move, gdb_draw, CYAN, &gdb_vect_left, 1, OBJ_TYPE_GDB, 1);
		if (o != NULL) {
			o->destroy = generic_destroy_func;
			o->tsd.gdb.awake = 0;
			o->radar_image = 1;

#if 0
			for (j=0;j<8;j++) {
				int length = randomn(30) + 9;
				double length_factor = 0.90;
				o->tsd.gdb.tentacle[j] = add_generic_object(o->x, o->y, 0, 0,
					tentacle_move, tentacle_draw, CYAN, NULL, 1, OBJ_TYPE_TENTACLE, 1);
				if (o->tsd.gdb.tentacle[j] != NULL) {
					struct game_obj_t *t = o->tsd.gdb.tentacle[j];
					t->tsd.tentacle.attached_to = o;
					t->tsd.tentacle.upper_angle = 340;
					t->tsd.tentacle.lower_angle = 200;
					t->tsd.tentacle.nsegs = MAX_TENTACLE_SEGS;
					t->tsd.tentacle.seg = (struct tentacle_seg_data *) 
						malloc(MAX_TENTACLE_SEGS * sizeof (t->tsd.tentacle.seg[0]));
					if (t->tsd.tentacle.seg != NULL) {
						t->tsd.tentacle.angle = 0;
						for (k=0;k<MAX_TENTACLE_SEGS;k++) {
							t->tsd.tentacle.seg[k].angle = randomn(60)-30;
							t->tsd.tentacle.seg[k].length = length;
							t->tsd.tentacle.seg[k].angular_v = 0;
							t->tsd.tentacle.seg[i].dest_angle = TENTACLE_RANGE(t->tsd.tentacle);
							length = length * length_factor;
							length_factor += 0.01;
							if (length_factor > 0.97) 
								length_factor = 0.97;
							if (length == 1) {
								t->tsd.tentacle.nsegs = k;
								break;
							}
						}
					}
				}
			}
#endif
		}
	
	}
}

/* Adds a bunch of loose tentacles sprinkled around the terrain. */
static void add_tentacles(struct terrain_t *t)
{
	int xi, i,j, length;
	struct game_obj_t *o;
	double length_factor;

	for (j=0;j<level.ntentacles;j++) {
		length = randomn(30) + 9;
		length_factor = 0.90;
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi], 0, 0,
			tentacle_move, tentacle_draw, CYAN, NULL, 1, OBJ_TYPE_TENTACLE, 1);
		if (o != NULL) {
			o->tsd.tentacle.upper_angle = 160;
			o->tsd.tentacle.lower_angle = 20;
			o->tsd.tentacle.nsegs = MAX_TENTACLE_SEGS;
			o->tsd.tentacle.seg = (struct tentacle_seg_data *) 
				malloc(MAX_TENTACLE_SEGS * sizeof (o->tsd.tentacle.seg[0]));
			if (o->tsd.tentacle.seg != NULL) {
				o->tsd.tentacle.angle = 0;
				for (i=0;i<MAX_TENTACLE_SEGS;i++) {
					o->tsd.tentacle.seg[i].angle = randomn(60)-30;
					o->tsd.tentacle.seg[i].length = length;
					o->tsd.tentacle.seg[i].angular_v = 0;
					o->tsd.tentacle.seg[i].dest_angle = TENTACLE_RANGE(o->tsd.tentacle);
					length = length * length_factor;
					length_factor += 0.01;
					if (length_factor > 0.97) 
						length_factor = 0.97;
					if (length == 1) {
						o->tsd.tentacle.nsegs = i;
						break;
					}
				}
			}
		}
	}

}

static void add_SAMs(struct terrain_t *t)
{
	int xi, i;
	for (i=0;i<level.nsams;i++) {
		xi = initial_x_location();
		add_generic_object(t->x[xi], t->y[xi], 0, 0, 
			sam_move, NULL, WHITE, &SAM_station_vect, 1, OBJ_TYPE_SAM_STATION, 1);
	}
}

static void add_humanoids(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;

	for (i=0;i<level.nhumanoids;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi], 0, 0, 
			humanoid_move, NULL, MAGENTA, &humanoid_vect, 1, OBJ_TYPE_HUMAN, 1);
		human[i] = o;
		if (o != NULL) {
			o->tsd.human.abductor = NULL;
			o->tsd.human.picked_up = 0;
			o->tsd.human.on_ground = 1;
			o->tsd.human.human_number = i;
			o->radar_image = 2;
		}
	}
}

struct game_obj_t *add_worm_tail(struct game_obj_t *parent, int coloroffset)
{
	int j;
	struct game_obj_t *o = NULL;

	if (parent->tsd.worm.parent != NULL && randomn(100) > 90)
		return NULL;

	o = add_generic_object(parent->x, parent->y, 0,0,
		worm_move, worm_draw, CYAN, NULL, 1, OBJ_TYPE_WORM, 1);
	if (o == NULL)
		return NULL;

	o->tsd.worm.parent = parent;
	o->counter = 0;
	o->radar_image = 1;
	o->tsd.worm.i = 0;
	o->tsd.worm.tx = o->x;
	o->tsd.worm.ty = o->y;
	o->tsd.worm.ltx = o->x;
	o->tsd.worm.lty = o->y;
	o->tsd.worm.coloroffset = coloroffset;
	for (j=0;j<NWORM_TIME_UNITS;j++) {
		o->tsd.worm.x[j] = o->x;
		o->tsd.worm.y[j] = o->y;
	}

	o->tsd.worm.child = add_worm_tail(o, coloroffset - 2);
	return o;
}

static void add_worms(struct terrain_t *t)
{
	int xi, i, j;
	struct game_obj_t *o;
	for (i=0;i<level.nworms;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-50, 0, 0, 
			worm_move, worm_draw, GREEN, NULL, 1, OBJ_TYPE_WORM, 1);
		if (o) {
			o->tsd.worm.parent = NULL;
			o->counter = 0;
			o->radar_image = 1;
			o->tsd.worm.i = 0;
			o->tsd.worm.tx = o->x;
			o->tsd.worm.ty = o->y;
			o->tsd.worm.ltx = o->x;
			o->tsd.worm.lty = o->y;
			o->tsd.worm.coloroffset = randomn(100)+100;
			for (j=0;j<NWORM_TIME_UNITS;j++) {
				o->tsd.worm.x[j] = o->x;
				o->tsd.worm.y[j] = o->y;
			}
		}
		o->tsd.worm.child = add_worm_tail(o, o->tsd.worm.coloroffset - 2);
	}
}

static void add_airships(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;
	for (i=0;i<level.nairships;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-50, 0, 0, 
			airship_move, airship_draw, CYAN, &airship_vect, 1, OBJ_TYPE_AIRSHIP, 300*PLAYER_LASER_DAMAGE);
		if (o) {
			o->counter = 0;
			o->radar_image = 4;
			o->tsd.airship.bannerline = 0;
			o->tsd.airship.pressure = 0;
		}
	}
}

static void add_socket(struct terrain_t *t)
{
	add_generic_object(t->x[TERRAIN_LENGTH-1] - 250, t->y[TERRAIN_LENGTH-1] - 250, 
		0, 0, socket_move, NULL, CYAN, &socket_vect, 0, OBJ_TYPE_SOCKET, 1);
}

static void add_balloons(struct terrain_t *t)
{
	struct game_obj_t *o;
	int xi, i;
	for (i=0;i<NBALLOONS;i++) {
		xi = initial_x_location();
		o = add_generic_object(t->x[xi], t->y[xi]-50, 0, 0, 
			balloon_move, NULL, CYAN, &balloon_vect, 1, OBJ_TYPE_BALLOON, 1);
		if (o) 
			o->radar_image = 1;
	}
}

static void draw_strings(GtkWidget *w);
void setup_text();

int they_used_the_source()
{

	/*
	 * A little joke for the programmers.  If *you* built it
	 * from source less than 1 hour ago you get....
	 * 
	 * a million bonus points.  
         *
	 * (Hey, the _very first_ thing the game tells you is 
	 * "Uuuuse the Soooource!!!!"  It's not kidding. :-)
	 *
	 */

	struct timeval now;
	int uid;

	gettimeofday(&now, NULL);
	uid = getuid();

	return (uid == builder && 
		now.tv_sec - builtat > 0 && 
		now.tv_sec - builtat < 60*60);
}

int bonus_points_this_round;
static int do_intermission(GtkWidget *w, GdkEvent *event, gpointer p)
{
#define LAST_INTERMISSION_STAGE 12
	static int intermission_stage = 0;
	static int intermission_timer = 0;
	static int add_bonus;
	int bonus_points = 0;
	int inc_bonus = 0;
	char s[100];
	int elapsed_secs;

	/* stop other events, if any... */
	next_timer = 0;
	timer_event = START_INTERMISSION_EVENT; 
	
	if (intermission_timer == 0) {
		add_sound(INTERMISSION_MUSIC_SOUND, MUSIC_SLOT);
	}

	if (timer >= intermission_timer) {
		add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);
		intermission_stage++;
		if (intermission_stage == LAST_INTERMISSION_STAGE-1
			&& !they_used_the_source())
			intermission_stage++;
		/* printf("intermission_stage  %d -> %d\n", 
 * 			intermission_stage-1,intermission_stage); */
		intermission_timer = timer + 
			(intermission_stage == LAST_INTERMISSION_STAGE ? 4 : 1) * 
			frame_rate_hz;
	}

	/* printf("timer=%d, timer_event=%d, intermission_timer=%d, stage=%d\n", 
		timer, timer_event, intermission_timer, intermission_stage); */

	cleartext();
	set_font(SMALL_FONT);
	switch (intermission_stage) {
		/* Cases are listed backwards to take advantage of fall-thru */
		case LAST_INTERMISSION_STAGE:
			timer_event = END_INTERMISSION_EVENT;
			next_timer = timer + 1;
			intermission_timer = 0;
			intermission_stage = 0;
			game_state.score += bonus_points_this_round;
			/* printf("added %d points to score --> %d\n", 
				bonus_points_this_round, game_state.score); */
			cleartext();
			gotoxy(0,0);
			sprintf(s, "Credits: %d Lives: %d", credits, game_state.lives);
			gameprint(s);
			break;
		case LAST_INTERMISSION_STAGE-1:
			if (they_used_the_source()) {
				gotoxy(5, LAST_INTERMISSION_STAGE-1+2);
				inc_bonus = 100000; /* million bonus points */
				sprintf(s, "Using The Source                   %6d",
					inc_bonus);
				bonus_points += inc_bonus;	
				add_bonus = bonus_points;
				gameprint(s);
			}
		case 10: 
			gotoxy(5, 10+2);
			elapsed_secs = game_state.finish_time.tv_sec - game_state.start_time.tv_sec;
			if (elapsed_secs < 90)
				inc_bonus = (90 - elapsed_secs) * 100;
			else
				inc_bonus = 0;
			sprintf(s, "Elapsed time               %02d:%02d    %5d",
				elapsed_secs / 60, elapsed_secs % 60, inc_bonus);
			bonus_points += inc_bonus;	
			add_bonus = bonus_points;
			gameprint(s);
		case 9: 
			gotoxy(5, 9+2);
			if (game_state.sams_killed >= level.nsams)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			bonus_points += inc_bonus;
			sprintf(s, "SAM stations destroyed:    %2d/%2d    %5d",
				game_state.sams_killed, level.nsams, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 8: 
			gotoxy(5, 8+2);
			if (game_state.guns_killed >= level.nflak)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Laser turrets killed:      %2d/%2d    %5d", 
				game_state.guns_killed, level.nflak, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 7: 
			gotoxy(5, 7+2);
			sprintf(s, "Missiles killed:              %2d    %5d",
				game_state.missiles_killed, 0);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 6: 
			gotoxy(5, 6+2);
			if (game_state.rockets_killed >= level.nrockets)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Rockets killed:            %2d/%2d    %5d",
				game_state.rockets_killed, level.nrockets, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 5: 
			gotoxy(5, 5+2);
			if (game_state.octos_killed >= level.noctopi)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Octo-viruses killed:       %2d/%2d    %5d",
				game_state.octos_killed, level.noctopi, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 4: 
			gotoxy(5, 4+2);
			if (game_state.gdbs_killed >= level.ngdbs)
				inc_bonus = 5000;
			else
				inc_bonus = 10*game_state.gdbs_killed;
			sprintf(s, "gdb processes killed:      %2d/%2d    %5d",
				game_state.gdbs_killed, level.ngdbs, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 3: 
			gotoxy(5, 3+2);
			inc_bonus = game_state.emacs_killed * 100;
			if (game_state.emacs_killed == level.nairships)
				inc_bonus += 20000;
			sprintf(s, "Emacs processes killed:    %2d/%2d    %5d",
				game_state.emacs_killed, level.nairships, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 2: 
			gotoxy(5, 2+2);
			inc_bonus = game_state.humanoids * 1000;
			if (game_state.humanoids == level.nhumanoids)
				inc_bonus = game_state.humanoids * 5000;
			sprintf(s, "vi .swp files rescued:     %2d/%2d    %5d", 
				game_state.humanoids, level.nhumanoids, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 1: 
			gotoxy(5, 1);
			sprintf(s, "Node cleared! Total bonus points:  %6d\n", bonus_points);
			bonus_points_this_round = bonus_points;
			gameprint( s);
	}
	if (intermission_stage != LAST_INTERMISSION_STAGE)
		draw_strings(w);
	else {
		game_state.score += add_bonus;
		printf("added %d points to score --> %d\n", add_bonus, game_state.score);
		setup_text();
	}
	/* printf("i, timer_event = %d\n", timer_event); */
	return 0;
}

char spinner[] = "||||////----\\\\\\\\";
char radar_msg1[] = "  Sirius Cybernetics Corp. RADAR -- firmware v. 1.03 (bootleg)";
char radar_msg2[] = "  Fly Safe!!! Fly Siriusly Safe!!!";

void draw_radar(GtkWidget *w)
{
	int xoffset, height, width, yoffset; 
	int x1, y1, x2, y2;
	char statusstr[100]; 
/*
	int viewport_left, viewport_right, viewport_top, viewport_bottom;
	int y_correction;
*/
	
	height = RADAR_HEIGHT;
	xoffset = RADAR_LEFT_MARGIN;
	yoffset = RADAR_YMARGIN;
	width = SCREEN_WIDTH-RADAR_LEFT_MARGIN - RADAR_RIGHT_MARGIN;

	y2 = SCREEN_HEIGHT - yoffset;
	y1 = y2 - height;
	x1 = xoffset;
	x2 = SCREEN_WIDTH - RADAR_RIGHT_MARGIN;

	gdk_gc_set_foreground(gc, &huex[RED]);

	/* draw radar border */
	wwvi_draw_line(w->window, gc, x1, y1, x1, y2);
	wwvi_draw_line(w->window, gc, x1, y2, x2, y2);
	wwvi_draw_line(w->window, gc, x2, y2, x2, y1);
	wwvi_draw_line(w->window, gc, x2, y1, x1, y1);

	/* draw health bar */
	gdk_gc_set_foreground(gc, &huex[ORANGE]);
	if (game_state.health > 0)
		wwvi_draw_rectangle(w->window, gc, TRUE, RADAR_LEFT_MARGIN, 
			SCREEN_HEIGHT - RADAR_HEIGHT - RADAR_YMARGIN - 10, 
			((SCREEN_WIDTH - RADAR_LEFT_MARGIN - RADAR_RIGHT_MARGIN) * game_state.health / MAXHEALTH), 
			RADAR_YMARGIN - 5);
#if 0
	/* calculate viewport edges projected onto radar screen. */

	viewport_left = ((SCREEN_WIDTH - (2*RADAR_XMARGIN)) * game_state.x) / WORLDWIDTH + RADAR_XMARGIN;
	viewport_right = ((SCREEN_WIDTH - (2*RADAR_XMARGIN)) * (game_state.x + SCREEN_WIDTH)) / WORLDWIDTH + RADAR_XMARGIN;
	viewport_top = SCREEN_HEIGHT - (RADAR_HEIGHT >> 1) - RADAR_YMARGIN + ((game_state.y * RADAR_HEIGHT) / 1500);
	viewport_bottom = SCREEN_HEIGHT - (RADAR_HEIGHT >> 1) - RADAR_YMARGIN + (((game_state.y+SCREEN_HEIGHT) * RADAR_HEIGHT) / 1500);

	y_correction = SCREEN_HEIGHT - (RADAR_HEIGHT >> 1) - RADAR_YMARGIN + ((player->y * RADAR_HEIGHT) / 1500);
	y_correction = (SCREEN_HEIGHT - (RADAR_HEIGHT >> 1)) - y_correction;

	viewport_bottom += y_correction;
	viewport_top += y_correction;

	if (viewport_top < y1) 
		viewport_top = y1;
	if (viewport_top > y2)
		viewport_top = y2;
	if (viewport_bottom < y1) 
		viewport_bottom = y1;
	if (viewport_bottom > y2)
		viewport_bottom = y2;

	if (viewport_left < x1) 
		viewport_left = x1;
	if (viewport_left > x2)
		viewport_left = x2;
	if (viewport_right < x1) 
		viewport_right = x1;
	if (viewport_right > x2)
		viewport_right = x2;

	wwvi_draw_line(w->window, gc, viewport_left, viewport_top, viewport_left, viewport_bottom);
	wwvi_draw_line(w->window, gc, viewport_right, viewport_top, viewport_right, viewport_bottom);
	wwvi_draw_line(w->window, gc, viewport_left, viewport_top, viewport_right, viewport_top);
	wwvi_draw_line(w->window, gc, viewport_left, viewport_bottom, viewport_right, viewport_bottom);
#endif

	if (game_state.corrosive_atmosphere && game_state.radar_state == RADAR_RUNNING) {
		gdk_gc_set_foreground(gc, &huex[GREEN]);
		abs_xy_draw_string(w, "Corrosive atmosphere detected!  Vacate the area immediately!", 
			TINY_FONT, x1 + 50, y1 + RADAR_HEIGHT/2);
	}

	gdk_gc_set_foreground(gc, &huex[RED]);
	sprintf(statusstr, "Score:   %7d", game_state.score);
	abs_xy_draw_string(w, statusstr, TINY_FONT, x2 + 5, y1 + 7);
	sprintf(statusstr, "Bombs:   %7d", game_state.nbombs);
	abs_xy_draw_string(w, statusstr, TINY_FONT, x2 + 5, y1 + 21);
	sprintf(statusstr, "Lives:   %7d", game_state.lives);
	abs_xy_draw_string(w, statusstr, TINY_FONT, x2 + 5, y1 + 35);
	sprintf(statusstr, "Credits: %7d", credits);
	abs_xy_draw_string(w, statusstr, TINY_FONT, x2 + 5, y1 + 49);
	sprintf(statusstr, "Humans:      %d/%d", game_state.humanoids, level.nhumanoids);
	abs_xy_draw_string(w, statusstr, TINY_FONT, x2 + 5, y1 + 63);

	if (game_state.radar_state == RADAR_RUNNING)
		return;

	if (game_state.radar_state == RADAR_FRITZED) {
		gdk_gc_set_foreground(gc, &huex[randomn(NCOLORS+NSPARKCOLORS+NRAINBOWCOLORS)]);
		wwvi_draw_line(w->window, gc, x1, y1 + timer % RADAR_HEIGHT, 
			x2,  y1 + timer % RADAR_HEIGHT);
	} else if (game_state.radar_state <= RADAR_BOOTUP) { /* radar is booting up, display bootup message. */
		gdk_gc_set_foreground(gc, &huex[GREEN]);
		radar_msg1[0] = spinner[timer % 16];
		abs_xy_draw_string(w, radar_msg1, TINY_FONT, x1 + 50, y1 + RADAR_HEIGHT/2-10);
		abs_xy_draw_string(w, radar_msg2, TINY_FONT, x1 + 80, y1 + RADAR_HEIGHT/2+10);
		game_state.radar_state --;
	}
}

/* This is the expose function of the main drawing area.  This is */
/* where everything gets drawn. */
static int main_da_expose(GtkWidget *w, GdkEvent *event, gpointer p)
{
	int i;
	int sx1, sx2;
	static int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;
	// int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;


	if (timer_event == START_INTERMISSION_EVENT) {
		do_intermission(w, event, p);
		return 0;
	}

	sx1 = game_state.x - 20;
	sx2 = game_state.x + SCREEN_WIDTH + 20;


	while (terrain.x[last_lowx] < sx1 && last_lowx+1 < TERRAIN_LENGTH)
		last_lowx++;
	while (terrain.x[last_lowx] > sx1 && last_lowx > 0)
		last_lowx--;
	while (terrain.x[last_highx] > sx2 && last_highx > 0)
		last_highx--;
	while (terrain.x[last_highx] < sx2 && last_highx+1 < TERRAIN_LENGTH) {
		last_highx++;
	}

	gdk_gc_set_foreground(gc, &huex[planet_color[level.ground_color]]);

	for (i=last_lowx;i<last_highx;i++) {
#if 0
		if (terrain.x[i] < sx1 && terrain.x[i+1] < sx1) /* offscreen to the left */
			continue;
		if (terrain.x[i] > sx2 && terrain.x[i+1] > sx2) /* offscreen to the right */
			continue;
#endif
#if 0
		if (zz < 5) {
				if (game_state.y < terrain.y[i+1] - 150) {
					game_state.vy = 3; 
					game_state.go[0].vy = 3;
				} else if (game_state.y > terrain.y[i+1] - 50) {
					game_state.vy = -3;
					game_state.go[0].vy = -3;
				} else {
					game_state.vy = 0;
					game_state.go[0].vy = 0;
				}
			zz++;
			printf(".\n");
		}
#endif
		wwvi_draw_line(w->window, gc, terrain.x[i] - game_state.x, terrain.y[i]+(SCREEN_HEIGHT/2) - game_state.y,  
					 terrain.x[i+1] - game_state.x, terrain.y[i+1]+(SCREEN_HEIGHT/2) - game_state.y);
	}
	gdk_gc_set_foreground(gc, &huex[RED]);

	/* draw "system memory boundaries" (ha!) */
	if (game_state.x > terrain.x[0] - SCREEN_WIDTH)
		wwvi_draw_line(w->window, gc, terrain.x[0] - game_state.x, 0, 
			terrain.x[0] - game_state.x, SCREEN_HEIGHT);
	if (game_state.x > terrain.x[TERRAIN_LENGTH-1] - SCREEN_WIDTH)
		wwvi_draw_line(w->window, gc, terrain.x[TERRAIN_LENGTH-1] - game_state.x, 0, 
			 terrain.x[TERRAIN_LENGTH-1] - game_state.x, SCREEN_HEIGHT); 
	set_font(SMALL_FONT);
	if (game_state.y < KERNEL_Y_BOUNDARY + SCREEN_HEIGHT) {
		wwvi_draw_line(w->window, gc, 0, KERNEL_Y_BOUNDARY  - game_state.y + SCREEN_HEIGHT/2, 
			SCREEN_WIDTH, KERNEL_Y_BOUNDARY - game_state.y + SCREEN_HEIGHT/2);
		livecursorx = (SCREEN_WIDTH - abs(game_state.x) % SCREEN_WIDTH);
		livecursory = KERNEL_Y_BOUNDARY - game_state.y + SCREEN_HEIGHT/2 - 10;
		draw_string(w, (unsigned char *) "Kernel Space");
	}

	if (want_starfield)
		draw_stars(w);
	draw_objs(w);
	draw_strings(w);
	draw_radar(w);

	return 0;
}
static void do_game_pause( GtkWidget *widget,
                   gpointer   data )
{
	if (game_pause)
		game_pause = 0;
	else
		game_pause = 1;
}

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below. */
static void destroy_event( GtkWidget *widget,
                   gpointer   data )
{
    /* g_print ("Bye bye.\n"); */
	exit(1); /* bad form to call exit here... */
}

static gboolean delete_event( GtkWidget *widget,
                              GdkEvent  *event,
                              gpointer   data )
{
    /* If you return FALSE in the "delete_event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */

    g_print ("delete event occurred\n");

    /* Change TRUE to FALSE and the main window will be destroyed with
     * a "delete_event". */

    return TRUE;
}

/* Another callback */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

void game_ended();
void start_level();
void timer_expired()
{
	static int game_over_count = 0;

	/* printf("timer expired, %d\n", timer_event); */
	switch (timer_event) {
	case BLINK_EVENT:
		setup_text();
		if (credits >= 1) {
			game_over_count = 0;
			next_timer = timer + 1;
			timer_event = GAME_ENDED_EVENT;
			strcpy(textline[GAME_OVER].string, " Game Over");
			break;
		}
		game_over_count++;
		timer_event = BLANK_GAME_OVER_1_EVENT;
		next_timer = timer + 20;
		strcpy(textline[GAME_OVER].string, " Game Over");
		break;
	case BLANK_GAME_OVER_1_EVENT:
		timer_event = INSERT_COIN_EVENT;
		next_timer = timer + 20;
		strcpy(textline[GAME_OVER].string, "");
		break;
	case INSERT_COIN_EVENT:
		timer_event = BLANK_GAME_OVER_2_EVENT;
		next_timer = timer + 20;
		strcpy(textline[GAME_OVER].string, "Insert Coin");
		break;
	case BLANK_GAME_OVER_2_EVENT:
		if (game_over_count >= 3) {
			int wherenext;
			wherenext = randomn(3); 
			switch (wherenext) {
			case 0: timer_event = CREDITS1_EVENT;
				break;
			case 1: timer_event = INTRO1_EVENT;
				break;
			case 2:
			default: timer_event = KEYS1_EVENT;
				break;
			}
		} else
			timer_event = BLINK_EVENT;
		next_timer = timer + 20;
		strcpy(textline[GAME_OVER].string, "");
		break;
	case CREDITS1_EVENT: {
		int yline = 6;
		int x = 12;
		ntextlines = 1;
		set_font(SMALL_FONT);
		gotoxy(x,yline++);
		yline++;
		gotoxy(x,yline++);
		gameprint("Programming:   Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("Game design:   Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("Music:         Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("               and Marty Kiel");
		gotoxy(x,yline++);
		gameprint("Sound effects: Stephen Cameron.");
		gotoxy(x,yline++);
		gotoxy(x,yline++);
		timer_event = CREDITS2_EVENT;
		next_timer = timer + 100;
		game_over_count = 0;
		break;
		}
	case CREDITS2_EVENT:
		ntextlines = 1;
		setup_text();
		timer_event = BLINK_EVENT;
		timer_event = BLANK_GAME_OVER_1_EVENT;
		next_timer = timer + 1;
		break;

	case INTRO1_EVENT: {
		int yline = 5;
		int x = 12;
		ntextlines = 1;
		set_font(SMALL_FONT);
		gotoxy(x,yline++);
		gameprint("In the beginning, there was ed."); gotoxy(x,yline++);
		gameprint("Ed is the standard text editor."); gotoxy(x,yline++);
		gameprint("Then there was vi, and it was good."); gotoxy(x,yline++);
		gameprint("Then came emacs, and disharmony."); gotoxy(x,yline++);
		gameprint("Your mission is to traverse core"); gotoxy(x,yline++);
		gameprint("memory and rid the host of emacs."); gotoxy(x,yline++);
		gameprint("It will not be an easy mission, as"); gotoxy(x,yline++);
		gameprint("there are many emacs friendly"); gotoxy(x, yline++);
		gameprint("processes."); gotoxy(x,yline++);
		timer_event = INTRO2_EVENT;
		next_timer = timer + 150;
		game_over_count = 0;
		break;
		}
	case INTRO2_EVENT: {
		ntextlines = 1;
		setup_text();
		timer_event = BLINK_EVENT;
		timer_event = BLANK_GAME_OVER_1_EVENT;
		// timer_event = KEYS1_EVENT;
		next_timer = timer + 1;
		break;
		}
	case KEYS1_EVENT: {
		int yline = 2;
		int x = 18;
		ntextlines = 1;
		set_font(SMALL_FONT);
		gotoxy(x,yline++);
		gameprint("Controls:"); yline++; gotoxy(x,yline++);
		gameprint("h - move left"); gotoxy(x,yline++);
		gameprint("l - move right"); gotoxy(x,yline++);
		gameprint("k - move up"); gotoxy(x,yline++);
		gameprint("j - move down"); gotoxy(x,yline++);
		gameprint("z - fires laser"); gotoxy(x,yline++);
		gameprint("b - drops bomb"); gotoxy(x,yline++);
		gameprint("c - drops chaff"); gotoxy(x,yline++);
		yline++;
		gameprint("Q - Insert Quarter"); gotoxy(x, yline++);
		gameprint("Esc to Exit"); gotoxy(x,yline++);
		next_timer = timer + 150;
		game_over_count = 0;
		timer_event = KEYS2_EVENT;
		break;
		}
	case KEYS2_EVENT: {
		ntextlines = 1;
		setup_text();
		timer_event = BLANK_GAME_OVER_1_EVENT;
		next_timer = timer + 1;
		break;
		}
	case READY_EVENT:
		start_level();
		add_sound(MUSIC_SOUND, MUSIC_SLOT);
		strcpy(textline[CREDITS].string, "");
		strcpy(textline[GAME_OVER].string, "Ready...");
		gettimeofday(&game_state.start_time, NULL);
		next_timer += 30;
		timer_event = SET_EVENT;
		ntextlines = 2;
		game_state.x = 0;
		game_state.y = 0;
		game_state.vy = 0;
		game_state.vx = 0;
		break;
	case SET_EVENT:
		strcpy(textline[GAME_OVER].string, "Set...");
		next_timer += 30;
		timer_event = GO_EVENT;
		break;
	case GO_EVENT:
		strcpy(textline[GAME_OVER].string, "Prepare to die!");
		next_timer += 30;
		timer_event = BLANK_EVENT;
		break;
	case BLANK_EVENT:
		ntextlines = 1;
		game_state.vx = PLAYER_SPEED;
		break;
	case GAME_ENDED_EVENT:
		timer_event = GAME_ENDED_EVENT_2;
		next_timer = timer + 30;
		if (credits <= 0) {
			setup_text();
			timer_event = GAME_ENDED_EVENT_2;
			next_timer = timer + 30;
			strcpy(textline[GAME_OVER+1].string, FINAL_MSG1);
			strcpy(textline[GAME_OVER].string, FINAL_MSG2);
			ntextlines = 4;
			next_timer = timer + 120;
		} else {
			strcpy(textline[GAME_OVER].string, "");
			timer_event = GAME_ENDED_EVENT_2;
			ntextlines = 2;
		}
		break;
	case GAME_ENDED_EVENT_2:
		next_timer = timer + 1;
		if (credits <= 0) {
			game_ended();
			start_level();
			timer_event = BLINK_EVENT;
			strcpy(textline[GAME_OVER].string, FINAL_MSG1);
			strcpy(textline[GAME_OVER+1].string, FINAL_MSG2);
			ntextlines = 4;
			next_timer = timer + 60;
		} else {
			timer_event = READY_EVENT; 
			ntextlines = 2;
		}
		game_ended();
		break;
	case START_INTERMISSION_EVENT:
		/* drawing area expose event handler handles this directly */
		break;
	case END_INTERMISSION_EVENT:
		advance_level();
		timer_event = READY_EVENT;
		ntextlines = 2;
		next_timer = timer+1;
		break;
	default: 
		break;
	}
}

void deal_with_joystick();

gint advance_game(gpointer data)
{
	int i, ndead, nalive;

	if (game_pause == 1) {
		return TRUE;
	}

	if (jsfd >= 0)
		deal_with_joystick();

	gdk_threads_enter();
	ndead = 0;
	nalive = 0;
	game_state.x += game_state.vx;
	game_state.y += game_state.vy; 


	timer++;
	if (timer == next_timer)
		timer_expired();

	if (timer_event == END_INTERMISSION_EVENT)
		return TRUE;

	game_state.missile_locked = 0;
	if (timer_event != START_INTERMISSION_EVENT) {
		for (i=0;i<MAXOBJS;i++) {
#if 0
			if (game_state.go[i].alive) {
				// printf("%d ", i);
				nalive++;
			} else {
				ndead++;
				clearbit(&free_obj_bitmap[i >> 5], i % 32);
			}
#endif

			if (game_state.go[i].alive && game_state.go[i].move != NULL)
				game_state.go[i].move(&game_state.go[i]);
			// if (game_state.go[i].alive && game_state.go[i].move == NULL)
				// printf("NULL MOVE!\n");
		}
		if (game_state.missile_locked && timer % 20 == 0)
			add_sound(MISSILE_LOCK_SIREN_SOUND, ANY_SLOT);
	}
	gtk_widget_queue_draw(main_da);
	nframes++;
	// printf("ndead=%d, nalive=%d\n", ndead, nalive);
	gdk_threads_leave();
#if 0
	if (WORLDWIDTH - game_state.x < 100)
		return FALSE;
	else
		return TRUE;
#endif
	return TRUE;
}

void setup_text()
{
	cleartext();
	gotoxy(4,1);
	gameprint("");
	set_font(BIG_FONT);
	gotoxy(4,3);
	gameprint(" Game Over\n");
	gotoxy(4,2);
	gameprint("Word War vi\n");
	set_font(SMALL_FONT);
	gotoxy(15,13);
	gameprint("(c) 2007 Stephen Cameron\n");
	timer_event = BLINK_EVENT;
	next_timer = timer + 30;
#if 0
	gotoxy(1,6);
	gameprint("abcdefghijklmn");
	gotoxy(1,7);
	gameprint("opqrstuvwxyz");
	gotoxy(1,8);
	gameprint("0123456789,.+-");
#endif
}

void initialize_game_state_new_level()
{
	game_state.lives = 3;
	game_state.humanoids = 0;
	game_state.prev_score = 0;
	game_state.health = MAXHEALTH;
	game_state.nbombs = level.nbombs;
	game_state.prev_bombs = -1;
	game_state.gdbs_killed = 0;
	game_state.crons_killed = 0;
	game_state.guns_killed = 0;
	game_state.sams_killed = 0;
	game_state.emacs_killed = 0;
	game_state.missiles_killed = 0;
	game_state.octos_killed = 0;
	game_state.rockets_killed = 0;
	game_state.cmd_multiplier = 1;
	game_state.radar_state = RADAR_BOOTUP;
	game_state.nextlasertime = timer;
	game_state.nextlasercolor = NCOLORS + NSPARKCOLORS;
	game_state.nextbombtime = timer;
	game_state.nextchafftime = timer;
}

void start_level()
{
	int i;


	for (i=0;i<MAXOBJS;i++) {
		game_state.go[i].alive = 0;
		game_state.go[i].vx = 0;
		game_state.go[i].vy = 0;
		game_state.go[i].move = move_obj;
	}
	memset(&game_state.go[0], 0, sizeof(game_state.go));
	memset(free_obj_bitmap, 0, sizeof(int) * NBITBLOCKS);
	init_object_numbers();
	init_stars();
	free_obj_bitmap[0] = 0x01;	
	game_state.humanoids = 0;
	game_state.direction = 1;
	player = &game_state.go[0];
	player->draw = player_draw;
	player->move = move_player;
	player->v = (game_state.direction == 1) ? &player_vect : &left_player_vect;
	player->x = 200;
	player->y = -100;
	player->vx = PLAYER_SPEED;
	player->vy = 0;
	player->target = add_target(player);
	player->alive = 1;
	
	player->destroy = generic_destroy_func;
	player->tsd.epd.count = -1;
	player->tsd.epd.count2 = 50;
	player->radar_image = 2;
	player->otype = OBJ_TYPE_PLAYER;
	game_state.health = MAXHEALTH;
	game_state.nbombs = level.nbombs;
	game_state.prev_bombs = -1;
	game_state.nobjs = MAXOBJS-1;
	game_state.x = 0;
	game_state.y = 0;
	game_state.radar_state = RADAR_BOOTUP;

	srandom(level.random_seed);
	generate_terrain(&terrain);
	add_rockets(&terrain);
	add_buildings(&terrain);
	add_fuel(&terrain);
	add_jammers(&terrain);
	add_cron(&terrain);
	add_ships(&terrain);
	add_SAMs(&terrain);
	add_humanoids(&terrain);
	add_bridges(&terrain);
	add_flak_guns(&terrain);
	add_kernel_guns(&terrain);
	add_airships(&terrain);
	add_worms(&terrain);
	add_balloons(&terrain);
	add_socket(&terrain);
	add_gdbs(&terrain);
	add_octopi(&terrain);
	add_tentacles(&terrain);

	if (credits == 0)
		setup_text();
	add_sound(USETHESOURCE_SOUND, ANY_SLOT);
	/* printf("use the force.\n"); */

}

void init_levels_to_beginning()
{
	level.nrockets = NROCKETS;
	level.nbridges = NBRIDGES;
	level.nflak = NFLAK;
	level.nfueltanks = NFUELTANKS;
	level.njammers = NJAMMERS;
	level.ncron = NCRON;
	level.nships = NSHIPS;
	level.ngdbs = NGDBS;
	level.noctopi = NOCTOPI;
	level.ntentacles = NTENTACLES;
	level.nsams = NSAMS;
	level.nairships = NAIRSHIPS;
	level.nworms = NWORMS;
	level.nbuildings = NBUILDINGS;
	level.nbombs = NBOMBS;
	level.nhumanoids = NHUMANOIDS;
	level.nkguns = NKGUNS;
	if (credits > 0) {
		level.random_seed = 31415927;
		level.laser_fire_chance = LASER_FIRE_CHANCE;
		level.large_scale_roughness = LARGE_SCALE_ROUGHNESS;
		level.small_scale_roughness = SMALL_SCALE_ROUGHNESS;;
	} else {
		level.random_seed = random();
		level.laser_fire_chance = LASER_FIRE_CHANCE;
		level.large_scale_roughness = LARGE_SCALE_ROUGHNESS;
		level.small_scale_roughness = 0.45;
		level.nflak = NFLAK + 20;
		level.nrockets = NROCKETS + 20;
	}
}

void game_ended()
{
	init_levels_to_beginning();
	initialize_game_state_new_level();
	game_state.score = 0;
	/* start_level(); */
}

void cancel_sound(int queue_entry);
void advance_level()
{
	/* This is harsh. */
	srandom(level.random_seed);
	level.random_seed = random(); /* deterministic */
	level.nrockets += 10;
	level.nbridges += 1;
	level.nflak += 5;
	level.nkguns += 5;
	level.nfueltanks += 2;
	level.njammers += 1;
	level.ncron += 1;
	level.nships += 1;
	level.ngdbs += 2;
	level.noctopi += 2;
	level.ntentacles += 2;
	level.nsams += 2;
	level.nairships += 1;
	level.nworms += 1;
	level.nhumanoids += 1; 
	if (level.nhumanoids > MAXHUMANS)
		level.nhumanoids = MAXHUMANS;
	level.large_scale_roughness+= (0.03);
	if (level.large_scale_roughness > 0.3)
		level.large_scale_roughness = 0.3;
	level.small_scale_roughness+=(0.03);
	if (level.small_scale_roughness > 0.60)
		level.small_scale_roughness = 0.60;
	// level.nbuildings;
	level.nbombs -= 5;
	if (level.nbombs < 50) 
		level.nbombs = 50;
	level.laser_fire_chance += 3; /* this is bad. */
	if (level.laser_fire_chance > 100)
		level.laser_fire_chance = 100;
	level.ground_color = (level.ground_color + 1) % (sizeof(planet_color) / sizeof(planet_color[0]));

	initialize_game_state_new_level();
	/* start_level(); */
}

void cancel_all_sounds();

void insert_quarter()
{
	credits++;
	if (credits == 1) {
		cancel_all_sounds();
		add_sound(INSERT_COIN_SOUND, ANY_SLOT);
		sleep(2);
		ntextlines = 1;
		game_ended();
		/* initialize_game_state_new_level();
		init_levels_to_beginning();
		start_level(); */
		init_levels_to_beginning();
		timer_event = READY_EVENT;
		next_timer = timer+1;
	} else
		add_sound(INSERT_COIN_SOUND, ANY_SLOT);
	strcpy(textline[CREDITS].string, "");
}

/* this is just a table to map joystick positions to numbers.  Notice there are more
 * lower numbers than higher numbers.  The idea is motion is finer in the middle, and
 * coarser at the edges.  I just pulled these numbers out of my ass, really.
 *
 * The -32767 to 0 range, and the 0 to 32767 range are mapped onto this array,
 * linearly, then the values in the array are used -- mapping it to nonlinear values.
 * This is used for the y axis.  
 *
 * The x axis of the joystick mostly has a big dead zone in the center, with
 * only things happening at the extreme edges of the stick's travel.
 *
 * Not sure that's the best way, but it seems to be sort of working ok,
 * which my previous schemes weren't really ok.
 */
int yjstable[] = { 1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2,  3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 10,
	11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 
	19, 20, 21, 22, 23, 24, 25 };
int nyjstable = (sizeof(yjstable)/sizeof(yjstable[0]));

void deal_with_joystick()
{
	int rc;
	/* why can I get away with this kind of initializer */
	/* in the linux kernel, but not here?  Well, not without the */
	/* compiler moaning, anyway. */
	static struct wwvi_js_event jse; 
	int index = 0;
	int newvy, diff;

#define JOYSTICK_SENSITIVITY 5000
#define XJOYSTICK_THRESHOLD 30000

	memset(&jse.button[0], 0, sizeof(jse.button[0]*10));
	rc = get_joystick_status(&jse);
	if (rc != 0)
		return;

	if (game_state.health <= 0 && credits >= 1)
		return;

	if (credits <= 0)
		goto no_credits;

	/* Stick 1 horizontal movement */	
	if (jse.stick1_x < -XJOYSTICK_THRESHOLD) {
		if (game_state.direction != -1) {
			player->vx = player->vx / 2;
			game_state.direction = -1;
			player->v = &left_player_vect;
		} else if (abs(player->vx + game_state.direction) < MAX_VX)
				player->vx += game_state.direction;
	} else if (jse.stick1_x > XJOYSTICK_THRESHOLD) {
		if (game_state.direction != 1) {
			player->vx = player->vx / 2;
			game_state.direction = 1;
			player->v = &player_vect;
		} else if (abs(player->vx + game_state.direction) < MAX_VX)
				player->vx += game_state.direction;
	} else {
		;
#if 0
		if (player->vx > 0 && (timer % 5) == 0)
			player->vx--;
		else if (player->vx < 0 && (timer % 5) == 0)
			player->vx++;
#endif
	}

	/* Stick 1 vertical movement */
	if (jse.stick1_y > JOYSTICK_SENSITIVITY) {
		if (player->vy < MAX_VY)
			index = nyjstable * jse.stick1_y / 32767;
			if (index < 0)
				index = 0;
			else if (index > nyjstable-1)
				index = nyjstable-1;
			newvy = yjstable[index];
			diff = newvy - player->vy;
			if (abs(diff) > 4)
				player->vy = player->vy + (diff >> 1);
			else 
				player->vy = player->vy + 2;
			// player->vy = yjstable[index];	
			// player->vy = MAX_VY * jse.stick1_y / 32767;
			// player->vy += 2;
			//player->vy += 4;
	} else if (jse.stick1_y < -JOYSTICK_SENSITIVITY) {
		if (player->vy > -MAX_VY)
			index = -nyjstable * jse.stick1_y / 32767;
			if (index < 0)
				index = 0;
			else if (index > nyjstable-1)
				index = nyjstable-1;
			newvy = -yjstable[index];
			diff = newvy - player->vy;
			if (abs(diff) > 4)
				player->vy = player->vy + (diff >> 1);
			else 
				player->vy = player->vy - 2;
			// player->vy = -yjstable[index];	
			// player->vy = MAX_VY * jse.stick1_y / 32767;
			// player->vy -= 4;
			//player->vy -= 4;
	} else {
		if (player->vy > 0)
			player->vy--;
		else if (player->vy < 0)
			player->vy++;
	}

	/* Buttons... */
	if (jse.button[5] == 1 || jse.button[2] == 1 || jse.button[1] == 1) {
		drop_bomb();
	} 

	if (jse.button[7] == 1 || jse.button[6] == 1 || jse.button[3] == 1 || jse.button[4] == 1) {
		player_fire_laser();
	} 

	if (jse.button[0] == 1) {
		drop_chaff();
	}

	/* buttons 8 or 9 on joystick will put in a quarter. */
	if ((jse.button[8] == 1 || jse.button[9] == 1) && timer > next_quarter_time) {
		insert_quarter();
		next_quarter_time = timer + (frame_rate_hz);
	}
	return;

no_credits:
	/* If credits are zero, or health is zero -- ANY button on the joystick */
	/* will put in a quarter. */
	if (timer > next_quarter_time) {
		if (jse.button[0] == 1 ||
			jse.button[1] == 1 ||
			jse.button[2] == 1 ||
			jse.button[3] == 1 ||
			jse.button[4] == 1 ||
			jse.button[5] == 1 ||
			jse.button[6] == 1 ||
			jse.button[7] == 1 ||
			jse.button[8] == 1 ||
			jse.button[9] == 1) {
			
			insert_quarter();
			next_quarter_time = timer + frame_rate_hz;
		}
	}
}

static gint key_press_cb(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
	/* char *x = (char *) data; */
#if 0
	if (event->length > 0)
		printf("The key event's string is `%s'\n", event->string);

	printf("The name of this keysym is `%s'\n", 
		gdk_keyval_name(event->keyval));
#endif
	switch (event->keyval)
	{
	case GDK_2:
			game_state.cmd_multiplier = 2;
			return TRUE;
	case GDK_3:
			game_state.cmd_multiplier = 3;
			return TRUE;
	case GDK_4:
			game_state.cmd_multiplier = 4;
			return TRUE;
	case GDK_5:
			game_state.cmd_multiplier = 5;
			return TRUE;
	case GDK_6:
			game_state.cmd_multiplier = 6;
			return TRUE;
	case GDK_8:
			player->tsd.epd.count = 50;
			player->tsd.epd.count2 = 0;
			break;
	case GDK_7:
			player->tsd.epd.count = -1;
			player->tsd.epd.count2 = 50;
			break;
	case GDK_9:
		if (credits > 0)
			game_state.health = -1;
		return TRUE;
	case GDK_s:
		game_state.sound_effects_on = !game_state.sound_effects_on;
		return TRUE;
	case GDK_m:
		game_state.music_on = !game_state.music_on;
		return TRUE;
	case GDK_Escape:
		    gettimeofday(&end_time, NULL);
		    printf("%d frames / %d seconds, %g frames/sec\n", 
				nframes, (int) (end_time.tv_sec - start_time.tv_sec),
				(0.0 + nframes) / (0.0 + end_time.tv_sec - start_time.tv_sec));
		destroy_event(widget, NULL);
		return TRUE;	
	case GDK_q:
		insert_quarter();
		return TRUE;
#if 0
	case GDK_Home:
		printf("The Home key was pressed.\n");
		return TRUE;
#endif
	case GDK_j:
	case GDK_Down:
		if (player->vy < MAX_VY && game_state.health > 0 && credits > 0)
			player->vy += 4 * game_state.cmd_multiplier;
		game_state.cmd_multiplier = 1;
		return TRUE;
	case GDK_k:
	case GDK_Up:
		if (player->vy > -MAX_VY && game_state.health > 0 && credits > 0)
			player->vy -= 4 * game_state.cmd_multiplier;
		return TRUE;
	case GDK_l:
	case GDK_Right:
	case GDK_period:
	case GDK_greater: {
		int i;

		for (i=0;i<game_state.cmd_multiplier;i++) {
			if (game_state.health <= 0 || credits <= 0)
				return TRUE;
			if (game_state.direction != 1) {
				game_state.direction = 1;
				player->vx = player->vx / 2;
				player->v = &player_vect;
			} else if (abs(player->vx + game_state.direction) < MAX_VX)
					player->vx += game_state.direction;
		}
		game_state.cmd_multiplier = 1;
		return TRUE;
	}
	case GDK_h:
	case GDK_Left:
	case GDK_comma:
	case GDK_less: {
		int i;

		for (i=0;i<game_state.cmd_multiplier;i++) {
			if (game_state.health <= 0 || credits <= 0)
				return TRUE;
			if (game_state.direction != -1) {
				player->vx = player->vx / 2;
				game_state.direction = -1;
				player->v = &left_player_vect;
			} else if (abs(player->vx + game_state.direction) < MAX_VX)
					player->vx += game_state.direction;
		}
		game_state.cmd_multiplier = 1;
		return TRUE;
	}
	case GDK_space:
	case GDK_z:
		if (game_state.health <= 0 || credits <= 0)
			return TRUE;
		player_fire_laser();
		return TRUE;
		break;	
	case GDK_x:
		if (game_state.health <= 0 || credits <= 0)
			return TRUE;
		if (abs(player->vx + game_state.direction) < MAX_VX)
			player->vx += game_state.direction;
		return TRUE;

	case GDK_c: if (game_state.health <= 0 || credits <= 0)
			return TRUE;
		drop_chaff();
		return TRUE;
	case GDK_b: if (game_state.health <= 0 || credits <= 0)
			return TRUE;
		drop_bomb();
		return TRUE;
	case GDK_p:
		// if (game_state.health <= 0 || credits <= 0)
		//	return TRUE;
		do_game_pause(widget, NULL);
		return TRUE;
	case GDK_n:
		if (game_state.health <= 0 || credits <= 0)
			return TRUE;
		player->x = human[lasthuman]->x;
		player->y = human[lasthuman]->y - 50;
		game_state.x = player->x;
		game_state.y = player->y - 50;
		lasthuman++;
		if (lasthuman >= level.nhumanoids)
			lasthuman = 0;
		break;
	case GDK_i:
		gettimeofday(&game_state.finish_time, NULL);
		timer_event = START_INTERMISSION_EVENT;
		next_timer = timer + 1;
		return TRUE;
#if 0
/* These two just for testing... */
	case GDK_R:
		start_level();
		break;
	case GDK_A:
		advance_level();
		break;
/* The above 2 just for testing */
#endif
	default:
		break;
	}

#if 0
	printf("Keypress: GDK_%s\n", gdk_keyval_name(event->keyval));
	if (gdk_keyval_is_lower(event->keyval)) {
		printf("A non-uppercase key was pressed.\n");
	} else if (gdk_keyval_is_upper(event->keyval)) {
		printf("An uppercase letter was pressed.\n");
	}
#endif
	return FALSE;
}

/***********************************************************************/
/* Beginning of AUDIO related code                                     */
/***********************************************************************/

#ifdef WITHAUDIOSUPPORT
struct sound_clip {
	int active;
	int nsamples;
	int pos;
	int16_t *sample;
} clip[NCLIPS];

struct sound_clip audio_queue[MAX_CONCURRENT_SOUNDS];

int nclips = 0;
#endif

int read_ogg_clip(int clipnum, char *filename)
{
#ifdef WITHAUDIOSUPPORT
	unsigned long long nframes;
	char filebuf[PATH_MAX];
	struct stat statbuf;
	int samplesize, sample_rate;
	int nchannels;
	int rc;

	strncpy(filebuf, filename, PATH_MAX);
	rc = stat(filebuf, &statbuf);
	if (rc != 0) {
		snprintf(filebuf, PATH_MAX, DATADIR"%s", filename);
		rc = stat(filebuf, &statbuf);
		if (rc != 0) {
			fprintf(stderr, "stat('%s') failed.\n", filebuf);
			return -1;
		}
	}
/*
	printf("Reading sound file: '%s'\n", filebuf);
	printf("frames = %lld\n", sfinfo.frames);
	printf("samplerate = %d\n", sfinfo.samplerate);
	printf("channels = %d\n", sfinfo.channels);
	printf("format = %d\n", sfinfo.format);
	printf("sections = %d\n", sfinfo.sections);
	printf("seekable = %d\n", sfinfo.seekable);
*/
	rc = ogg_to_pcm(filebuf, (uint16_t **) &clip[clipnum].sample, &samplesize,
		&sample_rate, &nchannels, &nframes);
	if (clip[clipnum].sample == NULL) {
		printf("Can't get memory for sound data for %llu frames in %s\n", 
			nframes, filebuf);
		goto error;
	}

	if (rc != 0) {
		fprintf(stderr, "Error: ogg_to_pcm('%s') failed.\n", 
			filebuf);
		goto error;
	}

	clip[clipnum].nsamples = (int) nframes;
	if (clip[clipnum].nsamples < 0)
		clip[clipnum].nsamples = 0;

	return 0;
error:
	return -1;
#else
	return 0;
#endif
}


#ifdef WITHAUDIOSUPPORT
int init_clips()
{
	memset(&audio_queue, 0, sizeof(audio_queue));

	printf("Decoding audio data..."); fflush(stdout);

	read_ogg_clip(PLAYER_LASER_SOUND, "sounds/synthetic_laser.ogg");
	read_ogg_clip(BOMB_IMPACT_SOUND, "sounds/bombexplosion.ogg");
	read_ogg_clip(ROCKET_LAUNCH_SOUND, "sounds/rocket_exhaust_1.ogg");
	read_ogg_clip(FLAK_FIRE_SOUND, "sounds/flak_gun_sound.ogg");
	read_ogg_clip(LARGE_EXPLOSION_SOUND, "sounds/big_explosion.ogg");
	read_ogg_clip(ROCKET_EXPLOSION_SOUND, "sounds/missile_explosion.ogg");
	read_ogg_clip(LASER_EXPLOSION_SOUND, "sounds/flak_hit.ogg");
	read_ogg_clip(GROUND_SMACK_SOUND, "sounds/new_ground_smack.ogg");
	read_ogg_clip(INSERT_COIN_SOUND, "sounds/us_quarter.ogg");
	read_ogg_clip(MUSIC_SOUND, "sounds/lucky13-steve-mono-mix.ogg");
	read_ogg_clip(SAM_LAUNCH_SOUND, "sounds/missile_launch_2.ogg");
	read_ogg_clip(THUNDER_SOUND, "sounds/synthetic_thunder_short.ogg");
	read_ogg_clip(INTERMISSION_MUSIC_SOUND, "sounds/dtox3monomix.ogg");
	read_ogg_clip(MISSILE_LOCK_SIREN_SOUND, "sounds/missile_alarm.ogg");
	read_ogg_clip(CARDOOR_SOUND, "sounds/toyota_celica_cardoor_sample.ogg");
	read_ogg_clip(WOOHOO_SOUND, "sounds/woohoo.ogg");
	read_ogg_clip(OWMYSPINE_SOUND, "sounds/ow_my_spine.ogg");
	read_ogg_clip(HELPDOWNHERE_SOUND, "sounds/help_down_here.ogg");
	read_ogg_clip(CRONSHOT, "sounds/synthetic_gunshot_2.ogg");
	read_ogg_clip(HELPUPHERE_SOUND, "sounds/help_up_here.ogg");
	read_ogg_clip(ABDUCTED_SOUND, "sounds/abducted.ogg");
	read_ogg_clip(CLANG_SOUND, "sounds/clang.ogg");
	read_ogg_clip(SCREAM_SOUND, "sounds/fallingscreamhi.ogg");
	read_ogg_clip(BODYSLAM_SOUND, "sounds/bodyslam.ogg");
	read_ogg_clip(USETHESOURCE_SOUND, "sounds/UseTheSource.ogg");
	read_ogg_clip(OOF_SOUND, "sounds/ooooof.ogg");
	read_ogg_clip(METALBANG1, "sounds/metalbang1.ogg");
	read_ogg_clip(METALBANG2, "sounds/metalbang2.ogg");
	read_ogg_clip(METALBANG3, "sounds/metalbang3.ogg");
	read_ogg_clip(METALBANG4, "sounds/metalbang4.ogg");
	read_ogg_clip(METALBANG5, "sounds/metalbang5.ogg");
	read_ogg_clip(METALBANG6, "sounds/metalbang6.ogg");
	read_ogg_clip(METALBANG7, "sounds/metalbang7.ogg");
	read_ogg_clip(METALBANG1, "sounds/metalbang1.ogg");
	read_ogg_clip(STONEBANG2, "sounds/stonebang2.ogg");
	read_ogg_clip(STONEBANG3, "sounds/stonebang3.ogg");
	read_ogg_clip(STONEBANG4, "sounds/stonebang4.ogg");
	read_ogg_clip(STONEBANG5, "sounds/stonebang5.ogg");
	read_ogg_clip(STONEBANG6, "sounds/stonebang6.ogg");
	read_ogg_clip(STONEBANG7, "sounds/stonebang7.ogg");
	read_ogg_clip(STONEBANG8, "sounds/stonebang8.ogg");
	read_ogg_clip(VOLCANO_ERUPTION, "sounds/volcano_eruption.ogg");
	read_ogg_clip(IT_BURNS, "sounds/aaaah_it_burns.ogg");
	read_ogg_clip(ZZZT_SOUND, "sounds/zzzt.ogg");
	printf("done.\n");
	return 0;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int patestCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags, __attribute__ ((unused)) void *userData )
{
	// void *data = userData; /* Prevent unused variable warning. */
	float *out = (float*)outputBuffer;
	int i, j, sample, count = 0;
	(void) inputBuffer; /* Prevent unused variable warning. */
	float output = 0.0;

	for (i=0; i<framesPerBuffer; i++) {
		output = 0.0;
		count = 0;
		for (j=0; j<MAX_CONCURRENT_SOUNDS; j++) {
			if (!audio_queue[j].active || 
				audio_queue[j].sample == NULL)
				continue;
			sample = i + audio_queue[j].pos;
			count++;
			if (sample >= audio_queue[j].nsamples) {
				audio_queue[j].active = 0;
				continue;
			}
			if (j != MUSIC_SLOT && game_state.sound_effects_on)
				output += (float) audio_queue[j].sample[sample] / (float) (INT16_MAX) ;
			else if (j == MUSIC_SLOT && game_state.music_on)
				output += (float) audio_queue[j].sample[sample] / (float) (INT16_MAX);
		}
		*out++ = (float) output / 2.0; /* (output / count); */
        }
	for (i=0;i<MAX_CONCURRENT_SOUNDS;i++) {
		if (!audio_queue[i].active)
			continue;
		audio_queue[i].pos += framesPerBuffer;
		if (audio_queue[i].pos >= audio_queue[i].nsamples)
			audio_queue[i].active = 0;
	}
	return 0; /* we're never finished */
}

static PaStream *stream = NULL;

void decode_paerror(PaError rc)
{
	if (rc == paNoError)
		return;
	fprintf(stderr, "An error occured while using the portaudio stream\n");
	fprintf(stderr, "Error number: %d\n", rc);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(rc));
}
#endif

#ifdef WITHAUDIOSUPPORT
void terminate_portaudio(PaError rc)
{
	Pa_Terminate();
	decode_paerror(rc);
}
#else
#define terminate_portaudio()
#endif

int initialize_portaudio()
{
#ifdef WITHAUDIOSUPPORT
	PaStreamParameters outparams;
	PaError rc;
	PaDeviceIndex device_count;

	init_clips();

	rc = Pa_Initialize();
	if (rc != paNoError)
		goto error;

	device_count = Pa_GetDeviceCount();
	printf("Portaudio reports %d sound devices.\n", device_count);

	if (device_count == 0) {
		printf("There will be no audio.\n");
		goto error;
		rc = 0;
	}
	sound_working = 1;
    
	outparams.device = Pa_GetDefaultOutputDevice();  /* default output device */

	printf("Portaudio says the default device is: %d\n", outparams.device);

	if (sound_device != -1) {
		if (sound_device >= device_count)
			fprintf(stderr, "wordwarvi:  Invalid sound device "
				"%d specified, ignoring.\n", sound_device);
		else
			outparams.device = sound_device;
		printf("Using sound device %d\n", outparams.device);
	}

	if (outparams.device < 0 && device_count > 0) {
		printf("Hmm, that's strange, portaudio says the default device is %d,\n"
			" but there are %d devices\n", 
			outparams.device, device_count);
		printf("I think we'll just skip sound for now.\n");
		printf("You might try the '--sounddevice' option and see if that helps.\n");
		sound_working = 0;
		return -1;
	}

	outparams.channelCount = 1;                      /* mono output */
	outparams.sampleFormat = paFloat32;              /* 32 bit floating point output */
	outparams.suggestedLatency = 
		Pa_GetDeviceInfo(outparams.device)->defaultLowOutputLatency;
	outparams.hostApiSpecificStreamInfo = NULL;

	rc = Pa_OpenStream(&stream,
		NULL,         /* no input */
		&outparams, SAMPLE_RATE, FRAMES_PER_BUFFER,
		paNoFlag, /* paClipOff, */   /* we won't output out of range samples so don't bother clipping them */
		patestCallback, NULL /* cookie */);    
	if (rc != paNoError)
		goto error;
	if ((rc = Pa_StartStream(stream)) != paNoError);
		goto error;
#if 0
	for (i=0;i<20;i++) {
		for (j=0;j<NCLIPS;j++) {
			// printf("clip[%d].pos = %d, active = %d\n", j, clip[j].pos, clip[j].active);
			Pa_Sleep( 250 );
			if (clip[j].active == 0) {
				clip[j].nsamples = CLIPLEN;
				clip[j].pos = 0;
				clip[j].active = 1;
			}
		}
		Pa_Sleep( 1500 );
	}
#endif
	return rc;
error:
	terminate_portaudio(rc);
	return rc;
#else
	return 0;
#endif
}


void stop_portaudio()
{
	if (!sound_working) 
		return;
#ifdef WITHAUDIOSUPPORT
	int rc;

	if ((rc = Pa_StopStream(stream)) != paNoError)
		goto error;
	rc = Pa_CloseStream(stream);
error:
	terminate_portaudio(rc);
#endif
	return;
}

int add_sound(int which_sound, int which_slot)
{
#ifdef WITHAUDIOSUPPORT
	int i;

	if (!sound_working)
		return 0;

	if (which_slot != ANY_SLOT) {
		if (audio_queue[which_slot].active)
			audio_queue[which_slot].active = 0;
		audio_queue[which_slot].pos = 0;
		audio_queue[which_slot].nsamples = 0;
		/* would like to put a memory barrier here. */
		audio_queue[which_slot].sample = clip[which_sound].sample;
		audio_queue[which_slot].nsamples = clip[which_sound].nsamples;
		/* would like to put a memory barrier here. */
		audio_queue[which_slot].active = 1;
		return which_slot;
	}
	for (i=1;i<MAX_CONCURRENT_SOUNDS;i++) {
		if (audio_queue[i].active == 0) {
			audio_queue[i].nsamples = clip[which_sound].nsamples;
			audio_queue[i].pos = 0;
			audio_queue[i].sample = clip[which_sound].sample;
			audio_queue[i].active = 1;
			break;
		}
	}
	return (i >= MAX_CONCURRENT_SOUNDS) ? -1 : i;
#else
	return 0;
#endif
}

void add_sound_low_priority(int which_sound)
{

	/* adds a sound if there are at least 5 empty sound slots. */
#ifdef WITHAUDIOSUPPORT
	int i;
	int empty_slots = 0;
	int last_slot;

	if (!sound_working)
		return;
	last_slot = -1;
	for (i=1;i<MAX_CONCURRENT_SOUNDS;i++)
		if (audio_queue[i].active == 0) {
			last_slot = i;
			empty_slots++;
			if (empty_slots >= 5)
				break;
	}

	if (empty_slots < 5)
		return;
	
	i = last_slot;

	if (audio_queue[i].active == 0) {
		audio_queue[i].nsamples = clip[which_sound].nsamples;
		audio_queue[i].pos = 0;
		audio_queue[i].sample = clip[which_sound].sample;
		audio_queue[i].active = 1;
	}
	return;
#endif
}


void cancel_sound(int queue_entry)
{
#ifdef WITHAUDIOSUPPORT
	if (!sound_working)
		return;
	audio_queue[queue_entry].active = 0;
#endif
}

void cancel_all_sounds()
{
#ifdef WITHAUDIOSUPPORT
	int i;
	if (!sound_working)
		return;
	for (i=0;i<MAX_CONCURRENT_SOUNDS;i++)
		audio_queue[i].active = 0;
#endif
}

/***********************************************************************/
/* End of AUDIO related code                                     */
/***********************************************************************/

void setup_rainbow_colors()
{

	int i, r, g, b, dr, dg, db, c;

	rainbow_color = &huex[NCOLORS + NSPARKCOLORS];

	
	r = 32766*2;
	g = 0;
	b = 0;

	dr = -r / NRAINBOWSTEPS;
	dg = r / NRAINBOWSTEPS;
	db = 0;

	c = 0;

	for (i=0;i<NRAINBOWSTEPS;i++) {
		rainbow_color[c].red = (unsigned short) r;
		rainbow_color[c].green = (unsigned short) g;
		rainbow_color[c].blue = (unsigned short) b;

		r += dr;
		g += dg;
		b += db;

		c++;
	}

	dg = (-32766*2) / NRAINBOWSTEPS;
	db = -dg;
	dr = 0;

	for (i=0;i<NRAINBOWSTEPS;i++) {
		rainbow_color[c].red = (unsigned short) r;
		rainbow_color[c].green = (unsigned short) g;
		rainbow_color[c].blue = (unsigned short) b;

		r += dr;
		g += dg;
		b += db;

		c++;
	}

	db = (-32766*2) / NRAINBOWSTEPS;
	dr = -db;
	dg = 0;

	for (i=0;i<NRAINBOWSTEPS;i++) {
		rainbow_color[c].red = (unsigned short) r;
		rainbow_color[c].green = (unsigned short) g;
		rainbow_color[c].blue = (unsigned short) b;

		r += dr;
		g += dg;
		b += db;

		c++;
	}
}

void setup_spark_colors()
{

/* Set up an array of colors that fade nicely from bright
   yellow to orange to red.  */

	int i, r,g,b, dr, dg, db;

	sparkcolor = &huex[NCOLORS];

	r = 32766*2;
	g = 32766*2;
	b = 32766;

	dr = 0;
	dg = (-(2500) / NSPARKCOLORS);
	db = 2 * dg;
	
	for (i=NSPARKCOLORS-1; i>=0; i--) {
		sparkcolor[i].red = (unsigned short) r;
		sparkcolor[i].green = (unsigned short) g;
		sparkcolor[i].blue = (unsigned short) b;
		sparkcolor[i].pixel = 0;

		r += dr;
		g += dg;
		b += db;

		if (r < 0) r = 0;
		if (g < 0) g = 0;
		if (b < 0) b = 0;

		dg *= 1.27;
		db *= 1.27;
	}
}

void paint_it_black()
{
	int i;
	unsigned int avg;
	for (i=0;i<NCOLORS + NSPARKCOLORS + NRAINBOWCOLORS;i++) {
		avg = huex[i].red + huex[i].green + huex[i].blue;
		avg = avg / 3;
		if (avg > 200)
			avg = 0;
		else
			avg = 32767*2;
		huex[i].red = avg;
		huex[i].green = avg;
		huex[i].blue = avg;
	}
}

static struct option wordwarvi_options[] = {
	{ "bw", 0, NULL, 0 },
	{ "sounddevice", 1, NULL, 1 },
	{ "version", 0, NULL, 2 },
	{ "brightsparks", 0, NULL, 3 },
	{ "width", 1, NULL, 4 },
	{ "height", 1, NULL, 5 },
	{ "framerate", 1, NULL, 6 },
	{ "nostarfield", 0, NULL, 7 },
	{ NULL, 0, NULL, 0 },
};

void usage()
{
	fprintf(stderr, "wordwarvi:  usage:\n");
	fprintf(stderr, "wordwarvi [options]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "--bw              Render in black and white.\n");
	fprintf(stderr, "--sounddevice n   Use the nth sound device for audio output.\n");
	fprintf(stderr, "--version         Print the version number and exit.\n");
	fprintf(stderr, "--brightsparks    Render sparks brighter than usual.\n");
	fprintf(stderr, "--width x         Render the game x pixels wide.\n");
	fprintf(stderr, "--height y        Render the game y pixels high.\n");
	fprintf(stderr, "--framerate n     Attempt to render the game at n frames per second.\n");
	fprintf(stderr, "--nostarfield n   Do not render the background starfield.\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	/* GtkWidget is the storage type for widgets */
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *vbox;
	GdkRectangle cliprect;
	int i;
	int no_colors_any_more = 0;
	int opt;

	struct timeval tm;

	real_screen_width = SCREEN_WIDTH;
	real_screen_height = SCREEN_HEIGHT;
	frame_rate_hz = FRAME_RATE_HZ;
	// current_draw_line = crazy_line;
	// current_draw_rectangle = crazy_rectangle;

	while (1) {
		int rc, n; 
		rc = getopt_long_only(argc, argv, "", wordwarvi_options, &opt);
		if (rc == -1)
			break;
		switch (rc) {
			case 0: no_colors_any_more = 1; /* --bw option */
				break;

			case 1: n = sscanf(optarg, "%d", &sound_device); /* --sounddevice option */
				if (n != 1) {
					fprintf(stderr, "wordwarvi: Bad sound device argument"
						" '%s', using 0.\n", optarg);
					sound_device = 0;
				}
				break;
			case 2: /* --version option */
				printf("Wordwarvi, version %s, (c) 2007,2008 Stephen M. Cameron.\n",
					WORDWARVI_VERSION);
				printf("Released under the GNU GPL v. 2.0 or later.  See the file\n");
				printf("COPYING, which should have accompanied this program, for\n");
				printf("information about redistributing this program.\n");
				printf("See http://wordwarvi.sourceforge.net for more information\n");
				printf("about this program.\n");
				exit(0);
			case 3: /* brightsparks */
				brightsparks = 1;
				break;
			case 4: /* width */
				real_screen_width = 800;
				current_draw_line = gdk_draw_line;
				current_draw_rectangle = gdk_draw_rectangle;
				n = sscanf(optarg, "%d", &real_screen_width);
				if (n != 1) {
					fprintf(stderr, "wordwarvi: Bad width argument"
						" '%s', using 800.\n", optarg);
					real_screen_width = 800;
					current_draw_line = gdk_draw_line;
					current_draw_rectangle = gdk_draw_rectangle;
					break;
				}
				if (real_screen_width >= 100 && real_screen_width <= 3000) {
					current_draw_line = scaled_line;
					current_draw_rectangle = scaled_rectangle;
				} else {
					fprintf(stderr, "wordwarvi: Bad width argument"
						" '%s', using 800.\n", optarg);
					real_screen_width = 800;
					real_screen_height = 600;
					current_draw_line = gdk_draw_line;
					current_draw_rectangle = gdk_draw_rectangle;
				}
				break;	
			case 5: /* height */
				real_screen_height = 600;
				current_draw_line = gdk_draw_line;
				current_draw_rectangle = gdk_draw_rectangle;
				n = sscanf(optarg, "%d", &real_screen_height);
				if (n != 1) {
					fprintf(stderr, "wordwarvi: Bad height argument"
						" '%s', using 600.\n", optarg);
					real_screen_height = 600;
					current_draw_line = gdk_draw_line;
					current_draw_rectangle = gdk_draw_rectangle;
					break;
				} 
				if (real_screen_height >= 100 && real_screen_height <= 2000) {
					current_draw_line = scaled_line;
					current_draw_rectangle = scaled_rectangle;
				} else {
					fprintf(stderr, "wordwarvi: Bad height argument"
						" '%s', using 800.\n", optarg);
					real_screen_width = 800;
					real_screen_height = 600;
					current_draw_line = gdk_draw_line;
					current_draw_rectangle = gdk_draw_rectangle;
				}
				break;	
			case 6: /* frame rate */
				n = sscanf(optarg, "%d", &frame_rate_hz);
				if (n != 1 || frame_rate_hz < 3 || frame_rate_hz > 300) {
					fprintf(stderr, "wordwarvi: Bad framerate argument"
						" '%s', using default %d frames/sec.\n", 
							optarg, FRAME_RATE_HZ);
					frame_rate_hz = FRAME_RATE_HZ;
				}
				break;
			case 7: /* --nostarfield */
				want_starfield = 0;
				break;
			case '?':usage(); /* exits. */
			default:printf("Unexpected return value %d from getopt_long_only()\n", rc);
				exit(0);
				
		}
	}

	xscale_screen = (float) real_screen_width / (float) SCREEN_WIDTH;
	yscale_screen = (float) real_screen_height / (float) SCREEN_HEIGHT;

	gettimeofday(&tm, NULL);
	srandom(tm.tv_usec);	

	jsfd = open_joystick();
	if (jsfd < 0) {
		printf("No joystick...\n");
	};
#ifdef WITHAUDIOSUPPORT
	if (initialize_portaudio() != paNoError)
		printf("Guess sound's not working...\n");
#endif
	gtk_set_locale();
	gtk_init (&argc, &argv);
   
	gdk_color_parse("white", &huex[WHITE]);
	gdk_color_parse("blue", &huex[BLUE]);
	gdk_color_parse("black", &huex[BLACK]);
	gdk_color_parse("green", &huex[GREEN]);
	gdk_color_parse("darkgreen", &huex[DARKGREEN]);
	gdk_color_parse("yellow", &huex[YELLOW]);
	gdk_color_parse("red", &huex[RED]);
	gdk_color_parse("orange", &huex[ORANGE]);
	gdk_color_parse("cyan", &huex[CYAN]);
	gdk_color_parse("MAGENTA", &huex[MAGENTA]);

	/* Set up the spark colors. */
	setup_spark_colors();
	setup_rainbow_colors();

	if (no_colors_any_more)
		paint_it_black();
 
    /* create a new window */
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (window), "delete_event",
		      G_CALLBACK (delete_event), NULL);
    
    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);
    
    /* Sets the border width of the window. */
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
   
	vbox = gtk_vbox_new(FALSE, 0); 
	main_da = gtk_drawing_area_new();
	gtk_widget_modify_bg(main_da, GTK_STATE_NORMAL, &huex[WHITE]);
	gtk_widget_set_size_request(main_da, real_screen_width, real_screen_height);

	g_signal_connect(G_OBJECT (main_da), "expose_event", G_CALLBACK (main_da_expose), NULL);

    button = gtk_button_new_with_label ("Quit");
    
    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (destroy_event), NULL);
    
    /* This will cause the window to be destroyed by calling
     * gtk_widget_destroy(window) when "clicked".  Again, the destroy
     * signal could come from here, or the window manager. */
    g_signal_connect_swapped (G_OBJECT (button), "clicked",
			      G_CALLBACK (gtk_widget_destroy),
                              G_OBJECT (window));
    
    /* This packs the button into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (window), vbox);

    gtk_box_pack_start(GTK_BOX (vbox), main_da, TRUE /* expand */, FALSE /* fill */, 2);
    gtk_box_pack_start(GTK_BOX (vbox), button, FALSE /* expand */, FALSE /* fill */, 2);
    
	init_vects();
	init_vxy_2_dxy();
	init_object_numbers();
	
	g_signal_connect(G_OBJECT (window), "key_press_event",
		G_CALLBACK (key_press_cb), "window");


	// print_target_list();

	for (i=0;i<NCOLORS+NSPARKCOLORS + NRAINBOWCOLORS;i++)
		gdk_colormap_alloc_color(gtk_widget_get_colormap(main_da), &huex[i], FALSE, FALSE);
	gtk_widget_modify_bg(main_da, GTK_STATE_NORMAL, &huex[BLACK]);


	make_debris_forms();
	initialize_game_state_new_level();
	init_radar_noise();
	game_state.score = 0;
	game_state.music_on = 1;
	game_state.sound_effects_on = 1;
	start_level();

    gtk_widget_show (vbox);
    gtk_widget_show (main_da);
    
    /* and the window */
    gtk_widget_show (window);
	gc = gdk_gc_new(GTK_WIDGET(main_da)->window);
	gdk_gc_set_foreground(gc, &huex[BLUE]);
	gdk_gc_set_foreground(gc, &huex[WHITE]);

	gdk_gc_set_clip_origin(gc, 0, 0);
	cliprect.x = 0;	
	cliprect.y = 0;	
	cliprect.width = real_screen_width;	
	cliprect.height = real_screen_height;	
	gdk_gc_set_clip_rectangle(gc, &cliprect);

    timer_tag = g_timeout_add(1000 / frame_rate_hz, advance_game, NULL);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */

    g_thread_init(NULL);
    gdk_threads_init();

    gettimeofday(&start_time, NULL);
    gtk_main ();
    stop_portaudio();
    free_debris_forms();
	close_joystick();
    
    return 0;
}
