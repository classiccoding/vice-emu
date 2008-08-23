/*
 * c64ui.c - C64-specific user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@axelero.hu>
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
#include <stdlib.h>
#include <windows.h>

#include "c64ui.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uic64dtv.h"
#include "uidrivec64.h"
#include "uikeyboard.h"
#include "uirom.h"
#include "uirs232user.h"
#include "uisid.h"
#include "uivicii.h"
#include "uivideo.h"

static const unsigned int romset_dialog_resources[UIROM_TYPE_MAX] = {
    IDD_C64ROM_RESOURCE_DIALOG,
    IDD_C64ROMDRIVE_RESOURCE_DIALOG,
    0 };


static const ui_menu_toggle_t c64dtv_ui_menu_toggles[] = {
    { "EmuID", IDM_TOGGLE_EMUID },
    { "VICIIDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICIIScale2x", IDM_TOGGLE_SCALE2X },
    { "PALEmulation", IDM_TOGGLE_FASTPAL },
    { "VICIIVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "PS2Mouse", IDM_PS2MOUSE },
    { "Mouse", IDM_MOUSE },
    { "FlashTrueFS", IDM_TOGGLE_FLASH_TRUE_FILESYSTEM },
    { NULL, 0 }
};

static const uirom_settings_t uirom_settings[] = {
    { UIROM_TYPE_MAIN, TEXT("Kernal"), "KernalName",
      IDC_C64ROM_KERNAL_FILE, IDC_C64ROM_KERNAL_BROWSE,
      IDC_C64ROM_KERNAL_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Basic"), "BasicName",
      IDC_C64ROM_BASIC_FILE, IDC_C64ROM_BASIC_BROWSE,
      IDC_C64ROM_BASIC_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Character"), "ChargenName",
      IDC_C64ROM_CHARGEN_FILE, IDC_C64ROM_CHARGEN_BROWSE,
      IDC_C64ROM_CHARGEN_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1541"), "DosName1541",
      IDC_DRIVEROM_1541_FILE, IDC_DRIVEROM_1541_BROWSE,
      IDC_DRIVEROM_1541_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1541-II"), "DosName1541ii",
      IDC_DRIVEROM_1541II_FILE, IDC_DRIVEROM_1541II_BROWSE,
      IDC_DRIVEROM_1541II_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1570"), "DosName1570",
      IDC_DRIVEROM_1570_FILE, IDC_DRIVEROM_1570_BROWSE,
      IDC_DRIVEROM_1570_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1571"), "DosName1571",
      IDC_DRIVEROM_1571_FILE, IDC_DRIVEROM_1571_BROWSE,
      IDC_DRIVEROM_1571_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1581"), "DosName1581",
      IDC_DRIVEROM_1581_FILE, IDC_DRIVEROM_1581_BROWSE,
      IDC_DRIVEROM_1581_RESOURCE },
    { 0, NULL, NULL, 0, 0, 0 }
};

#define C64DTVUI_KBD_NUM_MAP 3

static const uikeyboard_mapping_entry_t mapping_entry[C64DTVUI_KBD_NUM_MAP] = {
    { IDC_C64KBD_MAPPING_SELECT_SYM, IDC_C64KBD_MAPPING_SYM,
      IDC_C64KBD_MAPPING_SYM_BROWSE, "KeymapSymFile" },
    { IDC_C64KBD_MAPPING_SELECT_POS, IDC_C64KBD_MAPPING_POS,
      IDC_C64KBD_MAPPING_POS_BROWSE, "KeymapPosFile" },
    { IDC_C64KBD_MAPPING_SELECT_SYMDE, IDC_C64KBD_MAPPING_SYMDE,
      IDC_C64KBD_MAPPING_SYMDE_BROWSE, "KeymapSymDeFile" }
};

static uikeyboard_config_t uikeyboard_config =
    { IDD_C64KBD_MAPPING_SETTINGS_DIALOG, C64DTVUI_KBD_NUM_MAP, mapping_entry,
      IDC_C64KBD_MAPPING_DUMP };

ui_menu_translation_table_t c64dtvui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_EDIT_COPY, IDS_MI_EDIT_COPY },
    { IDM_EDIT_PASTE, IDS_MI_EDIT_PASTE },
    { IDM_AUTOSTART, IDS_MI_AUTOSTART_DISK },
    { IDM_RESET_HARD, IDS_MI_RESET_HARD },
    { IDM_RESET_SOFT, IDS_MI_RESET_SOFT },
    { IDM_RESET_DRIVE8, IDS_MI_DRIVE8 },
    { IDM_RESET_DRIVE9, IDS_MI_DRIVE9 },
    { IDM_RESET_DRIVE10, IDS_MI_DRIVE10 },
    { IDM_RESET_DRIVE11, IDS_MI_DRIVE11 },
    { IDM_ATTACH_8, IDS_MI_DRIVE8 },
    { IDM_ATTACH_9, IDS_MI_DRIVE9 },
    { IDM_ATTACH_10, IDS_MI_DRIVE10 },
    { IDM_ATTACH_11, IDS_MI_DRIVE11 },
    { IDM_DETACH_8, IDS_MI_DRIVE8 },
    { IDM_DETACH_9, IDS_MI_DRIVE9 },
    { IDM_DETACH_10, IDS_MI_DRIVE10 },
    { IDM_DETACH_11, IDS_MI_DRIVE11 },
    { IDM_DETACH_ALL, IDS_MI_DETACH_ALL },
    { IDM_ATTACH_DTV_FLASH, IDS_MI_ATTACH_DTV_FLASH },
    { IDM_DETACH_DTV_FLASH, IDS_MI_DETACH_DTV_FLASH },
    { IDM_CREATE_DTV_FLASH, IDS_MI_CREATE_DTV_FLASH },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_TOGGLE_DOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_DOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDS_MI_DRIVE_TRUE_EMULATION },
    { IDM_TOGGLE_FLASH_TRUE_FILESYSTEM, IDS_MI_FLASH_TRUE_FILESYSTEM },
    { IDM_TOGGLE_VIDEOCACHE, IDS_MI_TOGGLE_VIDEOCACHE },
    { IDM_DRIVE_SETTINGS, IDS_MI_DRIVE_SETTINGS },
    { 0, 0 }
};

ui_popup_translation_table_t c64dtvui_popup_translation_table[] = {
    { 1, IDS_MP_FILE },
    { 2, IDS_MP_ATTACH_DISK_IMAGE },
    { 2, IDS_MP_DETACH_DISK_IMAGE },
    { 2, IDS_MP_DTV_FLASH_ROM_IMAGE },
    { 2, IDS_MP_FLIP_LIST },
    { 2, IDS_MP_RESET },
#ifdef DEBUG
    { 2, IDS_MP_DEBUG },
    { 3, IDS_MP_MODE },
#endif
    { 1, IDS_MP_EDIT },
    { 1, IDS_MP_SNAPSHOT },
    { 2, IDS_MP_RECORDING_START_MODE },
    { 1, IDS_MP_OPTIONS },
    { 2, IDS_MP_REFRESH_RATE },
    { 2, IDS_MP_MAXIMUM_SPEED },
    { 2, IDS_MP_VIDEO_STANDARD },
    { 1, IDS_MP_SETTINGS },
    { 1, IDS_MP_LANGUAGE },
    { 1, IDS_MP_HELP },
    { 0, 0 }
};

static void c64dtv_ui_specific(WPARAM wparam, HWND hwnd)
{
    switch (wparam) {
      case IDM_VICII_SETTINGS:
        ui_vicii_settings_dialog(hwnd);
        break;
      case IDM_SID_SETTINGS:
        ui_sid_settings_dialog(hwnd);
        break;
      case IDM_ROM_SETTINGS:
        uirom_settings_dialog(hwnd, translate_res(IDD_C64ROM_SETTINGS_DIALOG),
                              translate_res(IDD_C64DRIVEROM_SETTINGS_DIALOG),
                              romset_dialog_resources, uirom_settings);
        break;
      case IDM_VIDEO_SETTINGS:
        ui_video_settings_dialog(hwnd, UI_VIDEO_CHIP_VICII, UI_VIDEO_CHIP_NONE);
        break;
      case IDM_DRIVE_SETTINGS:
        uidrivec64_settings_dialog(hwnd);
        break;
      case IDM_RS232USER_SETTINGS:
        ui_rs232user_settings_dialog(hwnd);
        break;
      case IDM_KEYBOARD_SETTINGS:
        uikeyboard_settings_dialog(hwnd, &uikeyboard_config);
        break;
      case IDM_ATTACH_DTV_FLASH:
        ui_c64dtv_attach_flash_dialog(hwnd);
        break;
      case IDM_DETACH_DTV_FLASH:
        resources_set_string("c64dtvromfilename", "");
        break;
      case IDM_CREATE_DTV_FLASH:
        ui_c64dtv_create_flash_dialog(hwnd);
        break;
      case IDM_C64DTV_SETTINGS:
        ui_c64dtv_settings_dialog(hwnd);
        break;
    }
}

int c64dtvui_init(void)
{
    ui_register_machine_specific(c64dtv_ui_specific);
    ui_register_menu_toggles(c64dtv_ui_menu_toggles);
    ui_register_translation_tables(c64dtvui_menu_translation_table, c64dtvui_popup_translation_table);

    return 0;
}

void c64dtvui_shutdown(void)
{
}
