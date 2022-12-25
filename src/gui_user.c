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
#endif
#include "mudix.h"

static void menu_select(gpointer data, guint action, GtkWidget *widget);

static char *wordDelimiters = " \"'`~!@#$%^&*()_-+=[]{}:;\\|/?.>,<";

typedef enum
{
    DIA_RESP_RECONNECT,
    DIA_RESP_OFFLINE,
    DIA_RESP_CLOSE
} DIA_RESPONSE;


typedef enum
{
    MENU_NONE, FILE_OPEN, FILE_SAVE, FILE_EXIT,
    VIEW_GENERAL, VIEW_GENERAL_CHAR, VIEW_GENERAL_COLOR, VIEW_GENERAL_FONTS,
    VIEW_ALIASES, VIEW_MACROS, VIEW_PATHS, VIEW_TABS, VIEW_TIMERS, VIEW_TRIGGERS, VIEW_VARS,
    WRAP_CHAR, WRAP_WORD, WRAP_NONE,
    SESSION_RECONNECT, SESSION_DISCONNECT, SESSION_TOGGLE_CLEAR,
    SESSION_TOGGLE_ECHO, SESSION_TOGGLE_MUTE,
    HELP_ABOUT,
    EDIT_SELECTALL, EDIT_CUT, EDIT_COPY, EDIT_PASTE, EDIT_CUTWORD, EDIT_CUTLINE
} MENU_ACTION;


static GtkItemFactoryEntry menu_items[] =
{
  { "/File",                    NULL,                   NULL,           MENU_NONE, "<Branch>" },
  { "/File/Open User-Selector", "<control>O",           menu_select,    FILE_OPEN },
  { "/File/Save",               "<control>S",           menu_select,    FILE_SAVE, "<StockItem>", GTK_STOCK_SAVE },
  { "/File/sep1",               NULL,                   NULL,           MENU_NONE, "<Separator>" },
  { "/File/Exit",               "<control>Q",           menu_select,    FILE_EXIT, "<StockItem>", GTK_STOCK_QUIT },
  { "/Edit",                    NULL,                   NULL,           MENU_NONE, "<Branch>" },
  { "/Edit/Select All",         "<control>A",           menu_select,    EDIT_SELECTALL },
  { "/Edit/sep1",               NULL,                   NULL,           MENU_NONE, "<Separator>" },
  { "/Edit/Cut",                "<control>X",           menu_select,    EDIT_CUT },
  { "/Edit/Copy",               "<control>C",           menu_select,    EDIT_COPY },
  { "/Edit/Paste",              "<control>V",           menu_select,    EDIT_PASTE },
  { "/Edit/sep1",               NULL,                   NULL,           MENU_NONE, "<Separator>" },
  { "/Edit/Cut Word",           "<control>W",           menu_select,    EDIT_CUTWORD },
  { "/Edit/Cut Line",           "<control>L",           menu_select,    EDIT_CUTLINE },

  { "/View",                    NULL,                   NULL,           MENU_NONE, "<Branch>" },
  { "/View/Preferences",        "<control>G",           menu_select,    VIEW_GENERAL },
  { "/View/sep1",               NULL,                   NULL,           MENU_NONE, "<Separator>" },
  { "/View/General",            NULL,                   NULL,           MENU_NONE, "<Branch>" },
  { "/View/General/Character",  "<control><alt>H",      menu_select,    VIEW_GENERAL_CHAR },
  { "/View/General/Color",      "<control><alt>C",      menu_select,    VIEW_GENERAL_COLOR },
  { "/View/General/Fonts",      "<control><alt>F",      menu_select,    VIEW_GENERAL_FONTS },

  /* NOTE: when the following wrapping paths are changed, do NOT forget to
           update the corresponding paths at the end of the gui_setup_user_window()
           function! */
  { "/View/General/Wrapping",   NULL,                   NULL,           MENU_NONE, "<Branch>" },
  { "/View/General/Wrapping/Character", NULL,           menu_select,    WRAP_CHAR, "<RadioItem>" },
  { "/View/General/Wrapping/Word", NULL,                menu_select,    WRAP_WORD, "/View/General/Wrapping/Character" },
  { "/View/General/Wrapping/None", NULL,                menu_select,    WRAP_NONE, "/View/General/Wrapping/Character" },

  { "/View/sep1",               NULL,                   NULL,           MENU_NONE, "<Separator>" },
  { "/View/Aliases",            "<control><alt>A",      menu_select,    VIEW_ALIASES },
  { "/View/Macros",             "<control><alt>M",      menu_select,    VIEW_MACROS },
  { "/View/Paths",              "<control><alt>P",      menu_select,    VIEW_PATHS },
  { "/View/Tab completion",     "<control><alt>B",      menu_select,    VIEW_TABS },
  { "/View/Timers",             "<control><alt>I",      menu_select,    VIEW_TIMERS },
  { "/View/Triggers",           "<control><alt>T",      menu_select,    VIEW_TRIGGERS },
  { "/View/Variables",          "<control><alt>V",      menu_select,    VIEW_VARS },

  { "/Session",                         NULL,           NULL,           MENU_NONE, "<Branch>" },
  { "/Session/Reconnect",               "<alt>R",       menu_select,    SESSION_RECONNECT },
  { "/Session/Disconnect",              "<alt>D",       menu_select,    SESSION_DISCONNECT },
  { "/Session/sep1",                    NULL,           NULL,           MENU_NONE, "<Separator>" },
  { "/Session/Toggle command mute",     "<alt>M",       menu_select,    SESSION_TOGGLE_MUTE },
  { "/Session/Toggle input echo",       "<alt>E",       menu_select,    SESSION_TOGGLE_ECHO },
  { "/Session/Toggle input auto-clear", "<alt>C",       menu_select,    SESSION_TOGGLE_CLEAR },

  { "/Help",                            NULL,           0,              MENU_NONE, "<LastBranch>" },
  { "/Help/_About",                     NULL,           menu_select,    HELP_ABOUT },
};


