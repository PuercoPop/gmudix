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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(WIN32)
  #include <arpa/telnet.h>
#else
  #include <windows.h>
#endif
#include "mudix.h"


typedef enum
{
    OP_NONE,
    OP_EQUAL,           /* == */
    OP_INEQUAL,         /* != */
    OP_GREATER,         /* >  */
    OP_GREATER_EQUAL,   /* >= */
    OP_LESSER,          /* <  */
    OP_LESSER_EQUAL     /* <= */
} OPERATOR_TYPE;


static gchar *get_operator(gchar *expr, OPERATOR_TYPE *operand)
{
    gunichar  c1, c2;

    /* get the two characters and advance expr by 2 characters */
    c1   = g_utf8_get_char(expr);
    expr = g_utf8_next_char(expr);
    c2   = g_utf8_get_char(expr);

    /* by default operand is OP_NONE */
    *operand = OP_NONE;
    switch (c1)
    {
        case '!':   /* != */
            if (c2 == '=')
            {
                *operand = OP_INEQUAL;
            }
            break;
        case '=':   /* == */
            if (c2 == '=')
            {
                *operand = OP_EQUAL;
            }
            break;
        case '>':   /* >=, > */
            if (c2 == '=')
            {
                *operand = OP_GREATER_EQUAL;
            }
            else
            {
                *operand = OP_GREATER;
            }
            break;
        case '<':   /* <=, < */
            if (c2 == '=')
            {
                *operand = OP_LESSER_EQUAL;
            }
            else
            {
                *operand = OP_LESSER;
            }
            break;
        default:
            break;
    }

    if (*operand != OP_LESSER && *operand != OP_GREATER)
    {
        /* advance one more! */
        expr = g_utf8_next_char(expr);
    }

    return expr;
}


static bool check_expression(USER *user, gchar *expr)
{
    gchar            arg1[MAX_INPUT_LEN];
    gchar            arg2[MAX_INPUT_LEN];
    VAR             *var;
    gchar           *pExpr = expr;
    gchar           *pArg1, *pArg2;
    gdouble          lval, rval;
    OPERATOR_TYPE    opr;
    bool             retval, fNumeric = FALSE;
    gunichar         c;

    /* get the first argument */
    pArg1 = arg1;
    while ((c = g_utf8_get_char(pExpr)) != '\0')
    {
        if (c == '!' ||
            c == '=' ||
            c == '>' ||
            c == '<')
        {
            break;
        }

        pArg1 += g_unichar_to_utf8(c, pArg1);
        pExpr  = g_utf8_next_char(pExpr);
    }
    *pArg1 = '\0';

    /* retrieve the operator type */
    pExpr = get_operator(pExpr, &opr);

    /* check if it was a valid operand */
    if (opr == OP_NONE)
    {
        return FALSE;
    }

    /* copy the 2nd argument into arg2 */
    strcpy(arg2, pExpr);

    /* look up if this references a variable */
    var = var_lookup(user, arg1);
    if (var)
    {
        /* set the argument pointer to point to the variable value :) */
        pArg1 = var->value;
    }
    else
    {
        /* set to the original argument */
        pArg1 = arg1;
    }

    /* look up if this references a variable */
    var = var_lookup(user, arg2);
    if (var)
    {
        /* set the argument pointer to point to the variable value :) */
        pArg2 = var->value;
    }
    else
    {
        /* set to the original argument */
        pArg2 = arg2;
    }

    /* check if arguments are numeric */
    if (is_numeric(pArg1, &lval) && is_numeric(pArg2, &rval))
    {
        fNumeric = TRUE;
    }

    /* now check the operand type and the values */
    switch (opr)
    {
        case OP_GREATER:
            retval = fNumeric? (lval > rval):
                               (strcmp(pArg1, pArg2) > 0);
            break;
        case OP_GREATER_EQUAL:
            retval = fNumeric? (lval >= rval):
                               (strcmp(pArg1, pArg2) >= 0);
            break;
        case OP_LESSER:
            retval = fNumeric? (lval < rval):
                               (strcmp(pArg1, pArg2) < 0);
            break;
        case OP_LESSER_EQUAL:
            retval = fNumeric? (lval <= rval):
                               (strcmp(pArg1, pArg2) <= 0);
            break;
        case OP_INEQUAL:
            retval = fNumeric? (lval != rval):
                               (strcmp(pArg1, pArg2) != 0);
            break;
        case OP_EQUAL:
            retval = fNumeric? (lval == rval):
                               (strcmp(pArg1, pArg2) == 0);
            break;
        default:
            retval = FALSE;
            break;
    }

    return retval;
}


