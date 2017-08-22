/* DEFINES */

#define START_ACTION (NULL)
#define STOP_ACTION (NULL)
#define SPEED_ACTION (NULL)
#define IFDOWN_ACTION (NULL)

#define STAMP_FILE "/var/run/ppp0.pid"

#define LED_PPP_RX              (1)
#define LED_PPP_TX		(2)
#define LED_PPP_POWER		(3)

#define BUT_V			(1)
#define BUT_X			(2)
#define BUT_REW			(3)
#define BUT_FF  		(4)

#define TIMER_SRC_Y		(176)
#define TIMER_DES_Y		(8)
#define TIMER_DES_X             (7)
#define TIMER_SZE_X             (6)

#define LED_ON_X                (51)
#define LED_ON_Y                (87)
#define LED_OFF_X               (51)
#define LED_OFF_Y               (82)

#define LED_ERR_X               (57)
#define LED_ERR_Y               (82)
#define LED_WTE_X               (57)
#define LED_WTE_Y               (87)
#define LED_SZE_X               (4)
#define LED_SZE_Y               (4)

#define LED_PWR_X               (52)
#define LED_PWR_Y                (9)
#define LED_SND_X               (44)
#define LED_SND_Y                (9)
#define LED_RCV_X               (37)
#define LED_RCV_Y                (9)

#define ISP_BASE_X               (6)
#define ISP_BASE_Y              (35)

#define UPPER_ABC_BASE_X         (1)
#define UPPER_ABC_BASE_Y       (124)
#define LOWER_ABC_BASE_X         (1)
#define LOWER_ABC_BASE_Y       (148)
#define DIGIT_BASE_X            (11)
#define DIGIT_BASE_Y           (164)
#define SPACE_BASE_X            (11)
#define SPACE_BASE_Y           (140)

#define ERR_DEST_X              (34)
#define ERR_DEST_Y              (35)

#define ERR_SRC_X                (0)
#define ERR_SRC_Y               (94)

#define BUT_V_X                 (35)
#define BUT_V_Y                 (48)
#define BUT_X_X                 (47)
#define BUT_X_Y                 (48)
#define BUT_R_X                  (5)
#define BUT_R_Y                 (48)
#define BUT_F_X                 (17)
#define BUT_F_Y                 (48)

#define BUT_V_SRC_X              (0)
#define BUT_V_SRC_Y             (70)
#define BUT_X_SRC_X             (12)
#define BUT_X_SRC_Y             (70)

#define BUT_R_SRC_X              (0)
#define BUT_R_SRC_Y             (82)
#define BUT_F_SRC_X             (12)
#define BUT_F_SRC_Y             (82)

#define BUT_UP_INC              (24)

#define ARR_UP_X                (50)
#define ARR_UP_Y                (70)
#define ARR_DN_X                (57)
#define ARR_DN_Y                (70)

#define ARR_ACTV                (5)

#define ARR_W                   (7)
#define ARR_H                   (4)

#define ORANGE_LED_TIMEOUT (60)

/* prototypes */

void usage (void);
void printversion (void);
void DrawTime (int, int);
void DrawStats (int, int, int, int);
void DrawSpeedInd (char *);
void DrawLoadInd (int);
void DrawISPName (void);

void SetOnLED (int);
void SetErrLED (int);
void SetWaitLED (int);
void SetOffLED (int);

void ButtonUp (int);
void ButtonDown (int);

void yawmppp_routine (int, char **);

int get_statistics (char *, long *, long *, long *, long *);
int stillonline (char *);

void draw_isp_char (int, char);
void grab_isp_info (int);

void sigusr_handler(int signum);
void remove_pid_file(void);
void write_pid_file(void);
void make_config_dir(void);

void run_pref_app(void);
void run_log_app(void);

void add_dns(void);
void remove_dns(void);

void warn_pref(void);
void make_delayed_update(void);

/* logs */
void clean_guards(void);
void make_guards(void);
void write_log(void);

/* pcmcia blues ? */

void open_ppp(void);
void close_ppp(void);