static void gui_about_box(USER *user)
{
    GtkWidget       *dialog;
    GtkWidget       *label;

    dialog = gtk_dialog_new_with_buttons("About gMUDix",
                            GTK_WINDOW(user->gui_user.g_window),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_STOCK_OK, GTK_RESPONSE_OK,
                            NULL);

    label = gtk_label_new("gMUDix " VERSION);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 5);

    label = gtk_label_new("Developed by Marko Boomstra.");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 5);

    label = gtk_label_new("Mail to: m.boomstra@chello.nl (home) or");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 5);

    label = gtk_label_new("marko.boomstra@ict.nl (work).");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 5);

    label = gtk_label_new("Check http://dw.nl.eu.org for the latest version.");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 10);

    gtk_widget_show_all(GTK_WIDGET(dialog));

    /* wait for a response */
    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
    return;
}


static long getWordDelimiterOffset(gchar *input)
{
    gchar  *lastFind = (gchar*)0L, *curFind = (gchar*)0L;
    gchar  *curLoc = input;
    long    numWordDelimiters = strlen(wordDelimiters);
    long    retVal = 0, i;
    long    amountToSearch = g_utf8_strlen(input, -1);
    gchar   gc;

    for (i = 0; i < numWordDelimiters; i++)
    {
        gc = (gchar)wordDelimiters[i];
        curFind = g_utf8_strrchr(curLoc, amountToSearch, gc);

        if (curFind > lastFind)
        {
            lastFind = curFind;
            curLoc = curFind;
            amountToSearch = g_utf8_strlen(lastFind, -1);
        }
    }

    if (lastFind != (gchar*)0L)
    {
        retVal = (long)lastFind - (long)input;
    }

    return(retVal);
}


static void menu_select(gpointer data, guint action, GtkWidget *widget)
{
    USER *user;

    if (!(user = get_user_window(GTK_WINDOW(data))))
    {
        /* window has no user?? something must be wrong now ;) */
        return;
    }

    switch (action)
    {
        case FILE_OPEN:
            gui_setup_main_window();
            break;
        case FILE_EXIT:
            destroy_with_signal(data);
            break;
        case FILE_SAVE:
            if (save_user(user))
            {
                gui_info_message(user, "User file saved.\n");
            }
            else
            {
                gui_info_message(user, "Error: could not open file for write!\n");
            }
            break;
        case VIEW_GENERAL:
            gui_preference_editor(user, SEL_GENERAL);
            break;
        case VIEW_GENERAL_CHAR:
            gui_preference_editor(user, SEL_CHAR);
            break;
        case VIEW_GENERAL_COLOR:
            gui_preference_editor(user, SEL_COLOR);
            break;
        case VIEW_GENERAL_FONTS:
            gui_preference_editor(user, SEL_FONTS);
            break;
        case VIEW_ALIASES:
            gui_preference_editor(user, SEL_ALIASES);
            break;
        case VIEW_MACROS:
            gui_preference_editor(user, SEL_MACROS);
            break;
        case VIEW_PATHS:
            gui_preference_editor(user, SEL_PATHS);
            break;
        case VIEW_TABS:
            gui_preference_editor(user, SEL_TABS);
            break;
        case VIEW_TIMERS:
            gui_preference_editor(user, SEL_TIMERS);
            break;
        case VIEW_TRIGGERS:
            gui_preference_editor(user, SEL_TRIGGERS);
            break;
        case VIEW_VARS:
            gui_preference_editor(user, SEL_VARS);
            break;
        case WRAP_CHAR:
            user->gui_user.wrap_mode = GTK_WRAP_CHAR;
            gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(user->gui_user.g_view),
                                        user->gui_user.wrap_mode);
            break;
        case WRAP_WORD:
            user->gui_user.wrap_mode = GTK_WRAP_WORD;
            gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(user->gui_user.g_view),
                                        user->gui_user.wrap_mode);
            break;
        case WRAP_NONE:
            user->gui_user.wrap_mode = GTK_WRAP_NONE;
            gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(user->gui_user.g_view),
                                        user->gui_user.wrap_mode);
            break;
        case SESSION_RECONNECT:
            /* just call the reconnect function */
            gui_user_reconnect(user);
            break;
        case SESSION_DISCONNECT:
            /* just disconnect */
            gui_user_disconnect(user);
            break;
        case SESSION_TOGGLE_CLEAR:
            cmd_input_clear(user, 0, "");
            break;
        case SESSION_TOGGLE_ECHO:
            cmd_input_echo(user, 0, "");
            break;
        case SESSION_TOGGLE_MUTE:
            cmd_mute(user, 0, "");
            break;
        case HELP_ABOUT:
            gui_about_box(user);
            break;
        case EDIT_SELECTALL:
            gtk_editable_select_region(GTK_EDITABLE(user->gui_user.g_input), 0, -1);
            break;
        case EDIT_CUTWORD:
            {
                long    start = 0;
                gchar  *input;

                input = gtk_editable_get_chars(GTK_EDITABLE(user->gui_user.g_input), start, -1);
                start = getWordDelimiterOffset(input);

                gtk_editable_delete_text(GTK_EDITABLE(user->gui_user.g_input), start, -1);
            }
            break;
        case EDIT_CUTLINE:
            gtk_editable_delete_text(GTK_EDITABLE(user->gui_user.g_input), 0, -1);
            break;
        case EDIT_COPY:
            {
              gint start, end;

              if (gtk_editable_get_selection_bounds(GTK_EDITABLE(user->gui_user.g_input),
                                                    &start, &end))
              {
                  gtk_editable_copy_clipboard(GTK_EDITABLE(user->gui_user.g_input));
              }
              else
              {
                  g_signal_emit_by_name(user->gui_user.g_view, "copy-clipboard");
              }
            }
            break;
        case EDIT_CUT:
            gtk_editable_cut_clipboard(GTK_EDITABLE(user->gui_user.g_input));
            break;
        case EDIT_PASTE:
            gtk_editable_paste_clipboard(GTK_EDITABLE(user->gui_user.g_input));
            break;
        default:
            break;
    }
}


