#define PROC_DEF "wmaker" 	      

#define MAX_CHAR 8
#define NB_LINE 4

#define CLICK_ONE 1
#define CLICK_TWO 3

#define WHEEL_UP 4
#define WHEEL_DOWN 5

#define X_PROC 6
#define Y_PROC 7

#define DOWN 7 			       /* minus */
#define UP 8			       /* plus */

#define DOUBLE_CLICK_DELAY 170000L
#define DELAY 10000
#define UPDATE_NB 60 		       /* checking delay = DELAY * UPDATE_NB (ms) */

typedef struct _desc{
   char name[40];
   pid_t pid;
   struct _desc *previous;
   struct _desc *next;
} _desc;

typedef struct _zone{
   int x;
   int y;
   int width;
   int height;
   char no;
   struct _zone *next;
} _zone;

void ZoneCreate(int x, int y, int width, int height, char no);
void GarbageCollector(_desc *garb);
char CheckZone(void);
int CheckProc(pid_t pid);
_desc *GetProcList(void);
int CheckProcToRemove(unsigned int *procList, unsigned int procListSize);
int CheckProcToAdd(int pos, unsigned int *procList, unsigned int procListSize);
int CheckProcChange(void);
void RemoveProc(_desc *cible);
void ShowString (int x, int y, char *doudou);
void DoExp();
void DoExpose();
void DoClick(XEvent ev);
void DoEvents();
void PrintUsage(void);
void GetArg(int argc, char *argv[]);
void CreateDock(int argc, char *argv[]);
