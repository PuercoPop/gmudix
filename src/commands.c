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
#if !defined(WIN32)
  #include <unistd.h>
#endif
#include <ctype.h>
#include "mudix.h"
#include "file.h"

typedef struct cmd_table CMDTAB;

/* global argument for all commands */
static char gl_arg1[MAX_STRING];
static char gl_arg2[MAX_STRING];

static void cmd_alias(USER *user, int index, char *args);
static void cmd_capture(USER *user, int index, char *args);
static void cmd_cls(USER *user, int index, char *args);
static void cmd_command(USER *user, int index, char *args);
static void cmd_cont_timer(USER *user, int index, char *args);
static void cmd_distrig(USER *user, int index, char *args);
static void cmd_echo(USER *user, int index, char *args);
static void cmd_enatrig(USER *user, int index, char *args);
static void cmd_for(USER *user, int index, char *args);
static void cmd_help(USER *user, int index, char *args);
static void cmd_if(USER *user, int index, char *args);
static void cmd_log(USER *user, int index, char *args);
static void cmd_macro(USER *user, int index, char *args);
static void cmd_oneshot(USER *user, int index, char *args);
static void cmd_path(USER *user, int index, char *args);
static void cmd_read(USER *user, int index, char *args);
#if !defined(WIN32)
  static void cmd_shell(USER *user, int index, char *args);
#endif
static void cmd_stop_timer(USER *user, int index, char *args);
static void cmd_tab(USER *user, int index, char *args);
static void cmd_timer(USER *user, int index, char *args);
static void cmd_trigger(USER *user, int index, char *args);
static void cmd_variable(USER *user, int index, char *args);


const struct cmd_table_type cmd_table[] =
{
    { "alias", cmd_alias, "{alias name} {response}",
            "Create or update an alias.\n" },
    { "capture", cmd_capture, "{title} <color> {text (optional)}",
            "Capture text or current line into a capture window.\n" },
    { "cls", cmd_cls, "{title of capture window (optional)}",
            "Clear the user window or clear the capture window identified by its title.\n" },
    { "cmd", cmd_command, "<id> {command line}",
            "Process the command line on the user window identified by 'id'.\n" },
    { "cont_timer", cmd_cont_timer, "<time>",
            "Continue all timers with the given time.\n" },
    { "distrig", cmd_distrig, "<level>",
            "Disable triggers of a specific level.\n" },
    { "echo", cmd_echo, "<color> {text}",
            "Echo text to the user window in the given color.\n" },
    { "enatrig", cmd_enatrig, "<level>",
            "Enable triggers of a specific level.\n" },
    { "for", cmd_for, "{nr1:nr2} {commands}",
            "For loop, to perform commands multiple times (from nr1 to nr2).\n" },
    { "if", cmd_if, "{expression} {do} {else}",
            "If expression for making comparisments.\n" },
    { "input_clear", cmd_input_clear, "{on/off}",
            "Defines whether the input bar is cleared after enter. No argument toggles this flag.\n" },
    { "input_echo", cmd_input_echo, "{on/off}",
            "Defines whether the input is echoed to the main window. No argument toggles this flag.\n" },
    { "log", cmd_log, "{filename} {date (optional)}",
            "Starts logging to a file or stops logging (to stop keep filename empty).\n" },
    { "macro", cmd_macro, "{key string} {response}",
            "Create or update a macro.\n" },
    { "mute", cmd_mute, "{on/off}",
            "Mutes/De-mutes the command ouput. No argument toggles this flag.\n" },
    { "oneshot", cmd_oneshot, "<time> {response}",
            "Create a timer (in seconds) which is deleted after it expires.\n" },
    { "path", cmd_path, "{path name} {path string}",
            "Create or execute (only the path name) a path.\n" },
    { "read", cmd_read, "{filename}",
            "Read (and execute) a file containing MUDix-commands.\n" },
#if !defined(WIN32)
    { "shell", cmd_shell, "<command>",
            "Execute command in a background shell.\n" },
#endif
    { "stop_timer", cmd_stop_timer, "<time>",
            "Pause all timers with the given time.\n" },
    { "tab", cmd_tab, "{tab name}",
            "Create a tab completion.\n" },
    { "timer", cmd_timer, "<time> <reload> {response}",
            "Create a timer (in seconds) which reloads a number of times.\n" },
    { "trigger", cmd_trigger, "<level> {trigger} {response}",
            "Create a trigger.\n" },
    { "variable", cmd_variable, "{variable name} {value}",
            "Create a variable.\n" },
    { "help", cmd_help, NULL,
            "This help message (help <cmd> for syntax).\n" },
    { "?", cmd_help, NULL,
            "Synonym for help.\n" },
    { NULL, NULL, NULL }
};


