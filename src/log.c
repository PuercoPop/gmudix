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

#if !defined(WIN32)
  #include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mudix.h"


bool open_log(USER *user, char *filename, bool date)
{
#if !defined(WIN32)
    struct timeval current_time;
#else
           time_t  current_time;
#endif
    char           path[MAX_FILEPATH];
    char           timestr[MAX_STRING];

    /* if a logfile is already open, return immediately */
    if (user->logfile)
    {
        return FALSE;
    }

#if !defined(WIN32)
    /* get the current time */
    gettimeofday(&current_time, NULL);
    strcpy(timestr, ctime(&current_time.tv_sec));
#else
    time(&current_time);
    strcpy(timestr, ctime(&current_time));
#endif
    /* hack off year */
    timestr[strlen(timestr)-5] = '\0';

    /* format the filename */
    sprintf(path, "%s/" LOG_PATH "%s%s.log", getenv("HOME"), 
                                             date? timestr: "", filename);

    /* open the file - in append mode */
    if (!(user->logfile = fopen(path, "a")))
    {
        return FALSE;
    }

    /* unbuffered writes to the file! :) */
    setbuf(user->logfile, NULL);

    /* first print a log started message in the file */
    fprintf(user->logfile, "%%%% STARTING NEW LOG, %s\n\n", timestr);

    return TRUE;
} 


bool close_log(USER *user)
{
    /* no logfile open? */
    if (!user->logfile)
    {
        return FALSE;
    }

    /* write an end of log mesage in the file */
    fprintf(user->logfile, "\n%%%% END OF LOG.\n");

    /* close the file */
    fclose(user->logfile);

    user->logfile = NULL;

    return TRUE;
}
        

void write_log(USER *user, gchar *data, gsize length)
{
    /* safety check - is there a logfile open? */
    if (!user->logfile)
    {
        return;
    }

    /* just dump the data in the file */
    fwrite(data, sizeof(gchar), length, user->logfile);
    return;
}
