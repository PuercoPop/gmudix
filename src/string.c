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


bool is_numeric(gchar *str, gdouble *value)
{
    gchar    *begin = str;
    gunichar  c;

    /* parse till end of string */
    while ((c = g_utf8_get_char(str)) != '\0')
    {
        /* character in the string is not a digit! no numeric string */
        if (!g_unichar_isdigit(c))
        {
            return FALSE;
        }
        str = g_utf8_next_char(str);
    }
    
    *value = g_ascii_strtod(begin, NULL);

    return TRUE;
}


gchar *smash_tilde(gchar *buf)
{
    gchar     *retbuf = buf;
    gunichar   c;

    /* parse till end of string */
    while ((c = g_utf8_get_char(buf)) != '\0')
    {
        if (c == '~')
        {
            /* convert ~ to - */
            g_unichar_to_utf8('-', buf);
        }
        buf = g_utf8_next_char(buf);
    }

    return retbuf;
}


/* 
 * checks if destination already was malloced - if so, free it
 * note that the destination pointer is NOT updated, this is the
 * responsibility of the caller ;)
 */
gchar *str_dup(gchar *dest, gchar *src)
{
    if (dest)
    {
        free(dest);
    }

    return strdup(src);
}


gchar *get_arg(USER *user, gchar *src, gchar *dst)
{
    gunichar c, cmd_stack, block_open, block_close;
    int      nested = 0;
    
    /* grab the customizable characters for this user */
    cmd_stack   = user->custom_chars[CUST_CMD_STACK];
    block_open  = user->custom_chars[CUST_BLOCK_OPEN];
    block_close = user->custom_chars[CUST_BLOCK_CLOSE];

    /* strip leading spaces */
    while (g_utf8_get_char(src) == ' ')
    {
	src = g_utf8_next_char(src);
    }

    /* parse through whole string */
    while ((c = g_utf8_get_char(src)) != '\0') 
    {
        if (c == block_open) 
        {
            /* start of a block found */
            nested++;
	    if (nested == 1) 
            {
                /* first block, then skip the character and continue */
	        src = g_utf8_next_char(src);
		continue;
	    }
	}
	else if (c == block_close)
        {
            /* end of a block found */
            nested--;
	    if (nested == 0) 
            {
                /* end of the first block, skip } and return */
	        src = g_utf8_next_char(src);
		break;
	    }
	}
	else if (!nested && (c == ' ' || c == '\n' || c == cmd_stack))
        {
            /* newline or space or stack (not within a block) encountered */
            src = g_utf8_next_char(src);
            break;
        }
        
        /* just copy and step to next character */
        dst += g_unichar_to_utf8(c, dst);
        src  = g_utf8_next_char(src);
    }

    *dst++ = '\0';

    return src;
}

