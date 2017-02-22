#include "calendar.h"

int  get_datetype(int day){return datetype[day][0];}

/*------------------------------------------------------
 *   checkicalversion
 -----------------------------------------------------*/
void checkicalversion()
{
    GtkWidget* dialog;
    GtkWidget* label;
    char* msg = "\n\nWARNING:\nIt is highly recommended to upgrade to libical 0.24!\nOtherwise wmCalendar will not work stable!\n";
    if(!strcmp(ICAL_VERSION, "0.23")){
	printf("%s",msg);
     	dialog = gtk_dialog_new_with_buttons ("Warning",
					      NULL,
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_NONE,
					      NULL);
	label = gtk_label_new (msg);
	g_signal_connect_swapped (GTK_OBJECT (dialog),
				  "response",
				  G_CALLBACK (gtk_widget_destroy),
				  GTK_OBJECT (dialog));
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
                        GTK_SIGNAL_FUNC(gtk_main_quit ),
                        NULL);
 	gtk_widget_show_all (dialog);
	gtk_main();
    }
}


void move(GtkWidget *widget, GdkEventButton *event)
{
    xr = event->x;
    yr = event->y;
}



void move2(GtkWidget *widget, GdkEventMotion *event)
{
    gtk_widget_set_uposition((GtkWidget*)gtk_widget_get_toplevel(widget),event->x_root - xr , event->y_root - yr);
}



void widget_kill (GtkWidget * widget)
{
    gtk_widget_destroy ((GtkWidget*)gtk_widget_get_toplevel(widget));
    gtk_main_quit ();
}



/*------------------------------------------------------
 *   calendar
 -----------------------------------------------------*/
void calendar(){
    int value;
    char* line;
    const char* text;
    const char* transp="";
    struct stat filestat;
    icalproperty *prop, *reocc;
    icalparser *parser;
    icalcomponent *c, *d;
    FILE *stream;
    struct icaltimetype t1, t2;
    if(get_debug())printf("check for new calendar data\n");
    stream = fopen((const char*)get_icsfile(),"r");
    if(stream == 0)
	return;
    fstat(fileno(stream), &filestat);
    if(filestat.st_mtime == modtime){
	fclose(stream);
	return;
    }
    if(get_debug())printf("read calendar data\n");
    deleteCalObjs();
    modtime = filestat.st_mtime;
    parser = icalparser_new();
    /* Tell the parser what input routie it should use. */
    icalparser_set_gen_data(parser, stream);
    do{
	line = icalparser_get_line(parser, read_stream);
	c = icalparser_add_line(parser, line);
	free(line);
	if(c != 0){
	    for(d = icalcomponent_get_first_component(c, ICAL_ANY_COMPONENT);d != 0;
		d = icalcomponent_get_next_component(c, ICAL_ANY_COMPONENT)){

		/* get date */
		t1 = icalcomponent_get_dtstart(d);
		t2 = icalcomponent_get_dtend(d);
		if(icaltime_is_null_time(t2)){
		    t2 = t1;
		    icaltime_adjust(&t2, 1,0,0,0);
		}

		/* get transparency */
		prop = icalcomponent_get_first_property(d, ICAL_TRANSP_PROPERTY );
		reocc = icalcomponent_get_first_property(d, ICAL_RRULE_PROPERTY);
		if(prop)
		    transp = icalproperty_get_value_as_string(prop);
		if(!strcmp(transp, "OPAQUE"))
		    value = 1;
		else if(!strcmp(transp, "TRANSPARENT"))
		    value = 2;
		else
		    value = 1 ;


		/* get desciption */
		prop = icalcomponent_get_first_property(d,     ICAL_SUMMARY_PROPERTY);
		if(prop) {
		    text = icalproperty_get_value_as_string(prop);
		    addCalObj(t1, t2, value, text, d);
		    if(get_debug())printf("read: %d.%d.%d - %d.%d.%d %s\n", t1.day, t1.month,
				    t1.year, t2.day, t2.month, t2.year, text);
	       }
		icalcomponent_free(d);
	    }
	    icalcomponent_free(c);
	}
    } while(line != 0);
    icalparser_free(parser);
    fclose(stream);
}



/*------------------------------------------------------
 *   showDay
 -----------------------------------------------------*/
