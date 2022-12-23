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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(WIN32)
  #include <unistd.h>
  #include <sys/time.h>
#endif
#include <time.h>
#include "mudix.h"


static CAPT_WINDOW *capt_window_list = NULL;


CAPT_WINDOW *capt_win_lookup(gchar *title)
{
    CAPT_WINDOW *lookup;

    for (lookup = capt_window_list; lookup; lookup = lookup->next)
    {
        if (!strcmp(gtk_window_get_title(GTK_WINDOW(lookup->g_window)), title))
        {
            return lookup;
        }
    }

    return NULL;
}


static CAPT_WINDOW *new_capt_window(void)
{
    CAPT_WINDOW *capt;

    if (!(capt = malloc(sizeof(CAPT_WINDOW))))
    {
	return NULL;
    }
     
    capt->next       = capt_window_list;
    capt_window_list = capt;

    return capt;
}


static void free_capt_window(CAPT_WINDOW *capt)
{
    if (!capt)
    {
	return;
    }

    if (capt == capt_window_list)
    {
        capt_window_list = capt->next;
    }
    else
    {
        CAPT_WINDOW *lookup;

        for (lookup = capt_window_list; lookup; lookup = lookup->next) 
        {
            if (lookup->next == capt) 
            {
                lookup->next = capt->next;
                break;
            }
        }
    }

    free(capt);
}


static void gui_set_view_window(USER *user, GtkWidget *view)
{
    /* change attributes of the view */
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_CHAR);

    /* update the FG and BG color in the view */
    gui_color_def_widget(user, view);
}


static void gui_destroy_window_callback(GtkObject *window, CAPT_WINDOW *capt)
{
    free_capt_window(capt);
}


static void gui_capt_setup_tags(USER *user, CAPT_WINDOW *capt)
{
    int i;

    /* create a buffer tag for all possible colors */
    for (i=0; i<NR_ALL_COLORS; i++)
    {
        capt->g_fg_color_tags[i] =
            gtk_text_buffer_create_tag(capt->g_buffer, NULL,
                                       "foreground-gdk",
                                       &user->gui_user.colors[i], NULL);
    }
}


static CAPT_WINDOW *gui_setup_capt_window(USER *user, gchar *title)
{
    CAPT_WINDOW            *capt;
    GtkWidget              *window;
    GtkTextBuffer          *buffer;
    GtkWidget              *view;
    GtkTextMark            *mark_begin, *mark_end;
    GtkWidget              *scrolled_window;
    GtkTextIter             iter;
    PangoFontDescription   *font_desc;


    if (!(capt = new_capt_window()))
    {
        /* eww - capture structure could not be created? */
        return NULL;
    }

    /* create the main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), title); 

    /* setup event handlers for the window */
    g_signal_connect(G_OBJECT(window), "destroy",
	             G_CALLBACK(gui_destroy_window_callback), capt);

    gtk_container_set_border_width(GTK_CONTAINER(window), 2);
    gtk_window_set_default_size(GTK_WINDOW(window),
                                DEFAULT_CAPT_WIDTH, 
                                DEFAULT_CAPT_HEIGHT);

    /* create a text view */
    view = gtk_text_view_new();

    /* set left/right margin */
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 2);

    /* create a scrolled window */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		   	           GTK_POLICY_AUTOMATIC,
				   GTK_POLICY_ALWAYS);
    /* put the text view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(scrolled_window), view);

    /* put the scrolled_window inside the window */
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    /* set the view properties */
    gui_set_view_window(user, view);

    /* get a pointer to the buffer within the view */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    /* set the begin and the end mark of the buffer */
    gtk_text_buffer_get_start_iter(buffer, &iter);
    mark_begin = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

    gtk_text_buffer_get_end_iter(buffer, &iter);
    mark_end = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

    /* store pointers */
    capt->g_window = window;
    capt->g_view   = view;
    capt->g_mark   = mark_end;
    capt->g_buffer = buffer;

    gui_capt_setup_tags(user, capt);

    /* change default font throughout the widget */
    font_desc = pango_font_description_from_string(user->gui_user.fonts[FONT_USER_CAPTURE]);
    gtk_widget_modify_font(capt->g_view, font_desc);
    pango_font_description_free(font_desc);

    /* show the window :-) */
    gtk_widget_show_all(window);

    return capt;
}


void gui_add_to_capt(USER *user, gchar *title, gchar *buf, gsize len, guint color)
{
    CAPT_WINDOW   *capt;
    gchar          buffer[MAX_STRING];
    gchar         *pBuf;
    GtkTextIter    iter;
#if !defined(WIN32)
    struct timeval time;
#endif
           time_t  current_time;

    /* see if we already have this capture window */
    if (!(capt = capt_win_lookup(title)))
    {
        /* nope - then create it */
        capt = gui_setup_capt_window(user, title);
    }

    if (!capt)
    {
        /* eww - capture window could not be created? */
        return;
    }

#if !defined(WIN32)
    /* get time */
    gettimeofday(&time, NULL);
    current_time = (time_t)time.tv_sec;
#else
    time(&current_time);
#endif

    /* hack off the annoying '\n' from ctime */
    strcpy(buffer, ctime(&current_time));

     pBuf   = buffer + 20;
    *pBuf++ = ':';
    *pBuf++ = ' ';
    *pBuf++ = '\0';

    len += (pBuf-buffer)-1;
    strcat(buffer, buf);

    /* get the end iter which will point to the insertion point */
    gtk_text_buffer_get_end_iter(capt->g_buffer, &iter);

    if (color < NR_ANSI_COLORS)
    {
        /* just send buffer with color to window */
        gtk_text_buffer_insert_with_tags(capt->g_buffer, 
                                         &iter,
                                         buffer, len, 
                                         capt->g_fg_color_tags[color],
                                         NULL);
    }
    else
    {
        /* insert text with default colors */
        gtk_text_buffer_insert(capt->g_buffer, &iter, buf, len);

    }

    /* scroll to end of the buffer */
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(capt->g_view), capt->g_mark);
}


bool gui_cls_capt(USER *user, gchar *title)
{
    CAPT_WINDOW   *capt;

    /* see if we have this capture window */
    if ((capt = capt_win_lookup(title)))
    {
        GtkTextIter start, end;

        gtk_text_buffer_get_start_iter(capt->g_buffer, &start);
        gtk_text_buffer_get_end_iter(capt->g_buffer, &end);
        gtk_text_buffer_delete(capt->g_buffer, &start, &end);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}






