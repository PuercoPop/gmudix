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

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mudix.h"
#include "file.h"


/* first pointer to a list of ALL users */
USER *user_list = NULL;


USER *new_user(void)
{
    USER *user;
    int   i;

    /* get us some memory */
    if (!(user = malloc(sizeof(USER)))) 
    {
        fprintf(stdout, "No memory for user?!\n");
        exit_mudix();
    }

    if (!(user->net.rxbuf = malloc(sizeof(gchar) * RXBUF_LENGTH))) 
    {
        fprintf(stdout, "No memory for receive buffer?!\n");
        exit_mudix();
    }

    if (!(user->net.iacbuf = malloc(sizeof(gchar) * RXBUF_LENGTH))) 
    {
        fprintf(stdout, "No memory for IAC buffer?!\n");
        exit_mudix();
    }

    user->next                  = NULL;
    user->net.rxp               = user->net.rxbuf;
    user->net.iacp              = user->net.iacbuf;
    user->net.stream            = NULL;
    user->net.mccp_out          = NULL;
    user->net.mccp_outp         = NULL;
    user->net.mccp_outsize      = 0;
    user->net.site              = NULL;
    user->net.host_name         = NULL;
    user->net.rx_tag            = 0;
    user->net.exc_tag           = 0;
    user->net.port              = 0;
    user->net.sock              = 0;
    user->net.thread_id         = 0;
    user->net.status            = NET_CLOSED;
    user->net.recon_timer_load  = 0;
    user->net.recon_timer       = 0;
    user->net.recon_id          = 0;
    user->net.rx_proc_state     = RX_PROC_STATE_DATA;
    user->net.charset           = strdup(DEFAULT_CHAR_SET);

    user->gui_user.g_window     = NULL;
    user->gui_user.g_view       = NULL;
    user->gui_user.g_scrollback = NULL;
    user->gui_user.g_buffer     = NULL;
    user->gui_user.g_status     = NULL;
    user->gui_user.g_input      = NULL;
    user->gui_user.g_mark       = NULL;
    user->gui_user.win_width    = DEFAULT_WIDTH;
    user->gui_user.win_height   = DEFAULT_HEIGHT;
    user->gui_user.win_pos_x    = 0;
    user->gui_user.win_pos_y    = 0;
    user->gui_user.lines_max    = DEFAULT_LINE_MAX;
    user->gui_user.wrap_mode    = GTK_WRAP_CHAR;
    user->gui_pref.g_preference = NULL;
    user->gui_user.space_above  = DEFAULT_SPACING;
    user->gui_user.space_below  = DEFAULT_SPACING;
    user->gui_pref.g_selecttree = NULL;
    user->gui_pref.pref_width   = DEFAULT_PREF_WIDTH;
    user->gui_pref.pref_height  = DEFAULT_PREF_HEIGHT;
    
    /* set all colors to their default */
    gui_color_set_def_all(user);

    /* clear all tags */
    for (i=0; i<NR_ALL_COLORS; i++)
    {
        user->gui_user.g_fg_color_tags[i] = NULL;
        user->gui_user.g_bg_color_tags[i] = NULL;
    }
    user->gui_user.g_underline_tag = NULL;

    /* setup default fonts */
    user->gui_user.fonts[FONT_USER_WINDOW]     = strdup(DEFAULT_FONT);
    user->gui_user.fonts[FONT_USER_SCROLLBACK] = strdup(DEFAULT_FONT);
    user->gui_user.fonts[FONT_USER_INPUT]      = strdup(DEFAULT_FONT_INPUT);
    user->gui_user.fonts[FONT_USER_CAPTURE]    = strdup(DEFAULT_FONT);

    /* clear all editors */
    for (i=0; i<NUM_SELECT; i++)
    {
        user->gui_pref.g_edit[i] = NULL;
    }

    user->alias_list   = NULL;
    user->macro_list   = NULL;
    user->path_list    = NULL;
    user->tabs_list    = NULL;
    user->timer_list   = NULL;
    user->trigger_list = NULL;
    user->vars_list    = NULL;
    user->logfile      = NULL;
    user->filename     = NULL;
    user->session      = NULL;
    user->flags        = 0;

    /* create a trigger buffer */
    if (!(user->trigger_buf = malloc(sizeof(gchar) * MAX_STRING))) 
    {
        fprintf(stdout, "No memory for trigger buffer?!\n");
        exit_mudix();
    }

    /* ansi settings */
    user->ansi.attrib  = 0;
    user->ansi.fg      = ANSI_COLOR_NONE;
    user->ansi.bg      = ANSI_COLOR_NONE;
    user->ansi.paramp  = user->ansi.parambuf;
    user->ansi.state   = ANSI_STATE_IDLE;

    /* also set current trigger pointer */
    user->trigger = user->trigger_buf;

    /* set customizable characters to default */
    user->custom_chars[CUST_CMD_STACK]   = DEFAULT_CMD_STACK;
    user->custom_chars[CUST_VAR_SIGN]    = DEFAULT_VAR_SIGN;
    user->custom_chars[CUST_BLOCK_OPEN]  = DEFAULT_BLOCK_OPEN;
    user->custom_chars[CUST_BLOCK_CLOSE] = DEFAULT_BLOCK_CLOSE;
    user->custom_chars[CUST_CMD_CHAR]    = DEFAULT_CMD_CHAR;
    user->custom_chars[CUST_SPEED_PATH]  = DEFAULT_SPEED_PATH;

    /* intialize the history buffers */
    init_history(user);

    /* create a 1 sec timer for the timers */
    user->timer_id = init_timer(user);

    /* give this user an ID */
    user->id = 0;
    while (get_user_id(user->id))
    {
        user->id++;
    }

    /* lock the mutex as we are going to change the user_list */
    g_mutex_lock(user_network_mutex);

    /* put user in the user_list */
    user->next = user_list;
    user_list  = user;

    /* unlock it again */
    g_mutex_unlock(user_network_mutex);

    return user;
}


