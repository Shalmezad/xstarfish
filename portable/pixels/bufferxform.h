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


Routines to transform entire buffers, of the grey or coloured variety.

*/

#ifndef __starfish_bufferxform__
#define __starfish_bufferxform__ 0

#include "rasterliberrs.h"
#include "pixmap.h"
#include "greymap.h"

//Copy the contents of one pixbuf into another.
srl_result CopyPixBuf(pixbuf src, pixbuf dest);
//Copy one greybuf into another greybuf.
srl_result CopyGreyBuf(greybuf src, greybuf dest);
//Copy a greybuf into all channels of a pixbuf.
srl_result ExpandGreyIntoPixels(greybuf src, pixbuf dest);
//Copy a greybuf into the pixbuf's alpha channel, leaving other data untouched.
srl_result CopyGreyIntoAlpha(greybuf src, pixbuf dest);
//Fill all *but* the alpha channel with the appropriate grey values.
srl_result CopyGreyIntoRGB(greybuf src, pixbuf dest);
//Expand the grey values into RGB using a colour gradient.
srl_result CopyGreyIntoGradient(greybuf src, pixbuf dest, const pixel* low, const pixel* high);
//Convert the alpha channel of a colour image into a greybuf.
srl_result CopyAlphaIntoGreyBuf(pixbuf src, greybuf dest);
//Average the RGB channels of a colour image into a greybuf
srl_result CopyRGBIntoGreyBuf(pixbuf src, greybuf dest);
//Merge two pixbufs into a third, merging alpha channels en route.
//The dest can be one of the src pixbufs.
srl_result MergePixBufs(pixbuf top, pixbuf bottom, pixbuf dest);
//Swaps a pixbuf both ways so the corners are in the centre.
srl_result SwapPixBufCorners(pixbuf it);
//Performs the same operation to a greybuf.
srl_result SwapGreyBufCorners(greybuf it);
//Inverts the contents of a greybuf.
srl_result InvertGreyBuf(greybuf it);

#endif