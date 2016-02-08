/******************************************/
/* WMTOP - Mini top in a dock app         */
/******************************************/

/*
 * wmtop.c -- WindowMaker process view dock app
 * Derived by Dan Piponi dan@tanelorn.demon.co.uk
 * http://www.tanelorn.demon.co.uk
 * http://wmtop.sourceforge.net 
 * from code originally contained in wmsysmon by Dave Clark (clarkd@skynet.ca)
 * This software is licensed through the GNU General Public License.
 */

/*
 * Ensure there's an operating system defined. There is *no* default
 * because every OS has it's own way of revealing CPU/memory usage.
 */
#if defined(FREEBSD)
#define OS_DEFINED
#endif /* defined(FREEBSD) */

#if defined(LINUX)
#define OS_DEFINED
#endif /* defined(LINUX) */

#if !defined(OS_DEFINED)
#error No operating system selected
#endif /* !defined(OS_DEFINED) */

/******************************************/
/* Includes                               */
/******************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>

#if defined(PARANOID)
#include <assert.h>
#endif /* defined(PARANOID) */

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/keysym.h>

#include <regex.h>

#include "wmgeneral/wmgeneral.h"
#include "wmgeneral/misc.h"
#include "xpm/wmtop-default.xpm"
#include "xpm/wmtop-lcd.xpm"
#include "xpm/wmtop-neon1.xpm"
#include "xpm/wmtop-neon2.xpm"
#include "xpm/wmtop-rainbow.xpm"

/******************************************/
/* Defines                                */
/******************************************/

#define WMTOP_VERSION "0.9"

/*
 * XXX: I shouldn't really use this WMTOP_BUFLENGTH variable but scanf is so
 * lame and it'll take me a while to write a replacement.
 */
#define WMTOP_BUFLENGTH 1024

#if defined(LINUX)
#define PROCFS_TEMPLATE "/proc/%d/stat"
#define PROCFS_CMDLINE_TEMPLATE "/proc/%d/cmdline"
#endif /* defined(LINUX) */

#if defined(FREEBSD)
#define PROCFS_TEMPLATE "/proc/%d/status"
#endif /* defined(FREEBSD) */

/******************************************/
/* Globals                                */
/******************************************/

regex_t *exclusion_expression = 0;
int user = -1;
char *process_command = 0;
/*
 * Default mode: zero=cpu one=memory
 */
int mode = 0;

/*
 * Number and default artistic styles.
 */
int nstyles = 5;
int style = 0;

char wmtop_mask_bits[64*64];
int wmtop_mask_width = 64;
int wmtop_mask_height = 64;

int update_rate = 1000000;
int refresh_rate = 100000;

extern char **environ;

char *ProgName;

/******************************************/
/* Debug                                  */
/******************************************/

#if defined(DEBUG)
/*
 * Memory handler
 */
int g_malloced = 0;

void *wmtop_malloc(int n) {
    int *p = (int *)malloc(sizeof(int)+n);
    p[0] = n;
    g_malloced += n;
    return (void *)(p+1);
}

void wmtop_free(void *n) {
    int *p = (int *)n;
    g_malloced -= p[-1];
    free(p-1);
}

void show_memory() {
    fprintf(stderr,"%d bytes allocated\n",g_malloced);
}
#else /* defined(DEBUG) */
#define wmtop_malloc malloc
#define wmtop_free free
#endif /* defined(DEBUG) */

char *wmtop_strdup(const char *s) {
    return strcpy((char *)wmtop_malloc(strlen(s)+1),s);
}

/******************************************/
/* Structures                             */
/******************************************/

struct {
    char **pixmap;
    char *description;
} styles[] = {
    { wmtop_default_xpm, "Light emitting diode (default)" },
    { wmtop_lcd_xpm, "Liquid crystal display" },
    { wmtop_rainbow_xpm, "Rainbow display" },
    { wmtop_neon1_xpm, "Neon lights" },
    { wmtop_neon2_xpm, "More neon lights" },
};

