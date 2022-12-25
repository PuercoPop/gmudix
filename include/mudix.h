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

#ifndef _MUDIX_H_
#define _MUDIX_H_

#include <glib.h>
#include <zlib.h>
#include "conf.h"
#include "gui.h"

/*
 * Defines
 */
#define MAX_STRING              1024
#define MAX_INPUT_LEN         	512
#define MAX_FILEPATH            256
#define MAX_SMALL_STR           128
#define MAX_HISTORY             50
#define MAX_ARGS                10
#define MAX_TRIG_ARG_LEN        MAX_STRING-(2*MAX_ARGS)
#define MAX_ALIAS_NESTS         5
#define MAX_STATUS_LEN          128
#define MAX_FOR_LOOPS           100

#define RXBUF_LENGTH            2048
#define IACBUF_LENGTH           128
#define ANSIBUF_LENGTH          128
#define MCCP_RX_LENGTH          RXBUF_LENGTH
#define CSIPARAMS               16
#define ADDR_STRING_SIZE        16
#define TIME_LEN                50
#define SB_TIMER                5
#define CONNECT_TIMER           1
#define TELNET_PORT             23
#define CHARSET_CONV            "UTF-8"

#define TIMER_TIMEOUT           1000    /* timers have a 1 second resolution */
#define TIMER_CONT             -1       /* indicates a continuous timer */
#define TIMER_ONESHOT          -2       /* indicates a oneshot timer */

#define DEFAULT_CHAR_SET        "ISO-8859-1"
#define DEFAULT_FONT            "monospace 10"
#define DEFAULT_FONT_INPUT      "monospace 10"
#define DEFAULT_WIDTH           675
#define DEFAULT_HEIGHT          590
#define DEFAULT_PREF_WIDTH      600
#define DEFAULT_PREF_HEIGHT     400
#define DEFAULT_CAPT_WIDTH      600
#define DEFAULT_CAPT_HEIGHT     200
#define DEFAULT_LINE_MAX        10000
#define DEFAULT_SPACING         0
#define DEFAULT_FG              "grey"
#define DEFAULT_BG              "black"
#define DEFAULT_VAR_SIGN        '%'
#define DEFAULT_CMD_STACK       '|'
#define DEFAULT_BLOCK_OPEN      '{'
#define DEFAULT_BLOCK_CLOSE     '}'
#define DEFAULT_CMD_CHAR        '#'
#define DEFAULT_SPEED_PATH      '$'
#define DIA_NEW_WIDTH           400
#define DIA_NEW_HEIGHT          100
#if !defined(WIN32)
  #define X_COMPENSATION        5
  #define Y_COMPENSATION        23
#else
  #define X_COMPENSATION        4
  #define Y_COMPENSATION        23
#endif
#define USER_PATH               ".gMUDix/user/"
#define LOG_PATH                ".gMUDix/"

#define BIT_1                   0x00000001
#define BIT_2                   0x00000002
#define BIT_3                   0x00000004
#define BIT_4                   0x00000008
#define BIT_5                   0x00000010
#define BIT_6                   0x00000020
#define BIT_7                   0x00000040
#define BIT_8                   0x00000080
#define BIT_9                   0x00000100
#define BIT_10                  0x00000200
#define BIT_11                  0x00000400
#define BIT_12                  0x00000800
#define BIT_13                  0x00001000
#define BIT_14                  0x00002000
#define BIT_15                  0x00004000
#define BIT_16                  0x00008000
#define BIT_17                  0x00010000
#define BIT_18                  0x00020000
#define BIT_19                  0x00040000
#define BIT_20                  0x00080000
#define BIT_21                  0x00100000
#define BIT_22                  0x00200000
#define BIT_23                  0x00400000
#define BIT_24                  0x00800000
#define BIT_25                  0x01000000
#define BIT_26                  0x02000000
#define BIT_27                  0x04000000
#define BIT_28                  0x08000000
#define BIT_29                  0x10000000
#define BIT_30                  0x20000000
#define BIT_31                  0x40000000
#define BIT_32                  0x80000000

#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    !FALSE
#endif

#define BEEP                    0x07
#define BS                      0x08
#define LF                      0x0A
#define CR                      0x0D
#define ESC                     0x1B
#define CSI                     0x9B

