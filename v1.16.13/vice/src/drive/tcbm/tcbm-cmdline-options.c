/*
 * tcbm-cmdline-options.c
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

#include "cmdline.h"
#include "tcbm-cmdline-options.h"

#ifdef HAS_TRANSLATION
#include "translate.h"

static const cmdline_option_trans_t cmdline_options[] = {
    { "-dos1551", SET_RESOURCE, 1, NULL, NULL, "DosName1551", "dos1551",
      IDCLS_P_NAME, IDCLS_SPECIFY_1551_DOS_ROM_NAME },
    { NULL }
};
#else
static const cmdline_option_t cmdline_options[] = {
    { "-dos1551", SET_RESOURCE, 1, NULL, NULL, "DosName1551", "dos1551",
      "<name>", "Specify name of 1551 DOS ROM image" },
    { NULL }
};
#endif

int tcbm_cmdline_options_init(void)
{
#ifdef HAS_TRANSLATION
    return cmdline_register_options_trans(cmdline_options);
#else
    return cmdline_register_options(cmdline_options);
#endif
}

