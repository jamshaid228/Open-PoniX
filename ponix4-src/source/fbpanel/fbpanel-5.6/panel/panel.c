/*! \file panel.c
 *  \brief main code is here
 *
 *             No details
 *
 */

/*! \mainpage My Personal Index Page
 *
 * \section intro_sec Introduction
 *
 * This is the introduction.
 *
 * \section install_sec Installation
 *
 * \subsection step1 Step 1: Opening the box
 *
 * etc...
 */


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include "plugin.h"
#include "panel.h"
#include "misc.h"
#include "bg.h"
#include "gtkbgbox.h"

/// config file
static gchar *cfgfile = NULL;
static gchar version[] = VERSION;
gchar *cprofile = "default";

guint mwid; // mouse watcher thread id
guint hpid; // hide panel thread id

int config = 0;
FbEv *fbev;
gint force_quit = 0;

//#define DEBUGPRN
#include "dbg.h"


/** verbosity level of dbg and log functions */
int log_level = LOG_WARN;

FILE *pconf; // plugin part of profile file

panel *p;
panel *the_panel;

/****************************************************
 *         panel's handlers for WM events           *
 ****************************************************/
/*
  static void
  panel_del_wm_strut(panel *p)
  {
  XDeleteProperty(GDK_DISPLAY(), p->topxwin, a_NET_WM_STRUT);
  XDeleteProperty(GDK_DISPLAY(), p->topxwin, a_NET_WM_STRUT_PARTIAL);
  }
*/

/** realy brie
 * cont. detailed follow
 * sdfsdfdsf
 * \param p panel yanni
 * \return nothing or
 * nothing
 * \retval 0 tipa nol
 * \retval !0 - ok
 */

void
panel_set_wm_strut(panel *p)
{
    gulong data[12] = { 0 };
    int i = 4;

    ENTER;
    if (!GTK_WIDGET_MAPPED (p->topgwin))
        return;
    switch (p->edge) {
    case EDGE_LEFT:
        i = 0;
        data[i] = p->aw;
        data[4 + i*2] = p->ay;
        data[5 + i*2] = p->ay + p->ah;
        if (p->autohide) data[i] = p->height_when_hidden;
        break;
    case EDGE_RIGHT:
        i = 1;
        data[i] = p->aw;
        data[4 + i*2] = p->ay;
        data[5 + i*2] = p->ay + p->ah;
        if (p->autohide) data[i] = p->height_when_hidden;
        break;
    case EDGE_TOP:
        i = 2;
        data[i] = p->ah;
        data[4 + i*2] = p->ax;
        data[5 + i*2] = p->ax + p->aw;
        if (p->autohide) data[i] = p->height_when_hidden;
        break;
    case EDGE_BOTTOM:
        i = 3;
        data[i] = p->ah;
        data[4 + i*2] = p->ax;
        data[5 + i*2] = p->ax + p->aw;
        if (p->autohide) data[i] = p->height_when_hidden;
        break;
    default:
        ERR("wrong edge %d. strut won't be set\n", p->edge);
        RET();
    }
    DBG("type %d. width %ld. from %ld to %ld\n", i, data[i], data[4 + i*2],
          data[5 + i*2]);

    /* if wm supports STRUT_PARTIAL it will ignore STRUT */
    XChangeProperty(GDK_DISPLAY(), p->topxwin, a_NET_WM_STRUT_PARTIAL,
          XA_CARDINAL, 32, PropModeReplace,  (unsigned char *) data, 12);
    /* old spec, for wms that do not support STRUT_PARTIAL */
    XChangeProperty(GDK_DISPLAY(), p->topxwin, a_NET_WM_STRUT,
          XA_CARDINAL, 32, PropModeReplace,  (unsigned char *) data, 4);

    RET();
}

static void
print_wmdata(panel *p)
{
    int i;

    ENTER;
    RET();
    DBG("desktop %d/%d\n", p->curdesk, p->desknum);
    DBG("workarea\n");
    for (i = 0; i < p->wa_len/4; i++)
        DBG("(%d, %d) x (%d, %d)\n",
              p->workarea[4*i + 0],
              p->workarea[4*i + 1],
              p->workarea[4*i + 2],
              p->workarea[4*i + 3]);
    RET();
}


