
/* 

   Sherman's aquarium - Screensaver part

   Updated and partly rewritten for Sherman's aquarium v3.0.0 on
   30th and 31st December 2003.

   Jonas Aaberg <cja@gmx.net>

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <glob.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <SDL/SDL.h>
#include <X11/Xlib.h>



#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xff000000
#define GMASK 0x00ff0000
#define BMASK 0x0000ff00
#define AMASK 0x000000ff

#else

#define RMASK 0x000000ff
#define GMASK 0x0000ff00
#define BMASK 0x00ff0000
#define AMASK 0xff000000
#endif




typedef struct {
    unsigned char r,g,b,alpha;
} GaiColor;


#include "aquarium.h"
#include "bottom.h"
#include "bubble.h"
#include "background.h"
#include "fish.h"
#include "draw.h"
#include "soundeffects.h"
#include "vroot.h"

#ifdef DARWIN
#include "getopt.h"
#else
#include <getopt.h>
#endif


#define ARG_SEAFLOOR 1
#define ARG_SOUND 2
#define ARG_PLANTS 3
#define ARG_PLANTSCALE 4
#define ARG_BOTTOMANIMALS 5
#define ARG_OGG 6
#define ARG_BG_SOLID 7
#define ARG_BG_SHADED 8
#define ARG_BG_WATERALIKE 9
#define ARG_BG_IMAGE 10
#define ARG_LCR 11
#define ARG_LCG 12
#define ARG_LCB 13
#define ARG_UCR 14
#define ARG_UCG 15
#define ARG_UCB 16
#define ARG_BG_IMAGE_FILE 17
#define ARG_RANDOM 18
#define ARG_SELECTED 19
#define ARG_RANDOM_POP 20
#define ARG_FISH1 21
#define ARG_FISH2 22
#define ARG_FISH3 23
#define ARG_FISH4 24
#define ARG_FISH5 25
#define ARG_FISH6 26
#define ARG_BLOWFISH 27
#define ARG_SWORDFISH 28
#define ARG_BDWELLER 29
#define ARG_FILLMORE 30
#define ARG_SHERMAN 31
#define ARG_MEGAN 32
#define ARG_HUNTER 33
#define ARG_PREY 34
#define ARG_LORI 35
#define ARG_ERNEST 36
#define ARG_SQUID 37
#define ARG_HAWTHORNE 38
#define ARG_EAT 39
#define ARG_EXPLODE 40
#define ARG_REBIRTH 41
#define ARG_SCALEDIFF 42
#define ARG_SPEED 43
#define ARG_SCALE 44
#define ARG_HUNTERA 45
#define ARG_SWORDA 46
#define ARG_SOUNDPRG 47
#define ARG_WINDOW_ID 48
#define ARG_FULLSCREEN 49
#define ARG_BUBBLE 50
#define ARG_DESKTOP 51
#define ARG_COMICS 52
#define ARG_COMICS_DIR 53
#define ARG_COMICS_DELAY 54
#define ARG_WALLPAPER 55

#define DEPTH 24

static int screen_width;
static int screen_height;
static gboolean comics = FALSE;
static char *comic_dirs[1024]; /* No more than 1024 comic dirs :-) Ugly, but... */
static int num_comic_dirs = 0;

static GdkWindow *window;
static GdkGC *gc;
static SDL_Surface *screen=NULL, *screen_image, *background, **thisfish;
static SDL_Rect *fish_dest, *fish_src, *clean_dest;
static int curr_dest, clean_count,  no_sdl_quit = 0, comics_delay = 50;
int window_id = -1, fullscreen = 0;
int wallpaper = 0;
static unsigned char *original_bg;
static AquariumData ad;



