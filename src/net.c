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
#include <errno.h>
#if !defined(WIN32)
  #include <netdb.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/telnet.h>
  #include <unistd.h>
#else
  #include <io.h>
  #include <windows.h>
  #include <winbase.h>
#endif
#include <fcntl.h>
#include "mudix.h"
#include "gui.h"


/* mutex for accessing the network data in the user structure */
GMutex *user_network_mutex = NULL;


/* handler for received data */
static void rx_handler(gpointer user, gint source, GdkInputCondition condition)
{
    if (read_data((USER *)user) > 0)
    {
        process_rx_buffer((USER *)user);
    }
}


/* handler for received exceptions (from socket) */
static void exc_handler(gpointer user, gint source, GdkInputCondition condition)
{
    gui_user_disconnect((USER *)user);
}


int read_data(USER *user)
{
    int nRead;

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    /* read the data from the socket */
#if !defined(WIN32)
    nRead = read(user->net.sock, user->net.rxp,
                 RXBUF_LENGTH - (user->net.rxp - user->net.rxbuf));
#else
    nRead = recv(user->net.sock, user->net.rxp,
                 RXBUF_LENGTH - (user->net.rxp - user->net.rxbuf), 0);
#endif

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    if (nRead > 0)
    {
        user->net.rxp += nRead;
    }
    /* if nRead is 0, then disconnect */
    else if (nRead == 0)
    {
        gui_user_disconnect(user);
    }
#if !defined(WIN32)
    /* also disconnect if nRead < 0 and errno is not EWOULDBLOCK and EAGAIN */
    else if (errno != EWOULDBLOCK && errno != EAGAIN)
#else
    else if (nRead == SOCKET_ERROR)
#endif
    {
        gui_user_disconnect(user);
    }

    return nRead;
}


int write_data(USER *user, gchar *buffer, int len)
{
    gchar *str;
    gsize  length;
    int    nrWrite = 0;

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    if (user->net.sock)
    {
        GError  *conv_err = NULL;

        /* convert the UTF8 buffer to the server's charset before sending it */
        str = g_convert(buffer, len, user->net.charset,
                        CHARSET_CONV, NULL, &length, &conv_err);

         /* check if conversion resulted in an error */
        if (!str)
        {
            /* print errors */
            fprintf(stderr, "write_data: conv_err (%d): %s\n", conv_err->code,
                                                               conv_err->message);

            /* free the error structure */
            g_error_free(conv_err);
        }
        else
        {
            gchar *pMrk = str;
            gchar *pStr = str;

            /* check for IACs in the outgoing buffer and escape them */
            while (length--)
            {
                if ((unsigned char)*pStr == IAC)
                {
                    /* send data upto the IAC */
#if !defined(WIN32)
                    if ((nrWrite = write(user->net.sock, pMrk, pStr-pMrk+1)) < 0)
#else
                    if ((nrWrite = send(user->net.sock, pMrk, pStr-pMrk+1, 0)) < 0)
#endif
                    {
                        break;
                    }

                    /* send an extra IAC */
#if !defined(WIN32)
                    if ((nrWrite = write(user->net.sock, pStr, 1)) < 0)
#else
                    if ((nrWrite = send(user->net.sock, pStr, 1, 0)) < 0)
#endif
                    {
                        break;
                    }

                    pMrk = pStr+1;
                }

                pStr++;
            }

            if (pMrk != pStr)
            {
#if !defined(WIN32)
                 nrWrite = write(user->net.sock, pMrk, pStr-pMrk);
#else
                 nrWrite = send(user->net.sock, pMrk, pStr-pMrk, 0);
#endif
            }

            /* finally free the temporary conversion buffer */
            g_free(str);
        }
    }

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    return nrWrite;
}


