#include <stdio.h>

int i;
FILE *fd;
int temp;
char *buf;


main() {
        buf=(char *)malloc(sizeof(char)*512);
	for (i=0;i<20000;i++) {
		 if ((fd = fopen("/proc/acpi/battery/BAT0/state", "r"))) {
			          bzero(buf, 512);
				           fseek(fd,57,0);
					            fscanf(fd, "charging state: %s", buf);

		 printf("We are at loop %d and charging is %s\n",i,buf);
		 fclose(fd);
		 }
	}
	free(buf);
}