void screensaver_draw_image(int x, int y, int idx, int rev, SA_Image *image)
{

    fish_dest[curr_dest].x=x;
    fish_dest[curr_dest].y=y;
    fish_dest[curr_dest].w=image->width;
    fish_dest[curr_dest].h=image->height;

    fish_src[curr_dest].x=0;
    fish_src[curr_dest].y=(int)((float)idx*(float)image->full_height/(float)image->frames+0.5);
    fish_src[curr_dest].w=image->width;
    fish_src[curr_dest].h=image->height;

    if(!rev)
	thisfish[curr_dest] = SDL_CreateRGBSurfaceFrom(image->image,
						       image->width,
						       (int)((float)image->full_height*(float)(idx+1) / 
							     (float)image->frames + 0.5),
						       32,image->rowstride,
						       RMASK, GMASK, BMASK, AMASK);
    else
	thisfish[curr_dest] = SDL_CreateRGBSurfaceFrom(image->rev,
						       image->width,
						       (int)((float)image->full_height*(float)(idx+1) / 
							     (float)image->frames + 0.5),
						       32,image->rowstride,
						       RMASK, GMASK, BMASK, AMASK);

    curr_dest++;
}




void screensaver_clean(int x,int y,int w,int h)
{

    clean_dest[clean_count].x=x-5;
    clean_dest[clean_count].y=y-5;
    clean_dest[clean_count].w=w+10;
    clean_dest[clean_count].h=h+10;


    SDL_BlitSurface(background,&clean_dest[clean_count],screen,&clean_dest[clean_count]);

    clean_count++;
}


void screensaver_quit()
{
    /* Resetting the term signal to the orignal so we can quit nicely.*/
    signal(SIGTERM, SIG_DFL);

    if(no_sdl_quit)
	kill(getpid(),SIGTERM);
    else
	exit(0);

    /* In case something is really weird */
    kill(getpid(),SIGKILL);
}

void comics_clean(void)
{
    int i;
    for(i=0;i<num_comic_dirs;i++)
	g_free(comic_dirs[i]);
    num_comic_dirs = 0;
    comic_dirs[0] = NULL;
}

void comics_prepare(char *dir)
{
    comic_dirs[num_comic_dirs] = g_strdup_printf("%s/*", dir);
    num_comic_dirs++;

    /* Make sure the final one is always followed by a NULL */
    comic_dirs[num_comic_dirs] = NULL;
}

char *comics_pick(void)
{
    int i, flags = GLOB_NOSORT;
    glob_t comic_files;
    char *the_comic = NULL;


    for(i=0;i<num_comic_dirs;i++){
	if(i)
	    flags |= GLOB_APPEND;
	glob(comic_dirs[i], flags, NULL, &comic_files);
    }

    if(comic_files.gl_pathc != 0)
	the_comic = g_strdup(comic_files.gl_pathv[g_rand_int_range(ad.rnd, 0, comic_files.gl_pathc)]);

    globfree(&comic_files);


    return the_comic;

}


void comics_load(void)
{
    int i,j, srs, ys, xs, alpha, sy, dy;
    unsigned char *src;
    char *comic_file = NULL;
    GError *ferror = NULL;
    GdkPixbuf *comic_pic, *tmp_pic;

    memcpy(ad.rgb, original_bg, ad.xmax*3*ad.ymax);
    memcpy(ad.bgr, original_bg, ad.xmax*3*ad.ymax);

    comic_file = comics_pick();

    if(comic_file == NULL)
	return;

    comic_pic = gdk_pixbuf_new_from_file(comic_file, &ferror);

    g_free(comic_file);

    if(comic_pic == NULL)
	return;

    if(screen_height < gdk_pixbuf_get_height(comic_pic) || screen_width < gdk_pixbuf_get_width(comic_pic)){

	tmp_pic = gdk_pixbuf_scale_simple(comic_pic,screen_width, screen_height, GDK_INTERP_BILINEAR);
	g_object_unref(comic_pic);
	comic_pic = tmp_pic;
    }


    ys = (screen_height - gdk_pixbuf_get_height(comic_pic))/2;
    xs = (screen_width - gdk_pixbuf_get_width(comic_pic))/2;


    srs = gdk_pixbuf_get_rowstride(comic_pic);
    alpha = gdk_pixbuf_get_has_alpha(comic_pic);
    src = gdk_pixbuf_get_pixels(comic_pic);

    for(i=0;i<gdk_pixbuf_get_height(comic_pic);i++){
	sy = i * srs;
	dy = (i+ys) * ad.xmax + xs;
	
	for(j=0;j<gdk_pixbuf_get_width(comic_pic);j++){
	    ad.bgr[(dy+j)*3+0] = ad.rgb[(dy+j)*3+0] = src[sy + j*(3+alpha)+0];
	    ad.bgr[(dy+j)*3+1] = ad.rgb[(dy+j)*3+1] = src[sy + j*(3+alpha)+1];
	    ad.bgr[(dy+j)*3+2] = ad.rgb[(dy+j)*3+2] = src[sy + j*(3+alpha)+2];
	}
    }
    g_object_unref(comic_pic);

}



