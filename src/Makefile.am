bin_PROGRAMS   = gmudix
gmudix_SOURCES = alias.c commands.c file.c gui_alias.c gui_capture.c gui_character.c gui_color.c gui_font.c gui_general.c gui_macro.c gui_main.c gui_path.c gui_preferences.c gui_tab.c gui_timer.c gui_trigger.c gui_user.c gui_variable.c history.c log.c macro.c mccp.c mudix.c net.c path.c process.c string.c tabs.c telnet.c timer.c trigger.c user.c variable.c
INCLUDES = -Wall -I.. -I../include `pkg-config --cflags gtk+-2.0`
gmudix_LDFLAGS = -lz `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`

if DEBUG_SYMBOLS
CFLAGS := $(CFLAGS) -O2 -g
else
CFLAGS := $(CFLAGS) -O2 
endif
