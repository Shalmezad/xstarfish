/*

Copyright �1999 Mars Saxman
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


Code to create and access the contents of greymaps. A greymap is
a pixel buffer where each pixel holds one 8-bit channel. It is 
parallel to a pixmap, but uses a different API.

OOP would be nice here.

*/

#include <stdlib.h>
#include <string.h>
#include "greymap.h"

struct greybufrec
	{
	//How many pixels wide is one line?
	int horz;
	//How many rows tall is the buffer?
	int vert;
	//The block of data the pixels live in
	void* buffer;
	//An array of channellines pointing into the buffer
#if defined(__MWERKS__)
	channelline linestart[];
#else
	channelline linestart[0];
#endif
	};

greybuf MakeGreyBuf(int horz, int vert)
	{
	/*
	Create a pixel buffer.
	We allocate a greybufrec, fill out its data,
	and give it a big block to store pixel data in.
	We also fill out the linestarts table.
	Once this is done, we give it back to the user.
	If we fail, we return NULL.
	*/
	size_t rasterlinesize, pixarraysize, greybufsize;
	void* pixelarray = NULL;
	greybuf out = NULL;
	/*
	Start out by creating a big array of bytes to
	store all the pixels in. We will separate this out into
	raster lines later.
	*/
	rasterlinesize = horz * sizeof(channelval);
	pixarraysize = vert * rasterlinesize;
	pixelarray = malloc(pixarraysize);
	if(pixelarray)
		{
		/*
		We got the pixel array. Now we can make the rest
		of the greybuffer: the record and linestarts array.
		The size of the block whose address we return varies
		by the number of rows in the buffer. This is because
		we create one row-pointer for each 
		*/
		greybufsize = sizeof(struct greybufrec) + (sizeof(channelline) * vert);
		out = (greybuf)malloc(greybufsize);
		if(out)
			{
			/*
			We successfully created the greybuf record.
			Link the pixel array to this record, then fill out the
			addresses in the rasterline array.
			*/
			int rowctr;
			out->horz = horz;
			out->vert = vert;
			out->buffer = pixelarray;
			for(rowctr = 0; rowctr < vert; rowctr++)
				{
				out->linestart[rowctr] = (channelline)((long)pixelarray + (rasterlinesize * rowctr));
				}
			}
		else
			{
			/*
			We were not able to allocate the greybuf record.
			So we have to throw away the pixel buffer we already
			created, in order to not cause a memory leak.
			*/
			free(pixelarray);
			pixelarray = NULL;
			}
		}
	return out;
	}
	
srl_result DumpGreyBuf(greybuf it)
	{
	/*
	The user is done with the pixel buffer they created.
	Release all memory associated with this buffer.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		//Is there a pixel array in this greybuf?
		if(it->buffer)
			{
			//Free the buffer and clear out the field that held it.
			free(it->buffer);
			it->buffer = NULL;
			}
		//Free the greybuf as a whole.
		free(it);
		//It is now the caller's responsibility to stop using this greybuf.
		}
	else err = srl_bogusBuffer;
	return err;
	}

srl_result FillGreyBuf(greybuf it, channelval src)
	{
	/*
	Fill the entire greybuf up with this value.
	This is good if you want to initialize it in one call.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		//Loop through all of the rasterlines.
		int rowctr;
		for(rowctr = 0; rowctr < it->vert; rowctr++)
			{
			channelline line;
			line = it->linestart[rowctr];
			if(line)
				{
				//Now loop through all of the pixels in this line.
				int pixctr;
				for(pixctr = 0; pixctr < it->horz; pixctr++)
					{
					//Set this pixel to the value supplied.
					line[pixctr] = src;
					}
				}
			else err = srl_bollixed;
			}
		}
	else err = srl_bogusBuffer;
	return err;
	}	 

size_t GetGreyBufLineSize(greybuf it)
	{
	/*
	How many pixels wide is one scan line?
	That would be: the width of the greybuf times the size of one pixel.
	*/
	size_t out = 0;
	if(it)
		{
		out = it->horz * sizeof(channelval);
		}
	return out;
	}

int GetGreyBufHeight(greybuf it)
	{
	/*
	How many rows tall is the buffer?
	Simple accessor function.
	*/
	int out = 0;
	if(it)
		{
		out = it->vert;
		}
	return out;
	}

int GetGreyBufWidth(greybuf it)
	{
	/*
	How many pixels wide is one raster line?
	Again, a simple accessor function.
	*/
	int out = 0;
	if(it)
		{
		out = it->horz;
		}
	return out;
	}

srl_result GetGreyBufPixel(greybuf it, int horz, int vert, channelval* dest)
	{
	/*
	Retrieve one pixel from the buffer.
	We could do the math directly and retrieve the pixel from the pixelbuffer,
	but that assumes more about the way the greybuf system works than I really
	want to. Instead, we use the known item: the array of raster lines.
	If you grab a pixel out of bounds, the contents of dest are undefined.
	This is a very inefficient way to deal with the contents of the greybuf
	and should not be used if you care about speed.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		if	(
			(horz >= 0 && horz < it->horz) &&
			(vert >= 0 && vert < it->vert)
			)
			{
			/*
			The requested pixel is within the bounds of a valid greybuf.
			Look up the rasterline indicated by the pixel's row.
			*/
			channelline destline;
			destline = it->linestart[vert];
			if(destline)
				{
				/*
				Now look up the individual pixel in this row
				and copy it to the caller's destination pixel.
				*/
				if(dest)
					{
					*dest = destline[horz];
					}
				else err = srl_bogusParamPtr;
				}
			else err = srl_bollixed;
			}
		else err = srl_outOfBounds;
		}
	else err = srl_bogusBuffer;
	return err;
	}

