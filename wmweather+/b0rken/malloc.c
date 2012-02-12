/* Version of malloc that avoids the malloc(0) bug */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#undef malloc

#include <sys/types.h>

void *malloc();

void *rpl_malloc(size_t size){
    if(size==0) size=1;
    return malloc(size);
}
