/*
 * vi:set tabstop=4:
 *
 * Copyright (C) 1999 by Udo Munk <um@compuserve.com>
 *
 * Permission granted to share and redistribute this program freely,
 * as long as no fee is demanded and as long as you keep my copyright
 * intact.
 *
 * This program is provided AS IS, there are no guarantees, none at all.
 *
 * For OS5 this program is of limited value, the console driver doesn't
 * return the scan codes in raw mode, because the scan code table can be
 * mapped to anything with mapkey or the PIO_KEYMAP ioctl(). So, you just
 * get the codes from the active map table (/usr/lib/keyboard/keys).
 */

/*#define LINUX*/		/* define this under Linux */
/*#define SCOOS5*/		/* define this under SCO OpenServer 5 */
/*#define SCOUW*/		/* define this under SCO Unixware */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#ifdef SCOOS5
#include <sys/vtkd.h>
#endif

#if defined(LINUX) || defined(SCOUW)
#include <sys/kd.h>
#endif

#ifdef LINUX
#define RAWMODE K_MEDIUMRAW /* Linux has 2 different raw modes, we use this */
#else
#define RAWMODE K_RAW
#endif

#ifdef SCOOS5
#define IMAXBEL 0	/* missing under OS5, buffer overruns silent? tee hee */
#endif

int main(int argc, char **argv)
{
	struct termios	ot, nt;
	int				flag = 1;
	unsigned char	c[4];
	int				i, n;

	/* try to switch keyboard into raw scan code mode */
	if (ioctl(fileno(stdin), KDSKBMODE, RAWMODE) < 0)
		perror("Could not switch keyboard into scan code mode");

	/* save tty line discipline */
	tcgetattr(fileno(stdin), &ot);

	/* now set line discipline to raw mode, 10s timeout */
	nt = ot;

	nt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	nt.c_iflag &= ~(IMAXBEL | IGNBRK | IGNCR | IGNPAR | BRKINT | INLCR |
		ICRNL | INPCK | ISTRIP | IXON | IUCLC | IXANY | IXOFF);
	nt.c_cflag &= ~(CSIZE | PARENB);
	nt.c_cflag |= CS8;
	nt.c_oflag &= ~(OCRNL | OLCUC | ONLCR | OPOST);
	nt.c_cc[VMIN] = 0;
	nt.c_cc[VTIME] = 100;
	tcsetattr(0, TCSAFLUSH, &nt);

	/* read from keyboard and display the scan code, until timeout */
	while (flag) {
		if ((n = read(fileno(stdin), &c, 4)) < 1) {
			flag = 0;
			continue;
		}

		for (i = 0; i < n; i++)
			printf("%02x (%d) ", (int)c[i], (int)c[i]);
		printf("\r\n");
	}

	/* restore line discipline */
	tcsetattr(fileno(stdin), TCSAFLUSH, &ot);

	/* switch keyboard back into xlate mode */
	ioctl(fileno(stdin), KDSKBMODE, K_XLATE);

	return 0;
}
