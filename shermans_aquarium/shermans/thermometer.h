#ifndef THERMOMETER_H
#define THERMOMETER_H

#define STATUS_OFF  		0
#define STATUS_FAN1 		1
#define STATUS_FAN2 		2
#define STATUS_TEMP1 		3
#define STATUS_TEMP2 		4
#define STATUS_SWAP 		5
#define STATUS_DISC 		6
#define STATUS_CPU 		7
#define STATUS_MEM 		8
#define STATUS_NET_ETH0_RECV 	9
#define STATUS_NET_ETH0_SEND 	10
#define STATUS_NET_ETH0_BOTH 	11
#define STATUS_NET_ETH1_RECV 	12
#define STATUS_NET_ETH1_SEND 	13
#define STATUS_NET_ETH1_BOTH 	14
#define STATUS_NET_PPP0_RECV 	15
#define STATUS_NET_PPP0_SEND 	16
#define STATUS_NET_PPP0_BOTH 	17
#define STATUS_NET_LO_RECV   	18
#define STATUS_NET_LO_SEND   	19
#define STATUS_NET_LO_BOTH   	20

#define STATUSES 21

typedef struct
{
    int draw1, draw2;
    int vert1, horz1;
    int vert2, horz2;
    int split1, split2;
    GaiColor c1, c2;
    GaiColor c1_s, c2_s;
    int messure1, messure2;
    int messure1_s, messure2_s;
    int roof1, roof2;
    int roof1_s, roof2_s;
    char *mount_point1, *mount_point2;
    char *mount_point1_s, *mount_point2_s;
    

} Thermometer_settings;

void thermometer_init(void);
void thermometer_update(int);
void thermometer_exit(void);
Thermometer_settings *thermometer_get_settings_ptr(void);

#endif