static GdkFilterReturn
panel_event_filter(GdkXEvent *xevent, GdkEvent *event, panel *p)
{
    Atom at;
    Window win;
    XEvent *ev = (XEvent *) xevent;

    ENTER;
    DBG("win = 0x%lx\n", ev->xproperty.window);
    if (ev->type != PropertyNotify )
        RET(GDK_FILTER_CONTINUE);

    at = ev->xproperty.atom;
    win = ev->xproperty.window;
    DBG("win=%lx at=%ld\n", win, at);
    if (win == GDK_ROOT_WINDOW()) {
        if (at == a_NET_CLIENT_LIST) {
            DBG("A_NET_CLIENT_LIST\n");
            fb_ev_trigger(fbev, EV_CLIENT_LIST);
        } else if (at == a_NET_CURRENT_DESKTOP) {
            DBG("A_NET_CURRENT_DESKTOP\n");
            p->curdesk = get_net_current_desktop();
            fb_ev_trigger(fbev, EV_CURRENT_DESKTOP);
        } else if (at == a_NET_NUMBER_OF_DESKTOPS) {
            DBG("A_NET_NUMBER_OF_DESKTOPS\n");
            p->desknum = get_net_number_of_desktops();
            fb_ev_trigger(fbev, EV_NUMBER_OF_DESKTOPS);
        } else if (at == a_NET_DESKTOP_NAMES) {
            DBG("A_NET_DESKTOP_NAMES\n");
            fb_ev_trigger(fbev, EV_DESKTOP_NAMES);
        } else if (at == a_NET_ACTIVE_WINDOW) {
            DBG("A_NET_ACTIVE_WINDOW\n");
            fb_ev_trigger(fbev, EV_ACTIVE_WINDOW);
        }else if (at == a_NET_CLIENT_LIST_STACKING) {
            DBG("A_NET_CLIENT_LIST_STACKING\n");
            fb_ev_trigger(fbev, EV_CLIENT_LIST_STACKING);
        } else if (at == a_NET_WORKAREA) {
            DBG("A_NET_WORKAREA\n");
            p->workarea = get_xaproperty (GDK_ROOT_WINDOW(), a_NET_WORKAREA,
                  XA_CARDINAL, &p->wa_len);
            print_wmdata(p);
        } else if (at == a_XROOTPMAP_ID) {
            if (p->transparent) 
                fb_bg_notify_changed_bg(p->bg);           
        } else
            RET(GDK_FILTER_CONTINUE);
        RET(GDK_FILTER_REMOVE);
    }
    DBG("non root %lx\n", win);
    RET(GDK_FILTER_CONTINUE);
}

/****************************************************
 *         panel's handlers for GTK events          *
 ****************************************************/


static gint
panel_destroy_event(GtkWidget * widget, GdkEvent * event, gpointer data)
{
    ENTER;
    gtk_main_quit();
    force_quit = 1;
    RET(FALSE);
}

static void
panel_size_req(GtkWidget *widget, GtkRequisition *req, panel *p)
{
    ENTER;
    DBG("IN req=(%d, %d)\n", req->width, req->height);
    if (p->widthtype == WIDTH_REQUEST)
        p->width = (p->orientation == ORIENT_HORIZ) ? req->width : req->height;
    if (p->heighttype == HEIGHT_REQUEST)
        p->height = (p->orientation == ORIENT_HORIZ) ? req->height : req->width;
    calculate_position(p);
    req->width  = p->aw;
    req->height = p->ah;
    DBG("OUT req=(%d, %d)\n", req->width, req->height);
    RET();
}

