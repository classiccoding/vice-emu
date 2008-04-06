/*
 * uitfe.h - Implementation of the REU settings dialog box.
 *
 * Written by
 *  Spiro Trikaliotis <spiro.trikaliotis@gmx.de>
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

#ifndef _UITFE_H
#define _UITFE_H

#ifdef HAVE_TFE
extern void ui_tfe_settings_dialog(HWND hwnd);

/*
 These functions let the UI enumerate the available interfaces.

 First, TfeEnumAdapterOpen() is used to start enumeration.

 TfeEnumAdapter is then used to gather information for each adapter present
 on the system, where:

   ppname points to a pointer which will hold the name of the interface
   ppdescription points to a pointer which will hold the description of the interface

   For each of these parameters, new memory is allocated, so it has to be
   freed with lib_free().

 TfeEnumAdapterClose() must be used to stop processing.

 Each function returns 1 on success, and 0 on failure.
 TfeEnumAdapter() only fails if there is no more adpater; in this case, 
   *ppname and *ppdescription are not altered.
*/
extern int TfeEnumAdapterOpen(void);
extern int TfeEnumAdapter(char **ppname, char **ppdescription);
extern int TfeEnumAdapterClose(void);

#endif // #ifdef HAVE_TFE

#endif

