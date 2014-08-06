#include "wmmixer-alsa.h"

void init_mixer(void)
{
	int err;
	if((err=snd_mixer_open(&mixer_handle,CARD_NUM,MIXER_NUM))<0)
	{
		fprintf(stderr,"Mixer open failed: %s\n",snd_strerror(err));
		return;
	}
	init_elements();
	return;
}

void init_elements(void)
{
	int err,i;
	count=0;
	memset(&elements,0,sizeof(elements));
	if((err=snd_mixer_elements(mixer_handle,&elements))<0)
	{
		fprintf(stderr,"Elements error: %s\n",snd_strerror(err));
		return;
	}
	elements.pelements=(snd_mixer_eid_t *)malloc(elements.elements_over*sizeof(snd_mixer_eid_t));
	if(!elements.pelements)
	{
		fprintf(stderr,"Elements error: Not enough memory\n");
		return;
	}
	elements.elements_size=elements.elements_over;
	elements.elements_over=elements.elements=0;
	if((err=snd_mixer_elements(mixer_handle,&elements))<0)
	{
		fprintf(stderr,"Elements error: %s\n",snd_strerror(err));
		return;
	}
	for(i=0;i<elements.elements;i++)
	{
		elementinfo *e;
		if(elements.pelements[i].type!=SND_MIXER_ETYPE_VOLUME1)
			continue;
		if(snd_mixer_element_has_control(&elements.pelements[i])!=1)
			continue;
		if(snd_mixer_element_has_info(&elements.pelements[i])!=1)
			continue;
		e=add_element();
		e->element.eid=elements.pelements[i];
		if((err=snd_mixer_element_build(mixer_handle,&e->element))<0)
		{
			fprintf(stderr,"Element error: %s\n",snd_strerror(err));
			return;
		}
		e->info.eid=elements.pelements[i];
		if((err=snd_mixer_element_info_build(mixer_handle,&e->info))<0)
		{
			fprintf(stderr,"Element info error: %s\n",snd_strerror(err));
			return;
		}
		count++;
	}
	free((void *)elements.pelements);
	return;
}

elementinfo *add_element(void)
{
	if(!element)
	{
		element=(elementinfo *)malloc(sizeof(elementinfo));
		element->next=NULL;
		element->prev=NULL;
		memset(&element->info,0,sizeof(element->info));
		memset(&element->element,0,sizeof(element->element));
		return element;
	}
	else
	{
		elementinfo *e;
		e=element;
		while(e->next)
			e=e->next;
		e->next=(elementinfo *)malloc(sizeof(elementinfo));
		e->next->prev=e;
		e=e->next;
		e->next=NULL;
		memset(&e->info,0,sizeof(e->info));
		memset(&e->element,0,sizeof(e->element));
		return e;
	}
}