/* incoming ANSI codes from the server */
#define ANSI_DEFAULT            0
#define ANSI_BOLD               1
#define ANSI_ITALIC             3
#define ANSI_UNDERLINE          4
#define ANSI_BLINK              5
#define ANSI_REVERSE            7
#define ANSI_BOLD_OFF           22
#define ANSI_ITALIC_OFF         23
#define ANSI_UNDERLINE_OFF      24
#define ANSI_BLINK_OFF          25
#define ANSI_REVERSE_OFF        27
#define ANSI_FG_BLACK           30
#define ANSI_FG_RED             31
#define ANSI_FG_GREEN           32
#define ANSI_FG_BROWN           33
#define ANSI_FG_BLUE            34
#define ANSI_FG_MAGENTA         35
#define ANSI_FG_CYAN            36
#define ANSI_FG_WHITE           37
#define ANSI_FG_DEFAULT         39
#define ANSI_BG_BLACK           40
#define ANSI_BG_RED             41
#define ANSI_BG_GREEN           42
#define ANSI_BG_BROWN           43
#define ANSI_BG_BLUE            44
#define ANSI_BG_MAGENTA         45
#define ANSI_BG_CYAN            46
#define ANSI_BG_WHITE           47
#define ANSI_BG_DEFAULT         49

#define ATTR_BOLD               BIT_1
#define ATTR_UNDERLINE          BIT_2
#define ATTR_ITALIC             BIT_3
#define ATTR_BLINK              BIT_4
#define ATTR_REVERSE            BIT_5

#define IAC_OK                  0
#define IAC_INCOMPLETE          1
#define IAC_ESCAPED             1
#define IAC_MCCP_START          2

#define IAC_ESCAPED_SIZE        2
#define IAC_MCCP_START_SIZE     5

#define TRG_NORMAL		0
#define TRG_LOGIN              -1
#define TRG_PASSWORD           -2

#if defined(WIN32)
#define IAC                     255             /* interpret as command: */
#define DONT                    254             /* you are not to use option */
#define DO                      253             /* please, you use option */
#define WONT                    252             /* I won't use option */
#define WILL                    251             /* I will use option */
#define SB                      250             /* interpret as subnegotiation */
#define GA                      249             /* you may reverse the line */
#define EL                      248             /* erase the current line */
#define EC                      247             /* erase the current character */
#define AYT                     246             /* are you there */
#define AO                      245             /* abort output--but let prog finish */
#define IP                      244             /* interrupt process--permanently */
#define BREAK                   243             /* break */
#define DM                      242             /* data mark--for connect. cleaning */
#define NOP                     241             /* nop */
#define SE                      240             /* end sub negotiation */
#define EOR                     239             /* end of record (transparent mode) */
#define ABORT                   238             /* Abort process */
#define SUSP                    237             /* Suspend process */
#define xEOF                    236             /* End of file: EOF is already used... */

#define SYNCH                   242             /* for telfunc calls */
#endif

/* bitwise flags for user->flags */
#define FLG_SAVE_FLAGS_MASK     0xFFFF
#define FLG_CMD_OUTPUT_MUTE     BIT_1
#define FLG_INPUT_ECHO_OFF      BIT_2
#define FLG_INPUT_AUTO_CLEAR    BIT_3
/* try to use the MSB for not-saving flags */
#define FLG_NAWS_UPDATES        BIT_17

typedef struct alias_type       ALIAS;
typedef struct macro_type       MACRO;
typedef struct path_type        PATH;
typedef struct tab_type         TAB;
typedef struct timer_type       TIMER;
typedef struct trigger_type     TRIGGER;
typedef struct user_type        USER;
typedef struct var_type         VAR;
typedef struct capt_window_type CAPT_WINDOW;
typedef struct history_type     HISTORY;

typedef unsigned char           bool;

/* ANSI color mapping colors */
typedef enum
{
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BROWN,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_GREY,
    COLOR_BOLD_BLACK,
    COLOR_BOLD_RED,
    COLOR_BOLD_GREEN,
    COLOR_YELLOW,
    COLOR_BOLD_BLUE,
    COLOR_BOLD_MAGENTA,
    COLOR_BOLD_CYAN,
    COLOR_WHITE,
    NR_ANSI_COLORS,
    ANSI_COLOR_NONE=NR_ANSI_COLORS,
    COLOR_INFO=NR_ANSI_COLORS,
    COLOR_ECHO,
    NR_ALL_COLORS
} ANSI_COLORS;


