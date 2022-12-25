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


PATH *create_path(USER *user, gchar *name, gchar *text)
{
    PATH *path;

    if (!(path = new_path_append(user)))
    {
        return NULL;
    }

    path->name = strdup(name);
    path->path = strdup(text);

    return path;
}


PATH *new_path(USER *user)
{
    PATH *path;

    if (!(path = malloc(sizeof(PATH))))
    {
        return NULL;
    }

    path->next      = user->path_list;
    user->path_list = path;
    path->name      = NULL;
    path->path      = NULL;

    return path;
}


PATH *new_path_append(USER *user)
{
    PATH  *path;
    PATH  *iPath;

    if (!(path = malloc(sizeof(PATH))))
    {
        return NULL;
    }

    for (iPath = user->path_list; iPath; iPath = iPath->next)
    {
        if (!iPath->next)
        {
            break;
        }
    }

    if (iPath)
    {
        iPath->next = path;
    }
    else
    {
        user->path_list = path;
    }

    path->next      = NULL;
    path->name      = NULL;
    path->path      = NULL;

    return path;
}


void free_path(USER *user, PATH *pPath)
{
    PATH *path;

    if (!pPath)
    {
	return;
    }

    if (pPath == user->path_list)
    {
        user->path_list = pPath->next;
    }
    else
    {
        for (path = user->path_list; path; path = path->next)
        {
    	    if (path->next == pPath)
            {
                path->next = pPath->next;
                break;
            }
        }
    }

    if (pPath->name)
    {
	free(pPath->name);
    }

    if (pPath->path)
    {
    	free(pPath->path);
    }

    free(pPath);
}


void cleanup_path_list(USER *user)
{
    PATH *path, *path_next;

    for (path = user->path_list; path; path = path_next)
    {
        path_next = path->next;

        free_path(user, path);
    }
}


bool process_path(USER *user, gchar *buffer)
{
    static gchar    path[MAX_STRING];
           gchar    digit[5];
	   int      dig_cnt=0, count;
           gunichar c, block_open;
           bool     retval = FALSE;

    /* grab the block_open character for this user */
    block_open = user->custom_chars[CUST_BLOCK_OPEN];

    /* parse through the incoming buffer */
    while ((c = g_utf8_get_char(buffer)) != '\0')
    {
        if (c == block_open)
        {
            /* retrieve the argument in the block */
            buffer = get_arg(user, buffer, path);

            /* get the count */
            if (dig_cnt)
            {
                digit[dig_cnt] = '\0';
                dig_cnt        = 0;
                count          = atoi(digit);
            }
            else
            {
                count = 1;
            }

            /* process the input count times */
            while (count--)
            {
                process_input(user, path, NULL);
            }

            retval = TRUE;
        }
        else
        {
            switch (c)
            {
	        case 'n':
	        case 'e':
	        case 's':
	        case 'w':
	        case 'u':
	        case 'd':
                    /* just set up path to walk in one direction */
                    path[0] = (char)c;
                    path[1] = '\0';

                    /* get the count */
                    if (dig_cnt)
                    {
                        digit[dig_cnt] = '\0';
                        dig_cnt        = 0;
                        count          = atoi(digit);
                    }
                    else
                    {
                        count = 1;
                    }

                    /* process the input count times */
                    while (count--)
                    {
                        process_input(user, path, NULL);
                    }

                    retval = TRUE;
                    break;

                default:
                    /* is it a digit? */
                    if (g_unichar_isdigit(c))
                    {
                        digit[dig_cnt++] = (char)c;

                        if (dig_cnt > 3)
                        {
                            /* integer value too large (ridicilous value) */
                            dig_cnt = 0;
                            break;
                        }
                    }
                    break;
            }

            /* go to next char in buffer */
            buffer = g_utf8_next_char(buffer);
        }
    }

    return retval;
}
