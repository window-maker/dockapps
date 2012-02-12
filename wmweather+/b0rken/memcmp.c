/* Cheapo version of memcmp */

#include <stdlib.h>

int memcmp(const void *s1, const void *s2, size_t n){
    int i, j;
    
    for(i=0; i<n; i++){
        j=((unsigned char *)s1)[i]-((unsigned char *)s2)[i];
        if(j) return j;
    }
    return 0;
}
