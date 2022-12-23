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
#include "file.h"


gchar *fread_string(FILE *fp) 
{
    static gchar  string[MAX_STRING];
           gchar *pStr = string;

    /* strip leading spaces and inprintable chars */
    do 
    {
        *pStr = fgetc(fp);
    } while (!feof(fp) && (*pStr == ' ' || !isprint(*pStr)));

    pStr++;

    /* read until we encounter a ~ */
    while ((*pStr++ = fgetc(fp)) != '~') 
    {
        /* do nothing */
    }
    *(pStr-1) = '\0';

    /* also read to end of the line */
    while (!feof(fp) && fgetc(fp) != '\n') 
    {
        /* do nothing */
    }

    /* alloc space for the string and return it */
    if (!(pStr = strdup(string)))
    {
	fprintf(stdout, "No memory for fread_string, exiting!\n");
	exit_mudix();
    }

    return pStr;
}


gchar *fread_to_eol(FILE *fp) 
{
    static gchar  string[MAX_STRING];
           gchar *pStr = string;

    while (!feof(fp) && (*pStr = fgetc(fp)) != '\n') 
    {
	if (!isprint(*pStr))
        {
	    continue;
        }
	pStr++;
    }
    *pStr = '\0';

    return &string[0];
}


gchar *fread_word(FILE *fp) 
{
    static gchar  word[MAX_STRING];
           gchar *pWord = word;

    /* strip leading spaces and inprintable chars */
    do 
    {
        *pWord = fgetc(fp);
    } while (!feof(fp) && (*pWord == ' ' || !isprint(*pWord)));

    pWord++;

    /* get the word from the file */
    while (isprint((*pWord = fgetc(fp)))) 
    {
	if (*pWord == ' ')
        {
	    break;
        }
	pWord++;
    }
    *pWord = '\0';

    return &word[0];
}
