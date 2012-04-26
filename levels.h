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

/* 
 *
 * This file controls how the levels look, to some degree.  If you want
 * to make your own custom levels, this is the file to modify.
 *
 * How to make a new level.
 *
 * 1.  Copy an existing level.
 *     a. Look for the comment below that says "level 1 starts here:"
 *        and another comment that says "level 1 ends here. ^^^".
 *        Copy the text between those comments to just above the
 *        line which says:
 *
 *        INSERT NEW LEVELS ABOVE THIS LINE ^^^^
 *
 * 2.  Modify the new level:
 *     a.  Change the name of the level.
 *         There are two structures which make up a level, the
 *         level_obj_descriptor_entry array, and the 
 *         level_descriptor_entry.  You must change the name of
 *         both of them to something not yet used:
 *
 *         Change "level_1_obj" to something else, "my_level_objects"
 *         or maybe some descriptive term, like maybe your level has a 
 *         lot of rockets and nothing else, so maybe you call it 
 *         "rocket_level".  Likewise, change "level1" to a new name
 *         which you make up.
 *
 *     b.  Customize the objects:
 *         The level_obj_descriptor_entry array contains a list of objects
 *         which get added to the game.  Each row in the table contains
 *         4 items, as follows:
 *
 * 	   1. Object type, 
 * 	   2. How many objects of this type this row adds.
 * 	   3. X position in the game.
 * 	   4. how much space to put between each object.
 *
 * 	   The value for Object type must be one of these:
 *
		OBJ_TYPE_AIRSHIP
		OBJ_TYPE_BALLOON
		OBJ_TYPE_CRON
		OBJ_TYPE_FUEL
		OBJ_TYPE_SHIP
		OBJ_TYPE_GUN
		OBJ_TYPE_ROCKET
		OBJ_TYPE_SAM_STATION
		OBJ_TYPE_GDB
		OBJ_TYPE_OCTOPUS
		OBJ_TYPE_TENTACLE
		OBJ_TYPE_JAMMER
		OBJ_TYPE_WORM
		OBJ_TYPE_KGU
		OBJ_TYPE_JET

		See below for a description of what each one is.

	   How many objects is pretty self explanatory.


	   X position is expressed as a percentage of the width
           of the game world.  A value of 50 would put the first
           item smack in the center of the game world.  The special
	   value "DO_IT_RANDOMLY" makes the game choose a random
           location.

	   The offset controls how far apart multiple objects are
           spaced.  If the X value is DO_IT_RANDOMLY, then this
           spacing value is not used (but you must specify a value
           anyway, 0, in that case is good. 

	   Some examples:

           { OBJ_TYPE_JET, 	15, DO_IT_RANDOMLY, 0 }, 
           This says add 15 jets, randomly sprinkled around on the terrain.

           { OBJ_TYPE_JET, 	1, 30, 0 }, 
           { OBJ_TYPE_JET, 	1, 50, 0 }, 
           { OBJ_TYPE_JET, 	1, 80, 0 }, 

	   The above adds 3 jets, one at 30% across the game's terrain,
	   one at 50%, and one at 80%.

           { OBJ_TYPE_JET, 	5, 20, 10 }, 
	
	   The above adds 5 jets, beginning at 20% across the terrain,
	   and evenly spaced at 10 unit intervals.

 *     c.  Customize the level_descriptor_entry values.

	   Each level can be further customized by specifying:

	   small scale roughness,
           large scale roughness,
	   laser_fire_chance,
	   number of bridges
           number of bombs
           number of gravity bombs.

	   Find the lines below near the bottom of this file 
           which begin "NEW_LEVEL(....)" and have a comment that
           says, "--- level descriptors begin here ---"

	   The NEW_LEVEL macro constructs a level_descriptor_entry.

	   The format is:

	   NEW_LEVEL(levelvar, levelobjectlist, ssr, lsr, lfc, nbr, nbo, ngb, lspd, mkgh, levelname);

	   levelvar is the name of the variable holding your level description. e.g., my_level.

           levelobjectlist is the name of the structure you made in step 2b, 
           above, e.g. "my_level_objects".

	   ssr is "small scale roughness."  It controls how smooth or rough
           the terrain is at a small scale.  It should be between 0 and 1.
           Values closer to zero are smoother, values closer to 1 are rougher.
           A value of 0.15 is pretty good.  Experiment to see what it does.

	   lsr is "large scale roughness."  It controls how smooth or rough
           the terrain is at a large scale.  It should be between 0 and 1.
           Values closer to zero are smoother, values closer to 1 are rougher.
           A value of 0.09 or so is pretty good.  Experiment to see what it does.

	   lfc is "laser fire chance", and it controls how aggressive or tame
	   the laser guns in the game are.  A value of 20 is very aggressive.
           smaller values are less aggressive, larger values more aggressive.
	   Experiment with it.

	   nbr is the number of bridges to add to the terrain.  Actually it is
	   a maximum number, you may get less bridges.  If your terrain is too
	   smooth, there might not be enough places to add bridges.  This is mostly
	   a cosmetic thing.  A value of 5 or so is fine.

	   nbo is the number of bombs the player starts the level with.  Normally
	   this is NBOMBS, which is 100.  If you want to make a very hard level,
	   you might set nbr to something small, or zero to take bombs out of
	   the game. (Some ground based things are hard to kill without bombs though,
	   so keep that in mind.

	   ngb is the number of gravity bombs the player starts out with.  Normally,
	   this is 3, as they are kind of like the "smart bomb" in this game.  You
	   can set it to what you like for your level.

	   lspd is the laser speed, either SLOW_LASER, or FAST_LASER.  This is an
	   integer factor which is multiplied into the velocity of the laserbolts
	   coming from the laser guns. 

	   mkgh is the maximum kernel gun (laser) health, or in other words the number
	   of laser hits it takes to kill.  Easy is 1, which means 1 shot will kill it.
	   3 or 4 makes tham significantly harder.  (Also they heal over time.)

	   levelname is the name of your level, e.g: "My Level".  This string will
	   appear in the game at the beginning of the level.

	   Example:

	NEW_LEVEL(my_level, my_level_obj, 0.09, 0.04, 20, 5, 100, 3, "My Level");

	This creates a new level named my_level ("My Level") using the objects specified in
	my_level_obj list (not shown), with small scale roughness of 0.09,
	large scale roughness of 0.04, laser fire chance of 20, up to 5 
	bridges, the player has 100 bombs, and 3 gravity bombs. 

	   
 * 3.   Add your level into the level list. 
 *
 *      Go to the bottom of this file, and find the code that looks like:
 *
 *           struct level_descriptor_entry *leveld[] = {
 *
 *      near the comment that says:
 *
 *      "Add your new level name above this line, with an ampersand."
 *
 *      This is an array of levels.  Add your level which you defined 
 *      with the NEW_LEVEL() macro in step 2c, above, into the list, 
 *      preceded by an ampersand, and followed by a comma.
 *
 *      You would add a line into the array like:
 *
 *      &my_level,
 *
 *      The position in the array controls which level it is.  E.g.
 *      if yours is first in the array, it will be level 1.  if it's 
 *      2nd, it will be level 2, etc.
 *
 * 4.   Testing your level:
 *
 *      To test your level, the easiest way is to make it level 1, put
 *      it first in the array of levels, then of course, build the 
 *      game (type "make") and run it.  Once you have the level working
 *      to your liking, you can then move it into the correct position 
 *      in the array.
 *
 *      That's basically it.
 *
 *
 *
 *  If you make any particularly cool levels, feel free to send them to
 *  me at stephenmcameron@gmail.com.  Maybe I'll put them in 
 *  the game.
 *
 */


