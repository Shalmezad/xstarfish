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

#include "bufferxform.h"
#include "rasterliberrs.h"
#include "pixmap.h"
#include "greymap.h"

static srl_result CopyGreyIntoPixBuf(greybuf src, pixbuf dest, int RGB, int alpha);

srl_result CopyGreyIntoPixBuf(greybuf src, pixbuf dest, int RGB, int alpha)
	{
	/*
	Copy a greybuf into selected channels of a pixbuf.
	We do this the quick way: by running through peeked raster lines
	in both the pixbuf and greybuf. This is a really dumb-simple
	function.
	*/
	srl_result err = srl_noErr;
	if(src && dest)
		{
		//Make sure these two buffers are exactly the same size.
		if	(
				(GetGreyBufWidth(src) == GetPixBufWidth(dest)) && 
				(GetGreyBufHeight(src) == GetPixBufHeight(dest))
				)
			{
			int vctr, vmax;
			int hctr, hmax;
			vmax = GetPixBufHeight(dest);
			hmax = GetPixBufWidth(dest);
			for(vctr = 0; vctr < vmax; vctr++)
				{
				channelline srcline;
				rasterline destline;
				srcline = PeekGreyRasterLine(src, vctr);
				destline = PeekRasterLine(dest, vctr);
				if(srcline && destline)
					{
					pixel* temppixel;
					channelval tempval;
					/*
					For each pixel in this row, read off the channel value from the greybuf.
					Then store this value into all four channels in the destination pixel.
					*/
					for(hctr = 0; hctr < hmax; hctr++)
						{
						temppixel = &destline[hctr];
						tempval = srcline[hctr];
						if(RGB) temppixel->red = temppixel->green = temppixel->blue = tempval;
						if(alpha) temppixel->alpha = tempval;
						}
					}
				else err = srl_bollixed;
				}
			}
		else err = srl_mismatchedSizes;
		}
	else err = srl_bogusBuffer;
	return err;
	}


//Copy the contents of one pixbuf into another.
srl_result CopyPixBuf(pixbuf src, pixbuf dest)
	{
	}

//Copy one greybuf into another greybuf.
srl_result CopyGreyBuf(greybuf src, greybuf dest)
	{
	}

srl_result ExpandGreyIntoPixels(greybuf src, pixbuf dest)
	{
	//Copy a greybuf into both colour and alpha channels on the pixbuf
	return CopyGreyIntoPixBuf(src, dest, 1, 1);
	}

srl_result CopyGreyIntoAlpha(greybuf src, pixbuf dest)
	{
	//Copy a greybuf into the pixbuf's alpha channel, leaving other data untouched.
	return CopyGreyIntoPixBuf(src, dest, 0, 1);
	}

srl_result CopyGreyIntoRGB(greybuf src, pixbuf dest)
	{
	//Fill all *but* the alpha channel with the appropriate grey values.
	return CopyGreyIntoPixBuf(src, dest, 1, 0);
	}

