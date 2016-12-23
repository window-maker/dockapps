#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <X11/xpm.h>
#include <gtk/gtk.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"
#include "wdryer.xpm"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define VERSION "1.1"

/* function headers */
int Read_Config_File(char* filename);
void DrawStatus(int intProcNum, int intStatus);
void DecrementTimer ();
void ExecAction (char chrType);
void displayUsage ();
int configure_washerdryer ();
void callback (GtkWidget * widget, gpointer data);
int delete_event (GtkWidget * widget, GdkEvent * event, gpointer data);
void destroy (GtkWidget * widget, gpointer data);
void destroyAndReloadConfig(GtkWidget* widget);

/* globals */
//I put these Widgets here to grab
//text in event handler (callback)
static GtkWidget *wEntry;
static GtkWidget *dEntry;
static GtkWidget *txtNewTime;
static GtkWidget *mainConfigWindow;

char cmdDry[256];
char *commandDry = cmdDry;
char cmdWash[256];
char *commandWash = cmdWash;
char wdryer_mask_bits[64 * 64];
int wdryer_mask_width = 64;
int wdryer_mask_height = 64;

int wInCommandMode = 0;      // 0 = default/bell     1 = command
int dInCommandMode = 0;

int tmp_wInCmdMode = 0;
int tmp_dInCmdMode = 0;
int oldsec = 0;

//starting times of washer and dryer
int intDefaultWasherTime = 0;
int intDefaultDryerTime = 0;
int intStartingTimes[10] = {0,0,0,0,0,0,0,0,0,0};

//washer and dryer processes
int intMin[10]   = {0,0,0,0,0,0,0,0,0,0};
int intSec[10]   = {0,0,0,0,0,0,0,0,0,0};
char chrType[10] = {'w','w','w','w','w',
                    'w','w','w','w','w'};
//for drawing status
int intPrevStatus[10] = {0,0,0,0,0,0,0,0,0,0};