#define COLOR_NONE  NR_ALL_COLORS

/* default colors for foreground/background (no tags required) */
typedef enum
{
    DEF_FG,
    DEF_BG,
    NR_DEF_COLORS
} DEF_COLORS;


/* fonts */
typedef enum
{
    FONT_USER_WINDOW,
    FONT_USER_SCROLLBACK,
    FONT_USER_INPUT,
    FONT_USER_CAPTURE,
    NR_FONTS
} DEF_FONTS;


/* network status */
typedef enum
{
    /* connection status */
    NET_CLOSED,
    NET_CONNECTED,

    /* connection errors */
    NET_CONNECT_FAILURE,
    NET_GETHOSTBYNAME,
    NET_NO_SOCKET,

    /* max net status numbers */
    NET_NR_STATUS
} NET_CODE;


/* ANSI reception processing states */
typedef enum
{
    ANSI_STATE_IDLE,
    ANSI_STATE_ESC,
    ANSI_STATE_PARAMS
} ANSI_RX_STATE;


/* rx processing states */
typedef enum
{
    RX_PROC_STATE_DATA,
    RX_PROC_STATE_IAC,
    RX_PROC_STATE_SB,
    RX_PROC_STATE_WILL,
    RX_PROC_STATE_WONT,
    RX_PROC_STATE_DO,
    RX_PROC_STATE_DONT
} RX_PRC_STATE;


/* customizable characters */
typedef enum
{
    CUST_CMD_STACK,
    CUST_VAR_SIGN,
    CUST_BLOCK_OPEN,
    CUST_BLOCK_CLOSE,
    CUST_CMD_CHAR,
    CUST_SPEED_PATH,
    NR_CUSTOM_CHARS
} CUST_CHARS;


/*
 * Global functions
 */
/* alias.c */
extern ALIAS     *new_alias                 (USER *user);
extern ALIAS     *new_alias_append          (USER *user);
extern ALIAS     *create_alias              (USER *user, gchar *name, gchar *text);
extern void       free_alias                (USER *user, ALIAS *alias);
extern void       cleanup_alias_list        (USER *user);
extern bool       process_alias             (USER *user, gchar *buffer);

/* commands.c */
extern bool       do_command                (USER *user, gchar *cmdline);
extern void       cmd_mute                  (USER *user, int index, char *args);
extern void       cmd_input_clear           (USER *user, int index, char *args);
extern void       cmd_input_echo            (USER *user, int index, char *args);

/* history.c */
extern void       add_to_history            (USER *user, gchar *input);
extern void       init_history              (USER *user);
extern void       cleanup_history           (USER *user);

/* log.c */
extern bool       open_log                  (USER *user, char *filename, bool date);
extern bool       close_log                 (USER *user);
extern void       write_log                 (USER *user, gchar *data, gsize length);

/* macro.c */
extern MACRO     *new_macro                 (USER *user);
extern MACRO     *new_macro_append          (USER *user);
extern MACRO     *create_macro              (USER *user, guint key, guint state, gchar *text);
extern MACRO     *macro_lookup              (USER *user, guint key, guint state);
extern void       free_macro                (USER *user, MACRO *macro);
extern void       cleanup_macro_list        (USER *user);
extern bool       process_macro             (USER *user, guint key, guint state);
extern void       setup_default_macros      (USER *user);

/* mccp.c */
extern void       decompress_rxbuf          (USER *user);
extern void       mccp_open                 (USER *user);
extern void       mccp_close                (USER *user);

/* net.c */
extern int        write_data                (USER *user, gchar *buffer, int len);
extern int        read_data                 (USER *user);
extern void       do_disconnect             (USER *user);
extern void      *connect_thread            (USER *user);

/* path.c */
extern PATH      *new_path                  (USER *user);
extern PATH      *new_path_append           (USER *user);
extern PATH      *create_path               (USER *user, gchar *name, gchar *text);
extern void       free_path                 (USER *user, PATH *path);
extern void       cleanup_path_list         (USER *user);
extern bool       process_path              (USER *user, gchar *pBuffer);

