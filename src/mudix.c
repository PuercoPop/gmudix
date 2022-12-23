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
#include <signal.h>
#if !defined(WIN32)
  #include <sys/wait.h>
#else
  #include <io.h>
  #include <windows.h>
  #include <winbase.h>
#endif
#include "mudix.h"


void signal_handler(int signal)
{
#if !defined(WIN32)
    /* just call wait with no hang so that we get no zombies */
    waitpid(-1, NULL, WNOHANG);
#endif
}


#if !defined(WIN32)
/*
 * Main loop
 */
int main(int argc, char *argv[])
{
    /* install a signal for SIGCHLD for when a child dies */
    signal(SIGCHLD, signal_handler);

    init_gui(argc, argv);

    return 0;
}

#else

int WINAPI WinMain(HINSTANCE hInstance,  // handle to current instance
                   HINSTANCE hPrevInstance,  // handle to previous instance
                   LPSTR lpCmdLine,      // pointer to command line
                   int nCmdShow)         // show state of window
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
 
    wVersionRequested = MAKEWORD(2, 2);
 
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) 
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return 0;
    }
 
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
 
    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) 
    {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        return 0; 
    }
        
    init_gui(0, NULL);

    return 0;
}

#endif