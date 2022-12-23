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
#if !defined(WIN32)
  #include <unistd.h>
  #ifdef IAC_DEBUG
    #define TELCMDS
    #define TELOPTS
  #endif
  #include <arpa/telnet.h>    /* has to be after IAC_DEBUG */
#else
  #include <io.h>
  #include <windows.h>
  #include <winbase.h>
#endif
#include "mudix.h"

#if defined(WIN32)
  /* telnet options */
  #define TELOPT_BINARY         0       /* 8-bit data path */
  #define TELOPT_ECHO           1       /* echo */
  #define TELOPT_RCP            2       /* prepare to reconnect */
  #define TELOPT_SGA            3       /* suppress go ahead */
  #define TELOPT_NAMS           4       /* approximate message size */
  #define TELOPT_STATUS         5       /* give status */
  #define TELOPT_TM             6       /* timing mark */
  #define TELOPT_RCTE           7       /* remote controlled transmission and echo */
  #define TELOPT_NAOL           8       /* negotiate about output line width */
  #define TELOPT_NAOP           9       /* negotiate about output page size */
  #define TELOPT_NAOCRD         10      /* negotiate about CR disposition */
  #define TELOPT_NAOHTS         11      /* negotiate about horizontal tabstops */
  #define TELOPT_NAOHTD         12      /* negotiate about horizontal tab disposition */
  #define TELOPT_NAOFFD         13      /* negotiate about formfeed disposition */
  #define TELOPT_NAOVTS         14      /* negotiate about vertical tab stops */
  #define TELOPT_NAOVTD         15      /* negotiate about vertical tab disposition */
  #define TELOPT_NAOLFD         16      /* negotiate about output LF disposition */
  #define TELOPT_XASCII         17      /* extended ascii character set */
  #define TELOPT_LOGOUT         18      /* force logout */
  #define TELOPT_BM             19      /* byte macro */
  #define TELOPT_DET            20      /* data entry terminal */
  #define TELOPT_SUPDUP         21      /* supdup protocol */
  #define TELOPT_SUPDUPOUTPUT   22      /* supdup output */
  #define TELOPT_SNDLOC         23      /* send location */
  #define TELOPT_TTYPE          24      /* terminal type */
  #define TELOPT_EOR            25      /* end or record */
  #define TELOPT_TUID           26      /* TACACS user identification */
  #define TELOPT_OUTMRK         27      /* output marking */
  #define TELOPT_TTYLOC         28      /* terminal location number */
  #define TELOPT_3270REGIME     29      /* 3270 regime */
  #define TELOPT_X3PAD          30      /* X.3 PAD */
  #define TELOPT_NAWS           31      /* window size */
  #define TELOPT_TSPEED         32      /* terminal speed */
  #define TELOPT_LFLOW          33      /* remote flow control */
  #define TELOPT_LINEMODE       34      /* Linemode option */
  #define TELOPT_XDISPLOC       35      /* X Display Location */
  #define TELOPT_OLD_ENVIRON    36      /* Old - Environment variables */
  #define TELOPT_AUTHENTICATION 37      /* Authenticate */
  #define TELOPT_ENCRYPT        38      /* Encryption option */
  #define TELOPT_NEW_ENVIRON    39      /* New - Environment variables */
  #define TELOPT_EXOPL          255     /* extended-options-list */

  /* sub-option qualifiers */
  #define TELQUAL_IS            0       /* option is... */
  #define TELQUAL_SEND          1       /* send option */
  #define TELQUAL_INFO          2       /* ENVIRON: informational version of IS */
  #define TELQUAL_REPLY         2       /* AUTHENTICATION: client version of IS */
  #define TELQUAL_NAME          3       /* AUTHENTICATION: client version of IS */
#endif

/* mccp */
#define TELOPT_COMPRESS        (85)
#define TELOPT_COMPRESS2       (86)

