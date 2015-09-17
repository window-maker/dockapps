#include "include/common.h"
#include "include/misc.h"
#include "include/mixer-alsa.h"

#include <alsa/asoundlib.h>

static bool get_mixer_state(void);

struct mixer_element {
    const char *name;		/* name of channel */
    const char *sname;		/* short name of the channel */
    snd_mixer_elem_t *element;  /* Mixer element pointer */
    long min;                   /* Min volume */
    long max;                   /* Max volume */
    int dev;			/* channel device number */
    int prev_dev_lr_volume;	/* last known left/right volume
				 * (in device format) */
    float volume;		/* volume, in [0, 1] */
    float balance;		/* balance, in [-1, 1] */
    bool has_playback;
    bool has_playback_switch;
    bool has_capture;
    bool has_capture_switch;
    bool is_recording;		/* is it recording? */
    bool is_stereo;		/* capable of stereo? */
    bool is_muted;		/* is it muted? */
    int (*get_volume)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long *);
    int (*set_volume)(snd_mixer_elem_t *, snd_mixer_selem_channel_id_t, long);
};

static snd_mixer_t *mixer;
static struct mixer_element *element;
static int cur_element = 0;
static int n_elements;
static bool needs_update = true;

static int elem_callback(__attribute__((unused)) snd_mixer_elem_t *elem,
                         __attribute__((unused)) unsigned int mask)
{
    needs_update = true;
    return 0;
}

static int mixer_callback(__attribute__((unused)) snd_mixer_t *ctl,
                          unsigned int mask,
			  snd_mixer_elem_t *elem)
{
    if (mask & SND_CTL_EVENT_MASK_ADD) {
        snd_mixer_elem_set_callback(elem, elem_callback);
        needs_update = true;
    }
    return 0;
}

static bool is_exclude(const char *short_name,
                       const char *exclude[])
{
    for (int i = 0; exclude[i] != NULL; i++) {
        if (!strcmp(short_name, exclude[i])) {
            return true;
        }
    }
    return false;
}

void mixer_alsa_init(const char *mixer_device, bool verbose, const char * exclude[])
{
    int err;
    static struct snd_mixer_selem_regopt selem_regopt = {
        .ver = 1,
        .abstract = SND_MIXER_SABSTRACT_NONE,
    };
    selem_regopt.device = mixer_device;
    if (verbose) {
        printf("Sound card: %s\n", mixer_device);
        puts("Supported elements:");
    }

    if ((err = snd_mixer_open(&mixer, 0)) < 0) {
        fprintf(stderr, "snd_mixer_open error");
        mixer = NULL;
        return;
    }

    if ((err = snd_mixer_selem_register(mixer, &selem_regopt, NULL)) < 0) {
        fprintf(stderr, "snd_mixer_selem_register error");
        snd_mixer_close(mixer);
        mixer = NULL;
        return;
    }

    snd_mixer_set_callback(mixer, mixer_callback);

    if ((err = snd_mixer_load(mixer)) < 0) {
        fprintf(stderr, "snd_mixer_load error");
        snd_mixer_close(mixer);
        mixer = NULL;
        return;
    }

    int all_elements = snd_mixer_get_count(mixer);
    element = (struct mixer_element *)malloc(all_elements *
                                             sizeof(struct mixer_element));
    memset(element, 0, all_elements * sizeof(struct mixer_element));
    snd_mixer_elem_t *elem = snd_mixer_first_elem(mixer);

    int f = 0;
    for (int e = 0; e < all_elements; e++) {
        element[f].element = elem;
        element[f].name = snd_mixer_selem_get_name(elem);

        if (is_exclude(element[f].name, exclude)) {
            if (verbose)
                printf("  x: '%s' - disabled\n", element[f].name);
            elem = snd_mixer_elem_next(elem);
            continue;
        }

        element[f].sname = element[f].name;

        element[f].has_capture = snd_mixer_selem_has_capture_volume(elem);
        element[f].has_capture &= snd_mixer_selem_has_capture_channel(elem, SND_MIXER_SCHN_FRONT_LEFT);

        element[f].has_playback = snd_mixer_selem_has_playback_volume(elem);
        element[f].has_playback &= snd_mixer_selem_has_playback_channel(elem,SND_MIXER_SCHN_FRONT_LEFT);

        if (element[f].has_playback) {
            snd_mixer_selem_get_playback_volume_range(elem, &(element[f].min), &(element[f].max));
            element[f].is_stereo = !snd_mixer_selem_is_playback_mono(elem);
            element[f].is_stereo &= snd_mixer_selem_has_playback_channel(elem,SND_MIXER_SCHN_FRONT_RIGHT);
            element[f].get_volume = snd_mixer_selem_get_playback_volume;
            element[f].set_volume = snd_mixer_selem_set_playback_volume;
            element[f].has_playback_switch = snd_mixer_selem_has_playback_switch(elem);
        }
        else if (element[f].has_capture) {
            snd_mixer_selem_get_capture_volume_range(elem, &(element[f].min), &(element[f].max));
            element[f].is_stereo = !snd_mixer_selem_is_capture_mono(element[f].element);
            element[f].is_stereo &= snd_mixer_selem_has_capture_channel(elem,SND_MIXER_SCHN_FRONT_RIGHT);
            element[f].get_volume = snd_mixer_selem_get_capture_volume;
            element[f].set_volume = snd_mixer_selem_set_capture_volume;
            element[f].has_capture_switch = snd_mixer_selem_has_capture_switch(elem);
        } else {
            elem = snd_mixer_elem_next(elem);
            continue;
        }

        if (verbose) {
            printf("  %d: '%s'%s%s%s%s %s (%ld - %ld)\n",
                   f,
                   element[f].name,
                   element[f].has_playback? " pvolume" : "",
                   element[f].has_playback_switch? " pswitch" : "",
                   element[f].has_capture? " cvolume" : "",
                   element[f].has_capture_switch? " cswitch" : "",
                   element[f].is_stereo? "Stereo" : "Mono",
                   element[f].min, element[f].max);
        }

        elem = snd_mixer_elem_next(elem);
        f++;
    }
    n_elements = f;
    get_mixer_state();
}

