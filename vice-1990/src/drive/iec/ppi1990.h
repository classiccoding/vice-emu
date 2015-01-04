/*
 * ppi1990.h - ppi emulation for the 1990 disk drive.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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

#ifndef VICE_PPI1990_H
#define VICE_PPI1990_H

#include "types.h"

struct drive_context_s;
typedef struct ppi1990_s ppi1990_t;

extern void ppi1990_init(struct drive_context_s *drv);
extern void ppi1990_shutdown(ppi1990_t *drv);

extern void ppi1990_setup_context(struct drive_context_s *drv);
extern void ppi1990_store(struct drive_context_s *drv, WORD addr, BYTE byte);
extern BYTE ppi1990_read(struct drive_context_s *drv, WORD addr);
extern void ppi1990_reset(ppi1990_t *drv);

extern int ppi1990_is_rom(struct drive_context_s *drv);
extern int ppi1990_is_shift(struct drive_context_s *drv);

#endif