void screensaver_main_sdl(void)
{
    SDL_Event event;
    clock_t totaltime1;
#ifdef PERFORMACE_CHECK
    clock_t totaltime2;
#endif
    clock_t cali1, cali2;
    int totalframes=0;
    int num_events=0;
    int main_loop=0;
    int frames=0;
    int i;
    int delay=0;
    int counter = 0;


    totaltime1 = clock();
    cali1 = clock();

    while(!main_loop){


	if(counter == 0 && comics){
	    comics_load();
	    SDL_BlitSurface(screen_image, NULL, screen, NULL);
	    SDL_UpdateRect(screen,0,0,0,0);
	    counter = 25*comics_delay;
	}
	if(wallpaper){
	    SDL_UpdateRect(screen,0,0,0,0);
        }
	counter --;


	curr_dest=0;
	clean_count=0;

	fish_update();
	bubble_update();

	for(i=0;i<curr_dest;i++)
	    SDL_BlitSurface(thisfish[i],&fish_src[i],screen,&fish_dest[i]);
	

	/* If we get a SIGTERM from screensaver in this loop, and we later do a SDL_Quit()
	   X will get problems. So we have to avoid calling SDL_Quit if a sigterm is caught
	   in this loop.

	   Otherwise we get:
	   Xlib: unexpected async reply (sequence 0xf03)!
	   And sherman's starts eating processor power like mad!
	*/
	no_sdl_quit = 1;
	for(i=0;i<curr_dest;i++){
	    SDL_UpdateRects(screen,1,&clean_dest[i]);
	    SDL_FreeSurface(thisfish[i]);
	}
	no_sdl_quit = 0;

	if(window_id!=-1)
	    SDL_Flip(screen);

	totalframes++;
	frames++;

	if(frames==10){
	    cali2=clock();

	    /* Check if we're going too fast! */
	    if((float)(cali2-cali1) < (float)(0.2*CLOCKS_PER_SEC)){
		delay=(int)(((float)((0.2*CLOCKS_PER_SEC)-(cali2-cali1))/CLOCKS_PER_SEC)*100000);
	    }
	    else{
		if(delay!=0){
		    if(delay<((int)(((float)((0.2*CLOCKS_PER_SEC)-(cali2-cali1))/CLOCKS_PER_SEC)*100000))){
			delay-=(int)(((float)((0.2*CLOCKS_PER_SEC)-(cali2-cali1))/CLOCKS_PER_SEC)*100000);
		    }
		    else
			delay=0;
		}
	    }
	    cali1=clock();
	    frames=0;
	}
	usleep(delay);
	

	while(SDL_PollEvent(&event)){
	    switch(event.type){
	    case SDL_QUIT:
		main_loop=1;
		break;
	    case SDL_KEYDOWN:
		if(!wallpaper){
		  num_events++;
		  if(num_events==2)
		      main_loop=1;
		  break;
		}
	    case SDL_MOUSEMOTION:
		if(!wallpaper){
		  num_events++;
		  if(num_events==2)
		      main_loop=1;
		  break;
		}
	    case  SDL_MOUSEBUTTONDOWN:
		if(!wallpaper){
		  num_events++;
		  if(num_events==2)
		    main_loop=1;
		  break;
		}
	    }
	}    
    }

#ifdef PERFORMACE_CHECK
    totaltime2=clock();
    printf("Frames: %d\n",totalframes);
    printf("Seconds: %f\n",(float)(totaltime2-totaltime1)/CLOCKS_PER_SEC);
    printf("FPS: %f\n",(float)totalframes/((float)(totaltime2-totaltime1)/CLOCKS_PER_SEC));
#endif

}