struct process {
#if defined(PARANOID)
    long id;
#endif /* defined(PARANOID) */
    /*
     * Store processes in a doubly linked list
     */
    struct process *next;
    struct process *previous;

    pid_t pid;
    char *name;
    float amount;
    int user_time;
    int kernel_time;
    int previous_user_time;
    int previous_kernel_time;
    int vsize;
    int rss;
    int time_stamp;
    int counted;
};

/******************************************/
/* Process class                          */
/******************************************/

/*
 * Global pointer to head of process list
 */
struct process *first_process = 0;

int g_time = 0;

struct process *find_process(pid_t pid) {
    struct process *p = first_process;
    while (p) {
	if (p->pid==pid)
	    return p;
	p = p->next;
    }
    return 0;
}

/*
 * Create a new process object and insert it into the process list
 */
struct process *new_process(int p) {
    struct process *process;
    process = wmtop_malloc(sizeof(struct process));

#if defined(PARANOID)
    process->id = 0x0badfeed;
#endif /* defined(PARANOID) */

    /*
     * Do stitching necessary for doubly linked list
     */
    process->name = 0;
    process->previous = 0;
    process->next = first_process;
    if (process->next)
	process->next->previous = process;
    first_process = process;

    process->pid = p;
    process->time_stamp = 0;
    process->previous_user_time = INT_MAX;
    process->previous_kernel_time = INT_MAX;
    process->counted = 1;

/*    process_find_name(process);*/

    return process;
}

/******************************************/
/* Functions                              */
/******************************************/

void wmtop_routine(int, char **);
int process_parse_procfs(struct process *);
int update_process_table(void);
int calculate_cpu(struct process *);
void process_cleanup(void);
void delete_process(struct process *);
inline void draw_processes(void);
int calc_cpu_total(void);
void calc_cpu_each(int);
#if defined(LINUX)
int calc_mem_total(void);
void calc_mem_each(int);
#endif
int process_find_top_three(struct process **);
void draw_bar(int, int, int, int, float, int, int);
inline void blit_string(char *, int, int);
void usage(void);
inline void printversion(void);

/******************************************/
/* Main                                   */
/******************************************/

int main(int argc, char *argv[]) {
    int i;
    struct stat sbuf;

    /*
     * Make sure we have a /proc filesystem. No point in continuing if we
     * haven't!
     */
    if (stat("/proc",&sbuf)<0) {
      fprintf(stderr,
	      "No /proc filesystem present. Unable to obtain processor info.\n");
      exit(1);
    }

    /*
     * Parse Command Line
     */

    ProgName = argv[0];
    if (strlen(ProgName) >= 5)
	ProgName += strlen(ProgName) - 5;

    for (i = 1; i<argc; i++) {
	char *arg = argv[i];

	if (*arg=='-') {
	    switch (arg[1]) {
	    case 'x' :
		if (argc>i+1) {
		    static regex_t reg;
		    exclusion_expression = &reg;
		    regcomp(exclusion_expression,argv[i+1],REG_EXTENDED);
		    i++;
		} else {
		    usage();
		    exit(1);
		}
		break;
	    case 'c' :
		if (argc>i+1) {
		    process_command = argv[i+1];
		    i++;
		    break;
		} else {
		    usage();
		    exit(1);
		}
#if defined(LINUX)
	    case 'm':
		/*
		* Display memory
		*/
		mode = 1;
		break;
#endif /* defined(LINUX) */
	    case 'd' :
		if (strcmp(arg+1, "display")) {
		    usage();
		    exit(1);
		}
		break;
	    case 'g' :
		if (strcmp(arg+1, "geometry")) {
		    usage();
		    exit(1);
		}
		break;
	    case 'v' :
		printversion();
		exit(0);
		break;
	    case 'U' :
		user = getuid();
		break;
	    case 's':
		if (argc > (i+1)) {
		    update_rate = (atoi(argv[i+1]) * 1000);
		    i++;
		}
		break;
	    case 'r':
		if (argc > (i+1)) {
		    refresh_rate = (atoi(argv[i+1]) * 1000);
		    i++;
		}
		break;
	    case 'a':
		if (argc > (i+1)) {
		    if (atoi(argv[i+1]) < 1 || atoi(argv[i+1]) > nstyles) {
			usage();
			exit(1);
		    }
		    style = atoi(argv[i+1]) - 1;
		    i++;
		}
		break;
	    default:
		usage();
		exit(0);
		break;
	    }
	}
    }

    wmtop_routine(argc, argv);

    return 0;
}

