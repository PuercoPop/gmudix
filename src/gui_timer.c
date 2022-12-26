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


/* the possible button labels */
#define BUTTON_NEW      "_New"
#define BUTTON_DELETE   "_Delete"
#define BUTTON_APPLY    "_Apply"
#define BUTTON_CONTINUE "_Continue"
#define BUTTON_STOP     "_Stop"
#define BUTTON_SYNC     "S_ync"
#define BUTTON_EXIT     "E_xit"

typedef enum
{
    COLUMN_TIMER,       /* the time column */
    COLUMN_RELOAD,      /* reload count column */
    COLUMN_STATUS,      /* the status column */
    COLUMN_RESPONSE,    /* response when the timer expires */
    COLUMN_POINTER,     /* a pointer to the timer - not visible */
    NUM_COLUMNS
} TIMER_COLUMNS;


static void gui_timer_button_release(GtkButton *button, USER *user)
{
    const gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_NEW))
    {
        /* create a new timer */
        TIMER       *timer;
        GtkWidget   *entry;
        GtkWidget   *spin1, *spin2;
        gchar       *input;
        gint         val1, val2;

        /* first get a pointer to the widgets involved */
        spin1 = gui_get_nth_child(gtk_bin_get_child(GTK_BIN(
                gui_get_nth_child(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 1), 0))), 0);
        spin2 = gui_get_nth_child(gtk_bin_get_child(GTK_BIN(
                gui_get_nth_child(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 1), 1))), 0);
        entry = gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_TIMERS), 0);

        /* then retrieve the data in the widgets */
        input = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry));
        val1  = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin1));
        val2  = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin2));

        if (!input[0] || !val1)
        {
            return;
        }

        /* create the timer */
        timer = create_timer(user, input, val1, val2);

        gui_timer_add_pref(user, timer);

        /* clear the entries and spins */
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin1), 0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin2), 0);
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        /* delete the selected timer */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        TIMER            *timer;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);

        /* remove the timer from the view */
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        free_timer(user, timer);
    }
    else if (!strcmp(label, BUTTON_APPLY))
    {
        /* apply the changes on the selected timer */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        GtkWidget        *entry;
        GtkWidget        *spin1, *spin2;
        TIMER            *timer;
        GtkTreeIter       iter;
        gchar            *input;
        gint              val1, val2;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);

        /* first get a pointer to the widgets involved */
        spin1 = gui_get_nth_child(gtk_bin_get_child(GTK_BIN(
                gui_get_nth_child(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 1), 0))), 0);
        spin2 = gui_get_nth_child(gtk_bin_get_child(GTK_BIN(
                gui_get_nth_child(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 1), 1))), 0);
        entry = gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_TIMERS), 0);

        /* then retrieve the data in the widgets */
        input = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry));
        val1  = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin1));
        val2  = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin2));

        if (!input[0] || !val1)
        {
            return;
        }

        /* update the timer */
        set_timer(user, timer, input, val1, val2);

        /* set the timer view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           COLUMN_TIMER, timer->relval,
                           COLUMN_RELOAD, timer->relcnt,
                           COLUMN_STATUS, !timer->timer? "Stopped": "Running",
			   COLUMN_RESPONSE, timer->response,
                           COLUMN_POINTER, timer, -1);
    }
    else if (!strcmp(label, BUTTON_STOP))
    {
        /* stop the selected timer */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        TIMER            *timer;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);

        /* just set the timer to 0 to stop it */
        timer->timer = 0;

        /* set the timer view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           COLUMN_TIMER, timer->relval,
                           COLUMN_RELOAD, timer->relcnt,
                           COLUMN_STATUS, !timer->timer? "Stopped": "Running",
			   COLUMN_RESPONSE, timer->response,
                           COLUMN_POINTER, timer, -1);
    }
    else if (!strcmp(label, BUTTON_CONTINUE))
    {
        /* continue the selected timer */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        TIMER            *timer;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);

        /* just reload the timer to make it continue */
        timer->timer = timer->relval;

        /* set the timer view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           COLUMN_TIMER, timer->relval,
                           COLUMN_RELOAD, timer->relcnt,
                           COLUMN_STATUS, !timer->timer? "Stopped": "Running",
			   COLUMN_RESPONSE, timer->response,
                           COLUMN_POINTER, timer, -1);
    }
    else if (!strcmp(label, BUTTON_SYNC))
    {
        /* just sync the timer */
        sync_timer(user);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the timer view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


