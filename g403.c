/*
 * Copyright (c) 2018 Jan Klemkow <j.klemkow@wemelug.de>
 *
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jan Klemkow wrote this file.  As long as you retain this notice you can do
 * whatever you want with this stuff.  If you meet Sebastian Rottmann some day,
 * and you think this stuff is worth it, you can buy him a beer in return.
 * Because, I don't drink beer, but he does a lot and helped me with his Windows
 * machine.
 */

/*
 * I apologize at mpi@ for using uhid(4) for this and I promise to change this
 * program when uhid(4) finally dies.
 */

#include <sys/ioctl.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define estrtonum(str, num)						\
	do {								\
		const char *errstr;					\
		(num) = strtonum((str), 0, 255, &errstr);		\
		if (errstr != NULL)					\
			errx(1, "strtonum: %s: %s", errstr, (str));	\
	} while (0);

void
setcolor(int fd, char target, char red, char green, char blue)
{
	struct usb_ctl_report report;
	unsigned char data[9] = {
		0xff,
		0x0e,
		0x3b,
		target,	/* wheel or logo */
		0x01,
		red,
		green,
		blue,
		0x02
	};

	report.ucr_report = 0x2;
	memcpy(report.ucr_data, data, sizeof data);

	if (ioctl(fd, USB_SET_REPORT, &report) == -1)
		err(EXIT_FAILURE, "ioctl");
}

void
usage(void)
{
	fputs("g403 [-h] [-l|-w] uhid red green blue\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int ch, fd;
	bool lflag = true;
	bool wflag = true;
	unsigned char red, green, blue;

	while ((ch = getopt(argc, argv, "lwh")) != -1) {
		switch (ch) {
		case 'l':
			if (lflag == false)
				usage();
			wflag = false;
			break;
		case 'w':
			if (wflag == false)
				usage();
			lflag = false;
			break;
		case 'h':
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 4)
		usage();

	/* TODO: search dyn. for vid/pid 0x046d/0xC083 ... */
	if ((fd = open(argv[0], O_RDWR)) == -1)
		err(EXIT_FAILURE, "open: %s", argv[0]);

	estrtonum(argv[1], red);
	estrtonum(argv[2], green);
	estrtonum(argv[3], blue);

	if (lflag)
		setcolor(fd, 0x01, red, green, blue);

	if (lflag && wflag)
		usleep(1);	/* HACK: Set both does not work w/o usleep(2) */

	if (wflag)
		setcolor(fd, 0x00, red, green, blue);

	if (close(fd) == -1)
		err(EXIT_FAILURE, "close");

	return EXIT_SUCCESS;
}
