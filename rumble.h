#ifndef __RUMBLE_H__
#define __RUMBLE_H__

/*
 * Copyright 2008 Stephen M. Cameron <smcameron@yahoo.com> 
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You can contact the author by email at this address:
 * Johann Deneux <deneux@ifrance.com>
 */

#define RUMBLE_SINE_VIBE_EFFECT 0
#define RUMBLE_CONSTANT_FORCE_EFFECT 1
#define RUMBLE_SPRING__EFFECT 2
#define RUMBLE_DAMPING_EFFECT 3
#define RUMBLE_STRONG_RUMBLE_EFFECT 4
#define RUMBLE_WEAK_RUMBLE_EFFECT 5

extern int stop_all_rumble_effects(void);
extern int play_rumble_effect(int effect);
extern void close_rumble_fd(void);
extern int get_ready_to_rumble(char *filename);

#endif