static int cmd_lookup(char *cmd)
{
    int i = 0;

    if (*cmd == '\0')
    {
        return -1;
    }

    while (cmd_table[i].cmd)
    {
        if (!strncmp(cmd, cmd_table[i].cmd, strlen(cmd)))
        {
            return i;
        }
        i++;
    }

    return -1;
}


bool do_command(USER *user, char *cmdline)
{
    int   index;

    cmdline = get_arg(user, cmdline, gl_arg1);
    index = cmd_lookup(gl_arg1);

    if (index < 0)
    {
        return FALSE;
    }

    (*(cmd_table[index].function))(user, index, cmdline);
    return TRUE;
}


static void syntax_msg(USER *user, int index)
{
    /* convert the command character to an UTF8 string */
    gl_arg1[g_unichar_to_utf8(user->custom_chars[CUST_CMD_CHAR], gl_arg1)] = '\0';

    sprintf(gl_arg2, "\nSyntax: %s%s %s\n", gl_arg1,
        cmd_table[index].cmd,
        cmd_table[index].syntax);
    gui_info_message(user, gl_arg2);
}


static void cmd_help(USER *user, int index, char *arglist)
{
    get_arg(user, arglist, gl_arg1);

    if ((index = cmd_lookup(gl_arg1)) != -1)
    {
        syntax_msg(user, index);
        sprintf(gl_arg1, "Description:\n%s", cmd_table[index].description);
        gui_info_message(user, gl_arg1);
        return;
    }

    gui_info_message(user, "\nCommand         Description\n");
    gui_info_message(user, "-------         -----------\n");

    index = 0;
    while (cmd_table[index].cmd)
    {
        sprintf(gl_arg1, "%-15s %s", cmd_table[index].cmd,
                                     cmd_table[index].description);
        gui_info_message(user, gl_arg1);
        index++;
    }
}


static void cmd_macro(USER *user, int index, char *arglist)
{
    MACRO *macro;
    guint  key;
    guint  state;
    bool   fExist = FALSE;

    arglist = get_arg(user, arglist, gl_arg1);
    arglist = get_arg(user, arglist, gl_arg2);

    gtk_accelerator_parse(gl_arg1, &key, &state);

    if (!key || gl_arg2[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (macro_lookup(user, key, state))
    {
        fExist = TRUE;
    }

    if (!(macro = create_macro(user, key, state, gl_arg2)))
    {
        exit_mudix();
    }

    if (!fExist)
    {
        if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
        {
            sprintf(gl_arg2, "Macro set for '%s'.\n", gl_arg1);
            gui_info_message(user, gl_arg2);
        }

        gui_macro_add_pref(user, macro);
    }

    return;
}


static void cmd_distrig(USER *user, int index, char *arglist)
{
    TRIGGER *trig;
    int      level, count = 0;

    get_arg(user, arglist, gl_arg1);
    level = atoi(gl_arg1);
    for (trig = user->trigger_list; trig; trig = trig->next)
    {
        if (trig->level == level)
        {
            trig->enabled = FALSE;
            count++;
        }
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg1, "All triggers of level %d disabled (counted %d).\n",
                level, count);
        gui_info_message(user, gl_arg1);
    }

    return;
}


static void cmd_enatrig(USER *user, int index, char *arglist)
{
    TRIGGER *trig;
    int      level, count = 0;

    get_arg(user, arglist, gl_arg1);
    level = atoi(gl_arg1);
    for (trig = user->trigger_list; trig; trig = trig->next)
    {
        if (trig->level == level)
        {
            trig->enabled = TRUE;
            count++;
        }
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg1, "All triggers of level %d enabled (counted %d).\n", level, count);
        gui_info_message(user, gl_arg1);
    }

    return;
}


static void cmd_for(USER *user, int index, char *arglist)
{
    process_for(user, arglist);
    return;
}


