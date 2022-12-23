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


typedef struct
{
          char      *name;
          int        depth;
    const gchar     *path;
          GtkWidget *(*gui_callback)(USER *user); 
} SELECT_STRUCT;


/* depth and path are linked together of course, if you add a depth
   usually you'd also add a ':' in path ;) */
static SELECT_STRUCT select_data[NUM_SELECT] =
{
    /* name */          /* depth */     /* path */      /* creation */
    {"General",         0,              "0",            gui_general_create},
    {"Characters",      1,              "0:0",          gui_character_create},
    {"Colors",          1,              "0:1",          gui_color_create},
    {"Fonts",           1,              "0:2",          gui_font_create},
    {"Aliases",         0,              "1",            gui_alias_create},
    {"Macros",          0,              "2",            gui_macro_create},
    {"Paths",           0,              "3",            gui_path_create},
    {"Tab completions", 0,              "4",            gui_tab_create},
    {"Timers",          0,              "5",            gui_timer_create},
    {"Triggers",        0,              "6",            gui_trigger_create},
    {"Variables",       0,              "7",            gui_var_create}
};


static void gui_destroy_window_callback(GtkObject *window, USER *user)
{
    SELECT_TREE i;

    /* remove the window pointers from the user data */
    user->gui_pref.g_preference = NULL;
    user->gui_pref.g_selecttree = NULL;

    for (i=0; i<NUM_SELECT; i++)
    {
        user->gui_pref.g_edit[i] = NULL;
    }

    /* if there's no user window, then EDIT was called from the main window */
    if (!user->gui_user.g_window)
    {
        /* first save the user */
        save_user(user);

        /* then call the main window updater */
        gui_main_update_row(user);

        /* and finally just delete the user */
        del_user(user);
    }
}


static gboolean gui_pref_config_event(GtkWidget         *widget,
                                      GdkEventConfigure *event,
                                      USER              *user)
{
    /* just store our new size so we can save it later */
    if (widget == user->gui_pref.g_preference)
    {
        user->gui_pref.pref_height = event->height;
        user->gui_pref.pref_width  = event->width;
    }

    /* FALSE passes the event through into the widget */
    return FALSE;
}


static void gui_selection_changed(GtkTreeSelection *selection, USER *user)
{
    GtkWidget           *box;
    GtkTreePath         *path;
    gchar               *path_str;
    GtkTreeIter          iter;
    SELECT_TREE          i;

    /* retrieve the path-string of the selected row */
    gtk_tree_selection_get_selected(selection, NULL, &iter);
    path = gtk_tree_model_get_path(
           gtk_tree_view_get_model(GTK_TREE_VIEW(user->gui_pref.g_selecttree)),
           &iter);
    path_str = gtk_tree_path_to_string(path);

    box = gtk_bin_get_child(GTK_BIN(user->gui_pref.g_preference));

    /* compare the path_string with our paths */
    for (i=0; i<NUM_SELECT; i++)
    {
        /* create the new view if this is the selected view and
           if it's not yet created */
        if (!strcmp(path_str, select_data[i].path))
        {
            GtkWidget *temp;

            if (user->gui_pref.g_edit[i])
            {
                /* just show if view already exists */
                gtk_widget_show_all(user->gui_pref.g_edit[i]);
                continue;
            }

            /* call the creation function for this view */
            temp = (*select_data[i].gui_callback)(user);

            if (temp)
            {
                /* add the returned widget to the window */
                gtk_container_add(GTK_CONTAINER(box), temp);
            }

            user->gui_pref.g_edit[i] = temp;
            gtk_widget_show_all(user->gui_pref.g_edit[i]);
        }
        else if (user->gui_pref.g_edit[i])
        {
            gtk_widget_hide_all(user->gui_pref.g_edit[i]);
            gtk_widget_destroy(user->gui_pref.g_edit[i]);

            user->gui_pref.g_edit[i] = NULL;
        }

    }

    /* clean up */
    gtk_tree_path_free(path);
    g_free(path_str);
}