void del_user(USER *user)
{
    int i;

    /* clean up the alias list */
    cleanup_alias_list(user);
    
    /* clean up the macro list */
    cleanup_macro_list(user);
    
    /* clean up the path list */
    cleanup_path_list(user);
    
    /* clean up the timer list */
    cleanup_timer_list(user);

    /* clean up the trigger list */
    cleanup_trigger_list(user);
    
    /* clean up the variable list */
    cleanup_vars_list(user);

    /* clean up the tab completion list */
    cleanup_tabs_list(user);

    /* clean up the history */
    cleanup_history(user);

    /* clean up the fonts */
    for (i=0; i<NR_FONTS; i++)
    {
        free(user->gui_user.fonts[i]);
    }

    if (user->trigger_buf)
    {
        free(user->trigger_buf);
    }

    /* clean up the filename */
    if (user->filename)
    {
        free(user->filename);
    }

    /* clean up the session name */
    if (user->session)
    {
        free(user->session);
    }

    /* remove the previously created timer */
    gtk_timeout_remove(user->timer_id);

    /* lock the mutex as we are going to remove the net data of a user */
    g_mutex_lock(user_network_mutex);

    /* clean up the site string */
    if (user->net.site)
    {
        free(user->net.site);
    }

    /* clean up the host name */
    if (user->net.host_name)
    {
        free(user->net.host_name);
    }

    /* clean up the MCCP buffer */
    if (user->net.mccp_out)
    {
        free(user->net.mccp_out);
    }

    /* clean up the MCCP stream */
    if (user->net.stream)
    {
        free(user->net.stream);
    }

    /* clean up the receive buffer */
    if (user->net.rxbuf)
    {
        free(user->net.rxbuf);
    }

    /* clean up the IAC buffer */
    if (user->net.iacbuf)
    {
        free(user->net.iacbuf);
    }

    /* remove the user from the user_list */
    if (user == user_list)
    {
        user_list = user->next;
    }
    else
    {
        USER *iUser;

        for (iUser = user_list; iUser; iUser = iUser->next)
        {
            if (iUser->next == user)
            {
                iUser->next = user->next;
                break;
            }
        }
    }

    /* unlock it again */
    g_mutex_unlock(user_network_mutex);

    free(user);
}


