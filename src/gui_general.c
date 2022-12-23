/* gMUDix -- MUDix for X windows
 * Copyright (c) 2002 Marko Boomstra (m.boomstra@chello.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mudix.h"


GtkWidget *gui_get_wid_frame(USER *user, int frame, SELECT_TREE select)
{
    GList  *list;
    int     count = 0;

    /* get a list pointer to the children of the mainbox container */
    list = gtk_container_get_children(
           GTK_CONTAINER(user->gui_pref.g_edit[select]));
    for (; list; list = list->next)
    {
        GtkWidget *widget = list->data;

        /* found a frame - return the widget that is in that frame */
        if (GTK_IS_FRAME(widget))
        {
            if (count++ == frame)
            {
                /* return the widget in the frame */
                return gtk_bin_get_child(GTK_BIN(widget));
            }
        }
    }

    return NULL;
}


GtkWidget *gui_get_child_type(GtkWidget *container, int n, guint type)
{
    GList  *list;
    int     count = 0;

    /* get a list pointer to the children of the widget */
    list = gtk_container_get_children(GTK_CONTAINER(container));
    for (; list; list = list->next)
    {
        GtkWidget *widget = list->data;

        /* found a child - check its type */
        if (GTK_CHECK_TYPE(widget, type))
        {
            if (count++ == n)
            {
                /* return the widget in the frame */
                return widget;
            }
        }
    }

    return NULL;
}


GtkWidget *gui_get_nth_child(GtkWidget *container, int n)
{
    GList *list;
    int    i;

    /* get a list pointer to the children of the container */
    list = gtk_container_get_children(GTK_CONTAINER(container));

    /* return the nth child in the container */
    for (i=0; (i<n && list); i++)
    {
        list = list->next;
    }

    if (list)
    {
        return GTK_WIDGET(list->data);
    }
    else
    {
        return NULL;
    }
}


static void gui_gen_spin_changed(GtkSpinButton *spinbutton, gint *value)
{
    *value = (gint)gtk_spin_button_get_value(spinbutton);
}


static gboolean gui_gen_entry_string(GtkEntry         *entry, 
                                     GdkEventFocus    *event, 
                                     gchar           **string)
{
    G_CONST_RETURN gchar *text;
                   USER  *user;

    /* first lock the mutex - required for site update */
    g_mutex_lock(user_network_mutex);

    text = gtk_entry_get_text(entry);
    
    free(*string);
    *string = strdup(text);

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    /* get the user that owns this pointer (if it was the session) */
    if ((user = get_user_session(*string)))
    {
        gui_set_window_title(user);
    }

    return FALSE;
}


GtkWidget *gui_general_create(USER *user)
{
    GtkWidget *mainbox;
    GtkWidget *entry;
    GtkWidget *spin;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *label;

    mainbox = gtk_vbox_new(FALSE, 0);

    /* create a hbox for an entry */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_gen_entry_string), &user->session);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_INPUT_LEN-1);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Session Name");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    gtk_entry_set_text(GTK_ENTRY(entry), user->session);

    /* create a hbox for an entry */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the entry */
    entry = gtk_entry_new();

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_INPUT_LEN-1);
    gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Filename");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    gtk_entry_set_text(GTK_ENTRY(entry), user->filename);

    /* create a hbox for the entry and spin */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_gen_entry_string), &user->net.site);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_INPUT_LEN-1);

    /* create the spin */
    spin = gtk_spin_button_new_with_range(0, 99999999, 1);

    /* update on value-change */
    g_signal_connect(G_OBJECT(spin), "value-changed",
		     G_CALLBACK(gui_gen_spin_changed), &user->net.port);

    /* put spin in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 2);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spin), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);

    /* put the entry and spin with a frame inside the mainbox */
    frame = gtk_frame_new("Server (Host/IP) - Port");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set values */
    gtk_entry_set_text(GTK_ENTRY(entry), user->net.site);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), user->net.port);

    /* create a hbox for the spin */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the spin */
    spin = gtk_spin_button_new_with_range(0, 9999, 1);

    /* put spin in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 2);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spin), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), user->net.recon_timer_load);

    /* update on value-change */
    g_signal_connect(G_OBJECT(spin), "value-changed",
		     G_CALLBACK(gui_gen_spin_changed), &user->net.recon_timer_load);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Auto-Reconnect");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    label = gtk_label_new("Seconds");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* create a hbox for the spin */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the spin */
    spin = gtk_spin_button_new_with_range(100, 50000000, 1);

    /* put spin in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), spin, FALSE, FALSE, 2);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spin), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), user->gui_user.lines_max);

    /* update on value-change */
    g_signal_connect(G_OBJECT(spin), "value-changed",
		     G_CALLBACK(gui_gen_spin_changed), &user->gui_user.lines_max);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Maximum Buffer Size");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    label = gtk_label_new("Lines");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    return mainbox;
}