srl_result SetGreyBufPixel(greybuf it, int horz, int vert, channelval src)
	{
	/*
	Places the caller's pixel into the buffer.
	This uses identical logic to GetGreyBufPixel and, like that function,
	should not be used in critical loops. Or in any situation where you are
	dealing with more than just a few pixels.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		if	(
			(horz >= 0 && horz < it->horz) &&
			(vert >= 0 && vert < it->vert)
			)
			{
			/*
			The requested pixel is within the bounds of a valid greybuf.
			Look up the rasterline indicated by the pixel's row.
			*/
			channelline destline;
			destline = it->linestart[vert];
			if(destline)
				{
				destline[horz] = src;
				}
			else err = srl_bollixed;
			}
		else err = srl_outOfBounds;
		}
	else err = srl_bogusBuffer;
	return err;
	}



srl_result GetGreyRasterLine(greybuf it, int start, int count, int vert, channelline dest)
	{
	/*
	Copy a range of pixels from one rasterline in the buffer.
	It is your responsibility to make sure that your dest buffer can
	hold the number of pixels you ask for.
	If your request falls off either end of the rasterline, or you
	ask for a negative number of pixels, this function will return
	srl_outOfBounds.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		//Is the user's request sane? Do we have the pixels they ask for?
		if	(
			(start >= 0 && start < it->horz) &&
			(count >= 0 && (start+count) <= it->horz) &&
			(vert >= 0 && vert < it->vert)
			)
			{
			/*
			Locate the line the caller wants to retrieve data from.
			Make sure it is valid and that we haven't gotten confused.
			*/
			channelline destline;
			destline = it->linestart[vert];
			if(destline)
				{
				/*
				If they supplied a non-null pixel buffer, copy
				the pixels into it.
				*/
				if(dest)
					{
					memcpy(dest, &destline[start], count * sizeof(channelval));
					}
				else err = srl_bogusParamPtr;
				}
			else err = srl_bollixed;
			}
		else err = srl_outOfBounds;
		}
	else err = srl_bogusBuffer;
	return err;
	}
	
srl_result SetGreyRasterLine(greybuf it, int start, int count, int vert, const channelline src)
	{
	/*
	Modify a range of pixels in one rasterline in the buffer.
	Takes data from the pixel array passed in.
	If your request falls off either end of the rasterline, or you
	supply a negative number of pixels, this function will return
	srl_outOfBounds.
	*/
	srl_result err = srl_noErr;
	if(it)
		{
		/*
		Is the user's request sane? Will their request overwrite 
		the legal area for this line?
		*/
		if	(
			(start >= 0 && start < it->horz) &&
			(count >= 0 && (start+count) <= it->horz) &&
			(vert >= 0 && vert < it->vert)
			)
			{
			/*
			Locate the rasterline the caller wants to retrieve data from.
			Make sure it is valid and that we haven't gotten confused.
			*/
			channelline destline;
			destline = it->linestart[vert];
			if(destline)
				{
				/*
				If the buffer they supplied is non-null, copy the
				data from it and into our rasterline.
				*/
				if(src)
					{
					memcpy(&destline[start], src, count * sizeof(channelval));
					}
				else err = srl_bogusParamPtr;
				}
			else err = srl_bollixed;
			}
		else err = srl_outOfBounds;
		}
	else err = srl_bogusBuffer;
	return err;
	}

channelline PeekGreyRasterLine(greybuf it, int vert)
	{
	/*
	If you want to examine all the pixels in a pixbuf, there are
	three ways to do it: iterate through each pixel, row by row,
	using GetRasterBufPixel on each one. This is miserably slow.
	Or you can iterate through each row, using GetRasterLine. This
	is still slow, since it is full of memcpy()s. Or, you can do
	it the right way: iterate through each row using PeekRasterLine
	to get direct access to the bits in the row.
	
	If you want to copy all of the data out of a pixbuf, use
	GetRasterLine; you won't save any time by using Peek. Or, if you want
	to wholesale change the contents of the pixbuf, use SetRasterLine.
	But if you want to read/modify/write most or all of the pixels in a
	pixbuf, use PeekRasterLine to get direct access and change the
	values by hand.
	
	The value you get back will either be NULL or a pointer to an array
	of pixels. If the latter, there will be precisely as many pixels in
	the array as there are columns in the pixbuf. If you write off the
	end of the rasterline, undefined things will happen.
	*/
	channelline out = NULL;
	if(it)
		{
		if (vert >= 0 && vert < it->vert)
			{
			out = it->linestart[vert];
			}
		}
	return out;
	}