static NET_CODE do_connect(char *site, int port, int *sock, int *addr, char **hostnm)
{
    struct hostent     *host;
    struct sockaddr_in  server;

    /* get host by name - this function will block as it queries the DNS! */
    if (!(host = gethostbyname(site)))
    {
        return NET_GETHOSTBYNAME;
    }

    /* create a socket */
    if ((*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        return NET_NO_SOCKET;
    }

    memcpy(&server.sin_addr, host->h_addr, host->h_length);
    server.sin_family = host->h_addrtype;
    server.sin_port   = htons((unsigned short)port);

    *addr   = ntohl(server.sin_addr.s_addr);
    *hostnm = strdup(host->h_name);

    if (connect(*sock, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        close(*sock);
        return NET_CONNECT_FAILURE;
    }

#if !defined(WIN32)
    if (fcntl(*sock, F_SETFL, O_NDELAY) == -1)
    {
        perror("fcntl: O_NDELAY (fatal?)");
    }
#else
    {
      unsigned long value = 1;

      ioctlsocket(*sock, FIONBIO, &value);
    }
#endif

    return NET_CONNECTED;
}


void do_disconnect(USER *user)
{
    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    /* remove the reconnect timer if it is active */
    if (user->net.recon_id)
    {
        gtk_timeout_remove(user->net.recon_id);
    }

    /* remove the receive and exception handler */
    if (user->net.rx_tag)
    {
        gtk_input_remove(user->net.rx_tag);
    }

    if (user->net.exc_tag)
    {
        gtk_input_remove(user->net.exc_tag);
    }

    /* close the socket */
    if (user->net.sock)
    {
        close(user->net.sock);
    }

    /* set network status */
    user->net.status = NET_CLOSED;

    /* set data on 0 */
    user->net.recon_id = 0;
    user->net.rx_tag   = 0;
    user->net.exc_tag  = 0;
    user->net.sock     = 0;

    /* close the MCCP stream */
    mccp_close(user);

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);
}


/*
 * Connection thread
 * Note: all network data that is used below in the thread should be
 * protected by the network mutex in other parts of the code where it
 * needs to be read or written!
 * Read/Write: sock, status, host_name, host_addr, rx_tag, exc_tag
 * Write-Only: site (actually port too, but is only an integer)
 */
void *connect_thread(USER *user)
{
    static int       st_id;
           int       id, sock, addr, port;
           char     *site, *host;
           NET_CODE  ret;

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    /* copy some data from the network structure */
    port = user->net.port;
    site = strdup(user->net.site);

    /* and put a thread id in the network structure */
    user->net.thread_id = id = st_id++;

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    /* make the connection to the server */
    ret = do_connect(site, port, &sock, &addr, &host);

    /* free temporary site string */
    free(site);

    /* first lock the mutex */
    g_mutex_lock(user_network_mutex);

    /* check here if user still exists or if ID was changed */
    if (!check_valid_user(user) || user->net.thread_id != (guint)id)
    {
        /* if do_connect malloced host, free it */
        if (host)
        {
            free(host);
        }

        /* just close the socket if it is open */
        if (ret == NET_CONNECTED)
        {
            close(sock);
        }
    }
    else
    {
        NET_CODE status = user->net.status;

        /* only print a message when the user is not already connected */
        if (status != NET_CONNECTED)
        {
            /* update the new status */
            user->net.status = ret;

            gdk_threads_enter();
            /* create a 1ms timer to update the connection status */
            gtk_timeout_add(CONNECT_TIMER, (GtkFunction)gui_connection_status, user);
            gdk_threads_leave();
        }

        /* same ID and the user still exists */
        if (ret == NET_CONNECTED)
        {
            /* socket connected to the server */
            if (status == NET_CONNECTED)
            {
                /* another thread connected already - disconnect it */
                gdk_threads_enter();
                gtk_input_remove(user->net.rx_tag);
                gtk_input_remove(user->net.exc_tag);
                gdk_threads_leave();
                close(user->net.sock);
            }

            /* we are connected, so set data in user structure :) */
            user->net.sock   = sock;
            user->net.status = ret;

            /* free the host_name if it was previously set */
            if (user->net.host_name)
            {
                free(user->net.host_name);
            }
            user->net.host_name = host;

            /* setup the IP address */
            sprintf(user->net.host_addr, "%d.%d.%d.%d",
                (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
                (addr >>  8) & 0xFF, (addr      ) & 0xFF);

            /* thread function for entering GTK routines */
            gdk_threads_enter();

            /* set up the receive and exception handler */
            user->net.rx_tag  = gtk_input_add_full(user->net.sock,
                                GDK_INPUT_READ, rx_handler, NULL, user, NULL);
            user->net.exc_tag = gtk_input_add_full(user->net.sock,
                                GDK_INPUT_EXCEPTION, exc_handler, NULL, user, NULL);

            /* thread function for leaving GTK routines */
            gdk_threads_leave();
        }
    }

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);

    return NULL;
}