void init_user_mutex(void)
{
    /* check if it was already initialized */
    if (user_network_mutex)
    {
        return;
    }

    /* create a mutex */
    user_network_mutex = g_mutex_new();
}


/* get the user data that belongs to an existing window */
USER *get_user_window(GtkWindow *window)
{
    USER *user;

    for (user = user_list; user; user = user->next)
    {
        if (GTK_WINDOW(user->gui_user.g_window) == window)
        {
            /* yes - this user owns the window :) */
            break;
        }
    }

    return user;
}


bool check_valid_user(USER *user)
{
    USER *list;

    for (list = user_list; list; list = list->next)
    {
        if (user == list)
        {
            /* yes, this user is valid */
            break;
        }
    }

    return (list)? TRUE: FALSE;
}


int user_count(void)
{
    USER *user;
    int   count = 0;

    for (user = user_list; user; user = user->next)
    {
        count++;
    }

    return count;
}


USER *get_user_with_color(GdkColor *color)
{
    USER *user;

    /* smart lookup: check if color address in range of user */
    for (user = user_list; user; user = user->next)
    {
        if (((gint)color >  (gint)user) && 
            ((gint)color < ((gint)user + (gint)sizeof(USER))))
        {
            /* address of color is within this user's range */
            return user;
        }
    }
    
    return NULL;
}


USER *get_user_id(guint id)
{
    USER *user;

    for (user = user_list; user; user = user->next)
    {
        if (user->id == id)
        {
            return user;
        }
    }
    
    return NULL;
}


USER *get_user_file(char *file)
{
    USER *user;

    for (user = user_list; user; user = user->next)
    {
        if (!strcmp(user->filename, file))
        {
            return user;
        }
    }
    
    return NULL;
}


/* look up the user that owns this session pointer */
USER *get_user_session(gchar *session)
{
    USER *user;

    for (user = user_list; user; user = user->next)
    {
        if (user->session == session)
        {
            return user;
        }
    }
    
    return NULL;
}


/* next functions are for reading and writing the user-file */

