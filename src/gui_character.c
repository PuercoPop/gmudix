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


static       GList *charset_list = NULL;
static const gchar *charsets[] =
{
    "ANSI_X3.110-1983",
    "ANSI_X3.4-1968",
    "ASMO_449",
    "BIG5",
    "BIG5HKSCS",
    "BS_4730",
    "CP1250",
    "CP1251",
    "CP1252",
    "CP1253",
    "CP1254",
    "CP1255",
    "CP1256",
    "CP1257",
    "CP1258",
    "CP737",
    "CP775",
    "CP949",
    "CSA_Z243.4-1985-1",
    "CSA_Z243.4-1985-2",
    "CSN_369103",
    "CWI",
    "DEC-MCS",
    "DIN_66003",
    "DS_2089",
    "EBCDIC-AT-DE",
    "EBCDIC-AT-DE-A",
    "EBCDIC-CA-FR",
    "EBCDIC-DK-NO",
    "EBCDIC-DK-NO-A",
    "EBCDIC-ES",
    "EBCDIC-ES-A",
    "EBCDIC-ES-S",
    "EBCDIC-FI-SE",
    "EBCDIC-FI-SE-A",
    "EBCDIC-FR",
    "EBCDIC-IS-FRISS",
    "EBCDIC-IT",
    "EBCDIC-PT",
    "EBCDIC-UK",
    "EBCDIC-US",
    "ECMA-CYRILLIC",
    "ES",
    "ES2",
    "EUC-JP",
    "EUC-KR",
    "EUC-TW",
    "GB18030",
    "GB2312",
    "GBK",
    "GB_1988-80",
    "GOST_19768-74",
    "GREEK-CCITT",
    "GREEK7",
    "GREEK7-OLD",
    "HP-ROMAN8",
    "IBM037",
    "IBM038",
    "IBM1004",
    "IBM1026",
    "IBM1047",
    "IBM256",
    "IBM273",
    "IBM274",
    "IBM275",
    "IBM277",
    "IBM278",
    "IBM280",
    "IBM281",
    "IBM284",
    "IBM285",
    "IBM290",
    "IBM297",
    "IBM420",
    "IBM423",
    "IBM424",
    "IBM437",
    "IBM500",
    "IBM850",
    "IBM851",
    "IBM852",
    "IBM855",
    "IBM857",
    "IBM860",
    "IBM861",
    "IBM862",
    "IBM863",
    "IBM864",
    "IBM865",
    "IBM866",
    "IBM868",
    "IBM869",
    "IBM870",
    "IBM871",
    "IBM874",
    "IBM875",
    "IBM880",
    "IBM891",
    "IBM903",
    "IBM904",
    "IBM905",
    "IBM918",
    "IEC_P27-1",
    "INIS",
    "INIS-8",
    "INIS-CYRILLIC",
    "ISIRI-3342",
    "ISO-8859-1",
    "ISO-8859-10",
    "ISO-8859-13",
    "ISO-8859-14",
    "ISO-8859-15",
    "ISO-8859-16",
    "ISO-8859-2",
    "ISO-8859-3",
    "ISO-8859-4",
    "ISO-8859-5",
    "ISO-8859-6",
    "ISO-8859-7",
    "ISO-8859-8",
    "ISO-8859-9",
    "ISO-IR-197",
    "ISO-IR-90",
    "ISO_10367-BOX",
    "ISO_2033-1983",
    "ISO_5427",
    "ISO_5427-EXT",
    "ISO_5428",
    "ISO_6937",
    "IT",
    "JIS_C6220-1969-RO",
    "JIS_C6229-1984-B",
    "JOHAB",
    "JUS_I.B1.002",
    "KOI-8",
    "KOI8-R",
    "KOI8-U",
    "KSC5636",
    "LATIN-GREEK",
    "LATIN-GREEK-1",
    "MAC-IS",
    "MAC-UK",
    "MACINTOSH",
    "MSZ_7795.3",
    "NATS-DANO",
    "NATS-SEFI",
    "NC_NC00-10",
    "NF_Z_62-010",
    "NF_Z_62-010_(1973)",
    "NF_Z_62-010_1973",
    "NS_4551-1",
    "NS_4551-2",
    "PT",
    "PT2",
    "SEN_850200_B",
    "SEN_850200_C",
    "SJIS",
    "T.61-8BIT",
    "TIS-620",
    "UTF-8",
    "VISCII",
    "WIN-SAMI-2",
    NULL
};