/* Object types, just arbitrary constants used to uniquely id object types */

/***********************************************************/
/* 
 * VALID OBJECT TYPES DESCRIBED BELOW
 *
 */
/* The ones below are the ones which can be custom placed. */
#define OBJ_TYPE_AIRSHIP 'a'		/* Blimp */ 
#define OBJ_TYPE_BALLOON 'B'		/* Balloons... don't really do anything yet. */
#define OBJ_TYPE_CRON 'C'		/* The green things that pick up humans */
#define OBJ_TYPE_FUEL 'f'		/* Fuel tanks. */
#define OBJ_TYPE_SHIP 'w'		/* Bill Gates's state of the art warship. */
#define OBJ_TYPE_GUN 'g'		/* ground based laser gun */
#define OBJ_TYPE_ROCKET 'r'		/* ground based rockets */
#define OBJ_TYPE_BIG_ROCKET 'I'		/* ground based rockets */
#define OBJ_TYPE_SAM_STATION 'S'	/* ground based missile launching station */
#define OBJ_TYPE_GDB 'd'		/* GDB enemy */
#define OBJ_TYPE_OCTOPUS 'o'		/* a big ol' octopus */
#define OBJ_TYPE_TENTACLE 'j'		/* a big ol' tentacle */
#define OBJ_TYPE_JAMMER 'J'		/* A radar jamming station */
#define OBJ_TYPE_WORM 'W'		/* A worm */
#define OBJ_TYPE_KGUN 'k'		/* a "kernel gun", inverted laser gun suspended on a tower */
#define OBJ_TYPE_JET '-'		/* a jet plane that swoops by and shoots missiles */
#define OBJ_TYPE_GUNWHEEL 'O'		/* a big gun wheel */
/* The ones above are the ones which can be custom placed. */
/***********************************************************/