/*************
* main method
*************/
int main (int argc, char *argv[])
{
  char config_file[256] = {'\0'};
  int intFreeChk;
  int intFullCount = 0;
  int i, intTemp;
  XEvent Event;
  int buttonStatus = -1;
  long starttime;
  long curtime;
  struct tm *time_struct;
  struct tm old_time_struct;

  //gtk_init (&argc, &argv);

  commandDry[0] = '\0';
  commandWash[0] = '\0';

  //parse command line
  for (i = 1; i < argc; i++)
  {
    char *arg = argv[i];
    if (*arg == '-')
    {
      switch (arg[1])
      {
        case 'd':
          if(argc > (i+1))
          {
            strncpy(commandDry, argv[i + 1], 256);
            ++i;
            dInCommandMode = 1;
          }
          break;
        case 'w':
          if(argc > (i+1))
          {
            strncpy (commandWash, argv[i + 1], 256);
            ++i;
            wInCommandMode = 1;
          }
          break;
        case 'W':
          if(argc > (i+1))
          {
              intDefaultWasherTime = atoi(argv[i+1]);
              for(intTemp = 0; intTemp < 10; ++intTemp)
                  intStartingTimes[intTemp] = atoi(argv[i+1]);
              ++i;
          }
          break;
        case 'D':
          if(argc > (i+1))
          {
              intDefaultDryerTime = atoi(argv[i+1]);
              ++i;
          }
          break;
        case 'v':
          printf ("%s\n", VERSION);
          _exit (0);
          break;
        case 'c':
          if(argc > (i+1))
          {
            strncpy(config_file, argv[i+1], 256);
            ++i;
          }
          break;
        default:
          fprintf(stderr, "Unrecognized option: %s\n", arg);
          displayUsage ();
          _exit (0);
          break;
      }
    }
    else
    {
        fprintf(stderr, "Unrecognized option: %s\n", arg);
        displayUsage ();
    }
  }

  //read config file
  if(config_file[0] != 0)
  {
    //fprintf(stderr, "Using user-specified config file '%s'.\n", config_file);
    Read_Config_File(config_file);
  }
  else
  {
    sprintf(config_file, "%s/.wdryerrc", getenv("HOME"));
    Read_Config_File(config_file);
  }

  //insure sanity
  if(intDefaultWasherTime < 1)
    intDefaultWasherTime = 1;
  if(intDefaultDryerTime < 1)
    intDefaultDryerTime = 1;
  for(intTemp = 0; intTemp<10; ++intTemp)
  {
      if(intStartingTimes[intTemp] < 1)
        intStartingTimes[intTemp] = 1;
  }


  gtk_init (&argc, &argv);
  createXBMfromXPM (wdryer_mask_bits, wdryer_xpm, wdryer_mask_width,
        wdryer_mask_height);

  openXwindow (argc, argv, wdryer_xpm, wdryer_mask_bits, wdryer_mask_width,
         wdryer_mask_height);

  //setMaskXY(-64, 0);

  //clickable regions
  AddMouseRegion (0, 8, 6, 28, 24); //0: washer
  AddMouseRegion (1, 32, 6, 52, 24); //1: dryer
  AddMouseRegion (2, 6, 26, 57, 57); //2: main area

  starttime = time (0);
  curtime = time (0);

  time_struct = localtime (&curtime);

  /*****************************
  * while program is running...
  *****************************/
  while (1)
  {
    curtime = time (0);

    waitpid (0, NULL, WNOHANG);

    old_time_struct = *time_struct;
    time_struct = localtime (&curtime);

    if (curtime >= starttime)
    {
        //decrement timer, draw status
        if (oldsec < time_struct->tm_sec)
          DecrementTimer ();
        oldsec = (time_struct->tm_sec);

        RedrawWindow ();

        // X Events
        while (XPending (display))
        {
            XNextEvent (display, &Event);
            switch (Event.type)
            {
                case Expose:
                    RedrawWindow ();
                    break;
                case DestroyNotify:
                    XCloseDisplay (display);
                    _exit (0);
                    break;
                case ButtonPress:
                    i = CheckMouseRegion (Event.xbutton.x, Event.xbutton.y);
                    buttonStatus = i;
                    if (buttonStatus == i && buttonStatus >= 0)
                    {
                        switch (buttonStatus)
                        {
                            /*****************
                            * click on washer
                            *****************/
                            case 0:
                              intFullCount = 0;
                              for(intFreeChk=0;intFreeChk<10;++intFreeChk)
                              {
                                if(  (intSec[intFreeChk] == 0) &&
                                  (intMin[intFreeChk] == 0) &&
                                  (intFullCount < 5))
                                {
                                  intMin[intFreeChk] = intDefaultWasherTime;
                                  intStartingTimes[intFreeChk] = intDefaultWasherTime;
                                  chrType[intFreeChk] = 'w';
                                  break;
                                }
                                if(chrType[intFreeChk] == 'w')
                                  ++intFullCount;
                              }
                              break;
                            /****************
                            * click on dryer
                            ****************/
                            case 1:
                              intFullCount = 0;
                              for(intFreeChk=0;intFreeChk<10;++intFreeChk)
                              {
                                if((intSec[intFreeChk] == 0) &&
                                  (intMin[intFreeChk] == 0) &&
                                  (intFullCount < 5))
                                {
                                  intMin[intFreeChk] = intDefaultDryerTime;
                                  intStartingTimes[intFreeChk] = intDefaultDryerTime;
                                  chrType[intFreeChk] = 'd';
                                  break;
                                }
                                if(chrType[intFreeChk] == 'd')
                                  ++intFullCount;
                              }
                              break;
                            /***************
                            * click on main
                            ***************/
                            case 2:  // main area
                                //open GUI
                                configure_washerdryer ();
                                break;
                        } //end switch
                    } //end if

                    break;

                case ButtonRelease:
                    i = CheckMouseRegion (Event.xbutton.x, Event.xbutton.y);

                    if (buttonStatus == i && buttonStatus >= 0)
                    {
                        switch (buttonStatus)
                        {
                            case 0:
                                break;
                            case 1:
                                break;
                            case 2:
                                break;
                            case 3:
                                break;
                            case 4:
                                break;
                            case 5:
                                break;
                        }
                    }
                    buttonStatus = -1;
                    RedrawWindow ();
                    break;

            } //end switch Event.type

        } //end X events

        usleep (100000L);

    } //end big if

  } //end infinite loop

  return 0;

} //end main