bool process_if(USER *user, gchar *buffer)
{
    gchar str[MAX_STRING];

    /* grab the expression */
    buffer = get_arg(user, buffer, str);

    /* and check it */
    if (check_expression(user, str))
    {
        /* grab first argument after expression and process it */
        get_arg(user, buffer, str);
        process_input(user, str, NULL);
        return TRUE;
    }
    else
    {
        /* skip to 2nd argument */
        buffer = get_arg(user, buffer, str);
        get_arg(user, buffer, str);

        if (str[0])
        {
            /* process the 'else' command */
            process_input(user, str, NULL);
        }
        return FALSE;
    }
}


bool process_for(USER *user, gchar *buffer)
{
    VAR    *var;
    gchar   arg1[MAX_INPUT_LEN];
    gchar   arg2[MAX_INPUT_LEN];
    gchar  *pArg1;
    gchar  *pArg2;
    gdouble lval;
    gdouble rval;

    /* store the for loop in arg1 first */
    buffer = get_arg(user, buffer, arg1);

    /* lookup the : in the loop and put the lval in arg1 */
    pArg1 = arg1;
    pArg2 = g_strrstr(arg1, ":");
    if (!pArg2)
    {
        return FALSE;
    }
    *pArg2++ = '\0';

    /* put the rval in arg2 */
    strcpy(arg2, pArg2);
    pArg2 = arg2;

    /* look up if this references a variable */
    var = var_lookup(user, arg1);
    if (var)
    {
        /* set the argument pointer to point to the variable value :) */
        pArg1 = var->value;
    }

    /* look up if this references a variable */
    var = var_lookup(user, arg2);
    if (var)
    {
        /* set the argument pointer to point to the variable value :) */
        pArg2 = var->value;
    }

    lval = g_ascii_strtod(pArg1, NULL);
    rval = g_ascii_strtod(pArg2, NULL);

    /* store the command in arg2 */
    get_arg(user, buffer, arg1);

    for (; lval < rval; lval++)
    {
        /* copy the value into arg2 */
        sprintf(arg2, "%.0f", lval);

        /* call the input processor, supply arg2 as argument */
        process_input(user, arg1, arg2);

        if ((rval-lval) > MAX_FOR_LOOPS)
        {
            break;
        }
    }

    return TRUE;
}


