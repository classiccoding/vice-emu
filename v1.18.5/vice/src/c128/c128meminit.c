/*
 * c128meminit.c -- Initialize C128 memory.
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

#include "vice.h"

#include <stdio.h>

#include "c128mem.h"
#include "c128meminit.h"
#include "c128memrom.h"
#include "c128mmu.h"
#include "c64cia.h"
#include "c64io.h"
#include "functionrom.h"
#include "sid.h"
#include "vdc-mem.h"
#include "vicii-mem.h"


void c128meminit(void)
{
    unsigned int i, j;

    for (j = 0; j < 256; j++) {

        for (i = 0; i <= 0x100; i++) {
            mem_read_base_set(j, i, NULL);
        }

        mem_read_tab_set(j, 0, zero_read);
        mem_set_write_hook(j, 0, zero_store);
        mem_read_tab_set(j, 1, one_read);
        mem_set_write_hook(j, 1, one_store);
    }

    for (j = 0; j < 128; j += 64) {
        for (i = 0x02; i <= 0x3f; i++) {
            mem_read_tab_set(0+j, i, ram_read);
            mem_read_tab_set(1+j, i, ram_read);
            mem_read_tab_set(2+j, i, ram_read);
            mem_read_tab_set(3+j, i, ram_read);
            mem_read_tab_set(4+j, i, ram_read);
            mem_read_tab_set(5+j, i, ram_read);
            mem_read_tab_set(6+j, i, ram_read);
            mem_read_tab_set(7+j, i, ram_read);
            mem_read_tab_set(8+j, i, ram_read);
            mem_read_tab_set(9+j, i, ram_read);
            mem_read_tab_set(10+j, i, ram_read);
            mem_read_tab_set(11+j, i, ram_read);
            mem_read_tab_set(12+j, i, ram_read);
            mem_read_tab_set(13+j, i, ram_read);
            mem_read_tab_set(14+j, i, ram_read);
            mem_read_tab_set(15+j, i, ram_read);
            mem_read_tab_set(16+j, i, ram_read);
            mem_read_tab_set(17+j, i, ram_read);
            mem_read_tab_set(18+j, i, ram_read);
            mem_read_tab_set(19+j, i, ram_read);
            mem_read_tab_set(20+j, i, ram_read);
            mem_read_tab_set(21+j, i, ram_read);
            mem_read_tab_set(22+j, i, ram_read);
            mem_read_tab_set(23+j, i, ram_read);
            mem_read_tab_set(24+j, i, ram_read);
            mem_read_tab_set(25+j, i, ram_read);
            mem_read_tab_set(26+j, i, ram_read);
            mem_read_tab_set(27+j, i, ram_read);
            mem_read_tab_set(28+j, i, ram_read);
            mem_read_tab_set(29+j, i, ram_read);
            mem_read_tab_set(30+j, i, ram_read);
            mem_read_tab_set(31+j, i, ram_read);
            mem_read_tab_set(32+j, i, lo_read);
            mem_read_tab_set(33+j, i, lo_read);
            mem_read_tab_set(34+j, i, lo_read);
            mem_read_tab_set(35+j, i, lo_read);
            mem_read_tab_set(36+j, i, lo_read);
            mem_read_tab_set(37+j, i, lo_read);
            mem_read_tab_set(38+j, i, lo_read);
            mem_read_tab_set(39+j, i, lo_read);
            mem_read_tab_set(40+j, i, lo_read);
            mem_read_tab_set(41+j, i, lo_read);
            mem_read_tab_set(42+j, i, lo_read);
            mem_read_tab_set(43+j, i, lo_read);
            mem_read_tab_set(44+j, i, lo_read);
            mem_read_tab_set(45+j, i, lo_read);
            mem_read_tab_set(46+j, i, lo_read);
            mem_read_tab_set(47+j, i, lo_read);
            mem_read_tab_set(48+j, i, lo_read);
            mem_read_tab_set(49+j, i, lo_read);
            mem_read_tab_set(50+j, i, lo_read);
            mem_read_tab_set(51+j, i, lo_read);
            mem_read_tab_set(52+j, i, lo_read);
            mem_read_tab_set(53+j, i, lo_read);
            mem_read_tab_set(54+j, i, lo_read);
            mem_read_tab_set(55+j, i, lo_read);
            mem_read_tab_set(56+j, i, lo_read);
            mem_read_tab_set(57+j, i, lo_read);
            mem_read_tab_set(58+j, i, lo_read);
            mem_read_tab_set(59+j, i, lo_read);
            mem_read_tab_set(60+j, i, lo_read);
            mem_read_tab_set(61+j, i, lo_read);
            mem_read_tab_set(62+j, i, lo_read);
            mem_read_tab_set(63+j, i, lo_read);
            mem_set_write_hook(0+j, i, ram_store);
            mem_set_write_hook(1+j, i, ram_store);
            mem_set_write_hook(2+j, i, ram_store);
            mem_set_write_hook(3+j, i, ram_store);
            mem_set_write_hook(4+j, i, ram_store);
            mem_set_write_hook(5+j, i, ram_store);
            mem_set_write_hook(6+j, i, ram_store);
            mem_set_write_hook(7+j, i, ram_store);
            mem_set_write_hook(8+j, i, ram_store);
            mem_set_write_hook(9+j, i, ram_store);
            mem_set_write_hook(10+j, i, ram_store);
            mem_set_write_hook(11+j, i, ram_store);
            mem_set_write_hook(12+j, i, ram_store);
            mem_set_write_hook(13+j, i, ram_store);
            mem_set_write_hook(14+j, i, ram_store);
            mem_set_write_hook(15+j, i, ram_store);
            mem_set_write_hook(16+j, i, ram_store);
            mem_set_write_hook(17+j, i, ram_store);
            mem_set_write_hook(18+j, i, ram_store);
            mem_set_write_hook(19+j, i, ram_store);
            mem_set_write_hook(20+j, i, ram_store);
            mem_set_write_hook(21+j, i, ram_store);
            mem_set_write_hook(22+j, i, ram_store);
            mem_set_write_hook(23+j, i, ram_store);
            mem_set_write_hook(24+j, i, ram_store);
            mem_set_write_hook(25+j, i, ram_store);
            mem_set_write_hook(26+j, i, ram_store);
            mem_set_write_hook(27+j, i, ram_store);
            mem_set_write_hook(28+j, i, ram_store);
            mem_set_write_hook(29+j, i, ram_store);
            mem_set_write_hook(30+j, i, ram_store);
            mem_set_write_hook(31+j, i, ram_store);
            mem_set_write_hook(32+j, i, lo_store);
            mem_set_write_hook(33+j, i, lo_store);
            mem_set_write_hook(34+j, i, lo_store);
            mem_set_write_hook(35+j, i, lo_store);
            mem_set_write_hook(36+j, i, lo_store);
            mem_set_write_hook(37+j, i, lo_store);
            mem_set_write_hook(38+j, i, lo_store);
            mem_set_write_hook(39+j, i, lo_store);
            mem_set_write_hook(40+j, i, lo_store);
            mem_set_write_hook(41+j, i, lo_store);
            mem_set_write_hook(42+j, i, lo_store);
            mem_set_write_hook(43+j, i, lo_store);
            mem_set_write_hook(44+j, i, lo_store);
            mem_set_write_hook(45+j, i, lo_store);
            mem_set_write_hook(46+j, i, lo_store);
            mem_set_write_hook(47+j, i, lo_store);
            mem_set_write_hook(48+j, i, lo_store);
            mem_set_write_hook(49+j, i, lo_store);
            mem_set_write_hook(50+j, i, lo_store);
            mem_set_write_hook(51+j, i, lo_store);
            mem_set_write_hook(52+j, i, lo_store);
            mem_set_write_hook(53+j, i, lo_store);
            mem_set_write_hook(54+j, i, lo_store);
            mem_set_write_hook(55+j, i, lo_store);
            mem_set_write_hook(56+j, i, lo_store);
            mem_set_write_hook(57+j, i, lo_store);
            mem_set_write_hook(58+j, i, lo_store);
            mem_set_write_hook(59+j, i, lo_store);
            mem_set_write_hook(60+j, i, lo_store);
            mem_set_write_hook(61+j, i, lo_store);
            mem_set_write_hook(62+j, i, lo_store);
            mem_set_write_hook(63+j, i, lo_store);
            mem_read_base_set(0+j, i, mem_ram + (i << 8));
            mem_read_base_set(1+j, i, mem_ram + (i << 8));
            mem_read_base_set(2+j, i, mem_ram + (i << 8));
            mem_read_base_set(3+j, i, mem_ram + (i << 8));
            mem_read_base_set(4+j, i, mem_ram + (i << 8));
            mem_read_base_set(5+j, i, mem_ram + (i << 8));
            mem_read_base_set(6+j, i, mem_ram + (i << 8));
            mem_read_base_set(7+j, i, mem_ram + (i << 8));
            mem_read_base_set(8+j, i, mem_ram + (i << 8));
            mem_read_base_set(9+j, i, mem_ram + (i << 8));
            mem_read_base_set(10+j, i, mem_ram + (i << 8));
            mem_read_base_set(11+j, i, mem_ram + (i << 8));
            mem_read_base_set(12+j, i, mem_ram + (i << 8));
            mem_read_base_set(13+j, i, mem_ram + (i << 8));
            mem_read_base_set(14+j, i, mem_ram + (i << 8));
            mem_read_base_set(15+j, i, mem_ram + (i << 8));
            mem_read_base_set(16+j, i, mem_ram + (i << 8));
            mem_read_base_set(17+j, i, mem_ram + (i << 8));
            mem_read_base_set(18+j, i, mem_ram + (i << 8));
            mem_read_base_set(19+j, i, mem_ram + (i << 8));
            mem_read_base_set(20+j, i, mem_ram + (i << 8));
            mem_read_base_set(21+j, i, mem_ram + (i << 8));
            mem_read_base_set(22+j, i, mem_ram + (i << 8));
            mem_read_base_set(23+j, i, mem_ram + (i << 8));
            mem_read_base_set(24+j, i, mem_ram + (i << 8));
            mem_read_base_set(25+j, i, mem_ram + (i << 8));
            mem_read_base_set(26+j, i, mem_ram + (i << 8));
            mem_read_base_set(27+j, i, mem_ram + (i << 8));
            mem_read_base_set(28+j, i, mem_ram + (i << 8));
            mem_read_base_set(29+j, i, mem_ram + (i << 8));
            mem_read_base_set(30+j, i, mem_ram + (i << 8));
            mem_read_base_set(31+j, i, mem_ram + (i << 8));
            mem_read_base_set(32+j, i, NULL);
            mem_read_base_set(33+j, i, NULL);
            mem_read_base_set(34+j, i, NULL);
            mem_read_base_set(35+j, i, NULL);
            mem_read_base_set(36+j, i, NULL);
            mem_read_base_set(37+j, i, NULL);
            mem_read_base_set(38+j, i, NULL);
            mem_read_base_set(39+j, i, NULL);
            mem_read_base_set(40+j, i, NULL);
            mem_read_base_set(41+j, i, NULL);
            mem_read_base_set(42+j, i, NULL);
            mem_read_base_set(43+j, i, NULL);
            mem_read_base_set(44+j, i, NULL);
            mem_read_base_set(45+j, i, NULL);
            mem_read_base_set(46+j, i, NULL);
            mem_read_base_set(47+j, i, NULL);
            mem_read_base_set(48+j, i, NULL);
            mem_read_base_set(49+j, i, NULL);
            mem_read_base_set(50+j, i, NULL);
            mem_read_base_set(51+j, i, NULL);
            mem_read_base_set(52+j, i, NULL);
            mem_read_base_set(53+j, i, NULL);
            mem_read_base_set(54+j, i, NULL);
            mem_read_base_set(55+j, i, NULL);
            mem_read_base_set(56+j, i, NULL);
            mem_read_base_set(57+j, i, NULL);
            mem_read_base_set(58+j, i, NULL);
            mem_read_base_set(59+j, i, NULL);
            mem_read_base_set(60+j, i, NULL);
            mem_read_base_set(61+j, i, NULL);
            mem_read_base_set(62+j, i, NULL);
            mem_read_base_set(63+j, i, NULL);
        }

        for (i = 0x40; i <= 0x7f; i++) {
            mem_read_tab_set(0+j, i, ram_read);
            mem_read_tab_set(1+j, i, basic_lo_read);
            mem_read_tab_set(2+j, i, ram_read);
            mem_read_tab_set(3+j, i, basic_lo_read);
            mem_read_tab_set(4+j, i, ram_read);
            mem_read_tab_set(5+j, i, basic_lo_read);
            mem_read_tab_set(6+j, i, ram_read);
            mem_read_tab_set(7+j, i, basic_lo_read);
            mem_read_tab_set(8+j, i, ram_read);
            mem_read_tab_set(9+j, i, basic_lo_read);
            mem_read_tab_set(10+j, i, ram_read);
            mem_read_tab_set(11+j, i, basic_lo_read);
            mem_read_tab_set(12+j, i, ram_read);
            mem_read_tab_set(13+j, i, basic_lo_read);
            mem_read_tab_set(14+j, i, ram_read);
            mem_read_tab_set(15+j, i, basic_lo_read);
            mem_read_tab_set(16+j, i, ram_read);
            mem_read_tab_set(17+j, i, basic_lo_read);
            mem_read_tab_set(18+j, i, ram_read);
            mem_read_tab_set(19+j, i, basic_lo_read);
            mem_read_tab_set(20+j, i, ram_read);
            mem_read_tab_set(21+j, i, basic_lo_read);
            mem_read_tab_set(22+j, i, ram_read);
            mem_read_tab_set(23+j, i, basic_lo_read);
            mem_read_tab_set(24+j, i, ram_read);
            mem_read_tab_set(25+j, i, basic_lo_read);
            mem_read_tab_set(26+j, i, ram_read);
            mem_read_tab_set(27+j, i, basic_lo_read);
            mem_read_tab_set(28+j, i, ram_read);
            mem_read_tab_set(29+j, i, basic_lo_read);
            mem_read_tab_set(30+j, i, ram_read);
            mem_read_tab_set(31+j, i, basic_lo_read);
            mem_read_tab_set(32+j, i, ram_read);
            mem_read_tab_set(33+j, i, basic_lo_read);
            mem_read_tab_set(34+j, i, ram_read);
            mem_read_tab_set(35+j, i, basic_lo_read);
            mem_read_tab_set(36+j, i, ram_read);
            mem_read_tab_set(37+j, i, basic_lo_read);
            mem_read_tab_set(38+j, i, ram_read);
            mem_read_tab_set(39+j, i, basic_lo_read);
            mem_read_tab_set(40+j, i, ram_read);
            mem_read_tab_set(41+j, i, basic_lo_read);
            mem_read_tab_set(42+j, i, ram_read);
            mem_read_tab_set(43+j, i, basic_lo_read);
            mem_read_tab_set(44+j, i, ram_read);
            mem_read_tab_set(45+j, i, basic_lo_read);
            mem_read_tab_set(46+j, i, ram_read);
            mem_read_tab_set(47+j, i, basic_lo_read);
            mem_read_tab_set(48+j, i, ram_read);
            mem_read_tab_set(49+j, i, basic_lo_read);
            mem_read_tab_set(50+j, i, ram_read);
            mem_read_tab_set(51+j, i, basic_lo_read);
            mem_read_tab_set(52+j, i, ram_read);
            mem_read_tab_set(53+j, i, basic_lo_read);
            mem_read_tab_set(54+j, i, ram_read);
            mem_read_tab_set(55+j, i, basic_lo_read);
            mem_read_tab_set(56+j, i, ram_read);
            mem_read_tab_set(57+j, i, basic_lo_read);
            mem_read_tab_set(58+j, i, ram_read);
            mem_read_tab_set(59+j, i, basic_lo_read);
            mem_read_tab_set(60+j, i, ram_read);
            mem_read_tab_set(61+j, i, basic_lo_read);
            mem_read_tab_set(62+j, i, ram_read);
            mem_read_tab_set(63+j, i, basic_lo_read);
            mem_set_write_hook(0+j, i, ram_store);
            mem_set_write_hook(1+j, i, basic_lo_store);
            mem_set_write_hook(2+j, i, ram_store);
            mem_set_write_hook(3+j, i, basic_lo_store);
            mem_set_write_hook(4+j, i, ram_store);
            mem_set_write_hook(5+j, i, basic_lo_store);
            mem_set_write_hook(6+j, i, ram_store);
            mem_set_write_hook(7+j, i, basic_lo_store);
            mem_set_write_hook(8+j, i, ram_store);
            mem_set_write_hook(9+j, i, basic_lo_store);
            mem_set_write_hook(10+j, i, ram_store);
            mem_set_write_hook(11+j, i, basic_lo_store);
            mem_set_write_hook(12+j, i, ram_store);
            mem_set_write_hook(13+j, i, basic_lo_store);
            mem_set_write_hook(14+j, i, ram_store);
            mem_set_write_hook(15+j, i, basic_lo_store);
            mem_set_write_hook(16+j, i, ram_store);
            mem_set_write_hook(17+j, i, basic_lo_store);
            mem_set_write_hook(18+j, i, ram_store);
            mem_set_write_hook(19+j, i, basic_lo_store);
            mem_set_write_hook(20+j, i, ram_store);
            mem_set_write_hook(21+j, i, basic_lo_store);
            mem_set_write_hook(22+j, i, ram_store);
            mem_set_write_hook(23+j, i, basic_lo_store);
            mem_set_write_hook(24+j, i, ram_store);
            mem_set_write_hook(25+j, i, basic_lo_store);
            mem_set_write_hook(26+j, i, ram_store);
            mem_set_write_hook(27+j, i, basic_lo_store);
            mem_set_write_hook(28+j, i, ram_store);
            mem_set_write_hook(29+j, i, basic_lo_store);
            mem_set_write_hook(30+j, i, ram_store);
            mem_set_write_hook(31+j, i, basic_lo_store);
            mem_set_write_hook(32+j, i, ram_store);
            mem_set_write_hook(33+j, i, basic_lo_store);
            mem_set_write_hook(34+j, i, ram_store);
            mem_set_write_hook(35+j, i, basic_lo_store);
            mem_set_write_hook(36+j, i, ram_store);
            mem_set_write_hook(37+j, i, basic_lo_store);
            mem_set_write_hook(38+j, i, ram_store);
            mem_set_write_hook(39+j, i, basic_lo_store);
            mem_set_write_hook(40+j, i, ram_store);
            mem_set_write_hook(41+j, i, basic_lo_store);
            mem_set_write_hook(42+j, i, ram_store);
            mem_set_write_hook(43+j, i, basic_lo_store);
            mem_set_write_hook(44+j, i, ram_store);
            mem_set_write_hook(45+j, i, basic_lo_store);
            mem_set_write_hook(46+j, i, ram_store);
            mem_set_write_hook(47+j, i, basic_lo_store);
            mem_set_write_hook(48+j, i, ram_store);
            mem_set_write_hook(49+j, i, basic_lo_store);
            mem_set_write_hook(50+j, i, ram_store);
            mem_set_write_hook(51+j, i, basic_lo_store);
            mem_set_write_hook(52+j, i, ram_store);
            mem_set_write_hook(53+j, i, basic_lo_store);
            mem_set_write_hook(54+j, i, ram_store);
            mem_set_write_hook(55+j, i, basic_lo_store);
            mem_set_write_hook(56+j, i, ram_store);
            mem_set_write_hook(57+j, i, basic_lo_store);
            mem_set_write_hook(58+j, i, ram_store);
            mem_set_write_hook(59+j, i, basic_lo_store);
            mem_set_write_hook(60+j, i, ram_store);
            mem_set_write_hook(61+j, i, basic_lo_store);
            mem_set_write_hook(62+j, i, ram_store);
            mem_set_write_hook(63+j, i, basic_lo_store);
            mem_read_base_set(0+j, i, mem_ram + (i << 8));
            mem_read_base_set(1+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(2+j, i, mem_ram + (i << 8));
            mem_read_base_set(3+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(4+j, i, mem_ram + (i << 8));
            mem_read_base_set(5+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(6+j, i, mem_ram + (i << 8));
            mem_read_base_set(7+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(8+j, i, mem_ram + (i << 8));
            mem_read_base_set(9+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(10+j, i, mem_ram + (i << 8));
            mem_read_base_set(11+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(12+j, i, mem_ram + (i << 8));
            mem_read_base_set(13+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(14+j, i, mem_ram + (i << 8));
            mem_read_base_set(15+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(16+j, i, mem_ram + (i << 8));
            mem_read_base_set(17+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(18+j, i, mem_ram + (i << 8));
            mem_read_base_set(19+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(20+j, i, mem_ram + (i << 8));
            mem_read_base_set(21+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(22+j, i, mem_ram + (i << 8));
            mem_read_base_set(23+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(24+j, i, mem_ram + (i << 8));
            mem_read_base_set(25+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(26+j, i, mem_ram + (i << 8));
            mem_read_base_set(27+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(28+j, i, mem_ram + (i << 8));
            mem_read_base_set(29+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(30+j, i, mem_ram + (i << 8));
            mem_read_base_set(31+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(32+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(33+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(34+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(35+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(36+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(37+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(38+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(39+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(40+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(41+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(42+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(43+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(44+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(45+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(46+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(47+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(48+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(49+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(50+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(51+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(52+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(53+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(54+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(55+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(56+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(57+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(58+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(59+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(60+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(61+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
            mem_read_base_set(62+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(63+j, i,
                              c128memrom_basic_rom + ((i & 0x3f) << 8));
        }

        for (i = 0x80; i <= 0xbf; i++) {
            mem_read_tab_set(0+j, i, basic_hi_read);
            mem_read_tab_set(1+j, i, basic_hi_read);
            mem_read_tab_set(2+j, i, internal_function_rom_read);
            mem_read_tab_set(3+j, i, internal_function_rom_read);
            mem_read_tab_set(4+j, i, external_function_rom_read);
            mem_read_tab_set(5+j, i, external_function_rom_read);
            mem_read_tab_set(6+j, i, ram_read);
            mem_read_tab_set(7+j, i, ram_read);
            mem_read_tab_set(8+j, i, basic_hi_read);
            mem_read_tab_set(9+j, i, basic_hi_read);
            mem_read_tab_set(10+j, i, internal_function_rom_read);
            mem_read_tab_set(11+j, i, internal_function_rom_read);
            mem_read_tab_set(12+j, i, external_function_rom_read);
            mem_read_tab_set(13+j, i, external_function_rom_read);
            mem_read_tab_set(14+j, i, ram_read);
            mem_read_tab_set(15+j, i, ram_read);
            mem_read_tab_set(16+j, i, basic_hi_read);
            mem_read_tab_set(17+j, i, basic_hi_read);
            mem_read_tab_set(18+j, i, internal_function_rom_read);
            mem_read_tab_set(19+j, i, internal_function_rom_read);
            mem_read_tab_set(20+j, i, external_function_rom_read);
            mem_read_tab_set(21+j, i, external_function_rom_read);
            mem_read_tab_set(22+j, i, ram_read);
            mem_read_tab_set(23+j, i, ram_read);
            mem_read_tab_set(24+j, i, basic_hi_read);
            mem_read_tab_set(25+j, i, basic_hi_read);
            mem_read_tab_set(26+j, i, internal_function_rom_read);
            mem_read_tab_set(27+j, i, internal_function_rom_read);
            mem_read_tab_set(28+j, i, external_function_rom_read);
            mem_read_tab_set(29+j, i, external_function_rom_read);
            mem_read_tab_set(30+j, i, ram_read);
            mem_read_tab_set(31+j, i, ram_read);
            mem_read_tab_set(32+j, i, basic_hi_read);
            mem_read_tab_set(33+j, i, basic_hi_read);
            mem_read_tab_set(34+j, i, internal_function_rom_read);
            mem_read_tab_set(35+j, i, internal_function_rom_read);
            mem_read_tab_set(36+j, i, external_function_rom_read);
            mem_read_tab_set(37+j, i, external_function_rom_read);
            mem_read_tab_set(38+j, i, ram_read);
            mem_read_tab_set(39+j, i, ram_read);
            mem_read_tab_set(40+j, i, basic_hi_read);
            mem_read_tab_set(41+j, i, basic_hi_read);
            mem_read_tab_set(42+j, i, internal_function_rom_read);
            mem_read_tab_set(43+j, i, internal_function_rom_read);
            mem_read_tab_set(44+j, i, external_function_rom_read);
            mem_read_tab_set(45+j, i, external_function_rom_read);
            mem_read_tab_set(46+j, i, ram_read);
            mem_read_tab_set(47+j, i, ram_read);
            mem_read_tab_set(48+j, i, basic_hi_read);
            mem_read_tab_set(49+j, i, basic_hi_read);
            mem_read_tab_set(50+j, i, internal_function_rom_read);
            mem_read_tab_set(51+j, i, internal_function_rom_read);
            mem_read_tab_set(52+j, i, external_function_rom_read);
            mem_read_tab_set(53+j, i, external_function_rom_read);
            mem_read_tab_set(54+j, i, ram_read);
            mem_read_tab_set(55+j, i, ram_read);
            mem_read_tab_set(56+j, i, basic_hi_read);
            mem_read_tab_set(57+j, i, basic_hi_read);
            mem_read_tab_set(58+j, i, internal_function_rom_read);
            mem_read_tab_set(59+j, i, internal_function_rom_read);
            mem_read_tab_set(60+j, i, external_function_rom_read);
            mem_read_tab_set(61+j, i, external_function_rom_read);
            mem_read_tab_set(62+j, i, ram_read);
            mem_read_tab_set(63+j, i, ram_read);
            mem_set_write_hook(0+j, i, basic_hi_store);
            mem_set_write_hook(1+j, i, basic_hi_store);
            mem_set_write_hook(2+j, i, ram_store);
            mem_set_write_hook(3+j, i, ram_store);
            mem_set_write_hook(4+j, i, ram_store);
            mem_set_write_hook(5+j, i, ram_store);
            mem_set_write_hook(6+j, i, ram_store);
            mem_set_write_hook(7+j, i, ram_store);
            mem_set_write_hook(8+j, i, basic_hi_store);
            mem_set_write_hook(9+j, i, basic_hi_store);
            mem_set_write_hook(10+j, i, ram_store);
            mem_set_write_hook(11+j, i, ram_store);
            mem_set_write_hook(12+j, i, ram_store);
            mem_set_write_hook(13+j, i, ram_store);
            mem_set_write_hook(14+j, i, ram_store);
            mem_set_write_hook(15+j, i, ram_store);
            mem_set_write_hook(16+j, i, basic_hi_store);
            mem_set_write_hook(17+j, i, basic_hi_store);
            mem_set_write_hook(18+j, i, ram_store);
            mem_set_write_hook(19+j, i, ram_store);
            mem_set_write_hook(20+j, i, ram_store);
            mem_set_write_hook(21+j, i, ram_store);
            mem_set_write_hook(22+j, i, ram_store);
            mem_set_write_hook(23+j, i, ram_store);
            mem_set_write_hook(24+j, i, basic_hi_store);
            mem_set_write_hook(25+j, i, basic_hi_store);
            mem_set_write_hook(26+j, i, ram_store);
            mem_set_write_hook(27+j, i, ram_store);
            mem_set_write_hook(28+j, i, ram_store);
            mem_set_write_hook(29+j, i, ram_store);
            mem_set_write_hook(30+j, i, ram_store);
            mem_set_write_hook(31+j, i, ram_store);
            mem_set_write_hook(32+j, i, basic_hi_store);
            mem_set_write_hook(33+j, i, basic_hi_store);
            mem_set_write_hook(34+j, i, ram_store);
            mem_set_write_hook(35+j, i, ram_store);
            mem_set_write_hook(36+j, i, ram_store);
            mem_set_write_hook(37+j, i, ram_store);
            mem_set_write_hook(38+j, i, ram_store);
            mem_set_write_hook(39+j, i, ram_store);
            mem_set_write_hook(40+j, i, basic_hi_store);
            mem_set_write_hook(41+j, i, basic_hi_store);
            mem_set_write_hook(42+j, i, ram_store);
            mem_set_write_hook(43+j, i, ram_store);
            mem_set_write_hook(44+j, i, ram_store);
            mem_set_write_hook(45+j, i, ram_store);
            mem_set_write_hook(46+j, i, ram_store);
            mem_set_write_hook(47+j, i, ram_store);
            mem_set_write_hook(48+j, i, basic_hi_store);
            mem_set_write_hook(49+j, i, basic_hi_store);
            mem_set_write_hook(50+j, i, ram_store);
            mem_set_write_hook(51+j, i, ram_store);
            mem_set_write_hook(52+j, i, ram_store);
            mem_set_write_hook(53+j, i, ram_store);
            mem_set_write_hook(54+j, i, ram_store);
            mem_set_write_hook(55+j, i, ram_store);
            mem_set_write_hook(56+j, i, basic_hi_store);
            mem_set_write_hook(57+j, i, basic_hi_store);
            mem_set_write_hook(58+j, i, ram_store);
            mem_set_write_hook(59+j, i, ram_store);
            mem_set_write_hook(60+j, i, ram_store);
            mem_set_write_hook(61+j, i, ram_store);
            mem_set_write_hook(62+j, i, ram_store);
            mem_set_write_hook(63+j, i, ram_store);
            mem_read_base_set(0+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(1+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(2+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(3+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(4+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(5+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(6+j, i, mem_ram + (i << 8));
            mem_read_base_set(7+j, i, mem_ram + (i << 8));
            mem_read_base_set(8+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(9+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(10+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(11+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(12+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(13+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(14+j, i, mem_ram + (i << 8));
            mem_read_base_set(15+j, i, mem_ram + (i << 8));
            mem_read_base_set(16+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(17+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(18+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(19+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(20+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(21+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(22+j, i, mem_ram + (i << 8));
            mem_read_base_set(23+j, i, mem_ram + (i << 8));
            mem_read_base_set(24+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(25+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(26+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(27+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(28+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(29+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(30+j, i, mem_ram + (i << 8));
            mem_read_base_set(31+j, i, mem_ram + (i << 8));
            mem_read_base_set(32+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(33+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(34+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(35+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(36+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(37+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(38+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(39+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(40+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(41+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(42+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(43+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(44+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(45+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(46+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(47+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(48+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(49+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(50+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(51+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(52+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(53+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(54+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(55+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(56+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(57+j, i, c128memrom_basic_rom
                              + 0x4000 + ((i & 0x3f) << 8));
            mem_read_base_set(58+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(59+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(60+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(61+j, i, ext_function_rom + ((i & 0x3f) << 8));
            mem_read_base_set(62+j, i, mem_ram + 0x10000 + (i << 8));
            mem_read_base_set(63+j, i, mem_ram + 0x10000 + (i << 8));
        }

        for (i = 0xc0; i <= 0xcf; i++) {
            mem_read_tab_set(0+j, i, editor_read);
            mem_read_tab_set(1+j, i, editor_read);
            mem_read_tab_set(2+j, i, editor_read);
            mem_read_tab_set(3+j, i, editor_read);
            mem_read_tab_set(4+j, i, editor_read);
            mem_read_tab_set(5+j, i, editor_read);
            mem_read_tab_set(6+j, i, editor_read);
            mem_read_tab_set(7+j, i, editor_read);
            mem_read_tab_set(8+j, i, internal_function_rom_read);
            mem_read_tab_set(9+j, i, internal_function_rom_read);
            mem_read_tab_set(10+j, i, internal_function_rom_read);
            mem_read_tab_set(11+j, i, internal_function_rom_read);
            mem_read_tab_set(12+j, i, internal_function_rom_read);
            mem_read_tab_set(13+j, i, internal_function_rom_read);
            mem_read_tab_set(14+j, i, internal_function_rom_read);
            mem_read_tab_set(15+j, i, internal_function_rom_read);
            mem_read_tab_set(16+j, i, ram_read);
            mem_read_tab_set(17+j, i, ram_read);
            mem_read_tab_set(18+j, i, ram_read);
            mem_read_tab_set(19+j, i, ram_read);
            mem_read_tab_set(20+j, i, ram_read);
            mem_read_tab_set(21+j, i, ram_read);
            mem_read_tab_set(22+j, i, ram_read);
            mem_read_tab_set(23+j, i, ram_read);
            mem_read_tab_set(24+j, i, ram_read);
            mem_read_tab_set(25+j, i, ram_read);
            mem_read_tab_set(26+j, i, ram_read);
            mem_read_tab_set(27+j, i, ram_read);
            mem_read_tab_set(28+j, i, ram_read);
            mem_read_tab_set(29+j, i, ram_read);
            mem_read_tab_set(30+j, i, ram_read);
            mem_read_tab_set(31+j, i, ram_read);
            mem_read_tab_set(32+j, i, editor_read);
            mem_read_tab_set(33+j, i, editor_read);
            mem_read_tab_set(34+j, i, editor_read);
            mem_read_tab_set(35+j, i, editor_read);
            mem_read_tab_set(36+j, i, editor_read);
            mem_read_tab_set(37+j, i, editor_read);
            mem_read_tab_set(38+j, i, editor_read);
            mem_read_tab_set(39+j, i, editor_read);
            mem_read_tab_set(40+j, i, internal_function_rom_read);
            mem_read_tab_set(41+j, i, internal_function_rom_read);
            mem_read_tab_set(42+j, i, internal_function_rom_read);
            mem_read_tab_set(43+j, i, internal_function_rom_read);
            mem_read_tab_set(44+j, i, internal_function_rom_read);
            mem_read_tab_set(45+j, i, internal_function_rom_read);
            mem_read_tab_set(46+j, i, internal_function_rom_read);
            mem_read_tab_set(47+j, i, internal_function_rom_read);
            mem_read_tab_set(48+j, i, top_shared_read);
            mem_read_tab_set(49+j, i, top_shared_read);
            mem_read_tab_set(50+j, i, top_shared_read);
            mem_read_tab_set(51+j, i, top_shared_read);
            mem_read_tab_set(52+j, i, top_shared_read);
            mem_read_tab_set(53+j, i, top_shared_read);
            mem_read_tab_set(54+j, i, top_shared_read);
            mem_read_tab_set(55+j, i, top_shared_read);
            mem_read_tab_set(56+j, i, top_shared_read);
            mem_read_tab_set(57+j, i, top_shared_read);
            mem_read_tab_set(58+j, i, top_shared_read);
            mem_read_tab_set(59+j, i, top_shared_read);
            mem_read_tab_set(60+j, i, top_shared_read);
            mem_read_tab_set(61+j, i, top_shared_read);
            mem_read_tab_set(62+j, i, top_shared_read);
            mem_read_tab_set(63+j, i, top_shared_read);
            mem_set_write_hook(0+j, i, editor_store);
            mem_set_write_hook(1+j, i, editor_store);
            mem_set_write_hook(2+j, i, editor_store);
            mem_set_write_hook(3+j, i, editor_store);
            mem_set_write_hook(4+j, i, editor_store);
            mem_set_write_hook(5+j, i, editor_store);
            mem_set_write_hook(6+j, i, editor_store);
            mem_set_write_hook(7+j, i, editor_store);
            mem_set_write_hook(8+j, i, ram_store);
            mem_set_write_hook(9+j, i, ram_store);
            mem_set_write_hook(10+j, i, ram_store);
            mem_set_write_hook(11+j, i, ram_store);
            mem_set_write_hook(12+j, i, ram_store);
            mem_set_write_hook(13+j, i, ram_store);
            mem_set_write_hook(14+j, i, ram_store);
            mem_set_write_hook(15+j, i, ram_store);
            mem_set_write_hook(16+j, i, ram_store);
            mem_set_write_hook(17+j, i, ram_store);
            mem_set_write_hook(18+j, i, ram_store);
            mem_set_write_hook(19+j, i, ram_store);
            mem_set_write_hook(20+j, i, ram_store);
            mem_set_write_hook(21+j, i, ram_store);
            mem_set_write_hook(22+j, i, ram_store);
            mem_set_write_hook(23+j, i, ram_store);
            mem_set_write_hook(24+j, i, ram_store);
            mem_set_write_hook(25+j, i, ram_store);
            mem_set_write_hook(26+j, i, ram_store);
            mem_set_write_hook(27+j, i, ram_store);
            mem_set_write_hook(28+j, i, ram_store);
            mem_set_write_hook(29+j, i, ram_store);
            mem_set_write_hook(30+j, i, ram_store);
            mem_set_write_hook(31+j, i, ram_store);
            mem_set_write_hook(32+j, i, editor_store);
            mem_set_write_hook(33+j, i, editor_store);
            mem_set_write_hook(34+j, i, editor_store);
            mem_set_write_hook(35+j, i, editor_store);
            mem_set_write_hook(36+j, i, editor_store);
            mem_set_write_hook(37+j, i, editor_store);
            mem_set_write_hook(38+j, i, editor_store);
            mem_set_write_hook(39+j, i, editor_store);
            mem_set_write_hook(40+j, i, top_shared_store);
            mem_set_write_hook(41+j, i, top_shared_store);
            mem_set_write_hook(42+j, i, top_shared_store);
            mem_set_write_hook(43+j, i, top_shared_store);
            mem_set_write_hook(44+j, i, top_shared_store);
            mem_set_write_hook(45+j, i, top_shared_store);
            mem_set_write_hook(46+j, i, top_shared_store);
            mem_set_write_hook(47+j, i, top_shared_store);
            mem_set_write_hook(48+j, i, top_shared_store);
            mem_set_write_hook(49+j, i, top_shared_store);
            mem_set_write_hook(50+j, i, top_shared_store);
            mem_set_write_hook(51+j, i, top_shared_store);
            mem_set_write_hook(52+j, i, top_shared_store);
            mem_set_write_hook(53+j, i, top_shared_store);
            mem_set_write_hook(54+j, i, top_shared_store);
            mem_set_write_hook(55+j, i, top_shared_store);
            mem_set_write_hook(56+j, i, top_shared_store);
            mem_set_write_hook(57+j, i, top_shared_store);
            mem_set_write_hook(58+j, i, top_shared_store);
            mem_set_write_hook(59+j, i, top_shared_store);
            mem_set_write_hook(60+j, i, top_shared_store);
            mem_set_write_hook(61+j, i, top_shared_store);
            mem_set_write_hook(62+j, i, top_shared_store);
            mem_set_write_hook(63+j, i, top_shared_store);
            mem_read_base_set(0+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(1+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(2+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(3+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(4+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(5+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(6+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(7+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(8+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(9+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(10+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(11+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(12+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(13+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(14+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(15+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(16+j, i, mem_ram + (i << 8));
            mem_read_base_set(17+j, i, mem_ram + (i << 8));
            mem_read_base_set(18+j, i, mem_ram + (i << 8));
            mem_read_base_set(19+j, i, mem_ram + (i << 8));
            mem_read_base_set(20+j, i, mem_ram + (i << 8));
            mem_read_base_set(21+j, i, mem_ram + (i << 8));
            mem_read_base_set(22+j, i, mem_ram + (i << 8));
            mem_read_base_set(23+j, i, mem_ram + (i << 8));
            mem_read_base_set(24+j, i, mem_ram + (i << 8));
            mem_read_base_set(25+j, i, mem_ram + (i << 8));
            mem_read_base_set(26+j, i, mem_ram + (i << 8));
            mem_read_base_set(27+j, i, mem_ram + (i << 8));
            mem_read_base_set(28+j, i, mem_ram + (i << 8));
            mem_read_base_set(29+j, i, mem_ram + (i << 8));
            mem_read_base_set(30+j, i, mem_ram + (i << 8));
            mem_read_base_set(31+j, i, mem_ram + (i << 8));
            mem_read_base_set(32+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(33+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(34+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(35+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(36+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(37+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(38+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(39+j, i, c128memrom_basic_rom
                              + 0x8000 + ((i & 0xf) << 8));
            mem_read_base_set(40+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(41+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(42+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(43+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(44+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(45+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(46+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(47+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(48+j, i, NULL);
            mem_read_base_set(49+j, i, NULL);
            mem_read_base_set(50+j, i, NULL);
            mem_read_base_set(51+j, i, NULL);
            mem_read_base_set(52+j, i, NULL);
            mem_read_base_set(53+j, i, NULL);
            mem_read_base_set(54+j, i, NULL);
            mem_read_base_set(55+j, i, NULL);
            mem_read_base_set(56+j, i, NULL);
            mem_read_base_set(57+j, i, NULL);
            mem_read_base_set(58+j, i, NULL);
            mem_read_base_set(59+j, i, NULL);
            mem_read_base_set(60+j, i, NULL);
            mem_read_base_set(61+j, i, NULL);
            mem_read_base_set(62+j, i, NULL);
            mem_read_base_set(63+j, i, NULL);
        }
    }

    for (i = 0xd0; i <= 0xdf; i++) {
        mem_read_tab_set(0, i, chargen_read);
        mem_read_tab_set(1, i, chargen_read);
        mem_read_tab_set(2, i, chargen_read);
        mem_read_tab_set(3, i, chargen_read);
        mem_read_tab_set(4, i, chargen_read);
        mem_read_tab_set(5, i, chargen_read);
        mem_read_tab_set(6, i, chargen_read);
        mem_read_tab_set(7, i, chargen_read);
        mem_read_tab_set(8, i, internal_function_rom_read);
        mem_read_tab_set(9, i, internal_function_rom_read);
        mem_read_tab_set(10, i, internal_function_rom_read);
        mem_read_tab_set(11, i, internal_function_rom_read);
        mem_read_tab_set(12, i, internal_function_rom_read);
        mem_read_tab_set(13, i, internal_function_rom_read);
        mem_read_tab_set(14, i, internal_function_rom_read);
        mem_read_tab_set(15, i, internal_function_rom_read);
        mem_read_tab_set(16, i, ram_read);
        mem_read_tab_set(17, i, ram_read);
        mem_read_tab_set(18, i, ram_read);
        mem_read_tab_set(19, i, ram_read);
        mem_read_tab_set(20, i, ram_read);
        mem_read_tab_set(21, i, ram_read);
        mem_read_tab_set(22, i, ram_read);
        mem_read_tab_set(23, i, ram_read);
        mem_read_tab_set(24, i, ram_read);
        mem_read_tab_set(25, i, ram_read);
        mem_read_tab_set(26, i, ram_read);
        mem_read_tab_set(27, i, ram_read);
        mem_read_tab_set(28, i, ram_read);
        mem_read_tab_set(29, i, ram_read);
        mem_read_tab_set(30, i, ram_read);
        mem_read_tab_set(31, i, ram_read);
        mem_read_tab_set(32, i, chargen_read);
        mem_read_tab_set(33, i, chargen_read);
        mem_read_tab_set(34, i, chargen_read);
        mem_read_tab_set(35, i, chargen_read);
        mem_read_tab_set(36, i, chargen_read);
        mem_read_tab_set(37, i, chargen_read);
        mem_read_tab_set(38, i, chargen_read);
        mem_read_tab_set(39, i, chargen_read);
        mem_read_tab_set(40, i, internal_function_rom_read);
        mem_read_tab_set(41, i, internal_function_rom_read);
        mem_read_tab_set(42, i, internal_function_rom_read);
        mem_read_tab_set(43, i, internal_function_rom_read);
        mem_read_tab_set(44, i, internal_function_rom_read);
        mem_read_tab_set(45, i, internal_function_rom_read);
        mem_read_tab_set(46, i, internal_function_rom_read);
        mem_read_tab_set(47, i, internal_function_rom_read);
        mem_read_tab_set(48, i, top_shared_read);
        mem_read_tab_set(49, i, top_shared_read);
        mem_read_tab_set(50, i, top_shared_read);
        mem_read_tab_set(51, i, top_shared_read);
        mem_read_tab_set(52, i, top_shared_read);
        mem_read_tab_set(53, i, top_shared_read);
        mem_read_tab_set(54, i, top_shared_read);
        mem_read_tab_set(55, i, top_shared_read);
        mem_read_tab_set(56, i, top_shared_read);
        mem_read_tab_set(57, i, top_shared_read);
        mem_read_tab_set(58, i, top_shared_read);
        mem_read_tab_set(59, i, top_shared_read);
        mem_read_tab_set(60, i, top_shared_read);
        mem_read_tab_set(61, i, top_shared_read);
        mem_read_tab_set(62, i, top_shared_read);
        mem_read_tab_set(63, i, top_shared_read);
        mem_set_write_hook(0, i, hi_store);
        mem_set_write_hook(1, i, hi_store);
        mem_set_write_hook(2, i, hi_store);
        mem_set_write_hook(3, i, hi_store);
        mem_set_write_hook(4, i, hi_store);
        mem_set_write_hook(5, i, hi_store);
        mem_set_write_hook(6, i, hi_store);
        mem_set_write_hook(7, i, hi_store);
        mem_set_write_hook(8, i, ram_store);
        mem_set_write_hook(9, i, ram_store);
        mem_set_write_hook(10, i, ram_store);
        mem_set_write_hook(11, i, ram_store);
        mem_set_write_hook(12, i, ram_store);
        mem_set_write_hook(13, i, ram_store);
        mem_set_write_hook(14, i, ram_store);
        mem_set_write_hook(15, i, ram_store);
        mem_set_write_hook(16, i, ram_store);
        mem_set_write_hook(17, i, ram_store);
        mem_set_write_hook(18, i, ram_store);
        mem_set_write_hook(19, i, ram_store);
        mem_set_write_hook(20, i, ram_store);
        mem_set_write_hook(21, i, ram_store);
        mem_set_write_hook(22, i, ram_store);
        mem_set_write_hook(23, i, ram_store);
        mem_set_write_hook(24, i, ram_store);
        mem_set_write_hook(25, i, ram_store);
        mem_set_write_hook(26, i, ram_store);
        mem_set_write_hook(27, i, ram_store);
        mem_set_write_hook(28, i, ram_store);
        mem_set_write_hook(29, i, ram_store);
        mem_set_write_hook(30, i, ram_store);
        mem_set_write_hook(31, i, ram_store);
        mem_set_write_hook(32, i, hi_store);
        mem_set_write_hook(33, i, hi_store);
        mem_set_write_hook(34, i, hi_store);
        mem_set_write_hook(35, i, hi_store);
        mem_set_write_hook(36, i, hi_store);
        mem_set_write_hook(37, i, hi_store);
        mem_set_write_hook(38, i, hi_store);
        mem_set_write_hook(39, i, hi_store);
        mem_set_write_hook(40, i, top_shared_store);
        mem_set_write_hook(41, i, top_shared_store);
        mem_set_write_hook(42, i, top_shared_store);
        mem_set_write_hook(43, i, top_shared_store);
        mem_set_write_hook(44, i, top_shared_store);
        mem_set_write_hook(45, i, top_shared_store);
        mem_set_write_hook(46, i, top_shared_store);
        mem_set_write_hook(47, i, top_shared_store);
        mem_set_write_hook(48, i, top_shared_store);
        mem_set_write_hook(49, i, top_shared_store);
        mem_set_write_hook(50, i, top_shared_store);
        mem_set_write_hook(51, i, top_shared_store);
        mem_set_write_hook(52, i, top_shared_store);
        mem_set_write_hook(53, i, top_shared_store);
        mem_set_write_hook(54, i, top_shared_store);
        mem_set_write_hook(55, i, top_shared_store);
        mem_set_write_hook(56, i, top_shared_store);
        mem_set_write_hook(57, i, top_shared_store);
        mem_set_write_hook(58, i, top_shared_store);
        mem_set_write_hook(59, i, top_shared_store);
        mem_set_write_hook(60, i, top_shared_store);
        mem_set_write_hook(61, i, top_shared_store);
        mem_set_write_hook(62, i, top_shared_store);
        mem_set_write_hook(63, i, top_shared_store);
        mem_read_base_set(0, i, NULL);
        mem_read_base_set(1, i, NULL);
        mem_read_base_set(2, i, NULL);
        mem_read_base_set(3, i, NULL);
        mem_read_base_set(4, i, NULL);
        mem_read_base_set(5, i, NULL);
        mem_read_base_set(6, i, NULL);
        mem_read_base_set(7, i, NULL);
        mem_read_base_set(8, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(9, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(10, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(11, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(12, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(13, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(14, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(15, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(16, i, mem_ram + (i << 8));
        mem_read_base_set(17, i, mem_ram + (i << 8));
        mem_read_base_set(18, i, mem_ram + (i << 8));
        mem_read_base_set(19, i, mem_ram + (i << 8));
        mem_read_base_set(20, i, mem_ram + (i << 8));
        mem_read_base_set(21, i, mem_ram + (i << 8));
        mem_read_base_set(22, i, mem_ram + (i << 8));
        mem_read_base_set(23, i, mem_ram + (i << 8));
        mem_read_base_set(24, i, mem_ram + (i << 8));
        mem_read_base_set(25, i, mem_ram + (i << 8));
        mem_read_base_set(26, i, mem_ram + (i << 8));
        mem_read_base_set(27, i, mem_ram + (i << 8));
        mem_read_base_set(28, i, mem_ram + (i << 8));
        mem_read_base_set(29, i, mem_ram + (i << 8));
        mem_read_base_set(30, i, mem_ram + (i << 8));
        mem_read_base_set(31, i, mem_ram + (i << 8));
        mem_read_base_set(32, i, NULL);
        mem_read_base_set(33, i, NULL);
        mem_read_base_set(34, i, NULL);
        mem_read_base_set(35, i, NULL);
        mem_read_base_set(36, i, NULL);
        mem_read_base_set(37, i, NULL);
        mem_read_base_set(38, i, NULL);
        mem_read_base_set(39, i, NULL);
        mem_read_base_set(40, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(41, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(42, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(43, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(44, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(45, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(46, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(47, i, int_function_rom + ((i & 0x7f) << 8));
        mem_read_base_set(48, i, NULL);
        mem_read_base_set(49, i, NULL);
        mem_read_base_set(50, i, NULL);
        mem_read_base_set(51, i, NULL);
        mem_read_base_set(52, i, NULL);
        mem_read_base_set(53, i, NULL);
        mem_read_base_set(54, i, NULL);
        mem_read_base_set(55, i, NULL);
        mem_read_base_set(56, i, NULL);
        mem_read_base_set(57, i, NULL);
        mem_read_base_set(58, i, NULL);
        mem_read_base_set(59, i, NULL);
        mem_read_base_set(60, i, NULL);
        mem_read_base_set(61, i, NULL);
        mem_read_base_set(62, i, NULL);
        mem_read_base_set(63, i, NULL);
    }

    for (j = 64; j < 128; j++) {
        for (i = 0xd0; i <= 0xd3; i++) {
            mem_read_tab_set(j, i, vicii_read);
            mem_set_write_hook(j, i, vicii_store);
        }

        mem_read_tab_set(j, 0xd4, sid_read);
        mem_set_write_hook(j, 0xd4, sid_store);
        mem_read_tab_set(j, 0xd5, mmu_read);
        mem_set_write_hook(j, 0xd5, mmu_store);
        mem_read_tab_set(j, 0xd6, vdc_read);
        mem_set_write_hook(j, 0xd6, vdc_store);
        mem_read_tab_set(j, 0xd7, d7xx_read);
        mem_set_write_hook(j, 0xd7, d7xx_store);

        mem_read_tab_set(j, 0xd8, colorram_read);
        mem_read_tab_set(j, 0xd9, colorram_read);
        mem_read_tab_set(j, 0xda, colorram_read);
        mem_read_tab_set(j, 0xdb, colorram_read);
        mem_set_write_hook(j, 0xd8, colorram_store);
        mem_set_write_hook(j, 0xd9, colorram_store);
        mem_set_write_hook(j, 0xda, colorram_store);
        mem_set_write_hook(j, 0xdb, colorram_store);

        mem_read_tab_set(j, 0xdc, cia1_read);
        mem_set_write_hook(j, 0xdc, cia1_store);
        mem_read_tab_set(j, 0xdd, cia2_read);
        mem_set_write_hook(j, 0xdd, cia2_store);

        mem_read_tab_set(j, 0xde, io1_read);
        mem_set_write_hook(j, 0xde, io1_store);
        mem_read_tab_set(j, 0xdf, io2_read);
        mem_set_write_hook(j, 0xdf, io2_store);
    }

    for (j = 0; j < 128; j += 64) {
        for (i = 0xe0; i <= 0xfe; i++) {
            mem_read_tab_set(0+j, i, hi_read);
            mem_read_tab_set(1+j, i, hi_read);
            mem_read_tab_set(2+j, i, hi_read);
            mem_read_tab_set(3+j, i, hi_read);
            mem_read_tab_set(4+j, i, hi_read);
            mem_read_tab_set(5+j, i, hi_read);
            mem_read_tab_set(6+j, i, hi_read);
            mem_read_tab_set(7+j, i, hi_read);
            mem_read_tab_set(8+j, i, internal_function_rom_read);
            mem_read_tab_set(9+j, i, internal_function_rom_read);
            mem_read_tab_set(10+j, i, internal_function_rom_read);
            mem_read_tab_set(11+j, i, internal_function_rom_read);
            mem_read_tab_set(12+j, i, internal_function_rom_read);
            mem_read_tab_set(13+j, i, internal_function_rom_read);
            mem_read_tab_set(14+j, i, internal_function_rom_read);
            mem_read_tab_set(15+j, i, internal_function_rom_read);
            mem_read_tab_set(16+j, i, ram_read);
            mem_read_tab_set(17+j, i, ram_read);
            mem_read_tab_set(18+j, i, ram_read);
            mem_read_tab_set(19+j, i, ram_read);
            mem_read_tab_set(20+j, i, ram_read);
            mem_read_tab_set(21+j, i, ram_read);
            mem_read_tab_set(22+j, i, ram_read);
            mem_read_tab_set(23+j, i, ram_read);
            mem_read_tab_set(24+j, i, ram_read);
            mem_read_tab_set(25+j, i, ram_read);
            mem_read_tab_set(26+j, i, ram_read);
            mem_read_tab_set(27+j, i, ram_read);
            mem_read_tab_set(28+j, i, ram_read);
            mem_read_tab_set(29+j, i, ram_read);
            mem_read_tab_set(30+j, i, ram_read);
            mem_read_tab_set(31+j, i, ram_read);
            mem_read_tab_set(32+j, i, hi_read);
            mem_read_tab_set(33+j, i, hi_read);
            mem_read_tab_set(34+j, i, hi_read);
            mem_read_tab_set(35+j, i, hi_read);
            mem_read_tab_set(36+j, i, hi_read);
            mem_read_tab_set(37+j, i, hi_read);
            mem_read_tab_set(38+j, i, hi_read);
            mem_read_tab_set(39+j, i, hi_read);
            mem_read_tab_set(40+j, i, internal_function_rom_read);
            mem_read_tab_set(41+j, i, internal_function_rom_read);
            mem_read_tab_set(42+j, i, internal_function_rom_read);
            mem_read_tab_set(43+j, i, internal_function_rom_read);
            mem_read_tab_set(44+j, i, internal_function_rom_read);
            mem_read_tab_set(45+j, i, internal_function_rom_read);
            mem_read_tab_set(46+j, i, internal_function_rom_read);
            mem_read_tab_set(47+j, i, internal_function_rom_read);
            mem_read_tab_set(48+j, i, top_shared_read);
            mem_read_tab_set(49+j, i, top_shared_read);
            mem_read_tab_set(50+j, i, top_shared_read);
            mem_read_tab_set(51+j, i, top_shared_read);
            mem_read_tab_set(52+j, i, top_shared_read);
            mem_read_tab_set(53+j, i, top_shared_read);
            mem_read_tab_set(54+j, i, top_shared_read);
            mem_read_tab_set(55+j, i, top_shared_read);
            mem_read_tab_set(56+j, i, top_shared_read);
            mem_read_tab_set(57+j, i, top_shared_read);
            mem_read_tab_set(58+j, i, top_shared_read);
            mem_read_tab_set(59+j, i, top_shared_read);
            mem_read_tab_set(60+j, i, top_shared_read);
            mem_read_tab_set(61+j, i, top_shared_read);
            mem_read_tab_set(62+j, i, top_shared_read);
            mem_read_tab_set(63+j, i, top_shared_read);
            mem_set_write_hook(0+j, i, hi_store);
            mem_set_write_hook(1+j, i, hi_store);
            mem_set_write_hook(2+j, i, hi_store);
            mem_set_write_hook(3+j, i, hi_store);
            mem_set_write_hook(4+j, i, hi_store);
            mem_set_write_hook(5+j, i, hi_store);
            mem_set_write_hook(6+j, i, hi_store);
            mem_set_write_hook(7+j, i, hi_store);
            mem_set_write_hook(8+j, i, ram_store);
            mem_set_write_hook(9+j, i, ram_store);
            mem_set_write_hook(10+j, i, ram_store);
            mem_set_write_hook(11+j, i, ram_store);
            mem_set_write_hook(12+j, i, ram_store);
            mem_set_write_hook(13+j, i, ram_store);
            mem_set_write_hook(14+j, i, ram_store);
            mem_set_write_hook(15+j, i, ram_store);
            mem_set_write_hook(16+j, i, ram_store);
            mem_set_write_hook(17+j, i, ram_store);
            mem_set_write_hook(18+j, i, ram_store);
            mem_set_write_hook(19+j, i, ram_store);
            mem_set_write_hook(20+j, i, ram_store);
            mem_set_write_hook(21+j, i, ram_store);
            mem_set_write_hook(22+j, i, ram_store);
            mem_set_write_hook(23+j, i, ram_store);
            mem_set_write_hook(24+j, i, ram_store);
            mem_set_write_hook(25+j, i, ram_store);
            mem_set_write_hook(26+j, i, ram_store);
            mem_set_write_hook(27+j, i, ram_store);
            mem_set_write_hook(28+j, i, ram_store);
            mem_set_write_hook(29+j, i, ram_store);
            mem_set_write_hook(30+j, i, ram_store);
            mem_set_write_hook(31+j, i, ram_store);
            mem_set_write_hook(32+j, i, hi_store);
            mem_set_write_hook(33+j, i, hi_store);
            mem_set_write_hook(34+j, i, hi_store);
            mem_set_write_hook(35+j, i, hi_store);
            mem_set_write_hook(36+j, i, hi_store);
            mem_set_write_hook(37+j, i, hi_store);
            mem_set_write_hook(38+j, i, hi_store);
            mem_set_write_hook(39+j, i, hi_store);
            mem_set_write_hook(40+j, i, top_shared_store);
            mem_set_write_hook(41+j, i, top_shared_store);
            mem_set_write_hook(42+j, i, top_shared_store);
            mem_set_write_hook(43+j, i, top_shared_store);
            mem_set_write_hook(44+j, i, top_shared_store);
            mem_set_write_hook(45+j, i, top_shared_store);
            mem_set_write_hook(46+j, i, top_shared_store);
            mem_set_write_hook(47+j, i, top_shared_store);
            mem_set_write_hook(48+j, i, top_shared_store);
            mem_set_write_hook(49+j, i, top_shared_store);
            mem_set_write_hook(50+j, i, top_shared_store);
            mem_set_write_hook(51+j, i, top_shared_store);
            mem_set_write_hook(52+j, i, top_shared_store);
            mem_set_write_hook(53+j, i, top_shared_store);
            mem_set_write_hook(54+j, i, top_shared_store);
            mem_set_write_hook(55+j, i, top_shared_store);
            mem_set_write_hook(56+j, i, top_shared_store);
            mem_set_write_hook(57+j, i, top_shared_store);
            mem_set_write_hook(58+j, i, top_shared_store);
            mem_set_write_hook(59+j, i, top_shared_store);
            mem_set_write_hook(60+j, i, top_shared_store);
            mem_set_write_hook(61+j, i, top_shared_store);
            mem_set_write_hook(62+j, i, top_shared_store);
            mem_set_write_hook(63+j, i, top_shared_store);
            mem_read_base_set(0+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(1+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(2+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(3+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(4+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(5+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(6+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(7+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(8+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(9+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(10+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(11+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(12+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(13+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(14+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(15+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(16+j, i, mem_ram + (i << 8));
            mem_read_base_set(17+j, i, mem_ram + (i << 8));
            mem_read_base_set(18+j, i, mem_ram + (i << 8));
            mem_read_base_set(19+j, i, mem_ram + (i << 8));
            mem_read_base_set(20+j, i, mem_ram + (i << 8));
            mem_read_base_set(21+j, i, mem_ram + (i << 8));
            mem_read_base_set(22+j, i, mem_ram + (i << 8));
            mem_read_base_set(23+j, i, mem_ram + (i << 8));
            mem_read_base_set(24+j, i, mem_ram + (i << 8));
            mem_read_base_set(25+j, i, mem_ram + (i << 8));
            mem_read_base_set(26+j, i, mem_ram + (i << 8));
            mem_read_base_set(27+j, i, mem_ram + (i << 8));
            mem_read_base_set(28+j, i, mem_ram + (i << 8));
            mem_read_base_set(29+j, i, mem_ram + (i << 8));
            mem_read_base_set(30+j, i, mem_ram + (i << 8));
            mem_read_base_set(31+j, i, mem_ram + (i << 8));
            mem_read_base_set(32+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(33+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(34+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(35+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(36+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(37+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(38+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(39+j, i,
                              c128memrom_kernal_trap_rom + ((i & 0x1f) << 8));
            mem_read_base_set(40+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(41+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(42+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(43+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(44+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(45+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(46+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(47+j, i, int_function_rom + ((i & 0x7f) << 8));
            mem_read_base_set(48+j, i, NULL);
            mem_read_base_set(49+j, i, NULL);
            mem_read_base_set(50+j, i, NULL);
            mem_read_base_set(51+j, i, NULL);
            mem_read_base_set(52+j, i, NULL);
            mem_read_base_set(53+j, i, NULL);
            mem_read_base_set(54+j, i, NULL);
            mem_read_base_set(55+j, i, NULL);
            mem_read_base_set(56+j, i, NULL);
            mem_read_base_set(57+j, i, NULL);
            mem_read_base_set(58+j, i, NULL);
            mem_read_base_set(59+j, i, NULL);
            mem_read_base_set(60+j, i, NULL);
            mem_read_base_set(61+j, i, NULL);
            mem_read_base_set(62+j, i, NULL);
            mem_read_base_set(63+j, i, NULL);
        }
    }

    for (j = 0; j < 128; j++) {
        mem_read_tab_set(j, 0xff, mmu_ffxx_read);
        mem_set_write_hook(j, 0xff, mmu_ffxx_store);

        mem_read_tab_set(j, 0x100, zero_read);
        mem_set_write_hook(j, 0x100, zero_store);

        mem_read_base_set(j, 0x100, NULL);
    }
}