static gboolean update_views_scrollback(gpointer data)
{
    USER      *user = (USER *)data;
    GtkWidget *view = get_sb_view(user);

    /* update the scrollback and normal view */
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(user->gui_user.g_view),
                                 user->gui_user.g_mark,
                                 0, TRUE, 1.0, 0);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(view),
                                 user->gui_user.g_mark,
                                 0, TRUE, 1.0, 0);

    return FALSE;   /* only be called once */
}


static gboolean gui_key_event_callback(GtkWidget    *widget,
                                       GdkEventKey  *event,
                                       USER         *user)
{
    switch (event->keyval)
    {
        case GDK_Page_Up:
            if (!GTK_WIDGET_VISIBLE(user->gui_user.g_scrollback))
            {
                gtk_widget_show_all(user->gui_user.g_scrollback);
                gtk_timeout_add(SB_TIMER, update_views_scrollback, user);
            }
            else
            {
                GtkAdjustment *adjust;
                gdouble        newval;

                adjust = gtk_scrolled_window_get_vadjustment(
                         GTK_SCROLLED_WINDOW(user->gui_user.g_scrollback));

                /* get the value from the vertical adjustment and update */
                newval = adjust->value - adjust->page_increment;
                if (newval < adjust->lower)
                {
                    newval = adjust->lower;
                }

                gtk_adjustment_set_value(adjust, newval);
            }
            break;
        case GDK_Page_Down:
            if (GTK_WIDGET_VISIBLE(user->gui_user.g_scrollback))
            {
                GtkAdjustment *adjust;
                gdouble        newval;

                adjust = gtk_scrolled_window_get_vadjustment(
                         GTK_SCROLLED_WINDOW(user->gui_user.g_scrollback));

                /* at end of scrollback - switch back to normal view */
                if (adjust->value == (adjust->upper - adjust->page_size))
                {
                    gtk_widget_hide_all(user->gui_user.g_scrollback);
                    gtk_timeout_add(SB_TIMER, update_views_scrollback, user);
                    break;
                }

                /* get the value from the vertical adjustment and update */
                newval = adjust->value + adjust->page_increment;
                if (newval > (adjust->upper - adjust->page_size))
                {
                    newval = (adjust->upper - adjust->page_size);
                }

                gtk_adjustment_set_value(adjust, newval);
            }
            break;
        case GDK_Escape:
            if (GTK_WIDGET_VISIBLE(user->gui_user.g_scrollback))
            {
                gtk_widget_hide_all(user->gui_user.g_scrollback);
                gtk_timeout_add(SB_TIMER, update_views_scrollback, user);
            }
            break;
        case GDK_Tab:
            check_tab(user, (gchar *)gtk_entry_get_text(GTK_ENTRY(user->gui_user.g_input)));
            /* do not send key to input bar */
            return TRUE;
        case GDK_Up:
          {
            gchar *history;

            if ((history = get_prev_history_update(user)))
            {
                gtk_entry_set_text(GTK_ENTRY(user->gui_user.g_input), history);
                gtk_editable_set_position(GTK_EDITABLE(user->gui_user.g_input), -1);
            }
            else
            {
                gui_input_clear(user);
            }
             /* do not send key to input bar */
            return TRUE;
          }
        case GDK_Down:
          {
            gchar *history;

            if ((history = get_next_history_update(user)))
            {
                gtk_entry_set_text(GTK_ENTRY(user->gui_user.g_input), history);
                gtk_editable_set_position(GTK_EDITABLE(user->gui_user.g_input), -1);
            }
            else
            {
                gui_input_clear(user);
            }
            /* do not send key to input bar */
            return TRUE;
          }
        default:
            if (process_macro(user, event->keyval, event->state))
            {
                /* do not send key to input bar */
                return TRUE;
            }

            break;
    }

    /* also send key to input bar */
    return FALSE;
}


