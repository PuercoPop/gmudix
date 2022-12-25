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
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include "mudix.h"

/* so it can be available in all functions */
static GtkWidget       *gui_main_window;
static GtkTreeModel    *gui_main_model;

#define BUTTON_CONNECT  "_Connect"
#define BUTTON_EDIT     "_Edit"
#define BUTTON_NEW      "_New"
#define BUTTON_DELETE   "_Delete"
#define BUTTON_EXIT     "E_xit"

typedef enum
{
    COLUMN_NAME,    /* user name */
    COLUMN_SITE,    /* remote host */
    COLUMN_PORT,    /* port to connect to */
    COLUMN_LAST,    /* last time connected (save time user file) */
    COLUMN_PATH,    /* path to the file */
    NUM_COLUMNS
} USER_COLUMNS;


static void gui_main_connect(char *path)
{
    USER *user;

    /* create and load the selected user */
    user = new_user();
    if (load_user(user, path))
    {
        if (!user->session)
        {
            /* this user has NO session (old MUDix?) - so set it to filename */
            user->session = strdup(user->filename);
        }

        /* create the window */
        gui_setup_user_window(user);
        gui_user_setup_tags(user);

        /* make the connection to the server */
        gui_user_connect(user);

        /* close the user selector */
        destroy_with_signal(gui_main_window);
    }
    else
    {
        /* user failed to load */
        gui_dialog_msg(user, "Error reading user file!");

        del_user(user);
    }
}


static void gui_main_edit(char *path)
{
    USER *user;

    /* see if we already have this user open */
    if ((user = get_user_file(path)))
    {
        /* open the preference editor for that user */
        gui_preference_editor(user, SEL_GENERAL);
        return;
    }

    /* no, then create and load the selected user */
    user = new_user();
    if (load_user(user, path))
    {
        if (!user->session)
        {
            /* this user has NO session (old MUDix?) - so set it to filename */
            user->session = strdup(user->filename);
        }

        /* open the preference editor for that user */
        gui_preference_editor(user, SEL_GENERAL);
    }
    else
    {
        /* user failed to load */
        gui_dialog_msg(user, "Error reading user file!");

        del_user(user);
    }
}


static void gui_destroy_main_window_callback(GtkWindow *window, gpointer data)
{
    /* if there are no user windows open, exit gMUDix */
    if (!user_count())
    {
        gtk_main_quit();
    }
    else
    {
        /* just destroy the main window */
        gtk_widget_destroy(gui_main_window);

        /* clear the pointer */
        gui_main_window = NULL;
    }
}


