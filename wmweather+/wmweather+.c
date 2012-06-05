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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#if TM_IN_SYS_TIME
# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif
#else
#include <time.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

#include "wmweather+.h"
#include "convert.h"
#include "download.h"
#include "dock.h"
#include "die.h"
#include "animation.h"


char *ProgName;
char *bigbuf;
int devnull;
char *monthnames[]={ "", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
char *monthnames2[]={ "", "", "", "", "", "", "JUNE", "JULY", "", "SEPT", "", "", "" };
char *wdaynames[]={ "SUNDAY", "MONDAY", "TUESDAY", "WEDN'SDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
char *directions[]={"VRB", "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};

char *error;
char *unknown_option="Unknown option";

#define F(a) fprintf(stderr, a "\n");

/***************************************************
 * Configuration parameters
 ***************************************************/
char *email_address=NULL;
char *metar_station=NULL;
char *metar_uri=NULL;
char *metar_post=NULL;
char **warning_zones=NULL;
char *warning_uri=NULL;
char *warning_post=NULL;
char *avn_station=NULL;
char *avn_uri=NULL;
char *avn_post=NULL;
char *eta_station=NULL;
char *eta_uri=NULL;
char *eta_post=NULL;
char *mrf_station=NULL;
char *mrf_uri=NULL;
char *mrf_post=NULL;
char *radar_uri=NULL;
char *radar_post=NULL;
char *radar_crop=NULL;
char *radar_cross=NULL;
char *viewer=NULL;
int pressure_mode=0;
int windspeed_mode=0;
int temp_mode=0;
int length_mode=0;
double latitude=999, longitude=999;
int start_do_animation=1;
int starting_mode=0;


/**********************************
 * Prototypes
 **********************************/
void usage(int i) __THROW __attribute__ ((__noreturn__));
void printversion(void);
int readconf(char *file);
int parse_option(char *option, char *value);
char *get_filename(char *file);

/**********************************
 * Functions
 **********************************/

void sigchld(int i){
    while(waitpid(-1, NULL, WNOHANG)>0);
}

int parse_option(char *option, char *value){
    int i;
    void *v;

    errno=0;
    error=unknown_option;
    if(option[0]=='-') option++;
    if(option[0]=='-' && option[1]!='\0' && option[2]!='\0') option++;
    if(option[0]=='\0') return 0;
    if(value!=NULL && value[0]=='\0') value=NULL;

    switch (option[0]){
      case 'a':
        if(!strncmp(option, "avn-", 4)){
            if(!strcmp(option+4, "station")){
                if(value==NULL){
                    error="avn-station given with no station ID";
                    return 0;
                }
                if(avn_station!=NULL) free(avn_station);
                avn_station=strdup(value);
                if(avn_station==NULL) die("avn-station strdup");
                return 2;
            } else if(!strcmp(option+4, "uri")){
                if(value==NULL){
                    error="avn-uri given with no URI";
                    return 0;
                }
                if(avn_uri!=NULL) free(avn_uri);
                avn_uri=strdup(value);
                if(avn_uri==NULL) die("avn-uri strdup");
                if(avn_post!=NULL) free(avn_post);
                avn_post=NULL;
                return 2;
            } else if(!strcmp(option+4, "post")){
                if(value==NULL){
                    error="avn-post given with no data";
                    return 0;
                }
                if(avn_uri==NULL){
                    error="avn-post must come after avn-uri";
                    return 0;
                }
                if(avn_post!=NULL) free(avn_post);
                avn_post=strdup(value);
                if(avn_post==NULL) die("avn-post strdup");
                return 2;
            }
            break;
        } else if(!strcmp(option, "atm")){
            pressure_mode=3;
            return 1;
        } else if(!strcmp(option, "animate")){
            start_do_animation=1;
            return 1;
        }
        break;
      case 'b':
        if(!strcmp(option, "beaufort")){
            windspeed_mode=4;
            return 1;
        }
        break;
      case 'c':
        if(option[1]=='\0') return 2; /* -c handled earlier */
        else if(!strcmp(option, "cm")){
            length_mode=1;
            return 1;
        }
        break;
      case 'd':
        if(!strcmp(option, "display-mode")){
            if(value==NULL){
                error="display-mode given with no mode specified";
                return 0;
            }
            if(!strcasecmp(value, "cur") || !strcasecmp(value, "current")){
                starting_mode=0;
                return 2;
            } else if(!strcasecmp(value, "fcst") || !strcasecmp(value, "forecast")){
                starting_mode=1;
                return 2;
            } else if(!strcasecmp(value, "map") || !strcasecmp(value, "radar")){
                starting_mode=2;
                return 2;
            } else {
                error="display-mode given with unrecognized mode";
                return 0;
            }
        } else if(!strcmp(option, "display")){
            return 1;
        }
        break;
      case 'e':
        if(!strncmp(option, "eta-", 4)){
            if(!strcmp(option+4, "station")){
                if(value==NULL){
                    error="eta-station given with no station ID";
                    return 0;
                }
                if(eta_station!=NULL) free(eta_station);
                eta_station=strdup(value);
                if(eta_station==NULL) die("eta-station strdup");
                return 2;
            } else if(!strcmp(option+4, "uri")){
                if(value==NULL){
                    error="eta-uri given with no URI";
                    return 0;
                }
                if(eta_uri!=NULL) free(eta_uri);
                eta_uri=strdup(value);
                if(eta_uri==NULL) die("eta-uri strdup");
                if(eta_post!=NULL) free(eta_post);
                eta_post=NULL;
                return 2;
            } else if(!strcmp(option+4, "post")){
                if(value==NULL){
                    error="eta-post given with no data";
                    return 0;
                }
                if(eta_uri==NULL){
                    error="eta-post must come after eta-uri";
                    return 0;
                }
                if(eta_post!=NULL) free(eta_post);
                eta_post=strdup(value);
                if(eta_post==NULL) die("eta-post strdup");
                return 2;
            }
            break;
        } else if(option[1]=='\0' || !strcmp(option, "email")){
            if(value==NULL){
                error="-e/email given with no address";
                return 0;
            }
            if(email_address!=NULL) free(email_address);
            email_address=strdup(value);
            if(email_address==NULL) die("email strdup");
            return 2;
        }
        break;
      case 'f':
        if(!strncmp(option, "forget-", 7)){
            if(!strcmp(option+7, "warning-zones")){
                if(warning_zones) free(warning_zones);
                warning_zones=NULL;
                return 1;
            }
            break;
        }
        break;
      case 'h':
        if(!strcmp(option, "hPa")){
            pressure_mode=1;
            return 1;
        }
        break;
      case 'i':
        if(!strcmp(option, "inHg")){
            pressure_mode=0;
            return 1;
        } else if(!strcmp(option, "in")){
            length_mode=0;
            return 1;
        }
        break;
      case 'k':
        if(!strcmp(option, "kph")){
            windspeed_mode=1;
            return 1;
        } else if(!strcmp(option, "knots")){
            windspeed_mode=2;
            return 1;
        }
        break;
      case 'l':
        if(!strcmp(option, "location")){
            if(value==NULL){
                error="location given with no value";
                return 0;
            }
            if(!str2dd(value, &latitude, &longitude)){
                error="location should be of the form \"dd'mm'ssN dd'mm'ssW\" or \"dd.ddddN dd.dddddW\"\n   Note that, if you're using '-location' on the command line, you will need\n   to quote the value, e.g. '-location \"dd.ddddN dd.dddddW\"'";
                return 0;
            }
            if(latitude>90 || latitude<-90 || longitude>180 || longitude<-180){
                error="latitude or longitude out of range";
                return 0;
            }
            return 2;
        }
        break;
      case 'm':
        if(option[1]=='\0' || !strcmp(option, "metric")){
            pressure_mode=windspeed_mode=temp_mode=length_mode=1;
            return 1;
        } else if(!strncmp(option, "metar-", 6)){
            if(!strcmp(option+6, "station")){
                if(value==NULL){
                    error="metar-station given with no station ID";
                    return 0;
                }
                if(metar_station!=NULL) free(metar_station);
                metar_station=strdup(value);
                if(metar_station==NULL) die("metar-station strdup");
                return 2;
            } else if(!strcmp(option+6, "uri")){
                if(value==NULL){
                    error="metar-uri given with no URI";
                    return 0;
                }
                if(metar_uri!=NULL) free(metar_uri);
                metar_uri=strdup(value);
                if(metar_uri==NULL) die("metar-uri strdup");
                if(metar_post!=NULL) free(metar_post);
                metar_post=NULL;
                return 2;
            } else if(!strcmp(option+6, "post")){
                if(value==NULL){
                    error="metar-post given with no data";
                    return 0;
                }
                if(metar_uri==NULL){
                    error="metar-post must come after metar-uri";
                    return 0;
                }
                if(metar_post!=NULL) free(metar_post);
                metar_post=strdup(value);
                if(metar_post==NULL) die("metar-post strdup");
                return 2;
            }
            break;
        } else if(!strncmp(option, "mrf-", 4)){
            if(!strcmp(option+4, "station")){
                if(value==NULL){
                    error="mrf-station given with no station ID";
                    return 0;
                }
                if(mrf_station!=NULL) free(mrf_station);
                mrf_station=strdup(value);
                if(mrf_station==NULL) die("mrf-station strdup");
                return 2;
            } else if(!strcmp(option+4, "uri")){
                if(value==NULL){
                    error="mrf-uri given with no URI";
                    return 0;
                }
                if(mrf_uri!=NULL) free(mrf_uri);
                mrf_uri=strdup(value);
                if(mrf_uri==NULL) die("mrf-uri strdup");
                if(mrf_post!=NULL) free(mrf_post);
                mrf_post=NULL;
                return 2;
            } else if(!strcmp(option+4, "post")){
                if(value==NULL){
                    error="mrf-post given with no data";
                    return 0;
                }
                if(mrf_uri==NULL){
                    error="mrf-post must come after mrf-uri";
                    return 0;
                }
                if(mrf_post!=NULL) free(mrf_post);
                mrf_post=strdup(value);
                if(mrf_post==NULL) die("mrf-post strdup");
                return 2;
            }
            break;
        } else if(!strcmp(option, "mmHg")){
            pressure_mode=2;
            return 1;
        } else if(!strcmp(option, "mph")){
            windspeed_mode=0;
            return 1;
        } else if(!strcmp(option, "mps")){
            windspeed_mode=3;
            return 1;
        } else if(!strcmp(option, "mbar")){
            pressure_mode=1;
            return 1;
        }
        break;
      case 'n':
        if(!strcmp(option, "noradar")){
            if(radar_uri!=NULL) free(radar_uri);
            radar_uri=NULL;
            return 1;
        } else if(!strcmp(option, "noanimate")){
            start_do_animation=0;
            return 1;
        }
        break;
      case 'r':
        if(!strcmp(option, "radar")){
            warn("'radar' is deprecated, please use 'radar-uri' instead");
            return parse_option("radar-uri", value);
        } else if(!strncmp(option, "radar-", 6)){
            if(!strcmp(option+6, "uri")){
                if(value==NULL){
                    error="radar-uri given with no URI";
                    return 0;
                }
                if(radar_uri!=NULL) free(radar_uri);
                radar_uri=strdup(value);
                if(radar_uri==NULL) die("radar-uri strdup");
                if(radar_post!=NULL) free(radar_post);
                radar_post=NULL;
                return 2;
            } else if(!strcmp(option+6, "post")){
                if(value==NULL){
                    error="radar-post given with no data";
                    return 0;
                }
                if(radar_uri==NULL){
                    error="radar-post must come after radar-uri";
                    return 0;
                }
                if(radar_post!=NULL) free(radar_post);
                radar_post=strdup(value);
                if(radar_post==NULL) die("radar-post strdup");
                return 2;
            } else if(!strcmp(option+6, "crop")){
                if(value==NULL){
                    error="radar-crop given with no value";
                    return 0;
                }
                if(radar_crop!=NULL) free(radar_crop);
                radar_crop=strdup(value);
                if(radar_crop==NULL) die("radar-crop strdup");
                return 2;
            } else if(!strcmp(option+6, "cross")){
                if(value==NULL){
                    error="radar-cross given with no value";
                    return 0;
                }
                if(radar_cross!=NULL) free(radar_cross);
                radar_cross=strdup(value);
                if(radar_cross==NULL) die("radar-cross strdup");
                return 2;
            }
            break;
        }
        break;
      case 's':
        if(option[1]=='\0' || !strcmp(option, "station")){
            if(value==NULL){
                error="-s/station given with no value";
                return 0;
            }
            if(parse_option("metar-station", value)==2
               && parse_option("avn-station", value)==2
               && parse_option("eta-station", value)==2
               && parse_option("mrf-station", value)==2)
                return 2;
            return 0;
        }
        break;
      case 't':
        if(!strcmp(option, "tempf")){
            temp_mode=0;
            return 1;
        } else if(!strcmp(option, "tempc")){
            temp_mode=1;
            return 1;
        }
        break;
      case 'v':
        if(option[1]=='\0' || !strcmp(option, "version")){
            printversion();
            exit(0);
        } else if(!strcmp(option, "viewer")){
            if(value==NULL){
                error="viewer given with no value";
                return 0;
            }
            if(viewer!=NULL) free(viewer);
            viewer=strdup(value);
            if(viewer==NULL) die("viewer strdup");
            return 2;
        }
        break;
      case 'w':
        if(!strncmp(option, "warning-", 8)){
            if(!strcmp(option+8, "zone")){
                if(value==NULL){
                    error="warning-zone given with no zone ID";
                    return 0;
                }
                if(warning_zones!=NULL){
                    for(i=0; warning_zones[i]!=NULL; i++);
                } else {
                    i=0;
                }
                v=realloc(warning_zones, sizeof(*warning_zones)*(i+2));
                if(v==NULL) die("warning-zone realloc");
                warning_zones=v;
                warning_zones[i+1]=NULL;
                warning_zones[i]=strdup(value);
                if(warning_zones[i]==NULL) die("warning-zone strdup");
                return 2;
            } else if(!strcmp(option+8, "uri")){
                if(value==NULL){
                    error="warning-uri given with no URI";
                    return 0;
                }
                if(warning_uri!=NULL) free(warning_uri);
                warning_uri=strdup(value);
                if(warning_uri==NULL) die("warning-uri strdup");
                if(warning_post!=NULL) free(warning_post);
                warning_post=NULL;
                return 2;
            } else if(!strcmp(option+8, "post")){
                if(value==NULL){
                    error="warning-post given with no data";
                    return 0;
                }
                if(warning_uri==NULL){
                    error="warning-post must come after warning-uri";
                    return 0;
                }
                if(warning_post!=NULL) free(warning_post);
                warning_post=strdup(value);
                if(warning_post==NULL) die("warning-post strdup");
                return 2;
            }
            break;
        }
        break;
      case 'z':
        if(!strcmp(option, "zone")){
            warn("'zone' is deprecated, please use 'warning-zone' instead");
            return parse_option("warning-zone", value);
        }
        break;
      default:
        break;
    }
    return 0;
}

int readconf(char *file){
    char *c, *d;
    int i, l, flag=1;
    FILE *fp;
    
    if(file==NULL){
        flag=0;
        file=get_filename("conf");
    }

    if((fp=fopen(file, "r"))==NULL){
        if(!flag){
            free(file);
            return 0;
        }
        return 1;
    }

    l=0;
    while(fgets(bigbuf, BIGBUF_LEN, fp)!=NULL){
        l++;
        for(i=strlen(bigbuf)-1; i>=0; i--){
            if (!isspace(bigbuf[i])) break;
            bigbuf[i]='\0';
        }
        c=bigbuf+strspn(bigbuf, " \t");
        if(*c=='#' || *c=='\0') continue;
        d=c+strcspn(c, " \t");
        if(*d=='\0') d=NULL;
        else {
            *d='\0';
            d++;
            d+=strspn(d+1, " \t");
            if(*d=='\0') d=NULL;
        }
        if(!parse_option(c, d)){
            warn("%s[%d]: %s", file, l, error);
            return 2;
        }
    }
    fclose(fp);
    if(!flag) free(file);
    return 0;
}


int check_dir(void){
    char *c;
    struct stat statbuf;
    int i;

    c=get_filename("");
    i=stat(c, &statbuf);
    if(i<0){
        if(errno==ENOENT){
            if(mkdir(c, 0777)<0) die("Couldn't create directory %s", c);
            errno=0;
            warn("Created directory %s", c);
            i=stat(c, &statbuf);
        }
        if(i<0) die("Couldn't stat %s", c);
    }
    if(!S_ISDIR(statbuf.st_mode)) die("%s is not a directory", c);
    free(c);
    c=get_filename(".dir-test");
    if(unlink(c)<0 && errno!=ENOENT) die("Couldn't delete %s", c);
    if((i=stat(c, &statbuf))!=-1 || errno!=ENOENT){
        if(i!=-1) errno=EEXIST;
        die("Couldn't verify nonexistence of %s", c);
    }
    if((i=creat(c, 0))<0) die("Couldn't create %s", c);
    close(i);
    if(stat(c, &statbuf)<0) die("Couldn't stat %s", c);
    unlink(c);
    free(c);
    return 1;
}

void sigint(int i){
    exit(0);
}

int main(int argc, char **argv){
    int i, j;
    char *c;
    
    ProgName = argv[0];
    if((c=strrchr(ProgName, '/'))!=NULL && *(c+1)!='\0'){
        ProgName=c+1;
    }

    if((bigbuf=malloc(BIGBUF_LEN))==NULL) die("bigbuf malloc");
    check_dir();

    devnull=open("/dev/null", O_RDWR);
    if(devnull<0) die("opening /dev/null");
    /* Parse Command Line */

    c=NULL;
    for(i=1;i<argc;i++){
        if(!strcmp(argv[i], "-c")){
            i++;
            if(!(i<argc)){
                F("-c given with no value");
                exit(1);
            }
            c=argv[i];
            break;
        }
    }
    if(readconf("/etc/wmweather+.conf")>1) exit(1);
    if((i=readconf(c))==1) warn("Couldn't open %s", c);
    if(i) exit(1);

    for(i=1;i<argc;i++){
        char *arg = argv[i];

        if(*arg=='-'){
            j=parse_option(argv[i], (i+1<argc)?argv[i+1]:NULL);
            if(j==0){
                if(error==unknown_option) usage(1);
                fprintf(stderr, "%s\n", error);
                exit(1);
            }
            i+=j-1;
        }
    }

    {
        struct sigaction act;
        sigemptyset(&act.sa_mask);
        act.sa_handler=sigchld;
        act.sa_flags=SA_RESTART;
        sigaction(SIGCHLD, &act, NULL);
        act.sa_handler=sigint;
        sigaction(SIGINT, &act, NULL);
    }

    i=0;
    if(metar_station==NULL){
        i=1;
        F("Please specify a METAR station.\n   See http://www.nws.noaa.gov/tg/siteloc.shtml\n");
    }
    if(latitude==999){{
        time_t t;
        int flag=0;

        /* note: if time_t isn't an arithmetic type, mkgmtime is screwed
         * anyway. So t=0 is as appropriate as anything else. */
        t=0;
        longitude=-mkgmtime(localtime(&t))/240.0;
        latitude=0;
        if(longitude<0){
            flag=1;
            longitude=-longitude;
        }
        fprintf(stderr, "-location not given. Guessing you're at 0N %d'%d'%d%c\n", (int)longitude, (int)(longitude*60)%60, (int)(longitude*3600)%60, flag?'E':'W');
        if(flag) longitude=-longitude;
    }} else if(latitude>89.8){
        F("Latitude greater then 89.9N automatically truncated.\n");
        latitude=89.8;
    } else if(latitude<-89.8){
        F("Latitude greater then 89.9S automatically truncated.\n");
        latitude=-89.8;
    }
    if(i) exit(1);
    if(viewer==NULL) viewer="xless";
    if(metar_uri==NULL) metar_uri="http://weather.noaa.gov/pub/data/observations/metar/stations/%s.TXT";
    if(avn_uri==NULL) avn_uri="http://www.nws.noaa.gov/cgi-bin/mos/getmav.pl?sta=%s";
    if(eta_uri==NULL) eta_uri="http://www.nws.noaa.gov/cgi-bin/mos/getmet.pl?sta=%s";
    if(mrf_uri==NULL) mrf_uri="http://www.nws.noaa.gov/cgi-bin/mos/getmex.pl?sta=%s";
    if(warning_uri==NULL) warning_uri="http://weather.noaa.gov/pub/data/watches_warnings/%f/%.2z/%z.txt";

    download_init(email_address);
    init_dock(argc, argv);
    while(1){
        update_dock();
        download_process(100000);
    }
}

void usage(int i) {
    F("Option         Value      Description");
    F("------         -----      -----------");
    F("-c             <file>     Specify a configuration file");
    F("-e             <address>  Alias for -email");
    F("-email         <address>  Specify anonymous FTP password");
    F("-location      <lat+lon>  Specify latitude and longitude. See manpage.");
    F("-v                        Alias for -version");
    F("-version                  Display version number");
    F("-viewer        <program>  Program to display text from stdin");
    F("-[no]animate              Turn animation on or off");
    F("");
    F("-s             <ID>       Alias for -station");
    F("-station       <ID>       Station ID (all stations)");
    F("-metar-station <ID>       Station ID for METAR observations");
    F("-avn-station   <ID>       Station ID for AVN forecasts");
    F("-eta-station   <ID>       Station ID for ETA forecasts");
    F("-mrf-station   <ID>       Station ID for MRF forecasts");
    F("-warning-zone  <zoneID>   Zone ID for weather warnings");
    F("-*-uri         <URI>      URI for the weather data (see docs for details)");
    F("-*-post        <DATA>     Post data for the weather data (see docs)");
    F("  '*' can be metar, avn, eta, mrf, warning");
    F("-noradar                  Do not display a radar image.");
    F("-radar-uri     <URI>      URI for radar image");
    F("-radar-post    <DATA>     Post data for radar image");
    F("-radar-crop    <string>   How to crop the radar image. XxY+W+H format.");
    F("-radar-cross   <string>   Where to draw radar crosshairs. XxY format.");
    F("");
    F("-m                        Alias for -metric");
    F("-metric                   Same as -cm -hPa -kph -tempc");
    F("");
    F("-in                       Display precipitation amounts in inches");
    F("-cm                       Display precipitation amounts in centimeters");
    F("");
    F("-inHg                     Display pressure in inHg");
    F("-hPa                      Display pressure in hPa (millibars)");
    F("-mbar                     Alias for -hPa");
    F("-mmHg                     Display pressure in mmHg");
    F("-atm                      Display pressure in atmospheres");
    F("");
    F("-mph                      Display windspeed in miles/hour");
    F("-kph                      Display windspeed in kilometers/hour");
    F("-knots                    Display windspeed in knots");
    F("-mps                      Display windspeed in meters/second");
    F("-beaufort                 Display windspeed on the Beaufort scale");
    F("");
    F("-tempf                    Display temperature in degrees Fahrenheit");
    F("-tempc                    Display temperature in degrees Celcius");

    exit(i);
}

void printversion(void) {
    fprintf(stderr, "%s\n", VERSION);
}

char *get_filename(char *file){
    char *f, *c;

    c=getenv("HOME");
    if((f=malloc(strlen(c)+strlen(file)+14))==NULL) die("get_filename");
    strcpy(f, c);
    strcat(f, "/.wmweather+/");
    strcat(f, file);
    return f;
}

char *get_pid_filename(char *file){
    char *f, *c;
    static unsigned short seq=0;
    char buf[15];

    snprintf(buf, sizeof(buf), "%08X.%04X-", getpid(), seq++);
    c=getenv("HOME");
    if((f=malloc(strlen(c)+strlen(file)+14+14))==NULL) die("get_pid_filename");
    strcpy(f, c);
    strcat(f, "/.wmweather+/");
    strcat(f, buf);
    strcat(f, file);
    return f;
}
