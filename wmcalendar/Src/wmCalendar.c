/*------------------------------------------------------
 *
 *  	wmCalendar (c)2003 Matthias Laabs
 *
 *       mattlaabs@users.sourceforge.net
 *
 *  		a calendar dockapp
 -----------------------------------------------------*/

#include "wmCalendar.h"

/*------------------------------------------------------
 *   main
 -----------------------------------------------------*/
int main(int argc, char *argv[])
{
    int n = 0;
    getSettings();
    ParseCMDLine(argc, argv);
    if(get_debug())printf("debugging mode:\nstarting wmCalendar version %s\n", getVersion());
    if(get_debug())printf("using libical version %s\n", ICAL_VERSION);
    dockapp_open_window("", "wmCalendar", 64, 64, argc, argv);
    initValues();
    gtk_init(&argc, &argv);
    checkicalversion();

    /*  Loop until we die */
    while(1) {
	if(n == 0){ /* do updates only every COUNTERth time */
	        getTime();
		calendar();
		draw();

	    n = COUNTER;
	}
	/* check for events every time */
	n--;
	if(processXEvents())
	    n = 0;
    }
}




/*------------------------------------------------------
 *  draw
 -----------------------------------------------------*/
void draw()
{

blankScreen();
    drawButtons();
    drawDays();
    drawMonthYear();
}



/*------------------------------------------------------
 *  initValues
 -----------------------------------------------------*/
void initValues(){
    XClassHint  class_hints;
    XpmColorSymbol colors[2];
    int i, j;
    monthOffset = 0;
    class_hints.res_name="wmCalendar";
    class_hints.res_class="wmCalendar";
    /* Compute widths of months and digits */
    for(i = 0; i < 12; ++i)
	for( j = 0; j < 6; ++j)
	    xdMonth[j][i] = xeMonth[j][i] - xsMonth[j][i] + 1;
    for(i = 0; i < 12; ++i)
	for( j = 0; j < 2; ++j)
	    xdYear[j][i] = xeYear[j][i] - xsYear[j][i] + 1;
    for(i = 0; i < MAXBUTTON; ++i)
	xdButton[i] = xeButton[i] - xsButton[i] + 1;

    dockapp_set_eventmask();
    dockapp_xpm2pixmap( wmCalendar_master2_xpm, &bg[1], &mask, colors, 0);
    dockapp_xpm2pixmap( wmCalendar_master_xpm, &bg[0], &mask, colors, 0);
    pixmap = dockapp_XCreatePixmap(64, 64);
    dockapp_setshape(mask, 0, 0);
    if(mask)
	XFreePixmap(display, mask);
    dockapp_set_background(pixmap);
    dockapp_show();
}



/*------------------------------------------------------
 *  read_stream
 -----------------------------------------------------*/
char* read_stream(char *s, size_t size, void *d){
  char *c = fgets(s, size, (FILE*)d);
  return c;
}


/*------------------------------------------------------
 *   getClickedDay
 -----------------------------------------------------*/
int getClickedDay(int bx, int by)
{
    int dayofweek, week, day;
    dayofweek = bx / 9 + 1;
    week = (by - 20) / 7;
    tgr = get_civil(t, calendartype);
    day = dayofweek + 7 * week - ((day_of_week(tgr) - t.day + 37 + get_start_of_week()) % 7);
    if(day <= days_in_month(t.month, t.year, calendartype) && day > 0){
	tgr.day = day;
	tgr.month = t.month;
	tgr.year = t.year;
	tgr = get_civil(tgr, calendartype);
	return 1;
    }
    return 0;
}



/*------------------------------------------------------
 *   getClickedButton
 -----------------------------------------------------*/
