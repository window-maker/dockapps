#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>

#include "die.h"
#include "download.h"

static CURLM *multi_handle=NULL;
static int still_running=0;
struct download_info {
    CURL *handle;
    FILE *fp;
    void (*callback)(char *filename, void *data);
    char *filename;
    void *data;
    int flags;
    struct download_info *next;
    struct download_info *prev;
};
static struct download_info *active_list=NULL;

static void add_active(struct download_info *d){
    d->next=active_list;
    d->prev=NULL;
    if(active_list!=NULL) active_list->prev=d;
    active_list=d;
}

static void remove_active(struct download_info *d){
    if(active_list==d) active_list=d->next;
    if(d->prev!=NULL) d->prev->next=d->next;
    if(d->next!=NULL) d->next->prev=d->prev;
    d->next=NULL;
    d->prev=NULL;
}

static struct download_info *find_active_file(char *f){
    struct download_info *d;
    for(d=active_list; d!=NULL; d=d->next){
        if(!strcmp(d->filename,f)) return d;
    }
    return NULL;
}

static void handle_done(CURLMsg *msg){
    struct download_info *info;
    long status;

    if(curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &info)!=CURLE_OK || info==NULL){
        warn("Could not retrieve info handle from CURL handle. WTF?");
        for(info=active_list; info && info->handle!=msg->easy_handle; info=info->next);
        if(info==NULL){
            warn("Could not find it in the active list either. WTF?");
            curl_multi_remove_handle(multi_handle, msg->easy_handle);
            curl_easy_cleanup(msg->easy_handle);
            return;
        }
    }

    remove_active(info);
    curl_multi_remove_handle(multi_handle, info->handle);
    fclose(info->fp);

    if(msg->data.result!=CURLE_OK){
        if(msg->data.result==CURLE_HTTP_RETURNED_ERROR){
            if(curl_easy_getinfo(info->handle, CURLINFO_RESPONSE_CODE, &status)!=CURLE_OK) status=600;
            if(status!=404 || !(info->flags&DOWNLOAD_NO_404))
                warn("HTTP download of %s returned %d", info->filename, status);
        } else {
            warn("Download of %s failed: %s", info->filename, curl_easy_strerror(msg->data.result));
        }
        unlink(info->filename);
    } else {
        (*info->callback)(info->filename, info->data);
    }

    curl_easy_cleanup(info->handle);
    free(info->filename);
    free(info);
}


void download_init(char *email){
    if(multi_handle==NULL){
        if(curl_global_init(CURL_GLOBAL_ALL))
            die("Could not initialize CURL");
        multi_handle=curl_multi_init();
        if(multi_handle==NULL) die("Could not create a CURL multihandle");
    }
}

void download_process(unsigned long sleeptime){
    fd_set rd, wr, er;
    struct timeval tv;
    int maxfd, n, x;
    CURLMsg *msg;

    if(sleeptime>0){
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        FD_ZERO(&er);
        curl_multi_fdset(multi_handle, &rd, &wr, &er, &maxfd);

        tv.tv_sec=0;
        tv.tv_usec=sleeptime;
        if(sleeptime>=1000000){
            tv.tv_sec=sleeptime/1000000;
            tv.tv_usec=sleeptime%1000000;
        }

        n=select(maxfd+1, &rd, &wr, &er, &tv);
        if(n==0) return;
        if(n<0){
            switch(errno){
              case EINTR:
              case ENOMEM:
                /* transient errors, hope it's good next time */
                break;

              default:
                warn("WTF? select errno=%d", errno);
                break;
            }
            usleep(sleeptime);
            return;
        }
    }
    while(curl_multi_perform(multi_handle, &x)==CURLM_CALL_MULTI_PERFORM);
    while((msg=curl_multi_info_read(multi_handle, &x))){
        switch(msg->msg){
          case CURLMSG_DONE:
            handle_done(msg);
            break;
          default:
            warn("Unknown CURL message type %d", msg->msg);
            break;
        }
    }
}

int download_kill(char *filename){
    struct download_info *info;
    info=find_active_file(filename);
    if(info==NULL) return ENOENT;
    remove_active(info);
    curl_multi_remove_handle(multi_handle, info->handle);
    warn("Download of %s interrupted", info->filename);
    unlink(info->filename);
    curl_easy_cleanup(info->handle);
    free(info->filename);
    fclose(info->fp);
    free(info);
    return 0;
}

int download_file(char *filename, char *from_addr, char *postdata, int flags, void (*callback)(char *filename, void *data), void *data){
    struct download_info *info=NULL;
    FILE *fp;

    if(callback==NULL || filename==NULL || from_addr==NULL) return 1;

    if(flags&DOWNLOAD_KILL_OTHER_REQUESTS){
        download_kill(filename);
    } else {
        info=find_active_file(filename);
        if(info!=NULL){
            errno=0;
            warn("Cannot download %s: download already in progress", filename);
            return 1;
        }
    }

    if((info=malloc(sizeof(*info)))==NULL){
        warn("Malloc error in download_file");
        goto fail;
    }
    info->handle=NULL;
    info->fp=NULL;
    info->callback=callback;
    info->filename=NULL;
    info->data=data;
    info->flags=flags;
    info->next=NULL;
    info->prev=NULL;

    if((info->filename=strdup(filename))==NULL) goto fail;
    if((info->fp=fopen(info->filename, "wb"))==NULL){
        warn("Error opening %s for output", info->filename);
        goto fail;
    }

    info->handle=curl_easy_init();
    if(info->handle==NULL){
        warn("Error creating a CURL handle");
        goto fail;
    }
    if(curl_easy_setopt(info->handle, CURLOPT_URL, from_addr)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_NOPROGRESS, 1)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_NOSIGNAL, 1)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_WRITEDATA, info->fp)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_FAILONERROR, 1)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_AUTOREFERER, 1)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_FOLLOWLOCATION, 1)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_TIMEOUT, 10*60)!=CURLE_OK ||
       curl_easy_setopt(info->handle, CURLOPT_PRIVATE, info)!=CURLE_OK
      ){
        warn("Error setting CURL options");
        goto fail;
    }
    if(postdata!=NULL){
        if(curl_easy_setopt(info->handle, CURLOPT_COPYPOSTFIELDS, postdata)!=CURLE_OK
          ){
            warn("Error setting CURL post options");
            goto fail;
        }
    }
    if(curl_multi_add_handle(multi_handle, info->handle)!=CURLM_OK){
        warn("Could not add handle for %s to multihandle", info->filename);
        goto fail;
    }
    add_active(info);

    /* Call download_process with 0 to force at least one call to
     * curl_multi_process, because curl won't actually create a socket until
     * that function is called and download_process won't otherwise call
     * curl_multi_process until the socket is created...
     */
    download_process(0);

    return 0;

fail:
    if(info){
        if(info->handle) curl_easy_cleanup(info->handle);
        info->handle=NULL;
        if(info->fp) fclose(info->fp);
        info->fp=NULL;
        if(info->filename){
            unlink(info->filename);
            free(info->filename);
        }
        info->filename=NULL;
        free(info);
    }
    return 1;
}
