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
    COLUMN_MACRO,       /* the macro column */
    COLUMN_RESPONSE,    /* response for the macro */
    COLUMN_POINTER,     /* a pointer to the alias - not visible */
    NUM_COLUMNS
} MACRO_COLUMNS;


static gboolean gui_macro_key(GtkWidget    *widget,
                              GdkEventKey  *event,
                              GtkWidget    *entry)
{
    gchar *str;

    /* convert the keycode to a readable string */
    str = gtk_accelerator_name(event->keyval, event->state);

    /* set both entries */
    gtk_entry_set_text(GTK_ENTRY(widget), str);

    /* free temporary string */
    g_free(str);

    return TRUE;
}


static void gui_macro_button_release(GtkButton *button, USER *user)
{
    const gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_NEW))
    {
        /* create a new macro */
        MACRO       *macro;
        GtkWidget   *entry1;
        GtkWidget   *entry2;
        gchar       *input1;
        gchar       *input2;
        guint        keyval;
        guint        state;

        /* first get a pointer to the widgets involved */
        entry1 = gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_MACROS), 0);
        entry2 = gui_get_nth_child(gui_get_wid_frame(user, 1, SEL_MACROS), 0);

        /* then retrieve the data in the widgets */
        input1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry1));
        input2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry2));

        gtk_accelerator_parse(input1, &keyval, &state);

        if (!input1[0] ||
            !input2[0] ||
            !keyval ||
            macro_lookup(user, keyval, state))
        {
            return;
        }

        /* create the macro */
        macro = create_macro(user, keyval, state, input2);

        gui_macro_add_pref(user, macro);

        /* clear the entries */
        gtk_entry_set_text(GTK_ENTRY(entry1), "");
        gtk_entry_set_text(GTK_ENTRY(entry2), "");
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        /* delete the selected macro */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        MACRO            *macro;
        GtkTreeIter       iter;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_MACROS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &macro, -1);

        /* remove the macro from the view */
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        free_macro(user, macro);
    }
    else if (!strcmp(label, BUTTON_APPLY))
    {
        /* apply the changes on the selected macro */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        GtkWidget        *view;
        MACRO            *macro;
        GtkTreeIter       iter;
        gchar            *entry1;
        gchar            *entry2;
        guint             keyval;
        guint             state;

        view   = gtk_bin_get_child(GTK_BIN(
                 gui_get_nth_child(user->gui_pref.g_edit[SEL_MACROS], 0)));
        select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &macro, -1);

        /* first retrieve the entries */
        entry1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(
                          gui_get_nth_child(gui_get_wid_frame(user, 0, SEL_MACROS), 0)));
        entry2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(
                          gui_get_nth_child(gui_get_wid_frame(user, 1, SEL_MACROS), 0)));

        gtk_accelerator_parse(entry1, &keyval, &state);

        if (!entry1[0] || !entry2[0] || !keyval)
        {
            return;
        }

        /* update the macro */
        free(macro->text);

        macro->key   = keyval;
        macro->state = state;
        macro->text  = strdup(entry2);

        /* set the macro view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			   COLUMN_MACRO, entry1,
                           COLUMN_RESPONSE, macro->text,
                           COLUMN_POINTER, macro, -1);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the alias view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


static void gui_macro_row_selected(GtkTreeView *treeview, USER *user)
{
    GtkTreeSelection *select;
    GtkTreeModel     *model;
    GtkWidget        *view;
    MACRO            *macro;
    GtkTreeIter       iter;
    gchar            *str;

    view   = gtk_bin_get_child(GTK_BIN(
             gui_get_nth_child(user->gui_pref.g_edit[SEL_MACROS], 0)));
    select = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &macro, -1);

    /* convert key codes to a readable string */
    str = gtk_accelerator_name(macro->key, macro->state);

    /* set the entries */
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(
                              gui_get_wid_frame(user, 0, SEL_MACROS), 0)),
                              str);
    gtk_entry_set_text(GTK_ENTRY(gui_get_nth_child(
                              gui_get_wid_frame(user, 1, SEL_MACROS), 0)),
                              macro->text);

    /* free the temporary string */
    g_free(str);
}


static GtkTreeModel *gui_macro_create_model(USER *user)
{
    GtkListStore *store;
    MACRO        *macro;
    GtkTreeIter   iter;
    gchar        *str;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
                               G_TYPE_STRING,
			       G_TYPE_STRING,
                               G_TYPE_POINTER);

    /* add data to the list store */
    for (macro = user->macro_list; macro; macro = macro->next)
    {
        /* convert key codes to a readable string */
        str = gtk_accelerator_name(macro->key, macro->state);

        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_MACRO, str,
			   COLUMN_RESPONSE, macro->text,
                           COLUMN_POINTER, macro, -1);

        /* free the temporary string */
        g_free(str);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_macro_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;

    /* column for macro */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Key",
						      renderer,
						      "text",
						      COLUMN_MACRO,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_MACRO);
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


void gui_macro_add_pref(USER *user, MACRO *macro)
{
    GtkListStore  *store;
    GtkWidget     *view;
    GtkTreeIter    iter;
    gchar         *str;

    /* preference window not created yet? */
    if (!user->gui_pref.g_preference ||
        !user->gui_pref.g_edit[SEL_MACROS])
    {
        return;
    }

    view = gtk_bin_get_child(GTK_BIN(
           gui_get_nth_child(user->gui_pref.g_edit[SEL_MACROS], 0)));

    /* retrieve the store */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

    /* convert key codes to a readable string */
    str = gtk_accelerator_name(macro->key, macro->state);

    /* append the macro to the list */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_MACRO, str,
                       COLUMN_RESPONSE, macro->text,
                       COLUMN_POINTER, macro, -1);

    /* free the temporary string */
    g_free(str);
}


GtkWidget *gui_macro_create(USER *user)
{
    GtkWidget       *view;
    GtkTreeModel    *model;
    GtkWidget       *bbox;
    GtkWidget       *hbox;
    GtkWidget       *mainbox;
    GtkWidget       *macro, *response;
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
    model = gui_macro_create_model(user);

    /* create tree view */
    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_MACRO);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(model));

    g_signal_connect(G_OBJECT(view), "cursor-changed",
                     G_CALLBACK(gui_macro_row_selected), user);

    /* add columns to the tree view */
    gui_macro_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a hbox for the macro */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the macro entry */
    macro = gtk_entry_new();

    /* connect a key event handler to the entry */
    g_signal_connect(G_OBJECT(macro), "key-press-event",
                     G_CALLBACK(gui_macro_key), user);

   /* put macro in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), macro, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Key Sequence");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(macro), MAX_INPUT_LEN-1);

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
                     G_CALLBACK(gui_macro_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_macro_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_macro_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_macro_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_APPLY);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_macro_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_macro_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_macro_button_release), user);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_macro_button_release), user);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