gchar *parse_input(USER *user, gchar *src, gchar *args, gchar *dest, int *len)
{
    static gchar        args_table[MAX_ARGS][MAX_INPUT_LEN];
           gchar       *pDest = dest;
           int          nested = 0, arg_count = 0;
           gunichar     c, var_sign, cmd_stack, block_open, block_close;

    /* grab the customizable characters for this user */
    cmd_stack   = user->custom_chars[CUST_CMD_STACK];
    var_sign    = user->custom_chars[CUST_VAR_SIGN];
    block_open  = user->custom_chars[CUST_BLOCK_OPEN];
    block_close = user->custom_chars[CUST_BLOCK_CLOSE];

    if (args)
    {
        int i;

        /* just take arguments from the args pointer */
        for (i=0; i<MAX_ARGS; i++)
        {
            args = get_arg(user, args, args_table[i]);

            if (args_table[i][0])
            {
                arg_count++;
            }
            else
            {
                break;
            }
        }
    }

    while ((c = g_utf8_get_char(src)) != '\0')
    {
        if (c == block_open)            /* open block character? */
        {
            nested++;
        }
        else if (c == block_close)      /* close block character? */
        {
            nested--;
        }
        else if (c == cmd_stack)        /* cmd stack character? */
        {
            /* if we are inside a block, then ignore command stack */
            if (!nested)
            {
                src = g_utf8_next_char(src);
                break;
            }
        }
        else if (c == var_sign)         /* variable sign? */
        {
            gchar      *tmp = g_utf8_next_char(src);
            gunichar    var;

            /* grab the character after the sign */
            var = g_utf8_get_char(tmp);

            if (var == var_sign)
            {
                /* encountered two var signs, skip over the 2nd */
                src = tmp;
            }
            /* check if there's a 0-9 present */
            else if (g_unichar_isdigit(var))
            {
                /* enough arguments in table for requested arg? */
                if (arg_count > (int)(var - '0'))
                {
                    gchar *pTmp;

                    /* copy the argument from the arg table */
                    pTmp = args_table[(var - '0')];
                    while (*pTmp != '\0')
                    {
                        *pDest++ = *pTmp++;

                        /* safety check */
                        if ((pDest - dest) >= MAX_STRING)
                        {
                            break;
                        }
                    }
                }

                /* skip over the variable */
                src = g_utf8_next_char(tmp);
                continue;
            }
            else
            {
                /* check for a variable */
                VAR    *var;
                gchar   variable[MAX_INPUT_LEN];

                /* grab the variable */
                tmp = get_arg(user, tmp, variable);

                var = var_lookup(user, variable);
                if (var)
                {
                    gchar *pTmp;

                    /* copy the value of the variable */
                    pTmp = var->value;
                    while (*pTmp != '\0')
                    {
                        *pDest++ = *pTmp++;

                        /* safety check */
                        if ((pDest - dest) >= MAX_STRING)
                        {
                            break;
                        }
                    }

                    /* skip over the variable */
                    src = tmp;
                    continue;
                }
            }
        }

        /* copy the character and skip to next */
        pDest += g_unichar_to_utf8(c, pDest);
        src    = g_utf8_next_char(src);

        /* safety check */
        if ((pDest - dest) >= MAX_STRING)
        {
            break;
        }
    }

    /* add a newline, then return */
    *pDest++ = '\n';
    *pDest   = '\0';
    *len     = pDest - dest;

    return src;
}


void process_input(USER *user, gchar *input, gchar *args)
{
    gchar  cmd[MAX_STRING]; /* not static for nested input */
    int    len;

    do
    {
        gunichar first_char;

        /* parse the input */
        input = parse_input(user, input, args, cmd, &len);

        /* grab the first character */
        first_char = g_utf8_get_char(cmd);

        if (first_char == user->custom_chars[CUST_CMD_CHAR])
        {
            /* check if a MUDix-command is to be processed */
            if (do_command(user, g_utf8_next_char(cmd)))
            {
                continue;
            }
        }
        else if (first_char == user->custom_chars[CUST_SPEED_PATH])
        {
            /* check if a speed path is to be processed */
            if (process_path(user, g_utf8_next_char(cmd)))
            {
                continue;
            }
        }

        /* check if an alias needs to be processed */
        if (process_alias(user, cmd))
        {
            continue;
        }

        /* write the data to the socket */
        write_data(user, cmd, len);

        /* check if input is invisible */
        if (gui_input_is_vis(user) && !(user->flags & FLG_INPUT_ECHO_OFF))
        {
            gui_add_color_window(user, cmd, len, COLOR_ECHO);
        }
    }
    while (*input != '\0');

    return;
}


