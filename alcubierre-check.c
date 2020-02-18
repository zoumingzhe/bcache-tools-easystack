#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	int disk_fd;
	char buf[11] = {0};
	ssize_t count;
	char bdev_name[100] = {0};
	char escache_dir[100] = {0};
	int ret;

	if(argc != 2) {
		printf("Usage: alcubierre-check NODE\n");
		return -1;
	}

	disk_fd = open(argv[1], O_RDONLY);
	if(disk_fd == -1) {
		printf("Can not open device %s\n", argv[1]);
		return -1;
	}

	count = read(disk_fd, buf, 10);
	if(count != 10) {
		printf("Can not read device %s\n", argv[1]);
		close(disk_fd);
		return -1;
	}

	if(!strcmp(buf, "alcubierre")) {
		printf("ALCUBIERRE_DEV=yes\n");
	} else {
		printf("ALCUBIERRE_DEV=no\n");
	}

	ret = sscanf(argv[1], "/dev/%s", bdev_name);
	if (ret != 1) {
		printf("Can not parse '/dev/bdev_name' from %s\n", argv[1]);
		close(disk_fd);
		return -1;
	}

	sprintf(escache_dir, "/sys/block/%s/escache/set", bdev_name);
	if (access(escache_dir, F_OK) != -1)
		printf("ALCUBIERRE_REGISTERED=yes\n");
	else
		printf("ALCUBIERRE_REGISTERED=no\n");

	close(disk_fd);
	return 0;
}