static void gui_timer_row_selected(GtkTreeView *treeview, USER *user)
{
    GtkTreeSelection *select;
    GtkTreeModel     *model;
    GtkWidget        *view;
    TIMER            *timer;
    GtkTreeIter       iter;

    view   = gtk_bin_get_child(GTK_BIN(
             gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);

    /* set the entries and spin button */
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui_get_nth_child(
        gtk_bin_get_child(GTK_BIN(gui_get_nth_child(gui_get_nth_child(
        user->gui_pref.g_edit[SEL_TIMERS], 1), 0))), 0)), timer->relval);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(gui_get_nth_child(
        gtk_bin_get_child(GTK_BIN(gui_get_nth_child(gui_get_nth_child(
        user->gui_pref.g_edit[SEL_TIMERS], 1), 1))), 0)), timer->relcnt);
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(gui_get_wid_frame(
        user, 0, SEL_TIMERS), 0)), timer->response);
}


static GtkTreeModel *gui_timer_create_model(USER *user)
{
    GtkListStore *store;
    TIMER        *timer;
    GtkTreeIter   iter;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
                               G_TYPE_INT,
                               G_TYPE_INT,
                               G_TYPE_STRING,
			       G_TYPE_STRING,
                               G_TYPE_POINTER);

    /* add data to the list store */
    for (timer = user->timer_list; timer; timer = timer->next)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_TIMER, timer->relval,
                           COLUMN_RELOAD, timer->relcnt,
                           COLUMN_STATUS, !timer->timer? "Stopped": "Running",
			   COLUMN_RESPONSE, timer->response,
                           COLUMN_POINTER, timer, -1);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_timer_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;

    /* column for timer */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Timer",
						      renderer,
						      "text",
						      COLUMN_TIMER,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_TIMER);
    gtk_tree_view_append_column(treeview, column);

    /* column for reload */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Reloads",
						      renderer,
						      "text",
						      COLUMN_RELOAD,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_RELOAD);
    gtk_tree_view_append_column(treeview, column);

    /* column for status */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Status",
						      renderer,
						      "text",
						      COLUMN_STATUS,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_STATUS);
    gtk_tree_view_append_column(treeview, column);

    /* column for response */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Response",
						      renderer,
						      "text",
						      COLUMN_RESPONSE,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_RESPONSE);
    gtk_tree_view_append_column(treeview, column);
}


static gboolean gui_timers_row_update(GtkTreeModel *model,
                                      GtkTreePath  *treepath,
                                      GtkTreeIter  *iter,
                                      USER         *user)
{
    TIMER *timer;

    gtk_tree_model_get(model, iter, COLUMN_POINTER, &timer, -1);

    /* update the row */
    gtk_list_store_set(GTK_LIST_STORE(model), iter,
                       COLUMN_TIMER, timer->relval,
                       COLUMN_RELOAD, timer->relcnt,
                       COLUMN_STATUS, !timer->timer? "Stopped": "Running",
                       COLUMN_RESPONSE, timer->response,
                       COLUMN_POINTER, timer, -1);

    return FALSE;   /* continue with next row */
}