static void process_esc(gunichar c, ANSI_T *ansi)
{
    char     *pParam;
    int       params[CSIPARAMS], npar = 0, i;

    *ansi->paramp = '\0';
    pParam        = ansi->parambuf;
    while (pParam < ansi->paramp)
    {
        params[npar++] = atoi(pParam);
        pParam        += (strlen(pParam) + 1);
    }

    switch (c)
    {
        /* we only support 'm' for now */
        case 'm':
            /* if no parameters are found, just clear */
            if (npar == 0)
            {
                ansi->attrib = 0;
                ansi->fg     = ANSI_COLOR_NONE;
                ansi->bg     = ANSI_COLOR_NONE;
            }

            for (i=0; i<npar; i++)
            {
                 switch (params[i])
                 {
                     case ANSI_DEFAULT:
                         ansi->attrib = 0;
                         ansi->fg     = ANSI_COLOR_NONE;
                         ansi->bg     = ANSI_COLOR_NONE;
                         break;
                     case ANSI_BOLD:
                         ansi->attrib |= ATTR_BOLD;
                         break;
                     case ANSI_UNDERLINE:
                         ansi->attrib |= ATTR_UNDERLINE;
                         break;
                     case ANSI_ITALIC:
                         ansi->attrib |= ATTR_ITALIC;
                         break;
                     case ANSI_BOLD_OFF:
                         ansi->attrib &= ~ATTR_BOLD;
                         break;
                     case ANSI_UNDERLINE_OFF:
                         ansi->attrib &= ~ATTR_UNDERLINE;
                         break;
                     case ANSI_ITALIC_OFF:
                         ansi->attrib &= ~ATTR_ITALIC;
                         break;
                     case ANSI_FG_BLACK:
                     case ANSI_FG_RED:
                     case ANSI_FG_GREEN:
                     case ANSI_FG_BROWN:
                     case ANSI_FG_BLUE:
                     case ANSI_FG_MAGENTA:
                     case ANSI_FG_CYAN:
                     case ANSI_FG_WHITE:
                         /* set fg color */
                         ansi->fg = (params[i] - ANSI_FG_BLACK);
                         break;
                     case ANSI_BG_BLACK:
                     case ANSI_BG_RED:
                     case ANSI_BG_GREEN:
                     case ANSI_BG_BROWN:
                     case ANSI_BG_BLUE:
                     case ANSI_BG_MAGENTA:
                     case ANSI_BG_CYAN:
                     case ANSI_BG_WHITE:
                         /* set bg color */
                         ansi->bg = (params[i] - ANSI_BG_BLACK);
                         break;
                     case ANSI_FG_DEFAULT:
                         ansi->fg = 0;
                         break;
                     case ANSI_BG_DEFAULT:
                         ansi->bg = 0;
                         break;
                     default:
                         break;
                 }
            }
            break;

        default:
            break;
    }

    /* reset the ansi state and rx pointer */
    ansi->state  = ANSI_STATE_IDLE;
    ansi->paramp = ansi->parambuf;
}


static guint check_esc(gunichar c, gchar *pRx, gchar *pEnd, ANSI_T *ansi)
{
    gchar *pStart = pRx;

    while (pRx < pEnd)
    {
        switch (ansi->state)
        {
            case ANSI_STATE_IDLE:
                if (c == CSI)
                {
                    ansi->state = ANSI_STATE_PARAMS;
                }
                else if (c == ESC)
                {
                    ansi->state = ANSI_STATE_ESC;
                }
                else
                {
                    /* do nothing */
                }
                break;

            case ANSI_STATE_ESC:
                if (c == '[')
                {
                    ansi->state = ANSI_STATE_PARAMS;
                }
                else
                {
                    /* process the complete esc seq */
                    process_esc(c, ansi);
                }
                break;

            case ANSI_STATE_PARAMS:
                if (c == ';')
                {
                    *ansi->paramp++ = '\0';
                    // overflow check here?
                }
                else if (g_unichar_isdigit(c))
                {
                    *ansi->paramp++ = (char)c;
                    // overflow check here?
                }
                else
                {
                    /* process the complete esc seq */
                    process_esc(c, ansi);
                }
                break;

            default:
                /* unknown state? */
                break;
        }

        /* advance pointer */
        pRx = g_utf8_next_char(pRx);

        if (ansi->state == ANSI_STATE_IDLE)
        {
            break;
        }
        else
        {
            /* get next character */
            c = g_utf8_get_char(pRx);
        }
    }

    return pRx-pStart;
}