static void gui_enter_callback(GtkWidget *entry, USER *user)
{
    gchar *input = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry));

    /* process the input */
    process_input(user, input, NULL);

    if (gui_input_is_vis(user))
    {
        /* add it to history */
        add_to_history(user, input);

        if (user->flags & FLG_INPUT_AUTO_CLEAR)
        {
            /* clear the input if set */
            gui_input_clear(user);
        }
        else
        {
            /* highlight the input in the entry bar */
            gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        }
    }
    else
    {
        /* clear the input if it's invisible */
        gui_input_clear(user);
    }
}


static void gui_destroy_window_callback(GtkObject *window, USER *user)
{
    /* close the preference window if it's open */
    if (user->gui_pref.g_preference)
    {
        gtk_widget_destroy(user->gui_pref.g_preference);
    }

    /* if this user is logging to a file, close it */
    if (user->logfile)
    {
        close_log(user);
    }

    /* close the socket - do not call gui_user_disconnect now */
    do_disconnect(user);

    /* save the user */
    save_user(user);

    /* update the row in the main window that belongs to this user */
    gui_main_update_row(user);

    /* clean up allocated stuff */
    del_user(user);
}


static void gui_set_view_window(USER *user, GtkWidget *view)
{
    /* change attributes of the view */
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), user->gui_user.wrap_mode);

    /* update the FG and BG color in the view */
    gui_color_def_widget(user, view);
}


static gboolean gui_user_config_event(GtkWidget         *widget,
                                      GdkEventConfigure *event,
                                      USER              *user)
{
    /* just store our new size so we can save it later */
    if (widget == user->gui_user.g_window)
    {
        if (event->height != user->gui_user.win_height ||
            event->width  != user->gui_user.win_width)
        {
            user->gui_user.win_height = event->height;
            user->gui_user.win_width  = event->width;

            /* size got updated, send NAWS if required */
            send_naws(user);
        }

        user->gui_user.win_pos_x = event->x;
        user->gui_user.win_pos_y = event->y;
    }


    /* FALSE passes the event through into the widget */
    return FALSE;
}


static gboolean gui_user_focus_in(GtkEntry      *entry,
                                  GdkEventFocus *event,
                                  USER          *user)
{
    /* if we had a selection prior to focus loss - re-set it now */
    if (user->gui_user.input_select_start ||
        user->gui_user.input_select_end)
    {
        gtk_editable_select_region(GTK_EDITABLE(entry),
                                   user->gui_user.input_select_start,
                                   user->gui_user.input_select_end);
    }

    return FALSE;
}


static gboolean gui_user_focus_out(GtkEntry         *entry,
                                   GdkEventFocus    *event,
                                   USER             *user)
{
    /* remember that we had selected text in our input */
    gtk_editable_get_selection_bounds(GTK_EDITABLE(entry),
                                      &user->gui_user.input_select_start,
                                      &user->gui_user.input_select_end);

    return FALSE;
}


static gboolean gui_reconnect_timer(gpointer data)
{
    GtkWidget *dialog = GTK_WIDGET(data);
    GtkWindow *window = gtk_window_get_transient_for(GTK_WINDOW(dialog));
    USER      *user;
    int        sock;

    if (window && (user = get_user_window(window)))
    {
        /* first lock the mutex */
        g_mutex_lock(user_network_mutex);

        /* retrieve the socket number to see if we are connected */
        sock = user->net.sock;

        /* unlock the mutex */
        g_mutex_unlock(user_network_mutex);

        /* if we are not yet connected */
        if (!sock)
        {
            /* update the timer and display time remaining */
            if (--user->net.recon_timer > 0)
            {
                gchar buf[MAX_SMALL_STR];

                sprintf(buf, "Reconnecting in %d second%s...",
                    user->net.recon_timer, (user->net.recon_timer > 1)? "s": "");
                gui_status_msg(user, buf);

                return TRUE;    /* keep timer alive */
            }
            else
            {
                /* destroy the dialog */
                gtk_widget_destroy(dialog);

                /* actually reconnect */
                gui_user_reconnect(user);
            }
        }
    }

    return FALSE;   /* destroy timer */
}


static void disconnect_response(GtkDialog *dialog, DIA_RESPONSE resp, USER *user)
{
    switch (resp)
    {
        case DIA_RESP_RECONNECT:
            /* first destroy the dialog */
            gtk_widget_destroy(GTK_WIDGET(dialog));

            /* reconnect */
            gui_user_reconnect(user);
            break;
        case DIA_RESP_CLOSE:
            /* destroy the user window */
            destroy_with_signal(user->gui_user.g_window);
            break;
        case DIA_RESP_OFFLINE:
        default:
            /* call disconnect function to be sure it's offline :) */
            do_disconnect(user);

            /* destroy the dialog */
            gtk_widget_destroy(GTK_WIDGET(dialog));

            /* update status bar */
            gui_connection_status(user);
            break;
    }
}


