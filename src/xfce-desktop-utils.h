/* -*- Mode: C; c-set-style: linux indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* mate-ditem.h - Utilities for the MATE Desktop

   Copyright (C) 1998 Tom Tromey
   All rights reserved.

   This file is part of the Mate Library.

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA  02110-1301, USA.  */
/*
  @NOTATION@
 */

#ifndef XFCE_DESKTOP_UTILS_H
#define XFCE_DESKTOP_UTILS_H

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* prepend the terminal command to a vector */
void xfce_desktop_prepend_terminal_to_vector (int *argc, char ***argv);

/* replace gdk_spawn_command_line_on_screen, not available in GTK3 */
gboolean xfce_gdk_spawn_command_line_on_screen (GdkScreen *screen, const gchar *command, GError **error);

void
xfce_desktop_gtk_style_get_light_color (GtkStyleContext *style,
                                        GtkStateFlags    state,
                                        GdkRGBA         *color);

void
xfce_desktop_gtk_style_get_dark_color (GtkStyleContext *style,
                                       GtkStateFlags    state,
                                       GdkRGBA         *color);

G_END_DECLS

#endif /* XFCE_DESKTOP_UTILS_H */