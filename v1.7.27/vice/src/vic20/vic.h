/*
 * vic.h - A VIC-I emulation (under construction)
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * 16/24bpp support added by
 *  Steven Tieu <stieu@physics.ubc.ca>
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#ifndef _VIC_H
#define _VIC_H

#include "vice.h"

#include "alarm.h"
#include "log.h"
#include "raster/raster.h"
#include "types.h"
#include "vic-mem.h"
#include "vic20.h"
#include "video.h"

#define VIC_PAL_SCREEN_WIDTH            272
#define VIC_NTSC_SCREEN_WIDTH           208

#define VIC_SCREEN_MAX_TEXT_COLS        31
#define VIC_SCREEN_MAX_TEXT_LINES       32

#define VIC_PAL_DISPLAY_WIDTH           224
#define VIC_NTSC_DISPLAY_WIDTH          200

#define VIC_NUM_SPRITES 0

#define VIC_NUM_COLORS 16

/* On MS-DOS, do not duplicate pixels.  Otherwise, we would always need at
   least 466 horizontal pixels to contain the whole screen.  */
/* But this is no problem as 320*200 does not fit anyhow.  */
#ifndef __OS2__
#define VIC_DUPLICATES_PIXELS
#endif

#ifdef VIC_DUPLICATES_PIXELS
typedef PIXEL2 VIC_PIXEL;
#define VIC_PIXEL(n)    RASTER_PIXEL2 (&vic.raster, n)
typedef PIXEL4 VIC_PIXEL2;
#define VIC_PIXEL2(n)   RASTER_PIXEL4 (&vic.raster, n)
#define VIC_PIXEL_WIDTH 2
#else
typedef PIXEL VIC_PIXEL;
#define VIC_PIXEL(n)    RASTER_PIXEL (&vic.raster, n)
typedef PIXEL2 VIC_PIXEL2;
#define VIC_PIXEL2(n)   RASTER_PIXEL2 (&vic.raster, n)
#define VIC_PIXEL_WIDTH 1
#endif

/* Cycle # within the current line.  */
#define VIC_RASTER_CYCLE(clk) ((unsigned int)((clk) % vic.cycles_per_line))

/* Current vertical position of the raster.  Unlike `rasterline', which is
   only accurate if a pending `A_RASTERDRAW' event has been served, this is
   guarranteed to be always correct.  It is a bit slow, though.  */
#define VIC_RASTER_Y(clk)     ((unsigned int)((clk) / vic.cycles_per_line)   \
                               % vic.screen_height)

#define VIC_RASTER_X(cycle)      (((int)(cycle) - 7) * 4)

#define VIC_RASTER_CHAR(cycle)   ((int)((cycle) \
                                 - vic.raster.display_xstart / 4 - 6) / 2)

/* Video mode definitions. */

enum vic_video_mode_s
{
    VIC_STANDARD_MODE,
    VIC_REVERSE_MODE,
    VIC_NUM_VMODES
};
typedef enum vic_video_mode_s vic_video_mode_t;

#define VIC_IDLE_MODE VIC_STANDARD_MODE

/* On MS-DOS, we do not need 2x drawing functions.  This is mainly to save
   memory and (little) speed.  */
#if !defined(__MSDOS__) && !defined(__riscos) && !defined(__OS2__)
#define VIC_NEED_2X 1
#endif

struct snapshot_s;
struct screenshot_s;
struct palette_s;

struct vic_s
{
    int initialized;

    log_t log;

    raster_t raster;

    struct palette_s *palette;

    BYTE regs[64];

    alarm_t raster_draw_alarm;

    int auxiliary_color;
    int mc_border_color;

    BYTE *color_ptr;
    BYTE *screen_ptr;
    BYTE *chargen_ptr; /* = chargen_rom + 0x400; */

    unsigned int char_height;   /* changes immediately for memory fetch */
    unsigned int row_increase_line; /* may change next line for row count */
    unsigned int text_cols;     /* = 22 */
    unsigned int text_lines;    /* = 23 */

    unsigned int memptr;

    /* next frame with different ystart; -1 for no value pending */
    int pending_ystart;

    /* next frame with different number of text lines;
       -1 for no value pending */
    int pending_text_lines;

    /* counting the text lines in the current frame */
    int row_counter;

    /* offset for screen memory pointer if char_height changes from 8 to 16  */
    int row_offset;

    /* area in the frame: 0=upper border, 1=visible screen; 2=lower border */
    int area;

    /* Clock cycle for the next "raster draw" alarm.  */
    CLOCK draw_clk;

    /* FIXME: Bad name.  FIXME: Has to be initialized.  */
    CLOCK last_emulate_line_clk;

    unsigned int cycles_per_line;
    unsigned int screen_height;
    unsigned int first_displayed_line;
    unsigned int last_displayed_line;
    unsigned int screen_width;
    unsigned int display_width;
    unsigned int cycle_offset;
};
typedef struct vic_s vic_t;

extern vic_t vic;

extern struct raster_s *vic_init(void);
extern struct video_canvas_s *vic_get_canvas(void);
extern void vic_reset(void);
extern void vic_raster_draw_alarm_handler(CLOCK offset);

extern int vic_resources_init(void);
extern int vic_cmdline_options_init(void);

extern int vic_write_snapshot_module(struct snapshot_s *s);
extern int vic_read_snapshot_module(struct snapshot_s *s);

extern int vic_screenshot(struct screenshot_s *screenshot);
extern void vic_free(void);

/* Private function calls, used by the other VIC modules.  FIXME:
   Prepend names with `_'?  */
extern void vic_update_memory_ptrs(void);
extern void vic_resize(void);

extern void vic_video_refresh(void);
/* Debugging options.  */

/* #define VIC_RASTER_DEBUG */
/* #define VIC_REGISTERS_DEBUG */

#ifdef VIC_RASTER_DEBUG
#define VIC_DEBUG_RASTER(x) log_debug x
#else
#define VIC_DEBUG_RASTER(x)
#endif

#ifdef VIC_REGISTERS_DEBUG
#define VIC_DEBUG_REGISTER(x) log_debug x
#else
#define VIC_DEBUG_REGISTER(x)
#endif

#endif /* _VIC_H */