/* These cannot be custom placed, they are generally for more transient */
/* objects like laser beams, and bullets and what not. */
#define OBJ_TYPE_BOMB 'p'
#define OBJ_TYPE_BUILDING 'b'
#define OBJ_TYPE_CHAFF 'c'
#define OBJ_TYPE_HUMAN 'h'
#define OBJ_TYPE_LASER 'L'
#define OBJ_TYPE_MISSILE 'm'
#define OBJ_TYPE_HARPOON 'H'
#define OBJ_TYPE_SOCKET 'x'
#define OBJ_TYPE_SPARK 's'
#define OBJ_TYPE_PIXIE_DUST '.'
#define OBJ_TYPE_BRIDGE 'T'
#define OBJ_TYPE_SYMBOL 'z'
#define OBJ_TYPE_FLOATING_MESSAGE 'M'
#define OBJ_TYPE_BULLET '>'
#define OBJ_TYPE_PLAYER '1'
#define OBJ_TYPE_DEBRIS 'D'
#define OBJ_TYPE_VOLCANO 'v'
#define OBJ_TYPE_TRUSS 't'
#define OBJ_TYPE_JETPILOT 'e'
#define OBJ_TYPE_REINDEER 'R'
#define OBJ_TYPE_HOUSE '^'
#define OBJ_TYPE_PRESENT 'G'
#define OBJ_TYPE_SCENERY '9'
#define OBJ_TYPE_TESLA 'V'

#define NHOUSES 10

#define DO_IT_RANDOMLY (-1)

/* Type definitions for level describing structures. */
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
	int jetpilot_firechance;
	int laser_velocity_factor;
	int max_kgun_health;
	char *level_name;
};


#define NEW_LEVEL(levelvar, theobjlist, ssr, lsr, lfc, nbr, nbo, ngb, jfc, lspd, mkgh, levelname) \
struct level_descriptor_entry levelvar = { \
	theobjlist, \
	sizeof(theobjlist) / sizeof(theobjlist[0]), \
	ssr, lsr, lfc, nbr, nbo, ngb, jfc, lspd, mkgh, levelname }

/* Below, the game's levels are defined. */

