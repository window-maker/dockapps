
/* Preference window generator example */

#include <gai/gai.h>
#include <stdio.h>
#include "config.h"


static int check_default = 1, check_result, radio_default = 2, radio_result;
static int spin_default = 10, spin_result;
static int option_default = 1, option_result;
static int combo_default = 1, combo_result;
static GList *combo_list = NULL;

static char *default_name = "Jonas", 
    *file_default = N_("Hello_fem_bananer"), 
    *result_name=NULL, 
    *pass_default=N_("Hello");
static char *pass_result=NULL, *file_result=NULL;
static char *radio_names[] = { N_("Apples"), N_("Bananas"), N_("Pears"), 
			       N_("Strawberry"), NULL};
static char *option_list[] = { N_("Cake"), N_("Milk"), N_("Icecream"), 
			       N_("Nuts"), NULL};

static GaiSS spin_limits = {0, 40, 3};
static float spinfloat_default = 43.3, spinfloat_result;
static GaiSSF spinfloat_limits = {40.0, 60.0, 1.0, 1};
static GaiColor color_default = {0xf0, 0xd0, 0xc0, 0xa0}, color_result;

static char *list_names[] = { N_("Mounted"), N_("auto-mount"), N_("Source dir"),
			      N_("Mountpoint"), N_("Idle time"), 
			      N_("some name"), NULL};

static GType list_types[] = {G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING};
static char *item_1[] = {"0", "1", "/dev/hda", "/", "5", "Linux"};
static char *item_2[] = {"1", "1", "/dev/scd0", "/cdrom", "8", "DVD"};
static char *item_3[] = {"0", "1", "/dev/sda", "/jaz", "30",  "Good ol' iomega jaz"};
static char *item_4[] = {"0", "0", "/dev/sdb", "/nex", "10",  "MP3 player"};

static char **list_defaults[] = {item_1, 
				 item_2, 
				 item_3, 
				 item_4, 
				 NULL};
static GList *list_results = NULL;


void func(void);