static void
make_round_corners(panel *p)
{
    GdkBitmap *b;
    GdkGC* gc;
    GdkColor black = { 0, 0, 0, 0};
    GdkColor white = { 1, 0xffff, 0xffff, 0xffff};
    int w, h, r, br;

    ENTER;
    w = p->aw;
    h = p->ah;
    r = p->round_corners_radius;
    if (2*r > MIN(w, h)) {
        r = MIN(w, h) / 2;
        DBG("chaning radius to %d\n", r);
    }
    b = gdk_pixmap_new(NULL, w, h, 1);
    gc = gdk_gc_new(GDK_DRAWABLE(b));
    gdk_gc_set_foreground(gc, &black);
    gdk_draw_rectangle(GDK_DRAWABLE(b), gc, TRUE, 0, 0, w, h);
    gdk_gc_set_foreground(gc, &white);
    gdk_draw_rectangle(GDK_DRAWABLE(b), gc, TRUE, r, 0, w-2*r, h);
    gdk_draw_rectangle(GDK_DRAWABLE(b), gc, TRUE, 0, r, r, h-2*r);
    gdk_draw_rectangle(GDK_DRAWABLE(b), gc, TRUE, w-r, r, r, h-2*r);

    br = 2 * r ;
    gdk_draw_arc(GDK_DRAWABLE(b), gc, TRUE, 0, 0, br, br, 0*64, 360*64);
    gdk_draw_arc(GDK_DRAWABLE(b), gc, TRUE, 0, h-br-1, br, br, 0*64, 360*64);
    gdk_draw_arc(GDK_DRAWABLE(b), gc, TRUE, w-br, 0, br, br, 0*64, 360*64);
    gdk_draw_arc(GDK_DRAWABLE(b), gc, TRUE, w-br, h-br-1, br, br, 0*64, 360*64);

    gtk_widget_shape_combine_mask(p->topgwin, b, 0, 0);
    g_object_unref(gc);
    g_object_unref(b);

    RET();
}


static  gboolean
panel_configure_event (GtkWidget *widget, GdkEventConfigure *e, panel *p)
{
    int dup,      /* duplicate event */
        fn_pos,   /* final position  */
        fn_size;  /* final size      */

    ENTER;
    DBG("cur geom: %dx%d+%d+%d\n", e->width, e->height, e->x, e->y);
    DBG("req geom: %dx%d+%d+%d\n", p->aw, p->ah, p->ax, p->ay);
    fn_pos  = (e->x == p->ax && e->y == p->ay);
    fn_size = (e->width == p->aw && e->height == p->ah);
    if (!(fn_pos || fn_size)) {
        DBG("not yet there. exiting\n");
        RET(FALSE);
    }

    dup = (e->width == p->cw && e->height == p->ch && e->x == p->cx && e->y == p->cy);
    if (dup) {
        DBG("dup. exiting\n");
        RET(FALSE);
    }

    // panel was just placed to the required postion
    p->cw = e->width;
    p->ch = e->height;
    p->cx = e->x;
    p->cy = e->y;

    if (p->transparent) {
        fb_bg_notify_changed_bg(p->bg);
        DBG("remake bg image\n");
    }
    if (p->setstrut) {
        panel_set_wm_strut(p);
        DBG("set_wm_strut\n");
    }
    if (p->round_corners) {
        make_round_corners(p);
        DBG("make_round_corners\n");
    }
    gtk_widget_show(p->topgwin);
    panel_set_wm_strut(p);

    RET(FALSE);

}

/****************************************************
 *         autohide                                 *
 ****************************************************/

/* Autohide is behaviour whne panel hides itself when mouse is "far enough"
 * and pops up again when mouse comes "close enough". 
 * Formally, it's a state machine with 3 states that driven by mouse 
 * coordinates and timer:
 * 1. VISIBLE - ensures that panel is visible. When/if mouse goes "far enough"
 *      switches to WAITING state
 * 2. WAITING - starts timer. If mouse comes "close enough", stops timer and switches to VISIBLE
 *      If timer expires, switches to HIDDEN
 * 3. HIDDEN - hides panel. When mouse comes "close enough" switches to VISIBLE
 *
 * Note 1
 * Mouse coordinates are queried every PERIOD milisec
 *
 * Note 2
 * If mouse is less then GAP pixels to panel it's considered to be close, otherwise far
 *
 */

#define GAP 30
#define PERIOD 500


static gboolean ah_state_visible(panel *p);
static gboolean ah_state_waiting(panel *p);
static gboolean ah_state_hidden(panel *p);

static gboolean 
mouse_watch(panel *p)
{
    gint x, y;

    ENTER;
    gdk_display_get_pointer(gdk_display_get_default(), NULL, &x, &y, NULL);
    p->ah_far = ((x < p->cx - GAP) || (x > p->cx + p->cw + GAP) || (y < p->cy - GAP) || (y > p->cy + p->ch + GAP));
    p->ah_state(p);
    RET(TRUE);
}

static gboolean
ah_state_visible(panel *p)
{
    ENTER;
    if (p->ah_state != ah_state_visible) {
        p->ah_state = ah_state_visible;
        gtk_widget_show(p->topgwin);
        gtk_window_stick (GTK_WINDOW(p->topgwin));
    } else if (p->ah_far) {
        ah_state_waiting(p);
    }
    RET(FALSE);
}

