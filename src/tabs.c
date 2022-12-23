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


TAB *create_tab(USER *user, gchar *name)
{
    TAB *tab;

    if (!(tab = new_tab_append(user)))
    {
        return NULL;
    }

    tab->name = strdup(name);

    return tab;
}


TAB *new_tab(USER *user)
{
    TAB *tabs;

    if (!(tabs = malloc(sizeof(TAB))))
    {
        return NULL;
    }

    tabs->next      = user->tabs_list;
    user->tabs_list = tabs;
    tabs->name      = NULL;

    return tabs;
}


TAB *new_tab_append(USER *user)
{
    TAB *tabs;
    TAB *iTab;

    if (!(tabs = malloc(sizeof(TAB))))
    {
        return NULL;
    }

    for (iTab = user->tabs_list; iTab; iTab = iTab->next)
    {
        if (!iTab->next)
        {
            break;
        }
    }

    if (iTab)
    {
        iTab->next = tabs;
    }
    else
    {
        user->tabs_list = tabs;
    }

    tabs->next      = NULL;
    tabs->name      = NULL;

    return tabs;
}


void free_tab(USER *user, TAB *pTabs)
{
    TAB *tabs;

    if (!pTabs)
    {
	return;
    }

    if (pTabs == user->tabs_list)
    {
        user->tabs_list = pTabs->next;
    }
    else
    {
        for (tabs = user->tabs_list; tabs; tabs = tabs->next) 
        {
    	    if (tabs->next == pTabs) 
            {
                tabs->next = pTabs->next;
                break;
            }
        }
    }

    if (pTabs->name)
    {
	free(pTabs->name);
    }

    free(pTabs);
}


void cleanup_tabs_list(USER *user)
{
    TAB *tab, *tab_next;

    for (tab = user->tabs_list; tab; tab = tab_next)
    {
        tab_next = tab->next;

        free_tab(user, tab);
    }
}


void check_tab(USER *user, gchar *buffer)
{
    gchar  input[MAX_INPUT_LEN];
    TAB   *tabs;
    gchar *pName;
    int    i = 0, len = 1;
 
    /* TODO: change this for UTF8? */
    pName = buffer + strlen(buffer);
    pName--;
    if (*pName == ' ')
    {
	return;
    }

    while (pName > buffer && *(pName-1) != ' ')
    {
        len++;
	pName--;
    }

    for (tabs = user->tabs_list; tabs; tabs = tabs->next) 
    {
	for (i=0; i<len; i++)
        {
	    if (tolower(tabs->name[i]) != tolower(pName[i]))
            {
		break;
            }
        }

	if (i == len)
        {
	    break;
        }
    }

    if (!tabs)
    {
	return;
    }
   
    /* new string too large? */
    if (strlen(buffer) + strlen(&tabs->name[i]) >= MAX_INPUT_LEN)
    {
        return;
    }

    strcpy(input, buffer);
    strcat(input, &tabs->name[i]);
    gui_input_set(user, input);
    gui_input_cursor_end(user);
    return;
}
