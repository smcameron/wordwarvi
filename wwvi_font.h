#ifndef WWVI_FONT_H
#define WWVI_FONT_H
/* 
    (C) Copyright 2007,2008, Stephen M. Cameron.

    This file is part of wordwarvi.

    wordwarvi is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    wordwarvi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with wordwarvi; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 */

#include "my_point.h"

#ifdef WWVI_FONT_DEFINE_GLOBALS
#define GLOBAL
#else 
#define GLOBAL extern
#endif

GLOBAL int wwvi_make_font(struct my_vect_obj ***font, int xscale, int yscale);

#endif
