/*
 * ui.h 
 *
 * Written by
 *  Martin Pottendorfer (Martin.Pottendorfer@alcatel.at)
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

#ifndef _UI_H_
#define _UI_H_

#include "vice.h"

#include "types.h"
#include "uiapi.h"
#ifdef USE_GNOMEUI
#include "x11/gnome/uiarch.h"
#else
#include "x11/xaw/uiarch.h"
#endif

/* If this is #defined, `Alt' is handled the same as `Meta'.  On
   systems which have Meta, it's better to use Meta instead of Alt as
   a shortcut modifier (because Alt is usually used by Window
   Managers), but systems that don't have Meta (eg. GNU/Linux, HP-UX)
   would suffer then.  So it's easier to just handle Meta as Alt in
   such cases.  */
#define ALT_AS_META

/* Number of drives we support in the UI.  */
#define NUM_DRIVES      2

typedef void (*ui_exposure_handler_t) (unsigned int width,
                                       unsigned int height);
typedef enum {
    UI_BUTTON_NONE, UI_BUTTON_CLOSE, UI_BUTTON_OK, UI_BUTTON_CANCEL,
    UI_BUTTON_YES, UI_BUTTON_NO, UI_BUTTON_RESET, UI_BUTTON_HARDRESET,
    UI_BUTTON_MON, UI_BUTTON_DEBUG, UI_BUTTON_CONTENTS, UI_BUTTON_AUTOSTART
} ui_button_t;

/* ------------------------------------------------------------------------- */
/* Prototypes */

struct canvas_s;
struct palette_s;

extern int ui_proc_read_msg(char* msg, size_t size, int block);
extern ui_window_t ui_open_canvas_window(struct canvas_s *c,
                                         const char *title, int width,
                                         int height, int no_autorepeat,
                                         ui_exposure_handler_t exposure_proc,
                                         const struct palette_s *p,
                                         PIXEL pixel_return[]);
extern void ui_resize_canvas_window(ui_window_t w, int height, int width);
extern void ui_map_canvas_window(ui_window_t w);
extern void ui_unmap_canvas_window(ui_window_t w);
extern int ui_canvas_set_palette(struct canvas_s *c, ui_window_t w,
                                 const struct palette_s *palette,
                                 PIXEL *pixel_return);
void ui_display_speed(float percent, float framerate, int warp_flag);
void ui_display_paused(int flag);
void ui_dispatch_next_event(void);
void ui_dispatch_events(void);
extern void ui_exit(void);
extern void ui_message(const char *format,...);
extern void ui_show_text(const char *title, const char *text, int width,
                         int height);
extern char *ui_select_file(const char *title,
                            char *(*read_contents_func)(const char *),
                            unsigned int allow_autostart,
                            const char *default_dir,
                            const char *default_pattern,
                            ui_button_t *button_return,
			    unsigned int show_preview);
extern ui_button_t ui_input_string(const char *title, const char *prompt,
                                   char *buf, unsigned int buflen);
extern ui_button_t ui_ask_confirmation(const char *title, const char *text);
extern void ui_autorepeat_on(void);
extern void ui_autorepeat_off(void);
extern void ui_pause_emulation(int flag);
extern int ui_emulation_is_paused(void);
extern void ui_create_dynamic_menues(void);
extern void ui_check_mouse_cursor(void);
extern void ui_update_flip_menus(int from_unit, int to_unit);

extern void archdep_ui_init(int argc, char *argv[]);
extern void ui_set_application_icon(const char *icon_data[]);
extern void ui_display_tape_current_image(char *image);
extern void ui_proc_start(void);
extern void ui_destroy_widget(ui_window_t w);
extern void ui_set_topmenu(void);
extern void ui_set_selected_file(int num);
extern int ui_get_display_depth(void);
extern Display *ui_get_display_ptr(void);

extern void ui_destroy_drive8_menu(void);
extern void ui_destroy_drive9_menu(void);

#endif /* !_UI_H_ */