/***********************************
* Draws status bar. Given the slot
* number (intProcNum) and the
* percentage completed (intStatus),
* implements the copyXPMArea
* command on the xpm file.
***********************************/
void DrawStatus(int intProcNum, int intStatus)
{
  int intXPMx;

  //error correcting
  if((intProcNum > 9) || (intProcNum < 0))
    return;
  if(intStatus > 100)
    intStatus = 100;
  if(intStatus < 0)
    intStatus = 0;

  //location of image on xpm
  //just accept the math :-)
  intXPMx = ((100 - intStatus) * 9) / 10;
  intXPMx = intXPMx - (intXPMx%3);

  //draws if necessary
  if(intPrevStatus[intProcNum] != intXPMx)
  {
    copyXPMArea (intXPMx, 64,
      3, 30,
      8 + intProcNum*5, 27);

    intPrevStatus[intProcNum] = intXPMx;
  }
}

/******************************
* calls DrawStatus, decrements
* timer, and executes command
* or beep if timer reaches zero
*******************************/
void DecrementTimer ()
{
  int intCurrWDisplay = 0;
  int intCurrDDisplay = 0;
  int intCtr;
  int intStatus;
  //for each instance...
  for(intCtr=0; intCtr < 10; ++intCtr)
  {
    /*******
    * draws
    *******/
    //calculate percentage
    intStatus = ((
      (intMin[intCtr] * 60) +
      intSec[intCtr])
      * 100)
      / (intStartingTimes[intCtr]*60);

    //draw status bar
    if((chrType[intCtr] == 'w') && (intCurrWDisplay < 5))
    {
      DrawStatus(intCurrWDisplay, intStatus);
      ++intCurrWDisplay;
    }
    else if(intCurrDDisplay < 10)
    {
      DrawStatus(5 + intCurrDDisplay, intStatus);
      ++intCurrDDisplay;
    }

    /*****************
    * decrements time
    *****************/
    //if it's already 0, don't decrement it
    if((intSec[intCtr] == 0) && (intMin[intCtr] == 0))
      continue;

    --intSec[intCtr];
    if (intSec[intCtr] == -1)
    {
      intSec[intCtr] = 59;
      --intMin[intCtr];
    }

    //executes action/beeps, if necessazry
    if (intSec[intCtr] == 0 && intMin[intCtr] == 0)
      ExecAction (chrType[intCtr]);

  } //end for

}

/***************************
* executes action, or beeps
***************************/
void ExecAction (char chrType)
{
  if(chrType == 'd')
  {
    if (dInCommandMode)
    {
      execCommand (commandDry);
    }
    else
    {
      printf ("\07"); //beep
      fflush (stdout);
    }
  }
  else
  {
    if (wInCommandMode)
    {
      execCommand (commandWash);
    }
    else
    {
      printf ("\07"); //beep
      fflush (stdout);
    }
  }
}

/*************************
* gives command line help
*************************/
void displayUsage (void)
{
  fprintf (stderr, "\nwasherDryer - by Mike Foley <foley@ucsd.edu>\n\n");
  fprintf (stderr, "usage:\n");
  fprintf (stderr, "    -c    <filename> use specified config file\n");
  fprintf (stderr, "    -W    <int> washer time\n");
  fprintf (stderr, "    -D    <int> dryer time\n");
  fprintf (stderr, "    -w    <command> washer command (system bell is default)\n");
  fprintf (stderr, "    -d    <command> dryer command (system bell is default)\n");
  fprintf (stderr, "    -h    this help screen\n");
  fprintf (stderr, "    -v    print the version number\n\n");
  _exit (0);
}