static bool element_is_muted(int e)
{
    if (!element[e].has_playback_switch)
        return false;

    snd_mixer_elem_t *elem = element[e].element;
    int left_on;
    snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &left_on);
    if (left_on)
        return false;
    if (element[e].is_stereo) {
        int right_on;
        snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, &right_on);
        if (right_on)
            return false;
    }
    return true;
}

static bool element_is_recording(int e)
{
    if (!element[e].has_capture_switch)
        return false;

    snd_mixer_elem_t *elem = element[e].element;
    int left_on;
    snd_mixer_selem_get_capture_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &left_on);
    if (left_on)
        return true;
    if (element[e].is_stereo) {
        int right_on;
        snd_mixer_selem_get_capture_switch(elem, SND_MIXER_SCHN_FRONT_RIGHT, &right_on);
        if (right_on)
            return true;
    }
    return false;
}

static bool get_mixer_state(void)
{
    if (!needs_update)
        return false;
    needs_update = false;

    for (int e = 0; e < n_elements; e++) {
        snd_mixer_elem_t *elem = element[e].element;
        long left, right;

        element[e].get_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &left);
        float fleft = 1.0 * (left-element[e].min) / (element[e].max-element[e].min);
        if (element[e].is_stereo) {
            element[e].get_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &right);
            float fright = 1.0 * (right-element[e].min) /
                (element[e].max-element[e].min);
            lr_to_vb(fleft, fright, &(element[e].volume), &(element[e].balance));
        } else {
            element[e].volume = fleft;
            element[e].balance = 0.0;
        }

        element[e].is_muted = element_is_muted(e);
        element[e].is_recording = element_is_recording(e);

        //printf("Channel %s, has_vol: %d, stereo: %d, muted: %d, max: %ld, min: %ld, left: %ld, right: %ld, vol: %f, bal: %f\n", element[e].name, element[e].has_playback, element[e].is_stereo, element[e].is_muted, element[e].max, element[e].min, left, element[e].is_stereo?right:-1, element[e].volume, element[e].balance);            
    }
    return true;
}

