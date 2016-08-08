#if defined (__FreeBSD__)
# include <machine/soundcard.h>
#else
# include <sys/mount.h>
# include <linux/soundcard.h>
#endif

#define DEFAULTMIXERDEVICE "/dev/mixer"

char mixer_device[128]=DEFAULTMIXERDEVICE;
int mixer_ok = 1;
int mixer_control;
int mixer_vol;
int mixer_but;
int mixer_old;