/**********************
* handles click events
* in GUI window
**********************/
void callback (GtkWidget * widget, gpointer data)
{
  int intProcToChange;
  GtkWidget *dialog, *btnOK, *lblMain;
  char strInputText[4];

  //fprintf(stderr,"event received: %s\n", (char *)data);
  /*****************************
  * if user clicked bell button
  *****************************/
  if ((char *) data == "d_bell_button") {
    tmp_dInCmdMode = 0;
    gtk_entry_set_text (GTK_ENTRY (dEntry), "");
    gtk_entry_set_editable (GTK_ENTRY (dEntry), FALSE);
  }
  else if ((char *) data == "w_bell_button") {
    tmp_wInCmdMode = 0;
    gtk_entry_set_text (GTK_ENTRY (wEntry), "");
    gtk_entry_set_editable (GTK_ENTRY (wEntry), FALSE);
  }

  /********************************
  * if user clicked command button
  ********************************/
  else if ((char *) data == "d_command_button") {
    tmp_dInCmdMode = 1;
    gtk_entry_set_editable (GTK_ENTRY (dEntry), TRUE);
    gtk_entry_set_text (GTK_ENTRY (dEntry), commandDry);
  }
  else if ((char *) data == "w_command_button") {
    tmp_wInCmdMode = 1;
    gtk_entry_set_editable (GTK_ENTRY (wEntry), TRUE);
    gtk_entry_set_text (GTK_ENTRY (wEntry), commandWash);
  }

  /********************
  * if user clicked ok
  ********************/
  else if ((char *) data == "ok")
  {
    if (tmp_dInCmdMode == 1)
      strncpy (commandDry, gtk_entry_get_text (GTK_ENTRY (dEntry)), 256);
    if (tmp_wInCmdMode == 1)
      strncpy (commandWash, gtk_entry_get_text (GTK_ENTRY (wEntry)), 256);
    wInCommandMode = tmp_wInCmdMode;
    dInCommandMode = tmp_dInCmdMode;
  }
  /************************
  * if user changed a time
  ************************/
  else if(strstr((char *) data, "Change_"))
  {
      intProcToChange = ((char *)data)[7] - 48;

      //min = sec = starting time = new time
      intMin[intProcToChange] = intSec[intProcToChange] =
        intStartingTimes[intProcToChange] =
        atoi(gtk_entry_get_text(GTK_ENTRY (txtNewTime)));

      if(intMin[intProcToChange] == 0)
      {
        intSec[intProcToChange] =
          intStartingTimes[intProcToChange] = 1;
      }

      //fprintf(stderr, "intMin[%i], intStartingTimes[%i] changed to %i, %i\n",
      //  intProcToChange, intProcToChange, intMin[intProcToChange],
      //  intStartingTimes[intProcToChange]);
  }
  /************************
  * if user clicked cancel
  ************************/
  else if (!strcmp ((char *) data, "cancel"))
  {
  }

  /***********************
  * if user clicked clear
  ***********************/
  /*if (!strcmp ((char *) data, "clear"))
  {
    commandDry[0] = '\0';
    commandWash[0] = '\0';
    gtk_entry_set_text (GTK_ENTRY (dEntry), commandDry);
    gtk_entry_set_text (GTK_ENTRY (wEntry), commandWash);
  }*/

  /************************************
  * if user clicked one of the Deletes
  ************************************/
  else if(strstr((char *) data, "Delete"))
  {
    intProcToChange = ((char *)data)[6] - 48;
    intMin[intProcToChange] = 0;
    intSec[intProcToChange] = 0;
  }

  /****************************************
  * if user clicked one of the Change btns
  ****************************************/
  else if(strstr((char *) data, "Change"))
  {
    intProcToChange = ((char *)data)[6] - 48;

    /* Create the widgets */
    dialog = gtk_dialog_new();

    lblMain = gtk_label_new("New time (min):");

    txtNewTime = gtk_entry_new_with_max_length(3);
    sprintf(strInputText, "%i", intMin[intProcToChange]);
    gtk_entry_set_text((GtkEntry *)txtNewTime, (char *)strInputText);

    btnOK = gtk_button_new_with_label("Ok");
    //sprintf(buffer, "Change_%i", intProcToChange);
    //I am so dumb
      switch(intProcToChange)
      {
          case 0:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_0");
            break;
          case 1:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_1");
            break;
          case 2:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_2");
            break;
          case 3:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_3");
            break;
          case 4:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_4");
            break;
          case 5:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_5");
            break;
          case 6:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_6");
            break;
          case 7:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_7");
            break;
          case 8:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_8");
            break;
          case 9:
            gtk_signal_connect (GTK_OBJECT (btnOK), "clicked",
                GTK_SIGNAL_FUNC (callback), (gpointer) "Change_9");
            break;
      }
    /* Ensure that the dialog box is destroyed when the user clicks ok. */
    //###
    /*
    gtk_signal_connect_object(GTK_OBJECT(btnOK),
      "clicked", gtk_widget_destroy, GTK_OBJECT(dialog));
    gtk_signal_connect_object (GTK_OBJECT (btnOK),
      "clicked", gtk_widget_destroy, GTK_OBJECT (mainConfigWindow));
    */
    gtk_signal_connect_object (GTK_OBJECT (btnOK),
      "clicked", destroyAndReloadConfig, GTK_OBJECT (dialog));

    /* add stuff */
    //gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), lblMain);
    //gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->action_area), btnOK);
    //gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), txtNewTime);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->action_area),
      btnOK, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox),
      lblMain, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox),
      txtNewTime, TRUE, TRUE, 0);
    gtk_widget_show_all (dialog);
  }
}