int getClickedButton(int bx, int by)
{
    if(bx < 12 && by < 12) /* arrow left : month back*/
	return BT_BACK;
    else if(bx > 50 && by < 12) /* arrow right : month forward*/
	return BT_FORWARD;
    else if(bx > 52 && by > 56) /* show/hide buttons */
	return BT_OPENCLOSE;
    else if(showbuttons){
	if(bx < 46 && bx > 37 && by > 55) /* toggle gregorian/persian/... */
	    return BT_MOON ;
	else if(bx < 52 && bx > 46 && by > 55) /* open settings dialog */
	    return BT_SETTINGS;
	else if(bx < 37 && bx > 28 && by > 55) /* toggle moonphase on/off */
	    return BT_CALTYPE;
	else if(bx < 28 && bx > 19 && by > 55)  /* start calendar application */
	    return BT_APP;
    }
    return 0;
}



/*------------------------------------------------------
 *   ButtonPressEvent
 -----------------------------------------------------*/
void buttonPress(int btype, int bx, int by, long etime){
    char* applaunch;
    if(get_debug() > 1) printf("push button %d\n", btype);

    switch(btype){
    case 1: /* leftmouse - button or mark */
	switch ( getClickedButton(bx, by)){
	case BT_BACK: /* arrow left : month back*/
	    monthOffset--;
	    break;

	case BT_FORWARD: /* arrow right : month forward*/
	    monthOffset++;
	    break;

	case BT_OPENCLOSE: /* show/hide buttons */
	    showbuttons = 1 - showbuttons;
	    break;

	case BT_CALTYPE: /* toggle gregorian/persian/... */
	    calendartype = (calendartype + 1) % CALTYPES;
	    break;

	case BT_MOON:/* toggle moonphase on/off */
	    showmoon = 1 - showmoon ;
	    break;

	case BT_SETTINGS: /* open settings dialog */
	    openSettings();
	    break;

	case BT_APP: /* start calendar application */
	    if(appDoubleClick && (etime - appClickTime < DBLCLKTIME)){
		applaunch = malloc(250);
		strcpy(applaunch, (const char*)get_application());
		strcat(applaunch, " &");
		system(applaunch);
		appDoubleClick = 0;
		free(applaunch);
	    }
	    else{
		appDoubleClick = 1;
		appClickTime = etime;
		}
	    break;

	default: /* no button clicked check for marking days*/
	    if(getClickedDay(bx, by)){
		if(!icaltime_is_null_time(mark))
		    mark = icaltime_null_time();
		else
		    mark = tgr;
	    }
	}
	break;

    case 4: /* mousewheel up : month back*/
	monthOffset--;
	break;

    case 5: /* mousewheel down : month forward*/
	monthOffset++;
	break;

    case 2: /* Middlemouse click : go to actual month*/
	monthOffset = 0;
	break;

    case 3: /* rightmouse click : show day entries*/
	if(getClickedDay(bx, by))
	    showDay(tgr);
	break;
    }

    told.month = 0; /* invalid month -> forces clearScreen at next draw*/
}



/*------------------------------------------------------
 *   ParseCMDLine
 -----------------------------------------------------*/
void ParseCMDLine(int argc, char *argv[]){
  int  i;

  for (i = 1; i < argc; i++){
    if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v")){
      printf("wmCalendar %s\n", getVersion());
      exit(1);
    }
    else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")){
      print_usage();
      exit(1);
    }
    else if (!strcmp(argv[i], "--farsi") || !strcmp(argv[i], "-f"))
	set_lang(1);
    else if (!strcmp(argv[i], "--persian") || !strcmp(argv[i], "-p"))
	calendartype = 1;
    else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "-d"))
	set_debug( 1);
    else{
      print_usage();
      exit(1);
    }
  }
}



/*------------------------------------------------------
 *   print_usage
 -----------------------------------------------------*/