void showDay(struct icaltimetype dt){
    static GtkWidget *dayView;
    static GtkWidget *table;
    static GtkWidget *label1;
    static GtkWidget *event_box;
    static GtkWidget *event_box2;
    char buf[28];
    char buf2[28];
    struct tm *timptr = NULL;
    time_t tt = icaltime_as_timet(dt);
    timptr = gmtime(&tt);
    event_box = gtk_event_box_new ();
    gtk_widget_show (event_box);
    event_box2 = gtk_event_box_new ();
    gtk_widget_show (event_box2);

    /* create titlebartext */
    strftime(buf, 26, "%A, %x", timptr);
    sprintf(buf2, "<b>%s</b>",buf);
    /* Create a new window with no boarder and day as titlebar*/
    dayView = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // xr =yr =0;
    gtk_window_set_position(GTK_WINDOW (dayView),GTK_WIN_POS_MOUSE);
    gtk_window_set_policy(GTK_WINDOW (dayView), FALSE, FALSE, FALSE);
    gtk_window_set_decorated (GTK_WINDOW (dayView), FALSE);
    gtk_window_set_title(GTK_WINDOW (dayView), buf);
    /* create a table */
    table = gtk_table_new(1, 4, FALSE);
    gtk_table_set_row_spacings ((GtkTable*)table, 4);
    gtk_table_set_col_spacings ((GtkTable*)table, 15);
    gtk_table_set_col_spacing ((GtkTable*)table,2, 0);

    gtk_table_attach_defaults (GTK_TABLE(table), event_box, 0, 3, 1, 2);
    gtk_table_attach_defaults (GTK_TABLE(table), event_box2, 3, 4, 1, 2);

    label1 = gtk_label_new (NULL);
    gtk_label_set_markup ((GtkLabel*)label1, buf2);
    gtk_container_add (GTK_CONTAINER (event_box), label1);

    gtk_table_set_row_spacing ((GtkTable*)table,0, 1);
    gtk_table_set_row_spacing ((GtkTable*)table,1, 1);
    gtk_widget_show (label1);
    label1 = gtk_label_new (" X ");
    gtk_container_add (GTK_CONTAINER (event_box2), label1);
    gtk_widget_show (label1);

    /* fill table with events and draw window if there are any events */
    if(dayevents(dt, table)){
	gtk_widget_show (dayView);

	gtk_widget_show (table);
	gtk_container_add (GTK_CONTAINER (dayView), table);
	gtk_signal_connect(GTK_OBJECT (event_box), "motion_notify_event",
			   GTK_SIGNAL_FUNC (move2), NULL);
	gtk_signal_connect(GTK_OBJECT (event_box), "button_press_event",
			   GTK_SIGNAL_FUNC (move), NULL);
	gtk_signal_connect(GTK_OBJECT (event_box2), "button_press_event",
			   (GtkSignalFunc) widget_kill, GTK_OBJECT(dayView));

	gtk_widget_realize(dayView);
	//	gtk_window_set_position(GTK_WINDOW (dayView),GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_main ();
    }
}



/*------------------------------------------------------
 *   dayevents
 -----------------------------------------------------*/
int dayevents(struct icaltimetype dt, GtkWidget *table){
    static GtkWidget   *label1;
    struct calobj* it;
    struct icaltimetype t1, t2;
    icalproperty *prop;
    int j; /* tablerow */
    char buftime1[30];
    char buftime2[30];
    char buf[1024];
    struct tm *timptr = NULL;
    time_t tt;

    GtkWidget *separator;
    j = 2;
    it = calRoot;
    while(it){
	if(eventOnDay(dt, it)){
	    t1 = it->start;
	    t2 = it->end;
	    separator = gtk_hseparator_new ();
	    gtk_widget_show (separator);
	    gtk_table_attach_defaults (GTK_TABLE(table),separator,  0, 4, j, j+1);
	    j++;
	    if(daysEqual(t1, t2)){ /* single event */
		/* create time entry */
		tt = icaltime_as_timet(t1);
		timptr = gmtime(&tt);
		strftime(buftime1, 26, "%X", timptr);
		tt = icaltime_as_timet(t2);
		timptr = gmtime(&tt);
		strftime(buftime2, 26, "%X", timptr);
		sprintf(buf,"%s - %s",buftime1, buftime2);
		label1 = gtk_label_new (NULL);
		gtk_label_set_markup ((GtkLabel*)label1, buf);
		gtk_table_attach_defaults (GTK_TABLE(table), label1, 1, 2, j, j+1);
		gtk_widget_show (label1);


		/* create description entry */
		label1 = gtk_label_new (it->text);
		gtk_table_attach_defaults (GTK_TABLE(table), label1, 2, 3, j, j+1);
		gtk_widget_show (label1);
		j++;
	    }
	    else {
		/* multiday event or allday event*/
		t2.day--; /* endtime always on the first day after the event ... */
		if(t2.day == 0){
		    t2.month--;
		    if(t2.month == 0){
			t2.month = 12;
			t2.year--;
		    }
		    t2.day = icaltime_days_in_month(t2.month, t2.year);
		}
		if(t2.day < dt.day && t2.month == dt.month && t2.year == dt.year)
		    continue; /* event ended the day before ... */
		if(daysEqual(t1,t2)){
		    /* allday event */
		    /* create description entry */
		    sprintf(buf, "<i>all day event</i>");
		}
		else{ /* multiday event */
		    /* create description with start and enddate */
		    tt = icaltime_as_timet(t1);
		    timptr = gmtime(&tt);
		    strftime(buftime1, 26, "%a, %x", timptr);
		    tt = icaltime_as_timet(t2);
		    timptr = gmtime(&tt);
		    strftime(buftime2, 26, "%a, %x", timptr);
		    sprintf(buf, "%s - %s", buftime1, buftime2);
		}
		label1 = gtk_label_new (NULL);
		gtk_label_set_markup ((GtkLabel*)label1, buf);

		gtk_table_attach_defaults (GTK_TABLE(table), label1, 1, 2, j, j+1);
		gtk_widget_show (label1);

		prop = icalcomponent_get_first_property(it->comp, ICAL_LOCATION_PROPERTY);
		if(prop)
		    sprintf(buf, "%s\n%s", it->text, icalproperty_get_location(prop));
		else
		    sprintf(buf, "%s", it->text);
		label1 = gtk_label_new (buf);
		gtk_label_set_justify(GTK_LABEL(label1),GTK_JUSTIFY_CENTER );
		gtk_table_attach_defaults (GTK_TABLE(table), label1, 2, 3, j, j+1);
		gtk_widget_show (label1);
		j++;
	    }
	}
	it = it->next;
    }
    label1 = gtk_label_new (NULL);
    gtk_table_attach_defaults (GTK_TABLE(table), label1, 2, 3, j+1, j+2);
    gtk_table_set_row_spacing (GTK_TABLE(table),j, 0);
    if(j==2) /* no entries for this day */
	return FALSE;
    return TRUE;
}