static void gui_main_dialog_new(void)
{
    GtkWidget *dialog;
    GtkWidget *entry1;
    GtkWidget *entry2;
    GtkWidget *entry3;
    GtkWidget *entry4;
    GtkWidget *entry5;
    GtkWidget *entry6;
    GtkWidget *spin;
    GtkWidget *label;
    GtkWidget *sep;
    gint       response;
    bool       fDestroyMain = FALSE;

    /* create a dialog popup window */
    dialog = gtk_dialog_new_with_buttons("New User",
                                         GTK_WINDOW(gui_main_window),
                                         GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_STOCK_OK,
                                         GTK_RESPONSE_OK,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    label = gtk_label_new("Session name:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the session name */
    entry1 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry1, FALSE, FALSE, 5);
    gtk_entry_set_max_length(GTK_ENTRY(entry1), 100);

    label = gtk_label_new("Hostname/IP:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the IP or hostname */
    entry2 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry2, FALSE, FALSE, 5);
    gtk_entry_set_max_length(GTK_ENTRY(entry2), 100);

    label = gtk_label_new("Port number:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create a spin button holding the port number */
    spin = gtk_spin_button_new_with_range(0, 99999999, 1);

    gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(spin), FALSE);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 1024);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), spin, FALSE, FALSE, 5);

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), sep, FALSE, FALSE, 10);

    label = gtk_label_new("Login trigger:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the login trigger */
    entry3 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry3, FALSE, FALSE, 5);

    label = gtk_label_new("Login trigger response:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the login response */
    entry4 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry4, FALSE, FALSE, 5);

    label = gtk_label_new("Password trigger:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the password trigger */
    entry5 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry5, FALSE, FALSE, 5);

    label = gtk_label_new("Password trigger response:");
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 0);

    /* create an entry for the password response */
    entry6 = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), entry6, FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    /* get the response from the dialog */
    response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK)
    {
        G_CONST_RETURN gchar *input1;
        G_CONST_RETURN gchar *input2;
                       int    port;

        /* grab the user input */
        input1 = gtk_entry_get_text(GTK_ENTRY(entry1));
        input2 = gtk_entry_get_text(GTK_ENTRY(entry2));
        port   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

        if (*input1 && *input2)
        {
            GtkTreeIter  iter;
            gchar        str[MAX_FILEPATH];
            char         path[MAX_FILEPATH];
            USER        *user;
            gchar       *pFile;
            int          i;

            /* create a new user */
            user = new_user();

            /* grab the first argument from input2 to get rid of any spaces */
            get_arg(user, (gchar *)input2, str);

            /* copy the user input */
            user->session   = strdup(input1);
            user->net.site  = strdup(str);
            user->net.port  = port;

            /* now convert the session name into a filename */
            strcpy(str, user->session);

            pFile = str;
            while (*pFile != '\0')
            {
                /* smash the following characters to a '_' */
                switch (*pFile)
                {
                    case '\\':
                    case '/':
                    case '~':
                    case '.':
                    case ' ':
                        *pFile = '_';
                        break;
                    default:
                        break;
                }
                pFile++;
            }

            /* check if filename already exist, if so add a counter to it */
            i = 0;
            while (1)
            {
                struct stat buf;
                sprintf(path, "%s/" USER_PATH "%s_%d.usr", getenv("HOME"), str, i);
                if (stat(path, &buf))
                {
                    /* file does not exist or there was an error */
                    if (errno == ENOENT)
                    {
                        /* file does not exist :) */
                        user->filename = strdup(path);
                    }
                    break;
                }
                i++;
            }

            /* add an entry to the store */
            gtk_list_store_append(GTK_LIST_STORE(gui_main_model), &iter);
            gtk_list_store_set(GTK_LIST_STORE(gui_main_model), &iter,
                               COLUMN_NAME, user->session,
                               COLUMN_SITE, user->net.site,
                               COLUMN_PORT, user->net.port,
                               COLUMN_LAST, NULL,
                               COLUMN_PATH, path,
                               -1);

            /* create the window */
            gui_setup_user_window(user);
            gui_user_setup_tags(user);
            setup_default_macros(user);

            /* grab the user input from the login trigger */
            input1 = gtk_entry_get_text(GTK_ENTRY(entry3));
            input2 = gtk_entry_get_text(GTK_ENTRY(entry4));

            if (*input1 && *input2)
            {
                create_trigger(user, TRG_LOGIN, (gchar *)input1, (gchar *)input2);
            }

            /* grab the user input from the password trigger */
            input1 = gtk_entry_get_text(GTK_ENTRY(entry5));
            input2 = gtk_entry_get_text(GTK_ENTRY(entry6));

            if (*input1 && *input2)
            {
                create_trigger(user, TRG_PASSWORD, (gchar *)input1, (gchar *)input2);
            }

            /* make the connection to the server */
            gui_user_connect(user);

            /* initial save to create the filename :) */
            save_user(user);

            fDestroyMain = TRUE;
        }
    }

    gtk_widget_destroy(dialog);

    if (fDestroyMain)
    {
        /* close the user selector (IMPORTANT: after gtk_widget_destroy(dialog)) */
        destroy_with_signal(gui_main_window);
    }
}


static void gui_main_button_release(GtkButton *button, GtkTreeView *treeview)
{
    G_CONST_RETURN gchar *label = gtk_button_get_label(button);

    if (!strcmp(label, BUTTON_CONNECT))
    {
        GtkTreeSelection *select;
        GtkTreeIter       iter;
        char             *path;

        select = gtk_tree_view_get_selection(treeview);

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        gtk_tree_model_get(gui_main_model, &iter, COLUMN_PATH, &path, -1);

        /* connect the selected user */
        gui_main_connect(path);
    }
    else if (!strcmp(label, BUTTON_EDIT))
    {
        GtkTreeSelection *select;
        GtkTreeIter       iter;
        char             *path;

        select = gtk_tree_view_get_selection(treeview);

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        gtk_tree_model_get(gui_main_model, &iter, COLUMN_PATH, &path, -1);

        /* edit the selected user */
        gui_main_edit(path);
    }
    else if (!strcmp(label, BUTTON_NEW))
    {
        gui_main_dialog_new();
    }
    else if (!strcmp(label, BUTTON_DELETE))
    {
        GtkTreeSelection *select;
        GtkTreeIter       iter;
        char             *path;

        select = gtk_tree_view_get_selection(treeview);

        /* retrieve the iter of the selected row */
        if (!gtk_tree_selection_get_selected(select, NULL, &iter))
        {
            return;
        }

        gtk_tree_model_get(gui_main_model, &iter, COLUMN_PATH, &path, -1);

        /* remove the entry from the view */
        gtk_list_store_remove(GTK_LIST_STORE(gui_main_model), &iter);

        /* delete the selected file */
        unlink(path);
    }
}


