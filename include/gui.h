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

#ifndef _GUI_H_
#define _GUI_H_

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

typedef enum
{
    SEL_GENERAL,
    SEL_CHAR,
    SEL_COLOR,
    SEL_FONTS,
    SEL_ALIASES,
    SEL_MACROS,
    SEL_PATHS,
    SEL_TABS,
    SEL_TIMERS,
    SEL_TRIGGERS,
    SEL_VARS,
    NUM_SELECT
} SELECT_TREE;

#define gui_destroy_window(user)        (gtk_widget_destroy((user)->gui_user.g_window))
#define gui_clear_status(user)          (gtk_statusbar_pop(GTK_STATUSBAR((user)->gui_user.g_status), 0))
#define gui_input_set(user, buf)        (gtk_entry_set_text(GTK_ENTRY((user)->gui_user.g_input), (buf)))
#define gui_input_cursor_end(user)      (gtk_editable_set_position(GTK_EDITABLE((user)->gui_user.g_input), -1))
#define gui_input_clear(user)           (gtk_editable_delete_text(GTK_EDITABLE((user)->gui_user.g_input), 0, -1))
#define gui_input_visible(user, flag)   (gtk_entry_set_visibility(GTK_ENTRY((user)->gui_user.g_input), (flag)))
#define gui_input_is_vis(user)          (GTK_ENTRY((user)->gui_user.g_input)->visible)
#define gui_info_message(user, msg)     (gui_add_color_window((user), (msg), strlen((msg)), COLOR_INFO))

#endif