void print_usage()
{
  printf("wmCalendar %s (c) 2003 Matthias Laabs (mattlaabs@users.sourceforge.net)\n", getVersion());
  printf("This is free software under GPL!\n");
  printf("\nOptions:");
  printf("\n -h  --help\n   Show summary of options.");
  printf("\n -v  --version\n   Show version of program.");
  printf("\n -d  --debug\n   Show debugging info.");
  printf("\n -f  --farsi\n   Show months and numbers in farsi.");
  printf("\n -p  --persian\n   Changes to persian calendar.");
  printf("\n\nUsage: \nLeft and right arrow or mousewheel change month.\n");
  printf("Middle mouseclick goes back to current month.\n");
  printf("Right mouseclick opens dayview.\nClick on Moon to toggle moonphase-view on/off\n");
  printf("Click on screwdriver to open settings menu\nClick on 'G' to toggle between Gregorian, Persian and Islamic calendar\n");
  printf("Doubleclick evolution/mozilla icon to start application\nClick on blue arrow to show/hide buttons\n");
  printf("Leftclick a day to highlight for converting between calendars\n");
}



/*------------------------------------------------------
 *   getTime
 -----------------------------------------------------*/
void getTime(){
  struct tm	*Time;
  long	        CurrentLocalTime;

  /* get time */
  CurrentLocalTime = time(CurrentTime);
  Time = localtime(&CurrentLocalTime);
  t.day = Time->tm_mday;
  t.month = Time->tm_mon + 1;
  t.year = Time->tm_year + 1900;
  today = t;

  /* change calendar*/
  if(calendartype == 1)
      t = civil_persian(t);
  else if(calendartype == 2)
      t = civil_islamic(t);
  /* set t to currently shown month and year */
  t.month += monthOffset;
  while(t.month > 12){
    t.month -= 12;
    t.year += 1;
  }
  while(t.month < 1){
    t.month += 12;
    t.year -= 1;
  }
}



/*------------------------------------------------------
 *   drawMonthYear
 -----------------------------------------------------*/
void drawMonthYear(){
  int xoff, yoff;
  int Year1, Year2, Year3, Year4;
  int monthset;
  yoff = 1;
  monthset = calendartype + get_lang() * CALTYPES;
  /* calculate digits of the year */
  Year4 = t.year % 10;
  Year3 = ((t.year - Year4) / 10) % 10;
  Year2 = ((t.year - Year4 - 10 * Year3) / 100) % 10;
  Year1 = (t.year - Year4 - 10 * Year3 - 100 * Year2) / 1000;

  xoff = 30 - (xdMonth[monthset][t.month-1] + xdYear[get_lang()][Year3] + xdYear[get_lang()][Year4]) / 2;

  /* draw Month */
  dockapp_copyarea(bg[1], pixmap, xsMonth[monthset][t.month - 1], yMonth[monthset],
			   xdMonth[monthset][t.month - 1], ydMonth, xoff, yoff);
  xoff += xdMonth[monthset][t.month - 1] + 3;

  /* draw Year 3rd and 4th digit */
  dockapp_copyarea(bg[1], pixmap,xsYear[get_lang()][Year3], yYear, xdYear[get_lang()][Year3], ydYear, xoff, yoff);
  xoff += xdYear[get_lang()][Year3];
  dockapp_copyarea(bg[1], pixmap,xsYear[get_lang()][Year4], yYear, xdYear[get_lang()][Year4], ydYear, xoff, yoff);
  dockapp_copy2window(pixmap);
}



/*------------------------------------------------------
 *   drawButtons
 -----------------------------------------------------*/
void drawButtons(){
  int xoff, yoff;
  yoff = 56;
  xoff = 56;
  if(showbuttons){
      xoff = drawButton(ARROW_CLOSE, xoff, yoff);
      xoff = drawButton(SETTINGS, xoff, yoff);
      xoff = drawButton(MOON + showmoon, xoff, yoff);
      xoff = drawButton(CALENDAR + calendartype, xoff, yoff);
      xoff = drawButton(APPLAUNCH + getAppicon(), xoff, yoff);
  }
  else
      drawButton(ARROW_OPEN, xoff, yoff);

  dockapp_copy2window(pixmap);
}



/*------------------------------------------------------
 *   drawButton
 -----------------------------------------------------*/
