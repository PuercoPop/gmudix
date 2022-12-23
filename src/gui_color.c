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


#define BUTTON_DEFAULT  "_Set Defaults"
#define BUTTON_EXIT     "E_xit"


static char *default_color_name[NR_ANSI_COLORS] =
{
    "- Black",
    "- Red",
    "- Green",
    "- Brown",
    "- Blue",
    "- Magenta",
    "- Cyan",
    "- Grey",
    "- Black (Bold)",
    "- Red (Bold)",
    "- Green (Bold)",
    "- Yellow",
    "- Blue (Bold)",
    "- Magenta (Bold)",
    "- Cyan (Bold)",
    "- White"
};


static GdkColor default_color_table[NR_ANSI_COLORS] =
{
    /* pixel    R           G           B */
    {0,         0x0,        0x0,        0x0         },   /* black */
    {0,         0x8B8B,     0x0,        0x0         },   /* red */
    {0,         0x0,        0x6464,     0x0         },   /* green */
    {0,         0x8B8B,     0x6969,     0x1414      },   /* brown */
    {0,         0x0,        0x0,        0x8B8B      },   /* blue */
    {0,         0x8B8B,     0x0,        0x8B8B      },   /* magenta */
    {0,         0x0,        0x8B8B,     0x8B8B      },   /* cyan */
    {0,         0xBEBE,     0xBEBE,     0xBEBE      },   /* grey */

    /* below are the bold colors of above */
    {0,         0x5F5F,     0x5F5F,     0x5F5F      },   /* bold black */
    {0,         0xFFFF,     0x0,        0x0         },   /* bold red */
    {0,         0x0,        0xFFFF,     0x0         },   /* bold green */
    {0,         0xFFFF,     0xFFFF,     0x0         },   /* yellow */
    {0,         0x0,        0x0,        0xFFFF      },   /* bold blue */
    {0,         0xFFFF,     0x0,        0xFFFF      },   /* bold magenta */
    {0,         0x0,        0xFFFF,     0xFFFF      },   /* bold cyan */
    {0,         0xFFFF,     0xFFFF,     0xFFFF      }    /* white */
};


static gboolean gui_color_button(GtkWidget      *widget, 
                                 GdkEventButton *event, 
                                 GdkColor       *color)
{
    GtkWidget         *dialog;
    GtkColorSelection *colorsel;
    gint               response;
    USER              *user;
    ANSI_COLORS        i;
    
    /* fetch the user that owns this color! ;) */
    user = get_user_with_color(color);
    if (!user)
    {
        fprintf(stderr, "No user found that owns color: 0x%0x\n", (gint)color);
        return FALSE;
    }

    /* create the color selection dialog */
    dialog = gtk_color_selection_dialog_new("Changing color");

    /* pop up in centre of parent */
    gtk_window_set_transient_for(GTK_WINDOW(dialog), 
                                 GTK_WINDOW(user->gui_pref.g_preference));
  
    colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);

    /* give pointers to the color to the colorselection */
    gtk_color_selection_set_previous_color(colorsel, color);
    gtk_color_selection_set_current_color(colorsel, color);
    gtk_color_selection_set_has_palette(colorsel, TRUE);
  
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
        /* grab the new color and set the widget to the new color */
        gtk_color_selection_get_current_color(colorsel, color);
        gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, color);
    }
  
    gtk_widget_destroy(dialog);

    /* is it a default fg/bg color? */
    if (color == &user->gui_user.default_color[DEF_FG] ||
        color == &user->gui_user.default_color[DEF_BG])
    {
        if (user->gui_user.g_window)
        {
            gui_color_def_widget(user, user->gui_user.g_view);
            gui_color_def_widget(user, get_sb_view(user));
        }
    }
    else
    {
        /* no then it's an tag color, look up which one it was */
        for (i=0; i<NR_ALL_COLORS; i++)
        {
            if (&user->gui_user.colors[i] == color)
            {
                /* we now got the index - use it for the tag table :) */
                g_object_set(user->gui_user.g_fg_color_tags[i], "foreground-gdk", 
                             &user->gui_user.colors[i], NULL);
                g_object_set(user->gui_user.g_bg_color_tags[i], "background-gdk", 
                             &user->gui_user.colors[i], NULL);
                break;
            }
        }
    }

    return TRUE;
}


