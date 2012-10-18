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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "joystick.h"

static int joystick_fd = -1;

/* These are sensible on Logitech Dual Action Rumble and xbox360 controller. */
static int joystick_x_axis = 0;
static int joystick_y_axis = 1;

int open_joystick(char *joystick_device)
{
	if (joystick_device == NULL)
		joystick_device = JOYSTICK_DEVNAME;
	joystick_fd = open(joystick_device, JOYSTICK_DEV_OPEN_FLAGS);
	if (joystick_fd < 0)
		return joystick_fd;

	/* maybe ioctls to interrogate features here? */

	return joystick_fd;
}

int read_joystick_event(struct js_event *jse)
{
	int bytes;

	bytes = read(joystick_fd, jse, sizeof(*jse)); 

	if (bytes == -1)
		return 0;

	if (bytes == sizeof(*jse))
		return 1;

	printf("Unexpected bytes from joystick:%d\n", bytes);

	return -1;
}

void close_joystick()
{
	close(joystick_fd);
}

int get_joystick_status(struct wwvi_js_event *wjse)
{
	int rc;
	struct js_event jse;
	if (joystick_fd < 0)
		return -1;

	/* memset(wjse, 0, sizeof(*wjse)); */
	while ((rc = read_joystick_event(&jse) == 1)) {
		jse.type &= ~JS_EVENT_INIT; /* ignore synthetic events */
		if (jse.type == JS_EVENT_AXIS) {
			if (jse.number == joystick_x_axis)
				wjse->stick_x = jse.value;
			if (jse.number == joystick_y_axis)
				wjse->stick_y = jse.value;
		} else if (jse.type == JS_EVENT_BUTTON) {
			if (jse.number < 11) {
				switch (jse.value) {
				case 0:
				case 1: wjse->button[jse.number] = jse.value;
					break;
				default:
					break;
				}
			}
		}
	}
	/* printf("%d\n", wjse->stick1_y); */
	return 0;
}

void set_joystick_y_axis(int axis)
{
	joystick_y_axis = axis;
}

void set_joystick_x_axis(int axis)
{
	joystick_x_axis = axis;
}


#if 0
/* a little test program */
int main(int argc, char *argv[])
{
	int fd, rc;
	int done = 0;

	struct js_event jse;

	fd = open_joystick();
	if (fd < 0) {
		printf("open failed.\n");
		exit(1);
	}

	while (!done) {
		rc = read_joystick_event(&jse);
		usleep(1000);
		if (rc == 1) {
			printf("Event: time %8u, value %8hd, type: %3u, axis/button: %u\n", 
				jse.time, jse.value, jse.type, jse.number);
		}
	}
}
#endif