/* This callback quits the program */
int delete_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  return (FALSE);
}

//destroys a widget, plus reloads mainConfigWindow
void destroyAndReloadConfig(GtkWidget* widget)
{
  if(widget != NULL)
    gtk_widget_destroy(widget);
  gtk_widget_destroy(mainConfigWindow);
  configure_washerdryer();
}

//destroys window
void destroy (GtkWidget * widget, gpointer data)
{
  gtk_main_quit ();
}

/******************************
* pops up configuration window
******************************/
int configure_washerdryer ()
{
  int intLoadsIn = 0;
  int intProcNo;
  char buffer[32];
  char bufferChangeBtn[16];
  int intTimeElapsed;
  GtkWidget *frame;
  GtkWidget *button;
  GtkWidget *buttonChange;
  GtkWidget *buttonDryC;
  GtkWidget *buttonWashC;
  GtkWidget *box1;
  GtkWidget *box2;
  GtkWidget *innerHbox;
  GtkWidget *sub_vbox;
  GtkWidget *label;

  tmp_wInCmdMode = wInCommandMode;
  tmp_dInCmdMode = dInCommandMode;

  // Create a new window
  mainConfigWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (mainConfigWindow), "Configure");
  gtk_window_set_wmclass (GTK_WINDOW (mainConfigWindow), "washerdryerconf", "");

  gtk_signal_connect (GTK_OBJECT (mainConfigWindow), "destroy",
     GTK_SIGNAL_FUNC (destroy), NULL);

  // Sets the border width of the window.
  gtk_container_set_border_width (GTK_CONTAINER (mainConfigWindow), 10);

  // Create Vertical box
  box1 = gtk_vbox_new (FALSE, 0);
  // Add vertical box to main window
  gtk_container_add (GTK_CONTAINER (mainConfigWindow), box1);
  gtk_widget_show (box1);