static gboolean
ah_state_waiting(panel *p)
{
    ENTER;
    if (p->ah_state != ah_state_waiting) {
        p->ah_state = ah_state_waiting;
        hpid = g_timeout_add(2 * PERIOD, (GSourceFunc) ah_state_hidden, p);
    } else if (!p->ah_far) {
        g_source_remove(hpid);
        hpid = 0;
        ah_state_visible(p);
    }
    RET(FALSE);
}

static gboolean
ah_state_hidden(panel *p)
{
    ENTER;
    if (p->ah_state != ah_state_hidden) {
        p->ah_state = ah_state_hidden;
        gtk_widget_hide(p->topgwin);
    } else if (!p->ah_far) {
        ah_state_visible(p);
    }
    RET(FALSE);
}

/* starts autohide behaviour */
void
ah_start(panel *p)
{
    ENTER;
    mwid = g_timeout_add(PERIOD, (GSourceFunc) mouse_watch, p);
    ah_state_visible(p);
    RET();
}

/* stops autohide */
void
ah_stop(panel *p)
{
    ENTER;
    if (mwid) {
        g_source_remove(mwid);
        mwid = 0;
    }
    if (hpid) {
        g_source_remove(hpid);
        hpid = 0;
    }
    RET();
}

/****************************************************
 *         panel creation                           *
 ****************************************************/


gboolean
panel_button_press_event(GtkWidget *widget, GdkEventButton *event, panel *p)
{
    ENTER;
    if (event->type == GDK_BUTTON_PRESS && event->button == 3
          && event->state & GDK_CONTROL_MASK) {
        DBG("ctrl-btn3\n");
        configure();
        RET(TRUE);
    }
    RET(FALSE);
}

void
panel_start_gui(panel *p)
{
    ENTER;

    // main toplevel window
    p->topgwin =  gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(p->topgwin), 0);
    g_signal_connect(G_OBJECT(p->topgwin), "destroy-event",
          G_CALLBACK(panel_destroy_event), p);
    g_signal_connect (G_OBJECT (p->topgwin), "size-request",
          (GCallback) panel_size_req, p);
    g_signal_connect (G_OBJECT (p->topgwin), "configure-event",
          (GCallback) panel_configure_event, p);
    g_signal_connect (G_OBJECT (p->topgwin), "button-press-event",
          (GCallback) panel_button_press_event, p);

    gtk_window_set_resizable(GTK_WINDOW(p->topgwin), FALSE);
    gtk_window_set_wmclass(GTK_WINDOW(p->topgwin), "panel", "fbpanel");
    gtk_window_set_title(GTK_WINDOW(p->topgwin), "panel");
    gtk_window_set_position(GTK_WINDOW(p->topgwin), GTK_WIN_POS_NONE);
    gtk_window_set_decorated(GTK_WINDOW(p->topgwin), FALSE);
    gtk_window_set_accept_focus(GTK_WINDOW(p->topgwin), FALSE);
    if (p->setdocktype)
        gtk_window_set_type_hint(GTK_WINDOW(p->topgwin), GDK_WINDOW_TYPE_HINT_DOCK);

    if (p->layer == LAYER_ABOVE)
        gtk_window_set_keep_above(GTK_WINDOW(p->topgwin), TRUE);
    else if (p->layer == LAYER_BELOW)
        gtk_window_set_keep_below(GTK_WINDOW(p->topgwin), TRUE);
    gtk_window_stick (GTK_WINDOW(p->topgwin));

    gtk_widget_realize(p->topgwin);
    gtk_widget_set_app_paintable(p->topgwin, TRUE);
    calculate_position(p);
    gtk_window_move(GTK_WINDOW(p->topgwin), p->ax, p->ay);
    gtk_window_resize(GTK_WINDOW(p->topgwin), p->aw, p->ah);
    DBG("move-resize x %d y %d w %d h %d\n", p->ax, p->ay, p->aw, p->ah);
    XSync(GDK_DISPLAY(), False);
    //gdk_flush();

    // background box all over toplevel
    p->bbox = gtk_bgbox_new();
    gtk_container_add(GTK_CONTAINER(p->topgwin), p->bbox);
    gtk_container_set_border_width(GTK_CONTAINER(p->bbox), 0);
    if (p->transparent) {
        p->bg = fb_bg_get_for_display();
        gtk_bgbox_set_background(p->bbox, BG_ROOT, p->tintcolor, p->alpha);
    }

    // main layout manager as a single child of background widget box
    p->lbox = p->my_box_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(p->lbox), 0);
    gtk_container_add(GTK_CONTAINER(p->bbox), p->lbox);

    p->box = p->my_box_new(FALSE, p->spacing);
    gtk_container_set_border_width(GTK_CONTAINER(p->box), 0);
    gtk_box_pack_start(GTK_BOX(p->lbox), p->box, TRUE, TRUE,
          (p->round_corners) ? p->round_corners_radius : 0);
    if (p->round_corners) {
        make_round_corners(p);
        DBG("make_round_corners\n");
    }
    /* window mapping point */
    gtk_widget_show_all(p->topgwin);
    gtk_widget_hide(p->topgwin);

    /* the settings that should be done after window is mapped */
    if (p->autohide) 
        ah_start(p);
    if (p->setstrut)
        panel_set_wm_strut(p);
    
    XSelectInput (GDK_DISPLAY(), GDK_ROOT_WINDOW(), PropertyChangeMask);
    gdk_window_add_filter(gdk_get_default_root_window (),
          (GdkFilterFunc)panel_event_filter, p);
    p->topxwin = GDK_WINDOW_XWINDOW(GTK_WIDGET(p->topgwin)->window);
    DBG("topxwin = %lx\n", p->topxwin);
    XSync(GDK_DISPLAY(), False);
    RET();
}

