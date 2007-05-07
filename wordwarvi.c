#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/time.h>

#define TERRAIN_LENGTH 2000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WORLDWIDTH (SCREEN_WIDTH * 40)

#define ROUGHNESS (0.30)
#define MAXOBJS 4500
#define NROCKETS 270 
/* #define NROCKETS 0  */
#define LAUNCH_DIST 500
#define MAX_ROCKET_SPEED -32
#define PLAYER_SPEED 8 
#define LASER_SPEED 40
#define LASER_PROXIMITY 300 /* square root of 300 */
#define LINE_BREAK (-999)
#define NBUILDINGS 40
#define MAXBUILDING_WIDTH 7
#define NFUELTANKS 20

#define MAX_ALT 100
#define MIN_ALT 50

int toggle = 0;
GdkColor whitecolor;
GdkColor bluecolor;
GdkColor blackcolor;

struct target_t;

struct my_point_t {
	int x,y;
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
	{ 18, -25 },
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

struct my_vect_obj {
	int npoints;
	struct my_point_t *p;	
};

struct my_vect_obj player_vect;
struct my_vect_obj rocket_vect;
struct my_vect_obj spark_vect;
struct my_vect_obj right_laser_vect;
struct my_vect_obj fuel_vect;

struct game_obj_t;
typedef void obj_move_func(struct game_obj_t *o);

struct game_obj_t {
	obj_move_func *move;
	struct my_vect_obj *v;
	struct target_t *target;
	int x, y;
	int vx, vy;
	int alive;
	int otype;
};

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
	int nobjs;
	struct game_obj_t go[MAXOBJS];
} game_state = { 0, 0, 0, 0, PLAYER_SPEED, 0 };

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
void remove_target(struct target_t *t)
{
	struct target_t *i;

	for (i=target_head;i!=NULL;i=i->next) {
		if (i == t) {
			if (i == target_head)
				target_head = i->next;
			if (i->next != NULL)
				i->next->prev = i->prev;
			if (i->prev != NULL)
				i->prev->next = i->next;
			i->next = NULL;
			i->prev = NULL;
			free(i);
			break;
		}
	}
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

void move_rocket(struct game_obj_t *o)
{
	int xdist, ydist;
	if (!o->alive)
		return;

	xdist = abs(o->x - player->x);
	if (xdist < LAUNCH_DIST) {
		ydist = o->y - player->y;
		if ((xdist <= ydist && ydist > 0) || o->vy != 0) {
			if (o->vy > MAX_ROCKET_SPEED)
				o->vy--;
		}

		if ((ydist*ydist + xdist*xdist) < 400) {
			explode(o->x, o->y, o->vx, 1, 70, 150, 20);
			o->alive = 0;
			remove_target(o->target);
			return;
		}
	}
	o->x += o->vx;
	o->y += o->vy;
	if (o->vy != 0)
		explode(o->x, o->y, 0, 9, 8, 17, 13);
	if (o->y < -2000) {
		o->alive = 0;
		remove_target(o->target);
	}
}

int find_free_obj();

void laser_move(struct game_obj_t *o);

void player_fire_right()
{
	int i;
	struct game_obj_t *o, *p;

	i = find_free_obj();
	o = &game_state.go[i];
	p = &game_state.go[0];

	o->x = p->x+30;
	o->y = p->y;
	o->vx = p->vx + LASER_SPEED;
	o->vy = 0;
	o->v = &right_laser_vect;
	o->move = laser_move;
	o->otype = 'L';
	o->alive = 20;
}

void move_player(struct game_obj_t *o)
{
	int i;
	o->x += o->vx;
	o->y += o->vy;
	explode(o->x-11, o->y, -7, 0, 7, 10, 9);
	for (i=0;i<TERRAIN_LENGTH;i++) {
		if (terrain.x[i] - o->x > 100 && (terrain.x[i] - o->x) < 300) {
			if (terrain.y[i] - o->y > MAX_ALT) {
				o->vy += 1;
				if (o->vy > 9)
				o->vy = 9;
			} else if (terrain.y[i] - o->y < MIN_ALT) {
				o->vy -= 1;
				if (o->vy < -9)
				o->vy = -9;
			} else if (o->vy > 0) o->vy--;
			else if (o->vy < 0) o->vy++;
			game_state.vy = o->vy;
			break;
		}
	}
	if (randomn(20) < 4)
		player_fire_right();
	
}

void no_move(struct game_obj_t *o)
{
	return;
}

void laser_move(struct game_obj_t *o)
{
	struct target_t *t;
	int dist2;

	if (!o->alive)
		return;
	o->x += o->vx;
	o->y += o->vy;

	for (t=target_head;t != NULL;t=t->next) {
		if (t->o->otype == 'L')
			continue;
		if (t->o->otype == 'b') 
			continue;
		if ((t->o->otype == 'r' || t->o->otype == 'f') && t->o->alive) {
			dist2 = (o->x - t->o->x)*(o->x - t->o->x) + (o->y - t->o->y)*(o->y - t->o->y);
			// printf("dist2 = %d\n", dist2);
			if (dist2 < LASER_PROXIMITY) { /* a hit */
				explode(t->o->x, t->o->y, t->o->vx, 1, 70, 150, 20);
				t->o->alive = 0;
				remove_target(t);
				o->alive = 0;
			}
		}
	}
	if (o->alive)
		o->alive--;
	if (!o->alive) {
		remove_target(o->target);
		o->target = NULL;
	}
}

void move_obj(struct game_obj_t *o)
{
	o->x += o->vx;
	o->y += o->vy;
}

void move_spark(struct game_obj_t *o)
{
	if (o->alive < 0)
		o->alive = 0;
	if (!o->alive)
		return;

	// printf("x=%d,y=%d,vx=%d,vy=%d, alive=%d\n", o->x, o->y, o->vx, o->vy, o->alive);
	o->x += o->vx;
	o->y += o->vy;
	o->alive--;
	if (!o->alive)
		return;
	// printf("x=%d,y=%d,vx=%d,vy=%d, alive=%d\n", o->x, o->y, o->vx, o->vy, o->alive);
	
	if (o->vx > 0)
		o->vx--;
	else if (o->vx < 0)
		o->vx++;
	if (o->vy > 0)
		o->vy--;
	else if (o->vy < 0)
		o->vy++;

	if (o->vx == 0 && o->vy == 0)
		o->alive = 0;
	if (o->y > 2000 || o->y < -2000 || o->x > 2000+WORLDWIDTH || o->x < -2000)
		o->alive = 0;
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

void init_vects()
{
	int i;

	/* memset(&game_state.go[0], 0, sizeof(game_state.go[0])*MAXOBJS); */
	for (i=0;i<MAXOBJS;i++) {
		game_state.go[i].alive = 0;
		game_state.go[i].move = move_obj;
	}
	player_vect.p = player_ship_points;
	player_vect.npoints = sizeof(player_ship_points) / sizeof(player_ship_points[0]);
	rocket_vect.p = rocket_points;
	rocket_vect.npoints = sizeof(rocket_points) / sizeof(rocket_points[0]);
	spark_vect.p = spark_points;
	spark_vect.npoints = sizeof(spark_points) / sizeof(spark_points[0]);
	right_laser_vect.p = right_laser_beam_points;
	right_laser_vect.npoints = sizeof(right_laser_beam_points) / sizeof(right_laser_beam_points[0]);
	fuel_vect.p = fuel_points;
	fuel_vect.npoints = sizeof(fuel_points) / sizeof(fuel_points[0]);
#if 0
	player_vect.npoints = 4;
	player_vect.p = malloc(sizeof(*player_vect.p) * player_vect.npoints);
	player_vect.p[0].x = 10; player_vect.p[0].y = 0;
	player_vect.p[1].x = -10; player_vect.p[1].y = -10;
	player_vect.p[2].x = -10; player_vect.p[2].y = 10;
	player_vect.p[3].x = 10; player_vect.p[3].y = 0;
#endif
	player->move = move_player;
	player->v = &player_vect;
	player->x = 200;
	player->y = -100;
	player->vx = PLAYER_SPEED;
	player->vy = 0;
	player->target = add_target(player);
	player->alive = 1;
	game_state.nobjs = MAXOBJS-1;
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
		for (j=0;j<v->npoints-1;j++) {
			if (v->p[j+1].x == LINE_BREAK) /* Break in the line segments. */
				j+=2;
			x1 = o->x + v->p[j].x - game_state.x;
			y1 = o->y + v->p[j].y - game_state.y + (SCREEN_HEIGHT/2);  
			x2 = o->x + v->p[j+1].x - game_state.x; 
			y2 = o->y + v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y;
			gdk_draw_line(w->window, gc, x1, y1, x2, y2); 
		}
	}
}

static void add_spark(int x, int y, int vx, int vy, int time)
{
	int i;
	for (i=0;i<MAXOBJS;i++) {
		struct game_obj_t *g = &game_state.go[i];
		if (!g->alive) {
			/* printf("i=%d\n", i); */
			g->move = move_spark;
			g->x = x;
			g->y = y;
			g->vx = vx;
			g->vy = vy;
			g->v = &spark_vect;
			g->otype = 's';
			g->alive = time;
			break;
		}
	}
	if (i>=MAXOBJS)
		printf("no sparks left\n");
}

void perturb(int *value, int lower, int upper, double percent)
{
	double perturbation;

	perturbation = percent * (lower - upper) * ((0.0 + random()) / (0.0 + RAND_MAX) - 0.5);
	*value += perturbation;
	return;
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
	
	perturb(&x3, x2, x1, ROUGHNESS);
	perturb(&y3, x2, x1, ROUGHNESS);

	t->x[midxi] = x3;
	t->y[midxi] = y3;
	printf("gst %d %d\n", x3, y3);

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

static void add_rockets(struct terrain_t *t)
{
	int i, xi;

	for (i=0;i<NROCKETS;i++) {
		struct game_obj_t *g = &game_state.go[i+1];
		xi = (int) (((0.0 + random()) / RAND_MAX) * (TERRAIN_LENGTH - 40) + 40);
		g->move = move_rocket;
		g->x = t->x[xi];
		g->y = t->y[xi] - 7;
		g->v = &rocket_vect;
		g->vx = 0;
		g->vy = 0;
		g->otype = 'r';
		g->target = add_target(g);
		g->alive = 1;
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
		case 3:
		case 4:
		case 5: indent_roof(building, npoints, left, right);
			break;
		case 6: crenellate_roof(building, npoints, left, right);
			break;
		case 9: addtower_roof(building, npoints, left, right);
			break;
		default:
			break;
	}
}

static void embellish_building(struct my_point_t *building, int *npoints)
{
	embellish_roof(building, npoints, 1, 2);
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
	width = randomab(1,MAXBUILDING_WIDTH);
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
	o->otype = 'b';
	o->target = add_target(o);
	o->alive = 1;
	printf("b, x=%d, y=%d\n", x, y);
}

static void add_buildings(struct terrain_t *t)
{
	int xi, i;

	for (i=0;i<NBUILDINGS;i++) {
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		add_building(t, xi);
	}
}

static void add_fuel(struct terrain_t *t)
{
	int xi, i, j;
	struct game_obj_t *o;

	for (i=0;i<NFUELTANKS;i++) {
		j = find_free_obj();
		o = &game_state.go[j];
		xi = randomn(TERRAIN_LENGTH-MAXBUILDING_WIDTH-1);
		o->x = t->x[xi];
		o->y = t->y[xi]-30;
		o->vx = 0;
		o->vy = 0;
		o->move = no_move;
		o->target = add_target(o);
		o->v = &fuel_vect;
		o->otype = 'f';
		o->alive = 1;
	}
}

static int main_da_expose(GtkWidget *w, GdkEvent *event, gpointer p)
{
	int i;
	int sx1, sx2;
	static int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;
	// int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;

	struct timeval tm;

	gettimeofday(&tm, NULL);
	srandom(tm.tv_usec);	
	

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

	gdk_gc_set_foreground(gc, &bluecolor);

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
	gdk_gc_set_foreground(gc, &blackcolor);
	draw_objs(w);
	return 0;
}

/* This is a callback function. The data arguments are ignored
 * in this example. More on callbacks below. */
static void hello( GtkWidget *widget,
                   gpointer   data )
{
    g_print ("Bye bye.\n");
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


gint advance_game(gpointer data)
{
	int i, ndead, nalive;

	gdk_threads_enter();
	ndead = 0;
	nalive = 0;
	game_state.x += game_state.vx;
	game_state.y += game_state.vy; 
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
	gtk_widget_queue_draw(main_da);
	// printf("ndead=%d, nalive=%d\n", ndead, nalive);
	gdk_threads_leave();
	if (WORLDWIDTH - game_state.x < 100)
		return FALSE;
	else
		return TRUE;
}


int main(int argc, char *argv[])
{
	/* GtkWidget is the storage type for widgets */
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *vbox;


	gtk_set_locale();
	gtk_init (&argc, &argv);
   
	gdk_color_parse("white", &whitecolor);
	gdk_color_parse("blue", &bluecolor);
	gdk_color_parse("black", &blackcolor);
 
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
	gtk_widget_modify_bg(main_da, GTK_STATE_NORMAL, &whitecolor);
	gtk_widget_set_size_request(main_da, SCREEN_WIDTH, SCREEN_HEIGHT);

	g_signal_connect(G_OBJECT (main_da), "expose_event", G_CALLBACK (main_da_expose), NULL);

    button = gtk_button_new_with_label ("Quit");
    
    /* When the button receives the "clicked" signal, it will call the
     * function hello() passing it NULL as its argument.  The hello()
     * function is defined above. */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (hello), NULL);
    
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
    /* The final step is to display this newly created widget. */

    generate_terrain(&terrain);
    add_rockets(&terrain);
    add_buildings(&terrain);
	add_fuel(&terrain);

	print_target_list();

    gtk_widget_show (vbox);
    gtk_widget_show (main_da);
    gtk_widget_show (button);
    
    /* and the window */
    gtk_widget_show (window);
	gc = gdk_gc_new(GTK_WIDGET(main_da)->window);

    timer_tag = g_timeout_add(30, advance_game, NULL);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_main ();
    
    return 0;
}
