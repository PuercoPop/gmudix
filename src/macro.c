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


/* these macros only work on X */
#if !defined(WIN32)

typedef struct
{
    guint   key;
    guint   state;
    gchar  *text;
} MACRO_DFLT;


MACRO_DFLT dflt_macros[] =
{
    /* key              state               text */
    {GDK_KP_Subtract,   GDK_MOD2_MASK,      "u"},
    {GDK_KP_Add,        GDK_MOD2_MASK,      "d"},
    {GDK_KP_8,          GDK_MOD2_MASK,      "n"},
    {GDK_KP_7,          GDK_MOD2_MASK,      "nw"},
    {GDK_KP_9,          GDK_MOD2_MASK,      "ne"},
    {GDK_KP_4,          GDK_MOD2_MASK,      "w"},
    {GDK_KP_6,          GDK_MOD2_MASK,      "e"},
    {GDK_KP_1,          GDK_MOD2_MASK,      "sw"},
    {GDK_KP_2,          GDK_MOD2_MASK,      "s"},
    {GDK_KP_3,          GDK_MOD2_MASK,      "se"},
    {0,                 0,                  NULL},
};

#endif

MACRO *create_macro(USER *user, guint key, guint state, gchar *text)
{
    MACRO *macro;

    if (!(macro = macro_lookup(user, key, state)) &&
        !(macro = new_macro_append(user)))
    {
        return NULL;
    }

    if (macro->text)
    {
        free(macro->text);
    }

    macro->key   = key;
    macro->state = state;
    macro->text  = strdup(text);

    return macro;
}


MACRO *new_macro(USER *user)
{
    MACRO *macro;

    if (!(macro = malloc(sizeof(MACRO))))
    {
        return NULL;
    }

    macro->next      = user->macro_list;
    user->macro_list = macro;
    macro->key       = 0;
    macro->state     = 0;
    macro->text      = NULL;

    return macro;
}


MACRO *new_macro_append(USER *user)
{
    MACRO *macro;
    MACRO *iMacro;

    if (!(macro = malloc(sizeof(MACRO))))
    {
        return NULL;
    }

    for (iMacro = user->macro_list; iMacro; iMacro = iMacro->next)
    {
        if (!iMacro->next)
        {
            break;
        }
    }

    if (iMacro)
    {
        iMacro->next = macro;
    }
    else
    {
        user->macro_list = macro;
    }

    macro->next      = NULL;
    macro->key       = 0;
    macro->state     = 0;
    macro->text      = NULL;

    return macro;
}


void free_macro(USER *user, MACRO *pMacro)
{
    MACRO *macro;

    if (!pMacro)
    {
	return;
    }

    if (pMacro == user->macro_list)
    {
        user->macro_list = pMacro->next;
    }
    else
    {
        for (macro = user->macro_list; macro; macro = macro->next)
        {
    	    if (macro->next == pMacro)
            {
                macro->next = pMacro->next;
                break;
            }
        }
    }

    if (pMacro->text)
    {
    	free(pMacro->text);
    }
    free(pMacro);
}


void cleanup_macro_list(USER *user)
{
    MACRO *macro, *macro_next;

    for (macro = user->macro_list; macro; macro = macro_next)
    {
        macro_next = macro->next;

        free_macro(user, macro);
    }
}


void setup_default_macros(USER *user)
{
/* these macros only work on X */
#if !defined(WIN32)
    MACRO *macro;
    int    i;

    for (i=0; dflt_macros[i].text; i++)
    {
        macro        = new_macro(user);
        macro->key   = dflt_macros[i].key;
        macro->state = dflt_macros[i].state;
        macro->text  = strdup(dflt_macros[i].text);
    }
#endif
}


MACRO *macro_lookup(USER *user, guint key, guint state)
{
    MACRO *iMacro;

    for (iMacro = user->macro_list; iMacro; iMacro = iMacro->next)
    {
    	if (iMacro->key == key && iMacro->state == state)
        {
	    break;
        }
    }

    return iMacro;
}


bool process_macro(USER *user, guint key, guint state)
{
    MACRO *macro;

    macro = macro_lookup(user, key, state);

    /* process it (can also be an alias) */
    if (macro)
    {
        if (!process_alias(user, macro->text))
        {
    	    process_input(user, macro->text, NULL);
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