#define PUTSHORT(cp, x) { if ((*cp++ = ((x)>>8)&0xFF) == IAC) *cp++ = IAC; \
                          if ((*cp++ = ((x))   &0xFF) == IAC) *cp++ = IAC; }


static void send_telnet(USER *user, unsigned char command, unsigned char option)
{
    unsigned char iac_buf[3];

#ifdef IAC_DEBUG
    printf("SEND: %6s - %02X\n", TELCMD(command), option);
#endif

    iac_buf[0] = IAC;
    iac_buf[1] = command;
    iac_buf[2] = option;

    /* lock the mutex */
    g_mutex_lock(user_network_mutex);

#if !defined(WIN32)
    write(user->net.sock, iac_buf, 3); /* bypassing normal send */
#else
    send(user->net.sock, iac_buf, 3, 0); /* bypassing normal send */
#endif

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);
}


static void send_data_telnet(USER *user, unsigned char *data, int len)
{
#ifdef IAC_DEBUG
    unsigned char *pData = data;
    int len2 = len;

    printf("SEND: %d - ", len);
    while (len2--) 
    {
        printf("%02X ", *pData);
        pData++;
    }
    printf("\n");
#endif

    /* lock the mutex */
    g_mutex_lock(user_network_mutex);

    write(user->net.sock, data, len); /* bypassing normal send */

    /* unlock the mutex */
    g_mutex_unlock(user_network_mutex);
}


void send_naws(USER *user)
{
    unsigned char  temp[128];
    unsigned char *cp = temp;
             gint  x, y;
           
    if (!(user->flags & FLG_NAWS_UPDATES))
    {
        /* did not receive DO NAWS yet */
        return;
    }

    /* grab the x and y */
    gui_user_get_xy(user, &x, &y);

    *cp++ = IAC;
    *cp++ = SB;
    *cp++ = TELOPT_NAWS;
    PUTSHORT(cp, x);
    PUTSHORT(cp, y);
    *cp++ = IAC;
    *cp++ = SE;
    send_data_telnet(user, temp, cp-temp);
}


