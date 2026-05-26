/* vim:set cin ft=c sw=4 sts=4 ts=8 et ai cino=Ls\:0t0(0 : -*- mode:c;fill-column:80;tab-width:8;c-basic-offset:4;indent-tabs-mode:nil;c-file-style:"k&r" -*-*/
/* NetHack 3.6 cursmisc.h */
/* Copyright (c) Karl Garrison, 2010. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CURSMISC_H
# define CURSMISC_H

/* Detect ncurses 6.1+ extended-color API (init_extended_pair,
   init_extended_color). These declarations appear in curses.h
   from ncurses 6.1 onward regardless of how the library was
   configured (the NCURSES_EXT_COLORS define is a separate axis
   that only controls whether NCURSES_PAIRS_T is short vs int,
   not whether the functions exist). Gate on the version macros
   so older ncurses and PDCurses fall back to plain init_pair();
   the 16/256-palette path still works in either case */
#if defined(NCURSES_VERSION_MAJOR)                                    \
    && (NCURSES_VERSION_MAJOR > 6                                     \
        || (NCURSES_VERSION_MAJOR == 6 && NCURSES_VERSION_MINOR >= 1))
# define NH_NCURSES_EXT_COLORS 1
#else
# define NH_NCURSES_EXT_COLORS 0
#endif

/* Global declarations */

int curses_getch(void);
int curses_read_char(void);
extern boolean curses_direct_color;
int nh_init_pair(int pair, int fg, int bg);
void curses_toggle_color_attr(WINDOW *win, int color, int attr, int onoff);
void curses_toggle_color_attr32(WINDOW *win, int color, unsigned long nhcolor,
                                int attr, int onoff);
void curses_show_color_palette(void);
void curses_menu_color_attr(WINDOW *win, int color, int attr, int onoff);
void curses_bail(const char *mesg);
winid curses_get_wid(int type);
char *curses_copy_of(const char *s);
int curses_num_lines(const char *str, int width);
char *curses_break_str(const char *str, int width, int line_num);
char *curses_str_remainder(const char *str, int width, int line_num);
boolean curses_is_menu(winid wid);
boolean curses_is_text(winid wid);
int curses_convert_glyph(int ch, int glyph);
int curses_convert_ibm_glyph(int ch, int glyph);
void curses_move_cursor(winid wid, int x, int y);
void curses_update_stdscr_cursor(void);
void curses_prehousekeeping(void);
void curses_posthousekeeping(void);
void curses_view_file(const char *filename, boolean must_exist);
void curses_rtrim(char *str);
int curses_get_count(int first_digit);
int curses_convert_attr(int attr);
int curses_read_attrs(const char *attrs);
char *curses_fmt_attrs(char *);
int curses_convert_keys(int key);
int curses_get_mouse(int *mousex, int *mousey, int *mod);
void curses_mouse_support(int);

#endif /* CURSMISC_H */