static GtkWidget *setup_selection_tree(USER *user, SELECT_TREE select)
{
    GtkWidget           *treeview;
    GtkTreeStore        *store;
    GtkTreeViewColumn   *column;
    GtkCellRenderer     *renderer;
    GtkTreeIter          iter, parent;
    int                  i, cur_depth;

    /* create a model */
    store = gtk_tree_store_new(1, G_TYPE_STRING);

    /* now set up the data in the tree */
    for (i=0, cur_depth=0; i<NUM_SELECT; i++)
    {
        /* root parent */
        if (select_data[i].depth == 0)
        {
            gtk_tree_store_append(store, &iter, NULL);
            parent = iter;
        }
        /* going down a depth or depth is same */
        else if (cur_depth >= select_data[i].depth)
        {
            /* as long as depth is > than 1 and higher than new depth */
            while (cur_depth > select_data[i].depth && cur_depth > 1)
            {
                /* grab the previous parent */
                gtk_tree_model_iter_parent(GTK_TREE_MODEL(store),
                                           &parent,
                                           &parent);
                cur_depth--;
            }
            gtk_tree_store_append(store, &iter, &parent);
        }
        /* depth is increasing */
        else
        {
            parent = iter;
            gtk_tree_store_append(store, &iter, &parent);
        }

        gtk_tree_store_set(store, &iter, 0, select_data[i].name, -1);

        /* update depth */
        cur_depth = select_data[i].depth;
    }

    /* create a view */
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    /* the view now holds a reference, unref ours */
    g_object_unref(G_OBJECT(store));

    /* disable the focus on the treeview (to get rid of annoying select edge) */
    g_object_set(treeview, "can-focus", FALSE, NULL);

    /* create a cell renderer that displays text */
    renderer = gtk_cell_renderer_text_new();

    /* create a column, associating the "text" attribute of the
       cell_renderer to the first column of the model */
    column = gtk_tree_view_column_new_with_attributes(
                        "Selection", renderer, "text", 0, NULL);

    /* add the column to the view */
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    /* set up the callback handler on the selection */
    g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview))), 
                     "changed", G_CALLBACK(gui_selection_changed), user);

    return treeview;
}


static void gui_preference_select(GtkTreeView *treeview, SELECT_TREE select)
{
    GtkTreePath         *path;
    GtkTreeSelection    *selection;

    path = gtk_tree_path_new_from_string(select_data[select].path);

    /* move up to root */
    while (gtk_tree_path_up(path))
    {
        /* expand the parents in this path up to root */
        gtk_tree_view_expand_row(GTK_TREE_VIEW(treeview), path, FALSE);
    }
    
    /* clean up */
    gtk_tree_path_free(path);

    /* select the row */
    path      = gtk_tree_path_new_from_string(select_data[select].path);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    gtk_tree_selection_select_path(selection, path);

    /* clean up */
    gtk_tree_path_free(path);
}


void gui_preference_editor(USER *user, SELECT_TREE select)
{
    GtkWidget       *window;
    GtkWidget       *treeview;
    GtkWidget       *mainbox;
    GtkWidget       *sw;

    if (user->gui_pref.g_preference)
    {
        /* the window is already created! make it visible */
        gtk_window_present(GTK_WINDOW(user->gui_pref.g_preference));
        gui_preference_select(GTK_TREE_VIEW(user->gui_pref.g_selecttree), select);
        return;
    }

    /* create window, etc */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Preferences Editor");

    /* setup some handlers for the window */
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK(gui_destroy_window_callback), user);
    g_signal_connect(G_OBJECT(window), "configure-event",
		     G_CALLBACK(gui_pref_config_event), user);

    /* set default border width */
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    mainbox = gtk_hbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(window), mainbox);

    /* window is split up in 2 windows, left and right */
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(mainbox), sw, FALSE, FALSE, 0);

    /* create the left view */
    treeview = setup_selection_tree(user, select);

    /* add the tree selector to a scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), treeview);

    /* copy the window and treeview into the user structure */
    user->gui_pref.g_preference = window;
    user->gui_pref.g_selecttree = treeview;

    /* set the default window size */
    gtk_window_set_default_size(GTK_WINDOW(window), user->gui_pref.pref_width,
                                                    user->gui_pref.pref_height);

    /* 
     * show the window - at this point the treeview selects the first row
     * IF we selected a preference through the menu :-(
     * therefore we select a preference AFTER showing the window
     */
    gtk_widget_show_all(window);

    /* select the selected preference */
    gui_preference_select(GTK_TREE_VIEW(treeview), select);
}