int drawButton(int btype, int xoff, int yoff){
    xoff-=  xdButton[btype];
    dockapp_copyarea(bg[0], pixmap, xsButton[btype], yButton[btype], xdButton[btype],
		     ydButton, xoff, yoff);
    return --xoff;
}



/*------------------------------------------------------
 *   drawDays
 -----------------------------------------------------*/
void drawDays()
{
    int i, dayOfWeek;
    int amount, startday;
    int moonphase = 0;
    int yoff = 20;

    tgr = t;
    tgr.day = 1;

    tgr = get_civil(tgr, calendartype);
    amount = days_in_month(t.month, t.year, calendartype);
    startday = (day_of_week(tgr) + get_start_of_week()) % 7;
    for(i = 1; i <= amount; i++){
	dayOfWeek = (startday + i) % 7;
	if((dayOfWeek == 0) && (i != 1))
	    yoff += 7; /* increase y-offset on Monday except it is day #1*/

	tgr = t;
	tgr.day = i;
	tgr = get_civil(tgr, calendartype);

	if(showmoon)
	    moonphase = moon(tgr);

	if(daysEqual(tgr, mark))
	    drawNumber(i, yoff, dayOfWeek, 3, 0);
	else if(moonphase == 0 || !showmoon) /* draw number */
	    drawNumber(i, yoff, dayOfWeek, getDayType(tgr), daysEqual(tgr, today));
	else /* draw moonphase */
	    dockapp_copyarea(bg[0], pixmap, xsMoon[moonphase - 1], yMoon, xdMoon, ydMoon,
			     dayOfWeek * 9, yoff);
    }

    dockapp_copy2window(pixmap);
}



/*------------------------------------------------------
 *   drawNumber
 -----------------------------------------------------*/
void drawNumber(int number,int yoff, int dayOfWeek, int type, int today){
    int digit1, digit2;
    int offset;
    offset = yNumbers + 20 * type + 10 * today;
    if(number < 10){ /* draw single digit numbers*/
	dockapp_copyarea(bg[0], pixmap, xeNumbers[get_lang()][1] - 5, offset, 2, ydNumbers,
			 dayOfWeek * 9, yoff);
	dockapp_copyarea(bg[0], pixmap, xeNumbers[get_lang()][1] - 5, offset, 2, ydNumbers,
			 dayOfWeek * 9 + 7, yoff);
	dockapp_copyarea(bg[0], pixmap, xeNumbers[get_lang()][number], offset, xdNumbers, ydNumbers,
			 dayOfWeek * 9 + 2, yoff);
    }
    else{ /* draw double digit numbers*/
	digit2 = number % 10;
	digit1 = (number - digit2) / 10;
	dockapp_copyarea(bg[0], pixmap, xeNumbers[get_lang()][digit1], offset, xdNumbers, ydNumbers,
			 dayOfWeek * 9, yoff);
	dockapp_copyarea(bg[0], pixmap, xeNumbers[get_lang()][digit2], offset, xdNumbers, ydNumbers,
			 dayOfWeek * 9 + 4, yoff);
    }
}



/*------------------------------------------------------
 *   processXEvents
 -----------------------------------------------------*/
int processXEvents()
{
    if(dockapp_nextevent_or_timeout(&event, 150)) {
	/* Next Event */
	switch (event.type) {
	case ButtonPress:
	    buttonPress(event.xbutton.button,event.xbutton.x, event.xbutton.y, event.xbutton.time);
	    return 1;
	default: break;
	}
    }
    return 0;
}



/*------------------------------------------------------
 *   blankScreen
 -----------------------------------------------------*/
void blankScreen()
{
    if(daysEqual(t, told))
	return;
    /* only neccessary if day has changed*/
    dockapp_copyarea(bg[0], pixmap, 0, 0, 64, 64, 0, 0);
    dockapp_copyarea(bg[0], pixmap, 9 * (7 - get_start_of_week()),70 + get_lang() * 12, 63, 8, 0, 12);
    dockapp_copy2window(pixmap);
    calendar(); /* reread calendardata */
    told = t;
}
