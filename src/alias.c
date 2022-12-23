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


ALIAS *create_alias(USER *user, char *name, char *text)
{
    ALIAS *alias;

    if (!(alias = new_alias_append(user)))
    {
        return NULL;
    }

    alias->name   = strdup(name);
    alias->string = strdup(text);

    return alias;
}


ALIAS *new_alias(USER *user)
{
    ALIAS *alias;

    if (!(alias = malloc(sizeof(ALIAS))))
    {
        return NULL;
    }

    alias->next       = user->alias_list;
    user->alias_list  = alias;
    alias->name       = NULL;
    alias->string     = NULL;

    return alias;
}


ALIAS *new_alias_append(USER *user)
{
    ALIAS *alias;
    ALIAS *iAlias;

    if (!(alias = malloc(sizeof(ALIAS))))
    {
        return NULL;
    }

    for (iAlias = user->alias_list; iAlias; iAlias = iAlias->next)
    {
        if (!iAlias->next)
        {
            break;
        }
    }

    if (iAlias)
    {
        iAlias->next = alias;
    }
    else
    {
        user->alias_list = alias;
    }

    alias->next       = NULL;
    alias->name       = NULL;
    alias->string     = NULL;

    return alias;
}


void free_alias(USER *user, ALIAS *name)
{
    ALIAS *alias;

    if (!name)
    {
	return;
    }

    for (alias = user->alias_list; alias; alias = alias->next) 
    {
    	if (alias->next == name) 
        {
            alias->next = name->next;
            break;
        }
    }

    if (name == user->alias_list)
    {
        user->alias_list = name->next;
    }

    if (name->name)
    {
	free(name->name);
    }

    if (name->string)
    {
    	free(name->string);
    }

    free(name);
    return;
}


void cleanup_alias_list(USER *user)
{
    ALIAS *alias, *alias_next;

    for (alias = user->alias_list; alias; alias = alias_next)
    {
        alias_next = alias->next;

        free_alias(user, alias);
    }
}


bool process_alias(USER *user, char *buffer)
{
           ALIAS *pAlias;
    static char   alias[MAX_STRING];
    static int    nested;

    /* too many nests? */
    if (nested > MAX_ALIAS_NESTS)
    {
        return FALSE;
    }

    /* grab the alias from the input and look it up */
    buffer = get_arg(user, buffer, alias);
    for (pAlias = user->alias_list; pAlias; pAlias = pAlias->next) 
    {
        if (strcmp(pAlias->name, alias))
        {
            continue;
        }

        /* alias found, process it and supply arguments */
        get_arg(user, buffer, alias);

        /* increase nested as we may only allow a max number of nests */
        nested++;

        /* process the alias */
        process_input(user, pAlias->string, alias[0]? buffer: NULL);

        /* decrease nested as we returned from the input processor */
        nested--;
        return TRUE;
    }

    return FALSE;
}