srl_result CopyGreyIntoGradient(greybuf src, pixbuf dest, const pixel* low, const pixel* high)
	{
	/*
	Expand the grey values into RGB using a colour gradient.
	This works a lot like CopyGreyIntoPixBuf does: it runs through rasterlines,
	copying grey channel values into colour. The trick, though, is that instead
	of running from black to white values, we run from the "low" to "high" colours.
	
	*/
	srl_result err = srl_noErr;
	if(src && dest)
		{
		//Make sure these two buffers are exactly the same size.
		if	(
				(GetGreyBufWidth(src) == GetPixBufWidth(dest)) && 
				(GetGreyBufHeight(src) == GetPixBufHeight(dest))
				)
			{
			int vctr, vmax;
			int hctr, hmax;
			vmax = GetPixBufHeight(dest);
			hmax = GetPixBufWidth(dest);
			for(vctr = 0; vctr < vmax; vctr++)
				{
				channelline srcline;
				rasterline destline;
				srcline = PeekGreyRasterLine(src, vctr);
				destline = PeekRasterLine(dest, vctr);
				if(srcline && destline)
					{
					pixel* temppixel;
					channelval tempval;
					/*
					For each pixel in this row, read off the channel value from the greybuf.
					Then store this value into all four channels in the destination pixel.
					*/
					for(hctr = 0; hctr < hmax; hctr++)
						{
						float interval;
						temppixel = &destline[hctr];
						tempval = srcline[hctr];
						//Calculate the red channel of the pixel.
						interval = tempval;
						interval /= CHANNEL_RANGE;
						interval *= high->red - low->red;
						temppixel->red = interval + low->red;
						//Next calculate the green channel.
						interval = tempval;
						interval /= CHANNEL_RANGE;
						interval *= high->green - low->green;
						temppixel->green = interval + low->green;
						//Calculate the blue channel in the same fashion
						interval = tempval;
						interval /= CHANNEL_RANGE;
						interval *= high->blue - low->blue;
						temppixel->blue = interval + low->blue;
						//We ignore the alpha channel.
						}
					}
				else err = srl_bollixed;
				}
			}
		else err = srl_mismatchedSizes;
		}
	else err = srl_bogusBuffer;
	return err;

	}

//Convert the alpha channel of a colour image into a greybuf.
srl_result CopyAlphaIntoGreyBuf(pixbuf src, greybuf dest)
	{
	}

//Average the RGB channels of a colour image into a greybuf
srl_result CopyRGBIntoGreyBuf(pixbuf src, greybuf dest)
	{
	}

srl_result MergePixBufs(pixbuf top, pixbuf bottom, pixbuf dest)
	{
	/*
	Merge two pixbufs into a third, merging alpha channels en route.
	This is like laying one slide on top of another, sort of.
	*/
	srl_result err = srl_noErr;
	if(top && bottom && dest)
		{
		//Make sure these two buffers are exactly the same size.
		if	(
				(GetPixBufWidth(top) == GetPixBufWidth(bottom)) && 
				(GetPixBufHeight(top) == GetPixBufHeight(bottom)) &&
				(GetPixBufWidth(bottom) == GetPixBufWidth(dest)) &&
				(GetPixBufHeight(bottom) == GetPixBufHeight(dest))
				)
			{
			int vctr, vmax;
			int hctr, hmax;
			vmax = GetPixBufHeight(dest);
			hmax = GetPixBufWidth(dest);
			for(vctr = 0; vctr < vmax; vctr++)
				{
				rasterline topline;
				rasterline botline;
				rasterline destline;
				topline = PeekRasterLine(top, vctr);
				botline = PeekRasterLine(bottom, vctr);
				destline = PeekRasterLine(dest, vctr);
				if(topline && botline && destline)
					{
					pixel* toppixel;
					pixel* botpixel;
					pixel* destpixel;
					/*
					For each pixel in this row, read off the channel value from the greybuf.
					Then store this value into all four channels in the destination pixel.
					*/
					for(hctr = 0; hctr < hmax; hctr++)
						{
						destpixel = &destline[hctr];
						toppixel = &topline[hctr];
						botpixel = &botline[hctr];
						
						//Calculate the red channel of the pixel.
						destpixel->red =
								(
								(toppixel->red * toppixel->alpha) + 
								(botpixel->red * (CHANNEL_RANGE - toppixel->alpha))
								) / (CHANNEL_RANGE);
						//Next calculate the green channel.
						destpixel->green =
								(
								(toppixel->green * toppixel->alpha) + 
								(botpixel->green * (CHANNEL_RANGE - toppixel->alpha))
								) / (CHANNEL_RANGE);
						//Calculate the blue channel in the same fashion
						destpixel->blue =
								(
								(toppixel->blue * toppixel->alpha) + 
								(botpixel->blue * (CHANNEL_RANGE - toppixel->alpha))
								) / (CHANNEL_RANGE);
						//The alpha channel is the sum of these alpha channels. 255 is max opaque.
						if(toppixel->alpha + botpixel->alpha > CHANNEL_RANGE) destpixel->alpha = CHANNEL_RANGE;
							else destpixel->alpha = toppixel->alpha + botpixel->alpha;
						}
					}
				else err = srl_bollixed;
				}
			}
		else err = srl_mismatchedSizes;
		}
	else err = srl_bogusBuffer;
	return err;

	}