/******************************************/
/* Main routine                           */
/******************************************/

void wmtop_routine(int argc, char **argv) {
    XEvent Event;
    struct timeval tv={0,0};
    struct timeval last={0,0};
    int count = update_rate;

    createXBMfromXPM(wmtop_mask_bits, styles[style].pixmap, wmtop_mask_width, wmtop_mask_height);

    openXwindow(argc, argv, styles[style].pixmap, wmtop_mask_bits, wmtop_mask_width, wmtop_mask_height);


    while (1) {

	waitpid(0, NULL, WNOHANG);

	if (count>=update_rate) {
	    memcpy(&last,&tv,sizeof(tv));

	    /*
	     * Update display
	     */
	    draw_processes();

	    RedrawWindow();
	    count = 0;
	}

	/*
	 * X Events
	 */
	while (XPending(display)) {
	    XNextEvent(display, &Event);
	    switch (Event.type) {
		case Expose:
		    RedrawWindow();
		    break;
		case DestroyNotify:
		    XCloseDisplay(display);
		    exit(0);
		case ButtonPress:
#if defined(LINUX)
		    if (Event.xbutton.button==1)
			mode = !mode;
#endif
		    if (Event.xbutton.button==2) {
			if (user==-1)
			    user=getuid();
			else
			    user=-1;
		    }
		    if (Event.xbutton.button==3 && process_command)
			execCommand(process_command);
		    break;
	    }
	}
	usleep(refresh_rate);
	count = count + refresh_rate;
    }
}

/******************************************/
/* Extract information from /proc         */
/******************************************/

/*
 * These are the guts that extract information out of /proc.
 * Anyone hoping to port wmtop should look here first.
 */
