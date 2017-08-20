#include "main.h"
#include "age_calc.h"
#include "arg_parser.h"
#include <signal.h>

static int	offset_w, offset_h;
static char name[200];
static char *file;
static int day, month, year;
static Pixmap *background_pixmap;

void free_resources() {
  XFreeGC(display, gc_core);
  XFreeGC(display, gc_border);
}

void sig_int() {
  free_resources();
  printf("\nThank you for using wmframepic\n");
  exit(1);

}

void draw_screen_with_mask() {
  XNextEvent(display, &event);

  while(event.type != Expose)
    ;

  XCopyArea(display, *background_pixmap, iconwin, gc_border, 0, 0, 64, 64, offset_w, offset_h);
  XCopyArea(display, pic_pixmap, iconwin, gc_core, 0, 0, 64, 64, offset_w, offset_h);
  XFlush(display);

}

void mainloop() {
  XButtonPressedEvent *bevent;
  for (;;) {
    XNextEvent(display, &event);
    switch (event.type) {
    case Expose:
      XCopyArea(display, *background_pixmap, iconwin, gc_border, 0, 0, 64, 64, offset_w, offset_h);
      XCopyArea(display, pic_pixmap, iconwin, gc_core, 0, 0, 64, 64, offset_w, offset_h);
      break;
    case ButtonPress:			
      bevent = (XButtonPressedEvent *) &event;
      switch (bevent->button & 0xff) {
      case Button1:
      case Button2:
      case Button3:
	if(show_pic) {
	  XCopyArea(display, pic_pixmap, iconwin, gc_core, 0, 0, 64, 64, offset_w, offset_h);
	  show_pic = False;
	} else {
	  restore_background();
	  draw_text(name, 3, 8, False);
	  char **phrases = get_phrases(day, month, year);
	  int i;
	  for(i = 0; i < NUMBER_OF_ROWS; i++) {
	    draw_text(phrases[i], 3, 23 + 10 * i, False);

	  }
	  clear_phrases(phrases);
	  flush_background();
	  show_pic = True;
	}
	
	break;
      default:
	break;
      }
      break;
    default:
      break;
    }
  }
}

int main(int argc, char **argv) {
  int		    ww, wh, w, h;
  w = SIZE;
  h = SIZE;
  Bool	dockapp_iswindowed = False;
  int error;
  XWMHints	*wmHints;
  XClassHint	*classHint;

  if(signal(SIGINT, sig_int) == SIG_ERR)
    fprintf(stderr, "signal error\n");

  error = get_values_from_command_line(argc, argv, name, &file, &day, &month, &year);

  if(COMMAND_LINE_FAIL == error || COMMAND_LINE_HELP == error) {
    return 1;
  }


  //_Xdebug = 1;
  if( ( display = XOpenDisplay(NULL) ) == NULL ){
    fprintf(stderr, "Error: XOpenDisplay\n");
    exit(1);
  }

  root = RootWindow(display, screen = DefaultScreen(display));
  XGCValues values;
  values.foreground = BlackPixel(display, screen);
  values.background = WhitePixel(display, screen);



  Pixmap pixmask_border = XCreateBitmapFromData(display, root, frame_mask_bits,
						frame_mask_width,
						frame_mask_height);

  gc_border = XCreateGC(display, root, GCForeground | GCBackground, &values);
  XSetClipMask(display, gc_border, pixmask_border);

  Pixmap pixmask_core = XCreateBitmapFromData(display, root, core_mask_bits,
					      core_mask_width,
					      core_mask_height);


  gc_core = XCreateGC(display, root, GCForeground | GCBackground, &values);
  XSetClipMask(display, gc_core, pixmask_core);

  XFreePixmap(display, pixmask_border);
  XFreePixmap(display, pixmask_core);

  if (dockapp_iswindowed) {
    offset_w = (WINDOWED_SIZE_W - w) / 2;
    offset_h = (WINDOWED_SIZE_H - h) / 2;
    ww = WINDOWED_SIZE_W;
    wh = WINDOWED_SIZE_H;
  } else {
    offset_w = offset_h = 0;
    ww = w;
    wh = h;
  }

  sizehints.flags = USSize | USPosition;
  if (!dockapp_iswindowed) {
    sizehints.flags |= USPosition;
    sizehints.x = sizehints.y = 0;
  } else {
    sizehints.flags |= PMinSize | PMaxSize;
    sizehints.min_width = sizehints.max_width = WINDOWED_SIZE_W;
    sizehints.min_height = sizehints.max_height = WINDOWED_SIZE_H;
  }
  sizehints.width = ww;
  sizehints.height = wh;

  win = XCreateSimpleWindow(display, root, 0, 0, ww, wh, depth, values.foreground,
			    values.background);

  iconwin = XCreateSimpleWindow(display, root, 0, 0, ww, wh, depth, values.foreground,
				values.background);



  XSetWMNormalHints(display, win, &sizehints);
  
  wmHints = XAllocWMHints();
  if (wmHints == NULL) {
    fprintf(stderr, "%s: can't allocate memory for wm hints!\n", argv[0]);
    exit(1);
  }

  wmHints->initial_state = WithdrawnState;
  wmHints->icon_window = iconwin;
  wmHints->icon_x = sizehints.x;
  wmHints->icon_y = sizehints.y;
  wmHints->window_group = win;
  wmHints->flags = StateHint|IconWindowHint|IconPositionHint|WindowGroupHint;
  XSetWMHints(display, win, wmHints);
  XFree(wmHints);

  classHint = XAllocClassHint();
  if (classHint == NULL) {
    fprintf(stderr, "%s: can't allocate memory for wm hints!\n", argv[0]);
    exit(1);
  }

  classHint->res_name = argv[0];
  classHint->res_class = argv[0];
  XSetClassHint(display, win, classHint);
  XFree(classHint);
  // Let the window manager know about the link of commands
  XSetCommand(display, win, argv, argc);
  
  //XpmCreatePixmapFromData(display, root, kid_xpm, &pic_pixmap,NULL,NULL);
  char **xpm_file_data;
  if(-1 == XpmReadFileToData(file, &xpm_file_data)) {
    printf("could not read from file %s\nplease check file path\n", file);
    return 1;
  }

  XpmCreatePixmapFromData(display, root, xpm_file_data, &pic_pixmap,NULL,NULL);
  XSelectInput(display, win, eventmask);
  XSelectInput(display,iconwin, eventmask);
  XMapWindow(display, win);
  
  init_variables(display, gc_core, iconwin, offset_w, offset_h, &background_pixmap);

  draw_screen_with_mask();
  show_pic = False;
  
  mainloop();
  return 0;
}