static int
panel_parse_global(panel *p, FILE *fp)
{
    line s;

    ENTER;
    while (get_line(fp, &s) != LINE_NONE) {
        if (s.type == LINE_VAR) {
            if (!g_ascii_strcasecmp(s.t[0], "edge")) {
                p->edge = str2num(edge_pair, s.t[1], EDGE_NONE);
            } else if (!g_ascii_strcasecmp(s.t[0], "allign")) {
                p->allign = str2num(allign_pair, s.t[1], ALLIGN_NONE);
            } else if (!g_ascii_strcasecmp(s.t[0], "margin")) {
                p->margin = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "widthtype")) {
                p->widthtype = str2num(width_pair, s.t[1], WIDTH_NONE);
            } else if (!g_ascii_strcasecmp(s.t[0], "width")) {
                p->width = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "heighttype")) {
                p->heighttype = str2num(height_pair, s.t[1], HEIGHT_NONE);
            } else if (!g_ascii_strcasecmp(s.t[0], "height")) {
                p->height = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "spacing")) {
                p->spacing = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "SetDockType")) {
                p->setdocktype = str2num(bool_pair, s.t[1], 0);
            } else if (!g_ascii_strcasecmp(s.t[0], "SetPartialStrut")) {
                p->setstrut = str2num(bool_pair, s.t[1], 0);
            } else if (!g_ascii_strcasecmp(s.t[0], "RoundCorners")) {
                p->round_corners = str2num(bool_pair, s.t[1], 0);
            } else if (!g_ascii_strcasecmp(s.t[0], "RoundCornersRadius")) {
                p->round_corners_radius = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "autohide")) {
                p->autohide = str2num(bool_pair, s.t[1], 0);
            } else if (!g_ascii_strcasecmp(s.t[0], "heightWhenHidden")) {
                p->height_when_hidden = atoi(s.t[1]);
            } else if (!g_ascii_strcasecmp(s.t[0], "Transparent")) {
                p->transparent = str2num(bool_pair, s.t[1], 0);
            } else if (!g_ascii_strcasecmp(s.t[0], "Alpha")) {
                p->alpha = atoi(s.t[1]);
                if (p->alpha > 255)
                    p->alpha = 255;
            } else if (!g_ascii_strcasecmp(s.t[0], "TintColor")) {
                if (!gdk_color_parse (s.t[1], &p->gtintcolor))
                    gdk_color_parse ("white", &p->gtintcolor);
                p->tintcolor = gcolor2rgb24(&p->gtintcolor);
                DBG("tintcolor=%x\n", p->tintcolor);
            } else if (!g_ascii_strcasecmp(s.t[0], "Layer")) {
                p->layer = str2num(layer_pair, s.t[1], 0);
            } else {
                ERR( "fbpanel: %s - unknown var in Global section\n", s.t[0]);
                RET(0);
            }
        } else if (s.type == LINE_BLOCK_END) {
            break;
        } else {
            ERR( "fbpanel: illegal in this context %s\n", s.str);
            RET(0);
        }
    }
    p->orientation = (p->edge == EDGE_TOP || p->edge == EDGE_BOTTOM)
        ? ORIENT_HORIZ : ORIENT_VERT;
    if (p->orientation == ORIENT_HORIZ) {
        p->my_box_new = gtk_hbox_new;
        p->my_separator_new = gtk_vseparator_new;
    } else {
        p->my_box_new = gtk_vbox_new;
        p->my_separator_new = gtk_hseparator_new;
    }
    if (p->width < 0)
        p->width = 100;
    if (p->widthtype == WIDTH_PERCENT && p->width > 100)
        p->width = 100;
    p->heighttype = HEIGHT_PIXEL;
    if (p->heighttype == HEIGHT_PIXEL) {
        if (p->height < PANEL_HEIGHT_MIN)
            p->height = PANEL_HEIGHT_MIN;
        else if (p->height > PANEL_HEIGHT_MAX)
            p->height = PANEL_HEIGHT_MAX;
    }
    p->curdesk = get_net_current_desktop();
    p->desknum = get_net_number_of_desktops();
    p->workarea = get_xaproperty (GDK_ROOT_WINDOW(), a_NET_WORKAREA,
          XA_CARDINAL, &p->wa_len);
    print_wmdata(p);
    RET(1);
}