static void gui_main_row_selected(GtkTreeView       *treeview,
                                  GtkTreePath       *treepath,
                                  GtkTreeViewColumn *column,
                                  gpointer           user_data)
{
    GtkTreeSelection *select;
    GtkTreeIter       iter;
    char             *path;

    select = gtk_tree_view_get_selection(treeview);

    /* retrieve the iter of the selected row */
    if (!gtk_tree_selection_get_selected(select, NULL, &iter))
    {
        return;
    }

    gtk_tree_model_get(gui_main_model, &iter, COLUMN_PATH, &path, -1);

    /* connect the selected user */
    gui_main_connect(path);
}


static gboolean gui_main_row_do_update(GtkTreeModel *model,
                                       GtkTreePath  *treepath,
                                       GtkTreeIter  *iter,
                                       USER         *user)
{
    gchar *path;

    gtk_tree_model_get(model, iter, COLUMN_PATH, &path, -1);

    if (!strcmp(path, user->filename))
    {
        struct stat buf;
               char time[TIME_LEN];

        if (stat(path, &buf))
        {
            /* something must be wrong - I thought we just saved the file? ;) */
            return FALSE;
        }

        /* hack off the annoying '\n' from ctime */
        strcpy(time, ctime(&buf.st_mtime));
        time[strlen(time)-1] = '\0';

        /* currently only the time gets updated */
        gtk_list_store_set(GTK_LIST_STORE(model), iter,
                           COLUMN_NAME, user->session,
                           COLUMN_SITE, user->net.site,
                           COLUMN_PORT, user->net.port,
                           COLUMN_LAST, time,
                           -1);

        return TRUE;
    }

    return FALSE;
}

static GtkTreeModel *gui_main_create_model(int *latest)
{
    GtkListStore    *store;
    GtkTreeIter      iter;
    struct dirent  **namelist;
    int              i, n;
    char             path[MAX_FILEPATH];
    time_t           latest_time = 0;

    /* create list store */
    store = gtk_list_store_new(NUM_COLUMNS,
                               G_TYPE_STRING,
                               G_TYPE_STRING,
                               G_TYPE_INT,
                               G_TYPE_STRING,
                               G_TYPE_STRING);
    /* scan our directory for files */
    sprintf(path, "%s/" USER_PATH, getenv("HOME"));

    n = scandir(path, &namelist, 0, alphasort);
    if (n < 0)
    {
        char dir[MAX_FILEPATH];

        /* our user dir is non existing - create it */
        sprintf(dir, "mkdir -p %s", path);
        system(dir);
        /* set latest to -1 */
        *latest = -1;
    }
    else
    {
        int count = 0;

        for (i=0; i<n; i++)
        {
            struct stat buf;
            sprintf(path, "%s/" USER_PATH "%s", getenv("HOME"), namelist[i]->d_name);
            if (stat(path, &buf) == -1)
            {
                perror("stat");
            }
            else if (!S_ISDIR(buf.st_mode))  /* regular file */
            {
                char  time[TIME_LEN];
                char *site = NULL;
                char *name = NULL;
                gint  port;

                if (!user_read_info(path, &name, &site, &port))
                {
                    /* couldn't read info, continue to next */
                    continue;
                }

                if (!name)
                {
                    /* old MUDix file perhaps... set name to filename */
                    name = strdup(namelist[i]->d_name);
                }

                if (buf.st_mtime > latest_time)
                {
                    /* file is newer, update the latest */
                    *latest = count;
                    latest_time = buf.st_mtime;
                }

                /* counts number of entries in liststore */
                count++;

                /* hack off the annoying '\n' from ctime */
                strcpy(time, ctime(&buf.st_mtime));
                time[strlen(time)-1] = '\0';

                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                   COLUMN_NAME, name,
                                   COLUMN_SITE, site,
                                   COLUMN_PORT, port,
                                   COLUMN_LAST, time,
                                   COLUMN_PATH, path,
                                   -1);

                /* free allocated memory */
                if (name)
                {
                    free(name);
                }
                if (site)
                {
                    free(site);
                }
            }
            free(namelist[i]);
        }
        free(namelist);
    }

    return GTK_TREE_MODEL(store);
}


static void gui_main_add_columns(GtkTreeView *treeview)
{
    GtkCellRenderer   *renderer;
    GtkTreeViewColumn *column;

    /* column for user name */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name",
                                                      renderer,
                                                      "text",
                                                      COLUMN_NAME,
                                                      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);
    gtk_tree_view_append_column(treeview, column);

    /* column for site description */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Site",
                                                      renderer,
                                                      "text",
                                                      COLUMN_SITE,
                                                      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_SITE);
    gtk_tree_view_append_column(treeview, column);

    /* column for port */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Port",
                                                      renderer,
                                                      "text",
                                                      COLUMN_PORT,
                                                      NULL);
    gtk_tree_view_append_column(treeview, column);

    /* column for last */
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Last Connect",
                                                      renderer,
                                                      "text",
                                                      COLUMN_LAST,
                                                      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_LAST);
    gtk_tree_view_append_column(treeview, column);
}


