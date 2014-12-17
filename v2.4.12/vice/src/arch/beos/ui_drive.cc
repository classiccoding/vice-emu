/*
 * ui_drive.cc - Drive settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Marcus Sutton <loggedoubt@gmail.com>
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
 
#include <Box.h>
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <TabView.h>
#include <Window.h>

extern "C" { 
#include "drive.h"
#include "resources.h"
#include "ui.h"
#include "ui_drive.h"
#include "vsync.h"
}

static ui_drive_type_t *drive_list;

static int machine_drive_type_count;

static int machine_drive_expansion_count;

static int machine_parallel_cable_type_count;

static struct _extend_image_policy {
    const char *name;
    int id;
} extend_image_policy[] = {
    { "Never extend", DRIVE_EXTEND_NEVER },
    { "Ask on extend", DRIVE_EXTEND_ASK },
    { "Extend on access", DRIVE_EXTEND_ACCESS },
    { NULL, 0 }
};

static struct _expansion {
    const char *name;
    const char *resource_name;
    int (*check_func) (int drive_type);
} expansion[] = {
    { "$2000-$3FFF RAM", "Drive%dRAM2000", drive_check_expansion2000 },
    { "$4000-$5FFF RAM", "Drive%dRAM4000", drive_check_expansion4000 },
    { "$6000-$7FFF RAM", "Drive%dRAM6000", drive_check_expansion6000 },
    { "$8000-$9FFF RAM", "Drive%dRAM8000", drive_check_expansion8000 },
    { "$A000-$BFFF RAM", "Drive%dRAMA000", drive_check_expansionA000 },
    { "Professional DOS", "Drive%dProfDOS", drive_check_profdos },
    { "SuperCard+", "Drive%dSuperCard", drive_check_supercard },
    { NULL, NULL, NULL }
};

static struct _idle_method {
    const char *name;
    int id;
} idle_method[] = {
    { "None", DRIVE_IDLE_NO_IDLE },
    { "Trap idle", DRIVE_IDLE_TRAP_IDLE },
    { "Skip cycles", DRIVE_IDLE_SKIP_CYCLES },
    { NULL, 0 }
};

static struct _parallel_cable {
    const char *name;
    int id;
} parallel_cable[] = {
    { "None", DRIVE_PC_NONE },
    { "Standard", DRIVE_PC_STANDARD },
    { "Dolphin DOS 3", DRIVE_PC_DD3 },
    { "Formel 64", DRIVE_PC_FORMEL64 },
    { NULL, 0 }
};

class DriveView : public BView {
    public:
        DriveView(BRect r, int drive_num);
        void EnableControlsForDriveSettings(int type_index);
    private:
        /* we have to remember some controls to enable/disable them */
        BRadioButton *rb_extendimagepolicy[3];
        BRadioButton *rb_idlemethod[3];
        BRadioButton *rb_parallelcable[4];
        BCheckBox *cb_expansion[7];
};

void DriveView::EnableControlsForDriveSettings(int type_index)
{
    int i;
    int current_drive_type;
    bool expand_is_possible;

    current_drive_type = drive_list[type_index].id;

    for (i = 0; extend_image_policy[i].name; i++) {
        rb_extendimagepolicy[i]->SetEnabled(drive_check_extend_policy(current_drive_type));
    }
    for (i = 0; idle_method[i].name; i++) {
        rb_idlemethod[i]->SetEnabled(drive_check_idle_method(current_drive_type));
    }
    for (i = 0; i < machine_drive_expansion_count; i++) {
        expand_is_possible = expansion[i].check_func(current_drive_type);
        cb_expansion[i]->SetEnabled(expand_is_possible);
    }
    for (i = 0; i < machine_parallel_cable_type_count; i++) {
        rb_parallelcable[i]->SetEnabled(drive_check_parallel_cable(current_drive_type));
    }

}

