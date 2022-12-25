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


TRIGGER *create_trigger(USER *user, int level, gchar *text, gchar *response)
{
    TRIGGER *trigger;

    if (!(trigger = new_trigger_append(user, level)))
    {
        return NULL;
    }

    trigger->input    = strdup(text);
    trigger->pArg     = trigger->arg;
    trigger->response = strdup(response);
    trigger->enabled  = TRUE;

    return trigger;
}


void set_trigger(USER *user, TRIGGER *trigger, gchar *text, gchar *response)
{
    if (trigger->input)
    {
        free(trigger->input);
    }

    trigger->input = strdup(text);

    if (trigger->response)
    {
        free(trigger->response);
    }

    trigger->response = strdup(response);
}


TRIGGER *new_trigger(USER *user, int level)
{
    TRIGGER *trigger;

    if (!(trigger = calloc(1, sizeof(TRIGGER))))
    {
        return NULL;
    }

    trigger->next      = user->trigger_list;
    user->trigger_list = trigger;

    trigger->pArg      = trigger->arg;
    trigger->enabled   = FALSE;
    trigger->level     = level;

    return trigger;
}


TRIGGER *new_trigger_append(USER *user, int level)
{
    TRIGGER *trigger;
    TRIGGER *iTrig;

    if (!(trigger = calloc(1, sizeof(TRIGGER))))
    {
        return NULL;
    }

    for (iTrig = user->trigger_list; iTrig; iTrig = iTrig->next)
    {
        if (!iTrig->next)
        {
            break;
        }
    }

    if (iTrig)
    {
        iTrig->next = trigger;
    }
    else
    {
        user->trigger_list = trigger;
    }

    trigger->pArg      = trigger->arg;
    trigger->enabled   = FALSE;
    trigger->level     = level;

    return trigger;
}


void free_trigger(USER *user, TRIGGER *trigger)
{
    TRIGGER *lookup;

    if (!trigger)
    {
        return;
    }

    if (trigger == user->trigger_list)
    {
        user->trigger_list = trigger->next;
    }
    else
    {
        for (lookup = user->trigger_list; lookup; lookup = lookup->next)
        {
            if (lookup->next == trigger)
            {
                lookup->next = trigger->next;
                break;
            }
        }
    }

    if (trigger->input)
    {
        free(trigger->input);
    }

    if (trigger->response)
    {
        free(trigger->response);
    }

    free(trigger);
}


void cleanup_trigger_list(USER *user)
{
    TRIGGER *trigger, *trigger_next;

    for (trigger = user->trigger_list; trigger; trigger = trigger_next)
    {
        trigger_next = trigger->next;

        free_trigger(user, trigger);
    }
}


TRIGGER *trig_lookup(USER *user, TRIGGER *beg, int level)
{
    TRIGGER *trig;

    for (trig = (beg? beg: user->trigger_list); trig; trig = trig->next)
    {
        if (trig->level == level)
        {
            break;
        }
    }

    return trig;
}