/////

  /********************
  * washer action frame
  ********************/
  frame = gtk_frame_new ("Washer Action");
  gtk_box_pack_start (GTK_BOX (box1), frame, TRUE, TRUE, 2);
  gtk_widget_show (frame);

  //Create Vertical box
  sub_vbox = gtk_vbox_new (FALSE, 0);
  //Add vertical box to main window
  gtk_container_add (GTK_CONTAINER (frame), sub_vbox);
  gtk_widget_show (sub_vbox);

  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (sub_vbox), box2, TRUE, TRUE, 2);

  //Bell radio button
  button = gtk_radio_button_new_with_label (NULL, "System Bell");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (callback), (gpointer) "w_bell_button");
  gtk_box_pack_start (GTK_BOX (box2), button, FALSE, FALSE, 2);
  gtk_widget_show (button);

  //Command radio button
  buttonWashC = gtk_radio_button_new_with_label (gtk_radio_button_group
              (GTK_RADIO_BUTTON (button)),
              "Command");
  gtk_signal_connect (GTK_OBJECT (buttonWashC), "clicked",
          GTK_SIGNAL_FUNC (callback),
          (gpointer) "w_command_button");
  gtk_box_pack_start (GTK_BOX (box2), buttonWashC, FALSE, FALSE, 2);
  gtk_widget_show (buttonWashC);
  gtk_widget_show (box2);


  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (sub_vbox), box2, TRUE, TRUE, 2);

  label = gtk_label_new ("Command: ");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_box_pack_start (GTK_BOX (box2), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  /* Create "Command" text entry area */
  wEntry = gtk_entry_new_with_max_length (100);
  gtk_entry_set_editable (GTK_ENTRY (wEntry), FALSE);
  gtk_signal_connect (GTK_OBJECT (wEntry), "activate",
          GTK_SIGNAL_FUNC (callback), wEntry);
  gtk_box_pack_start (GTK_BOX (box2), wEntry, FALSE, FALSE, 2);
  gtk_widget_show (wEntry);
  gtk_widget_show (box2);

  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 2);