//Swaps a pixbuf both ways so the corners are in the centre.
srl_result SwapPixBufCorners(pixbuf it)
	{
	/*
	Swap a pixbuf so that its corners are in its centre.
	Someday it might be nice to add an "offset" factor so the results
	don't land exactly in the middle, but are sort of crookedly wrapped.
	The purpose of this transformation is to fudge the wrapped edges of images
	around so that they don't become obvious when many layers are stacked
	up on top of each other, as tends to happen with Starfish.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		/*
		Iterate through all the rows of this pixbuf.
		Grab each row, one at a time, and exchange it with its neighbor
		on the opposite side of the pixbuf.
		*/
		rasterline tempstorage;
		rasterline peekline;
		pixel peekpixel;
		int vctr, vmax, vhalf;
		int hctr, hmax, hhalf;
		//Create a temporary storage line to hold the rasterline-in-transit.
		tempstorage = (rasterline)malloc(GetPixBufLineSize(it));
		if(tempstorage)
			{
			vmax = GetPixBufHeight(it);
			vhalf = vmax / 2;
			hmax = GetPixBufWidth(it);
			hhalf = hmax / 2;
			for(vctr = 0; vctr < vhalf; vctr++)
				{
				//Copy the top line into our temporary buffer.
				err = GetRasterLine(it, 0, hmax, vctr, tempstorage);
				//Swap the contents of the both lines horizontally.
				//We kind of have a smashed loop here.
				peekline = PeekRasterLine(it, vctr + vhalf);
				for(hctr = 0; hctr < hhalf; hctr++)
					{
					peekpixel = tempstorage[hctr];
					tempstorage[hctr] = tempstorage[hctr + hhalf];
					tempstorage[hctr+hhalf] = peekpixel;
					peekpixel = peekline[hctr];
					peekline[hctr] = peekline[hctr + hhalf];
					peekline[hctr+hhalf] = peekpixel;
					}
				//Copy the bottom line into the top line.
				err = SetRasterLine(it, 0, hmax, vctr, peekline);
				//Now put our temporary buffer into the bottom line.
				err = SetRasterLine(it, 0, hmax, vctr + vhalf, tempstorage);
				}
			//Throw away the temporary storage line, since we need it no longer.
			free(tempstorage);
			tempstorage = NULL;
			}
		else err = srl_bollixed;
		}
	else err = srl_bogusBuffer;
	return err;
	}

//Performs the same operation to a greybuf.
srl_result SwapGreyBufCorners(greybuf it)
	{
	}

srl_result InvertGreyBuf(greybuf it)
	{
	/*
	Invert the contents of a greybuf. Values from 0..255 become values from
	255..0. Black becomes white and vice versa.
	*/
	channelline peekline;
	srl_result err = srl_noErr;
	int vctr, hctr, hmax, vmax;
	hmax = GetGreyBufWidth(it);
	vmax = GetGreyBufHeight(it);
	if(it)
		{
		for(vctr = 0; vctr < vmax; vctr++)
			{
			peekline = PeekGreyRasterLine(it, vctr);
			if(peekline)
				{
				for(hctr = 0; hctr < hmax; hctr++)
					{
					peekline[hctr] = MAX_CHANVAL - peekline[hctr];
					}
				}
			else err = srl_bollixed;
			}
		}
	else err = srl_bogusBuffer;
	return err;
	}
