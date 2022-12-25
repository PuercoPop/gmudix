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
#define BUTTON_EXIT     "E_xit"

typedef enum
{
    COLUMN_TAB,         /* the tab column */
    COLUMN_POINTER,     /* a pointer to the tab - not visible */
    NUM_COLUMNS
} TAB_COLUMNS;


static void gui_tab_button_release(GtkButton *button, USER *user)
{
    G_CONST_RETURN gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_NEW))
    {
        /* create a new tab */
        TAB         *tab;
        GtkWidget   *entry1;
        gchar       *input1;

        /* first get a pointer to the widgets involved */
        entry1 = gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_TABS), 0);

        /* then retrieve the data in the widgets */
        input1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry1));

        if (!input1[0])
        {
            return;
        }

        /* create the tab */
        tab = create_tab(user, input1);

        gui_tab_add_pref(user, tab);

        /* clear the entries */
        gtk_entry_set_text(GTK_ENTRY(entry1), "");
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        /* delete the selected tab */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        TAB              *tab;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TABS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &tab, -1);

        /* remove the tab from the view */
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        free_tab(user, tab);
    }
    else if (!strcmp(label, BUTTON_APPLY))
    {
        /* apply the changes on the selected tab */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        TAB              *tab;
        GtkTreeIter       iter;
        gchar            *entry1;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_TABS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &tab, -1);

        /* first retrieve the entries */
        entry1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(
                          gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_TABS), 0)));

        if (!entry1[0])
        {
            return;
        }

        /* update the tab */
        free(tab->name);

        tab->name = strdup(entry1);

        /* set the tab view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COLUMN_TAB, tab->name,
                           COLUMN_POINTER, tab, -1);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the tab view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


static void gui_tab_row_selected(GtkTreeView *treeview, USER *user)
{
    GtkTreeSelection *select;
    GtkTreeModel     *model;
    GtkWidget        *view;
    TAB              *tab;
    GtkTreeIter       iter;

    view   = gtk_bin_get_child(GTK_BIN(
             gui_get_nth_child(user->gui_pref.g_edit[SEL_TABS], 0)));
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &tab, -1);

    /* set the entries and spin button */
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(
                              gui_get_wid_frame(user, 0, SEL_TABS), 0)),
                              tab->name);
}


static GtkTreeModel *gui_tab_create_model(USER *user)
{
    GtkListStore *store;
    TAB          *tab;
    GtkTreeIter   iter;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
			       G_TYPE_STRING,
                               G_TYPE_POINTER);

    /* add data to the list store */
    for (tab = user->tabs_list; tab; tab = tab->next)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_TAB, tab->name,
                           COLUMN_POINTER, tab, -1);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_tab_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;

    /* column for tab */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Tab Completion",
						      renderer,
						      "text",
						      COLUMN_TAB,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_TAB);
    gtk_tree_view_append_column(treeview, column);
}


void gui_tab_add_pref(USER *user, TAB *tab)
{
    GtkListStore  *store;
    GtkWidget     *view;
    GtkTreeIter    iter;

    /* preference window not created yet? */
    if (!user->gui_pref.g_preference ||
        !user->gui_pref.g_edit[SEL_TABS])
    {
        return;
    }

    view = gtk_bin_get_child(GTK_BIN(
           gui_get_nth_child(user->gui_pref.g_edit[SEL_TABS], 0)));

    /* retrieve the store */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

    /* append the tab to the list */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_TAB,   tab->name,
                       COLUMN_POINTER, tab, -1);
}


GtkWidget *gui_tab_create(USER *user)
{
    GtkWidget       *view;
    GtkTreeModel    *model;
    GtkWidget       *bbox;
    GtkWidget       *hbox;
    GtkWidget       *mainbox;
    GtkWidget       *tab;
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
    model = gui_tab_create_model(user);

    /* create tree view */
    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_TAB);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(model));

    g_signal_connect(G_OBJECT(view), "cursor-changed",
                     G_CALLBACK(gui_tab_row_selected), user);

    /* add columns to the tree view */
    gui_tab_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a hbox for the tab */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the tab entry */
    tab = gtk_entry_new();

    /* put tab in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), tab, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Tab Completion");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(tab), MAX_INPUT_LEN-1);

    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_NEW);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_tab_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_tab_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_tab_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_tab_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_APPLY);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_tab_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_tab_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_tab_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_tab_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
