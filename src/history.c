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
#include "mudix.h"


void init_history(USER *user)
{
    HISTORY   *prev  = NULL;
    HISTORY   *first = NULL;
    HISTORY   *hist;
    int        i;

    for (i=0; i<MAX_HISTORY; i++) 
    {
        hist = malloc(sizeof(HISTORY));

        hist->prev = prev;
        hist->str  = NULL;

        if (prev)
        {
            prev->next = hist;
        }
        else
        {
            first = hist;
        }

        prev = hist;
    }

    first->prev = hist;
    hist->next  = first;

    /* just initialize the pointers to the first */
    user->pCurHist = first;
    user->pGetHist = first;
}


void cleanup_history(USER *user)
{
    HISTORY *pHist = user->pCurHist->next;
    HISTORY *hist_next;

    for (pHist  = user->pCurHist->next;
         pHist != user->pCurHist;
         pHist  = hist_next)
    {
        hist_next = pHist->next;

        if (pHist->str)
        {
            free(pHist->str);
        }
    
        free(pHist);
    }
}


void add_to_history(USER *user, gchar *input)
{
    HISTORY *pHist;
    GList   *items = NULL;

    /* do not add short inputs to the history buffer */
    if (g_utf8_strlen(input, -1) < 3)
    {
        return;
    }

    /* get a pointer to the previous history */
    pHist = user->pCurHist->prev;

    if (pHist->str)
    {
        /* if same as last command do not insert into history buffer */
        if (g_utf8_collate(input, pHist->str) == 0)
        {
          return;
        }
    }

    if (user->pCurHist->str)
    {
        /* free string if slot was already taken */
        free(user->pCurHist->str);
    }

    /* just duplicate string into the circular history buffer */
    user->pCurHist->str = strdup(input);
    user->pGetHist      = user->pCurHist = user->pCurHist->next;

    /* set the drag down menu */
    for (pHist  = user->pCurHist->prev; 
         pHist != user->pCurHist && pHist->str;
         pHist  = pHist->prev)
    {
        items = g_list_append(items, pHist->str);
    }

    gtk_combo_set_popdown_strings(GTK_COMBO(user->gui_user.g_combo), items);
}