static void gui_disconnect_dialog(USER *user, gchar *msg)
{
    GtkWidget *dialog;
    GtkWidget *label;

    /* create a dialog with response buttons */
    dialog = gtk_dialog_new_with_buttons("Disconnected",
                            GTK_WINDOW(user->gui_user.g_window),
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            "Reconnect", DIA_RESP_RECONNECT,
                            "Offline", DIA_RESP_OFFLINE,
                            GTK_STOCK_CLOSE, DIA_RESP_CLOSE,
                            NULL);

    /* set some labels */
    label = gtk_label_new(msg);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 10);

    label = gtk_label_new("Make your choice:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE, FALSE, 10);

    /* this will cause the dialog to be closed on reconnection */
    gtk_window_set_transient_for(GTK_WINDOW(dialog),
                                 GTK_WINDOW(user->gui_user.g_window));

    /* check response */
    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(disconnect_response), user);

    /* start the reconnect timer if required, 1 sec resolution */
    if (user->net.recon_timer_load)
    {
        /* load the timer first */
        user->net.recon_timer = user->net.recon_timer_load;

        /* add a timer, NOTE that dialog is passed as argument */
        user->net.recon_id = gtk_timeout_add(TIMER_TIMEOUT,
                                             gui_reconnect_timer, dialog);
    }

    /* show the dialog box */
    gtk_widget_show_all(GTK_WIDGET(dialog));
}


void gui_user_reconnect(USER *user)
{
    TRIGGER *trig;

    /* first disconnect - do NOT call gui_user_disconnect! */
    do_disconnect(user);

    /* in case the input bar is invis, make it vis! */
    if (!gui_input_is_vis(user))
    {
        /* also clear the input if it's invisible */
        gui_input_clear(user);
        gui_input_visible(user, TRUE);
    }

    /* then re-enable the login triggers (if any) */
    for (trig = user->trigger_list; trig; trig = trig->next)
    {
        if (trig->level == TRG_LOGIN ||
            trig->level == TRG_PASSWORD)
        {
            trig->enabled = TRUE;
        }
    }

    /* finally connect again */
    gui_user_connect(user);
}


void gui_user_disconnect(USER *user)
{
    gint sock;

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    sock = user->net.sock;

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    /* actually disconnect */
    do_disconnect(user);

    /* only pop up a message if we were connected ;) */
    if (sock)
    {
        gui_disconnect_dialog(user, "Disconnected from remote host.");
    }

    /* update the status bar */
    gui_connection_status(user);
}


gboolean gui_connection_status(USER *user)
{
                  NET_CODE  status;
                  gchar     buf[MAX_STATUS_LEN];
    static const  gchar    *net_errors[NET_NR_STATUS-NET_CONNECT_FAILURE] = {
                                "Could not connect to host",
                                "Unable to resolve hostname",
                                "Failed to create a socket" };

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    /* grab the status only once */
    status = user->net.status;

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    switch (status)
    {
        case NET_CLOSED:
            gui_status_msg(user, "Not connected");
            break;
        case NET_CONNECTED:
            /* first lock the mutex */
            g_mutex_lock(user_network_mutex);

            sprintf(buf, "Connected to %s (%s) port %d",
                    user->net.host_name, user->net.host_addr, user->net.port);

            /* unlock the mutex */
            g_mutex_unlock(user_network_mutex);

            gui_add_color_window(user, buf, strlen(buf), COLOR_INFO);
            gui_add_color_window(user, "\n", 1, COLOR_NONE);
            gui_status_msg(user, buf);
            break;
        case NET_CONNECT_FAILURE:
        case NET_GETHOSTBYNAME:
        case NET_NO_SOCKET:
            sprintf(buf, "Error connecting to %s port %d.\nError code %d: %s.",
                    user->net.site, user->net.port, status,
                    net_errors[status-NET_CONNECT_FAILURE]);

            gui_status_msg(user, "Not connected");
            gui_disconnect_dialog(user, buf);
            break;
        default:
            break;
    }

    /* FALSE indicates to execute once for timer functions */
    return FALSE;
}


void gui_user_connect(USER *user)
{
    gui_status_msg(user, "Attempting to connect...");

    /* create the thread */
    g_thread_create((GThreadFunc)connect_thread, user, FALSE, NULL);
}


void gui_dialog_msg(USER *user, gchar *msg)
{
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(GTK_WINDOW(user->gui_user.g_window),
                GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
                GTK_BUTTONS_CLOSE, msg);

    gtk_window_set_transient_for(GTK_WINDOW(user->gui_user.g_window),
                                 GTK_WINDOW(dialog));

    /* close dialog on user response */
    g_signal_connect(G_OBJECT(dialog), "response",
                     G_CALLBACK(gtk_widget_destroy), NULL);

    gtk_widget_show(dialog);
}


void gui_status_msg(USER *user, gchar *msg)
{
    /* clear any previous message, underflow is allowed */
    gui_clear_status(user);
    gtk_statusbar_push(GTK_STATUSBAR(user->gui_user.g_status), 0, msg);
}