static void gui_main_select_row(GtkTreeView *treeview, int row)
{
    if (row >= 0)
    {
        GtkTreePath         *path;
        GtkTreeSelection    *selection;
        char                 path_str[MAX_SMALL_STR];

        /* create a path string */
        sprintf(path_str, "%d", row);

        /* select the row */
        path      = gtk_tree_path_new_from_string(path_str);
        selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

        gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
        gtk_tree_selection_select_path(selection, path);

        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(treeview),
                                     path, NULL, TRUE, 0, 0);

        /* clean up */
        gtk_tree_path_free(path);
    }
}


void gui_main_update_row(USER *user)
{
    /* check if the main window is still open */
    if (gui_main_window)
    {
        gtk_tree_model_foreach(gui_main_model,
                               (GtkTreeModelForeachFunc)gui_main_row_do_update,
                               user);
    }
    else
    {
        /* then create it again */
        gui_setup_main_window();
    }
}


void gui_setup_main_window(void)
{
    GtkWidget       *view;
    GtkWidget       *bbox;
    GtkWidget       *mainbox;
    GtkWidget       *button;
    GtkWidget       *sw;
    int              row;

    if (gui_main_window)
    {
        /* the window already exists! make it visible */
        gtk_window_present(GTK_WINDOW(gui_main_window));
        return;
    }

    /* create the main window */
    gui_main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(gui_main_window),
                         "gMUDix v" VERSION " by M. Boomstra");
    g_signal_connect(G_OBJECT(gui_main_window), "destroy",
                     G_CALLBACK(gui_destroy_main_window_callback), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(gui_main_window), 2);
    gtk_window_set_default_size(GTK_WINDOW(gui_main_window), 400, 165);
    gtk_window_set_position(GTK_WINDOW(gui_main_window), GTK_WIN_POS_CENTER_ALWAYS);

    /* create a vertical box where the complete editor is inside */
    mainbox = gtk_vbox_new(FALSE, 2);

    /* put the vertical box inside the window */
    gtk_container_add(GTK_CONTAINER(gui_main_window), mainbox);

    /* create a scroll window where the user view is placed in */
    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_AUTOMATIC);

    /* create tree model */
    gui_main_model = gui_main_create_model(&row);

    /* create tree view */
    view = gtk_tree_view_new_with_model(gui_main_model);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(view), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(view),
                                    COLUMN_NAME);

    /* select a row */
    gui_main_select_row(GTK_TREE_VIEW(view), row);

    /* unref the model since we handed it to the treeview */
    g_object_unref(G_OBJECT(gui_main_model));

    g_signal_connect(G_OBJECT(view), "row-activated",
                     G_CALLBACK(gui_main_row_selected), NULL);

    /* add columns to the tree view */
    gui_main_add_columns(GTK_TREE_VIEW(view));

    /* put the view inside the scrolled window */
    gtk_container_add(GTK_CONTAINER(sw), view);

    /* and also add scroll window to the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), sw, TRUE, TRUE, 0);

    /* create a horizontal bar for the buttons */
    bbox = gtk_hbutton_box_new();

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_CONNECT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_main_button_release), view);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_main_button_release), view);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_EDIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_main_button_release), view);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_main_button_release), view);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    /* create the buttons */
    button = gtk_button_new_with_label(BUTTON_NEW);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_main_button_release), view);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_main_button_release), view);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_DELETE);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_main_button_release), view);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_main_button_release), view);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 2);

    button = gtk_button_new_with_label(BUTTON_EXIT);
    gtk_button_set_use_underline(GTK_BUTTON(button) , TRUE);
    /* activate is when the mnemonic key is pressed. */
    g_signal_connect(G_OBJECT(button), "activate",
                     G_CALLBACK(gui_main_button_release), view);
    g_signal_connect(G_OBJECT(button), "released",
                     G_CALLBACK(gui_destroy_main_window_callback), NULL);

    /* put the button inside the horizontal box */
    gtk_box_pack_start(GTK_BOX(bbox), button, TRUE, TRUE, 2);

    /* put the horizontal button box in the mainbox */
    gtk_box_pack_start(GTK_BOX(mainbox), bbox, FALSE, FALSE, 2);

    /* show the window :-) */
    gtk_widget_show_all(gui_main_window);
}


void init_gui(int argc, char *argv[])
{
    /* init the threadsystem */
    g_thread_init(NULL);

    /* initialize gtk */
    gtk_init(&argc, &argv);

    /* set up the main user selector window */
    gui_setup_main_window();

    /* initialize the user mutex */
    init_user_mutex();

    /* initialize the charset list */
    init_charset_list();

    /* enter the infinite loop of GTK */
    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();
}