/***************************************************************************/
/* level 1 starts here: */
struct level_obj_descriptor_entry level_1_obj[] = {
	{ OBJ_TYPE_ROCKET, 	140, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET, 	0, DO_IT_RANDOMLY, 0 }, 
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
	{ OBJ_TYPE_OCTOPUS,	1, DO_IT_RANDOMLY, 0 },
};
/* level 1 ends here. ^^^ */
/***************************************************************************/



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
	{ OBJ_TYPE_AIRSHIP,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		4, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	{ OBJ_TYPE_TESLA,	1, 30, 1 }, 
	{ OBJ_TYPE_BIG_ROCKET, 	3, DO_IT_RANDOMLY, 0 }, 
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 },  */
};
/* end of level 2 */

/* level 3 */
struct level_obj_descriptor_entry level_3_obj[] = {
	{ OBJ_TYPE_ROCKET,	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	18, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SAM_STATION, 8, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN,		30, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN,	30, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	3, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		7, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 45, 1 }, 
	{ OBJ_TYPE_BIG_ROCKET, 	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_TESLA,       1, 60, 0 },
	{ OBJ_TYPE_TESLA,       1, 40, 0 },
	{ OBJ_TYPE_GUNWHEEL, 	1, 95, 0 },
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 }, */
};
/* end of level 3 */

/* level 4 */
struct level_obj_descriptor_entry level_4_obj[] = {
	{ OBJ_TYPE_ROCKET,	5, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	10, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SAM_STATION, 5, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN,		15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN,	20, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	4, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		30, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 25, 1 }, 
	{ OBJ_TYPE_BIG_ROCKET, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUNWHEEL, 	1, 75, 0 },
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 },  */
};
/* end of level 4 */

/* level 5 */
struct level_obj_descriptor_entry level_5_obj[] = {
	{ OBJ_TYPE_ROCKET,	5, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		4, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	23, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	10, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SAM_STATION, 8, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN,		18, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN,	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	4, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		9, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 75, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 35, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	{ OBJ_TYPE_BIG_ROCKET, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_TESLA, 	1, 40, 0 },
	{ OBJ_TYPE_TESLA, 	1, 60, 0 },
	{ OBJ_TYPE_GUNWHEEL, 	1, 25, 0 },
	{ OBJ_TYPE_GUNWHEEL, 	1, 75, 0 },
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 }, */
};
/* end of level 5 */

/* level 6 */
struct level_obj_descriptor_entry level_6_obj[] = {
	{ OBJ_TYPE_ROCKET,	25, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JET,		7, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	18, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	10, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_SAM_STATION, 9, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GUN,		28, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_KGUN,	28, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_AIRSHIP,	4, 90, 0 }, 
	{ OBJ_TYPE_WORM,	3, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		12, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 75, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 15, 1 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 65, 1 }, 
	{ OBJ_TYPE_BIG_ROCKET, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_TESLA, 	1, 40, 0 },
	{ OBJ_TYPE_TESLA, 	1, 60, 0 },
	{ OBJ_TYPE_TESLA, 	1, 80, 0 },
	{ OBJ_TYPE_TESLA, 	1, 90, 0 },
	{ OBJ_TYPE_TESLA, 	1, 10, 0 },
	{ OBJ_TYPE_GUNWHEEL, 	1, 25, 0 }, 
	{ OBJ_TYPE_GUNWHEEL, 	1, 50, 0 }, 
	{ OBJ_TYPE_GUNWHEEL, 	1, 75, 0 },
	{ OBJ_TYPE_GUNWHEEL, 	1, 15, 0 },
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 },  */
};
/* end of level 6 */