void gui_user_get_xy(USER *user, gint *x, gint *y)
{
    GdkRectangle          rect;
    PangoFontDescription *font_desc;
    PangoFontMetrics     *metrics;
    PangoContext         *context = gtk_widget_get_pango_context(user->gui_user.g_view);
    int                   width=PANGO_SCALE, height, i;
    const char           *q[] = { "W", "=" };

    /* grab the font description */
    font_desc = pango_font_description_from_string(user->gui_user.fonts[FONT_USER_WINDOW]);

    /* grab the metrics structure */
    metrics = pango_context_get_metrics(context, font_desc, NULL);

    /* free the font description again */
    pango_font_description_free(font_desc);

    /* get the size of the visible buffer */
    gtk_text_view_get_visible_rect(GTK_TEXT_VIEW(user->gui_user.g_view), &rect);

    height = pango_font_metrics_get_ascent(metrics) +
             pango_font_metrics_get_descent(metrics);

    pango_font_metrics_unref(metrics);

    /* now also get the width */
    for (i=0; i<2; i++)
    {
        PangoGlyphString *glyphs;
        GList *items;
        PangoItem *item;
        PangoAttrList *attrib;

        attrib = pango_attr_list_new();
        glyphs = pango_glyph_string_new();
        items = pango_itemize(context, q[i], 0, 1, attrib, NULL);
        item = (PangoItem *)items->data;

        /* get geometry information */
        pango_shape(q[i], 1, &item->analysis, glyphs);

        if (width < glyphs->glyphs[0].geometry.width)
        {
            width = glyphs->glyphs[0].geometry.width;
        }

        pango_attr_list_unref(attrib);
        pango_item_free (item);
        g_list_free(items);
        pango_glyph_string_free (glyphs);
    }

    /* return the calculated width and height */
    *x = rect.width / (width / PANGO_SCALE);
    *y = rect.height / (height / PANGO_SCALE);
}


void gui_user_backspace(USER *user)
{
    GtkTextIter start = user->gui_user.input_iter;

    /* get the iter pointing to the start of the buffer we inserted */
    gtk_text_iter_backward_chars(&start, 1);

    /* delete the character */
    gtk_text_buffer_delete(user->gui_user.g_buffer, &start,
                           &user->gui_user.input_iter);

    /* update the input iter because delete also deletes iters */
    gtk_text_buffer_get_end_iter(user->gui_user.g_buffer,
                                 &user->gui_user.input_iter);
}


void gui_add_color_window(USER *user, gchar *buf, gsize len, guint color)
{
    if (color < NR_ALL_COLORS)
    {
        /* just send buffer with color to window */
        gtk_text_buffer_insert_with_tags(
                user->gui_user.g_buffer,
                &user->gui_user.input_iter,
                buf, len,
                user->gui_user.g_fg_color_tags[color],
                NULL);
    }
    else
    {
        /* insert text with default colors */
        gtk_text_buffer_insert(user->gui_user.g_buffer,
                               &user->gui_user.input_iter, buf, len);

    }

    if (gtk_text_buffer_get_line_count(user->gui_user.g_buffer) >
                                       user->gui_user.lines_max)
    {
        GtkTextIter start, end;

        /* delete lines, 1/40 of the total buffer */
        gtk_text_buffer_get_start_iter(user->gui_user.g_buffer, &start);
        gtk_text_buffer_get_iter_at_line(
            user->gui_user.g_buffer, &end, (user->gui_user.lines_max / 50)+1);
        gtk_text_buffer_delete(user->gui_user.g_buffer, &start, &end);

        /* update the input iter because delete also deletes iters */
        gtk_text_buffer_get_end_iter(user->gui_user.g_buffer,
                                     &user->gui_user.input_iter);
    }

    /* if the user has a logfile open, dump the buffer in the file */
    if (user->logfile)
    {
        write_log(user, buf, len);
    }

    /* scroll to end of the buffer */
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(user->gui_user.g_view),
                                       user->gui_user.g_mark);
}


