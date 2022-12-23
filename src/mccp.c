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
#include <zlib.h>
#include "mudix.h"


/* zlib helper functions */
static void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}


static void zlib_free(void *opaque, void *address)
{
    free(address);
}


static void expand_mccp_rx(USER *user)
{
    unsigned char *temp;

    /* double the buffer capacity */
    user->net.mccp_outsize *= 2;
    temp = malloc(user->net.mccp_outsize);

    /* copy the existing data into the newly allocated buffer */
    memcpy(temp, user->net.mccp_out, 
                 user->net.mccp_outp - user->net.mccp_out);
    user->net.mccp_outp = temp + (user->net.mccp_outp - user->net.mccp_out);

    /* free the old buffer and assign the new one */
    free(user->net.mccp_out);
    user->net.mccp_out = temp;
}


void decompress_rxbuf(USER *user)
{
    int status;

    /* prepare the receive and output pointers */
    user->net.stream->next_in   = user->net.rxbuf;
    user->net.stream->next_out  = user->net.mccp_outp;
    user->net.stream->avail_in  = user->net.rxp - user->net.rxbuf;
    user->net.stream->avail_out = user->net.mccp_outsize - 
                                 (user->net.mccp_outp - user->net.mccp_out);
    
    /* decompress the data */
    status = inflate(user->net.stream, Z_SYNC_FLUSH);

    switch (status)
    {
        case Z_OK:
        case Z_STREAM_END:
            /* remove used data from inbuf */
            memmove(user->net.rxbuf, 
                    user->net.stream->next_in, 
                    user->net.stream->avail_in);
            user->net.rxp = user->net.rxbuf + user->net.stream->avail_in;

            /* update outbuf pointers */
            user->net.mccp_outp = user->net.stream->next_out;

            /* end of stream encountered */
            if (status == Z_STREAM_END) 
            {
                /* close the stream */
                mccp_close(user);
                break;
            }

            /* is there still MCCP data because the output buffer is full? */
            if (user->net.stream->avail_in && 
                user->net.stream->avail_out == 0)
            {
                /* somehow this does not result in a Z_BUF_ERROR!?
                   perform a FALL-THROUGH, the Z_BUF_ERROR case will also be
                   executed, so that the receive buffer will be expanded and
                   the MCCP data processed */
            }
            else
            {
                /* do NOT fall-through, but break out of switch */
                break;
            }
        case Z_BUF_ERROR:
            /* buffers not sufficient? */
            expand_mccp_rx(user);

            /* try again - possible stack overflow? */
            decompress_rxbuf(user);
            break;
        case Z_STREAM_ERROR:
            /* disconnect if a stream error was encountered */
            do_disconnect(user);
            break;
        default:
            fprintf(stdout, "MCCP compression error (%s).\n", user->net.stream->msg);
            break;
    }
}


void mccp_open(USER *user)
{
    if (MCCP_USER(user))
    {
        /* already opened MCCP for this user! */
        return;
    }

    /* allocate the stream and the MCCP buffer */
    if (!(user->net.stream = malloc(sizeof(z_stream))))
    {
        fprintf(stdout, "No memory for zlib stream?!\n");
	exit_mudix();
    }

    if (!user->net.mccp_out)
    {
        if (!(user->net.mccp_out = malloc(MCCP_RX_LENGTH))) 
        {
            fprintf(stdout, "No memory for MCCP buffer?!\n");
            exit_mudix();
        }

        /* only re-set outsize if buffer was re-allocated */
        user->net.mccp_outsize = MCCP_RX_LENGTH;
    }

    /* set the pointers */
    user->net.mccp_outp      = user->net.mccp_out;
    user->net.stream->zalloc = zlib_alloc;
    user->net.stream->zfree  = zlib_free;
    user->net.stream->opaque = NULL;

    /* initialise the stream */
    if (inflateInit(user->net.stream) != Z_OK) 
    {
        free(user->net.stream);
        user->net.stream = NULL;
    }
}


void mccp_close(USER *user)
{
    if (!MCCP_USER(user))
    {
        return;
    }

    /* close the stream */
    inflateEnd(user->net.stream);

    /* free the stream buffer */
    free(user->net.stream);

    /* just set the pointer to NULL now */
    user->net.stream = NULL;
}
