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
#define BUTTON_LOGIN    "_Login Level"
#define BUTTON_PASSWORD "_Password Level"
#define BUTTON_EXIT     "E_xit"

typedef enum
{
    COLUMN_ENABLE,      /* the enable toggle */
    COLUMN_LEVEL,       /* level of the trigger */
    COLUMN_TRIGGER,     /* trigger text */
    COLUMN_POINTER,     /* a pointer to the trigger - not visible */
    NUM_COLUMNS
} TRIGGER_COLUMNS;


static GtkTreeView *gui_trigger_get_treeview(USER *user)
{
    GtkWidget *widget = gtk_bin_get_child(GTK_BIN(gui_get_nth_child(
                        user->gui_pref.g_edit[SEL_TRIGGERS], 0)));

    return GTK_TREE_VIEW(widget);
}


static void gui_trigger_button_release(GtkButton *button, USER *user)
{
    G_CONST_RETURN gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_NEW))
    {
        /* create a new trigger */
        TRIGGER      *trigger;
        GtkWidget    *spin;
        GtkWidget    *entry1;
        GtkWidget    *entry2;
        gchar        *input1;
        gchar        *input2;
        gdouble       level;

        /* first get a pointer to the widgets involved */
        entry1   = user->gui_trigger.entry1;
        entry2   = user->gui_trigger.entry2;
        spin     = user->gui_trigger.spin;

        /* then retrieve the data in the widgets */
        input1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry1));
        input2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(entry2));
        level  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));

        if (!input1[0] || !input2[0])
        {
            return;
        }

        /* create the trigger */
        trigger = create_trigger(user, (gint)level, input1, input2);

        gui_trigger_add_pref(user, trigger);

        /* clear the entries and set the spin back to 0 */
        gtk_entry_set_text(GTK_ENTRY(entry1), "");
        gtk_entry_set_text(GTK_ENTRY(entry2), "");
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 0);
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        /* delete the selected trigger */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        TRIGGER          *trigger;
        GtkTreeIter       iter;

        select = gtk_tree_view_get_selection(gui_trigger_get_treeview(user));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(gui_trigger_get_treeview(user));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &trigger, -1);

        /* remove the trigger from the view */
        gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

        free_trigger(user, trigger);
    }
    else if (!strcmp(label, BUTTON_APPLY))
    {
        /* apply the changes on the selected trigger */
        GtkTreeSelection *select;
        GtkTreeModel     *model;
        TRIGGER          *trigger;
        GtkTreeIter       iter;
        gchar            *entry1;
        gchar            *entry2;
        gdouble           level;

        select = gtk_tree_view_get_selection(gui_trigger_get_treeview(user));

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        model = gtk_tree_view_get_model(gui_trigger_get_treeview(user));
        gtk_tree_model_get(model, &iter, COLUMN_POINTER, &trigger, -1);

        /* first retrieve the entries */
        entry1 = (gchar *)gtk_entry_get_text(GTK_ENTRY(user->gui_trigger.entry1));
        entry2 = (gchar *)gtk_entry_get_text(GTK_ENTRY(user->gui_trigger.entry2));
        level  = gtk_spin_button_get_value(GTK_SPIN_BUTTON(user->gui_trigger.spin));

        if (!entry1[0] || !entry2[0])
        {
            return;
        }

        /* update the trigger */
        set_trigger(user, trigger, entry1, entry2);
        trigger->level = (gint)level;

        /* set the trigger view */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           COLUMN_ENABLE, trigger->enabled,
			   COLUMN_LEVEL, trigger->level,
			   COLUMN_TRIGGER, trigger->input,
                           COLUMN_POINTER, trigger, -1);
    }
    else if (!strcmp(label, BUTTON_LOGIN))
    {
        GtkSpinButton *spin;
        
        /* grab the spin button and update its value */
        spin = GTK_SPIN_BUTTON(user->gui_trigger.spin);

        gtk_spin_button_set_value(spin, TRG_LOGIN);
    }
    else if (!strcmp(label, BUTTON_PASSWORD))
    {
        GtkSpinButton *spin;
        
        /* grab the spin button and update its value */
        spin = GTK_SPIN_BUTTON(user->gui_trigger.spin);

        gtk_spin_button_set_value(spin, TRG_PASSWORD);
    }
    else if (!strcmp(label, BUTTON_EXIT))
    {
        /* exit the trigger view */
        gtk_widget_destroy(user->gui_pref.g_preference);
    }
}


static void gui_trigger_toggle(GtkCellRendererToggle *cell, 
                               gchar                 *path_str, 
                               gpointer               data)
{
    GtkTreeModel *model = (GtkTreeModel *)data;
    GtkTreePath  *path = gtk_tree_path_new_from_string(path_str);
    TRIGGER      *trigger;
    GtkTreeIter   iter;
    gint          enabled;

    /* get toggled iter */
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, COLUMN_ENABLE, &enabled, -1);
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &trigger, -1);

    /* do something with the value */
    enabled ^= 1;

    /* store the enabled flag in the trigger data */
    trigger->enabled = enabled;

    /* set new value */
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, COLUMN_ENABLE, enabled, -1);

    /* clean up */
    gtk_tree_path_free(path);
}