static void user_read_old(USER *user, FILE *fp)
{
    int i;

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "name")) 
        {
            user->session = fread_string(fp);
        }   
        else if (!strcmp(word, "site")) 
        {
            user->net.site = fread_string(fp);
        }   
        /* OBSOLETE: al, but here to support old MUDix (only F1 to F12) */
        else if (!strcmp(word, "al"))
        {
            MACRO *macro;
            guint  key = atoi(fread_word(fp));

            if (!(macro = new_macro_append(user)))
            {
                break;
            }

            if (key < 2)
            {
                /* F1 and F2 */
                macro->key = GDK_F1+key;
            }
            else
            {
                /* F3 to F12 */
                macro->key = GDK_F3+(key-2);
            }
            macro->state = 0;
            macro->text  = fread_string(fp);
        }
        else if (!strcmp(word, "macro"))
        {
            MACRO *macro;
            guint  key   = atoi(fread_word(fp));
            guint  state = atoi(fread_word(fp));

            if (!(macro = new_macro_append(user)))
            {
                break;
            }

            macro->key   = key;
            macro->state = state;
            macro->text  = fread_string(fp);
        }   
        /* support for old MUDix alias: nmal */
        else if (!strcmp(word, "nmal") ||
                 !strcmp(word, "alias"))
        {
            ALIAS *alias;

            if (!(alias = new_alias_append(user)))
            {
                break;
            }

            alias->name   = fread_string(fp);
            alias->string = fread_string(fp);
        }
        else if (!strcmp(word, "tab")) 
        {
            TAB *tab;

            if (!(tab = new_tab_append(user)))
            {
                break;
            }

            tab->name = fread_string(fp);
        }
        else if (!strcmp(word, "path")) 
        {
            PATH *path;

            if (!(path = new_path_append(user)))
            {
                break;
            }

            path->name = fread_string(fp);
            path->path = fread_string(fp);
        }
        else if (!strcmp(word, "var")) 
        {
            VAR *var;

            if (!(var = new_var_append(user)))
            {
                break;
            }

            var->name  = fread_string(fp);
            var->value = fread_string(fp);
        }
        /* support for old MUDix trigger */
        else if (!strcmp(word, "trig")) 
        {
            TRIGGER *trigger;
            int      level = atoi(fread_word(fp));

            if (!(trigger = new_trigger_append(user, level)))
            {
                break;
            }

            trigger->input    = fread_string(fp);
            trigger->response = fread_string(fp);

            trigger->enabled  = TRUE;
        }   
        else if (!strcmp(word, "trigger")) 
        {
            TRIGGER *trigger;
            int      level = atoi(fread_word(fp));

            if (!(trigger = new_trigger_append(user, level)))
            {
                break;
            }

            trigger->enabled  = atoi(fread_word(fp));
            trigger->input    = fread_string(fp);
            trigger->response = fread_string(fp);

            /* if it's a login trigger, then always enable it */
            if (trigger->level == TRG_LOGIN ||
                trigger->level == TRG_PASSWORD)
            {
                trigger->enabled = TRUE;
            }
        }   
        else if (!strcmp(word, "timer")) 
        {
            TIMER *timer;
            int    relval = atoi(fread_word(fp));
            int    relcnt = atoi(fread_word(fp));
            int    time = atoi(fread_word(fp));

            if (!(timer = new_timer_append(user)))
            {
                break;
            }

            timer->response = fread_string(fp);
            timer->relval   = relval;
            timer->relcnt   = relcnt;

            /* time holds the time left when timer was saved */
            if (time)
            {
                /* reload the timer if the timer was running */
                timer->timer = relval;
            }
        }   
        else if (!strcmp(word, "port")) 
        {
            char *str = fread_word(fp);

            /* OLD port was read with fread_string, remove ~ */
            for (i=0; i<(int)strlen(str); i++)
            {
                if (str[i] == '~')
                {
                    str[i] = '\0';
                    break;
                }
            }

            user->net.port = atoi(str);
        }   
        else if (!strcmp(word, "winheight")) 
        {
            user->gui_user.win_height = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "winwidth"))
        {
            user->gui_user.win_width = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "winposx")) 
        {
            user->gui_user.win_pos_x = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "winposy"))
        {
            user->gui_user.win_pos_y = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "linemax")) 
        {
            user->gui_user.lines_max = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "prfheight")) 
        {
            user->gui_pref.pref_height = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "prfwidth"))
        {
            user->gui_pref.pref_width = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "fgcolor"))
        {
            user->gui_user.default_color[DEF_FG].red   = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_FG].green = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_FG].blue  = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "bgcolor"))
        {
            user->gui_user.default_color[DEF_BG].red   = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_BG].green = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_BG].blue  = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "colors"))
        {
            int nrcols = atoi(fread_word(fp));

            for (i=0; i<nrcols && i<NR_ALL_COLORS; i++)
            {
                user->gui_user.colors[i].red   = atoi(fread_word(fp));
                user->gui_user.colors[i].green = atoi(fread_word(fp));
                user->gui_user.colors[i].blue  = atoi(fread_word(fp));
            }
        }
        else if (!strcmp(word, "fonts"))
        {
            int nrfonts = atoi(fread_word(fp));

            /* only set our "known" fonts */
            for (i=0; i<nrfonts && i<NR_FONTS; i++)
            {
                free(user->gui_user.fonts[i]);
                user->gui_user.fonts[i] = fread_string(fp);
            }
        }
        else if (!strcmp(word, "flags"))
        {
            user->flags = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "rectime"))
        {
            user->net.recon_timer_load = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "charset"))
        {
            free(user->net.charset);
            user->net.charset = fread_string(fp);
        }
        else if (!strcmp(word, "custchars"))
        {
            int nrcust = atoi(fread_word(fp));

            /* only set our "known" custom chars */
            for (i=0; i<nrcust && i<NR_CUSTOM_CHARS; i++)
            {
                user->custom_chars[i] = atoi(fread_word(fp));
            }
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
    }
}


