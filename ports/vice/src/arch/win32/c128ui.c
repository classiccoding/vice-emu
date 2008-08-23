/*
 * c128ui.c - C128-specific user interface.
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

#include "c128ui.h"
#include "res.h"
#include "translate.h"
#include "ui.h"
#include "uiacia.h"
#include "uic128.h"
#include "uic64cart.h"
#include "uidigimax.h"
#include "uidrivec128.h"
#include "uiide64.h"
#include "uikeyboard.h"
#include "uimmc64.h"
#include "uireu.h"
#include "uigeoram.h"
#include "uimouse.h"
#include "uiramcart.h"
#include "uirom.h"
#include "uirs232user.h"
#include "uisid.h"
#include "uitfe.h"
#include "uivicii.h"
#include "uivideo.h"
#include "uilib.h"


static const unsigned int romset_dialog_resources[UIROM_TYPE_MAX] = {
    IDD_C128ROM_RESOURCE_DIALOG,
    IDD_C128ROMDRIVE_RESOURCE_DIALOG,
    0 };


static const ui_menu_toggle_t c128_ui_menu_toggles[] = {
    { "EmuID", IDM_TOGGLE_EMUID },
    { "VICIIDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "VICIIScale2x", IDM_TOGGLE_SCALE2X },
    { "PALEmulation", IDM_TOGGLE_FASTPAL },
    { "IEEE488", IDM_IEEE488 },
    { "Mouse", IDM_MOUSE },
    { "CartridgeReset", IDM_TOGGLE_CART_RESET },
    { "VDCDoubleSize", IDM_TOGGLE_VDC_DOUBLESIZE },
    { "VDCDoubleScan", IDM_TOGGLE_VDC_DOUBLESCAN },
    { "VDC64KB", IDM_TOGGLE_VDC64KB },
    { "InternalFunctionROM", IDM_TOGGLE_IFUNCTIONROM },
    { "ExternalFunctionROM", IDM_TOGGLE_EFUNCTIONROM },
    { NULL, 0 }
};

static const uirom_settings_t uirom_settings[] = {
    { UIROM_TYPE_MAIN, TEXT("International Kernal"), "KernalIntName",
      IDC_C128ROM_KERNALINT_FILE, IDC_C128ROM_KERNALINT_BROWSE,
      IDC_C128ROM_KERNALINT_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("German Kernal"), "KernalDEName",
      IDC_C128ROM_KERNALDE_FILE, IDC_C128ROM_KERNALDE_BROWSE,
      IDC_C128ROM_KERNALDE_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Finnish Kernal"), "KernalFIName",
      IDC_C128ROM_KERNALFI_FILE, IDC_C128ROM_KERNALFI_BROWSE,
      IDC_C128ROM_KERNALFI_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("French Kernal"), "KernalFRName",
      IDC_C128ROM_KERNALFR_FILE, IDC_C128ROM_KERNALFR_BROWSE,
      IDC_C128ROM_KERNALFR_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Italian Kernal"), "KernalITName",
      IDC_C128ROM_KERNALIT_FILE, IDC_C128ROM_KERNALIT_BROWSE,
      IDC_C128ROM_KERNALIT_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Norwegian Kernal"), "KernalNOName",
      IDC_C128ROM_KERNALNO_FILE, IDC_C128ROM_KERNALNO_BROWSE,
      IDC_C128ROM_KERNALNO_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Swedish Kernal"), "KernalSEName",
      IDC_C128ROM_KERNALSE_FILE, IDC_C128ROM_KERNALSE_BROWSE,
      IDC_C128ROM_KERNALSE_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Basic LO"), "BasicLoName",
      IDC_C128ROM_BASICLO_FILE, IDC_C128ROM_BASICLO_BROWSE,
      IDC_C128ROM_BASICLO_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Basic HI"), "BasicHiName",
      IDC_C128ROM_BASICHI_FILE, IDC_C128ROM_BASICHI_BROWSE,
      IDC_C128ROM_BASICHI_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("International Character"), "ChargenIntName",
      IDC_C128ROM_CHARGENINT_FILE, IDC_C128ROM_CHARGENINT_BROWSE,
      IDC_C128ROM_CHARGENINT_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("German Character"), "ChargenDEName",
      IDC_C128ROM_CHARGENDE_FILE, IDC_C128ROM_CHARGENDE_BROWSE,
      IDC_C128ROM_CHARGENDE_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("French Character"), "ChargenFRName",
      IDC_C128ROM_CHARGENFR_FILE, IDC_C128ROM_CHARGENFR_BROWSE,
      IDC_C128ROM_CHARGENFR_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("Swedish Character"), "ChargenSEName",
      IDC_C128ROM_CHARGENSE_FILE, IDC_C128ROM_CHARGENSE_BROWSE,
      IDC_C128ROM_CHARGENSE_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("C64 mode Kernal"), "Kernal64Name",
      IDC_C128ROM_KERNAL64_FILE, IDC_C128ROM_KERNAL64_BROWSE,
      IDC_C128ROM_KERNAL64_RESOURCE },
    { UIROM_TYPE_MAIN, TEXT("C64 mode Basic"), "Basic64Name",
      IDC_C128ROM_BASIC64_FILE, IDC_C128ROM_BASIC64_BROWSE,
      IDC_C128ROM_BASIC64_RESOURCE },
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
    { UIROM_TYPE_DRIVE, TEXT("1571CR"), "DosName1571cr",
      IDC_DRIVEROM_1571CR_FILE, IDC_DRIVEROM_1571CR_BROWSE,
      IDC_DRIVEROM_1571CR_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1581"), "DosName1581",
      IDC_DRIVEROM_1581_FILE, IDC_DRIVEROM_1581_BROWSE,
      IDC_DRIVEROM_1581_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("2031"), "DosName2031",
      IDC_DRIVEROM_2031_FILE, IDC_DRIVEROM_2031_BROWSE,
      IDC_DRIVEROM_2031_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("2040"), "DosName2040",
      IDC_DRIVEROM_2040_FILE, IDC_DRIVEROM_2040_BROWSE,
      IDC_DRIVEROM_2040_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("3040"), "DosName3040",
      IDC_DRIVEROM_3040_FILE, IDC_DRIVEROM_3040_BROWSE,
      IDC_DRIVEROM_3040_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("4040"), "DosName4040",
      IDC_DRIVEROM_4040_FILE, IDC_DRIVEROM_4040_BROWSE,
      IDC_DRIVEROM_4040_RESOURCE },
    { UIROM_TYPE_DRIVE, TEXT("1001"), "DosName1001",
      IDC_DRIVEROM_1001_FILE, IDC_DRIVEROM_1001_BROWSE,
      IDC_DRIVEROM_1001_RESOURCE },
    { 0, NULL, NULL, 0, 0, 0 }
};

#define C128UI_KBD_NUM_MAP 2

static const uikeyboard_mapping_entry_t mapping_entry[C128UI_KBD_NUM_MAP] = {
    { IDC_C128KBD_MAPPING_SELECT_SYM, IDC_C128KBD_MAPPING_SYM,
      IDC_C128KBD_MAPPING_SYM_BROWSE, "KeymapSymFile" },
    { IDC_C128KBD_MAPPING_SELECT_POS, IDC_C128KBD_MAPPING_POS,
      IDC_C128KBD_MAPPING_POS_BROWSE, "KeymapPosFile" }
};

static uikeyboard_config_t uikeyboard_config =
    { IDD_C128KBD_MAPPING_SETTINGS_DIALOG, C128UI_KBD_NUM_MAP, mapping_entry,
      IDC_C128KBD_MAPPING_DUMP };


ui_menu_translation_table_t c128ui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_EDIT_COPY, IDS_MI_EDIT_COPY },
    { IDM_EDIT_PASTE, IDS_MI_EDIT_PASTE },
    { IDM_AUTOSTART, IDS_MI_AUTOSTART },
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
    { IDM_ATTACH_TAPE, IDS_MI_ATTACH_TAPE },
    { IDM_DETACH_TAPE, IDS_MI_DETACH_TAPE },
    { IDM_DETACH_ALL, IDS_MI_DETACH_ALL },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_TOGGLE_DOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_VDC_DOUBLESIZE, IDS_MI_TOGGLE_DOUBLESIZE },
    { IDM_TOGGLE_DOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_VDC_DOUBLESCAN, IDS_MI_TOGGLE_DOUBLESCAN },
    { IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDS_MI_DRIVE_TRUE_EMULATION },
    { IDM_TOGGLE_VIDEOCACHE, IDS_MI_TOGGLE_VIDEOCACHE },
    { IDM_DRIVE_SETTINGS, IDS_MI_DRIVE_SETTINGS },
    { IDM_CART_ATTACH_CRT, IDS_MI_CART_ATTACH_CRT },
    { IDM_CART_ATTACH_8KB, IDS_MI_CART_ATTACH_8KB },
    { IDM_CART_ATTACH_16KB, IDS_MI_CART_ATTACH_16KB },
    { IDM_CART_ATTACH_AR, IDS_MI_CART_ATTACH_AR },
    { IDM_CART_ATTACH_AR3, IDS_MI_CART_ATTACH_AR3 },
    { IDM_CART_ATTACH_AR4, IDS_MI_CART_ATTACH_AR4 },
    { IDM_CART_ATTACH_STARDOS, IDS_MI_CART_ATTACH_STARDOS },
    { IDM_CART_ATTACH_AT, IDS_MI_CART_ATTACH_AT },
    { IDM_CART_ATTACH_EPYX, IDS_MI_CART_ATTACH_EPYX },
    { IDM_CART_ATTACH_IEEE488, IDS_MI_CART_ATTACH_IEEE488 },
    { IDM_CART_ATTACH_RR, IDS_MI_CART_ATTACH_RR },
    { IDM_CART_ATTACH_IDE64, IDS_MI_CART_ATTACH_IDE64 },
    { IDM_CART_ATTACH_SS4, IDS_MI_CART_ATTACH_SS4 },
    { IDM_CART_ATTACH_SS5, IDS_MI_CART_ATTACH_SS5 },
    { IDM_CART_ATTACH_STB, IDS_MI_CART_ATTACH_STB },
    { 0, 0 }
};

ui_popup_translation_table_t c128ui_popup_translation_table[] = {
    { 1, IDS_MP_FILE },
    { 2, IDS_MP_ATTACH_DISK_IMAGE },
    { 2, IDS_MP_DETACH_DISK_IMAGE },
    { 2, IDS_MP_FLIP_LIST },
    { 2, IDS_MP_DATASETTE_CONTROL },
    { 2, IDS_MP_ATTACH_CARTRIDGE_IMAGE },
    { 3, IDS_MP_EXPERT_CARTRIDGE },
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
    { 2, IDS_MP_VDC_SETTINGS },
    { 2, IDS_MP_VIDEO_STANDARD },
    { 1, IDS_MP_SETTINGS },
    { 2, IDS_MP_CARTRIDGE_IO_SETTINGS },
    { 1, IDS_MP_LANGUAGE },
    { 1, IDS_MP_HELP },
    { 0, 0 }
};

static void c128_ui_specific(WPARAM wparam, HWND hwnd)
{
    uic64cart_proc(wparam, hwnd);

    switch (wparam) {
      case IDM_VICII_SETTINGS:
        ui_vicii_settings_dialog(hwnd);
        break;
      case IDM_SID_SETTINGS:
        ui_sid_settings_dialog(hwnd);
        break;
      case IDM_REU_SETTINGS:
        ui_reu_settings_dialog(hwnd);
        break;
      case IDM_MMC64_SETTINGS:
        ui_mmc64_settings_dialog(hwnd);
        break;
      case IDM_DIGIMAX_SETTINGS:
        ui_digimax_settings_dialog(hwnd);
        break;
      case IDM_GEORAM_SETTINGS:
        ui_georam_settings_dialog(hwnd);
        break;
      case IDM_RAMCART_SETTINGS:
        ui_ramcart_settings_dialog(hwnd);
        break;
      case IDM_IDE64_SETTINGS:
        uiide64_settings_dialog(hwnd);
        break;
      case IDM_ROM_SETTINGS:
        uirom_settings_dialog(hwnd, translate_res(IDD_C128ROM_SETTINGS_DIALOG),
                              translate_res(IDD_C128DRIVEROM_SETTINGS_DIALOG),
                              romset_dialog_resources, uirom_settings);
        break;
#ifdef HAVE_TFE
      case IDM_TFE_SETTINGS:
        ui_tfe_settings_dialog(hwnd);
        break;
#endif
      case IDM_C128_SETTINGS:
        ui_c128_dialog(hwnd);
        break;
      case IDM_VIDEO_SETTINGS:
        ui_video_settings_dialog(hwnd, UI_VIDEO_CHIP_VICII, UI_VIDEO_CHIP_VDC);
        break;
      case IDM_DRIVE_SETTINGS:
        uidrivec128_settings_dialog(hwnd);
        break;
      case IDM_ACIA_SETTINGS:
        ui_acia_settings_dialog(hwnd, 1, NULL, 1, 1);
        break;
      case IDM_RS232USER_SETTINGS:
        ui_rs232user_settings_dialog(hwnd);
        break;
      case IDM_KEYBOARD_SETTINGS:
        uikeyboard_settings_dialog(hwnd, &uikeyboard_config);
        break;
      case IDM_MOUSE_SETTINGS:
        ui_mouse_settings_dialog(hwnd);
        break;
    }
}

int c128ui_init(void)
{
    uic64cart_init();

    ui_register_machine_specific(c128_ui_specific);
    ui_register_menu_toggles(c128_ui_menu_toggles);
    ui_register_translation_tables(c128ui_menu_translation_table, c128ui_popup_translation_table);

    return 0;
}

void c128ui_shutdown(void)
{
}