DriveView::DriveView(BRect r, int drive_num) : BView(r, "drive_view", B_FOLLOW_NONE, B_WILL_DRAW)
{
    BBox *box;
    BRadioButton *radiobutton;
    BCheckBox *checkbox;
    BMessage *msg;

    int i;
    int current_value;
    char resname[32];

    BView::SetViewColor(220, 220, 220, 0);

    /* extend image policy */
    r.OffsetTo(90, 0);
    r.right = 220;
    r.bottom = 95;
    box = new BBox(r);
    box->SetLabel("40 track handling");
    AddChild(box);
    r.InsetBy(10, 10);
    sprintf(resname, "Drive%dExtendImagePolicy", drive_num);
    resources_get_int(resname, &current_value);
    for (i = 0; extend_image_policy[i].name; i++) {
        msg = new BMessage(MESSAGE_DRIVE_EXTENDIMAGEPOLICY);
        msg->AddString("resname", resname);
        msg->AddInt32("resource_index", i);
        radiobutton = new BRadioButton(BRect(10, 20 + i * 25, 120, 30 + i * 25), extend_image_policy[i].name, extend_image_policy[i].name, msg);
        box->AddChild(radiobutton);
        rb_extendimagepolicy[i] = radiobutton;
        if (extend_image_policy[i].id == current_value) {
            radiobutton->SetValue(1);
        }
    }

    /* drive expansion */
    if (machine_drive_expansion_count > 0) {
        r.left = 90;
        r.top = 100;
        r.right = 220;
        r.bottom = r.top + 20 + machine_drive_expansion_count * 25; /* 245 or 295 */
        box = new BBox(r);
        box->SetLabel("Drive expansion");
        AddChild(box);
        for (i = 0; i < machine_drive_expansion_count; i++) {
            sprintf(resname, expansion[i].resource_name, drive_num);
            msg = new BMessage(MESSAGE_DRIVE_EXPANSION);
            msg->AddString("resname", resname);
            checkbox = new BCheckBox(BRect(10, 20 + i * 25, 120, 30 + i * 25), resname, expansion[i].name, msg);
            box->AddChild(checkbox);
            cb_expansion[i] = checkbox;
            resources_get_int(resname, &current_value);
            checkbox->SetValue(current_value);
        }
    }

    /* idle method */
    r.OffsetTo(230, 0);
    r.right = 330;
    r.bottom = 95;
    box = new BBox(r);
    box->SetLabel("Idle method");
    AddChild(box);
    r.InsetBy(10,10);
    sprintf(resname, "Drive%dIdleMethod", drive_num);
    resources_get_int(resname, &current_value);
    for (i = 0; idle_method[i].name; i++) {
        msg = new BMessage(MESSAGE_DRIVE_IDLEMETHOD);
        msg->AddString("resname", resname);
        msg->AddInt32("resource_index", i);
        radiobutton = new BRadioButton(BRect(10, 20 + i * 25, 90, 30 + i * 25), idle_method[i].name, idle_method[i].name, msg);
        box->AddChild(radiobutton);
        rb_idlemethod[i] = radiobutton;
        if (idle_method[i].id == current_value) {
            radiobutton->SetValue(1);
        }
    }

    /* parallel cable */
    if (machine_parallel_cable_type_count > 0) {
        r.left = 230;
        r.top = 100;
        r.right = 330;
        r.bottom = r.top + 20 + machine_parallel_cable_type_count * 25; /* 170 or 220 */
        box = new BBox(r);
        box->SetLabel("Parallel Cable");
        AddChild(box);
        sprintf(resname, "Drive%dParallelCable", drive_num);
        resources_get_int(resname, &current_value);
        for (i = 0; i < machine_parallel_cable_type_count; i++) {
            msg = new BMessage(MESSAGE_DRIVE_PARALLELCABLE);
            msg->AddString("resname", resname);
            msg->AddInt32("resource_index", i);
            radiobutton = new BRadioButton(BRect(10, 20 + i * 25, 90, 30 + i * 25), parallel_cable[i].name, parallel_cable[i].name, msg);
            box->AddChild(radiobutton);
            rb_parallelcable[i] = radiobutton;
            if (parallel_cable[i].id == current_value) {
                radiobutton->SetValue(1);
            }
        }
    }

    /* at last drive type, so we can enable/disable other controls */
    r.OffsetTo(0, 0);
    r.right = 80;
    r.bottom = 20 + machine_drive_type_count * 25; /* max 420 */
    box = new BBox(r);
    box->SetLabel("Drive type");
    AddChild(box);
    r.InsetBy(10, 10);
    sprintf(resname, "Drive%dType", drive_num);
    resources_get_int(resname, &current_value);
    for (i = 0; drive_list[i].name; i++) {
        msg = new BMessage(MESSAGE_DRIVE_TYPE);
        msg->AddString("resname", resname);
        msg->AddInt32("drive_num", drive_num);
        msg->AddInt32("resource_index", i);
        radiobutton = new BRadioButton(BRect(10, 20 + i * 25, 70, 30 + i * 25), drive_list[i].name, drive_list[i].name, msg);
        radiobutton->SetEnabled(drive_check_type(drive_list[i].id, drive_num - 8));
        box->AddChild(radiobutton);

        if (drive_list[i].id == current_value) {
            radiobutton->SetValue(1);
            EnableControlsForDriveSettings(i);
        }
    }
}

