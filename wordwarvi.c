#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <gtk/gtk.h>

#define TERRAIN_LENGTH 1000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WORLDWIDTH (SCREEN_WIDTH * 40)

#define ROUGHNESS (0.35)
#define MAXOBJS 100
#define NROCKETS 70 
#define MAX_ROCKET_SPEED -8

struct my_point_t {
	int x,y;
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

struct game_obj_t;
typedef void obj_move_func(struct game_obj_t *o);

struct game_obj_t {
	struct my_vect_obj *v;
	int x, y;
	int vx, vy;
	obj_move_func *move;
};

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
} game_state = { 0, 0, 0, 0, 5, 0 };

struct game_obj_t *player = &game_state.go[0];

GdkGC *gc = NULL;
GtkWidget *main_da;
gint timer_tag;

void move_rocket(struct game_obj_t *o)
{
	int xdist, ydist;
	xdist = abs(o->x - player->x);
	if (xdist < 250) {
		ydist = abs(o->y - player->y);
		if (xdist <= ydist) {
			if (o->vy > MAX_ROCKET_SPEED)
				o->vy--;
		}
	}
	o->x += o->vx;
	o->y += o->vy;
}

void move_obj(struct game_obj_t *o)
{
	o->x += o->vx;
	o->y += o->vy;
}

void init_vects()
{
	player_vect.p = player_ship_points;
	player_vect.npoints = sizeof(player_ship_points) / sizeof(player_ship_points[0]);
	rocket_vect.p = rocket_points;
	rocket_vect.npoints = sizeof(rocket_points) / sizeof(rocket_points[0]);
#if 0
	player_vect.npoints = 4;
	player_vect.p = malloc(sizeof(*player_vect.p) * player_vect.npoints);
	player_vect.p[0].x = 10; player_vect.p[0].y = 0;
	player_vect.p[1].x = -10; player_vect.p[1].y = -10;
	player_vect.p[2].x = -10; player_vect.p[2].y = 10;
	player_vect.p[3].x = 10; player_vect.p[3].y = 0;
#endif
	player->v = &player_vect;
	player->x = 200;
	player->y = 0;
	player->vx = 5;
	player->vy = 0;
	player->move = move_obj;
	game_state.nobjs = 1;
}

void draw_objs(GtkWidget *w)
{
	int i, j;
	for (i=0;i<game_state.nobjs;i++) {
		struct my_vect_obj *v = game_state.go[i].v;
		struct game_obj_t *o = &game_state.go[i];

		if (o->x < (game_state.x - (SCREEN_WIDTH/3)))
			continue;
		if (o->x > (game_state.x + 4*(SCREEN_WIDTH/3)))
			continue;
		if (o->y < (game_state.y - (SCREEN_HEIGHT/2)))
			continue;
		if (o->y > (game_state.y + (SCREEN_HEIGHT/2)))
			continue;

		for (j=0;j<v->npoints-1;j++) {
			gdk_draw_line(w->window, gc, o->x + v->p[j].x - game_state.x, o->y + v->p[j].y - game_state.y + (SCREEN_HEIGHT/2),  
					 o->x + v->p[j+1].x - game_state.x, o->y + v->p[j+1].y+(SCREEN_HEIGHT/2) - game_state.y);
			
		}
	}
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
	t->y[0] = 0;
	t->x[t->npoints-1] = WORLDWIDTH;
	t->y[t->npoints-1] = t->y[0];

	generate_sub_terrain(t, 0, t->npoints-1);
}

static void add_rockets(struct terrain_t *t)
{
	int i, xi;

	for (i=0;i<NROCKETS;i++) {
		xi = (int) (((0.0 + random()) / RAND_MAX) * TERRAIN_LENGTH);
		game_state.go[i+1].x = t->x[xi];
		game_state.go[i+1].y = t->y[xi] - 5;
		game_state.go[i+1].v = &rocket_vect;
		game_state.go[i+1].vx = 0;
		game_state.go[i+1].vy = 0;
		game_state.go[i+1].move = move_rocket;
		game_state.nobjs++;
	}
}


static int main_da_expose(GtkWidget *w, GdkEvent *event, gpointer p)
{
	int i;
	int sx1, sx2;
	static int last_lowx = 0, last_highx = TERRAIN_LENGTH-1;
	int zz = 0;


	sx1 = game_state.x - SCREEN_WIDTH / 3;
	sx2 = game_state.x + 4*SCREEN_WIDTH/3;


	while (terrain.x[last_lowx] < sx1)
		last_lowx++;
	while (terrain.x[last_lowx] > sx1)
		last_lowx--;
	while (terrain.x[last_highx] > sx2)
		last_highx--;
	while (terrain.x[last_highx] < sx2)
		last_highx++;

	for (i=last_lowx;i<last_highx;i++) {
#if 0
		if (terrain.x[i] < sx1 && terrain.x[i+1] < sx1) /* offscreen to the left */
			continue;
		if (terrain.x[i] > sx2 && terrain.x[i+1] > sx2) /* offscreen to the right */
			continue;

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
	int i;
	game_state.x += game_state.vx;
	game_state.y += game_state.vy;
	for (i=0;i<game_state.nobjs;i++)
		game_state.go[i].move(&game_state.go[i]);
	gtk_widget_queue_draw(main_da);
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

	GdkColor whitecolor;
	GdkColor bluecolor;
	GdkColor blackcolor;

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
    gtk_widget_show (vbox);
    gtk_widget_show (main_da);
    gtk_widget_show (button);

    generate_terrain(&terrain);
    add_rockets(&terrain);
    
    /* and the window */
    gtk_widget_show (window);
	gc = gdk_gc_new(GTK_WIDGET(main_da)->window);

    timer_tag = g_timeout_add(30, advance_game, NULL);
    
    /* All GTK applications must have a gtk_main(). Control ends here
     * and waits for an event to occur (like a key press or
     * mouse event). */
    gtk_main ();
    
    return 0;
}
