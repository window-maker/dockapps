#include "settings.h"
void set_lang(int language){lang = language;}
void setAppicon(int app){appicon = app;}
void enter_callback( GtkWidget *widget, GtkWidget *entry ){
    strcpy(application, gtk_entry_get_text(GTK_ENTRY(entry)));}
int getAppicon(){return appicon;}
int get_start_of_week(){return start_of_week;}
int get_lang(){return lang;}
int get_debug(){return debug;}
void set_debug(int deb){debug = deb;}
char* getVersion(){return WMCALENDAR_VERSION;}
const char* get_application(){return application;}
const char* get_icsfile(){return icsfile;}

/*------------------------------------------------------
 *   destroy
 -----------------------------------------------------*/
void destroy (GtkWidget * widget, gpointer data){
  gtk_main_quit ();
}



/*------------------------------------------------------
 *   getSettings
 -----------------------------------------------------*/
void getSettings(){
    char buf[15];
    struct tm *timptr = malloc(sizeof(struct tm));
    FILE *stream;
    int lSize;
    char *set = malloc(4096);
    char *pch;
    int i;
    int next = 0;
    int version = 0;
    strcpy (rcfile, (char *) getenv ("HOME"));
    strcat(rcfile, "/.wmcalendarrc");
    stream = fopen(rcfile,"r");

    for(i = 1; i < 8; i++){
	timptr->tm_wday = (8-i)%7;
	strftime(buf, 15, "%A", timptr);
	daystr[i]=malloc(15);
	strcpy(daystr[i], buf);
    }
    free(timptr);
    if(stream == 0){ /* no rcfile */
	writeSettings();
	printf("created %s with default settings:\nICON:evolution\nLANG:default\nAPP:%s\nICS:%s\n\n",
	       rcfile, application, icsfile );
	return;
    }
   else{
	fseek (stream , 0 , SEEK_END);
	lSize = ftell (stream);
	rewind (stream);
	fread (set,1,lSize,stream);

	pch = strtok (set,"\n=");
	while (pch != NULL)
	    {
		switch(next){
		case 1:
		    strcpy(icsfile, pch);
		    next = 0;
		    break;

		case 2:
		    strcpy(application, pch);
		    next = 0;
		    break;

		case 3:
		    if(!strcmp(pch, "farsi"))
		       lang = 1;
		    next = 0;
		    break;

		case 4:
		    if(!strcmp(pch, "mozilla"))
		       appicon = 1;
		    else if(!strcmp(pch, "evolution"))
		       appicon = 0;
		    else
		       appicon = 2;
		    next = 0;
		    break;

		case 5:
		    if(!strcmp(pch,WMCALENDAR_VERSION))
			version = 1;
		    next = 0;
		    break;

		case 6:
		    start_of_week =  atoi(pch);
		    next = 0;
		    break;

		default:
		    if(!strcmp(pch, "ICS"))
			next = 1;
		    else if(!strcmp(pch, "APP"))
			next = 2;
		    else if(!strcmp(pch, "LANG"))
			next = 3;
		    else if(!strcmp(pch, "ICON"))
			next = 4;
		    else if(!strcmp(pch, "VERSION"))
			next = 5;
		    else if(!strcmp(pch, "STARTOFWEEK"))
			next = 6;
		    else
			next = 0;
		}
		pch = strtok (NULL, "=\n");
	    }
    }
    fclose(stream);
    free(set);
    if(!version){
	writeSettings();
	printf("Old rcfile detected!\nCreated new %s with default settings:", rcfile);
	printf("\nICON:evolution\nLANG:default\nAPP:%s\nICS:%s\n\n", application, icsfile );
    }
    if(get_debug())printf("settings:\nICON:%d\nLANG:%d\nAPP:%s\nRC:%s\nICS:%s\n\n",
		    getAppicon(), lang, application, rcfile, icsfile );
}



/*------------------------------------------------------
 *   writeSettings
 -----------------------------------------------------*/