int process_parse_procfs(struct process *process) {
    char line[WMTOP_BUFLENGTH],filename[WMTOP_BUFLENGTH],procname[WMTOP_BUFLENGTH];
    int ps;
    struct stat sbuf;
    int user_time,kernel_time;
    int rc;
#if defined(LINUX)
    char *r,*q;
    char deparenthesised_name[WMTOP_BUFLENGTH];
		int endl;
#endif /* defined(LINUX) */
#if defined(FREEBSD)
    int us,um,ks,km;
#endif /* defined(FREEBSD) */

#if defined(PARANOID)
    assert(process->id==0x0badfeed);
#endif /* defined(PARANOID) */

    sprintf(filename,PROCFS_TEMPLATE,process->pid);

    /*
     * Permissions of /proc filesystem are permissions of process too
     */
    if (user>=0) {
	stat(filename,&sbuf);
	if (sbuf.st_uid!=user)
	    return 1;
    }

    ps = open(filename,O_RDONLY);
    if (ps<0)
	/*
	 * The process must have finished in the last few jiffies!
	 */
	return 1;

    /*
     * Mark process as up-to-date.
     */
    process->time_stamp = g_time;

    rc = read(ps,line,sizeof(line));
    close(ps);
    if (rc<0)
	return 1;

#if defined(LINUX)
    /*
     * Extract cpu times from data in /proc filesystem
     */
    rc = sscanf(line,"%*s %s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %d %*s %*s %*s %*s %*s %*s %*s %d %d",
	    procname,
	    &process->user_time,&process->kernel_time,
	    &process->vsize,&process->rss);
    if (rc<5)
	return 1;
    /*
     * Remove parentheses from the process name stored in /proc/ under Linux...
     */
    r = procname+1;
    /* remove any "kdeinit: " */
    if (r == strstr(r, "kdeinit"))
     {
      sprintf(filename,PROCFS_CMDLINE_TEMPLATE,process->pid);

      /*
       * Permissions of /proc filesystem are permissions of process too
       */
      if (user>=0) {
        stat(filename,&sbuf);
        if (sbuf.st_uid!=user)
            return 1;
      }

      ps = open(filename,O_RDONLY);
      if (ps<0)
        /*
         * The process must have finished in the last few jiffies!
         */
        return 1;

        endl = read(ps,line,sizeof(line));
       close(ps);

      /* null terminate the input */
      line[endl]=0;
      /* account for "kdeinit: " */
      if ((char*)line == strstr(line, "kdeinit: "))
        r = ((char*)line)+9; 
      else
        r = (char*)line;

      q = deparenthesised_name;
      /* stop at space */
      while (*r && *r!=' ')
        *q++ = *r++;
       *q = 0;
    }
    else
    {
      q = deparenthesised_name;
      while (*r && *r!=')')
        *q++ = *r++;
       *q = 0;
    }

    if (process->name)
	wmtop_free(process->name);
    process->name = wmtop_strdup(deparenthesised_name);
#endif /* defined(LINUX) */

#if defined(FREEBSD)
    /*
     * Extract cpu times from data in /proc/<pid>/stat
     * XXX: Process name extractor for FreeBSD is untested right now.
     */
    rc = sscanf(line,"%s %*s %*s %*s %*s %*s %*s %*s %d,%d %d,%d",
	    procname,
	    &us,&um,&ks,&km);
    if (rc<5)
	return 1;
    if (process->name)
	wmtop_free(process->name);
    process->name = wmtop_strdup(procname);
    process->user_time = us*1000+um/1000;
    process->kernel_time = ks*1000+km/1000;
#endif /* defined(FREEBSD) */

    process->rss *= getpagesize();

    if (process->previous_user_time==INT_MAX)
	process->previous_user_time = process->user_time;
    if (process->previous_kernel_time==INT_MAX)
	process->previous_kernel_time = process->kernel_time;

    user_time = process->user_time-process->previous_user_time;
    kernel_time = process->kernel_time-process->previous_kernel_time;

    process->previous_user_time = process->user_time;
    process->previous_kernel_time = process->kernel_time;

    process->user_time = user_time;
    process->kernel_time = kernel_time;

    return 0;
}

/******************************************/
/* Update process table                   */
/******************************************/

int update_process_table() {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir("/proc")))
	return 1;

    /*
     * Get list of processes from /proc directory
     */
    while ((entry = readdir(dir))) {
	pid_t pid;

	if (!entry) {
	    /*
	     * Problem reading list of processes
	     */
	    closedir(dir);
	    return 1;
	}

	if (sscanf(entry->d_name,"%d",&pid)>0) {
	    struct process *p;
	    p = find_process(pid);
	    if (!p)
		p = new_process(pid);

	    calculate_cpu(p);
	}
    }

    closedir(dir);

    return 0;
}

/******************************************/
/* Get process structure for process pid  */
/******************************************/

/*
 * This function seems to hog all of the CPU time. I can't figure out why - it
 * doesn't do much.
 */
int calculate_cpu(struct process *process) {
    int rc;

#if defined(PARANOID)
    assert(process->id==0x0badfeed);
#endif /* defined(PARANOID) */

    rc = process_parse_procfs(process);
    if (rc)
	return 1;

    /*
     * Check name against the exclusion list
     */
    if (process->counted && exclusion_expression && !regexec(exclusion_expression,process->name,0,0,0))
	process->counted = 0;

    return 0;
}

/******************************************/
/* Strip dead process entries             */
/******************************************/

void process_cleanup() {

    struct process *p = first_process;
    while (p) {
	struct process *current = p;

#if defined(PARANOID)
	assert(p->id==0x0badfeed);
#endif /* defined(PARANOID) */

	p = p->next;
	/*
	 * Delete processes that have died
	 */
	if (current->time_stamp!=g_time)
	    delete_process(current);
    }
}

