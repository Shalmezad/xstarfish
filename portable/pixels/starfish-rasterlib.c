/*

Copyright ©1999 Mars Saxman
All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


A set of routines used to manage a rudimentary pixmap library.
Pixmaps are collections of raster lines, which are arrays of
pixels. This lib creates such pixmaps and lets you access them
row-at-a-time.

Right now, the Starfish rasterlib assumes 32-bit pixels and fixed
size buffers. It doesn't do all that much, really...

*/

#include <stdlib.h>
#include <string.h>
#include "starfish-rasterlib.h"

//Hmm. It doesn't appear that the rasterlib itself actually contains any code...

