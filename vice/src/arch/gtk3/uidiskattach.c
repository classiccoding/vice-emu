/*
 * uidiskattach.c - GTK3 disk-attach dialog
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "attach.h"
#include "autostart.h"
#include "tape.h"
#include "debug_gtk3.h"
#include "contentpreviewwidget.h"
#include "filechooserhelpers.h"
#include "driveunitwidget.h"
#include "ui.h"

#include "uidiskattach.h"


/** \brief  Custom response ID's for the dialog
 *
 * Negative values are reserved by Gtk to handle stock responses
 */
enum {
    RESPONSE_AUTOSTART = 1  /**< Autostart button clicked */
};


/** \brief  File type filters for the dialog
 */
static ui_file_filter_t filters[] = {
    { "All files", file_chooser_pattern_all },
    { "Disk images", file_chooser_pattern_disk },
    { "Compressed files", file_chooser_pattern_compressed },
    { NULL, NULL }
};



/** \brief  Unit number to attach disk to
 */
static int unit_number = 8;



/** \brief  Handler for the 'toggled' event of the 'show hidden files' checkbox
 *
 * \param[in]   widget      checkbox triggering the event
 * \param[in]   user_data   data for the event (the dialog)
 */
static void on_hidden_toggled(GtkWidget *widget, gpointer user_data)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    debug_gtk3("show hidden files: %s\n", state ? "enabled" : "disabled");

    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(user_data), state);
}



/** \brief  Handler for the 'toggled' event of the 'show preview' checkbox
 *
 * \param[in]   widget      checkbox triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void on_preview_toggled(GtkWidget *widget, gpointer user_data)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    debug_gtk3("preview %s\n", state ? "enabled" : "disabled");
}


/** \brief  Handler for 'response' event of the dialog
 *
 * This handler is called when the user clicks a button in the dialog.
 *
 * \param[in]   widget      the dialog
 * \param[in]   response_id response ID
 * \param[in]   user_data   extra data (unused)
 *
 * TODO:    proper (error) messages, which requires implementing ui_error() and
 *          ui_message() and moving them into gtk3/widgets to avoid circular
 *          references
 */
static void on_response(GtkWidget *widget, gint response_id, gpointer user_data)
{
    gchar *filename;

    debug_gtk3("got response ID %d\n", response_id);

    switch (response_id) {

        /* 'Open' button, double-click on file */
        case GTK_RESPONSE_ACCEPT:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            /* ui_message("Opening file '%s' ...", filename); */
            debug_gtk3("Attaching file '%s' to unit #%d\n",
                    filename, unit_number);

            /* copied from Gtk2: I fail to see how brute-forcing your way
             * through file types is 'smart', but hell, it works */
            if (file_system_attach_disk(unit_number, filename) < 0) {
                /* failed */
                debug_gtk3("disk attach failed\n");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Autostart' button clicked */
        case RESPONSE_AUTOSTART:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            debug_gtk3("Autostarting file '%s'\n", filename);
            /* if this function exists, why is there no attach_autodetect()
             * or something similar? -- compyx */
            if (autostart_disk(
                        filename,
                        NULL,   /* program name */
                        0,      /* Program number? Probably used when clicking
                                   in the preview widget to load the proper
                                   file in an image */
                        AUTOSTART_MODE_RUN) < 0) {
                /* oeps */
                debug_gtk3("autostart disk attach failed\n");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Close'/'X' button */
        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            break;
    }
}


/** \brief  Create the 'extra' widget
 *
 * \return  GtkGrid
 *
 * TODO: 'grey-out'/disable units without a proper drive attached
 */
static GtkWidget *create_extra_widget(GtkWidget *parent, int unit)
{
    GtkWidget *grid;
    GtkWidget *hidden_check;
    GtkWidget *readonly_check;
    GtkWidget *preview_check;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    hidden_check = gtk_check_button_new_with_label("Show hidden files");
    g_signal_connect(hidden_check, "toggled", G_CALLBACK(on_hidden_toggled),
            (gpointer)(parent));
    gtk_grid_attach(GTK_GRID(grid), hidden_check, 0, 0, 1, 1);

    readonly_check = gtk_check_button_new_with_label("Attach read-only");
    gtk_grid_attach(GTK_GRID(grid), readonly_check, 1, 0, 1, 1);

    preview_check = gtk_check_button_new_with_label("Show image contents");
    g_signal_connect(preview_check, "toggled", G_CALLBACK(on_preview_toggled),
            NULL);
    gtk_grid_attach(GTK_GRID(grid), preview_check, 2, 0, 1, 1);

    /* second row, three cols wide */
    gtk_grid_attach(GTK_GRID(grid), drive_unit_widget_create(unit, &unit_number,
                NULL),
            0, 1, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the disk attach dialog
 *
 * \param[in]   parent  parent widget, used to get the top level window
 *
 * \return  GtkFileChooserDialog
 */
static GtkWidget *create_disk_attach_dialog(GtkWidget *parent, int unit)
{
    GtkWidget *dialog;
    GtkWidget *preview;
    size_t i;

    /* create new dialog */
    dialog = gtk_file_chooser_dialog_new(
            "Attach a disk image",
            ui_get_active_window(),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            /* buttons */
            "Open", GTK_RESPONSE_ACCEPT,
            "Autostart", RESPONSE_AUTOSTART,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    /* add 'extra' widget: 'readony' and 'show preview' checkboxes */
    if (unit < 8 || unit > 11) {
        unit = 8;
    }
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
                                      create_extra_widget(dialog, unit));

    preview = content_preview_widget_create(NULL);
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);

    /* add filters */
    for (i = 0; filters[i].name != NULL; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                create_file_chooser_filter(filters[i], TRUE));
    }

    /* connect "reponse" handler: the `user_data` argument gets filled in when
     * the "response" signal is emitted: a response ID */
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    return dialog;

}


/** \brief  Callback for the "smart-attach" and "attach to #%d" menu items
 *
 * Creates the smart-dialog and runs it.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   integer from 8-11 for the default drive to attach to
 */
void ui_disk_attach_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    debug_gtk3("called\n");
    dialog = create_disk_attach_dialog(widget, GPOINTER_TO_INT(user_data));
    gtk_widget_show(dialog);

}

/** \brief  Callback for "detach from #%d" menu items
 *
 * Removes any disk from the specified drive. No additional UI is
 * presented.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   integer from 8-11 for the drive to
 *                          close, or -1 to detach all disks
 */
void ui_disk_detach_callback(GtkWidget *widget, gpointer user_data)
{
    /* This function does its own interpretation and input validation,
     * so we can simply forward the call directly. */
    file_system_detach_disk(GPOINTER_TO_INT(user_data));
}