static void cmd_if(USER *user, int index, char *arglist)
{
    process_if(user, arglist);
    return;
}


static void cmd_log(USER *user, int index, char *arglist)
{
    get_arg(user, arglist, gl_arg1);
    if (gl_arg1[0])
    {
        get_arg(user, arglist, gl_arg2);

        if (!open_log(user, gl_arg1, gl_arg2[0]? TRUE: FALSE))
        {
            syntax_msg(user, index);
        }
        else if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
        {
            gui_info_message(user, "Log started.\n");
        }
    }
    else
    {
        if (!close_log(user))
        {
            syntax_msg(user, index);
        }
        else if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
        {
            gui_info_message(user, "Log closed.\n");
        }
    }

    return;
}


static void cmd_alias(USER *user, int index, char *arglist)
{
    ALIAS *alias;

    arglist = get_arg(user, arglist, gl_arg1);
    arglist = get_arg(user, arglist, gl_arg2);

    if (gl_arg2[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!(alias = create_alias(user, gl_arg1, gl_arg2)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg2, "Alias '%s' created.\n", gl_arg1);
        gui_info_message(user, gl_arg2);
    }

    gui_alias_add_pref(user, alias);
    return;
}


static void cmd_path(USER *user, int index, char *arglist)
{
    PATH *path;

    arglist = get_arg(user, arglist, gl_arg1);
    arglist = get_arg(user, arglist, gl_arg2);

    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!gl_arg2[0])
    {
        for (path = user->path_list; path; path = path->next)
        {
            if (!strcmp(gl_arg1, path->name))
            {
                break;
            }
        }

        if (!path)
        {
            sprintf(gl_arg2, "Path '%s' not found.\n", gl_arg1);
            gui_info_message(user, gl_arg2);
            return;
        }

        process_path(user, path->path);
        return;
    }

    if (!(path = create_path(user, gl_arg1, gl_arg2)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg2, "Path '%s' created.\n", gl_arg1);
        gui_info_message(user, gl_arg2);
    }

    gui_path_add_pref(user, path);
    return;
}


static void cmd_read(USER *user, int index, char *arglist)
{
    FILE *fp;
    char *line;

    get_arg(user, arglist, gl_arg1);
    if (!gl_arg1[0])
    {
        syntax_msg(user, index);
        return;
    }

    if (!(fp = fopen(gl_arg1, "r")))
    {
        gui_info_message(user, "Failed to open file.\n");
        return;
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg2, "Reading file '%s'.\n", gl_arg1);
        gui_info_message(user, gl_arg2);
    }

    while ((line = fread_to_eol(fp)))
    {
        if (*line == '\0')
        {
            break;
        }
        process_input(user, line, NULL);
    }

    fclose(fp);
    return;
}


static void cmd_tab(USER *user, int index, char *arglist)
{
    TAB *tab;

    get_arg(user, arglist, gl_arg1);

    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!(tab = create_tab(user, gl_arg1)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg2, "Tab completion '%s' created.\n", gl_arg1);
        gui_info_message(user, gl_arg2);
    }

    gui_tab_add_pref(user, tab);
    return;
}


static void cmd_timer(USER *user, int index, char *arglist)
{
    TIMER *timer;
    int    time, reload;

    arglist = get_arg(user, arglist, gl_arg1);
    time    = atoi(gl_arg1);

    arglist = get_arg(user, arglist, gl_arg1);
    reload  = atoi(gl_arg1);

    get_arg(user, arglist, gl_arg1);
    if (gl_arg1[0] == '\0' || time < 0)
    {
        syntax_msg(user, index);
        return;
    }

    if (!(timer = create_timer(user, gl_arg1, time, reload)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg1, "Timer created for %ds (%d times).\n", time, reload);
        gui_info_message(user, gl_arg1);
    }

    gui_timer_add_pref(user, timer);
    return;
}


static void cmd_oneshot(USER *user, int index, char *arglist)
{
    TIMER *timer;
    int    time;

    arglist = get_arg(user, arglist, gl_arg1);
    time    = atoi(gl_arg1);

    get_arg(user, arglist, gl_arg1);
    if (gl_arg1[0] == '\0' || time < 0)
    {
        syntax_msg(user, index);
        return;
    }

    if (!(timer = create_timer(user, gl_arg1, time, TIMER_ONESHOT)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg1, "Oneshot timer created for %ds.\n", time);
        gui_info_message(user, gl_arg1);
    }

    gui_timer_add_pref(user, timer);
    return;
}