/* "jet" level begins */
struct level_obj_descriptor_entry jet_level_obj[] = {
	/* { OBJ_TYPE_ROCKET,	25, DO_IT_RANDOMLY, 0 }, */
	{ OBJ_TYPE_JET,		55, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_FUEL, 	15, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_JAMMER,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_CRON,	10, DO_IT_RANDOMLY, 0 }, 
	/* { OBJ_TYPE_SHIP,	0, DO_IT_RANDOMLY, 0 },  */
	/* { OBJ_TYPE_SAM_STATION, 5, DO_IT_RANDOMLY, 0 },  */
	/* { OBJ_TYPE_GUN,	11, DO_IT_RANDOMLY, 0 },  */
	/* { OBJ_TYPE_KGUN,	0, DO_IT_RANDOMLY, 0 },  */
	{ OBJ_TYPE_AIRSHIP,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_WORM,	2, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_BALLOON,	1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_GDB,		1, DO_IT_RANDOMLY, 0 }, 
	{ OBJ_TYPE_OCTOPUS,	1, 85, 1 }, 
	/* { OBJ_TYPE_TENTACLE, 0, DO_IT_RANDOMLY, 0 },  */
};
/* "jet" level ends */


/* -------------------- INSERT NEW LEVELS ABOVE THIS LINE ^^^^ ----------- */

#define NBOMBS 100		/* Number of bombs allocated to player at beginning of levels */
#define NGBOMBS 3		/* Number of gravity bombs allocated at beginning of levels */

/* Some sample laser aggressiveness levels. */
#define DOCILE_LASER     2	/* chance/1000 that flak guns (laser turrets) will fire if in range */
#define LAZY_LASER       8
#define AVERAGE_LASER    13 
#define AGGRESSIVE_LASER 20
#define KILLER_LASER     25 
#define SLOW_LASER 1
#define FAST_LASER 2
#define EASY_KGUNS 1
#define MEDIUM_KGUNS 2
#define HARD_KGUNS 5 

#define NBRIDGES 2		/* max initial number of bridges in terrain (less, if no valleys) */

/* ---------------------level descriptors begin here.---------------------- */
NEW_LEVEL(jet_level, jet_level_obj, 0.11, 0.09, AVERAGE_LASER, NBRIDGES + 1, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, SLOW_LASER, EASY_KGUNS, "Welcome, Noob!");
NEW_LEVEL(level1, level_1_obj, 0.14, 0.04, LAZY_LASER, NBRIDGES, 
	NBOMBS, NGBOMBS, KILLER_LASER, SLOW_LASER, EASY_KGUNS, "Rocket Alley");
NEW_LEVEL(level2, level_2_obj, 0.17, 0.09, LAZY_LASER, NBRIDGES + 1, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, SLOW_LASER, EASY_KGUNS, "Vi! Vi! Vi!");
NEW_LEVEL(level3, level_3_obj, 0.23, 0.15, AVERAGE_LASER, NBRIDGES + 1, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, SLOW_LASER, EASY_KGUNS, "Joy of VIctory");
NEW_LEVEL(level4, level_4_obj, 0.29, 0.17, AVERAGE_LASER, NBRIDGES + 1, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, FAST_LASER, MEDIUM_KGUNS, "Debugging Hell");
NEW_LEVEL(level5, level_5_obj, 0.24, 0.22, AVERAGE_LASER, NBRIDGES + 1, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, FAST_LASER, MEDIUM_KGUNS, "EMACSed Out");
NEW_LEVEL(level6, level_6_obj, 0.14, 0.22, AGGRESSIVE_LASER, NBRIDGES + 3, 
	NBOMBS, NGBOMBS, AGGRESSIVE_LASER, FAST_LASER, HARD_KGUNS, "Revenge of RMS");
/* ---------------------level descriptors end here.---------------------- */


/* This is an array of all the levels.  Add your new level 
 * which you defined above, into the list below, where indicated: */
struct level_descriptor_entry *leveld[] = {
	&jet_level, /* 1st level */
	&level1,    /* 2nd level */
	&level2,    /* 3rd level, etc. */
	&level3,
	&level4,
	&level5,
	&level6,

	/* Add your new level name above this line, with an ampersand. */
	NULL,
};
	

#endif
