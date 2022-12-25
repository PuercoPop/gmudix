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


VAR *create_var(USER *user, gchar *name, gchar *value)
{
    VAR *var;

    if (!(var = var_lookup(user, name)) &&
        !(var = new_var_append(user)))
    {
        return NULL;
    }

    if (var->name)
    {
        free(var->name);
    }
    var->name = strdup(name);

    if (var->value)
    {
        free(var->value);
    }
    var->value = strdup(value);

    return var;
}


VAR *new_var(USER *user)
{
    VAR *var;

    if (!(var = malloc(sizeof(VAR))))
    {
        return NULL;
    }

    var->next       = user->vars_list;
    user->vars_list = var;
    var->name       = NULL;
    var->value      = NULL;

    return var;
}


VAR *new_var_append(USER *user)
{
    VAR *var;
    VAR *iVar;

    if (!(var = malloc(sizeof(VAR))))
    {
        return NULL;
    }

    for (iVar = user->vars_list; iVar; iVar = iVar->next)
    {
        if (!iVar->next)
        {
            break;
        }
    }

    if (iVar)
    {
        iVar->next = var;
    }
    else
    {
        user->vars_list = var;
    }

    var->next       = NULL;
    var->name       = NULL;
    var->value      = NULL;

    return var;
}


void free_var(USER *user, VAR *var)
{
    VAR *iVar;

    if (!var)
    {
        return;
    }

    if (var == user->vars_list)
    {
        user->vars_list = var->next;
    }
    else
    {
        for (iVar = user->vars_list; iVar; iVar = iVar->next)
        {
            if (iVar->next == var)
            {
                iVar->next = var->next;
                break;
            }
        }
    }

    if (var->name)
    {
        free(var->name);
    }

    if (var->value)
    {
        free(var->value);
    }

    free(var);
}


void cleanup_vars_list(USER *user)
{
    VAR *var, *var_next;

    for (var = user->vars_list; var; var = var_next)
    {
        var_next = var->next;

        free_var(user, var);
    }
}


VAR *var_lookup(USER *user, gchar *var)
{
    VAR *iVar;

    /* if the variable starts with a variable sign skip over it */
    if (*var == (gchar)user->custom_chars[CUST_VAR_SIGN])
    {
        var = g_utf8_next_char(var);
    }

    /* look up the variable */
    for (iVar = user->vars_list; iVar; iVar = iVar->next)
    {
    	if (!strcmp(var, iVar->name))
        {
	    break;
        }
    }

    return iVar;
}
