#include <stdio.h>

int main(void) {

	FILE	*fd;
	char	temp[256];

	fd = popen("tac /etc/ppp/connect-errors | grep '['CONNECT'|'CARRIER']' | head -1", "r");

	while (fgets(temp, 256, fd)) {
		printf("%s", temp);
	}

	pclose(fd);

	return 0;
}
