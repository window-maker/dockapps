#include <libical/ical.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <sys/stat.h>


time_t  modtime;              /* modified time of icalendar file */
struct  calobj* calRoot;  /* 1st element in list of calendar obj*/
int     xr, yr;               /*evil hack for moving dayview window*/
int     datetype[32][2];      /* hashtable for coloring days.[jdn%32][0] stores color of day jdn,
				 [jdn%32][1] stores jdn. */
struct calobj{
    struct icaltimetype start;
    struct icaltimetype end;
    icalcomponent *comp;
    int type;
    char *text;
    struct calobj* next;
    struct calobj* exclude;
};

int  get_datetype(int day);
void calendar();
void showDay(struct icaltimetype dt);
void destroy (GtkWidget * widget, gpointer data);
int  dayevents(struct icaltimetype dt, GtkWidget *table);

void deleteCalObjs();
void addCalObj(struct icaltimetype start, struct icaltimetype end,
	       int type, const char *text, icalcomponent * d);
int  getDayType(struct icaltimetype dt);
int  calcDayType(struct icaltimetype dt);
char* read_stream(char *s, size_t size, void *d);
void checkicalversion();