static void set_mixer_state(void)
{
    float left, right;
    long dev_left_volume, dev_right_volume;
    if (!element[cur_element].has_playback && !element[cur_element].has_capture)
        return;

    bool muted = element_is_muted(cur_element);
    if (muted != element[cur_element].is_muted) {
        snd_mixer_selem_set_playback_switch_all(element[cur_element].element,
                                                element[cur_element].is_muted?
                                                0:1);
    }

    bool recording = element_is_recording(cur_element);
    if (recording != element[cur_element].is_recording) {
        snd_mixer_selem_set_capture_switch_all(element[cur_element].element,
                                               element[cur_element].is_recording?
                                               1:0);
    }

    if (element[cur_element].is_stereo)
        vb_to_lr(element[cur_element].volume,
                 element[cur_element].balance, &left, &right);
    else
        left = element[cur_element].volume;

    long range = element[cur_element].max - element[cur_element].min;
    dev_left_volume = element[cur_element].min + (long) (range * left);
    element[cur_element].set_volume(element[cur_element].element,
                                    SND_MIXER_SCHN_FRONT_LEFT,
                                    dev_left_volume);
    if (element[cur_element].is_stereo) {
        dev_right_volume = element[cur_element].min + (long) (range * right);
        element[cur_element].set_volume(element[cur_element].element,
                                        SND_MIXER_SCHN_FRONT_RIGHT,
                                        dev_right_volume);
    }
}

bool mixer_alsa_is_changed(void)
{
    return get_mixer_state();
}

int mixer_alsa_get_channel_count(void)
{
    return n_elements;
}

int mixer_alsa_get_channel(void)
{
    return cur_element;
}

const char *mixer_alsa_get_channel_name(void)
{
    return element[cur_element].name;
}

const char *mixer_alsa_get_short_name(void)
{
    return element[cur_element].sname;
}

void mixer_alsa_set_channel(int element)
{
    assert((element >= 0) && (element < n_elements));

    cur_element = element;
    get_mixer_state();
}

void mixer_alsa_set_channel_rel(int delta_element)
{
    cur_element = (cur_element + delta_element) % n_elements;
    if (cur_element < 0)
	cur_element += n_elements;
    get_mixer_state();
}

float mixer_alsa_get_volume(void)
{
    get_mixer_state();
    return element[cur_element].volume;
}

void mixer_alsa_set_volume(float volume)
{
    assert((volume >= 0.0) && (volume <= 1.0));
    element[cur_element].volume = volume;
    set_mixer_state();
}

void mixer_alsa_set_volume_rel(float delta_volume)
{
    element[cur_element].volume += delta_volume;
    element[cur_element].volume = CLAMP(element[cur_element].volume, 0.0, 1.0);
    set_mixer_state();
}

float mixer_alsa_get_balance(void)
{
    get_mixer_state();
    return element[cur_element].balance;
}

void mixer_alsa_set_balance(float balance)
{
    assert((balance >= -1.0) && (balance <= 1.0));
    if (element[cur_element].is_stereo) {
	element[cur_element].balance = balance;
	set_mixer_state();
    }
}

void mixer_alsa_set_balance_rel(float delta_balance)
{
    if (element[cur_element].is_stereo) {
	element[cur_element].balance += delta_balance;
	element[cur_element].balance =
	    CLAMP(element[cur_element].balance, -1.0, 1.0);
	set_mixer_state();
    }
}

void mixer_alsa_toggle_mute(void)
{
    if (element[cur_element].has_playback_switch) {
        element[cur_element].is_muted ^= 1;
        set_mixer_state();
    }
}

void mixer_alsa_toggle_rec(void)
{
    if (element[cur_element].has_capture_switch) {
        element[cur_element].is_recording ^= 1;
        set_mixer_state();
    }
}

bool mixer_alsa_is_muted(void)
{
    return element[cur_element].is_muted;
}

bool mixer_alsa_is_stereo(void)
{
    return element[cur_element].is_stereo;
}

bool mixer_alsa_is_rec(void)
{
    return element[cur_element].is_recording;
}

bool mixer_alsa_can_rec(void)
{
    return element[cur_element].has_capture;
}

void mixer_alsa_tick(void)
{
    snd_mixer_handle_events(mixer);
}