static void cmd_stop_timer(USER *user, int index, char *arglist)
{
    TIMER *timer;
    int    time, count=0;

    get_arg(user, arglist, gl_arg1);

    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!strcmp(gl_arg1, "all"))
    {
        time = -1;
    }
    else
    {
        time = atoi(gl_arg1);
    }

    for (timer = user->timer_list; timer; timer = timer->next)
    {
        if (timer->relval == time || time == -1)
        {
            /* found a timer - stop it */
            timer->timer = 0;
            count++;
        }
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        if (time == -1)
        {
            sprintf(gl_arg1, "Stopped all timers (found %d).\n", count);
        }
        else
        {
            sprintf(gl_arg1, "Stopped all timers with a time of %ds (found %d).\n", time, count);
        }

        gui_info_message(user, gl_arg1);
    }

    gui_timer_update_list(user);
    return;
}


static void cmd_cont_timer(USER *user, int index, char *arglist)
{
    TIMER *timer;
    int    time, count=0;

    get_arg(user, arglist, gl_arg1);

    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!strcmp(gl_arg1, "all"))
    {
        time = -1;
    }
    else
    {
        time = atoi(gl_arg1);
    }

    for (timer = user->timer_list; timer; timer = timer->next)
    {
        if (timer->relval == time || time == -1)
        {
            /* found a timer - start it */
            timer->timer = timer->relval;
            count++;
        }
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        if (time == -1)
        {
            sprintf(gl_arg1, "Started all timers (found %d).\n", count);
        }
        else
        {
            sprintf(gl_arg1, "Started all timers with a time of %ds (found %d).\n", time, count);
        }

        gui_info_message(user, gl_arg1);
    }

    gui_timer_update_list(user);
    return;
}


