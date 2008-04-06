/*
 * vdrive-iec.h - Virtual disk-drive IEC implementation.
 *
 * Written by
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#ifndef _VDRIVEIEC_H
#define _VDRIVEIEC_H

#include "vdrive.h"
#include "types.h"

/* Generic IEC interface.  */
extern int vdrive_open(void *floppy, const char *name, int length,
                       int secondary);
extern int vdrive_close(void *floppy, int secondary);
extern int vdrive_read(void *floppy, BYTE *data, int secondary);
extern int vdrive_write(void *floppy, BYTE data, int secondary);
extern void vdrive_flush(void *floppy, int secondary);

/* Low level access to IEC interface.  */
extern void vdrive_open_create_dir_slot(bufferinfo_t *p, char *realname,
                                        int reallength, int filetype);
extern int vdrive_iec_attach(int unit, const char *name);

#endif

