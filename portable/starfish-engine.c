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


This is the library that does what makes Starfish cool.

Starfish textures are multilayered, seamless tiles.
This is the main controller for the starfish engine. It doesn't do any
direct mathematics to create textures; instead, it asks plug-in generators
to do the math. Then it combines the simple, greyscale textures these generators
create into complex, colourful, multilayered eye candy.

Someday, there will be a way to give Starfish some kind of guidance as to what
sort of texture you are looking for: how much contrast you want, how bright
the colours should be, whether you prefer pastels or more intense hues. But for
now, Starfish makes all of those decisions on its own.

Starfish loads up a list of available generators and creates a colour
palette. It initializes the output pixmap to black, then iterates through
the following process:
- Pick a generator at random and ask it for a texture.
- Copy this grey texture into a colour scratchbuffer, using two randomly
	selected (but non-equal) colours from the palette to form a gradient.
- 50% of the time, pick a new generator at random and ask it for a new texture.
	Copy this texture into the scratchbuffer's alpha channel.
+ 25% of the time, copy the greyscale version of the texture into the scratch
	buffer's alpha channel.
+ 25% of the time, invert the greyscale version of the texture and copy it into
	the scratch buffer's alpha channel.
- Lay the scratch buffer on top of the output pixmap, using the scratch buffer's
	alpha channel to control transparency. 
- Return to the beginning.

The loop ends after a randomly-chosen small number of iterations.
I may at some point add an "alpha channel sampling" feature that continues
iterating until the output image reaches a certain average opacity level.

It recently dawned on me that if I made a few changes to the generators module,
I could generate textures pixel-at-a-time instead of layer-at-a-time, allowing
me to skip all the intermediate steps involving greybufs and merging and whatnot.
This approach is faster and more memory efficient, since I do not have to do big
operations with colossal image buffers all the time; I just do the math, one
pixel at a time.
The original monolithic "Starfish" function still works, but it is now expressed
in terms of point-by-point image grabbing internally.

*/

#include "starfish-engine.h"
#include "generators.h"
#include "starfish-rasterlib.h"
#include "genutils.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#if TEST_MODE
#define MAX_LAYERS 1
#define MIN_LAYERS 1
#else
#define MAX_LAYERS 6
#define MIN_LAYERS 2
#endif


typedef struct ColourLayerRec
	{
	//The image layer, a reference to a layer generator:
	LayerRef image;
	//The foreground colour, used for high image values.
	pixel fore;
	//The background colour, used for low image values.
	pixel back;
	//The mask image. If NULL, we use the image layer as its own mask.
	LayerRef mask;
	//If the flag is true, we invert the mask.
	int invertmask;
	}
ColourLayerRec;

typedef struct StarfishTexRec
	{
	/*
	A starfish texture is an array of colour layers.
	Count must be greater than 0 and less than or equal to MAX_LAYERS.
	tex[count] and above are undefined.
	*/
	int count;
	int width, height;
	int cutoff_threshold;
	GenListRef list;
	StarfishPalette colours;
	ColourLayerRec tex[MAX_LAYERS];
	}
StarfishTexRec;

static void RandomPalettePixel(const StarfishPalette* colours, pixel* out);

