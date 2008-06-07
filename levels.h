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
#ifndef __LEVELS_H__
#define __LEVELS_H__

#define DO_IT_RANDOMLY (-1)

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
#define OBJ_TYPE_JET '-'


struct level_obj_descriptor_entry {
	int obj_type;
	int nobjs;
	int x;
	int xoffset;
};

struct level_descriptor_entry {
	struct level_obj_descriptor_entry *objdesc;
	int nobj_desc_entries;
	double small_scale_roughness;
	double large_scale_roughness;
	int laser_fire_chance;
	int nbridges;
	int nbombs;
	int ngbombs;
};

/* level 1 */
struct level_obj_descriptor_entry level_1_obj[] = {
	{ OBJ_TYPE_ROCKET, 	20, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	20, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER, 	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON, 	15, DO_IT_RANDOMLY, 0 }, 
	/* { OBJ_TYPE_SHIP, 	0, DO_IT_RANDOMLY, 0 },  */
	{ OBJ_TYPE_SAM_STATION, 3, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN, 	10, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN, 	30, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP, 	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM, 	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON, 	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB, 	3, DO_IT_RANDOMLY, 0 }, 
	/* { OBJ_TYPE_OCTOPUS,	0, DO_IT_RANDOMLY, 0 },  */
	/* { OBJ_TYPE_TENTACLE,	0, DO_IT_RANDOMLY, 0 },  */
};

struct level_descriptor_entry level1 = {
	level_1_obj,
	sizeof(level_1_obj) / sizeof(level_1_obj[0]),
	0.09, /* small scale terrain roughness */
	0.04, /* large scale terrain roughness */
	20, /* laser fire chance */
	NBRIDGES,
	NBOMBS,
	NGBOMBS,
};

/* level 2 */
struct level_obj_descriptor_entry level_2_obj[] = {
	{ OBJ_TYPE_ROCKET,	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	19, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SAM_STATION, 5, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN,		11, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN,	30, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		4, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	// { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 }, 
};

struct level_descriptor_entry level2 = {
	level_2_obj,
	sizeof(level_2_obj) / sizeof(level_2_obj[0]),
	0.15, /* small scale roughness */
	0.09, /* large scale roughness */
	LASER_FIRE_CHANCE,
	NBRIDGES + 1,
	NBOMBS,
	NGBOMBS,
};

struct level_obj_descriptor_entry jet_level_obj[] = {
	// { OBJ_TYPE_ROCKET,	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		55, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	10, DO_IT_RANDOMLY, 0 }, 
	// { OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	// { OBJ_TYPE_SAM_STATION, 5, DO_IT_RANDOMLY, 0 }, 
	// { OBJ_TYPE_GUN,	11, DO_IT_RANDOMLY, 0 }, 
	// { OBJ_TYPE_KGUN,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	// { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 }, 
};

struct level_descriptor_entry jet_level = {
	jet_level_obj,
	sizeof(jet_level_obj) / sizeof(jet_level_obj[0]),
	0.15, /* small scale roughness */
	0.09, /* large scale roughness */
	LASER_FIRE_CHANCE,
	NBRIDGES + 1,
	NBOMBS,
	NGBOMBS,
};

struct level_descriptor_entry *leveld[] = {
	&jet_level,
	&level1,
	&level2,
	NULL,
};
	

#endif
