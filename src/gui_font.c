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


/* for frame label */
char *font_name_table[NR_FONTS] =
{
    "Main Window Font",
    "Scrollback Font",
    "Input Font",
    "Capture Font"
};


static void gui_font_button_release(GtkButton *button, USER *user)
{
    GtkWidget        *dialog;
    GtkWidget        *hbox;
    GtkWidget        *entry = NULL;
    gint              response;
    int               i = 0;

    /* create a font selector dialog */
    dialog = gtk_font_selection_dialog_new("Select a font");

    do
    {
        /* retrieve the frame */
        if ((hbox = gui_get_nth_child(user->gui_pref.g_edit[SEL_FONTS], i)))
        {
            /* get the child of the frame */
            hbox = gtk_bin_get_child(GTK_BIN(hbox));
            /* then get the button and compare it */
            if (GTK_WIDGET(button) == gui_get_nth_child(hbox, 1))
            {
                /* yes, found the button that caused this signal */
                entry = gui_get_nth_child(hbox, 0);
                break;
            }
        }
        i++;
    }
    while (hbox != NULL);

    if (entry)
    {
        gtk_font_selection_dialog_set_font_name(
            GTK_FONT_SELECTION_DIALOG(dialog), user->gui_user.fonts[i]);
        gtk_font_selection_dialog_set_preview_text(
            GTK_FONT_SELECTION_DIALOG(dialog), "Select a new font.");

        /* wait for a response */
        response = gtk_dialog_run(GTK_DIALOG(dialog));

        if (response == GTK_RESPONSE_OK)
        {
            /* free before allocating new pointer */
            user->gui_user.fonts[i] = str_dup(user->gui_user.fonts[i],
                                      gtk_font_selection_dialog_get_font_name(
                                      GTK_FONT_SELECTION_DIALOG(dialog)));

            gui_font_update(user, i);

            /* grab the new font and set the widget to the new font */
            gtk_entry_set_text(GTK_ENTRY(entry), user->gui_user.fonts[i]);
        }
    }

    gtk_widget_destroy(dialog);

    return;
}


static void gui_font_above_changed(GtkSpinButton *spinbutton, USER *user)
{
    user->gui_user.space_above = (gint)gtk_spin_button_get_value(spinbutton);

    gui_font_spacing_update(user);
}


static void gui_font_below_changed(GtkSpinButton *spinbutton, USER *user)
{
    user->gui_user.space_below = (gint)gtk_spin_button_get_value(spinbutton);

    gui_font_spacing_update(user);
}


void gui_font_update(USER *user, DEF_FONTS index)
{
    PangoFontDescription *font_desc;

    if (!user->gui_user.g_window)
    {
        return;
    }

    /* change default font throughout the widget */
    font_desc = pango_font_description_from_string(user->gui_user.fonts[index]);
    switch (index)
    {
        case FONT_USER_WINDOW:
            gtk_widget_modify_font(user->gui_user.g_view, font_desc);

            /* font got updated, send NAWS if required */
            send_naws(user);
            break;
        case FONT_USER_SCROLLBACK:
            gtk_widget_modify_font(get_sb_view(user), font_desc);
            break;
        case FONT_USER_INPUT:
            gtk_widget_modify_font(user->gui_user.g_input, font_desc);
            break;
        default:
            break;
    }

    pango_font_description_free(font_desc);
}


void gui_font_spacing_update(USER *user)
{
    if (user->gui_user.g_scrollback)
    {
        gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(get_sb_view(user)),
                                             user->gui_user.space_above);
        gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(get_sb_view(user)),
                                             user->gui_user.space_below);
    }

    if (user->gui_user.g_view)
    {
        gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(user->gui_user.g_view),
                                             user->gui_user.space_above);
        gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(user->gui_user.g_view),
                                             user->gui_user.space_below);

        /* scroll to end of the buffer */
        gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(user->gui_user.g_view),
                                           user->gui_user.g_mark);
    }
}


GtkWidget *gui_font_create(USER *user)
{
    GtkWidget   *mainbox;
    GtkWidget   *frame;
    GtkWidget   *hbox;
    GtkWidget   *bbox;
    GtkWidget   *above;
    GtkWidget   *below;
    GtkWidget   *entry;
    GtkWidget   *button;
    GtkWidget   *label;
    DEF_FONTS    i;

    /* create a vertical box where the complete editor is inside */
    mainbox = gtk_vbox_new(FALSE, 0);

    for (i=0; i<NR_FONTS; i++)
    {
        /* create a hbox for the window font */
        hbox = gtk_hbox_new(FALSE, 0);

        /* create the entry */
        entry = gtk_entry_new();

        /* put entry in hbox */
        gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

        /* put the entry with a frame inside the mainbox */
        frame = gtk_frame_new(font_name_table[i]);
        gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
        gtk_container_add(GTK_CONTAINER(frame), hbox);
        gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

        /* entry not editable */
        gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);

        /* entry not focusable */
        g_object_set(entry, "can-focus", FALSE, NULL);

        /* set the current font */
        gtk_entry_set_text(GTK_ENTRY(entry), user->gui_user.fonts[i]);

        /* create a selector button */
        button = gtk_button_new_with_label("_Change");
        gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
        /* activate is when the mnemonic key is pressed. */
        g_signal_connect(G_OBJECT(button), "activate",
                         G_CALLBACK(gui_font_button_release), user);
        g_signal_connect(G_OBJECT(button), "released",
                         G_CALLBACK(gui_font_button_release), user);
        /* put the button inside the horizontal box */
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 4);
    }

    /* create a hbox for both the timer and the reload spin */
    bbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 0);

    /* create a hbox for the timer */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the above spin */
    above = gtk_spin_button_new_with_range(-100, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(above), user->gui_user.space_above);

    /* put timer in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), above, FALSE, FALSE, 0);

    /* update on value-change */
    g_signal_connect(G_OBJECT(above), "value-changed",
		     G_CALLBACK(gui_font_above_changed), user);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Spacing above lines");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(bbox), frame, TRUE, TRUE, 0);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(above), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(above), TRUE);

    label = gtk_label_new("Pixels");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* create a hbox for the below */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the reload spin */
    below = gtk_spin_button_new_with_range(-100, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(below), user->gui_user.space_below);

    /* put reload in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), below, FALSE, FALSE, 0);

    /* update on value-change */
    g_signal_connect(G_OBJECT(below), "value-changed",
		     G_CALLBACK(gui_font_below_changed), user);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Spacing below lines");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(bbox), frame, TRUE, TRUE, 0);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(below), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(below), TRUE);

    label = gtk_label_new("Pixels");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    return mainbox;
}
