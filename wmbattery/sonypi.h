int sonypi_supported(void);
int sonypi_read(apm_info *info);

/* There's no good place to get these constants, so I must define them
 * myself. */

/* get battery full capacity/remaining capacity */
#define SONYPI_IOCGBAT1CAP	_IOR('v', 2, uint16_t)
#define SONYPI_IOCGBAT1REM	_IOR('v', 3, uint16_t)
#define SONYPI_IOCGBAT2CAP	_IOR('v', 4, uint16_t)
#define SONYPI_IOCGBAT2REM	_IOR('v', 5, uint16_t)

/* get battery flags: battery1/battery2/ac adapter present */
#define SONYPI_BFLAGS_B1	0x01
#define SONYPI_BFLAGS_B2	0x02
#define SONYPI_BFLAGS_AC	0x04
#define SONYPI_IOCGBATFLAGS	_IOR('v', 7, uint8_t)
