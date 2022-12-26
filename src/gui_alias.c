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
    COLUMN_ALIAS,       /* the alias column */
    COLUMN_RESPONSE,    /* response of the alias */
    COLUMN_POINTER,     /* a pointer to the alias - not visible */
    NUM_COLUMNS
} ALIAS_COLUMNS;


static void gui_alias_button_release(GtkButton *button, USER *user)
{
    const gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_NEW))
    {
        /* create a new alias */
        ALIAS       *alias;
        GtkWidget   *entry1;
        GtkWidget   *entry2;
        gchar       *input1;
        gchar       *input2;

        /* first get a pointer to the widgets involved */
        entry1 = gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_ALIASES), 0);
        entry2 = gui_get_nth_child(gui_get_wid_frame(user, 1, SEL_ALIASES), 0);

        /* then retrieve the data in the widgets */
        input1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry1));
        input2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry2));

        if (!input1[0] || !input2[0])
        {
            return;
        }

        /* create the alias */
        alias = create_alias(user, input1, input2);

        gui_alias_add_pref(user, alias);

        /* clear the entries */
        gtk_entry_set_text(GTK_ENTRY(entry1), "");
        gtk_entry_set_text(GTK_ENTRY(entry2), "");
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        /* delete the selected alias */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        ALIAS            *alias;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_ALIASES], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &alias, -1);

        /* remove the alias from the view */
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        free_alias(user, alias);
    }
    else if (!strcmp(label, BUTTON_APPLY))
    {
        /* apply the changes on the selected alias */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        ALIAS            *alias;
        GtkTreeIter       iter;
        gchar            *entry1;
        gchar            *entry2;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_ALIASES], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &alias, -1);

        /* first retrieve the entries */
        entry1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(
                          gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_ALIASES), 0)));
        entry2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(
                          gui_get_nth_child(gui_get_wid_frame(user, 1, SEL_ALIASES), 0)));

        if (!entry1[0] || !entry2[0])
        {
            return;
        }

        /* update the alias */
        free(alias->name);
        free(alias->string);

        alias->name = strdup(entry1);
        alias->string = strdup(entry2);

        /* set the alias view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COLUMN_ALIAS, alias->name,
                           COLUMN_RESPONSE, alias->string,
                           COLUMN_POINTER, alias, -1);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the alias view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


static void gui_alias_row_selected(GtkTreeView *treeview, USER *user)
{
    GtkTreeSelection *select;
    GtkTreeModel     *model;
    GtkWidget        *view;
    ALIAS            *alias;
    GtkTreeIter       iter;

    view   = gtk_bin_get_child(GTK_BIN(
             gui_get_nth_child(user->gui_pref.g_edit[SEL_ALIASES], 0)));
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &alias, -1);

    /* set the entries and spin button */
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(
                              gui_get_wid_frame(user, 0, SEL_ALIASES), 0)),
                              alias->name);
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(
                              gui_get_wid_frame(user, 1, SEL_ALIASES), 0)),
                              alias->string);
}


static GtkTreeModel *gui_alias_create_model(USER *user)
{
    GtkListStore *store;
    ALIAS        *alias;
    GtkTreeIter   iter;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
                               G_TYPE_STRING,
			       G_TYPE_STRING,
                               G_TYPE_POINTER);

    /* add data to the list store */
    for (alias = user->alias_list; alias; alias = alias->next)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_ALIAS, alias->name,
			   COLUMN_RESPONSE, alias->string,
                           COLUMN_POINTER, alias, -1);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_alias_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;

    /* column for alias */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Alias",
						      renderer,
						      "text",
						      COLUMN_ALIAS,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_ALIAS);
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


void gui_alias_add_pref(USER *user, ALIAS *alias)
{
    GtkListStore  *store;
    GtkWidget     *view;
    GtkTreeIter    iter;

    /* preference window not created yet? */
    if (!user->gui_pref.g_preference ||
        !user->gui_pref.g_edit[SEL_ALIASES])
    {
        return;
    }

    view = gtk_bin_get_child(GTK_BIN(
           gui_get_nth_child(user->gui_pref.g_edit[SEL_ALIASES], 0)));

    /* retrieve the store */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

    /* append the alias to the list */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_ALIAS,   alias->name,
                       COLUMN_RESPONSE,   alias->string,
                       COLUMN_POINTER, alias, -1);
}


GtkWidget *gui_alias_create(USER *user)
{
    GtkWidget       *view;
    GtkTreeModel    *model;
    GtkWidget       *bbox;
    GtkWidget       *hbox;
    GtkWidget       *mainbox;
    GtkWidget       *alias, *response;
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
    model = gui_alias_create_model(user);

    /* create tree view */
    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_ALIAS);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(model));

    g_signal_connect(G_OBJECT(view), "cursor-changed",
                     G_CALLBACK(gui_alias_row_selected), user);

    /* add columns to the tree view */
    gui_alias_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a hbox for the alias */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the alias entry */
    alias = gtk_entry_new();

    /* put alias in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), alias, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Alias");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(alias), MAX_INPUT_LEN-1);

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
                     G_CALLBACK(gui_alias_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_alias_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_alias_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_alias_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_APPLY);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_alias_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_alias_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_alias_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_alias_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
