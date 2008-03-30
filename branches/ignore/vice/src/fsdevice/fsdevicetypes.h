/*
 * fsdevicetypes.h - File system device.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef _FSDEVICETYPES_H
#define _FSDEVICETYPES_H

#include "tape.h"
#include "types.h"

enum fsmode {
    Write, Read, Append, Directory
};

struct fileio_info_s;
struct ioutil_dir_s;

struct fs_buffer_info_s {
    struct fileio_info_s *info;
    struct ioutil_dir_s *ioutil_dir;
    tape_image_t tape;
    enum fsmode mode;
    char dir[MAXPATHLEN];
    BYTE name[MAXPATHLEN + 5];
    int buflen;
    BYTE *bufp;
    int eof;
    int reclen;
    int type;
};
typedef struct fs_buffer_info_s fs_buffer_info_t;

extern fs_buffer_info_t fs_info[];

struct vdrive_s;

extern void fsdevice_error(struct vdrive_s *vdrive, int code);
extern char *fsdevice_get_path(unsigned int unit);
extern int fsdevice_error_get_byte(struct vdrive_s *vdrive, BYTE *data);
extern int fsdevice_flush_write_byte(struct vdrive_s *vdrive, BYTE data);

extern char fs_dirmask[];

#endif