static void add_to_trigger_buf(USER *user, gchar *buf, int len)
{
    if ((user->trigger - user->trigger_buf) + len > MAX_STRING)
    {
        /* trigger buffer overflow - we can expand buffer on demand,
         * but MAX_STRING is plenty for a trigger buf, if the trigger
         * buffer flows over, then a newline is never received...
         * for now we just reset the trigger buffer */
        user->trigger = user->trigger_buf;
    }
    else
    {
        /* add to trigger buffer */
        memcpy(user->trigger, buf, len);
        user->trigger += len;
    }
}


static void process_rx_data(USER *user, char *data, int len)
{
    GError   *conv_err = NULL;
    gchar    *rxbuf, *pRx, *pMrk, *pEnd;
    gsize     rxcnt;
    gunichar  c;

    /* convert the incoming data to an UTF8 string */
    rxbuf = g_convert((const gchar *)data, (gssize)len, CHARSET_CONV,
                      user->net.charset, NULL, &rxcnt, &conv_err);

    /* check if conversion resulted in an error */
    if (!rxbuf)
    {
        /* print errors */
        fprintf(stderr, "process_rx_buffer: conv_err (%d): %s\n",
                conv_err->code, conv_err->message);

        /* free the error structure */
        g_error_free(conv_err);

        return;
    }

    pRx  = pMrk = rxbuf;
    pEnd        = rxbuf + rxcnt;

    /* check all characters in the buffer */
    while (pRx != pEnd)
    {
        c = g_utf8_get_char(pRx);

        if (user->ansi.state != ANSI_STATE_IDLE)
        {
            pRx += check_esc(c, pRx, pEnd, &user->ansi);
            pMrk = pRx;
        }
        else
        {
            switch (c)
            {
                case CSI:
                case ESC:
                    /* found an escape sequence, flush buffer and check ESC */
                    if (pRx != pMrk)
                    {
                        gui_add_ansi_window(user, pMrk, pRx-pMrk);
                        add_to_trigger_buf(user, pMrk, pRx-pMrk);
                    }

                    pRx += check_esc(c, pRx, pEnd, &user->ansi);
                    pMrk = pRx;
                    break;

                case LF:
                    /* encountered a newline, first add data (without newline) */
                    if (pRx != pMrk)
                    {
                        add_to_trigger_buf(user, pMrk, pRx-pMrk);
                    }

                    /* also grab the newline */
                    pRx = g_utf8_next_char(pRx);

                    /* newline is now also sent to user window */
                    if (pRx != pMrk)
                    {
                        gui_add_ansi_window(user, pMrk, pRx-pMrk);
                    }

                    /* update the marker to the new position */
                    pMrk = pRx;

                    /* finally check if this line is triggered */
                    trigger_check(user, TRUE);
                    break;

                case BS:
                    /* encountered a backspace, flush buffer and skip it */
                    if (pRx != pMrk)
                    {
                        /* do not print the last character in the buffer */
                        gchar *pTmp = g_utf8_prev_char(pRx);

                        gui_add_ansi_window(user, pMrk, pTmp-pMrk);
                        add_to_trigger_buf(user, pMrk, pTmp-pMrk);
                    }
                    else
                    {
                        /* delete the last character */
                        gui_user_backspace(user);
                    }

                    /* skip the BS */
                    pMrk = pRx = g_utf8_next_char(pRx);
                    break;

                case BEEP:
                    /* encountered a beep, flush buffer and skip it */
                    if (pRx != pMrk)
                    {
                        gui_add_ansi_window(user, pMrk, pRx-pMrk);
                        add_to_trigger_buf(user, pMrk, pRx-pMrk);
                    }

                    /* sound the beep :) */
#if !defined(WIN32)
                    gdk_beep();
#else
                    MessageBeep(MB_OK);
#endif

                    /* skip the BEEP */
                    pMrk = pRx = g_utf8_next_char(pRx);
                    break;

                default:
                    if (!g_unichar_isprint(c))
                    {
                        /* unsupported character, flush buffer */
                        if (pRx != pMrk)
                        {
                            gui_add_ansi_window(user, pMrk, pRx-pMrk);
                            add_to_trigger_buf(user, pMrk, pRx-pMrk);
                        }

                        /* skip it */
                        pMrk = pRx = g_utf8_next_char(pRx);
                    }
                    else
                    {
                        /* just continue with next character */
                        pRx = g_utf8_next_char(pRx);
                    }
                    break;
            }
        }
    }

    /* did we put all in the window yet? if not do it now */
    if (pRx != pMrk)
    {
        gui_add_ansi_window(user, pMrk, pRx-pMrk);
        add_to_trigger_buf(user, pMrk, pRx-pMrk);
    }

    /* finally check if this line is triggered */
    trigger_check(user, FALSE);

    /* free the temporary conversion buffer */
    g_free(rxbuf);

    return;
}


