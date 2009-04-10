
/*
 *
 * This code is modified version of the fftest.c program
 * which Tests the force feedback driver by Johan Deneux.
 * Modifications to incorporate into Word War vi 
 * by Stephen M.Cameron
 * 
 * Copyright 2001-2002 Johann Deneux <deneux@ifrance.com>
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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef __linux__
#define HAS_LINUX_JOYSTICK_INTERFACE 1
#endif

#ifdef HAS_LINUX_JOYSTICK_INTERFACE
#include <linux/input.h>
#endif

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

#define N_EFFECTS 6

char* effect_names[] = {
	"Sine vibration",
	"Constant Force",
	"Spring Condition",
	"Damping Condition",
	"Strong Rumble",
	"Weak Rumble"
};

#ifdef HAS_LINUX_JOYSTICK_INTERFACE


static int event_fd;
static char *default_event_file = "/dev/input/event5";
static int n_effects;	/* Number of effects the device can play at the same time */
static unsigned long features[4];
static struct ff_effect effects[N_EFFECTS];

#endif /* HAS_LINUX_JOYSTICK_INTERFACE */

int stop_all_rumble_effects(void)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	int i;
	struct input_event stop;

	for (i=0; i<N_EFFECTS; ++i) {
		stop.type = EV_FF;
		stop.code =  effects[i].id;
		stop.value = 0;
        
		if (write(event_fd, (const void*) &stop, sizeof(stop)) == -1) {
			perror("Stop effect");
			exit(1);
		}
	}
#endif
	return 0;
}

int play_rumble_effect(int effect)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	struct input_event play;

	if (effect < 0 || effect >= N_EFFECTS)
		return -1;

	play.type = EV_FF;
	play.code = effects[effect].id;
	play.value = 1;

	if (write(event_fd, (const void*) &play, sizeof(play)) == -1)
		return -1;
#endif
	return 0;
}

void close_rumble_fd(void)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	close(event_fd);
#endif
}

int get_ready_to_rumble(char *filename)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	if (filename == NULL)
		filename = default_event_file;

	event_fd = open(filename, O_RDWR);
	if (event_fd < 0) {
		fprintf(stderr, "Can't open %s: %s\n", 
			filename, strerror(errno));
		return -1;
	}

	printf("Device %s opened\n", filename);

	/* Query device */
	if (ioctl(event_fd, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1) {
		fprintf(stderr, "Query of rumble device failed: %s:%s\n", 
			filename, strerror(errno));
		return -1;	
	}

	printf("Axes query: ");

	if (test_bit(ABS_X, features)) printf("Axis X ");
	if (test_bit(ABS_Y, features)) printf("Axis Y ");
	if (test_bit(ABS_WHEEL, features)) printf("Wheel ");

	printf("\nEffects: ");

	if (test_bit(FF_CONSTANT, features)) printf("Constant ");
	if (test_bit(FF_PERIODIC, features)) printf("Periodic ");
	if (test_bit(FF_SPRING, features)) printf("Spring ");
	if (test_bit(FF_FRICTION, features)) printf("Friction ");
	if (test_bit(FF_RUMBLE, features)) printf("Rumble ");

	printf("\nNumber of simultaneous effects: ");

	if (ioctl(event_fd, EVIOCGEFFECTS, &n_effects) == -1) {
		fprintf(stderr, "Query of number of simultaneous "
			"effects failed, assuming 1. %s:%s\n", 
			filename, strerror(errno));
		n_effects = 1;	 /* assume 1. */
	}

	printf("%d\n", n_effects);

	/* download a periodic sinusoidal effect */
	effects[0].type = FF_PERIODIC;
	effects[0].id = -1;
	effects[0].u.periodic.waveform = FF_SINE;
	effects[0].u.periodic.period = 0.1*0x100;	/* 0.1 second */
	effects[0].u.periodic.magnitude = 0x4000;	/* 0.5 * Maximum magnitude */
	effects[0].u.periodic.offset = 0;
	effects[0].u.periodic.phase = 0;
	effects[0].direction = 0x4000;	/* Along X axis */
	effects[0].u.periodic.envelope.attack_length = 0x100;
	effects[0].u.periodic.envelope.attack_level = 0;
	effects[0].u.periodic.envelope.fade_length = 0x100;
	effects[0].u.periodic.envelope.fade_level = 0;
	effects[0].trigger.button = 0;
	effects[0].trigger.interval = 0;
	effects[0].replay.length = 20000;  /* 20 seconds */
	effects[0].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[0]) == -1) {
		fprintf(stderr, "%s: failed to upload sine effect: %s\n", 
				filename, strerror(errno));
		;
	}