int check_iac(USER *user, unsigned char *pStart, unsigned char *pEnd, int *length)
{
    unsigned char *pIac = pStart;
    int retval          = IAC_INCOMPLETE;
#ifdef IAC_DEBUG
    static int command, option;
#endif

    while (pIac != pEnd && retval == IAC_INCOMPLETE)
    {
        switch (user->net.rx_proc_state)
        {
            case RX_PROC_STATE_IAC:
                switch (*pIac)
                {
                    case IAC:
                        return IAC_ESCAPED;
                    case SB:                /* interpret as subnegotiation */
                        user->net.rx_proc_state = RX_PROC_STATE_SB;
                        break;
                    case WILL:              /* I will use option */
                        user->net.rx_proc_state = RX_PROC_STATE_WILL;
                        break;
                    case WONT:              /* I won't use option */
                        user->net.rx_proc_state = RX_PROC_STATE_WONT;
                        break;
                    case DO:                /* please, you use option */
                        user->net.rx_proc_state = RX_PROC_STATE_DO;
                        break;
                    case DONT:              /* you are not to use option */
                        user->net.rx_proc_state = RX_PROC_STATE_DONT;
                        break;
                    case GA:                /* you may reverse the line */
                    case EL:                /* erase the current line */
                    case EC:                /* erase the current character */
                    case AYT:               /* are you there */
                    case AO:                /* abort output--but let prog finish */
                    case IP:                /* interrupt process--permanently */
                    case BREAK:             /* break */
                    case DM:                /* data mark--for connect. cleaning */
                    case NOP:               /* nop */
                    case EOR:               /* end of record (transparent mode) */
                    case ABORT:             /* abort process */
                    case SUSP:              /* suspend process */
                    case xEOF:              /* end of file: EOF is already used... */
                    default:
                        retval  = IAC_OK;
                        break;
                }
#ifdef IAC_DEBUG
                command = *pIac;
#endif
                break;

            case RX_PROC_STATE_SB:
                if (*pIac == SE && *(user->net.iacp-1) == IAC)
                {
                    /* check the IAC option */
                    switch (user->net.iacbuf[0])
                    {
                        case TELOPT_COMPRESS2:
                            if (!MCCP_USER(user))
                            {
                                /* setup MCCP */
                                mccp_open(user);
                            }

                            retval = IAC_MCCP_START;
                            break;

                        case TELOPT_TTYPE:
                          {
                            unsigned char temp[50];

                            sprintf((char *)temp, "%c%c%c%c%s%c%c", 
                                IAC, SB, TELOPT_TTYPE, TELQUAL_IS, "vt100", IAC, SE);
                            send_data_telnet(user, temp, strlen((char *)temp+4)+4);

                            retval = IAC_OK;
                            break;
                          }

                        default:
                            retval = IAC_OK;
                            break;
                    }

#ifdef IAC_DEBUG
                    option = user->net.iacbuf[0];
#endif
                    /* reset the iac rx buffer */
                    user->net.iacp = user->net.iacbuf;
                }
                else
                {
                    /* store all data until IAC-SE is received */
                    *user->net.iacp++ = *pIac;
                }

                break;

            case RX_PROC_STATE_WILL:
                switch (*pIac)
                {
                  case TELOPT_COMPRESS:
                      send_telnet(user, DONT, TELOPT_COMPRESS);
                      break;

                  case TELOPT_COMPRESS2:
                      send_telnet(user, DO, TELOPT_COMPRESS2);
                      break;

                  case TELOPT_ECHO:
                      if (user->net.port != TELNET_PORT)
                      {
                          gui_input_visible(user, FALSE);
                      }
                      break;

                  default:
                      break;
                }

#ifdef IAC_DEBUG
                option = *pIac;
#endif
                retval = IAC_OK;
                break;

            case RX_PROC_STATE_WONT:
                switch (*pIac) 
                {
                    case TELOPT_ECHO:
                        gui_input_visible(user, TRUE);
                        break;
                    default:
                        break;
                }

#ifdef IAC_DEBUG
                option = *pIac;
#endif
                retval = IAC_OK;
                break;

            case RX_PROC_STATE_DO:
                switch (*pIac)
                {
                    case TELOPT_NAWS:
                        /* first send WILL */
                        send_telnet(user, WILL, TELOPT_NAWS);
                        user->flags |= FLG_NAWS_UPDATES;
                        /* then send a NAWS update */
                        send_naws(user);
                        break;

                    case TELOPT_TTYPE:
                        send_telnet(user, WILL, TELOPT_TTYPE);
                        break;

                    case TELOPT_ECHO:
                        gui_input_visible(user, TRUE);
                        send_telnet(user, WILL, TELOPT_ECHO);
                        break;

                    default:
                        send_telnet(user, WONT, *pIac);
                        break;
                }

#ifdef IAC_DEBUG
                option = *pIac;
#endif
                retval = IAC_OK;
                break;

            case RX_PROC_STATE_DONT:
                switch (*pIac)
                {
                    case TELOPT_ECHO:
                        if (user->net.port != TELNET_PORT)
                        {
                            gui_input_visible(user, FALSE);
                        }
                        send_telnet(user, WONT, TELOPT_ECHO);
                        break;

                    default:
                        send_telnet(user, WONT, *pIac);
                        break;
                }

#ifdef IAC_DEBUG
                option = *pIac;
#endif
                retval = IAC_OK;
                break;

            default:
                break;
        }

        pIac++;
    }

    /* return the length of the data processed */
    *length = pIac - pStart;

#ifdef IAC_DEBUG
    if (retval != IAC_INCOMPLETE)
    {
      printf("RECV: %s (option: %02X) in state %d (length=%d)\n\n", TELCMD(command), option, user->net.rx_proc_state, *length);
    }
#endif

    return retval;
}