static int
panel_parse_plugin(panel *p, FILE *fp)
{
    line s;
    plugin_instance *plug = NULL;
    gchar *type = NULL;
    FILE *tmpfp;
    int expand , padding, border, pno = 0;

    ENTER;
    if (!(tmpfp = tmpfile())) {
        ERR( "can't open temporary file with tmpfile()\n");
        RET(0);
    }
    border = expand = padding = 0;
    while (get_line(fp, &s) != LINE_BLOCK_END) {
        if (s.type == LINE_NONE) {
            ERR( "fbpanel: bad line %s\n", s.str);
            goto error;
        }
        if (s.type == LINE_VAR) {
            if (!g_ascii_strcasecmp(s.t[0], "type")) {
                type = g_strdup(s.t[1]);
                DBG("plug %s\n", type);
            } else if (!g_ascii_strcasecmp(s.t[0], "expand"))
                expand = str2num(bool_pair,  s.t[1], 0);
            else if (!g_ascii_strcasecmp(s.t[0], "padding"))
                padding = atoi(s.t[1]);
            else if (!g_ascii_strcasecmp(s.t[0], "border"))
                border = atoi(s.t[1]);
            else {
                ERR( "fbpanel: unknown var %s\n", s.t[0]);
                goto error;
            }
        } else if (s.type == LINE_BLOCK_START) {
            if (!g_ascii_strcasecmp(s.t[0], "Config")) {
                pno = 2;
                while (pno > 1) {
                    get_line_as_is(fp, &s);
                    if (s.type == LINE_NONE) {
                        ERR( "fbpanel: unexpected eof\n");
                        goto error;
                    } else if (s.type != LINE_BLOCK_END) {
                        fprintf(tmpfp, "%s%s\n", indent(pno), s.str);
                        //fprintf(stdout, "%s%s\n", indent(pno), s.str);
                        if (s.type == LINE_BLOCK_START)
                            pno++;
                    } else {
                        pno--;
                        fprintf(tmpfp, "%s%s\n", indent(pno), s.str);
                        //fprintf(stdout, "%s%s\n", indent(pno), s.str);
                    }
                }
            } else {
                ERR( "fbpanel: unknown block %s\n", s.t[0]);
                goto error;
            }
        } else {
            ERR( "fbpanel: illegal in this context %s\n", s.str);
            goto error;
        }
    }
    if (!pno) {
        //there is no config section, lets pretend one
        fprintf(tmpfp, "%s}\n", indent(1));
        //fprintf(stdout, "%s}\n", indent(1), s.str);
    }
    if (!type || !(plug = plugin_load(type))) {
        ERR( "fbpanel: can't load %s plugin\n", type);
        goto error;
    }
    plug->panel = p;
    plug->fp = tmpfp;
    plug->expand = expand;
    plug->padding = padding;
    plug->border = border;
    //fprintf(tmpfp, "}\n");
    fseek(tmpfp, 0, SEEK_SET);
    DBG("starting\n");
    if (!plugin_start(plug)) {
        ERR( "fbpanel: can't start plugin %s\n", type);
        goto error;
    }
    DBG("plug %s\n", type);
    p->plugins = g_list_append(p->plugins, plug);
    g_free(type);
    RET(1);

error:
    fclose(tmpfp);
    g_free(type);
    if (plug)
        plugin_put(plug);
    RET(0);

}