void screensaver_main_gdk(void)
{
    GdkEvent *event;
    int counter = 0;

    while(1){
	while(gdk_events_pending()){
	    event = gdk_event_get();
	    if(event){
		if(event->type == GDK_DESTROY)
		    exit(0);
	    }
	}

	if(counter == 0 && comics){
	    comics_load();
	    counter = 25*comics_delay;
	}
	counter --;

	memcpy(ad.rgb, ad.bgr, ad.ymax * ad.xmax * 3);
	fish_update();
	bubble_update();
	gdk_draw_rgb_image(window, gc, ad.xmin, ad.ymin, ad.xmax, ad.ymax,
			   GDK_RGB_DITHER_NONE, ad.rgb, ad.xmax * 3);
	gdk_flush();
	/* 25 fps */
	usleep(1000000/25);
    }

}

void init_sdl(int sdl_flags)
{

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
	printf("Can't init SDL: %s\n",SDL_GetError());
	exit(1);
    }


    signal(SIGTERM, screensaver_quit);
    atexit(SDL_Quit);
    

    if(!SDL_VideoModeOK(screen_width,screen_height, DEPTH, sdl_flags)){
	printf("Sorry, video mode %dx%d in %d bits isn't supported by hardware\n",
	       screen_width,screen_height, DEPTH);
	exit(2);
    }


    screen = SDL_SetVideoMode(screen_width, screen_height, DEPTH, sdl_flags);

    if(screen == NULL){
	printf("Unable to set video mode %dx%d in %d bits.\n",
	       screen_width,screen_height,DEPTH);
	exit(3);
    }

    SDL_WM_SetCaption("Sherman's aquarium",NULL);


    /* Hide the mouse cursor */
    if (!wallpaper){
    SDL_ShowCursor(0);
    }
    /* Start with all black */
    SDL_FillRect(screen,NULL,0x000000);

    screen_image = SDL_CreateRGBSurfaceFrom(ad.rgb, ad.xmax, ad.ymax, DEPTH, ad.xmax*3, 
					    RMASK, GMASK, BMASK, 0);
    background = SDL_CreateRGBSurfaceFrom(ad.bgr, ad.xmax, ad.ymax, DEPTH, ad.xmax*3, 
					  RMASK, GMASK, BMASK, 0);


    SDL_BlitSurface(screen_image, NULL, screen, NULL);
    SDL_UpdateRect(screen,0,0,0,0);

}