/* XBOX 360 controller doesn't do these, so don't bother. */
#if 0
	/* download a constant effect */
	effects[1].type = FF_CONSTANT;
	effects[1].id = -1;
	effects[1].u.constant.level = 0x2000;	/* Strength : 25 % */
	effects[1].direction = 0x6000;	/* 135 degrees */
	effects[1].u.constant.envelope.attack_length = 0x100;
	effects[1].u.constant.envelope.attack_level = 0;
	effects[1].u.constant.envelope.fade_length = 0x100;
	effects[1].u.constant.envelope.fade_level = 0;
	effects[1].trigger.button = 0;
	effects[1].trigger.interval = 0;
	/* effects[1].replay.length = 20000;*/  /* 20 seconds */
	effects[1].replay.length = 1000;  /* 1 seconds */
	effects[1].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[1]) == -1) {
		fprintf(stderr, "%s: failed to upload constant effect: %s\n", 
			filename, strerror(errno));
	}

	/* download an condition spring effect */
	effects[2].type = FF_SPRING;
	effects[2].id = -1;
	effects[2].u.condition[0].right_saturation = 0x7fff;
	effects[2].u.condition[0].left_saturation = 0x7fff;
	effects[2].u.condition[0].right_coeff = 0x2000;
	effects[2].u.condition[0].left_coeff = 0x2000;
	effects[2].u.condition[0].deadband = 0x0;
	effects[2].u.condition[0].center = 0x0;
	effects[2].u.condition[1] = effects[2].u.condition[0];
	effects[2].trigger.button = 0;
	effects[2].trigger.interval = 0;
	/* effects[2].replay.length = 20000;*/  /* 20 seconds */
	effects[2].replay.length = 1000;  /* 1 seconds */
	effects[2].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[2]) == -1) {
		fprintf(stderr, "%s: failed to upload spring effect: %s\n", 
			filename, strerror(errno));
	}

	/* download an condition damper effect */
	effects[3].type = FF_DAMPER;
	effects[3].id = -1;
	effects[3].u.condition[0].right_saturation = 0x7fff;
	effects[3].u.condition[0].left_saturation = 0x7fff;
	effects[3].u.condition[0].right_coeff = 0x2000;
	effects[3].u.condition[0].left_coeff = 0x2000;
	effects[3].u.condition[0].deadband = 0x0;
	effects[3].u.condition[0].center = 0x0;
	effects[3].u.condition[1] = effects[3].u.condition[0];
	effects[3].trigger.button = 0;
	effects[3].trigger.interval = 0;
	effects[3].replay.length = 20000;  /* 20 seconds */
	effects[3].replay.length = 1000;  /* 1 seconds */
	effects[3].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[3]) == -1) {
		fprintf(stderr, "%s: failed to upload damper effect: %s\n", 
			filename, strerror(errno));
	}
#endif

	/* a strong rumbling effect */
	effects[4].type = FF_RUMBLE;
	effects[4].id = -1;
	effects[4].u.rumble.strong_magnitude = 0x8000;
	effects[4].u.rumble.weak_magnitude = 0;
	effects[4].replay.length = 250;
	effects[4].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[4]) == -1) {
		fprintf(stderr, "%s: failed to upload strong rumbling effect: %s\n", 
			filename, strerror(errno));
	}

	/* a weak rumbling effect */
	effects[5].type = FF_RUMBLE;
	effects[5].id = -1;
	effects[5].u.rumble.strong_magnitude = 0x8000;
	effects[5].u.rumble.weak_magnitude = 0xc000;
	effects[5].replay.length = 250;
	effects[5].replay.delay = 0;

	if (ioctl(event_fd, EVIOCSFF, &effects[5]) == -1) {
		fprintf(stderr, "%s: failed to upload weak rumbling effect: %s\n", 
			filename, strerror(errno));
	}

	return 0;
#else
	return -1;
#endif
}