/* process.c */
extern void       process_input             (USER *user, gchar *input, gchar *args);
extern void       process_rx_buffer         (USER *user);
extern gchar     *parse_input               (USER *user, gchar *src, gchar *args, gchar *dest,  int *len);
extern bool       process_for               (USER *user, gchar *buffer);
extern bool       process_if                (USER *user, gchar *buffer);

/* string.c */
extern gchar     *smash_tilde               (gchar *buf);
extern bool       is_numeric                (gchar *str, gdouble *value);
extern gchar     *str_dup                   (gchar *dest, gchar *src);
extern gchar     *get_arg                   (USER *user, gchar *src, gchar *dst);

/* tabs.c */
extern TAB       *new_tab                   (USER *user);
extern TAB       *new_tab_append            (USER *user);
extern TAB       *create_tab                (USER *user, gchar *name);
extern void       free_tab                  (USER *user, TAB *tab);
extern void       cleanup_tabs_list         (USER *user);
extern void       check_tab                 (USER *user, gchar *buffer);

/* telnet.c */
extern int        check_iac                 (USER *user, unsigned char *pStart, unsigned char *pEnd, int *length);
extern void       send_naws                 (USER *user);

/* timer.c */
extern TIMER     *new_timer                 (USER *user);
extern TIMER     *new_timer_append          (USER *user);
extern TIMER     *create_timer              (USER *user, gchar *resp, gint time, gint reload);
extern void       free_timer                (USER *user, TIMER *timer);
extern void       cleanup_timer_list        (USER *user);
extern void       set_timer                 (USER *user, TIMER *timer, gchar *resp, gint time, gint reload);
extern guint      init_timer                (USER *user);
extern void       sync_timer                (USER *user);

/* trigger.c */
extern TRIGGER   *new_trigger               (USER *user, int level);
extern TRIGGER   *new_trigger_append        (USER *user, int level);
extern TRIGGER   *create_trigger            (USER *user, int level, gchar *text, gchar *response);
extern TRIGGER   *trig_lookup               (USER *user, TRIGGER *beg, int level);
extern void       free_trigger              (USER *user, TRIGGER *trigger);
extern void       cleanup_trigger_list      (USER *user);
extern void       set_trigger               (USER *user, TRIGGER *trigger, gchar *text, char *response);
extern void       trigger_check             (USER *user, bool fNewline);

/* user.c */
extern USER      *new_user                  (void);
extern void       del_user                  (USER *user);
extern bool       load_user                 (USER *user, char *file);
extern bool       save_user                 (USER *user);
extern bool       check_valid_user          (USER *user);
extern bool       user_read_info            (char *path, char **name, char **site, guint *port);
extern USER      *get_user_with_color       (GdkColor *color);
extern USER      *get_user_window           (GtkWindow *window);
extern USER      *get_user_id               (guint id);
extern USER      *get_user_file             (char *file);
extern USER      *get_user_session          (gchar *session);
extern void       init_user_mutex           (void);
extern int        user_count                (void);

/* variable.c */
extern VAR       *new_var                   (USER *user);
extern VAR       *new_var_append            (USER *user);
extern VAR       *create_var                (USER *user, gchar *name, gchar *text);
extern VAR       *var_lookup                (USER *user, gchar *var);
extern void       free_var                  (USER *user, VAR *var);
extern void       cleanup_vars_list         (USER *user);

