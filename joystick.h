#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#define JOYSTICK_DEVNAME "/dev/input/js0"

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */


struct js_event {
	unsigned int time;	/* event timestamp in milliseconds */
	short value;   /* value */
	unsigned char type;     /* event type */
	unsigned char number;   /* axis/button number */
};

struct wwvi_js_event {
	int button[10];
	int stick1_x;
	int stick1_y;
	int stick2_x;
	int stick2_y;
};

extern int open_joystick();
extern int read_joystick_event(struct js_event *jse);
extern void close_joystick();
extern int get_joystick_status(struct wwvi_js_event *wjse);

#endif