static void user_read_general(USER *user, FILE *fp)
{
    int i;

    while (1) 
    {
        char *word;

        word = fread_word(fp);
            if (*word == '\0')
        {
                break;
        }

        if (!strcmp(word, "Name")) 
        {
            user->session = fread_string(fp);
        }   
        else if (!strcmp(word, "Site")) 
        {
            user->net.site = fread_string(fp);
        }   
        else if (!strcmp(word, "Port")) 
        {
            user->net.port = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Winheight")) 
        {
            user->gui_user.win_height = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Winwidth"))
        {
            user->gui_user.win_width = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "Winposx")) 
        {
            user->gui_user.win_pos_x = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Winposy"))
        {
            user->gui_user.win_pos_y = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "Linemax")) 
        {
            user->gui_user.lines_max = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Spacing")) 
        {
            user->gui_user.space_above = atoi(fread_word(fp));
            user->gui_user.space_below = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Wrapmode")) 
        {
            user->gui_user.wrap_mode = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Prfheight")) 
        {
            user->gui_pref.pref_height = atoi(fread_word(fp));
        }   
        else if (!strcmp(word, "Prfwidth"))
        {
            user->gui_pref.pref_width = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "Foreground"))
        {
            user->gui_user.default_color[DEF_FG].red   = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_FG].green = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_FG].blue  = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "Background"))
        {
            user->gui_user.default_color[DEF_BG].red   = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_BG].green = atoi(fread_word(fp));
            user->gui_user.default_color[DEF_BG].blue  = atoi(fread_word(fp));
        }  
        else if (!strcmp(word, "Colors"))
        {
            int nrcols = atoi(fread_word(fp));

            for (i=0; i<nrcols && i<NR_ALL_COLORS; i++)
            {
                user->gui_user.colors[i].red   = atoi(fread_word(fp));
                user->gui_user.colors[i].green = atoi(fread_word(fp));
                user->gui_user.colors[i].blue  = atoi(fread_word(fp));
            }
        }
        else if (!strcmp(word, "Fonts"))
        {
            int nrfonts = atoi(fread_word(fp));

            /* only set our "known" fonts */
            for (i=0; i<nrfonts && i<NR_FONTS; i++)
            {
                free(user->gui_user.fonts[i]);
                user->gui_user.fonts[i] = fread_string(fp);
            }
        }
        else if (!strcmp(word, "Flags"))
        {
            user->flags = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "ReconTime"))
        {
            user->net.recon_timer_load = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "Charset"))
        {
            free(user->net.charset);
            user->net.charset = fread_string(fp);
        }
        else if (!strcmp(word, "CustomChars"))
        {
            int nrcust = atoi(fread_word(fp));

            /* only set our "known" custom chars */
            for (i=0; i<nrcust && i<NR_CUSTOM_CHARS; i++)
            {
                user->custom_chars[i] = atoi(fread_word(fp));
            }
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
    }
}


static void user_read_trigger(USER *user, FILE *fp)
{
    TRIGGER *trigger;

    /* by default create a trigger with level 0 */
    if (!(trigger = new_trigger_append(user, 0)))
    {
        return;
    }

    while (1) 
    {
        char    *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Input"))
        {
            trigger->input = fread_string(fp);
        }
        else if (!strcmp(word, "Response"))
        {
            trigger->response = fread_string(fp);
        }
        else if (!strcmp(word, "Level"))
        {
            trigger->level = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "Enabled"))
        {
            trigger->enabled = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #TRIGGER block: %s\n", word);
        }
    }

    /* always set login triggers to enabled */
    if (trigger->level == TRG_LOGIN ||
        trigger->level == TRG_PASSWORD)
    {
        trigger->enabled = TRUE;
    }

    /* check for pointers that need to be set */
    if (!trigger->input || !trigger->response)
    {
        fprintf(stdout, "Error in #TRIGGER block, missing input/response field!\n");
        free_trigger(user, trigger);
    }
}


