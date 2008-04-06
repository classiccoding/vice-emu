/*
 * joystick.c - Joystick support for Linux.
 *
 * Written by
 *  Bernhard Kuhn    (kuhn@eikon.e-technik.tu-muenchen.de)
 *  Ulmer Lionel     (ulmer@poly.polytechnique.fr)
 *
 * Patches by
 *  Daniel Sladic    (sladic@eecg.toronto.edu)
 *
 * NetBSD support by
 *  Krister Walfridsson (cato@df.lth.se)
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vice.h"
#include "types.h"
#include "kbd.h"
#include "resources.h"
#include "cmdline.h"
#include "joystick.h"

int joystick_port_map[2];

/* Resources */

static int joyport1select(resource_value_t v)
{

    kbd_flag_joykeys(joystick_port_map[0], 0);	/* disable old mapping */
    joystick_port_map[0] = (int) v;
    kbd_flag_joykeys(joystick_port_map[0], 1);	/* enable new mapping */

    return 0;
};

static int joyport2select(resource_value_t v)
{

    kbd_flag_joykeys(joystick_port_map[1], 0);	/* disable old mapping */
    joystick_port_map[1] = (int) v;
    kbd_flag_joykeys(joystick_port_map[1], 1);	/* enable new mapping */

    return 0;
};


static resource_t resources[] =
{
    {"JoyDevice1", RES_INTEGER, (resource_value_t) 0,
     (resource_value_t *) & joystick_port_map[0], joyport1select},
    {"JoyDevice2", RES_INTEGER, (resource_value_t) 0,
     (resource_value_t *) & joystick_port_map[1], joyport2select},
    {NULL},
};

/* Command-line options.  */

static cmdline_option_t cmdline_options[] =
{
    { "-joydev1", SET_RESOURCE, 1, NULL, NULL, "JoyDevice1", NULL,
      "<0-5>", "Set device for joystick port 1" },
    { "-joydev2", SET_RESOURCE, 1, NULL, NULL, "JoyDevice2", NULL,
      "<0-5>", "Set device for joystick port 2" },
    { NULL },
};

int joystick_init_resources(void)
{
    return resources_register(resources);
}

int joystick_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}


#ifdef HAS_JOYSTICK

#ifdef LINUX_JOYSTICK
#include <linux/joystick.h>
#elif defined(BSD_JOYSTICK)
#include <machine/joystick.h>
#define JS_DATA_TYPE joystick
#define JS_RETURN    sizeof(struct joystick)
#elif
#error Unknown Joystick
#endif

int ajoyfd[2] = { -1, -1 };
int djoyfd[2] = { -1, -1 };

#define JOYCALLOOPS 100
#define JOYSENSITIVITY 5
int joyxcal[2];
int joyycal[2];
int joyxmin[2];
int joyxmax[2];
int joyymin[2];
int joyymax[2];