void gui_add_ansi_window(USER *user, gchar *buf, gsize len)
{
    GtkTextIter start;
    guchar      fg = user->ansi.fg;
    guchar      bg = user->ansi.bg;
    guint       attrib = user->ansi.attrib;

    /* insert text */
    gtk_text_buffer_insert(user->gui_user.g_buffer,
                           &user->gui_user.input_iter, buf, len);

    /* get the iter pointing to the start of the buffer we inserted */
    start = user->gui_user.input_iter;
    gtk_text_iter_backward_chars(&start, g_utf8_strlen(buf, len));

    if (VALID_ANSI(fg))
    {
        /* ATTR_BOLD is only used with the ANSI fg colors */
        if (attrib & ATTR_BOLD)
        {
            /* grab the bright color */
            fg += COLOR_BOLD_BLACK;
        }

        /* FG color is valid, set FG color */
        gtk_text_buffer_apply_tag(user->gui_user.g_buffer,
                                  user->gui_user.g_fg_color_tags[fg],
                                  &start,
                                  &user->gui_user.input_iter);
    }

    if (VALID_ANSI(bg))
    {
        /* BG color is valid, set BG color */
        gtk_text_buffer_apply_tag(user->gui_user.g_buffer,
                                  user->gui_user.g_bg_color_tags[bg],
                                  &start,
                                  &user->gui_user.input_iter);
    }

    if (attrib & ATTR_UNDERLINE)
    {
        /* underline the text */
        gtk_text_buffer_apply_tag(user->gui_user.g_buffer,
                                  user->gui_user.g_underline_tag,
                                  &start,
                                  &user->gui_user.input_iter);
    }

    if (attrib & ATTR_ITALIC)
    {
        /* make the text italic */
        gtk_text_buffer_apply_tag(user->gui_user.g_buffer,
                                  user->gui_user.g_italic_tag,
                                  &start,
                                  &user->gui_user.input_iter);
    }

    if (gtk_text_buffer_get_line_count(user->gui_user.g_buffer) >
                                       user->gui_user.lines_max)
    {
        GtkTextIter start, end;

        /* delete lines, 1/40 of the total buffer */
        gtk_text_buffer_get_start_iter(user->gui_user.g_buffer, &start);
        gtk_text_buffer_get_iter_at_line(
            user->gui_user.g_buffer, &end, (user->gui_user.lines_max / 50)+1);
        gtk_text_buffer_delete(user->gui_user.g_buffer, &start, &end);

        /* important, update the input iter because delete also deletes iters */
        gtk_text_buffer_get_end_iter(user->gui_user.g_buffer,
                                     &user->gui_user.input_iter);
    }

    /* if the user has a logfile open, dump the buffer in the file */
    if (user->logfile)
    {
        write_log(user, buf, len);
    }

    /* scroll to end of the buffer */
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(user->gui_user.g_view),
                                       user->gui_user.g_mark);
}


void gui_user_cls(USER *user)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter(user->gui_user.g_buffer, &start);
    gtk_text_buffer_get_end_iter(user->gui_user.g_buffer, &end);
    gtk_text_buffer_delete(user->gui_user.g_buffer, &start, &end);

    /* important, update the input iter because delete also deletes iters */
    gtk_text_buffer_get_end_iter(user->gui_user.g_buffer,
                                 &user->gui_user.input_iter);
}


void gui_user_setup_tags(USER *user)
{
    int i;

    /* create a buffer tag for all possible colors */
    for (i=0; i<NR_ALL_COLORS; i++)
    {
        user->gui_user.g_fg_color_tags[i] =
            gtk_text_buffer_create_tag(user->gui_user.g_buffer, NULL,
                                       "foreground-gdk",
                                       &user->gui_user.colors[i], NULL);
        user->gui_user.g_bg_color_tags[i] =
            gtk_text_buffer_create_tag(user->gui_user.g_buffer, NULL,
                                       "background-gdk",
                                       &user->gui_user.colors[i], NULL);
    }

    user->gui_user.g_underline_tag =
            gtk_text_buffer_create_tag(user->gui_user.g_buffer, NULL,
                                       "underline",
                                       PANGO_UNDERLINE_SINGLE, NULL);

    user->gui_user.g_italic_tag =
            gtk_text_buffer_create_tag(user->gui_user.g_buffer, NULL,
                                       "style",
                                       PANGO_STYLE_ITALIC, NULL);
}


void gui_set_window_title(USER *user)
{
    char buf[MAX_STRING];

    if (!user->gui_user.g_window)
    {
        return;
    }

    sprintf(buf, "[Id: %d] %s", user->id, user->session);
    gtk_window_set_title(GTK_WINDOW(user->gui_user.g_window), buf);
}