void screensaver_init()
{
    char *sdl_command;
    XWindowAttributes win_attr;
    Display *display;
    Fish_settings *fish_settings;
    Bubble_settings *bubble_settings;
    int sdl_flags = SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_ANYFORMAT;

    screen_height = 480;
    screen_width = 640;
    
    if(window_id != -1){
	display = XOpenDisplay(NULL);
	XGetWindowAttributes(display, (Window)window_id, &win_attr);
	screen_height = win_attr.height;
	screen_width = win_attr.width;
	fullscreen = 0;
	ad.proximity = 1;	 		/* No sound effects */
	window = gdk_window_foreign_new((Window)window_id);
	gdk_window_show(window);
	gc = gdk_gc_new(window);
    }

    if(fullscreen || wallpaper){
	screen_width = gdk_screen_width();
	screen_height = gdk_screen_height();

	display=XOpenDisplay(NULL);
	sdl_command = g_strdup_printf("SDL_WINDOWID=%d",
				      (int)RootWindowOfScreen(ScreenOfDisplay(display, DefaultScreen(display))));

	putenv(sdl_command);
	ad.proximity = 0;
	sdl_flags |= SDL_FULLSCREEN;
    }

    ad.xmax = screen_width;
    ad.ymax = screen_height;
  
    ad.virtual_aquarium_x = ad.xmax + 2 * VIRTUAL_AQUARIUM_DX;
    ad.virtual_aquarium_y = ad.ymax + 2 * VIRTUAL_AQUARIUM_DY;

    ad.ymin = ad.xmin = ad.viewpoint_start_x = ad.viewpoint_start_y = 0;

    ad.rgb = g_malloc0(ad.xmax*3*ad.ymax);
    ad.bgr = g_malloc0(ad.xmax*3*ad.ymax);
    original_bg = g_malloc0(ad.xmax*3*ad.ymax);

    background_init();
    fish_init();
    fish_turn();
    bottom_init();
    bubble_init();

    memcpy(ad.rgb, ad.bgr, ad.xmax*3*ad.ymax);
    memcpy(original_bg, ad.bgr, ad.xmax*3*ad.ymax);


    if(fullscreen || wallpaper || window_id == -1)
	init_sdl(sdl_flags);


    fish_settings = fish_get_settings_ptr();
    bubble_settings = bubble_get_settings_ptr();

    fish_dest = g_malloc0(sizeof(SDL_Rect)*(fish_settings->num_fish + 
					    bubble_settings->max_bubbles));
    fish_src = g_malloc0(sizeof(SDL_Rect)*(fish_settings->num_fish +
					   bubble_settings->max_bubbles));
    clean_dest = g_malloc0(sizeof(SDL_Rect)*(fish_settings->num_fish  + 
					     bubble_settings->max_bubbles));

    thisfish = g_malloc0(sizeof(SDL_Surface*)*(fish_settings->num_fish + bubble_settings->max_bubbles));
    
}

int htoi(char *str)
{
  int i,sum=0,d;

  if(str[0]!='0' || str[1]!='x') return -1;
  for(i=2;i<strlen(str);i++){
    d=0;
    if(str[i]>='0' && str[i]<='9') d=(int)(str[i]-'0');
    if(str[i]>='A' && str[i]<='F') d=(int)(str[i]-'A'+10);
    if(str[i]>='a' && str[i]<='f') d=(int)(str[i]-'a'+10);

    sum+=d;
    sum=sum<<4;
  }

  return(sum>>4);
}

