/** \file   src/arch/gtk3/widgets/georamwidget.c
 * \brief   Widget to control GEO-RAM resources
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  GEORAM
 *  GEORAMsize
 *  GEORAMfilename
 *  GEORAMImageWrite
 *  GOERAMIOSwap (xvic)
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
#include <gtk/gtk.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "cartimagewidget.h"
#include "carthelpers.h"
#include "cartridge.h"

#include "georamwidget.h"

/** \brief  List of supported RAM sizes
 */
static ui_radiogroup_entry_t ram_sizes[] = {
    { "64KB", 64 },
    { "128KB", 128 },
    { "256KB", 256 },
    { "512KB", 512 },
    { "1MB", 1024 },
    { "2MB", 2048 },
    { "4MB", 4096 },
    { NULL, -1 }
};


/* list of widgets, used to enable/disable depending on GEORAM resource */
static GtkWidget *georam_enable_widget = NULL;  /* georam_enable lives in
                                                   georam.c */
static GtkWidget *georam_size = NULL;
static GtkWidget *georam_ioswap = NULL;
static GtkWidget *georam_image = NULL;


/** \brief  Handler for the "toggled" event of the georam_enable widget
 *
 * \param[in]   widget      check button
 * \param[in]   user_data   unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    gboolean state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(georam_size, state);
    if (georam_ioswap != NULL) {
        gtk_widget_set_sensitive(georam_ioswap, state);
    }
    gtk_widget_set_sensitive(georam_image, state);
}


/** \brief  Create GEORAM enable check button
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_georam_enable_widget(void)
{
    GtkWidget *check;

    check = resource_check_button_create("GEORAM", "Enable GEO-RAM");
    return check;
}


/** \brief  Create IO-swap check button (seems to be valid for xvic only)
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_georam_ioswap_widget(void)
{
    GtkWidget *check;

    check = resource_check_button_create("GEORAMIOSwap", "MasC=uarade I/O swap");
    return check;
}


/** \brief  Create radio button group to determine GEORAM RAM size
 *
 * \return  GtkGrid
 */
static GtkWidget *create_georam_size_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("RAM Size", 1);
    radio_group = resource_radiogroup_create("GEORAMsize", ram_sizes,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to load/save GEORAM image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_georam_image_widget(GtkWidget *parent)
{
    return cart_image_widget_create(parent, "GEORAM Image",
            "GEORAMfilename", "GEORAMImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_GEORAM, CARTRIDGE_GEORAM);
}


/** \brief  Create widget to control GEORAM resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *georam_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    georam_enable_widget = create_georam_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), georam_enable_widget, 0, 0, 1, 1);

    if (machine_class == VICE_MACHINE_VIC20) {
        georam_ioswap = create_georam_ioswap_widget();
        gtk_grid_attach(GTK_GRID(grid), georam_ioswap, 0, 2, 1, 1);
    }

    georam_size = create_georam_size_widget();
    gtk_grid_attach(GTK_GRID(grid), georam_size, 0, 1, 1, 1);

    georam_image = create_georam_image_widget(parent);
    gtk_grid_attach(GTK_GRID(grid), georam_image, 1, 1, 1, 1);

    g_signal_connect(georam_enable_widget, "toggled", G_CALLBACK(on_enable_toggled),
            NULL);

    /* enable/disable widget based on georam-enable (dirty trick, I know) */
    on_enable_toggled(georam_enable_widget, NULL);

    gtk_widget_show_all(grid);
    return grid;
}
