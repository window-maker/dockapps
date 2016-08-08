#ifndef FALSE
# define FALSE 0
#endif
#ifndef TRUE
# define TRUE 1
#endif

/* These #defines control the main loop timeouts */
#define RDTIME 50000L
#define RDTIME2 75000L

#define MAX_VOL 255


/*#define ACCTABLE "éÉèÈêÊàÀâÂùÙûÛîÎôÔçÇ**"*/
#define ACCTABLE "éeèeêeàaâaùuûuîiôoçc**"
#define UPACCTABLE "éEèEêEàAâAùUûUîIôOçC**"


#define COUNTER_PANEL 0
#define MSG_PANEL 1
#define TRACK_PANEL 2
#define DB_PANEL 3

/* ---------------------- CD control ---------------------- */

#define PLAY 0
#define PAUSE 1
#define STOP 2
#define UPTRACK 3
#define DNTRACK 4
#define CUE 5
#define REV 6
#define FIRST 7
#define LAST 8
#define LOOP 9
#define DIRECTACCESS 10
#define INTROSCAN 11
#define INTRONEXT 12
#define LOCACCESS 13
#define DIRECTTRACK 14
#define GLOBALACCESS 15

/* CLOSETRAY added 990417 */
#define CLOSETRAY 16

/* new modes added in GMan experimentation. They're
   also used in AScd >= 0.11 */
#define STOPONLY 20
#define EJECT 21

/* ------------------ FAKTORY defines: ------------------- */

#define FAK_BMAX 200             /* max buttons */
#define FAK_CMAX 80              /* max lenght of infos strings */

/* screen elements types: */

#define FAK_PIXMAP           1
#define FAK_COUNTER          2
#define FAK_TRACKNBR         3
#define FAK_MSG              4
#define FAK_DB               5
#define FAK_CD_BAR          10
#define FAK_VCD_BAR         11
#define FAK_ICD_BAR         12
#define FAK_VOL_BAR         13
#define FAK_VVOL_BAR        14
#define FAK_IVOL_BAR        15
#define FAK_MIXER_BAR       16
#define FAK_VMIXER_BAR      17
#define FAK_IMIXER_BAR      18

/* 0132 new pixmap sliders: */

#define FAK_CD_PIX          19
#define FAK_VOL_PIX         20

#define FAK_VVOL_PIX        21
#define FAK_VCD_PIX         22

/* general commands: the reserved range is 0 to 49 */

#define FAK_PANEL_SWITCH     1
#define FAK_QUIT             2
#define FAK_PANEL1           3
#define FAK_PANEL2           4
#define FAK_PANEL3           5
#define FAK_PANEL4           6
#define FAK_PANEL5           7
#define FAK_WINGS            8
#define FAK_COUNTER_MODE     9
#define FAK_TSELECT         10
#define FAK_TNEXT           11
#define FAK_TPREVIOUS       12
#define FAK_FTSELECT        13
#define FAK_FTNEXT          14
#define FAK_FTPREVIOUS      15
#define FAK_SAVE            16
#define FAK_LOAD            17
#define FAK_QREF            20

/* general modes toggles: (not yet supported!!!) */

#define FAK_TOG_AUTOPLAY    30
#define FAK_TOG_AUTOREPEAT  31
#define FAK_TOG_SHOWDB      32
#define FAK_TOG_SHOWARTIST  33
#define FAK_TOG_UPPER       34
#define FAK_TOG_ISKIPS      35

/* CD player commands: the reserved range is 50 to 99 */

#define FAK_CD_PLAY         50
#define FAK_CD_PAUSE        51
#define FAK_CD_STOP         52
#define FAK_CD_EJECT        53
#define FAK_CD_STOPEJECT    54
#define FAK_CD_EJECTQUIT    55

#define FAK_CD_REW          60
#define FAK_CD_FIRST        61
#define FAK_CD_PREVIOUS     62
#define FAK_CD_FWD          65
#define FAK_CD_LAST         66
#define FAK_CD_NEXT         67
#define FAK_CD_DIRECT       68

#define FAK_CD_LSTART       70
#define FAK_CD_LEND         71
#define FAK_CD_LOOP         72
#define FAK_CD_GOLSTART     73
#define FAK_CD_GOLEND       74
#define FAK_CD_LTRACK       75
#define FAK_CD_LTOTRACK     76
#define FAK_CD_LFROMTRACK   77
#define FAK_CD_LCLEAR       78

#define FAK_CD_INTRO        80
#define FAK_CD_FADE         81

/* these two ones are not yet supported: */
#define FAK_CD_RANDOM       82
#define FAK_CD_RMODE        83

#define FAK_CD_MUTE         90
#define FAK_CD_VOLUME       91

/* Mixer commands: the reserved range is 100 to 199 */

#define FAK_MIXER_SET       100
#define FAK_MIXER_50        101
#define FAK_MIXER_75        102
#define FAK_MIXER_100       103
#define FAK_MIXER_0         104
#define FAK_MIXER_LOAD      110
#define FAK_MIXER_SAVE      111

/* -------------------------------------------------------------------- */

typedef struct _XpmIcon {
    Pixmap pixmap;
    Pixmap mask;
    XpmAttributes attributes;
} XpmIcon;

struct fak_button
{
    unsigned int type;
    unsigned int panel;
    char xpm_file[FAK_CMAX];
    char altxpm_file[FAK_CMAX];
    XpmIcon xpm;
    XpmIcon altxpm;
    unsigned int left;
    unsigned int right;
    unsigned int mid;
    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;
    unsigned int arg;
    int icon;
    unsigned int ox;
    unsigned int oy;
};