/* gui_xxx.c */
extern void       init_gui                  (int argc, char *argv[]);
extern void       init_charset_list         (void);
extern void       gui_setup_main_window     (void);
extern void       gui_add_ansi_window       (USER *user, gchar *buf, gsize len);
extern void       gui_add_color_window      (USER *user, gchar *buf, gsize len, guint color);
extern void       gui_status_msg            (USER *user, gchar *msg);
extern void       gui_set_title             (USER *user, gchar *msg);
extern void       gui_setup_user_window     (USER *user);
extern void       gui_user_setup_tags       (USER *user);
extern void       gui_user_connect          (USER *user);
extern void       gui_user_disconnect       (USER *user);
extern void       gui_user_reconnect        (USER *user);
extern void       gui_dialog_msg            (USER *user, gchar *msg);
extern void       gui_preference_editor     (USER *user, SELECT_TREE select);
extern void       gui_color_def_widget      (USER *user, GtkWidget *widget);
extern void       gui_color_set_def_all     (USER *user);
extern void       gui_color_set_def         (GdkColor *color, ANSI_COLORS index);
extern void       gui_user_dialog_file      (USER *user);
extern void       gui_main_update_row       (USER *user);
extern void       gui_font_update           (USER *user, DEF_FONTS index);
extern void       gui_font_spacing_update   (USER *user);
extern void       gui_alias_add_pref        (USER *user, ALIAS *alias);
extern void       gui_macro_add_pref        (USER *user, MACRO *macro);
extern void       gui_path_add_pref         (USER *user, PATH *path);
extern void       gui_tab_add_pref          (USER *user, TAB *tab);
extern void       gui_timer_add_pref        (USER *user, TIMER *timer);
extern void       gui_trigger_add_pref      (USER *user, TRIGGER *trigger);
extern void       gui_var_add_pref          (USER *user, VAR *var);
extern void       gui_timer_update_list     (USER *user);
extern void       gui_timer_remove          (USER *user, TIMER *search);
extern void       gui_set_window_title      (USER *user);
extern void       gui_user_backspace        (USER *user);
extern gboolean   gui_connection_status     (USER *user);
extern void       gui_add_to_capt           (USER *user, gchar *title, gchar *buf, gsize len, guint attrib);
extern void       gui_user_get_xy           (USER *user, gint *x, gint *y);
extern bool       gui_cls_capt              (USER *user, gchar *title);
extern void       gui_user_cls              (USER *user);

extern GtkWidget *gui_alias_create          (USER *user);
extern GtkWidget *gui_macro_create          (USER *user);
extern GtkWidget *gui_path_create           (USER *user);
extern GtkWidget *gui_tab_create            (USER *user);
extern GtkWidget *gui_timer_create          (USER *user);
extern GtkWidget *gui_trigger_create        (USER *user);
extern GtkWidget *gui_var_create            (USER *user);
extern GtkWidget *gui_general_create        (USER *user);
extern GtkWidget *gui_character_create      (USER *user);
extern GtkWidget *gui_color_create          (USER *user);
extern GtkWidget *gui_font_create           (USER *user);
extern GtkWidget *gui_get_wid_frame         (USER *user, int frame, SELECT_TREE select);
extern GtkWidget *gui_get_nth_child         (GtkWidget *container, int n);
extern GtkWidget *gui_get_child_type        (GtkWidget *container, int n, guint type);

/* global mutex for connection thread */
extern GMutex    *user_network_mutex;
/* the list of users */
extern USER      *user_list;

/* macros */
#define exit_mudix()                    (gtk_main_quit())
#define get_prev_history_update(user)   (((user)->pGetHist = (user)->pGetHist->prev)->str)
#define get_next_history_update(user)   (((user)->pGetHist = (user)->pGetHist->next)->str)
#define get_sb_view(user)               (gtk_bin_get_child(GTK_BIN((user)->gui_user.g_scrollback)))
#define destroy_with_signal(object)     (g_signal_emit_by_name(G_OBJECT(object), "destroy"))
#define VALID_ANSI(color)               ((color) != ANSI_COLOR_NONE)
#define MCCP_USER(user)                 ((user)->net.stream)


struct history_type
{
    HISTORY    *next;
    HISTORY    *prev;
    gchar      *str;
};


/* data structure for ansi coding */
typedef struct
{
    guchar         fg;
    guchar         bg;
    guint          attrib;
    char           parambuf[ANSIBUF_LENGTH];
    char          *paramp;
    ANSI_RX_STATE  state;
} ANSI_T;