void writeSettings(){
    char *set = malloc(1024);
    char *bufint = malloc(5);
    FILE *stream;
    if(debug)printf("write settings\n");
    stream = fopen(rcfile,"w");
    if(strlen(icsfile)==0){
	strcpy(icsfile, (char *) getenv ("HOME"));
	strcat(icsfile , "/evolution/local/Calendar/calendar.ics");
    }
    if(strlen(application)==0)
	strcpy(application , "evolution");
    strcpy(set, "ICS=");
    strcat(set, icsfile);
    strcat(set, "\nAPP=");
    strcat(set, application);
    if(appicon == 0)
	strcat(set, "\nICON=evolution");
    if(appicon == 1)
	strcat(set, "\nICON=mozilla");
    if(appicon == 2)
	strcat(set, "\nICON=other");
    if(lang == 1)
	strcat(set, "\nLANG=farsi");
    else
	strcat(set, "\nLANG=default");
    strcat(set,"\nVERSION=");
    strcat(set,WMCALENDAR_VERSION);
    strcat(set,"\nSTARTOFWEEK=");
    sprintf(bufint, "%d",start_of_week);
    strcat(set, bufint);
    strcat(set,"\n");
    fwrite(set, 1, strlen(set), stream);
    fclose(stream);
    free(set);
    free(bufint);
}



/*------------------------------------------------------
 *   SettingsSeperator
 -----------------------------------------------------*/
void SettingsSeperator(GtkWidget *box1){
    GtkWidget *separator;
    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (box1), separator, FALSE, TRUE, 5);
    gtk_widget_show (separator);

}



/*------------------------------------------------------
 *   SettingsLabel
 -----------------------------------------------------*/
void SettingsLabel(GtkWidget *box1, char* str){
    GtkWidget *label;
    label = gtk_label_new (str);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_box_pack_start (GTK_BOX (box1), label, TRUE, TRUE, 3);
    gtk_widget_show (label);

}



/*------------------------------------------------------
 *   openSettings
 -----------------------------------------------------*/
void openSettings(){
    GtkWidget *window;
    GtkWidget *box1;
    GtkWidget *box2;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *menu;
    GtkWidget *optionmenu;
    GtkWidget *menuitem;
    GSList    *group;
    int ii;
    char title[30];
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_policy(GTK_WINDOW (window), FALSE, FALSE, FALSE);

    gtk_signal_connect (GTK_OBJECT (window), "destroy",
                        GTK_SIGNAL_FUNC(destroy),
                        NULL);
    sprintf(title, "wmCalendar %s", WMCALENDAR_VERSION);
    gtk_window_set_title (GTK_WINDOW (window), title);
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);

    box1 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), box1);
    gtk_widget_show (box1);

    /* --------------  Applaunch icon  --------------------------*/
    SettingsLabel(box1, "Applaunch icon");

    box2 = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (box2), 0);
    gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 3);
    gtk_widget_show (box2);

    /* create appicon boxes*/
    button = gtk_radio_button_new_with_label (NULL, "Evolution");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(setAppicon),
                               0);
    if(appicon == 0)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
    button = gtk_radio_button_new_with_label(group, "Mozilla");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(setAppicon),
                               (gpointer)1);
    if(appicon == 1)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    button = gtk_radio_button_new_with_label(
                 gtk_radio_button_group (GTK_RADIO_BUTTON (button)),
                 "Other");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(setAppicon),
                               (gpointer)2);
    if(appicon == 2)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    SettingsSeperator(box1);

    /* --------------  Applaunch command  -----------------------*/
    SettingsLabel(box1, "Applaunch command");

    entry = gtk_entry_new_with_max_length (100);
    gtk_signal_connect(GTK_OBJECT(entry), "changed",
                       GTK_SIGNAL_FUNC(enter_callback),
                       entry);
    gtk_entry_set_text (GTK_ENTRY (entry), application);
    gtk_box_pack_start (GTK_BOX (box1), entry, TRUE, TRUE, 3);
    gtk_widget_show (entry);

    SettingsSeperator(box1);

    /* --------------  Language  -------------------------------*/
    SettingsLabel(box1, "Language");

    /* create Language boxes */
    box2 = gtk_hbox_new (FALSE, 10);
    gtk_container_set_border_width (GTK_CONTAINER (box2), 0);
    gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 3);
    gtk_widget_show (box2);

    button = gtk_radio_button_new_with_label (NULL, "default");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(set_lang),
                               0);
    if(lang == 0)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    group = gtk_radio_button_group (GTK_RADIO_BUTTON (button));
    button = gtk_radio_button_new_with_label(group, "farsi");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(set_lang),
                               (gpointer)1);
    if(lang == 1)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
    gtk_widget_show (button);

    SettingsSeperator(box1);

    /* --------------  first day of week  --------------------------*/
    SettingsLabel(box1, "Week starts");
    optionmenu = gtk_option_menu_new();
    menu = gtk_menu_new();
    for(ii = 7; ii > 0; ii--){
	menuitem = gtk_menu_item_new_with_label(daystr[ii]);
	gtk_widget_show (menuitem);
	gtk_menu_append(menu, menuitem);
    }
    gtk_menu_set_active(GTK_MENU (menu), 7 - start_of_week);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu),menu);
    gtk_box_pack_start (GTK_BOX (box1), optionmenu, TRUE, TRUE, 5);
    gtk_widget_show (optionmenu);

    SettingsSeperator(box1);


    /* --------------  Buttons  -------------------------------*/
    button = gtk_button_new_with_label ("change iCalendar file");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                               GTK_SIGNAL_FUNC(changeFilename),
                               NULL);
    gtk_box_pack_start (GTK_BOX (box1), button, TRUE, TRUE, 2);
    gtk_widget_show (button);

    button = gtk_button_new_with_label ("save");
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                              (GtkSignalFunc) setFirstDay,
			      GTK_OPTION_MENU (optionmenu));
    gtk_box_pack_start (GTK_BOX (box1), button, TRUE, FALSE, 2);
    gtk_widget_show (button);

    button = gtk_button_new_with_label ("close");
    gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                              (GtkSignalFunc) gtk_widget_destroy,
                               GTK_OBJECT (window));
    gtk_box_pack_start (GTK_BOX (box1), button, TRUE, TRUE, 2);
    gtk_widget_show (button);
    gtk_widget_show (window);
    gtk_main();
}



