struct animation {
    int do_animate:1;       /* animate, or use min_pct? */
    int show_counter:1;     /* display the percentage counter? */
    unsigned int min_pct:7; /* when not animating, show any occurence with more
                               than this percent chance */
    int changed:1;          /* Set this if you change any of the above */

    int active:1;
    int x, y;
    int sky;
    int obs;
    int vis;
    int items[5];
    int ac;
    double moon;
    unsigned int old_pct:7;
    int pct;
};

void SetAnimation(struct animation *a, int x, int y, int sky, int obs, int vis,
                  int frz, int snow, int rain, int tstorm, int svtstorm,
                  double moon);
void DoAnimation(struct animation *a);