static void cmd_trigger(USER *user, int index, char *arglist)
{
    TRIGGER *trigger;
    int      level;

    arglist = get_arg(user, arglist, gl_arg1);
    level   = atoi(gl_arg1);

    arglist = get_arg(user, arglist, gl_arg1);
    arglist = get_arg(user, arglist, gl_arg2);
    if (gl_arg2[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (!(trigger = create_trigger(user, level, gl_arg1, gl_arg2)))
    {
        exit_mudix();
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        sprintf(gl_arg1, "Trigger created for level %d.\n", level);
        gui_info_message(user, gl_arg1);
    }

    gui_trigger_add_pref(user, trigger);
    return;
}


static void cmd_variable(USER *user, int index, char *arglist)
{
    VAR    *var;
    bool    fExist = FALSE;

    arglist = get_arg(user, arglist, gl_arg1);
    arglist = get_arg(user, arglist, gl_arg2);

    if (gl_arg2[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    if (var_lookup(user, gl_arg1))
    {
        fExist = TRUE;
    }

    if (!(var = create_var(user, gl_arg1, gl_arg2)))
    {
        exit_mudix();
    }

    if (!fExist)
    {
        if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
        {
            sprintf(gl_arg1, "Variable '%s' created.\n", var->name);
            gui_info_message(user, gl_arg1);
        }

        gui_var_add_pref(user, var);
    }

    return;
}


static void cmd_echo(USER *user, int index, char *args)
{
    int color;

    args = get_arg(user, args, gl_arg1);
    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    color = atoi(gl_arg1) & (NR_ANSI_COLORS-1);
    if (color == COLOR_BLACK)
    {
        color = COLOR_INFO;
    }

    gui_add_color_window(user, args, strlen(args), color);
}


static void cmd_command(USER *user, int index, char *args)
{
    USER *lookup;

    args = get_arg(user, args, gl_arg1);
    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    /* grab the user with that ID */
    lookup = get_user_id(atoi(gl_arg1));

    if (!lookup)
    {
        gui_info_message(user, "No user found with that ID.\n");
    }
    else if (!lookup->gui_user.g_window)
    {
        gui_info_message(user, "User has no user-window.\n");
    }
    else
    {
        /* remove the newline at the end of the input */
        if (args[strlen(args)-1] == '\n')
        {
            args[strlen(args)-1] = '\0';
        }

        process_input(lookup, args, NULL);
    }
}


static void cmd_capture(USER *user, int index, char *args)
{
    char *argument;
    int   color;

    args = get_arg(user, args, gl_arg1);
    if (gl_arg1[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    args = get_arg(user, args, gl_arg2);
    if (gl_arg2[0] == '\0')
    {
        syntax_msg(user, index);
        return;
    }

    color = atoi(gl_arg2) & (NR_ANSI_COLORS-1);
    if (color == COLOR_BLACK)
    {
        color = COLOR_INFO;
    }

    get_arg(user, args, gl_arg2);

    if (!gl_arg2[0])
    {
        *user->trigger = '\n';
        *(user->trigger+1) = '\0';
        argument = user->trigger_buf;
    }
    else
    {
        argument = args;
    }

    gui_add_to_capt(user, gl_arg1, argument, strlen(argument), color);
}


void cmd_mute(USER *user, int index, char *args)
{
    get_arg(user, args, gl_arg1);

    if (!gl_arg1[0])
    {
        /* toggle the mute flag */
        user->flags ^= FLG_CMD_OUTPUT_MUTE;
    }
    else if (!strcmp(gl_arg1, "on"))
    {
        /* set the mute flag */
        user->flags |= FLG_CMD_OUTPUT_MUTE;
    }
    else if (!strcmp(gl_arg1, "off"))
    {
        /* clear the mute flag */
        user->flags &= ~FLG_CMD_OUTPUT_MUTE;
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        gui_info_message(user, "Command output de-muted.\n");
    }
    else
    {
        gui_info_message(user, "Command output muted.\n");
    }
}


void cmd_input_echo(USER *user, int index, char *args)
{
    get_arg(user, args, gl_arg1);

    if (!gl_arg1[0])
    {
        /* toggle the input echo flag */
        user->flags ^= FLG_INPUT_ECHO_OFF;
    }
    else if (!strcmp(gl_arg1, "on"))
    {
        /* clear the echo-off flag */
        user->flags &= ~FLG_INPUT_ECHO_OFF;
    }
    else if (!strcmp(gl_arg1, "off"))
    {
        /* set the echo-off flag */
        user->flags |= FLG_INPUT_ECHO_OFF;
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        if (!(user->flags & FLG_INPUT_ECHO_OFF))
        {
            gui_info_message(user, "Input echo enabled.\n");
        }
        else
        {
            gui_info_message(user, "Input echo disabled.\n");
        }
    }
}


void cmd_input_clear(USER *user, int index, char *args)
{
    get_arg(user, args, gl_arg1);

    if (!gl_arg1[0])
    {
        /* toggle the input-clear flag */
        user->flags ^= FLG_INPUT_AUTO_CLEAR;
    }
    else if (!strcmp(gl_arg1, "on"))
    {
        /* set the input-clear flag */
        user->flags |= FLG_INPUT_AUTO_CLEAR;
    }
    else if (!strcmp(gl_arg1, "off"))
    {
        /* clear the input-clear flag */
        user->flags &= ~FLG_INPUT_AUTO_CLEAR;
    }

    if (!(user->flags & FLG_CMD_OUTPUT_MUTE))
    {
        if (!(user->flags & FLG_INPUT_AUTO_CLEAR))
        {
            gui_info_message(user, "Input auto-clear disabled.\n");
        }
        else
        {
            gui_info_message(user, "Input auto-clear enabled.\n");
        }
    }
}


#if !defined(WIN32)
void cmd_shell(USER *user, int index, char *args)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        /* fork failed */
        return;
    }

    if (pid == 0)
    {
        /* this is the child */
        char *argv[4];

        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = args;
        argv[3] = 0;
        execv("/bin/sh", argv);
        exit(127);
    }
    else
    {
        /* this is the parent */
    }
}
#endif


void cmd_cls(USER *user, int index, char *args)
{
    get_arg(user, args, gl_arg1);

    if (gl_arg1[0])
    {
        if (gui_cls_capt(user, gl_arg1) && !(user->flags & FLG_CMD_OUTPUT_MUTE))
        {
            gui_info_message(user, "Capture window cleared.\n");
        }
    }
    else
    {
        gui_user_cls(user);
    }
}