/*------------------------------------------------------
 *   setFirstDay
 -----------------------------------------------------*/
void setFirstDay(GtkWidget *widget, GtkWidget *optionmenu ){
    gchar *daystring;
    int ii;
    gtk_label_get (GTK_LABEL (GTK_BIN (optionmenu)->child), &daystring);
    for(ii = 1; ii < 8; ii++)
	if(!strcmp(daystring, daystr[ii]))
	    start_of_week = ii;

    writeSettings();
}



/*------------------------------------------------------
 *   changeFilename
 -----------------------------------------------------*/
void changeFilename(){
    static GtkWidget *settings;

    settings = gtk_file_selection_new("iCalendar file");

    gtk_file_selection_set_filename (GTK_FILE_SELECTION(settings),
				     icsfile);
    /* Connect the ok_button to file_ok_sel function */
    gtk_signal_connect(GTK_OBJECT (GTK_FILE_SELECTION (settings)->ok_button),
		      "clicked",(GtkSignalFunc) file_ok_sel, (gpointer) settings);
    gtk_signal_connect(GTK_OBJECT (settings), "destroy",
		       GTK_SIGNAL_FUNC (destroy), NULL);
    gtk_signal_connect_object(GTK_OBJECT (GTK_FILE_SELECTION(settings)->cancel_button),
			      "clicked", (GtkSignalFunc) gtk_widget_destroy, GTK_OBJECT (settings));

    gtk_window_position (GTK_WINDOW(settings), GTK_WIN_POS_MOUSE );
    gtk_widget_show (settings);
    gtk_main ();
}



/*------------------------------------------------------
 *   file_ok_sel
 -----------------------------------------------------*/
static void file_ok_sel( GtkWidget        *w,
                         GtkFileSelection *fs )
{
    strcpy(icsfile, gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));
    gtk_widget_destroy((GtkWidget*)fs);
    writeSettings();
}
