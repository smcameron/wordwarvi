/* 
    (C) Copyright 2005,2006, Stephen M. Cameron.

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

#include <gdk/gdkkeysyms.h>
#include <math.h>

#ifdef WITHAUDIOSUPPORT
/* For Audio stuff... */
#include "portaudio.h"
#include <sndfile.h>
#endif

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (1024)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif
#define TWOPI (M_PI * 2.0)
#define NCLIPS (18)
#define MAX_CONCURRENT_SOUNDS (16)
#define CLIPLEN (12100) 
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
#define NHUMANOIDS 4

/* ...End of audio stuff */


#define FRAME_RATE_HZ 30
#define TERRAIN_LENGTH 1000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WORLDWIDTH (SCREEN_WIDTH * 40)
#define KERNEL_Y_BOUNDARY (-1000)

#define LARGE_SCALE_ROUGHNESS (0.04)
#define SMALL_SCALE_ROUGHNESS (0.09)
#define MAXOBJS 8500
#define NFLAK 10
#define LASER_BOLT_DAMAGE 5 /* damage done by flak guns to player */
#define PLAYER_LASER_DAMAGE 20
#define NROCKETS 20 
// #define NROCKETS 0
#define LAUNCH_DIST 500
#define MAX_ROCKET_SPEED -32
#define SAM_LAUNCH_DIST 400
#define GDB_LAUNCH_DIST 700
#define SAM_LAUNCH_CHANCE 15 
#define PLAYER_SPEED 8 
#define MAX_VX 15
#define MAX_VY 25
#define LASER_FIRE_CHANCE 10
#define LASERLEAD (11)
#define LASER_SPEED 40
#define LASER_PROXIMITY 300 /* square root of 300 */
#define BOMB_PROXIMITY 10000 /* square root of 30000 */
#define BOMB_X_PROXIMITY 100
#define GDB_DX_THRESHOLD 250
#define GDB_DY_THRESHOLD 250
#define GDB_MAX_VX 13 
#define GDB_MAX_VY 13 
#define LINE_BREAK (-999)
#define NBUILDINGS 20
#define NBRIDGES 7
#define MAXBUILDING_WIDTH 9
#define NFUELTANKS 20
#define NSHIPS 1
#define NGDBS 3 
#define NOCTOPI 0 
#define NTENTACLES 2 
#define NSAMS 5
#define BOMB_SPEED 10
#define NBOMBS 100
#define MAX_ALT 100
#define MIN_ALT 50
#define MAXHEALTH 100
#define NAIRSHIPS 1
#define NBALLOONS 3 
#define MAX_BALLOON_HEIGHT 300
#define MIN_BALLOON_HEIGHT 50
#define MAX_MISSILE_VELOCITY 19 
#define MISSILE_DAMAGE 20
#define MISSILE_PROXIMITY 10
#define MISSILE_FIRE_PERIOD (FRAME_RATE_HZ / 2);
#define HUMANOID_PICKUP_SCORE 1000
#define HUMANOID_DIST 15
#define MAX_TENTACLE_SEGS 40
#define MAX_SEG_ANGLE 60
#define TENTACLE_RANGE(t) (randomn(t.upper_angle - t.lower_angle) + t.lower_angle)
/* Scoring stuff */
#define ROCKET_SCORE 200
#define BRIDGE_SCORE 10 
#define FLAK_SCORE 250
#define OCTOPUS_SCORE 800 
#define SAM_SCORE 400
#define GDB_SCORE 400


int game_pause = 0;
int attract_mode = 0;
int credits = 0;
int toggle = 0;
int timer = 0;
int next_timer = 0;
int timer_event = 0;
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

#define NCOLORS 9
#define NSPARKCOLORS 25 

GdkColor huex[NCOLORS + NSPARKCOLORS];

GdkColor *sparkcolor;

#define WHITE 0
#define BLUE 1
#define BLACK 2
#define GREEN 3
#define YELLOW 4
#define RED 5
#define ORANGE 6
#define CYAN 7
#define MAGENTA 8


/* Object types */
#define OBJ_TYPE_AIRSHIP 'a'
#define OBJ_TYPE_BOMB 'p'
#define OBJ_TYPE_BALLOON 'B'
#define OBJ_TYPE_BUILDING 'b'
#define OBJ_TYPE_CHAFF 'c'
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

int current_level = 0;
struct level_parameters_t {
	int random_seed;
	int nrockets;
	int nbridges;
	int nflak;
	int nfueltanks;
	int nships;
	int ngdbs;
	int noctopi;
	int ntentacles;
	int nsams;
	int nhumanoids;
	int nbuildings;
	int nbombs;
	int nairships;
	int laser_fire_chance;
	double large_scale_roughness;
	double small_scale_roughness;
} level = {
	31415927,
	NROCKETS,
	NBRIDGES,
	NFLAK,
	NFUELTANKS,
	NSHIPS,
	NGDBS,
	NOCTOPI,
	NTENTACLES,
	NSAMS,
	NHUMANOIDS,
	NBUILDINGS,
	NBOMBS,
	LASER_FIRE_CHANCE,
	LARGE_SCALE_ROUGHNESS,
	SMALL_SCALE_ROUGHNESS,
};

/**** LETTERS and stuff *****/


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



struct target_t;

struct my_point_t {
	int x,y;
};


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

#define MAX_VELOCITY_TO_COMPUTE 20 
#define V_MAGNITUDE (20.0)
struct my_point_t vxy_2_dxy[MAX_VELOCITY_TO_COMPUTE+1][MAX_VELOCITY_TO_COMPUTE+1];

double sine[361];
double cosine[361];

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

struct my_point_t gdb_points_left[] = {
	{ 10, 0 },
	{ -20, 0 },
	{ -20, -5 },
	{ 0, -15, },
	{ 0, 15 },
	{ -5, 20 },
	{ -10, 20 },
	{ LINE_BREAK, LINE_BREAK },
	{ 10, -15 },
	{ 0, -15 },
	{ 0, 0 },
	{ 15, 0  },
	{ 15, -15  },
	{ 35, -45  },
	{ 38, -45  },
	{ 18, -15  },
	{ 18, 0  },
	{ 28, 0  },
	{ 28, -15  },
	{ 18, -15  },
};

struct my_point_t gdb_points_right[] = {
	{ 10, 0 },
	{ -20, 0 },
	{ -20, -5 },
	{ 0, -15, },
	{ 0, 15 },
	{ -5, 20 },
	{ -10, 20 },
	{ LINE_BREAK, LINE_BREAK },
	{ 10, -15 },
	{ 0, -15 },
	{ 0, 0 },
	{ 15, 0  },
	{ 15, -15  },
	{ 35, -45  },
	{ 38, -45  },
	{ 18, -15  },
	{ 18, 0  },
	{ 28, 0  },
	{ 28, -15  },
	{ 18, -15  },
};

struct my_point_t humanoid_points[] = {
	{ -5, 0 },
	{ -3, -5 },
	{ 3, -5, },
	{ 3, 0 },
	{ LINE_BREAK, LINE_BREAK },
	{ 3, -5 },
	{ 3, -9 },
	{ 1, -9 },
	{ 1, -12 },
	{ -1, -12 },
	{ -1, -9 },
	{ -3, -9 },
	{ LINE_BREAK, LINE_BREAK },
	{ 6, -5 },
	{ 3, -9 },
	{ -3, -9 },
	{ -6, -5 },
};

struct my_point_t SAM_station_points[] = {
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

struct my_point_t fuel_points[] = {
	{ -30, -10 },
	{ 30, -10 },
	{ 30, 30 },
	{ -30, 30 },
	{ -30, -10 },
	{ LINE_BREAK, LINE_BREAK },
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

struct my_point_t flak_points[] = {
	{ -10, 5 },
	{ -5, -3 },
	{ 5, -3},
	{ 10, 5 },
	{ LINE_BREAK, LINE_BREAK },
	{ -3, -3 },
	{ -3, -5},
	{ 3, -5},
	{ 3, -3},
};

struct my_point_t rocket_points[] = {
	{ -2, 3 },
	{ -4, 7 },
	{ -2, 7 },
	{ -2, -8 },
	{ 0, -10 },
	{ 2, -8 },
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
	{ 51, 15 },
	{ -12, 15 }, /* keel */
	{ LINE_BREAK, LINE_BREAK },
	{ 5, -5 },
	{ 8, -110 }, /* main mast */
	{ 0, -108 }, /* flag */
	{ 8, -105 },
	