static void gui_color_button_release(GtkButton *button, USER *user)
{
    G_CONST_RETURN gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_DEFAULT))
    {
        ANSI_COLORS  i;

        /* set all colors to default */
        gui_color_set_def_all(user);

        /* set the fg and bg of both the window and scrollback */
        if (user->gui_user.g_window)
        {
            gui_color_def_widget(user, user->gui_user.g_view);
            gui_color_def_widget(user, get_sb_view(user));
        }

        /* set the tags */
        for (i=0; i<NR_ALL_COLORS; i++)
        {
            /* we now got the index - use it for the tag table :) */
            g_object_set(user->gui_user.g_fg_color_tags[i], "foreground-gdk", 
                         &user->gui_user.colors[i], NULL);
            g_object_set(user->gui_user.g_bg_color_tags[i], "background-gdk", 
                         &user->gui_user.colors[i], NULL);
        }

        /* destroy the preference window, this is an EASY work around :) */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the trigger view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


void gui_color_def_widget(USER *user, GtkWidget *widget)
{
    /* change default color throughout the widget */
    gtk_widget_modify_base(widget, GTK_STATE_NORMAL, 
                           &user->gui_user.default_color[DEF_BG]);
    gtk_widget_modify_text(widget, GTK_STATE_NORMAL, 
                           &user->gui_user.default_color[DEF_FG]);
}


void gui_color_set_def(GdkColor *color, ANSI_COLORS index)
{
    /* grab the color from the table and return it */
    *color = default_color_table[index];
}


void gui_color_set_def_all(USER *user)
{
    ANSI_COLORS i;

    /* setup default background & foreground color */
    gui_color_set_def(&user->gui_user.default_color[DEF_BG], COLOR_BLACK);
    gui_color_set_def(&user->gui_user.default_color[DEF_FG], COLOR_GREY);

    /* setup default echo and info color */
    gui_color_set_def(&user->gui_user.colors[COLOR_INFO], COLOR_YELLOW);
    gui_color_set_def(&user->gui_user.colors[COLOR_ECHO], COLOR_GREEN);

    /* sets all ansi colors of the user to default */
    for (i=0; i<NR_ANSI_COLORS; i++)
    {
        user->gui_user.colors[i] = default_color_table[i];
    }
}


GtkWidget *gui_color_create(USER *user)
{
    GtkWidget       *mainbox;
    GtkWidget       *label;
    GtkWidget       *frame;
    GtkWidget       *color_frame;
    GtkWidget       *draw;
    GtkWidget       *table;
    GtkWidget       *hbox;
    GtkWidget       *bbox;
    GtkWidget       *button;
    ANSI_COLORS      i;

    /* create a vertical box where the complete editor is inside */
    mainbox = gtk_vbox_new(FALSE, 0);

    frame = gtk_frame_new("ANSI Color Mapping");
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 2);

    table = gtk_table_new(1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    /* create rows with color squares (clickable) 2 on each row */
    for (i=0; i<(NR_ANSI_COLORS/2); i++)
    {
        /* set up the left part of a row */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, i*2, (i*2)+1);
                                
        color_frame = gtk_frame_new(NULL);
        gtk_widget_set_size_request(color_frame, 20, 20);

        gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 2);

        label = gtk_label_new(default_color_name[i]);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

        /* a small drawing area with the background set to the color */
        draw = gtk_drawing_area_new();
        gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(G_OBJECT(draw), "button-press-event",
                         G_CALLBACK(gui_color_button), 
                         &user->gui_user.colors[i]);

        /* set the color */
        gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                             &user->gui_user.colors[i]);

        /* put the draw area in the frame */
        gtk_container_add(GTK_CONTAINER(color_frame), draw);

        /* set up the right part of the row */
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, i*2, (i*2)+1);

        color_frame = gtk_frame_new(NULL);
        gtk_widget_set_size_request(color_frame, 20, 20);

        gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 2);

        label = gtk_label_new(default_color_name[i+(NR_ANSI_COLORS/2)]);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

        /* a small drawing area with the background set to the color */
        draw = gtk_drawing_area_new();
        gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(G_OBJECT(draw), "button-press-event",
                         G_CALLBACK(gui_color_button),
                         &user->gui_user.colors[i+(NR_ANSI_COLORS/2)]);

        /* set the color */
        gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                             &user->gui_user.colors[i+(NR_ANSI_COLORS/2)]);

        /* put the draw area in the frame */
        gtk_container_add(GTK_CONTAINER(color_frame), draw);
    }
    
    /* setup one more row - first the foreground */
    frame = gtk_frame_new("Default Colors");
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 2);

    table = gtk_table_new(1, NR_DEF_COLORS, TRUE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, 0, 1);
                                
    color_frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(color_frame, 20, 20);

    gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 2);

    label = gtk_label_new("- Foreground");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    draw = gtk_drawing_area_new();
    gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(draw), "button-press-event",
                     G_CALLBACK(gui_color_button),
                     &user->gui_user.default_color[DEF_FG]);

    /* set the color */
    gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                         &user->gui_user.default_color[DEF_FG]);

    /* put the draw area in the frame */
    gtk_container_add(GTK_CONTAINER(color_frame), draw);

    /* right part is the background */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 0, 1);

    color_frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(color_frame, 20, 20);

    gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 5);

    label = gtk_label_new("- Background");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    draw = gtk_drawing_area_new();
    gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(draw), "button-press-event",
                     G_CALLBACK(gui_color_button),
                     &user->gui_user.default_color[DEF_BG]);

    /* set the color */
    gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                         &user->gui_user.default_color[DEF_BG]);

    /* put the draw area in the frame */
    gtk_container_add(GTK_CONTAINER(color_frame), draw);
    
    /* setup one more row - first the info color */
    frame = gtk_frame_new("Additional Colors");
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 2);

    table = gtk_table_new(1, 2, TRUE);
    gtk_container_add(GTK_CONTAINER(frame), table);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, 0, 1);
                                
    color_frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(color_frame, 20, 20);

    gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 2);

    label = gtk_label_new("- Info messages");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    draw = gtk_drawing_area_new();
    gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(draw), "button-press-event",
                     G_CALLBACK(gui_color_button),
                     &user->gui_user.colors[COLOR_INFO]);

    /* set the color */
    gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                         &user->gui_user.colors[COLOR_INFO]);

    /* put the draw area in the frame */
    gtk_container_add(GTK_CONTAINER(color_frame), draw);

    /* right part is the echo */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 0, 1);

    color_frame = gtk_frame_new(NULL);
    gtk_widget_set_size_request(color_frame, 20, 20);

    gtk_frame_set_shadow_type(GTK_FRAME(color_frame), GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(hbox), color_frame, FALSE, FALSE, 5);

    label = gtk_label_new("- Input echo");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);

    draw = gtk_drawing_area_new();
    gtk_widget_add_events(draw, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(draw), "button-press-event",
                     G_CALLBACK(gui_color_button),
                     &user->gui_user.colors[COLOR_ECHO]);

    /* set the color */
    gtk_widget_modify_bg(draw, GTK_STATE_NORMAL, 
                         &user->gui_user.colors[COLOR_ECHO]);

    /* put the draw area in the frame */
    gtk_container_add(GTK_CONTAINER(color_frame), draw);
    
    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_DEFAULT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_color_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_color_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_color_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_color_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_end(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