StarfishRef MakeStarfish(int hsize, int vsize, const StarfishPalette* colours)
	{
	/*
	Create a series of layers which we will later use to generate
	pixel data. These will contain the complete package of settings
	used to calculate image values.
	*/
	StarfishRef out = NULL;
	int dead = 0;		//error flag we set if allocations failed
	out = (StarfishRef)malloc(sizeof(StarfishTexRec));
	if(out)
		{
		int ctr;
		//How many layers are we going to use?
		out->count = irandge(MIN_LAYERS, MAX_LAYERS);
		out->width = hsize;
		out->height = vsize;
		out->cutoff_threshold = irand(MAX_CHANVAL / 16);
		//Copy in the colour palette, if we were given one.
		if(colours)
			{
			if(colours->colourcount > 1)
				{
				out->colours = *colours;
				if(out->colours.colourcount >= MAX_PALETTE_ENTRIES)
					{
					out->colours.colourcount = MAX_PALETTE_ENTRIES - 1;
					}
				}
			else out->colours.colourcount = 0;
			}
		else out->colours.colourcount = 0;
		//Load up all of the generators we can use.
		out->list = LoadGenerators();
		//Make some texture layers to generate from.
		if(out->list)
			{
			/*
			Clear out the values in the array before we begin allocating things.
			This makes recovery a lot easier if we fail midway through the allocation.
			*/
			for(ctr = 0; ctr < out->count; ctr++)
				{
				out->tex[ctr].image = NULL;
				out->tex[ctr].mask = NULL;
				}
			/*
			Now allocate random layers to use for the image and mask of this layer.
			Half the time, we use the image as its own mask.
			Half the time, we invert the mask.
			*/
			for(ctr = 0; ctr < out->count; ctr++)
				{
				int genid;
				#if TEST_MODE
				genid = TEST_GENERATOR;
				#else
				genid = irand(CountGenerators(out->list));
				#endif
				out->tex[ctr].image = MakeLayer(genid, hsize, vsize, out->list);
				//If we successfully created the image layer, see about creating a mask.
				//Otherwise, die now.
				if(!out->tex[ctr].image)
					{
					dead = !0;
					break;
					}
				//Flip a coin. If it lands heads-up, create another layer for use as a mask.
				if(maybe())
					{
					out->tex[ctr].mask = MakeLayer((rand() * CountGenerators(out->list)) / RAND_MAX, hsize, vsize, out->list);
					}
				//Flip another coin. If it lands heads-up, set the flag so we invert this layer.
				out->tex[ctr].invertmask = (maybe());
				//Now pick some random colours to use as fore and back of gradients.
				#if TEST_MODE
				out->tex[ctr].back.red = out->tex[ctr].back.green = out->tex[ctr].back.blue = MIN_CHANVAL;
				out->tex[ctr].fore.red = out->tex[ctr].fore.green = out->tex[ctr].fore.blue = MAX_CHANVAL;
				#else
				RandomPalettePixel(&out->colours, &out->tex[ctr].back);
				//The fore and back colours should NEVER be equal. 
				//Keep picking random colours until they don't match.
				do
					{
					RandomPalettePixel(&out->colours, &out->tex[ctr].fore);
					
					}
				while
					(
					out->tex[ctr].fore.red == out->tex[ctr].back.red &&
					out->tex[ctr].fore.green == out->tex[ctr].back.green &&
					out->tex[ctr].fore.blue == out->tex[ctr].back.blue
					);
				#endif
				}
			}
		else dead = (!0);	//we failed to load the list of generators
		/*
		Did we fail while setting up the layers? If so, throw away
		any layers we did successfully create.
		*/
		if(dead)
			{
			//If we have a generator list, unload it.
			if(out->list) UnloadGenerators(out->list);
			//Run through the list of colourtexlayers and throw away any grey layers
			//we managed to allocate.
			for(ctr = 0; ctr < out->count; ctr++)
				{
				if(out->tex[ctr].image) DumpLayer(out->tex[ctr].image);
				if(out->tex[ctr].mask) DumpLayer(out->tex[ctr].mask);
				}
			//Now throw away the out record, so we don't return anything to the caller.
			free(out);
			out = NULL;
			}
		}
	return out;
	}

void GetStarfishPixel(int h, int v, StarfishRef texture, pixel* out)
	{
	/*
	Calculate one pixel.
	We start with a black pixel.
	Then we loop through all of the layers, calculating each one with its
	mask. We then merge each layer's resulting pixel onto the out image.
	Once we're done, we return the merged pixel.
	We use alpha kind of backwards: high values mean high opacity, low values
	mean low opacity.
	*/
	channelval imageval, maskval;
	pixel outval;
	//Start out by initializing the output data.
	outval.red = outval.green = outval.blue = outval.alpha = 0;
	//Did we get valid parameters?
	if(texture && out && h >= 0 && v >= 0 && h < texture->width && v < texture->height)
		{
		//All of our parameters check out.
		int ctr;
		for(ctr = 0; ctr < texture->count; ctr++)
			{
			pixel layerpixel;
			float interval;
			ColourLayerRec* layer = &texture->tex[ctr];
			//Get the image value for this pixel, for this layer.
			imageval = GetLayerPixel(h, v, layer->image);
			#if TEST_MODE
			maskval = MAX_CHANVAL;
			#else
			//Do we have a mask texture? If we do, calculate its value.
			if(layer->mask) maskval = GetLayerPixel(h, v, layer->mask);
				else maskval = imageval;
			//Are we supposed to invert the mask value we got?
			if(layer->invertmask) maskval = MAX_CHANVAL - maskval;
			#endif
			/*
			Now we are ready. Calculate the image value for this layer.
			We use the image value as the proportion of the distance between
			two colours. We calculate this one channel at a time. This results
			in a smooth gradient of colour from min to max.
			*/
			//Calculate the red channel of the pixel.
			interval = imageval;
			interval /= CHANNEL_RANGE;
			interval *= layer->fore.red - layer->back.red;
			layerpixel.red = interval + layer->back.red;
			//Next calculate the green channel.
			interval = imageval;
			interval /= CHANNEL_RANGE;
			interval *= layer->fore.green - layer->back.green;
			layerpixel.green = interval + layer->back.green;
			//Calculate the blue channel in the same fashion
			interval = imageval;
			interval /= CHANNEL_RANGE;
			interval *= layer->fore.blue - layer->back.blue;
			layerpixel.blue = interval + layer->back.blue;
			//The alpha channel is merely the mask value.
			layerpixel.alpha = maskval;
			/*
			The image value for this layer is calculated.
			But the image is more than just this layer: it is the merged
			results of all the layers. So now we merge this value with
			the existing value calculated from the previous layers.
			We use the alpha channel to determine the proportion of blending.
			The new layer goes behind the existing layers; we use the existing
			alpha channel to determine what proportion of the new value shows
			through.
			*/
			//Calculate the red channel of the pixel.
			outval.red =
					(
					(outval.red * outval.alpha) + 
					(layerpixel.red * (CHANNEL_RANGE - outval.alpha))
					) / (CHANNEL_RANGE);
			//Next calculate the green channel.
			outval.green =
					(
					(outval.green * outval.alpha) + 
					(layerpixel.green * (CHANNEL_RANGE - outval.alpha))
					) / (CHANNEL_RANGE);
			//Calculate the blue channel in the same fashion
			outval.blue =
					(
					(outval.blue * outval.alpha) + 
					(layerpixel.blue * (CHANNEL_RANGE - outval.alpha))
					) / (CHANNEL_RANGE);
			/*
			Add the alpha channels (representing opacity); if the result is greater
			than 100% opacity, we just stop calculating (since no further layers
			will produce visible data).
			*/
			layerpixel.alpha = layerpixel.alpha * (MAX_CHANVAL - outval.alpha) / CHANNEL_RANGE;
			if(layerpixel.alpha + outval.alpha + texture->cutoff_threshold >= MAX_CHANVAL)
				{
				outval.alpha = MAX_CHANVAL;
				/*
				And now end the loop, because we've collected all the data we need.
				Calculating pixels from any of the deeper layers would just be a waste of time.
				*/
				break;
				}
			else outval.alpha += layerpixel.alpha;
			//And that, my friends, is that.
			}
		}
	//Return our modified pixel to the caller.
	*out = outval;
	}