void gui_timer_remove(USER *user, TIMER *search)
{
    TIMER *timer;

    /* only do so if the timer window is visible */
    if (user->gui_pref.g_edit[SEL_TIMERS])
    {
        GtkTreeModel *model;
        GtkTreeIter   iter;

        /* retrieve the model */
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(
                gtk_bin_get_child(GTK_BIN(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 0)))));

        if (gtk_tree_model_get_iter_first(model, &iter))
        {
            do
            {
                gtk_tree_model_get(model, &iter, COLUMN_POINTER, &timer, -1);
                if (timer == search)
                {
                    /* remove from the model and free it */
                    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
                    break;
                }
            } while (gtk_tree_model_iter_next(model, &iter));
        }
    }
}


void gui_timer_update_list(USER *user)
{
    /* call the row update routine for each row */
    if (user->gui_pref.g_edit[SEL_TIMERS])
    {
        GtkTreeModel *model;

        /* retrieve the model */
        model = gtk_tree_view_get_model(GTK_TREE_VIEW(
                gtk_bin_get_child(GTK_BIN(gui_get_nth_child(
                user->gui_pref.g_edit[SEL_TIMERS], 0)))));

        gtk_tree_model_foreach(model,
                               (GtkTreeModelForeachFunc)gui_timers_row_update,
                               user);
    }
}


void gui_timer_add_pref(USER *user, TIMER *timer)
{
    GtkListStore  *store;
    GtkWidget     *view;
    GtkTreeIter    iter;

    /* preference window not created yet? */
    if (!user->gui_pref.g_preference ||
        !user->gui_pref.g_edit[SEL_TIMERS])
    {
        return;
    }

    view = gtk_bin_get_child(GTK_BIN(
           gui_get_nth_child(user->gui_pref.g_edit[SEL_TIMERS], 0)));

    /* retrieve the store */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

    /* append the timer to the list */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_TIMER, timer->relval,
                       COLUMN_RELOAD, timer->relcnt,
                       COLUMN_STATUS, !timer->timer? "Stopped": "Running",
                       COLUMN_RESPONSE, timer->response,
                       COLUMN_POINTER, timer, -1);
}


GtkWidget *gui_timer_create(USER *user)
{
    GtkWidget       *view;
    GtkTreeModel    *model;
    GtkWidget       *bbox;
    GtkWidget       *hbox;
    GtkWidget       *mainbox;
    GtkWidget       *timer, *reload, *response;
    GtkWidget       *button;
    GtkWidget       *frame;
    GtkWidget       *sw;

    /* create a vertical box where the complete editor is inside */
    mainbox = gtk_vbox_new(FALSE, 2);

    /* create a scroll window where the edit view is placed in */
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    /* create tree model */
    model = gui_timer_create_model(user);

    /* create tree view */
    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_TIMER);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(model));

    g_signal_connect(G_OBJECT(view), "cursor-changed",
                     G_CALLBACK(gui_timer_row_selected), user);

    /* add columns to the tree view */
    gui_timer_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a hbox for both the timer and the reload spin */
    bbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 0);

    /* create a hbox for the timer */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the timer spin */
    timer = gtk_spin_button_new_with_range(0, 100000, 1);

    /* put timer in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), timer, TRUE, TRUE, 0);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Timer (seconds)");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(bbox), frame, TRUE, TRUE, 0);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(timer), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(timer), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(timer), 0);

    /* create a hbox for the reload */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the reload spin */
    reload = gtk_spin_button_new_with_range(-1, 100000, 1);

    /* put reload in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), reload, TRUE, TRUE, 0);

    /* put the spin with a frame inside the mainbox */
    frame = gtk_frame_new("Reload Count");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(bbox), frame, TRUE, TRUE, 0);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(reload), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(reload), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(reload), 0);

    button = gtk_button_new_with_label(BUTTON_CONTINUE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 4);

    button = gtk_button_new_with_label(BUTTON_STOP);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 4);

    button = gtk_button_new_with_label(BUTTON_SYNC);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 4);

    /* create a hbox for the response */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the response entry */
    response = gtk_entry_new();

    /* put response in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), response, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Response");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(response), MAX_INPUT_LEN-1);

    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_NEW);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_APPLY);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_timer_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_timer_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