/////

  /********************
  * dryer action frame
  ********************/
  frame = gtk_frame_new ("Dryer Action");
  gtk_box_pack_start (GTK_BOX (box1), frame, TRUE, TRUE, 2);
  gtk_widget_show (frame);

  // Create Vertical box
  sub_vbox = gtk_vbox_new (FALSE, 0);
  // Add vertical box to main window
  gtk_container_add (GTK_CONTAINER (frame), sub_vbox);
  gtk_widget_show (sub_vbox);

  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (sub_vbox), box2, TRUE, TRUE, 2);

  //Bell radio button
  button = gtk_radio_button_new_with_label (NULL, "System Bell");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (callback), (gpointer) "d_bell_button");
  gtk_box_pack_start (GTK_BOX (box2), button, FALSE, FALSE, 2);
  gtk_widget_show (button);

  //Command radio button
  buttonDryC = gtk_radio_button_new_with_label (gtk_radio_button_group
              (GTK_RADIO_BUTTON (button)),
              "Command");
  gtk_signal_connect (GTK_OBJECT (buttonDryC), "clicked",
          GTK_SIGNAL_FUNC (callback),
          (gpointer) "d_command_button");
  gtk_box_pack_start (GTK_BOX (box2), buttonDryC, FALSE, FALSE, 2);
  gtk_widget_show (buttonDryC);
  gtk_widget_show (box2);


  /**************
  * Status frame
  **************/
  frame = gtk_frame_new ("Status");
  gtk_box_pack_start (GTK_BOX (box1), frame,TRUE, TRUE, 2);
  gtk_widget_show (frame);

  for(intProcNo = 0; intProcNo<10; ++intProcNo)
  {
    //if process is still running
    if((intSec[intProcNo] != 0) || (intMin[intProcNo] != 0))
    {
      ++intLoadsIn;

      if(intLoadsIn == 1)
      {
        box2 = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (frame), box2);
        gtk_widget_show (box2);
      }

      innerHbox = gtk_hbox_new (FALSE, 2);
      gtk_container_add (GTK_CONTAINER (box2), innerHbox);
      gtk_widget_show (innerHbox);

      if(chrType[intProcNo] == 'w')
        sprintf(buffer,"Washer:   ");
      else
        sprintf(buffer," Dryer:   ");

      label = gtk_label_new (buffer);
      gtk_misc_set_alignment (GTK_MISC (label), 2, 2);
      gtk_box_pack_start (GTK_BOX (innerHbox), label, TRUE, FALSE, 2);
      gtk_widget_show (label);

      sprintf(bufferChangeBtn, "%2i", intMin[intProcNo]);
      buttonChange = gtk_button_new_with_label (bufferChangeBtn);
      sprintf(bufferChangeBtn,"Change%i", intProcNo);
      /**************************************
      * why do I do it this way? because I'm
      * dumb and I don't know a better way
      **************************************/
      switch(intProcNo)
      {
          case 0:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change0");
            break;
          case 1:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change1");
            break;
          case 2:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change2");
            break;
          case 3:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change3");
            break;
          case 4:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change4");
            break;
          case 5:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change5");
            break;
          case 6:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change6");
            break;
          case 7:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change7");
            break;
          case 8:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change8");
            break;
          case 9:
            gtk_signal_connect (GTK_OBJECT (buttonChange), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Change9");
            break;
      }
      gtk_box_pack_start (GTK_BOX(innerHbox), buttonChange, FALSE, FALSE, 2);
      gtk_widget_show (buttonChange);

      sprintf(buffer, " min remaining");
      label = gtk_label_new (buffer);
      gtk_misc_set_alignment (GTK_MISC (label), 2, 2);
      gtk_box_pack_start (GTK_BOX (innerHbox), label, FALSE, FALSE, 2);
      gtk_widget_show (label);

      button = gtk_button_new_with_label ("Delete");
      sprintf(buffer,"Delete%i", intProcNo);

      /*************************************
      * why this way? see above switch stmt
      *************************************/
      switch(intProcNo)
      {
          case 0:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete0");
            break;
          case 1:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete1");
            break;
          case 2:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete2");
            break;
          case 3:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete3");
            break;
          case 4:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete4");
            break;
          case 5:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete5");
            break;
          case 6:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete6");
            break;
          case 7:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete7");
            break;
          case 8:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete8");
            break;
          case 9:
            gtk_signal_connect (GTK_OBJECT (button), "clicked",
              GTK_SIGNAL_FUNC (callback), (gpointer)"Delete9");
            break;
      }

      /*
      gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (gtk_widget_destroy),
          GTK_OBJECT (mainConfigWindow));
      */

    gtk_signal_connect_object (GTK_OBJECT (button),
      "clicked", destroyAndReloadConfig, NULL);

      gtk_box_pack_start (GTK_BOX (innerHbox), button, FALSE, FALSE, 4);
      gtk_widget_show (button);

    }
  } //end for


  //if no loads
  if(intLoadsIn == 0)
  {
    box2 = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame), box2);
    gtk_widget_show (box2);

    label = gtk_label_new ("   No loads in washer/dryer.");
    gtk_misc_set_alignment (GTK_MISC (label), 2, 2);
    gtk_box_pack_start (GTK_BOX (box2), label, FALSE, FALSE, 2);
    gtk_widget_show (label);
  }

  /****************************
  * doing some more dryer shit
  ****************************/
  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (sub_vbox), box2, TRUE, TRUE, 2);

  label = gtk_label_new ("Command: ");
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
  gtk_box_pack_start (GTK_BOX (box2), label, FALSE, FALSE, 2);
  gtk_widget_show (label);

  /* Create "Command" text entry area */
  dEntry = gtk_entry_new_with_max_length (100);
  gtk_entry_set_editable (GTK_ENTRY (dEntry), FALSE);
  gtk_signal_connect (GTK_OBJECT (dEntry), "activate",
          GTK_SIGNAL_FUNC (callback), dEntry);
  gtk_box_pack_start (GTK_BOX (box2), dEntry, FALSE, FALSE, 2);
  gtk_widget_show (dEntry);
  gtk_widget_show (box2);

  box2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 2);

  // Create "Cancel" button
  button = gtk_button_new_with_label ("Cancel");
  //gtk_signal_connect (GTK_OBJECT (button), "clicked", GTK_SIGNAL_FUNC (delete_event), NULL);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (callback), "cancel");
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
           GTK_SIGNAL_FUNC (gtk_widget_destroy),
           GTK_OBJECT (mainConfigWindow));
  gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 2);
  gtk_widget_show (button);

  // Create "Clear" button
