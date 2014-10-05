int sonypi_supported(void);
int sonypi_read(apm_info *info);

/* There's no good place to get these constants, so I must define them
 * myself. */

#include <linux/types.h>

/* get battery full capacity/remaining capacity */
#define SONYPI_IOCGBAT1CAP	_IOR('v', 2, __u16)
#define SONYPI_IOCGBAT1REM	_IOR('v', 3, __u16)
#define SONYPI_IOCGBAT2CAP	_IOR('v', 4, __u16)
#define SONYPI_IOCGBAT2REM	_IOR('v', 5, __u16)

/* get battery flags: battery1/battery2/ac adapter present */
#define SONYPI_BFLAGS_B1	0x01
#define SONYPI_BFLAGS_B2	0x02
#define SONYPI_BFLAGS_AC	0x04
#define SONYPI_IOCGBATFLAGS	_IOR('v', 7, __u8)
