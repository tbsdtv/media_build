/* femon-s2 -- monitor frontend status
 *
 * Copyright (C) 2003 convergence GmbH
 * Johannes Stezenbach <js@convergence.de>
 *
 * Copyright (C) 2009 Igor M. Liplianin <liplianin@me.by>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * femon 2011/06/16
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <sys/time.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/version.h>

#if DVB_API_VERSION < 5
#error femon-s2 requires Linux DVB driver API version >= 5.0!
#endif

#define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"

static char *usage_str =
    "\nusage: femon-s2 [options]\n"
    "     -H        : human readable output\n"
    "     -a number : use given adapter (default 0)\n"
    "     -f number : use given frontend (default 0)\n"
    "     -c number : samples to take (default 0 = infinite)\n\n";


static void usage(void)
{
	fprintf(stderr, usage_str);
	exit(1);
}

static
int check_frontend (int fe_fd, int human_readable, unsigned int count)
{
	fe_status_t status;
	uint16_t snr = 10, signal = 10;
	uint32_t ber = 1, uncorrected_blocks =1;
	int16_t temp = 0;
	int timeout = 0;
	unsigned int samples = 0;

	do {
		if (ioctl(fe_fd, FE_READ_STATUS, &status) == -1)
			perror("FE_READ_STATUS failed");
		/* some frontends might not support all these ioctls, thus we
		 * avoid printing errors
		 */
		if (ioctl(fe_fd, FE_READ_SIGNAL_STRENGTH, &signal) == -1)
			signal = -2;
		if (ioctl(fe_fd, FE_READ_SNR, &snr) == -1)
			snr = -2;
		if (ioctl(fe_fd, FE_READ_BER, &ber) == -1)
			ber = -2;
		if (ioctl(fe_fd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks) == -1)
			uncorrected_blocks = -2;
		if (ioctl(fe_fd, FE_READ_TEMP, &temp) == -1)
			ber = -2;

		if (human_readable) {
			printf ("status %02x | signal %3u%% | snr %3u%% | ber %d | unc %d | temp %d | ",
				status, (signal * 100) / 0xffff, (snr * 100) / 0xffff, ber, uncorrected_blocks, temp);
		} else {
			printf ("status %02x | signal %04x | snr %04x | ber %08x | unc %08x | temp %d | ",
				status, signal, snr, ber, uncorrected_blocks, temp);
		}
		if (status & FE_HAS_LOCK)
			printf("FE_HAS_LOCK");
		printf("\n");

		usleep(1000000);
		samples++;
	} while ((!count) || (count-samples));

	return 0;
}

static
int do_mon(unsigned int adapter, unsigned int frontend, int human_readable, unsigned int count)
{
	int result;
	static int fefd;
	char fedev[128];

	if (!fefd) {
		snprintf(fedev, sizeof(fedev), FRONTENDDEVICE, adapter, frontend);

		if ((fefd = open(fedev, O_RDONLY | O_NONBLOCK)) < 0) {
			perror("opening frontend failed");
			return 0;
		}
	}

	/* TODO here we may put frontend description */

	result = check_frontend (fefd, human_readable, count);

	close(fefd);

	return result;
}

int main(int argc, char *argv[])
{
	unsigned int adapter = 0, frontend = 0, count = 0;
	int human_readable = 0;
	int opt;

	while ((opt = getopt(argc, argv, "Ha:f:c:")) != -1) {
		switch (opt)
		{
		default:
			usage();
			break;
		case 'a':
			adapter = strtoul(optarg, NULL, 0);
			break;
		case 'c':
			count = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			frontend = strtoul(optarg, NULL, 0);
			break;
		case 'H':
			human_readable = 1;
			break;
		}
	}

	do_mon(adapter, frontend, human_readable, count);

	return 0;
}
