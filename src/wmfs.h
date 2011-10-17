/*
 *  wmfs2 by Martin Duquesnoy <xorg62@gmail.com> { for(i = 2011; i < 2111; ++i) ©(i); }
 *  For license, see COPYING.
 */

#ifndef WMFS_H
#define WMFS_H

/* Standard */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>
#include <err.h>
#include <sys/queue.h>

/* Xlib */
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* Local */

#define ButtonMask (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask  (ButtonMask | PointerMotionMask)
#define KeyMask    (KeyPressMask | KeyReleaseMask)

typedef unsigned int Flags;
typedef unsigned int Color;
typedef const char* Uicb;

enum barpos
{
     BarTop = 0,
     BarBottom,
     BarHide,
     BarLast
};

enum position
{
     Right = 0,
     Left,
     Top,
     Bottom,
     Center,
     PositionLast
};

enum size_hints
{
     BASEW, BASEH,
     INCW,  INCH,
     MAXW,  MAXH,
     MINW,  MINH,
     MINAX, MINAY,
     MAXAX, MAXAY,
     SHLAST
};

/*
 * Structures
 */

struct geo
{
     int x, y, w, h;
};

struct geo_list
{
     struct geo geo;
     SLIST_ENTRY(geo_list) next;
};

struct colpair
{
     Color fg, bg;
};

struct barwin
{
     struct geo geo;
     Window win;
     Drawable dr;
     Color fg, bg;
     void *ptr; /* Special cases */
     SLIST_HEAD(, mousebind) mousebinds;
     SLIST_ENTRY(barwin) next;  /* global barwin */
     SLIST_ENTRY(barwin) enext; /* element barwin */
};

struct element
{
     struct geo geo;
     struct infobar *infobar;
     int type;
     void (*func_init)(struct element *e);
     void (*func_update)(struct element *e);
     SLIST_HEAD(, barwin) bars;
     TAILQ_ENTRY(element) next;
};

struct infobar
{
     struct barwin *bar;
     struct geo geo;
     struct screen *screen;
     struct theme *theme;
     enum barpos pos;
     char *elemorder;
     TAILQ_HEAD(esub, element) elements;
     SLIST_ENTRY(infobar) next;
};

struct screen
{
     struct geo geo, ugeo;
     struct tag *seltag;
     int id;
     Flags elemupdate;
     TAILQ_HEAD(tsub, tag) tags;
     SLIST_HEAD(, infobar) infobars;
     SLIST_ENTRY(screen) next;
};

struct tag
{
     struct screen *screen;
     struct client *sel;
     struct client *prevsel;
     char *name;
     Flags flags;
     SLIST_HEAD(, client) clients;
     TAILQ_HEAD(ssub, layout_set) sets;
     TAILQ_ENTRY(tag) next;
};

struct client
{
     struct tag *tag;
     struct screen *screen;
     struct barwin *titlebar;
     struct geo geo, wgeo, tgeo, rgeo;
     struct colpair ncol, scol;
     int sizeh[SHLAST];
     char *title;
     int border, tbarw;
#define CLIENT_HINT_FLAG    0x01
#define CLIENT_IGNORE_ENTER 0x02
#define CLIENT_DID_WINSIZE  0x04
     Flags flags;
     Window win, frame;
     SLIST_ENTRY(client) next;  /* Global list */
     SLIST_ENTRY(client) tnext; /* struct tag list */
};

struct layout_set
{
     int n;
     SLIST_HEAD(, geo_list) geos;
     TAILQ_ENTRY(layout_set) next;
};

struct keybind
{
     unsigned int mod;
     void (*func)(Uicb);
     Uicb cmd;
     KeySym keysym;
     SLIST_ENTRY(keybind) next;
};

struct mousebind
{
     struct geo area;
     unsigned int button;
     bool use_area;
     void (*func)(Uicb);
     Uicb cmd;
     SLIST_ENTRY(mousebind) next;
};

struct theme
{
     char *name;

     /* Font */
     struct
     {
          int as, de, width, height;
          XFontSet fontset;
     } font;

     /* Bars */
     struct colpair bars;
     int bars_width;

     /* struct elements */
     struct colpair tags_n, tags_s; /* normal / selected */
     int tags_border_width;
     Color tags_border_col;

     /* client / frame */
     struct colpair client_n, client_s;
     Color frame_bg;
     int client_titlebar_width;
     int client_border_width;

     SLIST_ENTRY(theme) next;
};

struct wmfs
{
     /* X11 stuffs */
     Display *dpy;
     Window root;
     int xscreen, xdepth;
     Flags numlockmask;
     GC gc, rgc;
     Atom *net_atom;
     bool running;

     /* FIFO stuffs */
     struct
     {
          char *path;
          int fd;
     } fifo;

     /* Lists heads */
     struct
     {
          SLIST_HEAD(, screen) screen;
          SLIST_HEAD(, client) client;
          SLIST_HEAD(, keybind) keybind;
          SLIST_HEAD(, barwin) barwin;
          SLIST_HEAD(, theme) theme;
     } h;

     /*
      * Selected screen, client
      */
     struct screen *screen;
     struct client *client;

};

int wmfs_error_handler(Display *d, XErrorEvent *event);
int wmfs_error_handler_dummy(Display *d, XErrorEvent *event);
void wmfs_grab_keys(void);
void wmfs_numlockmask(void);
void wmfs_init_font(char *font, struct theme *t);
void wmfs_quit(void);
void uicb_reload(Uicb cmd);
void uicb_quit(Uicb cmd);


/* Single global variable */
struct wmfs *W;

#endif /* WMFS_H */
