
/* DEFINES */

#include <gtk/gtk.h>

#define MAX_ISPS 40

#define START_ACTION (NULL)
#define STOP_ACTION (NULL)
#define SPEED_ACTION (NULL)
#define IFDOWN_ACTION (NULL)

#define STAMP_FILE "/var/run/ppp0.pid"

/* leds */

#define LED_PWR_X              (226)
#define LED_PWR_Y                (5)
#define LED_SND_X              (255)
#define LED_SND_Y                (5)
#define LED_RCV_X              (242)
#define LED_RCV_Y                (5)

#define LED_PPP_RX              (1)
#define LED_PPP_TX		(2)
#define LED_PPP_POWER		(3)

#define LED_ON_X                 (1)
#define LED_ON_Y                 (1)
#define LED_OFF_X               (34)
#define LED_OFF_Y                (1)
#define LED_ERR_X               (23)
#define LED_ERR_Y                (1)
#define LED_WTE_X               (12)
#define LED_WTE_Y                (1)
#define LED_SZE_X               (10)
#define LED_SZE_Y               (10)

#define LED_GREEN   (1)
#define LED_RED     (2)
#define LED_YELLOW  (3)
#define LED_DARK    (4)

/* buttons */

#define BUT_V			(1)
#define BUT_X			(2)
#define BUT_REW			(3)
#define BUT_FF  		(4)
#define BUT_CONF   		(5)
#define BUT_LOG  		(6)
#define BUT_KILL                (7)
#define BUT_DRAG                (8)

#define BUT_V_X                 (112)
#define BUT_V_Y                   (5)
#define BUT_X_X                 (124)
#define BUT_X_Y                   (5)
#define BUT_R_X                  (53)
#define BUT_R_Y                   (5)
#define BUT_F_X                  (65)
#define BUT_F_Y                   (5)
#define BUT_C_X                 (273)
#define BUT_C_Y                   (5)
#define BUT_L_X                 (285)
#define BUT_L_Y                   (5)

#define BUT_K_X              (302)
#define BUT_K_Y                (1)

#define BUT_K_SRC_X               (0)
#define BUT_K_SRC_Y              (30)

/* displays */

#define TIMER_SRC_Y		(176)
#define TIMER_DES_Y		(6)
#define TIMER_DES_X             (141)
#define TIMER_SZE_X             (6)

#define ISP_BASE_X               (82)
#define ISP_BASE_Y               (6)

#define ERR_DEST_X              (195)
#define ERR_DEST_Y                (6)

#define ERR_SRC_X                (0)
#define ERR_SRC_Y               (94)

#define UPPER_ABC_BASE_X         (1)
#define UPPER_ABC_BASE_Y       (124)
#define LOWER_ABC_BASE_X         (1)
#define LOWER_ABC_BASE_Y       (148)
#define DIGIT_BASE_X            (11)
#define DIGIT_BASE_Y           (164)
#define SPACE_BASE_X            (11)
#define SPACE_BASE_Y           (140)

#define HIST_SRC_X              (58)
#define HIST_SRC_Y              (92)

#define BUT_V_SRC_X              (0)
#define BUT_V_SRC_Y             (70)
#define BUT_X_SRC_X             (12)
#define BUT_X_SRC_Y             (70)

#define BUT_R_SRC_X              (0)
#define BUT_R_SRC_Y             (82)
#define BUT_F_SRC_X             (12)
#define BUT_F_SRC_Y             (82)

#define BUT_C_SRC_X              (0)
#define BUT_C_SRC_Y             (58)
#define BUT_L_SRC_X             (12)
#define BUT_L_SRC_Y             (58)

#define BUT_UP_INC              (24)

#define ORANGE_LED_TIMEOUT (60)

void create_thinppp(void);
gboolean exposed(GtkWidget *w,GdkEventExpose *gee,gpointer data);
gboolean bpress(GtkWidget *w,GdkEventButton *geb,gpointer data);
gboolean brelease(GtkWidget *w,GdkEventButton *geb,gpointer data);
gboolean bmotion(GtkWidget *w,GdkEventMotion *geb,gpointer data);
gboolean wdestroy(GtkWidget *w,GdkEvent *ev,gpointer data);

void refresh(void);

void setled(int index,int type);
void paste_xpm(int dx,int dy,int sx,int sy,int w,int h);
void DrawISPName (void);
void draw_isp_char (int pos, char letter);
void DrawTime (int i, int j);
void DrawStats (int num, int size, int x_left, int y_bottom);
void PrintLittle (int i, int *k);
void DrawSpeedInd (char *speed_action);
void DrawLoadInd (int speed);

void init_ppp(void);
gint thinppp(gpointer data);

void sigusr_handler(int signum);
void make_delayed_update(void);
void usage (void);
void printversion (void);

void grab_me(void);
void ungrab_me(void);
gboolean inbox(int x,int y,int bx,int by,int bw,int bh);

void read_initial_position(void);
void save_initial_position(void);
