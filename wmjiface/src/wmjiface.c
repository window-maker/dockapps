#include "jDockApp/jDockApp.h"
#include "jDockApp/colors.h"
#include <getopt.h>

int delay;

int skip = 0;
int out  = 0;
int hw1  = 2000;
int hw2  = 4000;
int showboth = 0;
int use_expavg;

double alpha = 0.4;
double beta  = 1.0;
double exp_avg[2][5];

void show_help() {
    printf("wmjiface version %s was built on %s.\n\n", VERSION, BDATE);
    printf("-s <sec>:  Seconds between executions of ifacechk\n");
    printf("-o      :  Show the traffic headed out instead.\n");
    printf("-D      :  Doommaker's Show-Both (in&out traf simultaneously).\n");
    printf("-A      :  Use approximate exponential averaging.\n");
    printf("-a <num>:  Alpha for the exp. avg. {default %f}.\n", alpha);
    printf("-b <num>:  Beta: subst for alpha if bps==0 {default %f}.\n", beta);
    printf("-l <num>:  Skip <num> lines before showing the first device.\n");
    printf("-1 <num>:  High-water-mark 1 (red color) {default %i B/s}.\n",hw1);
    printf("-2 <num>:  High-water-mark 2 (red color) {default %i B/s}.\n",hw2);

    exit(0);
}

void setup(int argc, char** argv) {
    //char c;
    int  c;  // thanx to a PowerPC user named Carine Bournez
    int  i = 0;

    while( -1 != (c = getopt(argc, argv, "hoADua:b:s:l:1:2:"))) {
        switch(c) {
            case '?': exit(1);
            case 'h': show_help();

            case 'o': out          = 1; break;
            case 'A': use_expavg   = 1; break;
            case 'D': showboth     = 1; break;

            case 'a': alpha=atof(optarg); break;
            case 'b':  beta=atof(optarg); break;

            case 's':    i=atoi(optarg); break;
            case 'l': skip=atoi(optarg); break;

            case '1': hw1=atoi(optarg); break;
            case '2': hw2=atoi(optarg); break;
        }
    }
    delay = (i) ? i : 1;
    set_update_delay(delay);   /* seconds */
    set_loop_delay(1000);      /* mu seconds */
}

void clear_avg() {
    int i;
    for(i=0; i<5; i++) {
        exp_avg[0][i] = 0;
        exp_avg[1][i] = 0;
    }
}

void do_print_one_dev(char iface[5], char io, int bps_, int row) {
    int color;
    char letter;
    float display;

    jpprintf(0, row, GREEN, "%c", io);
    if(iface[0] >= '0' && iface[0] <= '9') {
        jprintf(RED, "%s", "unkn");
    } else {
        jprintf(BLUE, "%s", iface);
    }
    jprintf(CYAN, ":", io);

    if(bps_ >= 0) {
        if(bps_ > 1000) { display = (bps_)/1024.0; letter  = 'k'; }
        else            { display = bps_;          letter  = ' '; }

        display += 0.5;  /* Round Up */

             if(!bps_)      {                    color = BLUE;   }
        else if(bps_ < hw1) { /* 2000 default */ color = GREEN;  }
        else if(bps_ < hw2) { /* 4000 default */ color = YELLOW; }
        else                {                    color = RED;    }

        if(letter=='k' && display<100) {
            jpprintf(6, row, color, "%.1f", display);
        } else {
            jpprintf(6, row, color, "%.0f", display);
        }

        jprintf(ORANGE, "%c", letter);
    } else {
        jprintf(RED, "err");
        clear_avg();
    }
}

#define IN    0
#define OUT   1

#define Fal0d(X) ((X) ? alpha : beta)

void do_update() {
    FILE *f = popen("ifacechk", "r");
    char iface[5];
    int bps_in, bps_out;
    int row = 2 + skip;
    int c = 0;
    int toshow_in, toshow_out;

    clear_window();
    jpprintf(0, 0, YELLOW, "    J-Iface");  /* title */
    jpprintf(0, 1, PINK,  "Dev:");   /* desc line */
    jpprintf(5, 1, PINK, "bits/s");

    while(1 + fscanf(f, "%s%i%i", iface, &bps_in, &bps_out)) {
        if(use_expavg) {
            exp_avg[IN][c] =
                (Fal0d(bps_in)*bps_in) + (1-Fal0d(bps_in))*exp_avg[IN][c];
            exp_avg[OUT][c] =
                (Fal0d(bps_out)*bps_out) + (1-Fal0d(bps_out))*exp_avg[OUT][c];
            toshow_in  = exp_avg[IN ][c];
            toshow_out = exp_avg[OUT][c];
        } else {
            toshow_in  = bps_in;
            toshow_out = bps_out;
        }

        if(showboth) {
            if(row > 4) break;
            do_print_one_dev(iface, 'i', toshow_in,  row++);
            do_print_one_dev(iface, 'o', toshow_out, row++);
        } else {
            if(row > 5) break;
            do_print_one_dev(
                iface,
                (out) ? 'o':'i',
                (out) ? toshow_out : toshow_in,
                row++
            );
        }
        c++;
    }

    pclose(f);
}

void do_expose() {
    do_update();
}

void do_button_release() {
    do_update();
}