/* data structure for the user GUI */
typedef struct
{
    GtkWidget     *g_window;
    GtkWidget     *g_view;
    GtkWidget     *g_scrollback;
    GtkWidget     *g_status;
    GtkWidget     *g_input;
    GtkWidget     *g_combo;
    GtkTextBuffer *g_buffer;
    GtkTextMark   *g_mark;
    GtkTextTag    *g_fg_color_tags[NR_ALL_COLORS];
    GtkTextTag    *g_bg_color_tags[NR_ALL_COLORS];
    GtkTextTag    *g_underline_tag;
    GtkTextTag    *g_italic_tag;
    GtkTextIter    input_iter;
    GtkWrapMode    wrap_mode;
    GdkColor       default_color[NR_DEF_COLORS];
    GdkColor       colors[NR_ALL_COLORS];
    gchar         *fonts[NR_FONTS];
    gint           win_width;
    gint           win_height;
    gint           win_pos_x;
    gint           win_pos_y;
    gint           input_select_start;
    gint           input_select_end;
    gint           lines_max;
    gint           space_above;
    gint           space_below;
} GUI_USER;


/* data structure for the preference GUI */
typedef struct
{
    GtkWidget     *g_preference;
    GtkWidget     *g_selecttree;
    GtkWidget     *g_edit[NUM_SELECT];
    gint           pref_width;
    gint           pref_height;
} GUI_PREF;


/* data structure for the trigger GUI */
typedef struct
{
    GtkWidget     *entry1;
    GtkWidget     *entry2;
    GtkWidget     *spin;
} GUI_TRIGGER;


/* network related data structure */
typedef struct
{
    unsigned char *rxbuf;
    unsigned char *rxp;
    unsigned char *iacbuf;
    unsigned char *iacp;
    z_stream      *stream;
    unsigned char *mccp_out;
    unsigned char *mccp_outp;
    guint          mccp_outsize;
    gchar         *charset;
    gchar         *site;
    gchar         *host_name;
    gchar          host_addr[ADDR_STRING_SIZE];
    gint           rx_tag;
    gint           exc_tag;
    guint          thread_id;
    guint          recon_timer_load;
    guint          recon_timer;
    guint          recon_id;
    guint          port;
    gint           sock;
    RX_PRC_STATE   rx_proc_state;
    NET_CODE       status;
} NETWORK;


/* data structure containing all data for a user */
struct user_type
{
    USER          *next;
    GUI_USER       gui_user;
    GUI_PREF       gui_pref;
    GUI_TRIGGER    gui_trigger;
    NETWORK        net;
    ALIAS         *alias_list;
    MACRO         *macro_list;
    PATH          *path_list;
    TAB           *tabs_list;
    TIMER         *timer_list;
    TRIGGER       *trigger_list;
    VAR           *vars_list;
    FILE          *logfile;
    HISTORY       *pCurHist;
    HISTORY       *pGetHist;
    gchar         *filename;
    gchar         *session;
    gchar         *trigger_buf;
    gchar         *trigger;
    guint          timer_id;
    gunichar       custom_chars[NR_CUSTOM_CHARS];
    guint          id;
    guint          flags;
    ANSI_T         ansi;
};


/* trigger data */
struct trigger_type
{
    TRIGGER     *next;
    gchar       *input;
    gchar       *response;
    GtkTextTag  *color_tag;
    gchar 	*pArg;
    gchar        arg[MAX_STRING];
    gint         level;
    bool     	 enabled;
};


/* timer data */
struct timer_type
{
    TIMER       *next;
    gchar       *response;
    gint         relval;
    gint         relcnt;
    gint         timer;
};


/* alias data */
struct alias_type
{
    ALIAS       *next;
    gchar       *name;
    gchar       *string;
};


/* macro data */
struct macro_type
{
    MACRO       *next;
    gchar       *text;
    guint        key;
    guint        state;
};


/* path data */
struct path_type
{
    PATH        *next;
    gchar       *name;
    gchar       *path;
};


/* tab completion data */
struct tab_type
{
    TAB         *next;
    gchar       *name;
};


/* variable data */
struct var_type
{
    VAR         *next;
    gchar       *name;
    gchar       *value;         /* value is a string also */
};


/* command table data */
struct cmd_table_type
{
    gchar       *cmd;
    void       (*function)(USER *user, int index, gchar *args);
    gchar       *syntax;
    gchar       *description;
};


struct capt_window_type
{
    CAPT_WINDOW     *next;
    GtkWidget       *g_window;
    GtkWidget       *g_view;
    GtkTextBuffer   *g_buffer;
    GtkTextMark     *g_mark;
    GtkTextTag      *g_fg_color_tags[NR_ALL_COLORS];
};

#endif