static gboolean
panel_parse_plugins(panel *p)
{
    line s;

    ENTER;
    fseek(pconf, 0, SEEK_SET);
    while (get_line(pconf, &s) != LINE_NONE) {
        if ((s.type  != LINE_BLOCK_START) || g_ascii_strcasecmp(s.t[0], "plugin")) {
            ERR( "fbpanel: expecting plugin section\n");
            goto error;
        }
        if (!panel_parse_plugin(p, pconf)) {
            ERR( "fbpanel: can;t parse plugin\n");
            goto error;
        }
    }
    RET(FALSE);
error:
    exit(1);
}

static int
panel_start(panel *p, FILE *fp)
{
    line s;
    long pos;

    /* parse global section */
    ENTER;
    memset(p, 0, sizeof(panel));
    p->allign = ALLIGN_CENTER;
    p->edge = EDGE_BOTTOM;
    p->widthtype = WIDTH_PERCENT;
    p->width = 100;
    p->heighttype = HEIGHT_PIXEL;
    p->height = PANEL_HEIGHT_DEFAULT;
    p->setdocktype = 1;
    p->setstrut = 1;
    p->round_corners = 1;
    p->round_corners_radius = 7;
    p->autohide = 0;
    p->height_when_hidden = 2;
    p->transparent = 0;
    p->alpha = 127;
    p->tintcolor = 0xFFFFFFFF;
    p->spacing = 0;
    p->layer = LAYER_NONE;
    fbev = fb_ev_new();
    if ((get_line(fp, &s) != LINE_BLOCK_START) || g_ascii_strcasecmp(s.t[0],
                "Global")) {
        ERR( "fbpanel: config file must start from Global section\n");
        RET(0);
    }
    if (!panel_parse_global(p, fp))
        RET(0);
    panel_start_gui(p);

    if (!(pconf = tmpfile())) {
        ERR("can't open temporary file\n");
        RET(0);
    }
    pos = ftell(fp);
    while (get_line_as_is(fp, &s) != LINE_NONE) {
        fprintf(pconf, "%s\n", s.str);
        //fprintf(stdout, "%s\n", s.str);
    }
    fseek(fp, pos, SEEK_SET);
    fflush(pconf);
    panel_parse_plugins(p);

    //gtk_widget_show_all(p->topgwin);
    //gdk_flush();
    RET(1);
}

static void
delete_plugin(gpointer data, gpointer udata)
{
    ENTER;
    plugin_stop((plugin_instance *)data);
    plugin_put((plugin_instance *)data);
    RET();

}

static void
panel_stop(panel *p)
{
    ENTER;

    if (p->autohide) 
        ah_stop(p);
    g_list_foreach(p->plugins, delete_plugin, NULL);
    g_list_free(p->plugins);
    p->plugins = NULL;

    XSelectInput (GDK_DISPLAY(), GDK_ROOT_WINDOW(), NoEventMask);
    gdk_window_remove_filter(gdk_get_default_root_window (),
          (GdkFilterFunc)panel_event_filter, p);
    gtk_widget_destroy(p->topgwin);
    g_object_unref(fbev);
    g_free(p->workarea);
    fclose(pconf);
    gdk_flush();
    XFlush(GDK_DISPLAY());
    XSync(GDK_DISPLAY(), True);
    RET();
}


void
usage()
{
    ENTER;
    printf("fbpanel %s - lightweight GTK2+ panel for UNIX desktops\n", version);
    printf("Command line options:\n");
    printf(" --help      -- print this help and exit\n");
    printf(" --version   -- print version and exit\n");
    printf(" --log <number> -- set log level 0-5. 0 - none 5 - chatty\n");
    printf(" --configure -- launch configuration utility\n");
    printf(" --profile name -- use specified profile\n");
    printf("\n");
    printf(" -h  -- same as --help\n");
    printf(" -p  -- same as --profile\n");
    printf(" -v  -- same as --version\n");
    printf(" -C  -- same as --configure\n");
    printf("\nVisit http://fbpanel.sourceforge.net/ for detailed documentation,\n\n");
}

