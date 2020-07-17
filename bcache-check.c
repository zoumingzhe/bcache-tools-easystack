#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

static bool is_path_exist(char *path) {
	if (!path)
		return false;

	if (access(path, F_OK)!= -1)
		return true;

	return false;
}

static int get_parent_device(char *dev, char *base)
{
	int i, end = 0, digit_end = 0, ret = 0;
	int length;
	char *p = NULL;
	char buf[100] = {0};

	if (strlen(dev) < 2)
		return 0;

	length = strlen(dev);
	for (i = length - 1; i >= 0; i--) {
		if (!isdigit(dev[i])) {
			// 1. block device's partition end with "pX"
			if (dev[i] == 'p' && i != length - 1)
				end = i;
			break;
		} else {
			digit_end = i;
		}
	}

	// 2. for nvme0n1[p1], drbd1[p1], rbd1[p1]  etc.., also need ensure
	// if it is a partition
	if (digit_end) {
		p = calloc(digit_end + 1, 1);
		if (!p) {
			printf("Can not alloc memory\n");
			return 0;
		}

		memcpy(p, dev, digit_end);
		sprintf(buf, "/sys/block/%s/%s/", p, dev);
		if (is_path_exist(buf)) {
			memcpy(base, p, digit_end);
			ret = digit_end;
			goto out;
		}

		if (!end) {
			ret = 0;
			goto out;
		}

		// 3. incase the device is sdp[1]...
		memset(p, 0, digit_end + 1);
		memcpy(p, dev, end);
		sprintf(buf, "/sys/block/%s/%s/", p, dev);
		if (is_path_exist(buf)) {
			memcpy(base, p, end);
			ret = end;
			goto out;
		}

		ret = 0;
	}

out:
	if (!p)
		free(p);
	return ret;
}

int main(int argc, char **argv)
{
	int disk_fd;
	char buf[11] = {0};
	ssize_t count;
	char bdev_name[100] = {0};
	char escache_dir[100] = {0};
	char device[100] = {0};
	int ret;

	if(argc != 2) {
		printf("Usage: disk-check NODE\n");
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

	if(!strcmp(buf, "alcubierre") ||
	   !strcmp(buf, "##skipudev")) {
		printf("SKIPREGISTER_DEV=yes\n");
	} else {
		printf("SKIPREGISTER_DEV=no\n");
	}

	ret = sscanf(argv[1], "/dev/%s", bdev_name);
	if (ret != 1) {
		printf("Can not parse '/dev/bdev_name' from %s\n", argv[1]);
		close(disk_fd);
		return -1;
	}

	if (get_parent_device(bdev_name, device)) {
		sprintf(escache_dir, "/sys/block/%s/%s/escache",
			device, bdev_name);
	} else {
		sprintf(escache_dir, "/sys/block/%s/escache", bdev_name);
	}

	if (is_path_exist(escache_dir))
		printf("DISK_REGISTERED=yes\n");
	else
		printf("DISK_REGISTERED=no\n");

	close(disk_fd);
	return 0;
}
