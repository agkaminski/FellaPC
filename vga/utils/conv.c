#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) { perror(argv[1]); return 1; }
	char c[2];
	size_t cnt = 0;
	while (1) {
		memset(c, 0, sizeof(c));
		ssize_t ret = read(fd, c, sizeof(c));
		if (ret <= 0) break;
		uint16_t data = c[1] << 8 | c[0];
		printf("0x%04x, ", data);
		if (++cnt > 7) { cnt = 0; printf("\n"); }
	}
	printf("\n");
	
	return 0;
}