void init_charset_list(void)
{
    int i=0;

    if (charset_list)
    {
        /* only to be called once */
        return;
    }

    while (charsets[i])
    {
        /* create the list */
        charset_list = g_list_append(charset_list, (gchar *)charsets[i++]);
    }
}


static gboolean gui_char_entry_unichar(GtkEntry         *entry,
                                       GdkEventFocus    *event,
                                       gunichar         *value)
{
    const gchar *text;

    text = gtk_entry_get_text(entry);

    *value = g_utf8_get_char(text);

    return FALSE;
}


static gboolean gui_char_entry_string(GtkEntry         *entry,
                                      GdkEventFocus    *event,
                                      gchar           **string)
{
    const gchar *text;

    text = gtk_entry_get_text(entry);

    free(*string);
    *string = strdup(text);

    return FALSE;
}


GtkWidget *gui_character_create(USER *user)
{
    GtkWidget *mainbox;
    GtkWidget *entry;
    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *combo;
    GtkWidget *table;
    gchar      tempbuf[MAX_SMALL_STR];

    mainbox = gtk_vbox_new(FALSE, 0);

    /* create a hbox for a combo */
    hbox = gtk_hbox_new(FALSE, 0);

    /* create the combo */
    combo = gtk_combo_new();

    /* put combo in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), combo, TRUE, TRUE, 2);

    /* put the combo with a frame inside the mainbox */
    frame = gtk_frame_new("Server's Character Set");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    gtk_combo_set_popdown_strings(GTK_COMBO(combo), charset_list);
    gtk_combo_set_value_in_list(GTK_COMBO(combo), TRUE, FALSE);
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry), user->net.charset);

    /* update on focus-out */
    g_signal_connect(G_OBJECT(GTK_COMBO(combo)->entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_string), &user->net.charset);

    /* create a hbox for a table */
    hbox = gtk_hbox_new(TRUE, 0);

    /* put the entry with a frame inside the mainbox */
    frame = gtk_frame_new("Customizable Characters");
    gtk_container_set_border_width(GTK_CONTAINER(hbox), 5);
    gtk_container_add(GTK_CONTAINER(frame), hbox);
    gtk_box_pack_start(GTK_BOX(mainbox), frame, FALSE, FALSE, 0);

    table = gtk_table_new(1, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(hbox), table);

    /* set up the left part of a row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, 0, 1);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_CMD_STACK]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_CMD_STACK], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Command Stack");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* set up the right part of the row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 0, 1);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_VAR_SIGN]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_VAR_SIGN], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Variable Sign");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* set up the left part of a row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, 2, 3);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_BLOCK_OPEN]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_BLOCK_OPEN], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Block Open");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* set up the right part of the row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 2, 3);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_BLOCK_CLOSE]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_BLOCK_CLOSE], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Block Close");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* set up the left part of a row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 0, 1, 4, 5);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_CMD_CHAR]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_CMD_CHAR], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Command Character");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    /* set up the right part of the row */
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_table_attach_defaults(GTK_TABLE(table), hbox, 1, 2, 4, 5);

    /* create the entry */
    entry = gtk_entry_new();

    /* update on focus-out */
    g_signal_connect(G_OBJECT(entry), "focus-out-event",
		     G_CALLBACK(gui_char_entry_unichar),
                     &user->custom_chars[CUST_SPEED_PATH]);

    /* put entry in hbox */
    gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 2);

    /* set max length of the entry */
    gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 2);

    /* convert the unicode to UTF8 and put it in the entry */
    tempbuf[g_unichar_to_utf8(user->custom_chars[CUST_SPEED_PATH], tempbuf)] = '\0';
    gtk_entry_set_text(GTK_ENTRY(entry), tempbuf);

    label = gtk_label_new("Speed path");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);

    return mainbox;
}