void process_rx_buffer(USER *user)
{
    unsigned char *pRx, *pMrk1, *pMrk2, *pEnd;

    /* check if the MCCP stream is allocated */
    if (MCCP_USER(user))
    {
        /* decompress MCCP */
        decompress_rxbuf(user);

        /* set pointers to the MCCP buffer */
        pRx  = pMrk1 = pMrk2 = user->net.mccp_out;
        pEnd                 = user->net.mccp_outp;
    }
    else
    {
        /* set pointers to the receive buffers */
        pRx  = pMrk1 = pMrk2 = user->net.rxbuf;
        pEnd                 = user->net.rxp;
    }

    /* look for IAC's */
    while (pRx != pEnd)
    {
        if (user->net.rx_proc_state == RX_PROC_STATE_DATA)
        {
            if (*pRx++ == IAC)
            {
                /* set the state to process IAC data */
                user->net.rx_proc_state = RX_PROC_STATE_IAC;
            }
            else
            {
                /* second marker follows pRx if non-IAC */
                pMrk2 = pRx;
            }
        }
        else
        {
            /* currently processing IAC data */
            int iac_length, iac_retval;

            iac_retval = check_iac(user, pRx, pEnd, &iac_length);

            switch (iac_retval)
            {
                case IAC_ESCAPED:
                    /* restore the state */
                    user->net.rx_proc_state = RX_PROC_STATE_DATA;

                    /* skip one IAC */
                    pMrk2 = pRx++;
                    break;

                case IAC_MCCP_START:
                    /* restore the state */
                    user->net.rx_proc_state = RX_PROC_STATE_DATA;

                    /* skip the complete IAC */
                    pRx += iac_length;

                    if (pMrk1 != pMrk2)
                    {
                        /* process the data that is still left */
                        process_rx_data(user, pMrk1, pMrk2-pMrk1);
                    }

                    /* move the binary data after this IAC */
                    if (pRx < pEnd)
                    {
                        memmove(user->net.rxbuf, pRx, pEnd-pRx);
                        user->net.rxp = user->net.rxbuf + (pEnd-pRx);

                        /* re-call ourselves */
                        process_rx_buffer(user);
                    }
                    else
                    {
                        user->net.rxp = user->net.rxbuf;
                    }
                    return;     /* NOTE: return */

                case IAC_OK:
                    /* restore the state */
                    user->net.rx_proc_state = RX_PROC_STATE_DATA;
                    /* NOTE: FALL-THROUGH */

                default:
                    /* just advance the buffer pointer */
                    pRx += iac_length;
                    break;
            }

            if (pMrk1 != pMrk2)
            {
                /* process the data that is still left */
                process_rx_data(user, pMrk1, pMrk2-pMrk1);
            }

            /* reset the markers */
            pMrk1 = pMrk2 = pRx;
        }
    }

    /* reset reception buffer pointers */
    if (MCCP_USER(user))
    {
        user->net.mccp_outp = user->net.mccp_out;
    }
    else
    {
        user->net.rxp = user->net.rxbuf;
    }

    if (pMrk1 != pMrk2)
    {
        /* process the data that is still left */
        process_rx_data(user, pMrk1, pMrk2-pMrk1);
    }

    return;
}