void screensaver_init_param(int argc, char **argv)
{
    int i,c, numfish = 0, comic_start;
    char *comic_buff;

    Sound_settings *s, sound_settings = {0,TYPE_MP3, NULL};
    Bubble_settings *bub, bubble_settings = {20};
    Bottom_settings *b, bottom_settings = {0,5,1,75,2};
    Background_settings *bg, background_settings = {NULL,NULL, 0,1, 
						    (GaiColor){0, 100, 150, 0},
						    (GaiColor){10,120, 250, 0},
						    (GaiColor){0,0,0,0}};
    Fish_settings *f, fish_settings = {0, 0, 75, 0, 100, 0, 15, 0, 75, 75,
				       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    struct option cmdline_options[] = {
	{"sound", no_argument, NULL, ARG_SOUND},
	{"soundprg", required_argument, NULL, ARG_SOUNDPRG},
	{"ogg", no_argument, NULL, ARG_OGG},


	{"seafloor", no_argument, NULL, ARG_SEAFLOOR},
	{"plants", required_argument, NULL, ARG_PLANTS},
	{"plantscale", required_argument, NULL, ARG_PLANTSCALE},
	{"bottomanimals", required_argument, NULL, ARG_BOTTOMANIMALS},

	{"bg_solid", no_argument, NULL, ARG_BG_SOLID},
	{"bg_shaded", no_argument, NULL, ARG_BG_SHADED},
	{"bg_wateralike", no_argument, NULL, ARG_BG_WATERALIKE},
	{"bg_image", no_argument, NULL, ARG_BG_IMAGE},


	{"lcr", required_argument, NULL, ARG_LCR},
	{"lcg", required_argument, NULL, ARG_LCG},
	{"lcb", required_argument, NULL, ARG_LCB},

	{"ucr", required_argument, NULL, ARG_UCR},
	{"ucg", required_argument, NULL, ARG_UCG},
	{"ucb", required_argument, NULL, ARG_UCB},
	
	{"bg_image_file", required_argument, NULL, ARG_BG_IMAGE_FILE},

	{"comics", no_argument, NULL, ARG_COMICS},
	{"comics_dir", required_argument, NULL, ARG_COMICS_DIR},
	{"comics_delay", required_argument, NULL, ARG_COMICS_DELAY},


	{"random", no_argument, NULL, ARG_RANDOM},
	{"selected", no_argument, NULL, ARG_SELECTED},
	{"random_pop", required_argument, NULL, ARG_RANDOM_POP},

	{"fish1", required_argument, NULL, ARG_FISH1},
	{"fish2", required_argument, NULL, ARG_FISH2},
	{"fish3", required_argument, NULL, ARG_FISH3},
	{"fish4", required_argument, NULL, ARG_FISH4},
	{"fish5", required_argument, NULL, ARG_FISH5},
	{"fish6", required_argument, NULL, ARG_FISH6},
	{"swordfish", required_argument, NULL, ARG_SWORDFISH},
	{"blowfish", required_argument, NULL, ARG_BLOWFISH},
	{"bdweller", required_argument, NULL, ARG_BDWELLER},
	{"fillmore", required_argument, NULL, ARG_FILLMORE},
	{"sherman", required_argument, NULL, ARG_SHERMAN},
	{"megan", required_argument, NULL, ARG_MEGAN},
	{"hunter", required_argument, NULL, ARG_HUNTER},
	{"prey", required_argument, NULL, ARG_PREY},
	{"lori", required_argument, NULL, ARG_LORI},
	{"ernest", required_argument, NULL, ARG_ERNEST},
	{"squid", required_argument, NULL, ARG_SQUID},
	{"hawthorne", required_argument, NULL, ARG_HAWTHORNE},

	{"eat", no_argument, NULL, ARG_EAT},
	{"explode", no_argument, NULL, ARG_EXPLODE},
	{"rebirth", no_argument, NULL, ARG_REBIRTH},
	{"scalediff", no_argument, NULL, ARG_SCALEDIFF},
	{"speed", required_argument, NULL, ARG_SPEED},
	{"scale", required_argument, NULL, ARG_SCALE},
	{"huntera", required_argument, NULL, ARG_HUNTERA},
	{"sworda", required_argument, NULL, ARG_SWORDA},
	{"window-id", required_argument, NULL, ARG_WINDOW_ID},
	{"root", no_argument, NULL, ARG_FULLSCREEN},
	{"wallpaper", no_argument, NULL, ARG_WALLPAPER},
	{"bubble", required_argument, NULL, ARG_BUBBLE},
	{"desktop", no_argument, NULL, ARG_DESKTOP},
	{0,0,0,0}};


    b = bottom_get_settings_ptr();
    bub = bubble_get_settings_ptr();
    bg = background_get_settings_ptr();
    f = fish_get_settings_ptr();
    s = sound_get_settings_ptr();

    memcpy(f,&fish_settings, sizeof(Fish_settings));
    memcpy(bg,&background_settings, sizeof(Background_settings));
    memcpy(b,&bottom_settings, sizeof(Bottom_settings));
    memcpy(bub, &bubble_settings, sizeof(Bubble_settings));
    memcpy(s, &sound_settings, sizeof(Sound_settings));

    while ((c =
	    getopt_long_only(argc, argv, "", cmdline_options, NULL)) != -1){

	switch(c){

	case ARG_SEAFLOOR:
	    b->have_sea_floor = 1;
	    break;
	case ARG_SOUND:
	    s->on = 1;
	    break;
	case ARG_PLANTS:
	    b->max_plants = atoi(optarg);
	    break;
	case ARG_PLANTSCALE:
	    b->scale = atoi(optarg);
	    break;
	case ARG_BOTTOMANIMALS:
	    b->num_bottom_animals = atoi(optarg);
	    break;
	case ARG_OGG:
	    s->type = TYPE_OGG;
	    break;
	case ARG_BG_SOLID:
	    bg->type = BG_SOLID;
	    bg->desktop = 0;
	    break;
	case ARG_BG_SHADED:
	    bg->type = BG_SHADED;
	    bg->desktop = 0;
	    break;
	case ARG_BG_WATERALIKE:
	    bg->type = BG_WATER;
	    bg->desktop = 0;
	    break;
	case ARG_BG_IMAGE:
	    bg->type = BG_IMAGE;
	    bg->desktop = 0;
	    break;
	case ARG_LCR:
	    bg->solid_c.r = bg->shaded_bot_c.r = atoi(optarg);
	    break;
	case ARG_LCG:
	    bg->solid_c.g = bg->shaded_bot_c.g = atoi(optarg);
	    break;
	case ARG_LCB:
	    bg->solid_c.b = bg->shaded_bot_c.b = atoi(optarg);
	    break;
	case ARG_UCR:
	    bg->shaded_top_c.r = atoi(optarg);
	    break;
	case ARG_UCG:
	    bg->shaded_top_c.g = atoi(optarg);
	    break;
	case ARG_UCB:
	    bg->shaded_top_c.b = atoi(optarg);
	    break;
	case ARG_BG_IMAGE_FILE:
	    bg->imagename = g_strdup_printf(optarg);
	    break;
	case ARG_RANDOM:
	    f->type = RANDOM_FISH;
	    break;
	case ARG_SELECTED:
	    f->type = SELECTION_FISH;
	    break;
	case ARG_RANDOM_POP:
	    f->num_fish = atoi(optarg);
	    break;
	case ARG_FISH1:
	    f->fish1 = atoi(optarg);
	    break;
	case ARG_FISH2:
	    f->fish2 = atoi(optarg);
	    break;
	case ARG_FISH3:
	    f->fish3 = atoi(optarg);
	    break;
	case ARG_FISH4:
	    f->fish4 = atoi(optarg);
	    break;
	case ARG_FISH5:
	    f->fish5 = atoi(optarg);
	    break;
	case ARG_FISH6:
	    f->fish6 = atoi(optarg);
	    break;
	case ARG_BLOWFISH:
	    f->blowfish = atoi(optarg);
	    break;
	case ARG_SWORDFISH:
	    f->swordfish = atoi(optarg);
	    break;
	case ARG_BDWELLER:
	    f->bdweller = atoi(optarg);
	    break;
	case ARG_FILLMORE:
	    f->fillmore = atoi(optarg);
	    break;
	case ARG_SHERMAN:
	    f->sherman = atoi(optarg);
	    break;
	case ARG_MEGAN:
	    f->megan = atoi(optarg);
	    break;
	case ARG_HUNTER:
	    f->hunter = atoi(optarg);
	    break;
	case ARG_PREY:
	    f->prey = atoi(optarg);
	    break;
	case ARG_LORI:
	    f->lori = atoi(optarg);
	    break;
	case ARG_ERNEST:
	    f->ernest = atoi(optarg);
	    break;
	case ARG_SQUID:
	    f->squid = atoi(optarg);
	    break;
	case ARG_HAWTHORNE:
	    f->hawthorne = atoi(optarg);
	    break;
	case ARG_EAT:
	    f->eat = 1;
	    break;
	case ARG_EXPLODE:
	    f->explode = 1;
	    break;
	case ARG_REBIRTH:
	    f->rebirth = 1;
	    break;
	case ARG_SCALEDIFF:
	    f->scale_diff = 1;
	    break;
	case ARG_SPEED:
	    f->speed = atoi(optarg);
	    break;
	case ARG_SCALE:
	    f->scale = atoi(optarg);
	case ARG_HUNTERA:
	    f->hunter_agr = atoi(optarg);
	    break;
	case ARG_SWORDA:
	    f->swordfish_agr = atoi(optarg);
	    break;
	case ARG_SOUNDPRG:
	    s->prg = g_strdup_printf(optarg);
	    break;
	case ARG_WINDOW_ID:
	    window_id = htoi(optarg);
	    break;
	case ARG_FULLSCREEN:
	    fullscreen = 1;
	    break;
	case ARG_WALLPAPER:
	    wallpaper = 1;
	    //bg->type = BG_WATER;
	    //bg->type = BG_IMAGE;
	    bg->type = BG_SHADED;
	    //bg->type = BG_SOLID;
	    bg->desktop = 0;
	    b->have_sea_floor = 1;
            b->num_bottom_animals = 7;
	    b->max_plants = 23;
	    break;
	case ARG_BUBBLE:
	    bub->max_bubbles = atoi(optarg);
	    break;
	case ARG_DESKTOP:
	    bg->type = 0;
	    bg->desktop = 1;
	    break;
	case ARG_COMICS:
	    comics = TRUE;
	    comics_prepare(IMAGE_PATH "/strips");
	    break;
	case ARG_COMICS_DELAY:
	    comics_delay = atoi(optarg);
	    break;
	case ARG_COMICS_DIR:
	    comics_clean();
	    comic_start = 0;
	    comic_buff = g_strdup(optarg);
	    for(i=0;i<strlen(optarg);i++){
		if(comic_buff[i] == ';'){
		    comic_buff[i] = '\0';
		    comics_prepare(comic_buff+comic_start);
		    comic_start = i+1;
		}
	    }
	    comics_prepare(comic_buff+comic_start);
	    g_free(comic_buff);

	    break;
     	default:
	    break;
	}
    }

    numfish += f->fish1;
    numfish += f->fish2;
    numfish += f->fish3;
    numfish += f->fish4;
    numfish += f->fish5;
    numfish += f->fish6;
    numfish += f->swordfish;
    numfish += f->blowfish;
    numfish += f->fillmore;
    numfish += f->sherman;
    numfish += f->prey;
    numfish += f->hunter;
    numfish += f->lori;
    numfish += f->ernest;
    numfish += f->squid;
    numfish += f->megan;
    numfish += f->bdweller;
    numfish += f->hawthorne;

    if(f->type == SELECTION_FISH)
	f->num_fish = numfish;

}

int main(int argc, char **argv)
{
    gdk_init(&argc, &argv);

    ad.rnd = g_rand_new();

    screensaver_init_param(argc, argv);
    screensaver_init(0);

    if(window_id==-1)
	screensaver_main_sdl();
    else
	screensaver_main_gdk();
    return 0;
}


AquariumData *aquarium_get_settings_ptr(void)
{
    return &ad;
}

unsigned char *aquarium_install_path(void)
{
    return IMAGE_PATH;
}


void aquarium_draw_image(int x, int y, int idx, int rev, SA_Image *image)
{
    if(window_id==-1)
	screensaver_draw_image(x, y, idx, rev, image);
    else
	draw_image(x, y, idx, rev, image);
}

void aquarium_draw_pic_alpha(SA_Image *image, int w, int h, int x, int y, int idx, int alpha)
{
    if(window_id==-1)
	screensaver_draw_image(x, y, idx, 0, image);
    else
	draw_pic_alpha(image->image, w, h, x, y, idx,alpha);
} 


void aquarium_clean_image(int x, int y, int w, int h)
{
    if(window_id==-1)
	screensaver_clean(x, y, w, h);
}

GdkPixbuf *gai_load_image(char *fname)
{
    GError *msg = NULL;
    GdkPixbuf *pix;
    char *full_name;

    full_name = g_strdup_printf("%s/%s",IMAGE_PATH,fname);
    pix = gdk_pixbuf_new_from_file(full_name, &msg);
    if(!pix){
	printf("%s\n",msg->message);
	exit(1);
    }
    g_free(full_name);

    return pix;
    
}
void gai_display_error_continue(char *msg)
{
    printf(" *** Error: %s\n",msg);
}
