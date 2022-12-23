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


/* 1 second timer */
static gboolean update_timers(gpointer data)
{
    USER  *user = (USER *)data;
    TIMER *timer, *timer_next;

    for (timer = user->timer_list; timer; timer = timer_next)
    {
        timer_next = timer->next;

        /* check if timer may tick (not 0) */
        if (!timer->timer)
        {
            continue;
        }

        /* see if timer expired */
        if (--timer->timer <= 0)
        {
            /* process the response of the timer */
            process_input(user, timer->response, NULL);

            /* may this timer reload? */
            if (timer->relcnt == TIMER_CONT)
            {
                /* continuous reload */
                timer->timer = timer->relval;
            }
            else if (timer->relcnt == TIMER_ONESHOT)
            {
                gui_timer_remove(user, timer);
		
                free_timer(user, timer);
            }
            else if (timer->relcnt > 0)
            {
                /* reload while reload count is greater than 0 */
                timer->timer = timer->relval;
                timer->relcnt--;
            }
        }
    }

    /* if the timer preference window is open update it */
    gui_timer_update_list(user);

    return TRUE;   /* keep timer alive */
}


void sync_timer(USER *user)
{
    /* delete the old timer */
    gtk_timeout_remove(user->timer_id);

    /* create a new timer */
    user->timer_id = init_timer(user);
}


guint init_timer(USER *user)
{
    /* create a 1 second timer for updating the timers */
    return gtk_timeout_add(TIMER_TIMEOUT, update_timers, user);
}


TIMER *create_timer(USER *user, gchar *resp, gint time, gint reload)
{
    TIMER *timer;

    if (!(timer = new_timer_append(user)))
    {
        return NULL;
    }

    timer->response  = strdup(resp);
    timer->timer     = time;
    timer->relval    = time;
    timer->relcnt    = reload;

    return timer;
}


void set_timer(USER *user, TIMER *timer, gchar *resp, gint time, gint reload)
{
    if (timer->response)
    {
        free(timer->response);
    }

    timer->response  = strdup(resp);
    timer->timer     = time;
    timer->relval    = time;
    timer->relcnt    = reload;
}


TIMER *new_timer(USER *user)
{
    TIMER *timer;

    if (!(timer = malloc(sizeof(TIMER))))
    {
	return NULL;
    }
     
    timer->next      = user->timer_list;
    user->timer_list = timer;

    timer->response  = NULL;
    timer->timer     = 0;
    timer->relval    = 0;
    timer->relcnt    = 0;

    return timer;
}


TIMER *new_timer_append(USER *user)
{
    TIMER *timer;
    TIMER *iTimer;

    if (!(timer = malloc(sizeof(TIMER))))
    {
	return NULL;
    }

    for (iTimer = user->timer_list; iTimer; iTimer = iTimer->next)
    {
        if (!iTimer->next)
        {
            break;
        }
    }

    if (iTimer)
    {
        iTimer->next = timer;
    }
    else
    {
        user->timer_list = timer;
    }

    timer->next      = NULL;
    timer->response  = NULL;
    timer->timer     = 0;
    timer->relval    = 0;
    timer->relcnt    = 0;

    return timer;
}


void free_timer(USER *user, TIMER *timer)
{
    TIMER *lookup;

    if (!timer)
    {
	return;
    }

    if (timer == user->timer_list)
    {
        user->timer_list = timer->next;
    }
    else
    {
        for (lookup = user->timer_list; lookup; lookup = lookup->next) 
        {
            if (lookup->next == timer) 
            {
                lookup->next = timer->next;
                break;
            }
        }
    }

    if (timer->response)
    {
        free(timer->response);
    }

    free(timer);
    return;
}


void cleanup_timer_list(USER *user)
{
    TIMER *timer, *timer_next;

    for (timer = user->timer_list; timer; timer = timer_next)
    {
        timer_next = timer->next;

        free_timer(user, timer);
    }
}
