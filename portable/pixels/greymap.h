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


Code to manage Greymaps - pixmaps with only one 8-bit data channel

*/

#ifndef __starfish_greymap__
#define __starfish_greymap__ 0

#include <stdlib.h>
#include "rasterliberrs.h"

typedef struct greybufrec* greybuf;
typedef unsigned char channelval;
#define MIN_CHANVAL 0
#define MAX_CHANVAL 255
#define CHANNEL_RANGE 256
typedef channelval* channelline;

//Create a new greybuf with the specified number of columns and rows.
greybuf MakeGreyBuf(int horz, int vert);
//Dispose of an already-existing greybuf.
srl_result DumpGreyBuf(greybuf it);
//Fill the buffer with this value.
srl_result FillGreyBuf(greybuf it, channelval src);

//The next three functions return zero if the greybuf is bogus.
//How many pixels wide is the buffer?
int GetGreyBufWidth(greybuf it);
//How many rows tall is the buffer?
int GetGreyBufHeight(greybuf it);
//How many bytes does one raster line occupy?
size_t GetGreyBufLineSize(greybuf it);

//Retrieve one pixel from the buffer.
srl_result GetGreyBufPixel(greybuf it, int horz, int vert, channelval* dest);
//Place one pixel into the buffer.
srl_result SetGreyBufPixel(greybuf it, int horz, int vert, channelval src);
//Get part of one rasterline from the buffer. It is your responsibility to make sure
//your buffer is big enough to hold the entire line. Use GetPixBufLineSize to check.
srl_result GetGreyRasterLine(greybuf it, int start, int count, int vert, channelline dest);
//Put this rasterline into the buffer. Again, your responsibility to make sure your
//source buffer is big enough.
srl_result SetGreyRasterLine(greybuf it, int start, int count, int vert, const channelline dest);
//Returns the address of the specified raster line, if available.
channelline PeekGreyRasterLine(greybuf it, int vert);

#endif //__starfish_greymap__