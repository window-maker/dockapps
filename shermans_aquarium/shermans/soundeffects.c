

#include "soundeffects.h"
#include "settings.h"
#include "aquarium.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

static Sound_settings sound_settings;

Sound_settings *sound_get_settings_ptr(void)
{
    return &sound_settings;
}

static void sound_exec(char *prg, char *sound)
{
    static int pid=-1;
    char *argv[256]; 
    int argc=0, start=0,i;
    
    if(pid!=-1){
	if(kill(pid,0)==0)
	    return;
    }

    for(i=0;i<strlen(prg)+1;i++){
	if(prg[i]==' ' || prg[i]=='\0'){
	    argv[argc]=g_malloc0(i-start);
	    strncpy(argv[argc],prg+start,i-start);
	    //printf("--argv[%d]=%s--\n",argc,argv[argc]);
	    argc++;
	    start=i+1;
	}
    }

    argv[argc]=g_strdup_printf("%s/%s", aquarium_install_path(), sound);
    argv[argc+1]=NULL;
    g_spawn_async(".", argv, NULL, 
		  G_SPAWN_STDOUT_TO_DEV_NULL|G_SPAWN_STDERR_TO_DEV_NULL|G_SPAWN_SEARCH_PATH,
		  NULL,NULL, &pid, NULL);


    for(i=0;i<=argc;i++){
	g_free(argv[i]);
    }

}


void sound_eatscream(void)
{
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    if(sound_settings.on && !ad->proximity){
	if(sound_settings.type == TYPE_MP3)
	    sound_exec(sound_settings.prg, "sounds/mp3/deathscream.mp3");
	else
	    sound_exec(sound_settings.prg, "sounds/ogg/deathscream.ogg");

    }
}

void sound_explode(void)
{
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    if(sound_settings.on  && !ad->proximity){
	if(sound_settings.type == TYPE_MP3)
	    sound_exec(sound_settings.prg, "sounds/mp3/explode.mp3");
	else
	    sound_exec(sound_settings.prg, "sounds/ogg/explode.ogg");
    }
}

void sound_bubbles(void)
{
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();
    if(sound_settings.on  && !ad->proximity){

	if((rand()%600)<4){
	    if((rand()%10)<4){
		if(sound_settings.type == TYPE_MP3)
		    sound_exec(sound_settings.prg, "sounds/mp3/manybubbles.mp3");
		else
		    sound_exec(sound_settings.prg, "sounds/ogg/manybubbles.ogg");
	    }
	    else{
		if(sound_settings.type == TYPE_MP3)
		    sound_exec(sound_settings.prg, "sounds/mp3/fewbubbles.mp3");
		else
		    sound_exec(sound_settings.prg, "sounds/ogg/fewbubbles.ogg");
	    }
	}
    }
}