void joystick_init(void)
{
    int i;

    /* close all device files */
    for (i = 0; i < 2; i++) {
	if (ajoyfd[i] != -1)
	    close(ajoyfd[i]);
	if (djoyfd[i] != -1)
	    close(djoyfd[i]);
    }

    /* open analog device files */
    for (i = 0; i < 2; i++) {

	char *dev;
#ifdef LINUX_JOYSTICK
	dev = (i == 0) ? "/dev/js0" : "/dev/js1";
#elif defined(BSD_JOYSTICK)
	dev = (i == 0) ? "/dev/joy0" : "/dev/joy1";
#endif

	ajoyfd[i] = open(dev, O_RDONLY);
	if (ajoyfd[i] < 0) {
	    fprintf(stderr, "Warning: couldn't open the joystick device %s!\n", dev);
	} else {
	    int j;

	    /* calibration loop */
	    for (j = 0; j < JOYCALLOOPS; j++) {
		struct JS_DATA_TYPE js;
		int status = read(ajoyfd[i], &js, JS_RETURN);

		if (status != JS_RETURN) {
		    fprintf(stderr, "Warning: error reading the joystick device%s!\n",
			    dev);
		} else {
		    /* determine average */
		    joyxcal[i] += js.x;
		    joyycal[i] += js.y;
		}
	    }

	    /* correct average */
	    joyxcal[i] /= JOYCALLOOPS;
	    joyycal[i] /= JOYCALLOOPS;

	    /* determine treshoulds */
	    joyxmin[i] = joyxcal[i] - joyxcal[i] / JOYSENSITIVITY;
	    joyxmax[i] = joyxcal[i] + joyxcal[i] / JOYSENSITIVITY;
	    joyymin[i] = joyycal[i] - joyycal[i] / JOYSENSITIVITY;
	    joyymax[i] = joyycal[i] + joyycal[i] / JOYSENSITIVITY;

	    printf("hardware joystick calibration %s:\n", dev);
	    printf("X: min: %i , mid: %i , max: %i\n", joyxmin[i], joyxcal[i],
		   joyxmax[i]);
	    printf("Y: min: %i , mid: %i , max: %i\n", joyymin[i], joyycal[i],
		   joyymax[i]);
	}
    }

#ifdef HAS_DIGITAL_JOYSTICK
    /* open device files for digital joystick */
    for (i = 0; i < 2; i++) {
	char *dev;
	dev = (i == 0) ? "/dev/djs0" : "/dev/djs1";

	djoyfd[i] = open(dev, O_RDONLY);
	if (djoyfd[i] < 0) {
	    fprintf(stderr, "Warning: couldn't open the joystick device %s!\n", dev);
	}
    }
#endif
}

void joystick_close(void)
{
    if (ajoyfd[0] > 0)
	close(ajoyfd[0]);
    if (ajoyfd[1] > 0)
	close(ajoyfd[1]);
    if (djoyfd[0] > 0)
	close(djoyfd[0]);
    if (djoyfd[1] > 0)
	close(djoyfd[1]);
}

void joystick(void)
{
    int i;
    for (i = 1; i <= 2; i++) {
	int joyport = joystick_port_map[i - 1];

#ifdef HAS_DIGITAL_JOYSTICK
	if (joyport == JOYDEV_DIGITAL_0 || joyport == JOYDEV_DIGITAL_1) {
	    int status;
	    struct DJS_DATA_TYPE djs;
	    int djoyport = joyport - JOYDEV_DIGITAL_0;

	    if (djoyfd[djoyport] > 0) {
		status = read(djoyfd[djoyport], &djs, DJS_RETURN);
		if (status != DJS_RETURN) {
		    fprintf(stderr,
			    "Warning: error reading the digital joystick device!\n");
		} else {
		    joy[i] = (joy[i] & 0xe0) | ((~(djs.switches >> 3)) & 0x1f);
		}
	    }
	} else
#endif
	if (joyport == JOYDEV_ANALOG_0 || joyport == JOYDEV_ANALOG_1) {
	    int status;
	    struct JS_DATA_TYPE js;
	    int ajoyport = joyport - JOYDEV_ANALOG_0;

	    if (ajoyfd[ajoyport] > 0) {
		status = read(ajoyfd[ajoyport], &js, JS_RETURN);
		if (status != JS_RETURN) {
		    fprintf(stderr, "Warning: error reading the joystick device!\n");
		} else {
		    if (js.y < joyymin[ajoyport])
			joy[i] |= 1;
		    else
			joy[i] &= ~1;
		    if (js.y > joyymax[ajoyport])
			joy[i] |= 2;
		    else
			joy[i] &= ~2;
		    if (js.x < joyxmin[ajoyport])
			joy[i] |= 4;
		    else
			joy[i] &= ~4;
		    if (js.x > joyxmax[ajoyport])
			joy[i] |= 8;
		    else
			joy[i] &= ~8;
#ifdef LINUX_JOYSTICK
		    if (js.buttons)
			joy[i] |= 16;
#elif defined(BSD_JOYSTICK)
		    if (js.b1 || js.b2)
			joy[i] |= 16;
#endif
		    else
			joy[i] &= ~16;

		}
	    }
	}
    }
}

#else

void joystick_init(void)
{
}
void joystick_close(void)
{
}

#endif				/* HAS_JOYSTICK */