	{ LINE_BREAK, LINE_BREAK },
	{ 39, -14 }, /* aft mast */
	{ 41, -95 },
	{ LINE_BREAK, LINE_BREAK },
	{ -22, -12 }, /* fore mast */
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

struct my_vect_obj {
	int npoints;
	struct my_point_t *p;	
};

struct my_vect_obj player_vect;
struct my_vect_obj left_player_vect;
struct my_vect_obj rocket_vect;
struct my_vect_obj spark_vect;
struct my_vect_obj right_laser_vect;
struct my_vect_obj fuel_vect;
struct my_vect_obj ship_vect;
struct my_vect_obj bomb_vect;
struct my_vect_obj bridge_vect;
struct my_vect_obj flak_vect;
struct my_vect_obj airship_vect;
struct my_vect_obj balloon_vect;
struct my_vect_obj SAM_station_vect;
struct my_vect_obj humanoid_vect;
struct my_vect_obj socket_vect;
struct my_vect_obj gdb_vect_right;
struct my_vect_obj gdb_vect_left;
struct my_vect_obj octopus_vect;
struct my_vect_obj sail_segment;
struct my_vect_obj ships_hull;

struct my_vect_obj **gamefont[3];
#define BIG_FONT 0
#define SMALL_FONT 1
#define TINY_FONT 2
#define BIG_FONT_SCALE 14 
#define SMALL_FONT_SCALE 5 
#define TINY_FONT_SCALE 3 
#define BIG_LETTER_SPACING (10)
#define SMALL_LETTER_SPACING (5)
#define TINY_LETTER_SPACING (3)
#define MAXTEXTLINES 20
int current_color = WHITE;
int current_font = BIG_FONT;
int cursorx = 0;
int cursory = 0;
int livecursorx = 0;
int livecursory = 0;
int font_scale[] = { BIG_FONT_SCALE, SMALL_FONT_SCALE, TINY_FONT_SCALE };
int letter_spacing[] = { BIG_LETTER_SPACING, SMALL_LETTER_SPACING, TINY_LETTER_SPACING };

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
					((double) y / (double) MAX_VELOCITY_TO_COMPUTE) * V_MAGNITUDE;
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

struct game_obj_t;

typedef void obj_move_func(struct game_obj_t *o);
typedef void obj_draw_func(struct game_obj_t *o, GtkWidget *w);
typedef void obj_destroy_func(struct game_obj_t *o);

struct extra_player_data {
	int count;
	int count2;
};

struct harpoon_data {
	struct game_obj_t *gdb;
};

struct octopus_data {
	int awake;
	int tx, ty;
	struct game_obj_t *tentacle[8];
};

struct gdb_data {
	int awake;
	int tx, ty;
	// struct game_obj_t *tentacle[8];
};

struct tentacle_seg_data {
	int angle;
	int length;
	int angular_v;
	int dest_angle;
};

struct tentacle_data {
	struct game_obj_t *attached_to;
	int angle;
	int nsegs;
	int upper_angle, lower_angle;
	struct tentacle_seg_data *seg;
};

struct floating_message_data {
	int font;
	char msg[21];
};

union type_specific_data {
	struct harpoon_data harpoon;
	struct gdb_data gdb;
	struct octopus_data octopus;
	struct extra_player_data epd;
	struct tentacle_data tentacle;
	struct floating_message_data floating_message;
};

struct game_obj_t {
	obj_move_func *move;
	obj_draw_func *draw;
	obj_destroy_func *destroy;
	struct my_vect_obj *v;
	struct target_t *target;
	int x, y;
	int vx, vy;
	int color;
	int alive;
	int otype;
	struct game_obj_t *bullseye;
	int last_xi;
	int counter;
	union type_specific_data tsd;
	int missile_timer;
};

GtkWidget *score_label;
GtkWidget *bombs_label;

struct target_t {
	struct game_obj_t *o;
	struct target_t *prev, *next;
} *target_head = NULL;

struct terrain_t {
	int npoints;
	int x[TERRAIN_LENGTH];
	int y[TERRAIN_LENGTH];
} terrain;

struct game_state_t {
	int x;
	int y;
	int last_x1, last_x2;
	int vx;
	int vy;
	int lives;
	int nobjs;
	int direction;
	int health;
	int score;
	int prev_score;
	int nbombs;
	int prev_bombs;
	int humanoids;
	int gdbs_killed;
	int guns_killed;
	int sams_killed;
	int missiles_killed;
	int octos_killed;
	int emacs_killed;
	int rockets_killed;
	int missile_locked;
	struct timeval start_time, finish_time;
	struct game_obj_t go[MAXOBJS];
	int cmd_multiplier;
} game_state = { 0, 0, 0, 0, PLAYER_SPEED, 0, 0 };

struct game_obj_t *player = &game_state.go[0];

GdkGC *gc = NULL;
GtkWidget *main_da;
gint timer_tag;

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

void print_target_list()
{
	struct target_t *t;
	printf("Targetlist:\n");
	for (t=target_head; t != NULL;t=t->next) {
		printf("%c: %d,%d\n", t->o->otype, t->o->x, t->o->y);
	}
	printf("end of list.\n");
}
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

int randomn(int n)
{
	return (int) (((random() + 0.0) / (RAND_MAX + 0.0)) * (n + 0.0));
}

int randomab(int a, int b)
{
	int x, y;
	if (a > b) {
		x = b;
		y = a;
	} else {
		x = a;
		y = b;
	}
	return (int) (((random() + 0.0) / (RAND_MAX + 0.0)) * (y - x + 0.0)) + x;
}

void explode(int x, int y, int ivx, int ivy, int v, int nsparks, int time);

void move_laserbolt(struct game_obj_t *o)
{
	int dy;
	if (!o->alive)
		return;
	dy = (o->y - player->y);
	if (dy < -1000) {
		o->alive = 0;
		o->destroy(o);
		return;
	}
	if (abs(dy) < 9 && abs(player->x - o->x) < 15) {
		explode(o->x, o->y, o->vx, 1, 70, 20, 20);
		add_sound(LASER_EXPLOSION_SOUND, ANY_SLOT);
		game_state.health -= LASER_BOLT_DAMAGE;
		o->alive = 0;	
		o->destroy(o);
	}
	o->x += o->vx;
	o->y += o->vy;
	o->alive--;
}

static void add_laserbolt(int x, int y, int vx, int vy, int time);
void move_flak(struct game_obj_t *o)
{
	int xdist;
	int dx, dy, bx,by;
	int x1, y1;
	xdist = abs(o->x - player->x);
	if (xdist < SCREEN_WIDTH && randomn(1000) < level.laser_fire_chance) {
		dx = player->x+LASERLEAD*player->vx - o->x;
		dy = player->y+LASERLEAD*player->vy - o->y;

		add_sound(FLAK_FIRE_SOUND, ANY_SLOT);
		if (dy >= 0) {
			if (player->x+player->vx*LASERLEAD < o->x)
				bx = -20;
			else
				bx = 20;
			by = 0;
		} else if (dx == 0) {
			bx = -0;
			by = -20;
		} else if (abs(dx) > abs(dy)) {
			if (player->x+player->vx*LASERLEAD < o->x)
				bx = -20;
			else
				bx = 20;
			by = -abs((20*dy)/dx);
		} else {
			by = -20;
			/* if (player->x < o->x)
				bx = -20;
			else
				bx = 20; */
			bx = (-20*dx)/dy;
		}
		x1 = o->x-5;
		y1 = o->y-5;  
		add_laserbolt(x1, y1, bx, by, 50);
		add_laserbolt(x1+10, y1, bx, by, 50);
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

	xdist = abs(o->x - player->x);
	if (xdist < LAUNCH_DIST) {
		ydist = o->y - player->y;
		if ((xdist <= ydist && ydist > 0) || o->vy != 0) {
			if (o->vy == 0)
				add_sound(ROCKET_LAUNCH_SOUND, ANY_SLOT);
			if (o->vy > MAX_ROCKET_SPEED)
				o->vy--;
		}

		if ((ydist*ydist + xdist*xdist) < 400) {
			add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
			explode(o->x, o->y, o->vx, 1, 70, 150, 20);
			o->alive = 0;
			game_state.health -= 20;
			remove_target(o->target);
			return;
		}
	}
	o->x += o->vx;
	o->y += o->vy;
	if (o->vy != 0)
		explode(o->x, o->y, 0, 9, 8, 7, 13);
	if (o->y - player->y < -1000 && o->vy != 0) {
		o->alive = 0;
		remove_target(o->target);
		o->destroy(o);
	}
}

void sam_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	xdist = abs(o->x - player->x);
	if (xdist < SAM_LAUNCH_DIST) {
		ydist = o->y - player->y;
		if (ydist > 0 && randomn(1000) < SAM_LAUNCH_CHANCE && timer >= o->missile_timer) {
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_missile(o->x+20, o->y-30, 0, 0, 300, GREEN, player);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
		}
	}
}

void draw_generic(struct game_obj_t *o, GtkWidget *w);
void gdb_draw(struct game_obj_t *o, GtkWidget *w)
{
	if (o->vx < 0)
		o->v = &gdb_vect_left;
	else if (o->vx > 0)
		o->v = &gdb_vect_right;
	draw_generic(o, w);
}

void draw_lightning( GtkWidget *w, int x1, int y1, int x2, int y2) 
{
	int x3, y3, dx, dy;

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	if (dx < 10 && dy < 10) {
		gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
		return;
	}

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

	x3 += (int) (0.2 * (randomn(2*dx) - dx)); 
	y3 += (int) (0.2 * (randomn(2*dy) - dy)); 

	draw_lightning(w, x1, y1, x3, y3);	
	draw_lightning(w, x3, y3, x2, y2);	
}

static void xy_draw_string(GtkWidget *w, unsigned char *s, int font, int x, int y) ;
void floating_message_draw(struct game_obj_t *o, GtkWidget *w)
{
	gdk_gc_set_foreground(gc, &huex[o->color]);
	xy_draw_string(w, o->tsd.floating_message.msg, o->tsd.floating_message.font, o->x, o->y) ;
}

void tentacle_draw(struct game_obj_t *o, GtkWidget *w)
{
	int i;
	int x1, y1, x2, y2;
	int angle = 0;
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
		gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
		x1 = x2;
		y1 = y2;
	}
	if (randomn(1000) < 20 && abs(o->x - player->x) < 100 && abs(o->y - player->y) < 300) {
		gdk_gc_set_foreground(gc, &huex[WHITE]);
		draw_lightning(w, x2, y2, player->x - game_state.x, player->y - game_state.y + (SCREEN_HEIGHT/2));
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		add_sound(THUNDER_SOUND, ANY_SLOT);
	}
}

int find_ground_level(struct game_obj_t *o);

void tentacle_move(struct game_obj_t *o) 
{

	struct tentacle_seg_data *t;
	int i, gy;

	if (abs(o->x - game_state.x) > SCREEN_WIDTH*1.5)
		return;

	if (o->tsd.tentacle.attached_to == NULL) {	
		o->x += o->vx;
		o->y += o->vy;
		o->vy += 1;

		gy = find_ground_level(o);

		if (o->y >= gy) {
			o->vy = 0;
			o->vx = 0;
			o->y = gy;
		}

		if (o->x < 0) {
			o->x = 0;
			o->vx = 20;
		}
		if (o->x > terrain.x[TERRAIN_LENGTH-1]) {
			o->x = terrain.x[TERRAIN_LENGTH-1];
			o->vx = -20;
		}
	} else {
		o->x = o->tsd.tentacle.attached_to->x;
		o->y = o->tsd.tentacle.attached_to->y;
	}

	for (i=0;i<o->tsd.tentacle.nsegs;i++) {
		int da;
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
		t->angle += t->angular_v;
		if (i != 0 && t->angle > MAX_SEG_ANGLE)
			t->angle = MAX_SEG_ANGLE;
		else if (i != 0 && t->angle < -MAX_SEG_ANGLE)
			t->angle = -MAX_SEG_ANGLE;
	}

	if (randomn(1000) < 50) {

		o->tsd.tentacle.angle = TENTACLE_RANGE(o->tsd.tentacle);
		if (o->tsd.tentacle.angle < 0)
			o->tsd.tentacle.angle += 360; 
		if (o->tsd.tentacle.angle > 360)
			o->tsd.tentacle.angle -= 360; 

		switch (randomn(11)) {

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
				for (i=1;i<o->tsd.tentacle.nsegs;i++) {
					t = &o->tsd.tentacle.seg[i];
					t->dest_angle = 0;
				}
				o->vx = cosine[o->tsd.tentacle.angle] * 20;
				o->vy = -sine[o->tsd.tentacle.angle] * 20;
			}
			break;
		case 5:
			for (i=0;i<o->tsd.tentacle.nsegs;i++) {
				t = &o->tsd.tentacle.seg[i];
				t->angular_v = -1;
			}
			break;
		case 9:
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

	gy = find_ground_level(o);
	

	if (o->tsd.octopus.awake) {
		tx = o->tsd.octopus.tx;
		ty = o->tsd.octopus.ty;
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

		if (abs(player->x - tx) > GDB_DX_THRESHOLD ||
			abs(player->y - ty) > GDB_DY_THRESHOLD || randomn(100) < 3) {
			o->tsd.octopus.tx = player->x + randomn(300)-150;
			o->tsd.octopus.ty = player->y + randomn(300)-150;
		}

		if (o->y > gy - 100 && dvy > -3)
			dvy = -10;	

		if (o->vx < dvx)
			o->vx++;
		else if (o->vx > dvx)
			o->vx--;
		if (o->vy < dvy)
			o->vy++;
		else if (o->vy > dvy)
			o->vy--;

		o->x += o->vx;
		o->y += o->vy;

		
		explode(o->x - dvx + randomn(4)-2, o->y - dvy + randomn(4)-2, -dvx, -dvy, 4, 8, 9);
	}
	
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

	for (i=0;i<8;i++) {
		if (o->tsd.octopus.tentacle[i]) {
			o->tsd.octopus.tentacle[i]->x = o->x;
			o->tsd.octopus.tentacle[i]->y = o->y;
		}
	}

	if (o->y >= gy + 3) {
		o->alive = 0;
		explode(o->x, o->y, o->vx, 1, 70, 150, 20);
		o->destroy(o);
	}
}

void gdb_move(struct game_obj_t *o)
{
	int xdist, ydist;
	int dvx, dvy, tx, ty;
	int gy;

	if (!o->alive)
		return;

	gy = find_ground_level(o);
	

	if (o->tsd.gdb.awake) {
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

		if (abs(player->x - tx) > GDB_DX_THRESHOLD ||
			abs(player->y - ty) > GDB_DY_THRESHOLD || randomn(100) < 3) {
			o->tsd.gdb.tx = player->x + randomn(300)-150;
			o->tsd.gdb.ty = player->y + randomn(300)-150;
		}

		if (o->y > gy - 100 && dvy > -3)
			dvy = -10;	

		if (o->vx < dvx)
			o->vx++;
		else if (o->vx > dvx)
			o->vx--;
		if (o->vy < dvy)
			o->vy++;
		else if (o->vy > dvy)
			o->vy -= 2;

		o->x += o->vx;
		o->y += o->vy;

		o->vy++; /* gravity */

		
		explode(o->x - dvx + randomn(4)-2, o->y - dvy + randomn(4)-2, -dvx, -dvy, 4, 8, 9);
	}
	
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

	if (o->y >= gy + 3) {
		o->alive = 0;
		explode(o->x, o->y, o->vx, 1, 70, 150, 20);
		o->destroy(o);
	}
}

void humanoid_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	xdist = abs(o->x - player->x);
	ydist = abs(o->y - player->y);
	if (xdist < HUMANOID_DIST) {
		if (ydist < HUMANOID_DIST) {
			add_sound(CARDOOR_SOUND, ANY_SLOT);
			add_sound(WOOHOO_SOUND, ANY_SLOT);
			o->x = -1000; /* take him off screen. */
			o->y = -1000;
			game_state.score += HUMANOID_PICKUP_SCORE;
			game_state.humanoids++;
		} else {
			if (o->counter == 0) {
				add_sound(HELPDOWNHERE_SOUND, ANY_SLOT);
				o->counter = 1;
			}
		}
	} else if (xdist > 1000) 
		o->counter = 0;
}

void advance_level();
void socket_move(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

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
		timer_event = START_INTERMISSION_EVENT;
		next_timer = timer + 1;
		// advance_level();
	}
}

int find_free_obj();

void laser_move(struct game_obj_t *o);
void generic_destroy_func(struct game_obj_t *o);

void player_fire_laser()
{
	int i;
	struct game_obj_t *o, *p;
	int j;
	int y;

	p = &game_state.go[0];
	y = p->y - ((game_state.cmd_multiplier-1)/2) * 10;
	for (j=0;j<game_state.cmd_multiplier;j++) {
	i = find_free_obj();
	o = &game_state.go[i];

	if (p != player) {
		printf("p != player!\n");
	} 

	o->last_xi = -1;
	o->x = p->x+(30 * game_state.direction);
	o->y = y;
	y += 10;
	o->vx = p->vx + LASER_SPEED * game_state.direction;
	o->vy = 0;
	o->v = &right_laser_vect;
	o->draw = NULL;
	o->move = laser_move;
	o->destroy = generic_destroy_func;
	o->otype = OBJ_TYPE_LASER;
	o->color = GREEN;
	o->alive = 20;
	o->target = NULL;
	}
	game_state.cmd_multiplier = 1;
	add_sound(PLAYER_LASER_SOUND, ANY_SLOT);
}

int interpolate(int x, int x1, int y1, int x2, int y2)
{
	/* return corresponding y on line x1,y1,x2,y2 for value x */
	/*
		(y2 -y1)/(x2 - x1) = (y - y1) / (x - x1)
		(x -x1) * (y2 -y1)/(x2 -x1) = y - y1
		y = (x - x1) * (y2 - y1) / (x2 -x1) + y1;
	*/
	if (x2 == x1)
		return y1;
	else
		return (x - x1) * (y2 - y1) / (x2 -x1) + y1;
}

#define GROUND_OOPS 64000
int ground_level(int x, int *xi)
{
	/* Find the level of the ground at position x */
	int deepest, i;

	*xi = -1;
	/* Detect smashing into the ground */
	deepest = GROUND_OOPS;
	for (i=0;i<TERRAIN_LENGTH-1;i++) {
		if (x >= terrain.x[i] && x < terrain.x[i+1]) {
			*xi = i;
			deepest = interpolate(x, terrain.x[i], terrain.y[i],
					terrain.x[i+1], terrain.y[i+1]);
			break;
		}
	}
	return deepest;
}

int find_ground_level(struct game_obj_t *o)
{
	int xi1, xi2, i;
	xi1 = o->last_xi;
	xi2 = xi1 + 1;

	if (xi1 < 0 || xi2 >= TERRAIN_LENGTH)
		return ground_level(o->x, &o->last_xi);

	if (terrain.x[xi1] <= o->x && terrain.x[xi2] >= o->x)
		return interpolate(o->x, terrain.x[xi1], terrain.y[xi1],
				terrain.x[xi2], terrain.y[xi2]);

	if (terrain.x[xi1] > o->x) {
		for (i=xi1;i>=0;i--) {
			if (o->x >= terrain.x[i] && o->x < terrain.x[i+1]) {
				o->last_xi = i;
				return interpolate(o->x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	} else if (terrain.x[xi2] < o->x) {
		for (i=xi1;i<=TERRAIN_LENGTH-10;i++) {
			if (o->x >= terrain.x[i] && o->x < terrain.x[i+1]) {
				o->last_xi = i;
				return interpolate(o->x, terrain.x[i], terrain.y[i],
						terrain.x[i+1], terrain.y[i+1]);
			}
		}
	}
	o->last_xi = GROUND_OOPS;
	return GROUND_OOPS;
	
}

void generic_destroy_func(struct game_obj_t *o)
{
	return;
}

void octopus_destroy(struct game_obj_t *o)
{
	int i;
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


void bridge_move(struct game_obj_t *o);
void no_move(struct game_obj_t *o);
static void add_score_floater(int x, int y, int score);

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
	o->vy++;

	for (t=target_head;t != NULL;) {
		if (!t->o->alive) {
			t=t->next;
			continue;
		}
		removed = 0;
		switch (t->o->otype) {
			case OBJ_TYPE_ROCKET:
			case OBJ_TYPE_MISSILE:
			case OBJ_TYPE_HARPOON:
			case OBJ_TYPE_GDB:
			case OBJ_TYPE_OCTOPUS:
			case OBJ_TYPE_FUEL:
			case OBJ_TYPE_GUN:
			/* case OBJ_TYPE_BOMB:  no, bomb can't bomb himself... */
			case OBJ_TYPE_SAM_STATION:  {
				dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
					(o->y - t->o->y)*(o->y - t->o->y);
				if (dist2 < LASER_PROXIMITY) { /* a hit */
					if (t->o->otype == OBJ_TYPE_ROCKET) {
						game_state.score += ROCKET_SCORE;
						add_score_floater(t->o->x, t->o->y, ROCKET_SCORE);
					} else if (t->o->otype == OBJ_TYPE_SAM_STATION) { 
						game_state.score += SAM_SCORE;
						add_score_floater(t->o->x, t->o->y, SAM_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GDB) { 
						game_state.score += GDB_SCORE;
						add_score_floater(t->o->x, t->o->y, GDB_SCORE);
					} else if (t->o->otype == OBJ_TYPE_GUN) { 
						game_state.score += FLAK_SCORE;
						add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
					} else if (t->o->otype == OBJ_TYPE_OCTOPUS) { 
						game_state.score += OCTOPUS_SCORE;
						add_score_floater(t->o->x, t->o->y, OCTOPUS_SCORE);
					}
					add_sound(BOMB_IMPACT_SOUND, ANY_SLOT);
					explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
					t->o->alive = 0;
					t->o->destroy(t->o);
					t = remove_target(t);
					removed = 1;
					o->alive = 0;
					o->destroy(o);
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
					else if (t->o->otype == OBJ_TYPE_GUN)
						game_state.guns_killed++;
				}
			}
			default:
				break;
		}
		if (!removed)
			t=t->next;
	}

	/* Detect smashing into the ground */
	deepest = find_ground_level(o);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		o->alive = 0;
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
				case OBJ_TYPE_TENTACLE: /* Doesn't hurt them, just moves them */
					if (abs(o->x - t->o->x) < BOMB_X_PROXIMITY) { /* a hit */
						t->o->vx = ((t->o->x < o->x) ? -1 : 1) * randomn(16);
						t->o->vy = ((t->o->y < o->y) ? -1 : 1) * randomn(16);
					}
					break;
				case OBJ_TYPE_ROCKET:
				case OBJ_TYPE_HARPOON:
				case OBJ_TYPE_MISSILE:
				case OBJ_TYPE_GDB:
				case OBJ_TYPE_OCTOPUS:
				case OBJ_TYPE_GUN:
				case OBJ_TYPE_BOMB:
				case OBJ_TYPE_SAM_STATION:
				case OBJ_TYPE_FUEL: {
					dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
						(o->y - t->o->y)*(o->y - t->o->y);
					if (dist2 < BOMB_PROXIMITY) { /* a hit */
						if (t->o->otype == OBJ_TYPE_ROCKET) {
							game_state.score += ROCKET_SCORE;
							add_score_floater(t->o->x, t->o->y, ROCKET_SCORE);
						} else if (t->o->otype == OBJ_TYPE_SAM_STATION) { 
							game_state.score += SAM_SCORE;
							add_score_floater(t->o->x, t->o->y, SAM_SCORE);
						} else if (t->o->otype == OBJ_TYPE_GDB) { 
							game_state.score += GDB_SCORE;
							add_score_floater(t->o->x, t->o->y, GDB_SCORE);
						} else if (t->o->otype == OBJ_TYPE_GUN) { 
							game_state.score += FLAK_SCORE;
							add_score_floater(t->o->x, t->o->y, FLAK_SCORE);
						} else if (t->o->otype == OBJ_TYPE_OCTOPUS) { 
							game_state.score += OCTOPUS_SCORE;
							add_score_floater(t->o->x, t->o->y, OCTOPUS_SCORE);
						}
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
						t->o->alive = 0;
						t->o->destroy(t->o);
						if (t->o->otype == OBJ_TYPE_FUEL) {
							game_state.health += 10;
							if (game_state.health > MAXHEALTH)
								game_state.health = MAXHEALTH;
						}
						t = remove_target(t);
						removed = 1;
					}
				}
				break;

				case OBJ_TYPE_BRIDGE:
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
			if (!removed)
				t = t->next;
		}
	}
	if (!o->alive) {
		remove_target(o->target);
		o->target = NULL;
	}
}

void chaff_move(struct game_obj_t *o)
{
	int deepest;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;
	o->vy++;
	o->alive--;

	if (o->vx > 0)
		o->vx--;
	else if (o->vx < 0);
		o->vx++;

	explode(o->x, o->y, 0, 0, 10, 7, 19);
	/* Detect smashing into the ground */
	deepest = find_ground_level(o);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		o->alive = 0;
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

	for (j=0;j<3;j++) {
		i[j] = find_free_obj();
		if (i[j] < 0)
			continue;
		o = &game_state.go[i[j]];
		o->last_xi = -1;
		o->x = player->x;
		o->y = player->y;
		o->vx = player->vx + ((j-1) * 7);
		o->vy = player->vy + 7;
		o->v = &spark_vect;
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
		if (t->o->bullseye == player) {
			j = randomn(3);
			if (j >= 0 && j <= 3 && i[j] > 0 && randomn(100) < 50)
				t->o->bullseye = &game_state.go[i[j]];
		}
	}
	/* Bug: when (bullseye->alive == 0) some new object will allocate there
	   and become the new target... probably a spark. */
}

void drop_bomb()
{
	int i, j;
	struct game_obj_t *o;

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
		o->target = add_target(o);
	}
	game_state.cmd_multiplier = 1;
}

void player_draw(struct game_obj_t *o, GtkWidget *w)
{

	int i, countdir;
	double scale, scalefactor;

	if (player->tsd.epd.count == 0)
		draw_generic(o, w);
	else {
		if (player->tsd.epd.count > 0) {
			scale = 1.07;
			scalefactor = 1.07;
			countdir = 1;
		} else {
			scale = 1.07;
			scalefactor = 1.07;
			countdir = -1;
		}

		for (i = 0; i<o->tsd.epd.count2; i++) {
			int j;
			int x1, y1, x2, y2;
			gdk_gc_set_foreground(gc, &huex[o->color]);
			for (j=0;j<o->v->npoints-1;j++) {
				if (o->v->p[j+1].x == LINE_BREAK) /* Break in the line segments. */
					j+=2;
				x1 = o->x + o->v->p[j].x*scale - game_state.x;
				y1 = o->y + o->v->p[j].y*scale - game_state.y + (SCREEN_HEIGHT/2);  
				x2 = o->x + o->v->p[j+1].x*scale - game_state.x; 
				y2 = o->y + o->v->p[j+1].y*scale+(SCREEN_HEIGHT/2) - game_state.y;
				gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
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

static void add_debris(int x, int y, int vx, int vy, int r, struct game_obj_t **victim);
void no_draw(struct game_obj_t *o, GtkWidget *w);
void move_player(struct game_obj_t *o)
{
	int i;
	int deepest;
	static int was_healthy = 1;
	o->x += o->vx;
	o->y += o->vy;

	if (game_state.health > 0) {
		if (credits > 0 && o->vy > 0) /* damp y velocity. */
			o->vy--;
		if (credits > 0 && o->vy < 0)
			o->vy++;
		was_healthy = 1;
	} else if (was_healthy) {
		was_healthy = 0;
		player->move = bridge_move;
		explode(player->x, player->y, player->vx, player->vy, 90, 350, 30);
		player->draw = no_draw;
		add_debris(o->x, o->y, o->vx, o->vy, 20, &player);
		add_sound(LARGE_EXPLOSION_SOUND, ANY_SLOT);
		printf("decrementing lives %d.\n", game_state.lives);
		game_state.lives--;
		sprintf(textline[CREDITS].string, "Credits: %d Lives: %d", 
			credits, game_state.lives);
		if (game_state.lives <= 0 || credits <= 0) {
			if (credits > 0) 
				credits--;
			if (credits <= 0) {
				timer_event = GAME_ENDED_EVENT;
				next_timer = timer + 30;
			} else {
				timer_event = READY_EVENT;
				game_state.lives = 3;
				next_timer = timer + 30;
			}
		} else {
			next_timer = timer + 30;
			timer_event = READY_EVENT;
		}
	} 
	if (abs(o->vx) < 5 || game_state.health <= 0) {
		o->vy+=1;
		if (o->vy > MAX_VY)
			o->vy = MAX_VY;
		if (was_healthy)
			explode(o->x-(11 * game_state.direction), o->y, -(7*game_state.direction), 0, 7, 10, 9);
	} else
		if (was_healthy)
			explode(o->x-(11 * game_state.direction), o->y, -((abs(o->vx)+7)*game_state.direction), 0, 10, 10, 9);
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
	deepest = find_ground_level(player);
	if (deepest != GROUND_OOPS && player->y > deepest) {
		player->y = deepest - 5;
		if (abs(player->vy) > 7) 
			player->vy = -0.65 * abs(player->vy);
		else
			player->vy = 0;
		player->vx = 0.65 * player->vx;
		if (player->vy < -15) {
			player->vy = -15;
		}
		if (abs(player->vx) > 5 || abs(player->vy) > 5) {
			add_sound(GROUND_SMACK_SOUND, ANY_SLOT);
			add_sound(OWMYSPINE_SOUND, ANY_SLOT);
			explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
			game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		}
	}

	/* Detect smashing into sides and roof */
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
			add_sound(GROUND_SMACK_SOUND, ANY_SLOT);
			add_sound(OWMYSPINE_SOUND, ANY_SLOT);
			explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
			game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		}
	}
	if (player->x < 0) {
		add_sound(GROUND_SMACK_SOUND, ANY_SLOT);
		add_sound(OWMYSPINE_SOUND, ANY_SLOT);
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		player->x = 20;
		player->vx = 5;
	} else if (player->x > terrain.x[TERRAIN_LENGTH - 1]) {
		add_sound(GROUND_SMACK_SOUND, ANY_SLOT);
		add_sound(OWMYSPINE_SOUND, ANY_SLOT);
		explode(player->x, player->y, player->vx*1.5, 1, 20, 20, 15);
		game_state.health -= 4 - player->vy * 0.3 -abs(player->vx) * 0.1;
		player->x = terrain.x[TERRAIN_LENGTH - 1] - 20;
		player->vx = -5;
	}

	/* Autopilot, "attract mode", if credits <= 0 */
	if (credits <= 0) {
		for (i=0;i<TERRAIN_LENGTH;i++) {
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
			if (player->vx < PLAYER_SPEED)
				player->vx++; 
		}
		if (randomn(40) < 4)
			player_fire_laser();
		if (randomn(100) < 2)
			drop_bomb();
	}
	/* End attract mode */
}

void bridge_move(struct game_obj_t *o) /* move bridge pieces when hit by bomb */
{
	int i;
	int deepest;
	int slope;
	o->x += o->vx;
	o->y += o->vy;
	o->vy++;
	if (o->alive >1)
		o->alive--;

	/* Detect smashing into the ground */
	deepest = 64000;
	for (i=0;i<TERRAIN_LENGTH-1;i++) {
		if (o->x >= terrain.x[i] && o->x < terrain.x[i+1]) {
			deepest = interpolate(o->x, terrain.x[i], terrain.y[i],
					terrain.x[i+1], terrain.y[i+1]);
			slope = (100*(terrain.y[i+1] - terrain.y[i])) / 
					(terrain.x[i+1] - terrain.x[i]);
			break;
		}
	}
	if (deepest != 64000 && o->y > deepest) {
		o->y = deepest-2;
		o->vx = 0;
		o->vy = 0;
		if (slope > 25 && o->alive > 1) {
			o->vx = 3;
			o->vy += 1;
		} else if (slope < -25 && o->alive > 1) {
			o->vx = -3;
			o->vy += 1;
		}
		if (o->alive == 1) 
			o->move = NULL;
	}
}

void no_move(struct game_obj_t *o)
{
	return;
}

void laser_move(struct game_obj_t *o)
{
	struct target_t *t;
	int dist2;
	int removed;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;

	for (t=target_head;t != NULL;) {
		if (!t->o->alive) {
			t = t->next;
			continue;
		}
		removed = 0;
		switch (t->o->otype) {
			case OBJ_TYPE_SHIP:
			case OBJ_TYPE_AIRSHIP:
				if (abs(o->x - t->o->x) < 3*60 &&
					o->y - t->o->y <= 0 &&
					o->y - t->o->y > -50*3) {
					explode(o->x, o->y, o->vx/2, 1, 70, 20, 20);
					o->alive = 0;
					o->destroy(o);
					t->o->alive -= PLAYER_LASER_DAMAGE;
					if (t->o->alive <= 0) {
						game_state.emacs_killed++;
						t->o->alive = 0;
						t->o->destroy(t->o);
						explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
						t = remove_target(t);
						removed = 1;
					}
				}
				break;
			case OBJ_TYPE_ROCKET:
			case OBJ_TYPE_HARPOON:
			case OBJ_TYPE_GDB:
			case OBJ_TYPE_OCTOPUS:
			case OBJ_TYPE_FUEL:
			case OBJ_TYPE_GUN:
			case OBJ_TYPE_BOMB:
			case OBJ_TYPE_SAM_STATION:
			case OBJ_TYPE_MISSILE:{
				dist2 = (o->x - t->o->x)*(o->x - t->o->x) + 
					(o->y - t->o->y)*(o->y - t->o->y);
				// printf("dist2 = %d\n", dist2);
				if (dist2 < LASER_PROXIMITY) { /* a hit */
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
					t->o->alive = 0;
					if (t->o->otype == OBJ_TYPE_FUEL) {
						game_state.health += 10;
						if (game_state.health > MAXHEALTH)
							game_state.health = MAXHEALTH;
					}
					t->o->destroy(t->o);
					t = remove_target(t);
					removed = 1;
					o->alive = 0;
					o->destroy(o);
				}
			}
			break;
			default:
				break;
		}
		if (!removed)
			t=t->next;
	}
	if (o->alive)
		o->alive--;
	// if (!o->alive) {
		//remove_target(o->target);
		//o->target = NULL;
	//}
}

void move_obj(struct game_obj_t *o)
{
	o->x += o->vx;
	o->y += o->vy;
}

void draw_spark(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2;

	gdk_gc_set_foreground(gc, &huex[o->color]);
	x1 = o->x - o->vx - game_state.x;
	y1 = o->y - o->vy - game_state.y + (SCREEN_HEIGHT/2);  
	x2 = o->x - game_state.x; 
	y2 = o->y + (SCREEN_HEIGHT/2) - game_state.y;
	gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
}

void draw_missile(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1;
	int dx, dy;

	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  
	dx = dx_from_vxy(o->vx, o->vy);
	dy = -dy_from_vxy(o->vx, o->vy);
	gdk_gc_set_foreground(gc, &huex[o->color]);
	gdk_draw_line(w->window, gc, x1, y1, x1+dx*2, y1+dy*2); 
}

void draw_harpoon(struct game_obj_t *o, GtkWidget *w)
{
	int x1, y1, x2, y2;
	int dx, dy;

	x1 = o->x - game_state.x;
	y1 = o->y - game_state.y + (SCREEN_HEIGHT/2);  
	x2 = o->tsd.harpoon.gdb->x - game_state.x;
	y2 = o->tsd.harpoon.gdb->y - game_state.y + (SCREEN_HEIGHT/2);  
	dx = dx_from_vxy(o->vx, o->vy);
	dy = -dy_from_vxy(o->vx, o->vy);
	gdk_gc_set_foreground(gc, &huex[o->color]);
	gdk_draw_line(w->window, gc, x1, y1, x1+dx*2, y1+dy*2); 
	if (o->tsd.harpoon.gdb->alive && o->tsd.harpoon.gdb->otype == OBJ_TYPE_GDB) {
		x2 = o->tsd.harpoon.gdb->x - game_state.x;
		y2 = o->tsd.harpoon.gdb->y - game_state.y + (SCREEN_HEIGHT/2);  
		gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
	}
}

void move_missile(struct game_obj_t *o)
{
	struct game_obj_t *target_obj;
	int dx, dy, desired_vx, desired_vy;
	int exvx,exvy,deepest;

	/* move one step... */
	o->x += o->vx;
	o->y += o->vy;
	
	deepest = find_ground_level(o);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		o->alive = 0;
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	o->alive--;
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

	/* Figure out where we're trying to go */
	target_obj = o->bullseye;
	if (target_obj == player) {
		game_state.missile_locked = 1;
		/* printf("mlock1\n"); */
	}
	dx = target_obj->x + target_obj->vx - o->x;
	dy = target_obj->y + target_obj->vy - o->y;

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
		o->alive = 0;
		target_obj->alive -= MISSILE_DAMAGE;
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

	/* Try to get to desired vx,vy, but, only once every other clock tick */
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

	/* move one step... */
	o->x += o->vx;
	o->y += o->vy;
	
	deepest = find_ground_level(o);
	if (deepest != GROUND_OOPS && o->y > deepest) {
		add_sound(ROCKET_EXPLOSION_SOUND, ANY_SLOT);
		explode(o->x, o->y, o->vx, o->vy, 70, 150, 20);
		o->alive = 0;
		if (o->target) {
			remove_target(o->target);
			o->target = NULL;
		}
		o->destroy(o);
		return;
	}

	o->alive--;
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

	/* Figure out where we're trying to go */
	target_obj = o->bullseye;
	if (target_obj == player) {
		game_state.missile_locked = 1;
		/* printf("mlock2\n"); */
	}
	dx = target_obj->x + target_obj->vx - o->x;
	dy = target_obj->y + target_obj->vy - o->y;

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
		o->alive = 0;
		target_obj->alive -= MISSILE_DAMAGE;
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

	/* Try to get to desired vx,vy, but, only once every other clock tick */
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

static struct game_obj_t *add_generic_object(int x, int y, int vx, int vy,
	obj_move_func *move_func,
	obj_draw_func *draw_func,
	int color, 
	struct my_vect_obj *vect, 
	int target, char otype, int alive)
{
	int j;
	struct game_obj_t *o;

	j = find_free_obj();
	if (j < 0)
		return NULL;
	o = &game_state.go[j];
	o->last_xi = -1;
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
	return o;
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

	deepest = find_ground_level(o);
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
	o->alive--;
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
		o->alive--;
		// if (!o->alive)
		//	remove_target(o->target);
	} else
		o->destroy(o);
}

static void flying_thing_shoot_missile(struct game_obj_t *o)
{
	int gambling, xdist, ydist;

	xdist = abs(o->x - player->x);
	ydist = abs(o->y - player->y);
	if (xdist < SAM_LAUNCH_DIST && ydist < SAM_LAUNCH_DIST) {
		if (xdist < SAM_LAUNCH_DIST/2)
			gambling = 2;
		if (xdist < SAM_LAUNCH_DIST/3)
			gambling = 4;
		
		if (randomn(2000) < (SAM_LAUNCH_CHANCE+gambling) && timer >= o->missile_timer) {
			add_sound(SAM_LAUNCH_SOUND, ANY_SLOT);
			add_missile(o->x, o->y, 0, 0, 300, RED, player);
			o->missile_timer = timer + MISSILE_FIRE_PERIOD;
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

	deepest = find_ground_level(o);
	if (deepest != GROUND_OOPS && o->y > deepest - MIN_BALLOON_HEIGHT)
		o->vy = -1;
	else if (deepest != GROUND_OOPS && o->y < deepest - MAX_BALLOON_HEIGHT)
		o->vy = 1;
}

void airship_leak_lisp(struct game_obj_t *o)
{
	/* The airships have "memory leaks" in which they spew "lisp code" */
	/* out into core memory... */
	if ((timer % 8) && randomn(100) < 15) {
		if (randomlisp[o->counter] != ' ') /* skip putting out "space" objects. */
			add_symbol(randomlisp[o->counter], TINY_FONT, o->x + 70*3, o->y - 30*3, o->vx+4, 0, 200);
		o->counter--;
		if (o->counter < 0)
			o->counter = strlen(randomlisp)-1;
	}
}

void airship_move(struct game_obj_t *o)
{
	if (!o->alive)
		return;

	flying_thing_move(o);
	flying_thing_shoot_missile(o);
	airship_leak_lisp(o);
}

void ship_move(struct game_obj_t *o)
{
	if (!o->alive)
		return;

	flying_thing_move(o);
	/* flying_thing_shoot_missile(o); */
}

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
	o->alive--;
	// printf("x=%d,y=%d,vx=%d,vy=%d, alive=%d\n", o->x, o->y, o->vx, o->vy, o->alive);

	if (o->alive >= NSPARKCOLORS)
		o->color = NCOLORS + NSPARKCOLORS - 1; 
	else if (o->alive < 0) 
		o->color = NCOLORS;
	else
		o->color = NCOLORS + o->alive;

	if (o->vx > 0)
		o->vx--;
	else if (o->vx < 0)
		o->vx++;
	if (o->vy > 0)
		o->vy--;
	else if (o->vy < 0)
		o->vy++;

	if (o->vx == 0 && o->vy == 0) {
		o->alive = 0;
		o->draw = NULL;
	}
	if (abs(o->y - player->y) > 2000 || o->x > 2000+WORLDWIDTH || o->x < -2000) {
		o->alive = 0;
		o->draw = NULL;
	}
}

static void add_spark(int x, int y, int vx, int vy, int time);

void explode(int x, int y, int ivx, int ivy, int v, int nsparks, int time)
{
	int vx, vy, i;

	for (i=0;i<nsparks;i++) {
		vx = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (v + 0.0) + (0.0 + ivx));
		vy = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (v + 0.0) + (0.0 + ivy));
		add_spark(x, y, vx, vy, time); 
		/* printf("%d,%d, v=%d,%d, time=%d\n", x,y, vx, vy, time); */
	}
}

struct my_vect_obj *prerender_glyph(stroke_t g[], int xscale, int yscale)
{
	int i, x, y;
	int npoints = 0;
	struct my_point_t scratch[100];
	struct my_vect_obj *v;

	printf("Prerendering glyph..\n");

	for (i=0;g[i] != 99;i++) {
		if (g[i] == 21) {
			printf("LINE_BREAK\n");
			x = LINE_BREAK;
			y = LINE_BREAK;
		} else {
			// x = ((g[i] % 3) * xscale);
			// y = ((g[i]/3)-4) * yscale ;     // truncating division.
			x = decode_glyph[g[i]].x * xscale;
			y = decode_glyph[g[i]].y * yscale;
			printf("x=%d, y=%d\n", x,y);
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

void init_vects()
{
	int i;

	for (i=0;i<=360;i++) {
		sine[i] = sin((double)i * 3.1415927 * 2.0 / 360.0);
		cosine[i] = cos((double)i * 3.1415927 * 2.0 / 360.0);
		// printf("%d, %g, %g\n", i, cosine[i], sine[i]);
	}

	/* memset(&game_state.go[0], 0, sizeof(game_state.go[0])*MAXOBJS); */
	player_vect.p = player_ship_points;
	player_vect.npoints = sizeof(player_ship_points) / sizeof(player_ship_points[0]);
	left_player_vect.p = left_player_ship_points;
	left_player_vect.npoints = sizeof(left_player_ship_points) / sizeof(left_player_ship_points[0]);
	for (i=0;i<left_player_vect.npoints;i++)
		left_player_ship_points[i].x *= -1;
	rocket_vect.p = rocket_points;
	rocket_vect.npoints = sizeof(rocket_points) / sizeof(rocket_points[0]);
	spark_vect.p = spark_points;
	spark_vect.npoints = sizeof(spark_points) / sizeof(spark_points[0]);
	right_laser_vect.p = right_laser_beam_points;
	right_laser_vect.npoints = sizeof(right_laser_beam_points) / sizeof(right_laser_beam_points[0]);
	fuel_vect.p = fuel_points;
	fuel_vect.npoints = sizeof(fuel_points) / sizeof(fuel_points[0]);

	ship_vect.p = ships_hull_points;
	ship_vect.npoints = sizeof(ships_hull_points) / sizeof(ships_hull_points[0]);
	for (i=0;i<ship_vect.npoints;i++) {
		if (ship_vect.p[i].x != LINE_BREAK) {
			ship_vect.p[i].x *= 2;
			ship_vect.p[i].y = (ship_vect.p[i].y+20) * 2;
		}
	}
	octopus_vect.p = octopus_points;
	octopus_vect.npoints = sizeof(octopus_points) / sizeof(octopus_points[0]);
	gdb_vect_right.p = gdb_points_right;
	gdb_vect_right.npoints = sizeof(gdb_points_right) / sizeof(gdb_points_right[0]);
	gdb_vect_left.p = gdb_points_left;
	gdb_vect_left.npoints = sizeof(gdb_points_left) / sizeof(gdb_points_left[0]);
	for (i=0;i<gdb_vect_right.npoints;i++)
		if (gdb_vect_right.p[i].x != LINE_BREAK) 
			gdb_vect_right.p[i].x = -gdb_vect_left.p[i].x;
	bomb_vect.p = bomb_points;
	bomb_vect.npoints = sizeof(bomb_points) / sizeof(bomb_points[0]);
	bridge_vect.p = bridge_points;
	bridge_vect.npoints = sizeof(bridge_points) / sizeof(bridge_points[0]);
	flak_vect.p = flak_points;
	flak_vect.npoints = sizeof(flak_points) / sizeof(flak_points[0]);
	airship_vect.p = airship_points;
	airship_vect.npoints = sizeof(airship_points) / sizeof(airship_points[0]);
	for (i=0;i<airship_vect.npoints;i++) {
		if (airship_vect.p[i].x != LINE_BREAK) {
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

	make_font(&gamefont[BIG_FONT], font_scale[BIG_FONT], font_scale[BIG_FONT]);
	make_font(&gamefont[SMALL_FONT], font_scale[SMALL_FONT], font_scale[SMALL_FONT]);
	make_font(&gamefont[TINY_FONT], font_scale[TINY_FONT], font_scale[TINY_FONT]);
	set_font(BIG_FONT);
}

void no_draw(struct game_obj_t *o, GtkWidget *w)
{
	return;
}

void draw_generic(struct game_obj_t *o, GtkWidget *w)
{
	int j;
	int x1, y1, x2, y2;
	gdk_gc_set_foreground(gc, &huex[o->color]);
	for (j=0;j<o->v->npoints-1;j++) {
		if (o->v->p[j+1].x == LINE_BREAK) /* Break in the line segments. */
			j+=2;
		x1 = o->x + o->v->p[j].x - game_state.x;
		y1 = o->y + o->v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
		x2 = o->x + o->v->p[j+1].x - game_state.x; 
		y2 = o->y + o->v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y;
		if (x1 > 0 && x2 > 0)
			gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
	}
}

void draw_flak(struct game_obj_t *o, GtkWidget *w)
{
	int dx, dy, bx,by;
	int x1, y1;
	draw_generic(o, w);

	/* Draw the gun barrels... */
	dx = player->x+LASERLEAD*player->vx - o->x;
	dy = player->y+LASERLEAD*player->vy - o->y;

	if (dy >= 0) {
		if (player->x + LASERLEAD*player->vx < o->x)
			bx = -20;
		else
			bx = 20;
		by = 0;
	} else if (dx == 0) {
		bx = -0;
		by = -20;
	} else if (abs(dx) > abs(dy)) {
		if (player->x+LASERLEAD*player->vx < o->x)
			bx = -20;
		else
			bx = 20;
		by = -abs((20*dy)/dx);
	} else {
		by = -20;
		/* if (player->x < o->x)
			bx = -20;
		else
			bx = 20; */
		bx = (-20*dx)/dy;
	}
	x1 = o->x-5 - game_state.x;
	y1 = o->y-5 - game_state.y + (SCREEN_HEIGHT/2);  
	gdk_draw_line(w->window, gc, x1, y1, x1+bx, y1+by); 
	gdk_draw_line(w->window, gc, x1+10, y1, x1+bx+6, y1+by); 
}

void draw_objs(GtkWidget *w)
{
	int i, j, x1, y1, x2, y2;

	for (i=0;i<MAXOBJS;i++) {
		struct my_vect_obj *v = game_state.go[i].v;
		struct game_obj_t *o = &game_state.go[i];

		if (!o->alive)
			continue;

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
		if (o->draw == NULL && o->v != NULL) {
			gdk_gc_set_foreground(gc, &huex[o->color]);
			for (j=0;j<v->npoints-1;j++) {
				if (v->p[j+1].x == LINE_BREAK) /* Break in the line segments. */
					j+=2;
				x1 = o->x + v->p[j].x - game_state.x;
				y1 = o->y + v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
				x2 = o->x + v->p[j+1].x - game_state.x; 
				y2 = o->y + v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y;
				if (x1 > 0 && x2 > 0)
					gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
			}
		} else if (o->draw != NULL)
			o->draw(o, w);
	}
}

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

	for (i=0;i<font[letter]->npoints-1;i++) {
		if (font[letter]->p[i+1].x == LINE_BREAK)
			i+=2;
		x1 = font[letter]->p[i].x;
		x1 = livecursorx + x1;
		y1 = livecursory + font[letter]->p[i].y;
		x2 = livecursorx + font[letter]->p[i+1].x;
		y2 = livecursory + font[letter]->p[i+1].y;
		gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
		gdk_draw_line(w->window, gc, x1-1, y1+1, x2-1, y2+1); 
	}
	livecursorx += font_scale[current_font]*2 + letter_spacing[current_font];
}

static void draw_string(GtkWidget *w, unsigned char *s) 
{

	unsigned char *i;

	for (i=s;*i;i++)
		draw_letter(w, gamefont[current_font], *i);  
}

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
		draw_string(w, textline[i].string);
	}
}

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
			gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
		// gdk_draw_line(w->window, gc, x1-1, y1+1, x2-1, y2+1); 
	}
}

static void xy_draw_string(GtkWidget *w, unsigned char *s, int font, int x, int y) 
{

	int i;	
	int deltax = font_scale[font]*2 + letter_spacing[font];

	for (i=0;s[i];i++)
		xy_draw_letter(w, gamefont[font], s[i], x + deltax*i, y);  
}


static void add_laserbolt(int x, int y, int vx, int vy, int time)
{
	add_generic_object(x, y, vx, vy, move_laserbolt, draw_spark,
		CYAN, &spark_vect, 0, OBJ_TYPE_SPARK, time);
}

static void add_spark(int x, int y, int vx, int vy, int time)
{
	add_generic_object(x, y, vx, vy, move_spark, draw_spark,
		YELLOW, &spark_vect, 0, OBJ_TYPE_SPARK, time);
}

void perturb(int *value, int lower, int upper, double percent)
{
	double perturbation;

	perturbation = percent * (lower - upper) * ((0.0 + random()) / (0.0 + RAND_MAX) - 0.5);
	*value += perturbation;
	return;
}

static void add_symbol(int c, int myfont, int x, int y, int vx, int vy, int time)
{	
	struct my_vect_obj **font = gamefont[myfont];
	if (font[c] != NULL)
		add_generic_object(x, y, vx, vy, symbol_move, NULL,
			WHITE, font[c], 0, OBJ_TYPE_SYMBOL, time);
}

static void add_floating_message(char *msg, int font, int x, int y, int vx, int vy, int time)
{
	struct game_obj_t *o;
	o = add_generic_object(x, y, vx, vy, floating_message_move, 
		floating_message_draw, WHITE, NULL, 0,
		OBJ_TYPE_FLOATING_MESSAGE, time);
	if (o == NULL)
		return;
	strncpy(o->tsd.floating_message.msg, msg, 20);
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

void generate_sub_terrain(struct terrain_t *t, int xi1, int xi2)
{
	int midxi;
	int y1, y2, y3, tmp;
	int x1, x2, x3;


	midxi = (xi2 - xi1) / 2 + xi1;
	if (midxi == xi2 || midxi == xi1)
		return;

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

	if ((x2 - x1) > 1000) {	
		perturb(&x3, x2, x1, level.large_scale_roughness);
		do { 
			perturb(&y3, x2, x1, level.large_scale_roughness);
		} while ((KERNEL_Y_BOUNDARY - y3) > -150);
	} else {
		perturb(&x3, x2, x1, level.small_scale_roughness);
		do { 
			perturb(&y3, x2, x1, level.small_scale_roughness);
		} while ((KERNEL_Y_BOUNDARY - y3) > -150);
	}

	t->x[midxi] = x3;
	t->y[midxi] = y3;
	// printf("gst %d %d\n", x3, y3);

	generate_sub_terrain(t, xi1, midxi);
	generate_sub_terrain(t, midxi, xi2);
}

void generate_terrain(struct terrain_t *t)
{
	t->npoints = TERRAIN_LENGTH;
	t->x[0] = 0;
	t->y[0] = 100;
	t->x[t->npoints-1] = WORLDWIDTH;
	t->y[t->npoints-1] = t->y[0];

	generate_sub_terrain(t, 0, t->npoints-1);
}

static struct my_vect_obj *make_debris_vect()
{
	int i, n;
	struct my_point_t *p;
	struct my_vect_obj *v;

	/* FIXME, this malloc'ing is a memory leak, */
	
	n = randomab(5,10);
	v = (struct my_vect_obj *) malloc(sizeof(*v));
	p = (struct my_point_t *) malloc(sizeof(*p) * n);
	if (!v || !p) {
		if (v)
			free(v);
		if (p)
			free (p);
		return NULL;
	}

	v->p = &p[0];
	v->npoints = n;
	
	for (i=0;i<n;i++) {
		p[i].x = randomn(20)-10;
		p[i].y = randomn(10)-5;
	}
	return v;
}

static void add_debris(int x, int y, int vx, int vy, int r, struct game_obj_t **victim)
{
	int i, z; 
	struct game_obj_t *o;

	for (i=0;i<=12;i++) {
		z = find_free_obj();
		if (z < 0)
			return;
		o = &game_state.go[z];
		o->last_xi = -1;
		o->x = x;
		o->y = y;
		o->move = bridge_move;
		o->draw = draw_generic;
		o->destroy = generic_destroy_func;
		o->alive = 30;	
		o->color = WHITE;
		o->vx = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (r + 0.0) + (0.0 + vx));
		o->vy = (int) ((-0.5 + random() / (0.0 + RAND_MAX)) * (r + 0.0) + (0.0 + vy));
		o->target = NULL;
		o->v = make_debris_vect();
		if (o->v == NULL)
			o->draw = no_draw;
		if (i==0)
			*victim = o;
	}
}

static void add_flak_guns(struct terrain_t *t)
{
	int i, xi;
	for (i=0;i<level.nflak;i++) {
		xi = (int) (((0.0 + random()) / RAND_MAX) * 
			(TERRAIN_LENGTH - 40) + 40);
		add_generic_object(t->x[xi], t->y[xi] - 7, 0, 0, 
			move_flak, draw_flak, GREEN, &flak_vect, 1, OBJ_TYPE_GUN, 1);
	}
}

static void add_rockets(struct terrain_t *t)
{
	int i, xi;
	for (i=0;i<level.nrockets;i++) {
		xi = (int) (((0.0 + random()) / RAND_MAX) * (TERRAIN_LENGTH - 40) + 40);
		add_generic_object(t->x[xi], t->y[xi] - 7, 0, 0, 
			move_rocket, NULL, WHITE, &rocket_vect, 1, OBJ_TYPE_ROCKET, 1);
	}
}

/* Insert the points in one list into the middle of another list */
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
	printf("aw: npoints = %p\n", npoints); fflush(stdout);
	if (*npoints > 1000) {
		printf("npoints = %d\n", *npoints); fflush(stdout);
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

	printf("aws: npoints = %p\n", npoints); fflush(stdout);
	xindent = randomab(5, 20);
	yindent = randomab(5, 20);
	spacing = randomab(3, 15);
	x2 -= xindent;
	x1 += xindent;
	y1 -= yindent;
	y2 += yindent;
	printf("add_windows, %d,%d  %d,%d\n", x1, y1, x2, y2);

	if (x2 - x1 < 30)
		return;
	if (y1 - y2 < 20)
		return;

	nwindows = randomab(1, 5);
	width = (x2-x1) / nwindows; 
	printf("adding %d windows, *npoints = %d\n", nwindows, *npoints);
	for (i=0;i<nwindows;i++) {
		printf("Adding window -> %d, %d, %d, %d, npoints = %d\n",
			x1 + (i*width), y1, x1 + (i+1)*width - spacing, y2, *npoints);
		fflush(stdout);
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

int find_free_obj()
{
	int i;
	for (i=0;i<MAXOBJS;i++)
		if (!game_state.go[i].alive)
			return i;
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

	memset(scratch, 0, sizeof(scratch[0]) * 1000);
	objnum = find_free_obj();
	if (objnum == -1)
		return;

	height = randomab(50, 100);
	width = randomab(5,MAXBUILDING_WIDTH);
	scratch[0].x = t->x[xi];	
	scratch[0].y = t->y[xi];	
	scratch[1].x = scratch[0].x;
	scratch[1].y = scratch[0].y - height;
	scratch[2].x = t->x[xi+width];
	scratch[2].y = scratch[1].y; /* make roof level, even if ground isn't. */
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
	printf("b, x=%d, y=%d\n", x, y);
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
	printf("top of find_dip, n=%d\n", n);

	found=0;
	x1 = 0;
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
	
			printf("found a dip x1=%d, x2=%d.\n", x1, x2);
			break;
		}
		if (x1 >= TERRAIN_LENGTH-100)
			break;
		found++;
		if (found == n)
			break;
		x1 = x2;
	}
	printf("found %d dips.\n", found);
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

	printf("add_bridge_column, rx =%d, ry=%d, x1 = %d, x2=%d\n", rx, ry, x1, x2);
	while (t->x[i] <= rx && i <= x2)
		i++;
	if (i>x2)  /* we're at the rightmost end of the bridge, that is, done. */
		return;

	terminal_y = interpolate(rx, t->x[i-1], t->y[i-1], t->x[i], t->y[i]);
	printf("term_y = %d, intr(%d, %d, %d, %d, %d)\n", 
		terminal_y, rx, t->y[i-1], t->x[i-1], t->y[i], t->x[i]);

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

static void add_fuel(struct terrain_t *t)
{
	int xi, i;
	for (i=0;i<level.nfueltanks;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			no_move, NULL, ORANGE, &fuel_vect, 1, OBJ_TYPE_FUEL, 1);
	}
}

static void add_ships(struct terrain_t *t)
{
	int xi, i;
	for (i=0;i<level.nships;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			ship_move, NULL, ORANGE, &ship_vect, 1, OBJ_TYPE_SHIP, 50*PLAYER_LASER_DAMAGE);
	}
}

static void add_octopi(struct terrain_t *t)
{
	int xi, i, j, k, count;

	count = 0;
	struct game_obj_t *o;
	for (i=0;i<level.noctopi;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		o = add_generic_object(t->x[xi], t->y[xi]-50 - randomn(100), 0, 0, 
			octopus_move, NULL, YELLOW, &octopus_vect, 1, OBJ_TYPE_OCTOPUS, 1);
		if (o != NULL) {
			count++;
			o->destroy = octopus_destroy;
			o->tsd.octopus.awake = 0;

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
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		o = add_generic_object(t->x[xi], t->y[xi]-30, 0, 0, 
			gdb_move, gdb_draw, CYAN, &gdb_vect_left, 1, OBJ_TYPE_GDB, 1);
		if (o != NULL) {
			o->destroy = generic_destroy_func;
			o->tsd.gdb.awake = 0;

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

static void add_tentacles(struct terrain_t *t)
{
	int xi, i,j, length;
	struct game_obj_t *o;
	double length_factor;

	for (j=0;j<level.ntentacles;j++) {
		length = randomn(30) + 9;
		length_factor = 0.90;
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
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
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_generic_object(t->x[xi], t->y[xi], 0, 0, 
			sam_move, NULL, GREEN, &SAM_station_vect, 1, OBJ_TYPE_SAM_STATION, 1);
	}
}

static void add_humanoids(struct terrain_t *t)
{
	int xi, i;
	for (i=0;i<level.nhumanoids;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_generic_object(t->x[xi], t->y[xi], 0, 0, 
			humanoid_move, NULL, MAGENTA, &humanoid_vect, 1, OBJ_TYPE_HUMAN, 1);
	}
}

static void add_airships(struct terrain_t *t)
{
	int xi, i;
	struct game_obj_t *o;
	for (i=0;i<level.nairships;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		o = add_generic_object(t->x[xi], t->y[xi]-50, 0, 0, 
			airship_move, NULL, CYAN, &airship_vect, 1, OBJ_TYPE_AIRSHIP, 300*PLAYER_LASER_DAMAGE);
		o->counter = 0;
	}
}

static void add_socket(struct terrain_t *t)
{
	add_generic_object(t->x[TERRAIN_LENGTH-1] - 250, t->y[TERRAIN_LENGTH-1] - 250, 
		0, 0, socket_move, NULL, CYAN, &socket_vect, 0, OBJ_TYPE_SOCKET, 1);
}

static void add_balloons(struct terrain_t *t)
{
	int xi, i;
	for (i=0;i<NBALLOONS;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_generic_object(t->x[xi], t->y[xi]-50, 0, 0, 
			balloon_move, NULL, CYAN, &balloon_vect, 1, OBJ_TYPE_BALLOON, 1);
	}
}

static void draw_strings(GtkWidget *w);
void setup_text();

static int do_intermission(GtkWidget *w, GdkEvent *event, gpointer p)
{
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
		intermission_timer = timer + (intermission_stage == 10 ? 4 : 1) * FRAME_RATE_HZ;
	}

	/* printf("timer=%d, timer_event=%d, intermission_timer=%d, stage=%d\n", 
		timer, timer_event, intermission_timer, intermission_stage); */

	cleartext();
	set_font(SMALL_FONT);
	switch (intermission_stage) {
		/* Cases are listed backwards to take advantage of fall-thru */
		case 11:
			timer_event = END_INTERMISSION_EVENT;
			next_timer = timer + 1;
			intermission_timer = 0;
			intermission_stage = 0;
			cleartext();
			gotoxy(0,0);
			sprintf(s, "Credits: %d Lives: %d", credits, game_state.lives);
			game_state.score += add_bonus;
			gameprint(s);
			break;
		case 10: 
			gotoxy(8, 10+3);
			elapsed_secs = game_state.finish_time.tv_sec - game_state.start_time.tv_sec;
			if (elapsed_secs < 90)
				inc_bonus = (90 - elapsed_secs) * 10;
			else
				inc_bonus = 0;
			sprintf(s, "Elapsed time                  %d:%d      %d pts",
				elapsed_secs / 60, elapsed_secs % 60, inc_bonus);
			bonus_points += inc_bonus;	
			add_bonus = bonus_points;
			gameprint(s);
		case 9: 
			gotoxy(8, 9+2);
			if (game_state.sams_killed >= level.nsams)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			bonus_points += inc_bonus;
			sprintf(s, "SAM stations destroyed:       %d/%d     %d pts",
				game_state.sams_killed, level.nsams, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 8: 
			gotoxy(8, 8+2);
			if (game_state.guns_killed >= level.nflak)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Laser turrets killed:         %d/%d    %d pts", 
				game_state.guns_killed, level.nflak, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 7: 
			gotoxy(8, 7+2);
			sprintf(s, "Missiles killed:              %d       0 pts",
				game_state.missiles_killed);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 6: 
			gotoxy(8, 6+2);
			if (game_state.rockets_killed >= level.nrockets)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Rockets killed:               %d/%d   %d pts",
				game_state.rockets_killed, level.nrockets, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 5: 
			gotoxy(8, 5+2);
			if (game_state.octos_killed >= level.noctopi)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "Octo-viruses killed:          %d/%d   %d pts",
				game_state.octos_killed, level.noctopi, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 4: 
			gotoxy(8, 4+2);
			if (game_state.gdbs_killed >= level.ngdbs)
				inc_bonus = 50;
			else
				inc_bonus = 0;
			sprintf(s, "gdb processes killed:         %d/%d   %d pts",
				game_state.gdbs_killed, level.ngdbs, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 3: 
			gotoxy(8, 3+2);
			inc_bonus = game_state.emacs_killed * 100;
			sprintf(s, "Emacs processes terminated:   %d/%d   %d pts",
				game_state.emacs_killed, level.nairships, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 2: 
			gotoxy(8, 2+2);
			inc_bonus = game_state.humanoids * 100;
			sprintf(s, "vi .swp files rescued:        %d/%d   %d pts", 
				game_state.humanoids, level.nhumanoids, inc_bonus);
			bonus_points += inc_bonus;	
			gameprint(s);
		case 1: 
			add_bonus = 0;
			gotoxy(8, 1);
			sprintf(s, "Node cleared!  Total bonus points: %d\n", bonus_points);
			gameprint( s);
	}
	if (intermission_stage != 11)
		draw_strings(w);
	else
		setup_text();
	printf("i, timer_event = %d\n", timer_event);
	return 0;
}

static int main_da_expose(GtkWidget *w, GdkEvent *event, gpointer p)
{
	int i;
	int sx1, sx2;
	static int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;
	char score_str[100];
	// int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;


	if (timer_event == START_INTERMISSION_EVENT) {
		do_intermission(w, event, p);
		return 0;
	}

	sx1 = game_state.x - SCREEN_WIDTH / 3;
	sx2 = game_state.x + 4*SCREEN_WIDTH/3;


	while (terrain.x[last_lowx] < sx1 && last_lowx+1 < TERRAIN_LENGTH)
		last_lowx++;
	while (terrain.x[last_lowx] > sx1 && last_lowx > 0)
		last_lowx--;
	while (terrain.x[last_highx] > sx2 && last_highx > 0)
		last_highx--;
	while (terrain.x[last_highx] < sx2 && last_highx+1 < TERRAIN_LENGTH) {
		last_highx++;
	}

	gdk_gc_set_foreground(gc, &huex[RED]);

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
		gdk_draw_line(w->window, gc, terrain.x[i] - game_state.x, terrain.y[i]+(SCREEN_HEIGHT/2) - game_state.y,  
					 terrain.x[i+1] - game_state.x, terrain.y[i+1]+(SCREEN_HEIGHT/2) - game_state.y);
	}

	/* draw "system memory boundaries" (ha!) */
	if (game_state.x > terrain.x[0] - SCREEN_WIDTH)
		gdk_draw_line(w->window, gc, terrain.x[0] - game_state.x, 0, 
			terrain.x[0] - game_state.x, SCREEN_HEIGHT);
	if (game_state.x > terrain.x[TERRAIN_LENGTH-1] - SCREEN_WIDTH)
		gdk_draw_line(w->window, gc, terrain.x[TERRAIN_LENGTH-1] - game_state.x, 0, 
			 terrain.x[TERRAIN_LENGTH-1] - game_state.x, SCREEN_HEIGHT); 
	set_font(SMALL_FONT);
	if (game_state.y < KERNEL_Y_BOUNDARY + SCREEN_HEIGHT) {
		gdk_draw_line(w->window, gc, 0, KERNEL_Y_BOUNDARY  - game_state.y + SCREEN_HEIGHT/2, 
			SCREEN_WIDTH, KERNEL_Y_BOUNDARY - game_state.y + SCREEN_HEIGHT/2);
		livecursorx = (SCREEN_WIDTH - abs(game_state.x) % SCREEN_WIDTH);
		livecursory = KERNEL_Y_BOUNDARY - game_state.y + SCREEN_HEIGHT/2 - 10;
		draw_string(w, "Kernel Space");
	}

	if (game_state.x > terrain.x[TERRAIN_LENGTH] - SCREEN_WIDTH)
	/* draw health bar */
	if (game_state.health > 0)
		gdk_draw_rectangle(w->window, gc, TRUE, 10, 10, 
			((SCREEN_WIDTH - 20) * game_state.health / MAXHEALTH), 30);
	draw_objs(w);
		sprintf(textline[CREDITS].string, "Credits: %d Lives: %d Score: %d Humans:%d/%d ", 
			credits, game_state.lives, game_state.score, 
			game_state.humanoids, level.nhumanoids);
	draw_strings(w);

	if (game_state.prev_score != game_state.score) {
		sprintf(score_str, "Score: %06d", game_state.score);
		game_state.prev_score = game_state.score;
		gtk_label_set_text(GTK_LABEL(score_label), score_str);
	}
	if (game_state.prev_bombs != game_state.nbombs) {
		sprintf(score_str, "Bombs: %02d", game_state.nbombs);
		game_state.prev_bombs = game_state.nbombs;
		gtk_label_set_text(GTK_LABEL(bombs_label), score_str);
	}

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
    g_print ("Bye bye.\n");
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
			if (randomn(10) < 5)
				timer_event = CREDITS1_EVENT;
			else
				timer_event = INTRO1_EVENT;
		} else
			timer_event = BLINK_EVENT;
		next_timer = timer + 20;
		strcpy(textline[GAME_OVER].string, "");
		break;
	case CREDITS1_EVENT: {
		int yline = 7;
		int x = 12;
		ntextlines = 1;
		set_font(SMALL_FONT);
		gotoxy(x,yline++);
		gameprint("Credits:");
		gotoxy(x,yline++);
		gameprint("Programming:   Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("Game design:   Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("Music:         Stephen Cameron");
		gotoxy(x,yline++);
		gameprint("               and Marty Kiel");
		gotoxy(x,yline++);
		gameprint("Sound effects: Freesound users:");
		gotoxy(x,yline++);
		gameprint("   dobroide, inferno and oniwe");
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
		int yline = 7;
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
		next_timer = timer + 350;
		game_over_count = 0;
		break;
		}
	case INTRO2_EVENT: {
		ntextlines = 1;
		setup_text();
		timer_event = BLINK_EVENT;
		timer_event = BLANK_GAME_OVER_1_EVENT;
		next_timer = timer + 1;
		break;
		}
	case READY_EVENT:
		start_level();
		add_sound(MUSIC_SOUND, MUSIC_SLOT);
		sprintf(textline[CREDITS].string, "Credits: %d Lives: %d Score: %d Humans:%d/%d ", 
			credits, game_state.lives, game_state.score, 
			game_state.humanoids, level.nhumanoids);
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

gint advance_game(gpointer data)
{
	int i, ndead, nalive;

	if (game_pause == 1) {
		return TRUE;
	}

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
			if (game_state.go[i].alive) {
				// printf("%d ", i);
				nalive++;
			} else
				ndead++;
			if (game_state.go[i].alive && game_state.go[i].move != NULL)
				game_state.go[i].move(&game_state.go[i]);
			// if (game_state.go[i].alive && game_state.go[i].move == NULL)
				// printf("NULL MOVE!\n");
		}
		if (game_state.missile_locked && timer % 20 == 0)
			add_sound(MISSILE_LOCK_SIREN_SOUND, ANY_SLOT);
	}
	gtk_widget_queue_draw(main_da);
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
	set_font(SMALL_FONT);
	gotoxy(0,0);
	gameprint("Credits: 0 Lives: 3");
	set_font(BIG_FONT);
	gotoxy(4,3);
	gameprint(" Game Over\n");
	gotoxy(4,2);
	gameprint("Word War vi\n");
	set_font(SMALL_FONT);
	gotoxy(13,15);
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
	game_state.guns_killed = 0;
	game_state.sams_killed = 0;
	game_state.emacs_killed = 0;
	game_state.missiles_killed = 0;
	game_state.octos_killed = 0;
	game_state.rockets_killed = 0;
	game_state.cmd_multiplier = 1;
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
	game_state.health = MAXHEALTH;
	game_state.nbombs = level.nbombs;
	game_state.prev_bombs = -1;
	game_state.nobjs = MAXOBJS-1;
	game_state.x = 0;
	game_state.y = 0;

	srandom(level.random_seed);
	generate_terrain(&terrain);
	add_rockets(&terrain);
	add_buildings(&terrain);
	add_fuel(&terrain);
	add_ships(&terrain);
	add_SAMs(&terrain);
	add_humanoids(&terrain);
	add_bridges(&terrain);
	add_flak_guns(&terrain);
	add_airships(&terrain);
	add_balloons(&terrain);
	add_socket(&terrain);
	add_gdbs(&terrain);
	add_octopi(&terrain);
	add_tentacles(&terrain);

	if (credits == 0)
		setup_text();

}

void init_levels_to_beginning()
{
	level.nrockets = NROCKETS;
	level.nbridges = NBRIDGES;
	level.nflak = NFLAK;
	level.nfueltanks = NFUELTANKS;
	level.nships = NSHIPS;
	level.ngdbs = NGDBS;
	level.noctopi = NOCTOPI;
	level.ntentacles = NTENTACLES;
	level.nsams = NSAMS;
	level.nairships = NAIRSHIPS;
	level.nbuildings = NBUILDINGS;
	level.nbombs = NBOMBS;
	level.nhumanoids = NHUMANOIDS;
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
	start_level();
}

void cancel_sound(int queue_entry);
void advance_level()
{
	/* This is harsh. */
	srandom(level.random_seed);
	level.random_seed = random(); /* deterministic */
	level.nrockets += 10;
	level.nbridges += 1;
	level.nflak += 10;
	level.nfueltanks += 2;
	level.nships += 1;
	level.ngdbs += 2;
	level.noctopi += 2;
	level.ntentacles += 2;
	level.nsams += 2;
	level.nairships += 1;
	level.nhumanoids += 1;
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

	initialize_game_state_new_level();
	start_level();
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
	case GDK_Escape:
		destroy_event(widget, NULL);
		return TRUE;	
	case GDK_q:
		add_sound(INSERT_COIN_SOUND, ANY_SLOT);
		credits++;
		if (credits == 1) {
			cancel_sound(MUSIC_SLOT);
			sleep(2);
			ntextlines = 1;
			game_ended();
			/* initialize_game_state_new_level();
			init_levels_to_beginning();
			start_level(); */
			init_levels_to_beginning();
			timer_event = READY_EVENT;
			next_timer = timer+1;
		}
		sprintf(textline[CREDITS].string, "Credits: %d Lives: %d", credits, game_state.lives);
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
	case GDK_i:
		gettimeofday(&game_state.finish_time, NULL);
		timer_event = START_INTERMISSION_EVENT;
		next_timer = timer + 1;
		return TRUE;
#if 1
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
	double *sample;
} clip[NCLIPS];

struct sound_clip audio_queue[MAX_CONCURRENT_SOUNDS];

int nclips = 0;
#endif

int read_clip(int clipnum, char *filename)
{
#ifdef WITHAUDIOSUPPORT
	SNDFILE *f;
	SF_INFO sfinfo;
	sf_count_t nframes;

	memset(&sfinfo, 0, sizeof(sfinfo));
	f = sf_open(filename, SFM_READ, &sfinfo);
	if (f == NULL) {
		fprintf(stderr, "sf_open('%s') failed.\n", filename);
		return -1;
	}

	printf("Reading sound file: '%s'\n", filename);
	printf("frames = %lld\n", sfinfo.frames);
	printf("samplerate = %d\n", sfinfo.samplerate);
	printf("channels = %d\n", sfinfo.channels);
	printf("format = %d\n", sfinfo.format);
	printf("sections = %d\n", sfinfo.sections);
	printf("seekable = %d\n", sfinfo.seekable);

	clip[clipnum].sample = (double *) 
		malloc(sizeof(double) * sfinfo.channels * sfinfo.frames);
	if (clip[clipnum].sample == NULL) {
		printf("Can't get memory for sound data for %llu frames in %s\n", 
			sfinfo.frames, filename);
		goto error;
	}

	nframes = sf_readf_double(f, clip[clipnum].sample, sfinfo.frames);
	if (nframes != sfinfo.frames) {
		printf("Read only %llu of %llu frames from %s\n", 
			nframes, sfinfo.frames, filename);
	}
	clip[clipnum].nsamples = (int) nframes;
	if (clip[clipnum].nsamples < 0)
		clip[clipnum].nsamples = 0;

	sf_close(f);
	return 0;
error:
	sf_close(f);
	return -1;
#else
	return 0;
#endif
}

#ifdef WITHAUDIOSUPPORT
/* precompute 16 2-second clips of various sine waves */
int init_clips()
{
	memset(&audio_queue, 0, sizeof(audio_queue));

	read_clip(PLAYER_LASER_SOUND, "sounds/18385_inferno_laserbeam.wav");
	read_clip(BOMB_IMPACT_SOUND, "sounds/18390_inferno_plascanh.wav");
	read_clip(ROCKET_LAUNCH_SOUND, "sounds/18386_inferno_lightrl.wav");
	read_clip(FLAK_FIRE_SOUND, "sounds/18382_inferno_hvylas.wav");
	read_clip(LARGE_EXPLOSION_SOUND, "sounds/18384_inferno_largex.wav");
	read_clip(ROCKET_EXPLOSION_SOUND, "sounds/9679__dobroide__firecracker.04_modified.wav");
	read_clip(LASER_EXPLOSION_SOUND, "sounds/18399_inferno_stormplas.wav");
	read_clip(GROUND_SMACK_SOUND, "sounds/ground_smack.wav");
	read_clip(INSERT_COIN_SOUND, "sounds/us_quarter.wav");
	read_clip(MUSIC_SOUND, "sounds/lucky13-steve-mono-mix.wav");
	read_clip(SAM_LAUNCH_SOUND, "sounds/18395_inferno_rltx.wav");
	read_clip(THUNDER_SOUND, "sounds/thunder.wav");
	read_clip(INTERMISSION_MUSIC_SOUND, "sounds/dtox3monomix.wav");
	read_clip(MISSILE_LOCK_SIREN_SOUND, "sounds/34561__DrNI__ob12_triangular_growling_wailing.wav");
	read_clip(CARDOOR_SOUND, "sounds/cardoor.wav");
	read_clip(WOOHOO_SOUND, "sounds/woohoo.wav");
	read_clip(OWMYSPINE_SOUND, "sounds/ow_my_spine.wav");
	read_clip(HELPDOWNHERE_SOUND, "sounds/help_down_here.wav");
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
		for (j=0; j<NCLIPS; j++) {
			if (!audio_queue[j].active || 
				audio_queue[j].sample == NULL)
				continue;
			sample = i + audio_queue[j].pos;
			count++;
			if (sample >= audio_queue[j].nsamples) {
				audio_queue[j].active = 0;
				continue;
			}
			output += audio_queue[j].sample[sample];
		}
		*out++ = (float) output / 2; /* (output / count); */
        }
	for (i=0;i<NCLIPS;i++) {
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

	init_clips();

	rc = Pa_Initialize();
	if (rc != paNoError)
		goto error;
    
	outparams.device = Pa_GetDefaultOutputDevice();  /* default output device */
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

void cancel_sound(int queue_entry)
{
#ifdef WITHAUDIOSUPPORT
	audio_queue[queue_entry].active = 0;
#endif
}

void cancel_all_sounds()
{
#ifdef WITHAUDIOSUPPORT
	int i;
	for (i=0;i<MAX_CONCURRENT_SOUNDS;i++)
		audio_queue[i].active = 0;
#endif
}

/***********************************************************************/
/* End of AUDIO related code                                     */
/***********************************************************************/

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

int main(int argc, char *argv[])
{
	/* GtkWidget is the storage type for widgets */
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *vbox;
	int i;

	struct timeval tm;
	gettimeofday(&tm, NULL);
	srandom(tm.tv_usec);	

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
	gdk_color_parse("yellow", &huex[YELLOW]);
	gdk_color_parse("red", &huex[RED]);
	gdk_color_parse("orange", &huex[ORANGE]);
	gdk_color_parse("cyan", &huex[CYAN]);
	gdk_color_parse("MAGENTA", &huex[MAGENTA]);

	/* Set up the spark colors. */
	setup_spark_colors();

 
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
	gtk_widget_set_size_request(main_da, SCREEN_WIDTH, SCREEN_HEIGHT);

	g_signal_connect(G_OBJECT (main_da), "expose_event", G_CALLBACK (main_da_expose), NULL);

    button = gtk_button_new_with_label ("Quit");
    score_label = gtk_label_new("Score: 000000");
    bombs_label = gtk_label_new("Bombs: 99");
    
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
    gtk_box_pack_start(GTK_BOX (vbox), score_label, FALSE /* expand */, FALSE /* fill */, 2);
    gtk_box_pack_start(GTK_BOX (vbox), bombs_label, FALSE /* expand */, FALSE /* fill */, 2);
    
	init_vects();
	init_vxy_2_dxy();
	
	g_signal_connect(G_OBJECT (window), "key_press_event",
		G_CALLBACK (key_press_cb), "window");


	// print_target_list();

	for (i=0;i<NCOLORS+NSPARKCOLORS;i++)
		gdk_colormap_alloc_color(gtk_widget_get_colormap(main_da), &huex[i], FALSE, FALSE);
	gtk_widget_modify_bg(main_da, GTK_STATE_NORMAL, &huex[BLACK]);


	initialize_game_state_new_level();
	game_state.score = 0;
	start_level();

    gtk_widget_show (vbox);
    gtk_widget_show (main_da);
    // gtk_widget_show (button);
    gtk_widget_show (score_label);
    gtk_widget_show (bombs_label);
    
    /* and the window */
    gtk_widget_show (window);
	gc = gdk_gc_new(GTK_WIDGET(main_da)->window);
	gdk_gc_set_foreground(gc, &huex[BLUE]);
	gdk_gc_set_foreground(gc, &huex[WHITE]);

    timer_tag = g_timeout_add(1000 / FRAME_RATE_HZ, advance_game, NULL);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_main ();

    stop_portaudio();
    
    return 0;
}