/*  button = gtk_button_new_with_label ("Clear");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (callback), "clear");
  gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
*/


  // Create "Ok" button
  button = gtk_button_new_with_label ("Ok");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
          GTK_SIGNAL_FUNC (callback), "ok");
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
           GTK_SIGNAL_FUNC (gtk_widget_destroy),
           GTK_OBJECT (mainConfigWindow));
  gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 2);
  gtk_widget_show (button);
  gtk_widget_show (box2);

  gtk_widget_show (mainConfigWindow);

  if (dInCommandMode == 1)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDryC), TRUE);
  if (wInCommandMode == 1)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonWashC), TRUE);

  intTimeElapsed = time(0);
  gtk_main ();
  intTimeElapsed = time(0) - intTimeElapsed;
  while(intTimeElapsed > 0)
  {
    DecrementTimer();
    --intTimeElapsed;
  }
  return 0;
}

/***********************************
* Reads a string from configuration
* file. Taken from wminet
***********************************/
int ReadConfigString(FILE *fp, char *setting, char *value)
{
    char str[1024];
    char buf[1024];
    int i;
    int len;
    int slen;
    char *p=NULL;


    if (!fp)
    {
        return 0;
    }

    sprintf(str, "%s=", setting);
    slen = strlen(str);

    fseek(fp, 0, SEEK_SET);

    while ( !feof(fp) )
    {
        if (!fgets(buf, 512, fp))
            break;

        len = strlen(buf);

        // strip linefeed
        for (i=0; i!=len; i++)
        {
            if (buf[i] == '\n')
            {
                buf[i] = 0;
            }
        }

        if ( strncmp(buf, str, strlen(str)) == 0)
        {
            // found our setting
            for(i=0; i!=slen; i++)
            {
                if ( buf[i] == '=' )
                {
                    p=buf+i+1;
                    strcpy(value, p);
                    return 1;
                }
            }
        }
    }
    return 0;
}


/*****************************
* reads int from config file.
* taken from wminet
*****************************/
int ReadConfigInt(FILE *fp, char *setting, int *value)
{
    char buf[1024];

    if (ReadConfigString(fp, setting, (char *) &buf))
    {
        *value = atoi(buf);
        return 1;
    }

    return 0;
}


/*****************************
* Read config file and stores
* values to local variables
*****************************/
int Read_Config_File(char* filename )
{
    FILE* fp;

    fp = fopen(filename, "r");
    if (fp)
    {
        if(intDefaultWasherTime == 0)
        {
          ReadConfigInt(fp, "washer_time", &intDefaultWasherTime);
        }
        if(intDefaultDryerTime == 0)
        {
          ReadConfigInt(fp, "dryer_time", &intDefaultDryerTime);
        }
        if(commandWash[0] == '\0')
        {
          ReadConfigInt(fp, "exec_washer_action", &wInCommandMode);
        }
        if(commandDry[0] == '\0')
        {
          ReadConfigInt(fp, "exec_dryer_action", &dInCommandMode);
        }
        tmp_wInCmdMode = wInCommandMode;
        tmp_dInCmdMode = dInCommandMode;
        if(commandWash[0] == '\0')
        {
          ReadConfigString(fp, "washer_action", commandWash);
        }
        if(commandDry[0] == '\0')
        {
          ReadConfigString(fp, "dryer_action", commandDry);
        }

        fclose(fp);
        return 1;
    }
    else
    {
        perror("Read_Config_File");
        fprintf(stderr, "Unable to open %s, no settings read.\n", filename);
        return 0;
    }
}