static void user_read_timer(USER *user, FILE *fp)
{
    TIMER *timer;

    if (!(timer = new_timer_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Response"))
        {
            timer->response = fread_string(fp);
        }
        else if (!strcmp(word, "Timer"))
        {
            timer->timer = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "ReloadValue"))
        {
            timer->relval = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "ReloadCounter"))
        {
            timer->relcnt = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #TIMER block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!timer->response)
    {
        fprintf(stdout, "Error in #TIMER block, missing response field!\n");
        free_timer(user, timer);
    }
}


static void user_read_alias(USER *user, FILE *fp)
{
    ALIAS *alias;

    if (!(alias = new_alias_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Name"))
        {
            alias->name = fread_string(fp);
        }
        else if (!strcmp(word, "String"))
        {
            alias->string = fread_string(fp);
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #ALIAS block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!alias->name || !alias->string)
    {
        fprintf(stdout, "Error in #ALIAS block, missing name/string field!\n");
        free_alias(user, alias);
    }
}


static void user_read_macro(USER *user, FILE *fp)
{
    MACRO *macro;

    if (!(macro = new_macro_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Key"))
        {
            macro->key = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "State"))
        {
            macro->state = atoi(fread_word(fp));
        }
        else if (!strcmp(word, "Text"))
        {
            macro->text = fread_string(fp);
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #MACRO block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!macro->text)
    {
        fprintf(stdout, "Error in #MACRO block, missing text field!\n");
        free_macro(user, macro);
    }
}


static void user_read_path(USER *user, FILE *fp)
{
    PATH *path;

    if (!(path = new_path_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Name"))
        {
            path->name = fread_string(fp);
        }
        else if (!strcmp(word, "Path"))
        {
            path->path = fread_string(fp);
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #PATH block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!path->path || !path->name)
    {
        fprintf(stdout, "Error in #PATH block, missing path/name field!\n");
        free_path(user, path);
    }
}


static void user_read_tab(USER *user, FILE *fp)
{
    TAB *tab;

    if (!(tab = new_tab_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Name"))
        {
            tab->name = fread_string(fp);
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #TAB block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!tab->name)
    {
        fprintf(stdout, "Error in #TAB block, missing name field!\n");
        free_tab(user, tab);
    }
}


static void user_read_var(USER *user, FILE *fp)
{
    VAR *var;

    if (!(var = new_var_append(user)))
    {
        return;
    }

    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        if (!strcmp(word, "Name"))
        {
            var->name  = fread_string(fp);
        }
        else if (!strcmp(word, "Value"))
        {
            var->value = fread_string(fp);
        }
        else if (!strcmp(word, "#END"))
        {
            break;
        }
        else
        {
            fprintf(stdout, "Warning: wrong field in #VAR block: %s\n", word);
        }
    }

    /* check for pointers that need to be set */
    if (!var->name || !var->value)
    {
        fprintf(stdout, "Error in #VAR block, missing name/value field!\n");
        free_var(user, var);
    }
}


bool load_user(USER *user, char *file)
{
    FILE *fp;
    bool  found;

    if (!(fp = fopen(file, "r")))
    {
            return FALSE;
    }

    if (!(user->filename = strdup(file)))
    {
            fprintf(stdout, "No memory for filename?!\n");
            exit_mudix();
    }

    found = FALSE;
    while (1) 
    {
        char *word;

        word = fread_word(fp);
            if (*word == '\0')
        {
                break;
        }

        if (!strcmp(word, "#USER")) 
        {
            /* USER block found, old user-file style */
            user_read_old(user, fp);
            found = TRUE;
            break;
        }
        else if (!strcmp(word, "#GENERAL")) 
        {
            /* general block found */
            user_read_general(user, fp);
        }
        else if (!strcmp(word, "#TRIGGER")) 
        {
            /* trigger block found */
            user_read_trigger(user, fp);
        }
        else if (!strcmp(word, "#TIMER")) 
        {
            /* timer block found */
            user_read_timer(user, fp);
        }
        else if (!strcmp(word, "#ALIAS")) 
        {
            /* alias block found */
            user_read_alias(user, fp);
        }
        else if (!strcmp(word, "#MACRO")) 
        {
            /* macro block found */
            user_read_macro(user, fp);
        }
        else if (!strcmp(word, "#PATH")) 
        {
            /* path block found */
            user_read_path(user, fp);
        }
        else if (!strcmp(word, "#TAB")) 
        {
            /* tab block found */
            user_read_tab(user, fp);
        }
        else if (!strcmp(word, "#VAR")) 
        {
            /* var block found */
            user_read_var(user, fp);
        }
        else if (!strcmp(word, "#$")) 
        {
            /* entire file read */
                found = TRUE;
                break;
        }
        else
        {
            fprintf(stdout, "Error reading user file, unknown: %s\n", word);
            break;
        }
    }
  
    fclose(fp);

    return found;
}


bool save_user(USER *user)
{
    FILE    *fp;
    TRIGGER *trig;
    TIMER   *timer;
    ALIAS   *alias;
    MACRO   *macro;
    VAR     *var;
    PATH    *path;
    TAB     *tabs;
    int      i;

    if (!user->filename || !(fp = fopen(user->filename, "w")))
    {
        return FALSE;
    }

    /* save general user info */
    fprintf(fp, "#GENERAL\n");
    fprintf(fp, "Name           %s~\n",      smash_tilde(user->session));
    fprintf(fp, "Site           %s~\n",      smash_tilde(user->net.site));
    fprintf(fp, "Port           %d\n",       user->net.port);
    fprintf(fp, "Charset        %s~\n",      smash_tilde(user->net.charset));
    fprintf(fp, "ReconTime      %d\n",       user->net.recon_timer_load);
    fprintf(fp, "Flags          %d\n",       user->flags & FLG_SAVE_FLAGS_MASK);

    fprintf(fp, "Winheight      %d\n",       user->gui_user.win_height);
    fprintf(fp, "Winwidth       %d\n",       user->gui_user.win_width);
    fprintf(fp, "Winposx        %d\n",       user->gui_user.win_pos_x);
    fprintf(fp, "Winposy        %d\n",       user->gui_user.win_pos_y);
    fprintf(fp, "Linemax        %d\n",       user->gui_user.lines_max);
    fprintf(fp, "Wrapmode       %d\n",       user->gui_user.wrap_mode);
    fprintf(fp, "Spacing        %d %d\n",    user->gui_user.space_above,
                                             user->gui_user.space_below);

    fprintf(fp, "Prfheight      %d\n",       user->gui_pref.pref_height);
    fprintf(fp, "Prfwidth       %d\n",       user->gui_pref.pref_width);

    /* save the customizable characters */
    fprintf(fp, "CustomChars    %d",         NR_CUSTOM_CHARS);
    for (i=0; i<NR_CUSTOM_CHARS; i++)
    {
        fprintf(fp, " %d", user->custom_chars[i]);
    }
    fprintf(fp, "\n");

    /* default text FG and BG color */
    fprintf(fp, "Foreground     %d %d %d\n", user->gui_user.default_color[DEF_FG].red,
                                             user->gui_user.default_color[DEF_FG].green,
                                             user->gui_user.default_color[DEF_FG].blue);
    fprintf(fp, "Background     %d %d %d\n", user->gui_user.default_color[DEF_BG].red,
                                             user->gui_user.default_color[DEF_BG].green,
                                             user->gui_user.default_color[DEF_BG].blue);

    /* save all colors */
    fprintf(fp, "Colors         %d",         NR_ALL_COLORS);
    for (i=0; i<NR_ALL_COLORS; i++)
    {
        fprintf(fp, " %d %d %d", user->gui_user.colors[i].red,
                                 user->gui_user.colors[i].green,
                                 user->gui_user.colors[i].blue);
    }
    fprintf(fp, "\n");

    /* save the fonts */
    fprintf(fp, "Fonts          %d\n",         NR_FONTS);
    for (i=0; i<NR_FONTS; i++)
    {
        fprintf(fp, "               %s~\n", smash_tilde(user->gui_user.fonts[i]));
    }
    fprintf(fp, "#END\n");

    /* save all the triggers */
    for (trig = user->trigger_list; trig; trig = trig->next) 
    {
        fprintf(fp, "\n#TRIGGER\n");
        fprintf(fp, "Input          %s~\n", smash_tilde(trig->input));
        fprintf(fp, "Response       %s~\n", smash_tilde(trig->response));
        fprintf(fp, "Level          %d\n",  trig->level);
        fprintf(fp, "Enabled        %d\n",  trig->enabled);
        fprintf(fp, "#END\n");
    }

    /* save all the timers */
    for (timer = user->timer_list; timer; timer = timer->next) 
    {
        if (timer->relcnt != TIMER_ONESHOT)
        {
            fprintf(fp, "\n#TIMER\n");
            fprintf(fp, "Response       %s~\n", smash_tilde(timer->response));
            fprintf(fp, "Timer          %d\n",  timer->timer);
            fprintf(fp, "ReloadValue    %d\n",  timer->relval);
            fprintf(fp, "ReloadCounter  %d\n",  timer->relcnt);
            fprintf(fp, "#END\n");
        }
    }

    /* save aliases */
    for (alias = user->alias_list; alias; alias = alias->next) 
    {
        fprintf(fp, "\n#ALIAS\n");
        fprintf(fp, "Name           %s~\n", smash_tilde(alias->name));
        fprintf(fp, "String         %s~\n", smash_tilde(alias->string));
        fprintf(fp, "#END\n");
    }
            
    /* save the macro keys */
    for (macro = user->macro_list; macro; macro = macro->next) 
    {
        fprintf(fp, "\n#MACRO\n");
        fprintf(fp, "Key            %d\n",  macro->key);
        fprintf(fp, "State          %d\n",  macro->state);
        fprintf(fp, "Text           %s~\n", smash_tilde(macro->text));
        fprintf(fp, "#END\n");
    }

    /* save paths */
    for (path = user->path_list; path; path = path->next) 
    {
        fprintf(fp, "\n#PATH\n");
        fprintf(fp, "Name           %s~\n", smash_tilde(path->name));
        fprintf(fp, "Path           %s~\n", smash_tilde(path->path));
        fprintf(fp, "#END\n");
    }
            
    /* save tab completions */
    for (tabs = user->tabs_list; tabs; tabs = tabs->next) 
    {
        fprintf(fp, "\n#TAB\n");
        fprintf(fp, "Name           %s~\n", smash_tilde(tabs->name));
        fprintf(fp, "#END\n");
    }
            
    /* save variables */
    for (var = user->vars_list; var; var = var->next) 
    {
        fprintf(fp, "\n#VAR\n");
        fprintf(fp, "Name           %s~\n", smash_tilde(var->name));
        fprintf(fp, "Value          %s~\n", smash_tilde(var->value));
        fprintf(fp, "#END\n");
    }
            
    fprintf(fp, "\n#$");
    fclose(fp);

    return TRUE;
}


bool user_read_info(char *path, char **name, char **site, guint *port)
{
    FILE *fp;
    bool  found;
    int   i;

    if (!(fp = fopen(path, "r")))
    {
        return FALSE;
    }

    found = FALSE;
    while (1) 
    {
        char *word;

        word = fread_word(fp);
        if (*word == '\0')
        {
            break;
        }

        /* support for both old and new-style user-file */
        if (!strcmp(word, "#USER") || !strcmp(word, "#GENERAL")) 
        {
            /* USER/GENERAL block found :) */
            found = TRUE;
        }
        else if (!strcmp(word, "name") || !strcmp(word, "Name")) 
        {
            *name = fread_string(fp);
        }   
        else if (!strcmp(word, "site") || !strcmp(word, "Site")) 
        {
            *site = fread_string(fp);
        }   
        else if (!strcmp(word, "port") || !strcmp(word, "Port")) 
        {
            char *str = fread_word(fp);

            /* OLD port was read with fread_string, remove ~ */
            for (i=0; i<(int)strlen(str); i++)
            {
                if (str[i] == '~')
                {
                    str[i] = '\0';
                    break;
                }
            }

            *port = atoi(str);
            break;  /* we know that port is the LAST to read, so break */
        }   
        else
        {
            if (feof(fp))
            {
                break;
            }
            else
            {
                fread_to_eol(fp);
                continue;
            }
        }
    }
  
    fclose(fp);

    return found;
}