FILE *
open_profile(gchar *profile)
{
    gchar *fname;
    FILE *fp;

    ENTER;
    LOG(LOG_INFO, "loading %s profile\n", profile);
    fname = g_strdup_printf("%s/.fbpanel/%s", getenv("HOME"), profile);
    fp = fopen(fname, "r");
    LOG(LOG_INFO, "   %s %s\n", fname, fp ? "ok" : "no");
    if (fp) {
        cfgfile = fname;
        RET(fp);
    }
    //ERR("Can't load %s\n", fname);
    g_free(fname);

    /* check private configuration directory */
    fname = g_strdup_printf("%s/share/fbpanel/%s", PREFIX, profile);
    fp = fopen(fname, "r");
    LOG(LOG_INFO, "   %s %s\n", fname, fp ? "ok" : "no");
    if (fp) {
        cfgfile = fname;
        RET(fp);
    }
    //ERR("Can't load %s\n", fname);
    g_free(fname);
    LOG(LOG_ERR, "Can't open '%s' profile\n", profile);
    RET(NULL);
}

void 
close_profile(FILE *file)
{
    ENTER;
    fclose(file);
    if (cfgfile) {
        g_free(cfgfile);
        cfgfile = NULL;
    }
    RET();
}

void
handle_error(Display * d, XErrorEvent * ev)
{
    char buf[256];

    ENTER;
    XGetErrorText(GDK_DISPLAY(), ev->error_code, buf, 256);
    DBG("fbpanel : X error: %s\n", buf);

    RET();
}

static void
sig_usr1(int signum)
{
    if (signum != SIGUSR1)
        return;
    gtk_main_quit();
}

static void
sig_usr2(int signum)
{
    if (signum != SIGUSR2)
        return;
    gtk_main_quit();
    force_quit = 1;
}

#ifdef TEST    
static gboolean
panel_exit(void *p)
{
    static int count = 0;

    count++;
    DBG2("count=%d\n", count);
    gtk_main_quit();
    if (count == 1) {
        g_timeout_add(20 * 1000, (GSourceFunc) panel_exit, p);
        RET(FALSE);
    }
    if (count > 2) {
        force_quit = 1;    
        DBG2("force_quit=%d\n", force_quit);
        RET(FALSE);
    }
    RET(TRUE);
}
#endif

int
main(int argc, char *argv[], char *env[])
{
    int i;
    FILE *pfp; /* current profile FP */

    ENTER;
    setlocale(LC_CTYPE, "");
    gtk_set_locale();
    gtk_init(&argc, &argv);
    XSetLocaleModifiers("");
    XSetErrorHandler((XErrorHandler) handle_error);
    fb_init();
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage();
            exit(0);
        } else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
            printf("fbpanel %s\n", version);
            exit(0);
        } else if (!strcmp(argv[i], "--log")) {
            i++;
            if (i == argc) {
                ERR( "fbpanel: missing log level\n");
                usage();
                exit(1);
            } else {
                log_level = atoi(argv[i]);
            }
        } else if (!strcmp(argv[i], "--configure") || !strcmp(argv[i], "-C")) {
            config = 1;
        } else if (!strcmp(argv[i], "--profile") || !strcmp(argv[i], "-p")) {
            i++;
            if (i == argc) {
                ERR( "fbpanel: missing profile name\n");
                usage();
                exit(1);
            } else {
                cprofile = g_strdup(argv[i]);
            }
        } else {
            printf("fbpanel: unknown option - %s\n", argv[i]);
            usage();
            exit(1);
        }
    }

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), IMGPREFIX);
    signal(SIGUSR1, sig_usr1);
    signal(SIGUSR2, sig_usr2);
#ifdef TEST    
    g_timeout_add(25 * 1000, (GSourceFunc) panel_exit, p);
#endif
    do {
        if (!(pfp = open_profile(cprofile)))
            exit(1);
        the_panel = p = g_new0(panel, 1);
        g_return_val_if_fail (p != NULL, 1);
        if (!panel_start(p, pfp)) {
            ERR( "fbpanel: can't start panel\n");
            exit(1);
        }
        if (config)
            configure();
        gtk_main();
        panel_stop(p);
        close_profile(pfp);
        g_free(p);
        DBG("force_quit=%d\n", force_quit);
    } while (force_quit == 0);

    exit(0);
}