/*------------------------------------------------------
 *   deleteCalObjs
 -----------------------------------------------------*/
void deleteCalObjs(){
    int i;
    struct calObj* help;
    while(calRoot){
	help = (struct calObj*) calRoot->next;
	free(calRoot->text);
	free(calRoot->comp);
	free(calRoot);
	calRoot = help;
    }
    for(i = 0; i < 32; i++)
	datetype[i][1] = 0;
}



/*------------------------------------------------------
 *  addCalObj
 -----------------------------------------------------*/
void addCalObj(struct icaltimetype start, struct icaltimetype end,
	       int type, const char *text, icalcomponent * d){
    struct calobj *newobj;
    icalcomponent *newcomp = malloc(sizeof(struct calobj));
    char* buf = malloc (strlen(text) + 1);
    newcomp =  icalcomponent_new_clone(d);
    strcpy(buf,text);
    newobj = malloc(sizeof(struct calobj));
    newobj->comp = newcomp;
    newobj->start = start;
    newobj->end = end;
    newobj->type = type;
    newobj->text = buf;
    newobj->next = calRoot;
    calRoot = newobj;
}



/*------------------------------------------------------
 *   getDayType
 -----------------------------------------------------*/
int  getDayType(struct icaltimetype dt){
    int jd;
    jd = civil_jdn(dt);
    if(datetype[jd % 31][1] == jd)
	return datetype[jd % 31][0];

    return calcDayType(dt);
}



/*------------------------------------------------------
 *   isExluded
 -----------------------------------------------------*/
int isExluded(icalcomponent *comp, struct icaltimetype dt){
    icalproperty *prop;
    prop = icalcomponent_get_first_property(comp, ICAL_EXDATE_PROPERTY);
    while(prop){
	if(daysEqual(icalproperty_get_exdate(prop), dt))
	    return 1;
	prop = icalcomponent_get_next_property(comp, ICAL_EXDATE_PROPERTY);
    }
    return 0;
}



/*------------------------------------------------------
 *   eventOnDay
 -----------------------------------------------------*/
int eventOnDay(struct icaltimetype dt, struct calobj* it)
{
    icalrecur_iterator* ritr;
    icalproperty *rrule;
    struct icaltimetype next;
    if((daysEarlierEqual(dt, it->start)	&& daysLater(dt, it->end))
       || (daysEqual(dt, it->start) && daysEqual(dt, it->end)))
	return 1;
    rrule = icalcomponent_get_first_property((icalcomponent*)it->comp, ICAL_RRULE_PROPERTY);

    if(rrule){
	if(daysEarlierEqual(dt, it->start)){
	    ritr = icalrecur_iterator_new( icalproperty_get_rrule(rrule), it->start);
	    if(ritr)
		next = icalrecur_iterator_next(ritr);
	    while(daysEarlierEqual(dt, next) && !icaltime_is_null_time(next)){
		if(daysEqual(dt, next) && !isExluded((icalcomponent*)it->comp, dt))
		    {
			free(ritr);
			return 1;
		    }
		next = icalrecur_iterator_next(ritr);
	    }
	    free(ritr);
	}
    }
    return 0;
}



/*------------------------------------------------------
 *   calcDayType
 -----------------------------------------------------*/
int calcDayType(struct icaltimetype dt){
    int jd;
    struct calobj* it;
    jd = civil_jdn(dt);
    datetype[jd % 31][1] = jd;
    datetype[jd % 31][0] = 0;
    it = calRoot;
    while(it){
	if(eventOnDay(dt, it))
	    if(datetype[jd % 31][0] != 1)
		datetype[jd % 31][0] = it->type;
	it = it->next;
    }
    return datetype[jd % 31][0];
}
