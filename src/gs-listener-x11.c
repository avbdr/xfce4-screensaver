/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2004-2006 William Jon McCann <mccann@jhu.edu>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors: William Jon McCann <mccann@jhu.edu>
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtk/gtkx.h>

#include <X11/extensions/scrnsaver.h>

#include "gs-listener-x11.h"
#include "gs-marshal.h"
#include "gs-debug.h"

static void         gs_listener_x11_class_init      (GSListenerX11Class *klass);
static void         gs_listener_x11_init            (GSListenerX11      *listener);
static void         gs_listener_x11_finalize        (GObject            *object);

struct GSListenerX11Private {
    int scrnsaver_event_base;
    gint     timeout;
    gboolean idle_activation_enabled;
    gboolean saver_enabled;
};

enum {
    ACTIVATE,
    LOCK,
    LAST_SIGNAL
};

static guint         signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE (GSListenerX11, gs_listener_x11, G_TYPE_OBJECT)

static void
gs_listener_x11_class_init (GSListenerX11Class *klass) {
    GObjectClass   *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize       = gs_listener_x11_finalize;

    signals[ACTIVATE] =
        g_signal_new ("activate",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GSListenerX11Class, activate),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);

    signals[LOCK] =
        g_signal_new ("lock",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GSListenerX11Class, lock),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE,
                      0);
}

static GdkFilterReturn
xroot_filter (GdkXEvent *xevent,
              GdkEvent  *event,
              gpointer   data) {
    GSListenerX11 *listener;
    XEvent *ev;

    g_return_val_if_fail (data != NULL, GDK_FILTER_CONTINUE);
    g_return_val_if_fail (GS_IS_LISTENER_X11 (data), GDK_FILTER_CONTINUE);

    listener = GS_LISTENER_X11 (data);

    if (!listener->priv->idle_activation_enabled)
        return GDK_FILTER_CONTINUE;

    ev = xevent;

    switch (ev->xany.type) {
    default:
        /* extension events */
        if (ev->xany.type == (listener->priv->scrnsaver_event_base + ScreenSaverNotify)) {
            XScreenSaverNotifyEvent *xssne = (XScreenSaverNotifyEvent *) ev;
            switch (xssne->state) {
                case ScreenSaverOn:
                    gs_debug("Activating screensaver on ScreenSaverOn");
                    g_signal_emit (listener, signals[ACTIVATE], 0);
                    break;
            }
        }
        break;
    }

    return GDK_FILTER_CONTINUE;
}


gboolean
gs_listener_x11_acquire (GSListenerX11 *listener) {
    GdkDisplay *display;
    GdkScreen *screen;
    GdkWindow *window;
    int scrnsaver_error_base;
    unsigned long events;

    g_return_val_if_fail (listener != NULL, FALSE);

    display = gdk_display_get_default ();
    screen = gdk_display_get_default_screen (display);
    window = gdk_screen_get_root_window (screen);

    gdk_x11_display_error_trap_push (display);
    if (XScreenSaverQueryExtension (GDK_DISPLAY_XDISPLAY (display),
                                    &listener->priv->scrnsaver_event_base,
                                    &scrnsaver_error_base)) {
        events = ScreenSaverNotifyMask;
        XScreenSaverSelectInput (GDK_DISPLAY_XDISPLAY (display), GDK_WINDOW_XID (window), events);
        gs_debug ("ScreenSaver Registered");
    } else {
        gs_debug ("ScreenSaverExtension not found");
    }

    gdk_x11_display_error_trap_pop_ignored (display);

    gdk_window_add_filter (NULL, (GdkFilterFunc)xroot_filter, listener);

    return TRUE;
}

void
gs_listener_x11_set_timeout (GSListenerX11 *listener,
                             gint           timeout) {
    Display *display = gdk_x11_display_get_xdisplay(gdk_display_get_default());

    /* set X server timeouts and disable screen blanking */
    XSetScreenSaver(display, timeout * 60, timeout * 60, 0,0);
    listener->priv->timeout = timeout * 60;
    gs_debug("Screensaver timeout updated to %i minutes", timeout);
}

void
gs_listener_x11_set_saver_enabled (GSListenerX11 *listener,
                                   gboolean       enabled) {
    listener->priv->saver_enabled = enabled;
}

void
gs_listener_x11_set_activation_enabled (GSListenerX11 *listener,
                                        gboolean       enabled) {
    listener->priv->idle_activation_enabled = enabled;
}

static void
gs_listener_x11_init (GSListenerX11 *listener) {
    listener->priv = gs_listener_x11_get_instance_private (listener);
}

static void
gs_listener_x11_finalize (GObject *object) {
    GSListenerX11 *listener;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GS_IS_LISTENER_X11 (object));

    listener = GS_LISTENER_X11 (object);

    g_return_if_fail (listener->priv != NULL);

    gdk_window_remove_filter (NULL, (GdkFilterFunc)xroot_filter, NULL);

    G_OBJECT_CLASS (gs_listener_x11_parent_class)->finalize (object);
}

GSListenerX11 *
gs_listener_x11_new (void) {
    GSListenerX11 *listener;

    listener = g_object_new (GS_TYPE_LISTENER_X11, NULL);

    return GS_LISTENER_X11 (listener);
}