static void gui_trigger_row_selected(GtkTreeView *treeview, USER *user)
{
    GtkTreeSelection *select;
    GtkTreeModel     *model;
    TRIGGER          *trigger;
    GtkTreeIter       iter;

    select = gtk_tree_view_get_selection(gui_trigger_get_treeview(user));

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    model = gtk_tree_view_get_model(gui_trigger_get_treeview(user));
    gtk_tree_model_get(model, &iter, COLUMN_POINTER, &trigger, -1);

    /* set the entries and spin button */
    gtk_entry_set_text(GTK_ENTRY(user->gui_trigger.entry1), trigger->input);
    gtk_entry_set_text(GTK_ENTRY(user->gui_trigger.entry2), trigger->response);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(user->gui_trigger.spin), 
                              (gdouble)trigger->level);
}


static GtkTreeModel *gui_trigger_create_model(USER *user)
{
    GtkListStore *store;
    GtkTreeIter   iter;
    TRIGGER      *trigger;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
			       G_TYPE_BOOLEAN,
			       G_TYPE_INT,
			       G_TYPE_STRING,
                               G_TYPE_POINTER);

    /* add data to the list store */
    for (trigger = user->trigger_list; trigger; trigger = trigger->next)
    {
        /* we do NOT use gui_trigger_add_to_pref(user, trigger) in this case
           for efficiency! */
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           COLUMN_ENABLE,  trigger->enabled,
			   COLUMN_LEVEL,   trigger->level,
			   COLUMN_TRIGGER, trigger->input,
                           COLUMN_POINTER, trigger, -1);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_trigger_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;
    GtkTreeModel      *model = gtk_tree_view_get_model(treeview);

    /* column for trigger toggles */
    renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(G_OBJECT(renderer), "toggled",
                     G_CALLBACK(gui_trigger_toggle), model);

    column = gtk_tree_view_column_new_with_attributes("Enabled",
                                                      renderer,
						      "active", 
                                                      COLUMN_ENABLE,
						      NULL);

    /* set this column to a fixed sizing */
    gtk_tree_view_column_set_sizing(GTK_TREE_VIEW_COLUMN(column),
                                    GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(column), 60);
    gtk_tree_view_append_column(treeview, column);

    /* column for level numbers */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Level",
                                                      renderer,
						      "text",
						      COLUMN_LEVEL,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_LEVEL);
    gtk_tree_view_append_column(treeview, column);

    /* column for trigger */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Trigger",
						      renderer,
						      "text",
						      COLUMN_TRIGGER,
						      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_TRIGGER);
    gtk_tree_view_append_column(treeview, column);
}


void gui_trigger_add_pref(USER *user, TRIGGER *trigger)
{
    GtkListStore *store;
    GtkTreeIter   iter;

    /* preference window not created yet? */
    if (!user->gui_pref.g_preference ||
        !user->gui_pref.g_edit[SEL_TRIGGERS])
    {
        return;
    }

    /* retrieve the store */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(gui_trigger_get_treeview(user)));

    /* append the trigger to the list */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COLUMN_ENABLE,  trigger->enabled,
                       COLUMN_LEVEL,   trigger->level,
                       COLUMN_TRIGGER, trigger->input,
                       COLUMN_POINTER, trigger, -1);
}


GtkWidget *gui_trigger_create(USER *user)
{
    GtkWidget       *view;
    GtkTreeModel    *model;
    GtkWidget       *bbox;
    GtkWidget       *hbox;
    GtkWidget       *mainbox;
    GtkWidget       *trigger, *response, *spin;
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
    model = gui_trigger_create_model(user);

    /* create tree view */
    view = gtk_tree_view_new_with_model(model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_TRIGGER);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(model));

    g_signal_connect(G_OBJECT(view), "cursor-changed",
                     G_CALLBACK(gui_trigger_row_selected), user);    

    /* add columns to the tree view */
    gui_trigger_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a hbox for the trigger */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the trigger entry */
    user->gui_trigger.entry1 = trigger = gtk_entry_new();

    /* put trigger in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), trigger, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Trigger");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(trigger), MAX_INPUT_LEN-1);

    /* create a hbox for the response */
    hbox = gtk_hbox_new(TRUE, 0);

    /* create the response entry */
    user->gui_trigger.entry2 = response = gtk_entry_new();

    /* put response in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), response, TRUE, TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Response");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(response), MAX_INPUT_LEN-1);

    /* create a hbox for spin button and 2 buttons */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create a spin button for the level */
    user->gui_trigger.spin = spin = gtk_spin_button_new_with_range(-100000, 100000, 1);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 0);

    gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, TRUE, 0);

    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* put the horizontal button box in the hbox */
    gtk_box_pack_start(GTK_BOX(hbox), bbox, TRUE, TRUE, 5);

    /* create buttons for login and password triggers */
    button = gtk_button_new_with_label(BUTTON_LOGIN);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    
    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 4);

    button = gtk_button_new_with_label(BUTTON_PASSWORD);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    
    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 4);

    frame = gtk_frame_new("Level");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_NEW);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_APPLY);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_trigger_button_release), user);    
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_trigger_button_release), user);    

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    return mainbox;
}
