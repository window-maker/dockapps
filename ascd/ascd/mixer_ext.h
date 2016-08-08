#if defined (__FreeBSD__)
# include <machine/soundcard.h>
#else
# include <sys/mount.h>
# include <linux/soundcard.h>
#endif

extern char mixer_device[];
extern int mixer_ok;
extern int mixer_control;
extern int mixer_vol;
extern int mixer_but;
extern int mixer_old;