void gui_setup_user_window(USER *user)
{
    GtkWidget              *window;
    GtkTextBuffer          *buffer;
    GtkWidget              *view;
    GtkWidget              *scrollback;
    GtkTextMark            *mark_begin, *mark_end;
    GtkWidget              *scrolled_window;
    GtkWidget              *vbox;
    GtkWidget              *combo;
    GtkWidget              *entry;
    GtkWidget              *statusbar;
    GtkWidget              *vpaned;
    GtkAccelGroup          *accel_group;
    GtkItemFactory         *item_factory;
    GtkTextIter             iter;

    /* create the main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* setup event handlers for the window */
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gui_destroy_window_callback), user);
    g_signal_connect(G_OBJECT(window), "configure-event",
                     G_CALLBACK(gui_user_config_event), user);
    g_signal_connect(G_OBJECT(window), "key-press-event",
                     G_CALLBACK(gui_key_event_callback), user);

    gtk_container_set_border_width(GTK_CONTAINER(window), 2);
    gtk_window_set_default_size(GTK_WINDOW(window),
                                user->gui_user.win_width,
                                user->gui_user.win_height);

    /* create a vertical box */
    vbox = gtk_vbox_new(FALSE, 0);
    /* put the vertical box inside the window */
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* create the menubar */
    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
    g_object_unref(accel_group);

    /* set up item factory to go away with the window */
    g_object_ref(item_factory);
    gtk_object_sink(GTK_OBJECT(item_factory));
    g_object_set_data_full(G_OBJECT(window), "<main>",
                           item_factory, (GDestroyNotify)g_object_unref);

    /* create menu items */
    gtk_item_factory_create_items(item_factory, G_N_ELEMENTS(menu_items), menu_items, window);

     /* put the menu in the vertical box */
    gtk_box_pack_start(GTK_BOX(vbox),
                       gtk_item_factory_get_widget(item_factory, "<main>"),
                       FALSE, FALSE, 1);

    /* create a paned window for scrollback */
    vpaned = gtk_vpaned_new();
    gtk_container_add(GTK_CONTAINER(vbox), vpaned);

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

    /* add the normal view to the 2nd position of the paned */
    gtk_paned_add2(GTK_PANED(vpaned), scrolled_window);

    /* set the view properties */
    gui_set_view_window(user, view);

    /* get a pointer to the buffer within the view */
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    /* create a text view for the scrollback */
    scrollback = gtk_text_view_new_with_buffer(buffer);

    /* create a scrolled window */
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_ALWAYS);
    /* put the text view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(scrolled_window), scrollback);

    /* add the scrollback view to the 1st position of the paned */
    gtk_paned_add1(GTK_PANED(vpaned), scrolled_window);

    /* set the view properties */
    gui_set_view_window(user, scrollback);

    /* set the position of the divider */
    gtk_paned_set_position(GTK_PANED(vpaned), user->gui_user.win_height/2);

    /* set the begin and the end mark of the buffer */
    gtk_text_buffer_get_start_iter(buffer, &iter);
    mark_begin = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

    gtk_text_buffer_get_end_iter(buffer, &iter);
    mark_end = gtk_text_buffer_create_mark(buffer, NULL, &iter, FALSE);

    /* create an entry widget */
    combo = gtk_combo_new();
    entry = GTK_COMBO(combo)->entry;
    gtk_combo_disable_activate(GTK_COMBO(combo));
    gtk_entry_set_max_length(GTK_ENTRY(entry), MAX_INPUT_LEN-1);

    /* set up event handlers for the input bar */
    g_signal_connect(G_OBJECT(entry), "activate",
                     G_CALLBACK(gui_enter_callback), user);
    g_signal_connect(G_OBJECT(entry), "focus-in-event",
                     G_CALLBACK(gui_user_focus_in), user);
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
                     G_CALLBACK(gui_user_focus_out), user);

    /* also put the entry inside the vertical box */
    gtk_box_pack_start(GTK_BOX(vbox), combo, FALSE, FALSE, 1);

    /* create the statusbar */
    statusbar = gtk_statusbar_new();

    /* also put the status inside the vertical box */
    gtk_box_pack_start(GTK_BOX(vbox), statusbar, FALSE, FALSE, 1);

    /* get the end iter which will point to the insertion point */
    gtk_text_buffer_get_end_iter(buffer, &user->gui_user.input_iter);

    /* set up some useful pointers in the user structure */
    user->gui_user.g_window     = window;
    user->gui_user.g_view       = view;
    user->gui_user.g_buffer     = buffer;
    user->gui_user.g_input      = entry;
    user->gui_user.g_combo      = combo;
    user->gui_user.g_status     = statusbar;
    user->gui_user.g_mark       = mark_end;
    user->gui_user.g_scrollback = GTK_PANED(vpaned)->child1;

    /* set the focus to the entry bar only */
    g_object_set(view, "can-focus", FALSE, NULL);
    g_object_set(scrollback, "can-focus", FALSE, NULL);
    g_object_set(statusbar, "can-focus", FALSE, NULL);
    gtk_widget_grab_focus(entry);

    /* set the fonts in the windows */
    gui_font_update(user, FONT_USER_WINDOW);
    gui_font_update(user, FONT_USER_SCROLLBACK);
    gui_font_update(user, FONT_USER_INPUT);
    gui_font_spacing_update(user);

    /* move window to saved position */
    if (user->gui_user.win_pos_x && user->gui_user.win_pos_y)
    {
        int x = user->gui_user.win_pos_x - X_COMPENSATION;
        int y = user->gui_user.win_pos_y - Y_COMPENSATION;

        if (x < 0)
        {
            x = 0;
        }

        if (y < 0)
        {
            y = 0;
        }

        gtk_window_move(GTK_WINDOW(user->gui_user.g_window), x, y);
    }

    /* initially set the menu radio button for wrap mode */
    /* NOTE: This has to be called at this location or else error messages are generated
       as soon as the user window is created (bug in GTK?) */
    switch (user->gui_user.wrap_mode)
    {
        case GTK_WRAP_NONE:
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                gtk_item_factory_get_item(item_factory, "/View/General/Wrapping/None")), TRUE);
            break;
        case GTK_WRAP_CHAR:
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                gtk_item_factory_get_item(item_factory, "/View/General/Wrapping/Character")), TRUE);
            break;
        case GTK_WRAP_WORD:
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                gtk_item_factory_get_item(item_factory, "/View/General/Wrapping/Word")), TRUE);
            break;
          default:
            break;
    }

    /* set the title of the window */
    gui_set_window_title(user);

    /* show the window :-) */
    gtk_widget_show_all(window);

    /* but... hide the scrollback pane */
    gtk_widget_hide_all(scrolled_window);
}