class DriveWindow : public BWindow {
    public:
        DriveWindow(BRect r);
        ~DriveWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        DriveView *dv[DRIVE_NUM]; /* pointers to the DriveViews */
};

static DriveWindow *drivewindow = NULL;

DriveWindow::DriveWindow(BRect r)
    : BWindow(r, "Drive settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    BTabView *tabview;
    BTab *tab;
    int i;
    char str[16];

    r = Bounds();

    tabview = new BTabView(r, "tab_view");
    tabview->SetViewColor(220, 220, 220, 0);

    r = tabview->Bounds();
    r.InsetBy(5, 5);
    r.bottom -= tabview->TabHeight();

    for (i = 0; i < DRIVE_NUM; i++) {
        tab = new BTab();
        tabview->AddTab(dv[i] = new DriveView(r, 8 + i), tab);
        sprintf(str, "Drive %d", 8 + i);
        tab->SetLabel(str);
    }

    AddChild(tabview);
    tabview->SetTabWidth(B_WIDTH_FROM_WIDEST);
    Show();
}

DriveWindow::~DriveWindow()
{
    drivewindow = NULL;
}

void DriveWindow::MessageReceived(BMessage *msg)
{
    const char *resname;
    int32 drive_num;
    int32 resource_index;

    msg->FindString("resname", &resname);
    msg->FindInt32("resource_index", &resource_index);

    switch (msg->what) {
        case MESSAGE_DRIVE_TYPE:
            msg->FindInt32("drive_num", &drive_num);
            resources_set_int(resname, drive_list[resource_index].id);
            dv[drive_num - 8]->EnableControlsForDriveSettings(resource_index);
            break;
        case MESSAGE_DRIVE_EXTENDIMAGEPOLICY:
            resources_set_int(resname, extend_image_policy[resource_index].id);
            break;
        case MESSAGE_DRIVE_IDLEMETHOD:
            resources_set_int(resname, idle_method[resource_index].id);
            break;
        case MESSAGE_DRIVE_EXPANSION:
            resources_toggle(resname, NULL);
            break;
        case MESSAGE_DRIVE_PARALLELCABLE:
            resources_set_int(resname, parallel_cable[resource_index].id);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_drive(ui_drive_type_t *drive_types, int caps)
{
    thread_id drivethread;
    status_t exit_value;
    BRect r;
    int i;

    if (drivewindow != NULL) {
        return;
    }

    drive_list = drive_types;
    machine_drive_expansion_count = 0;
    for (i = 0; drive_types[i].name; i++) {
        if (drive_types[i].id >= DRIVE_TYPE_1541 &&
            drive_types[i].id <= DRIVE_TYPE_1571CR &&
            drive_types[i].id != DRIVE_TYPE_1551) {
            machine_drive_expansion_count = (caps & HAS_PROFDOS) ? 7 : 5;
        }
    }
    machine_drive_type_count = i;

    if (caps == HAS_PARA_CABLE) {
        /* This hack deals with Plus4 only having the Standard parallel cable. */
        machine_parallel_cable_type_count = 2;
    } else {
        machine_parallel_cable_type_count = (caps & HAS_PARA_CABLE) ? 4 : 0;
    }

    r.Set(50, 50, 400, 310);
    if (machine_drive_type_count > 8) {
        r.bottom = 110 + machine_drive_type_count * 25;  /* max 510 */
    } else if (machine_drive_expansion_count > 0) {
        r.bottom = 210 + machine_drive_expansion_count * 25;  /* 335 or 385 */
    }

    drivewindow = new DriveWindow(r);

    vsync_suspend_speed_eval();

    /* wait until window closed */
    drivethread = drivewindow->Thread();
    wait_for_thread(drivethread, &exit_value);
}