/******************************************/
/* Destroy and remove a process           */
/******************************************/

void delete_process(struct process *p) {
#if defined(PARANOID)
    assert(p->id==0x0badfeed);

    /*
     * Ensure that deleted processes aren't reused.
     */
    p->id = 0x007babe;
#endif /* defined(PARANOID) */

    /*
     * Maintain doubly linked list.
     */
    if (p->next)
	p->next->previous = p->previous;
    if (p->previous)
	p->previous->next = p->next;
    else
	first_process = p->next;

    if (p->name)
	wmtop_free(p->name);
    wmtop_free(p);
}

/******************************************/
/* Generate display                       */
/******************************************/

void draw_processes() {
    int i,n;
    struct process *best[3] = { 0, 0, 0 };
    int total;

    /*
     * Invalidate time stamps
     */
    ++g_time;

    update_process_table();
    
    switch (mode) {
    case 0:
	total = calc_cpu_total();
	calc_cpu_each(total);
	break;
#if defined(LINUX)
    case 1:
	total = calc_mem_total();
	calc_mem_each(total);
	break;
#endif
    }

    process_cleanup();

    /*
     * Find the top three!
     */
    n = process_find_top_three(best);

    for (i = 0; i<3; ++i) {
	int j;
	char s[10];
	strcpy(s,"         ");
	if (i<n) {
	    for (j = 0; j<9; ++j) {
		char c;
		c = best[i]->name[j];
		if (c)
		    s[j] = c;
		else
		    break;
	    }
	    draw_bar(0, 97, 55, 6, best[i]->amount, 4, 13+i*20);
	} else
	    draw_bar(0, 97, 55, 6, 0, 4, 13+i*20);
	blit_string(s,4,4+i*20);
    }

#if defined(DEBUG)
    show_memory();
#endif
}

/******************************************/
/* Calculate cpu total                    */
/******************************************/

int calc_cpu_total() {
    int total,t;
    static int previous_total = INT_MAX;
#if defined(LINUX)
    int rc;
    int ps;
    char line[WMTOP_BUFLENGTH];
    int cpu,nice,system,idle;

    ps = open("/proc/stat",O_RDONLY);
    rc = read(ps,line,sizeof(line));
    close(ps);
    if (rc<0)
	return 0;
    sscanf(line,"%*s %d %d %d %d",&cpu,&nice,&system,&idle);
    total = cpu+nice+system+idle;
#endif /* defined(LINUX) */

#if defined(FREEBSD)
    struct timeval tv;

    gettimeofday(&tv,0);
    total = tv.tv_sec*1000+tv.tv_usec/1000;
#endif /* defined(FREEBSD) */

    t = total-previous_total;
    previous_total = total;
    if (t<0)
	t = 0;

    return t;
}

/******************************************/
/* Calculate each processes cpu           */
/******************************************/

void calc_cpu_each(int total) {
    struct process *p = first_process;
    while (p) {

#if defined(PARANOID)
    assert(p->id==0x0badfeed);
#endif /* defined(PARANOID) */

	p->amount = total ? 100*(float)(p->user_time+p->kernel_time)/total : 0;
	p = p->next;
    }
}

/******************************************/
/* Calculate total memory                 */
/******************************************/

#if defined(LINUX)
int calc_mem_total() {
    int ps;
    char line[512];
    char *ptr;
    int rc;

    ps = open("/proc/meminfo",O_RDONLY);
    rc = read(ps,line,sizeof(line));
    close(ps);
    if (rc<0)
	return 0;

    if ((ptr = strstr(line, "Mem:")) == NULL) {
        return 0;
    } else {
        ptr += 4;
        return atoi(ptr);
    }

}
#endif /* defined(LINUX) */

/******************************************/
/* Calculate each processes memory        */
/******************************************/

#if defined(LINUX)
void calc_mem_each(int total) {
    struct process *p = first_process;
    while (p) {
	p->amount = 100*(float)p->rss/total;
	p = p->next;
    }
}
#endif /* defined(LINUX) */