void trigger_check(USER *user, bool fNewline)
{
    TRIGGER     *trig;
    gchar       *pBuf, *pTrg, *pArg, *pMrk, *pStart;
    gunichar     var_sign, block_open, block_close;
    gunichar     cTrg;
    bool         fReset = FALSE;

    /* grab the customizable characters for this user */
    var_sign    = user->custom_chars[CUST_VAR_SIGN];
    block_open  = user->custom_chars[CUST_BLOCK_OPEN];
    block_close = user->custom_chars[CUST_BLOCK_CLOSE];

    if (user->trigger == user->trigger_buf)
    {
        /* nothing in the buffer */
        return;
    }

    for (trig = user->trigger_list; trig; trig = trig->next)
    {
        /*
         * skip if this trigger is disabled or
         * if level < 0 and it's a newline trigger or
         * if level >= 0 and it's not a newline trigger
         */
        if ( !trig->enabled
          || (trig->level < 0 && fNewline)
          || (trig->level >= 0 && !fNewline))
        {
            continue;
        }

        cTrg   = g_utf8_get_char(pTrg = trig->input);
        pBuf   = user->trigger_buf;
        pArg   = trig->pArg;
        pMrk   = NULL;
        pStart = NULL;
        while (pBuf < user->trigger)
        {
            gunichar cBuf = g_utf8_get_char(pBuf);

            /* not inside an argument, and buffer unequal to trigger? */
            if (cTrg != cBuf && cTrg != var_sign && pArg == trig->pArg)
            {
                /* backup pointers */
                pArg   = trig->pArg = trig->arg;
                cTrg   = g_utf8_get_char(pTrg = trig->input);
                pMrk   = NULL;
                pStart = NULL;
            }

            /* check for a variable */
            if (cTrg == var_sign)
            {
                /* get next char */
                cTrg = g_utf8_get_char(pTrg = g_utf8_next_char(pTrg));

                if (g_unichar_isdigit(cTrg))
                {
                    /* if we are already within another variable, close that one */
                    if (pArg != trig->pArg)
                    {
                        pArg += g_unichar_to_utf8(block_close, pArg);
                        trig->pArg = pArg;
                    }

                    /* did we place a marker? if so clear it */
                    if (pMrk)
                    {
                        pMrk = NULL;
                    }

                    /* start a new argument in our arg buffer */
                    pArg += g_unichar_to_utf8(block_open, pArg);
                }

                /* get next char */
                cTrg = g_utf8_get_char(pTrg = g_utf8_next_char(pTrg));
           }

            /* is the character in the trigger unequal to the incoming buffer? */
            if (cTrg != cBuf)
            {
                /* yes, characters not the same... check if in an argument */
                if (pArg != trig->pArg)
                {
                    /* yup, did we place a marker? */
                    if (pMrk)
                    {
                        gchar *pTmp = pMrk;

                        /* copy everything from the marker into argument */
                        while (pTmp != pTrg)
                        {
                            *pArg++ = *pTmp++;
                            if ((pArg - trig->arg) >= MAX_TRIG_ARG_LEN)
                            {
                                break;
                            }
                        }

                        /* backup the trigger pointer */
                        cTrg = g_utf8_get_char(pTrg = pMrk);
                        pMrk = NULL;
                    }

                    /* copy character from buf into argument */
                    pArg += g_unichar_to_utf8(cBuf, pArg);
                }
            }
            else
            {
                /* characters are equal: if marker's not set, set it now */
                if (!pMrk && pArg != trig->pArg)
                {
                    pMrk = pTrg;
                }

                if (!pStart)
                {
                    pStart = pBuf;
                }

                /* get next char */
                cTrg = g_utf8_get_char(pTrg = g_utf8_next_char(pTrg));
            }

            /* is last argument passing max arg length? */
            if ((pArg - trig->arg) >= MAX_TRIG_ARG_LEN)
            {
                /* set argument pointer to start of last argument */
                pArg = trig->pArg;
            }

            /* advance pointer */
            pBuf = g_utf8_next_char(pBuf);

            if (cTrg == '\0')
            {
                /* trigger reached the end of its buffer */
                if (pArg != trig->pArg && !pMrk)
                {
                    /* in case there's a variable at the end, get remaining text */
                    while (pBuf != user->trigger)
                    {
                        if ((pArg - trig->arg) >= MAX_TRIG_ARG_LEN)
                        {
                            break;
                        }

                        *pArg++ = *pBuf++;
                    }
                }
                *pArg = '\0';

                /* call the input processor */
                process_input(user, trig->response, (pArg != trig->arg)? trig->arg: NULL);

                /* disable triggers of level LOGIN and PASSWORD */
                if (trig->level == TRG_LOGIN || trig->level == TRG_PASSWORD)
                {
                    trig->enabled = FALSE;
                }

                if (trig->level < 0)
                {
                    /* negative trigger, then also reset the trigger buffer */
                    fReset = TRUE;
                }

                /* update the pointers */
                pTrg = trig->input;
                pArg = trig->pArg = trig->arg;
                pMrk = NULL;
            }
        }
    }

    /* re-set pointer, new characters will be stored in start of buffer */
    if (fNewline || fReset)
    {
        user->trigger = user->trigger_buf;
    }

    return;
}