static GaiPI g_pref[] = {{GAI_NOTEBOOK,  N_("Book one")},
			    {GAI_SCROLL_BOX},
			       {GAI_FRAME,  N_("Folder 1")},
			           {GAI_TEXT,  N_("Hello")},
			           {GAI_TEXT|GAI_LEFT, N_("Duduesssssssssssssss")},
			           {GAI_TEXT|GAI_RIGHT,  N_("Banana")},
			       {GAI_FRAME_E},
		  
			       {GAI_FRAME,  N_("Folder 2")},
			           {GAI_TEXT,  N_("Kalle anka")},
			           {GAI_TEXT,  N_("Apple")},
			           {GAI_COMBO,  N_("Network interface:"), &combo_default, &combo_result, &combo_list},
			       {GAI_FRAME_E},
			       {GAI_HLINE},
			       {GAI_FRAME,  N_("Folder 3")},
			           {GAI_ALL_RIGHT},
			           {GAI_TEXT, N_("Kalle anka2")},
			           {GAI_FRAME,  N_("Folder Inside")},
			               {GAI_TEXT, N_("Kaniner")},
			               {GAI_TEXT, N_("Bavrar")},
			           {GAI_FRAME_E},
			           {GAI_TEXT,  N_("Apple2")},
			       {GAI_FRAME_E},
			    {GAI_SCROLL_BOX_E},
			    {GAI_FRAME_R,  N_("Right frame")},
			       {GAI_TEXT, N_("Hej!")},
			    {GAI_FRAME_E},

		      {GAI_NOTEBOOK_E},
		  
		      {GAI_NOTEBOOK,  N_("Book two")},
			 {GAI_ALL_LEFT},
			 {GAI_TEXT|GAI_LEFT, N_("Hellooo")},
			 {GAI_CHECKBUTTON|GAI_CENTER,  N_("My checkbutton"), &check_default, &check_result},
			 {GAI_TEXT|GAI_RIGHT, N_("Hellooo2")},
			 {GAI_TEXT|GAI_CENTER,  N_("<span foreground=\"blue\" size=\"x-large\">Blue Hello</span> is <i>cool</i>!")},
			 {GAI_COLORSELECTOR,  N_("Background colour:"), &color_default, &color_result},
			 {GAI_FILESELECTOR,  N_("Image file:"), &file_default, &file_result},
			 {GAI_BUTTON_IMAGE, N_("Hej!"),"gai-pref-icon.png", (void *)func},
			 {GAI_BUTTON_TEXT|GAI_RIGHT, N_("<i>Italic button</i>"), (void *)func},
			 {GAI_BUTTON_STOCK,  N_("<b>Bold stock button</b>"), "gtk-open", (void *)func},
		      {GAI_NOTEBOOK_E},
		  
		      {GAI_NOTEBOOK,  N_("Book three")},
		         {GAI_TEXTENTRY,  N_("Enter your name:"), &default_name, &result_name},
		         {GAI_RADIOBUTTON, radio_names, &radio_default, &radio_result},
		         {GAI_SPINBUTTON,  N_("Some value"), &spin_default, &spin_result, &spin_limits},
			 {GAI_SPINBUTTONFLOAT,  N_("Float values"), &spinfloat_default, &spinfloat_result, &spinfloat_limits},
			 {GAI_OPTIONMENU,  N_("Options:"),&option_default, &option_result, option_list},
			 {GAI_PASSWORDENTRY,  N_("Secret"), &pass_default, &pass_result},
		      {GAI_NOTEBOOK_E},
		      {GAI_NOTEBOOK,  N_("ListStore Text")},
		         {GAI_LISTSTORE, 
			  list_names, 
			  list_defaults, 
			  &list_results, 
			  list_types},
		      {GAI_NOTEBOOK_E},
		      {GAI_NOTEBOOK,  N_("ListStore Text - Edit")},
		      {GAI_EDITLISTSTORE, 
			list_names, 
			list_defaults, 
			&list_results, 
			list_types},
		      {GAI_NOTEBOOK_E},

		  {GAI_END}};
void func(void)
{
    printf(N_("ksajldk\n"));
}

void pref_func(gpointer d)
{
    int i,j;
    GList *list;
    printf(_("Text entry returns:%s\nCheck button returns: %d\n"), (char *)result_name, check_result);

    printf(_("Combo box (%d)\n"), combo_result);
    for(i=0;i<g_list_length(combo_list);i++)
	printf(_("Entry %d is: %s\n"),i,(char *)g_list_nth_data(combo_list,i));

    printf(_("colour is: %d %d %d\n"),color_result.r, color_result.g, color_result.b);

    printf(_("Radio button choise is: %d\n"),radio_result);
    printf(_("File selector result is: %s\n"), file_result);
    printf(_("Option menu result is: %d\n"), option_result);
    printf(_("Password entry is: %s\n"),pass_result);
    printf(_("Spin button float: %f\n"), spinfloat_result);
    printf(_("Spin button: %d\n"), spin_result);

    for(i=0;i<g_list_length(list_results);i++){
	list = g_list_nth_data(list_results, i);
	for(j=0;j<g_list_length(list);j++)
	    printf("%d %d - %s\n", j, i, (char*) g_list_nth_data(list, j)); 
    }	

}


int main(int argc, char  **argv)
{

    printf(_("func:%x\n"), (int)((void *)func));
    printf(_("files: %x(%s)\n"),(int) &file_default, (char *)(&file_default)[0]);

    gai_init2(&applet_defines, &argc, &argv);

    combo_list = g_list_append(combo_list, _("Hej"));
    combo_list = g_list_append(combo_list, _("asfsd"));
    combo_list = g_list_append(combo_list, _("asfsd2"));
    
    gai_preferences2(_("Hello, this is a preference window"), g_pref, 
		     _("Example code for the prefrence window generator\n"
		     "Try it from the commandprompt to see how it works.\n"),
		     (GaiCallback1 *)pref_func, NULL);

    gai_start();

    return 0;


}
