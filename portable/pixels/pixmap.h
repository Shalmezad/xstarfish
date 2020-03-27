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
pixels. This lib creates such pixmaps and lets you access them.
Normal pixmaps have 32-bit pixels: one 8-bit channel each for
red, green, blue, and alpha. 

*/

#ifndef __starfish_pixmaplib__
#define __starfish_pixmaplib__ 0

#include <stdlib.h>
#include "rasterliberrs.h"

typedef struct pixbufrec* pixbuf;
typedef struct pixel
	{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
	}
pixel;
typedef pixel* rasterline;

//Colour Pixel Buffers

//Create a new pixbuf with a certain number of columns and rows.
pixbuf MakePixBuf(int horz, int vert);
//Dispose of an existing pixbuf.
srl_result DumpPixBuf(pixbuf it);
//Fill this buffer with the specified pixel.
srl_result FillPixBuf(pixbuf it, const pixel* src);
//Fill all channels in this pixbuf with this value.
srl_result GreyFillPixBuf(pixbuf it, unsigned char src);

//The following functions return zero if the pixbuf is invalid.
//How many pixels wide is one scan line?
int GetPixBufWidth(pixbuf it);
//And how many rows tall is the buffer?
int GetPixBufHeight(pixbuf it);
//How many bytes long is one raster line?
size_t GetPixBufLineSize(pixbuf it);

//Get one pixel from the buffer.
srl_result GetPixBufPixel(pixbuf it, int horz, int vert, pixel* dest);
//Set one pixel into the buffer.
srl_result SetPixBufPixel(pixbuf it, int horz, int vert, const pixel* dest);
//Get part of one rasterline from the buffer. It is your responsibility to make sure
//your buffer is big enough to hold the entire line. Use GetPixBufLineSize to check.
srl_result GetRasterLine(pixbuf it, int start, int count, int vert, rasterline dest);
//Put this rasterline into the buffer. Again, your responsibility to make sure your
//source buffer is big enough.
srl_result SetRasterLine(pixbuf it, int start, int count, int vert, const rasterline dest);
//If you want real speed, you have to go direct. Use with caution.
//Returns NULL if you ask for a rasterline that doesn't exist.
rasterline PeekRasterLine(pixbuf it, int vert);


#endif //__starfish_rasterlib__