/******************************************/
/* Find the top three processes           */
/******************************************/

/*
 * Result is stored in decreasing order in best[0-2].
 */
int process_find_top_three(struct process **best) {
    struct process *p = first_process;
    int n = 0;

    /*
     * Insertion sort approach to skim top 3
     */
    while (p) {
	if (p->counted && p->amount>0 && (!best[0] || p->amount>best[0]->amount)) {
	    best[2] = best[1];
	    best[1] = best[0];
	    best[0] = p;
	    ++n;
	} else if (p->counted && p->amount>0 && (!best[1] || p->amount>best[1]->amount)) {
	    best[2] = best[1];
	    best[1] = p;
	    ++n;
	} else if (p->counted && p->amount>0 && (!best[2] || p->amount>best[2]->amount)) {
	    ++n;
	    best[2] = p;
	}

	p = p->next;
    }

    return n>3 ? 3 : n;
}

/******************************************/
/* Blit bar at co-ordinates               */
/******************************************/

void draw_bar(int sx, int sy, int w, int h, float percent, int dx, int dy) {
    int tx;

    if (percent<=100)
	tx = w * (float)percent / 100;
    else
	tx = w;

    if (tx>0)
	copyXPMArea(sx, sy, tx, h, dx, dy);
    if (tx<w)
	copyXPMArea(sx+tx, sy+h, w-tx, h, dx+tx, dy);
}

/******************************************/
/* Blit string at co-ordinates            */
/******************************************/

void blit_string(char *name, int x, int y) {
    int	i;
    int	c;
    int	k;

    k = x;
    for ( i = 0; name[i]; i++) {
	c = toupper(name[i]); 
	if (c >= 'A' && c <= 'J') {   
	    c -= 'A';
	    copyXPMArea(c*6,73,6,7,k,y);
	} else if (c>='K' && c<='T') {
	    c -= 'K';
	    copyXPMArea(c*6,81,6,7,k,y);
	} else if (c>='U' && c<='Z') {
	    c -= 'U';
	    copyXPMArea(c*6,89,6,7,k,y);
	} else if (c>='0' && c<='9') { 
	    c -= '0';
	    copyXPMArea(c*6,65,6,7,k,y);
	} else {
	    copyXPMArea(36,89,6,7,k,y);
	}
	k += 6;
    }
}

/******************************************/
/* Usage                                  */
/******************************************/

void usage(void) {
    int i;
    fprintf(stderr,"\nWMtop - Dan Piponi <dan@tanelorn.demon.co.uk>  http://www.tanelorn.demon.co.uk\n\n");
    fprintf(stderr,"usage:\n");
    fprintf(stderr,"    -display <display name>\n");
    fprintf(stderr,"    -geometry +XPOS+YPOS      initial window position\n");
    fprintf(stderr,"    -s <...>                  sample rate in milliseconds (default:%d)\n", update_rate/1000);
    fprintf(stderr,"    -r <...>                  refresh rate in milliseconds (default:%d)\n", refresh_rate/1000);
    fprintf(stderr,"    -U                        display user processes only\n");
    fprintf(stderr,"    -x <...>                  exclude matching processes\n");
    fprintf(stderr,"    -c <...>                  command\n");
#if defined(LINUX)
    fprintf(stderr,"    -m                        display memory usage\n");
#endif /* defined(LINUX) */
    fprintf(stderr,"    -v                        print version number\n");
    fprintf(stderr,"    -a <1..%d>                 select artistic style\n", nstyles);
    fprintf(stderr,"\n");
    fprintf(stderr,"The artistic style is one of:\n");
    for (i = 0; i<nstyles; ++i)
	fprintf(stderr,"  %d - %s\n",i+1,styles[i].description);
}

/******************************************/
/* Print version                          */
/******************************************/

void printversion(void) {
    fprintf(stderr, "wmtop v%s\n",WMTOP_VERSION);
}