void DumpStarfish(StarfishRef it)
	{
	/*
	We are done with this starfish image.
	Throw away all the layers we allocated, and the array we used
	to keep track of them.
	*/
	if(it)
		{
		int ctr;
		//Walk through the array and throw away any layers we loaded.
		for(ctr = 0; ctr < it->count; ctr++)
			{
			if(it->tex[ctr].image) DumpLayer(it->tex[ctr].image);
			if(it->tex[ctr].mask) DumpLayer(it->tex[ctr].mask);
			}
		//Unload the list of generators.
		if(it->list) UnloadGenerators(it->list);
		//Throw away our data structure, now we're done with it.
		free(it);
		}
	}

/*
The Starfish function by itself rolls the above three functions
into one step. Use this when you are in an environment with preemptive
multitasking (or you just don't care how long it takes) and you want
Starfish to create the pixel buffer for you.
*/

pixbuf Starfish(int horz, int vert, const StarfishPalette* colours)
	{
	/*
	Create a pixel buffer and fill it, using layers of
	algorithmically-created subtextures. The result, a complex
	blend of math, gradients, alpha blending, and fractals, will
	in theory be suitable for a wide variety of decorative purposes.
	This is basically the same code as MakeStarfish, GetStarfishPixel,
	and DumpStarfish all rolled together.
	*/
	pixbuf out = NULL;
	pixbuf templayer = NULL;
	StarfishRef it = NULL;
	//Create space for the destination texture.
	out = MakePixBuf(horz, vert);
	if(out)
		{
		//Create a starfish texture.
		it = MakeStarfish(horz, vert, colours);
		if(it)
			{
			//Now loop through all of the pixels of the scratchbox, filling in each one.
			int h, v;
			for(v = 0; v < vert; v++)
				{
				for(h = 0; h < horz; h++)
					{
					pixel temppixel;
					GetStarfishPixel(h, v, it, &temppixel);
					SetPixBufPixel(out, h, v, &temppixel);
					}
				}
			//Now we're done with the starfish texture, so throw it away.
			DumpStarfish(it);
			}
		else
			{
			//We couldn't make our starfish - throw the destination pixbuf away.
			DumpPixBuf(out);
			out = NULL;
			}
		}
	else
		{
		//Could not allocate the output pix buf. Uh oh.
		}
	return out;
	}

/*
A pair of trivial accessor functions
*/
int StarfishWidth(StarfishRef texture)
	{
	return texture ? texture->width : 0;
	}

int StarfishHeight(StarfishRef texture)
	{
	return texture ? texture->height : 0;
	}

static void RandomPalettePixel(const StarfishPalette* colours, pixel* out)
	{
	/*
	Pick a random pixel from this palette.
	If the palette is empty, create it from random values.
	*/
	if(out)
		{
		if(colours && colours->colourcount > 1)
			{
			int index;
			index = (rand() * colours->colourcount) / RAND_MAX;
			*out = colours->colour[index];
			}
		else
			{
			out->red = irand(MAX_CHANVAL);
			out->green = irand(MAX_CHANVAL);
			out->blue = irand(MAX_CHANVAL);
			}
		}
	}
