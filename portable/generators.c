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


Starfish - Generator Manager
Textures become complex by merging layers of simpler textures.
Each basic layer is the result of a Generator - a function or
algorithm that creates two-dimensional images.

The generator manager creates a table of all installed generators.
There are several different types of generators, and they produce
graphics in different ways. The manager keeps track of all of this
information and provides a uniform interface for the starfish
engine itself.

There are two ways to use this library. The easy way: Call Generate
with a set of dimensions and your choice of generator. It will return
a greybuf containing a texture created by that generator. Or there
is the hard, but more configurable way: Create a layer, then ask it
for pixel values at your leisure. When you're done asking your questions,
throw the layer away. Or use it again, we don't really care. The
advantage(s) to calling the generator library point-by-point are that
it uses a fraction of the memory (since there is no need to allocate
a greybuf to hold the image under construction) and that scheduling is
under your control (you don't need to spawn off a separate thread to
have more than one thing going on at a time).

*/

#include "greymap.h"
#include "generators.h"
#include <math.h>

/*
We hard-link the generators for now. Eventually these will become
dynamically loaded sharedlibs, picked out of a directory.
Or maybe they won't. There doesn't seem to be much need for a pluggable
system. The whole thing works not because of its diversity, but because of
its carefully designed limitations.
*/
//#include "ramp-gen.h"				//test lib, not used in production code
#include "coswave-gen.h"
#include "spinflake-gen.h"
#include "rangefrac-gen.h"
#include "flatwave-gen.h"
#include "bubble-gen.h"
//#include "branchfrac-gen.h"

#ifndef true
#define true 1 
#endif
#ifndef false
#define false 0
#endif

/*
Function to allocate storage & create values for the generator.
The generator should use this opportunity to create random settings,
allocate space, and otherwise get ready to spew out a bunch of data.
The value the init function returns will be saved and passed back to
the generator on every call from now on as the "refcon" value.
This is designed to make it easier for generators to support multiple
threads working on different output layers. 
*/
typedef void* (*GenInitProc)(void);
/*
Function to clean up all the mess created by the InitProc. After the
exit proc has been called, the refcon value will be discarded, so
you'd better throw away everything you allocated and stored in it.
*/
typedef void (*GenExitProc)(void* refcon);
/*
This returns the value of one point.
The assumed area covered by the output texture (the "interesting
area") is 0..1 in both horizontal and vertical. You can ask for
data outside the interesting area, but it's not guaranteed to have
anything worth looking at. The generator is, however, required to
return meaningful data UNLESS it handles its own seamless-wrapping.
The process of wrapping the edges of a generator's output requires
out-of-range values.
The returned value will be from 0..1; if the generator screws up
and sends you something outside that range, this module will
simply cap it off at 0 or 1, as appropriate.
*/
typedef float (*GenPointProc)(float h, float v, void* refcon);

//Description of one generator - everything we know about it.
typedef struct GeneratorRec
	{
	int isAntiAliased;
	int isSeamless;
	GenInitProc init;	//function to start up the generator
	GenExitProc exit;	//function to close the generator down
	GenPointProc process;		//processor function that does all the real work	
	}
GeneratorRec;

//Structure to manage a list of all installed generators
struct GeneratorList
	{
	int generatorCount;
	GeneratorRec* gen;
	};

//Structure keeping track of a single generator layer.
typedef struct LayerRec
	{
	GeneratorRec* gencode;
	void* refcon;
	int hmax, vmax;
	int rollh, rollv;
	}
LayerRec;

#define CHANNELVAL_FMAX 255.0

static greybuf GeneratePointFunction(int h, int v, LayerRef gen);
static float GetWrappedPoint(float hpos, float vpos, void* refcon, GeneratorRec* gen);
static float GetAntiAliasedPoint(float hpos, float vpos, float fudge, void* refcon, GeneratorRec* gen);

GenListRef LoadGenerators(void)
	{
	/*
	Seek out and load up all available generators.
	Someday, this will be a really neat, fancy routine that scans a directory
	looking for shared libraries. It'll load those libraries, interrogate them,
	and build a generator list from that.
	But, for now, this is a quick hack: we create the table by hand, using
	compiled-in code.
	*/
	int gencount;
	GenListRef out;
	size_t genlistsize;
	//Count the number of shared libraries in the directory.
	gencount = 5;
	//Create a genlist big enough to hold that many generators.
	out = malloc(sizeof(struct GeneratorList));
	genlistsize = gencount * sizeof(GeneratorRec);
	if(out) out->gen = (GenListRef)malloc(genlistsize);
	if(out && out->gen)
		{
		//Poke in the generator count so we can get at it later.
		out->generatorCount = gencount;
		//Loop through the generators, filling out each record one by one.
		//Our first one is the workhorse Coswave. It can do anything. 
		out->gen[0].isAntiAliased = false;
		out->gen[0].isSeamless = false;
		out->gen[0].init = &CoswaveInit;
		out->gen[0].exit = &CoswaveExit;
		out->gen[0].process = &Coswave;
		//Next is the spinflake generator, for more shapely patterns.
		out->gen[1].isAntiAliased = false;
		out->gen[1].isSeamless = true;
		out->gen[1].init = &SpinflakeInit;
		out->gen[1].exit = &SpinflakeExit;
		out->gen[1].process = &Spinflake;
		//The range fractal, which creates mountainous organic rough textures.
		out->gen[2].isAntiAliased = true;
		out->gen[2].isSeamless = true;
		out->gen[2].init = &RangefracInit;
		out->gen[2].exit = &RangefracExit;
		out->gen[2].process = &Rangefrac;
		//The flatwave generator, which creates interfering linear waves.
		out->gen[3].isAntiAliased = false;
		out->gen[3].isSeamless = false;
		out->gen[3].init = &FlatwaveInit;
		out->gen[3].exit = &FlatwaveExit;
		out->gen[3].process = &Flatwave;
		/*
		//The branch fractal, which creates vegetable structures
		out->gen[4].isAntiAliased = true;
		out->gen[4].isSeamless = true;
		out->gen[4].init = &BranchfracInit;
		out->gen[4].exit = &BranchfracExit;
		out->gen[4].process = &Branchfrac;
		*/
		//Bubble generator, which creates lumpy, curved turbulences.
		out->gen[4].isAntiAliased = true;
		out->gen[4].isSeamless = true;
		out->gen[4].init = &BubbleInit;
		out->gen[4].exit = &BubbleExit;
		out->gen[4].process = &Bubble;
		}
	return out;
	}

void UnloadGenerators(GenListRef list)
	{
	/*
	Unload all generators and release the memory they occupied.
	For the moment, there is nothing we need to do to release the generators.
	All we have to do is throw away the memory occupied by the generator list.
	*/
	if(list)
		{
		if(list->gen) free(list->gen);
		free(list);
		list = NULL;
		}
	}

int CountGenerators(GenListRef list)
	{
	/*
	How many generators are available?
	If this is a bogus list, the answer is necessarily zero.
	*/
	int out = 0;
	if(list)
		{
		out = list->generatorCount;
		}
	return out;
	}

greybuf Generate(int ctr, int h, int v, GenListRef list)
	{
	/*
	Create a texture of appropriate dimensions from this generator.
	This is where all the interesting work starts getting done for
	the generators. 
	Our job is to verify the request - make sure it is a valid generator,
	and that the input parameters are sane.
	The end result of Generate is either NULL, or a greybuf containing an anti-aliased,
	seamlessly wrapped greyscale 8-bit monolayer texture.
	We don't care what happens to the greybuf after we produce it.
	*/
	LayerRef layer = NULL;
	greybuf out = NULL;
	if(list)
		{
		//Create a new layer with the settings we were given.
		layer = MakeLayer(ctr, h, v, list);
		if(layer)
			{
			//Pass the layer to the greybuf creator to retrieve our image.
			out = GeneratePointFunction(h, v, layer);
			//Now throw away the layer, since we no longer need it.
			DumpLayer(layer);
			}
		}
	return out;
	}

LayerRef MakeLayer(int genctr, int h, int v, GenListRef list)
	{
	/*
	Create a layer for later inspection.
	We store all the data about this layer that we know into a
	LayerRec. We also initialize the specific generator we are
	going to use.
	*/
	LayerRef out = NULL;
	//Verify our input parameters.
	//The following line was the source of an extremely stupid bug in 1.0 through 1.1d3.
	if(genctr >= 0 && genctr < CountGenerators(list) && h > 0 && v > 0)
		{
		out = (LayerRef)malloc(sizeof(LayerRec));
		if(out)
			{
			//Put in all the info the caller gave us in parameters.
			out->gencode = &list->gen[genctr];
			out->hmax = h;
			out->vmax = v;
			#if ROLL_TEXTURE
			out->rollh = (rand() * h) / RAND_MAX;
			out->rollv = (rand() * v) / RAND_MAX;
			#else
			out->rollh = out->rollv = 0;
			#endif
			//Now initialize our generator and save its refcon.
			out->refcon = out->gencode->init ? out->gencode->init() : NULL;
			}
		}
	return out;
	}
	
channelval GetLayerPixel(int h, int v, LayerRef it)
	{
	/*
	Get a pixel value from the layer. If out of bounds, returns MIN_CHANVAL.
	There is no hard guarantee that if you ask for the same pixel twice, you
	will get exactly the same answer; however, the value should fit within
	its surrounding texture no matter when you ask for it.
	You don't have to ask for pixels in any specific order.
	*/
	channelval out = MIN_CHANVAL;
	if(it && h >= 0 && v >= 0)
		{
		if(h < it->hmax && v < it->vmax)
			{
			/*
			Calculate the point they wanted.
			Basically, we convert all of the coordinates into floating point
			values from 0 through 1. This lets the generators put out the same
			images regardless of the dimensions of the output data.
			Then we calculate the image, using the traditional old wrap/alias
			code. Then we convert the floating point value to a standard 0..255
			value and return it to the caller.
			*/
			float fhpos, fvpos, fhmax, fvmax, pixelval, fudge;
			fhpos = ((h + it->rollh) < it->hmax) ? h + it->rollh : h + it->rollh - it->hmax;
			fvpos = ((v + it->rollv) < it->vmax) ? v + it->rollv : v + it->rollv - it->vmax;
			fhmax = it->hmax;
			fvmax = it->vmax;
			fudge = 1.0 / (fhmax + fvmax);
			out = GetAntiAliasedPoint(fhpos / fhmax, fvpos / fvmax, fudge, it->refcon, it->gencode) * CHANNELVAL_FMAX;
			}
		}
	return out;
	}

void DumpLayer(LayerRef it)
	{
	/*
	We are done with this layer; throw it away.
	Also shut down the generator.
	*/
	if(it)
		{
		//Shut down the generator.
		if(it->gencode->exit) it->gencode->exit(it->refcon);
		//That's about all we have to do.
		free(it);
		it = NULL;
		}
	}

static greybuf GeneratePointFunction(int hmax, int vmax, LayerRef it)
	{
	/*
	We've been given a layer. The caller wants a greybuf with the entire
	contents of the layer. We create a greybuf, iterate through the layer,
	and pick off all its pixel values.
	*/
	greybuf out = NULL;
	/*
	First order of business: Create a greybuf to hold our output.
	This will have the size and specifications given in the parameters.
	*/
	out = MakeGreyBuf(hmax, vmax);
	if(out)
		{
		/*
		Iterate through every pixel in the greybuf.
		For each pixel, grab a value from the layer function and install it
		as the value in the pixel.
		*/
		int hctr, vctr;
		//Iterate through each line of the texture, calculating points as we go.
		for(vctr = 0; vctr < vmax; vctr++)
			{
			for(hctr = 0; hctr < hmax; hctr++)
				{
				SetGreyBufPixel(out, hctr, vctr, GetLayerPixel(hctr, vctr, it));
				}
			}
		}
	return out;
	}

static float GetAntiAliasedPoint(float fhpos, float fvpos, float fudge, void* refcon, GeneratorRec* gen)
	{
	float pixelval;
	pixelval = GetWrappedPoint(fhpos, fvpos, refcon, gen);
	if(!gen->isAntiAliased)
		{
		/*
		This generator does not anti-alias itself.
		We need to do the anti-aliasing for it.
		The way we do this is to ask for a few more points, positioned
		between this point and the next one that will be computed.
		We then average all of these point values together. This does
		not affect the appearance of smooth gradients, but it significantly
		improves the way sharp transitions look. You can't see the individual
		pixels nearly so easily.
		*/
		pixelval += GetWrappedPoint(fhpos + fudge, fvpos, refcon, gen);
		pixelval += GetWrappedPoint(fhpos, fvpos + fudge, refcon, gen);
		pixelval += GetWrappedPoint(fhpos + fudge, fvpos + fudge, refcon, gen);
		pixelval /= 4;
		}
	return pixelval;
	}

static float GetWrappedPoint(float fhpos, float fvpos, void* refcon, GeneratorRec* gen)
	{
	/*
	Get a point from this function.
	But don't just get the point - also get some out-of-band values and mix
	them in proportionately. This results in a seamlessly wrapped texture,
	where you can't see the edges.
	Some functions do this on their own; if that's the case, we let it do it.
	Otherwise, we do the computations ourself.
	*/
	float pixelval = 0;
	GenPointProc tempfn;
	tempfn = (GenPointProc)gen->process;
	pixelval = tempfn(fhpos, fvpos, refcon);
	/*
	If this function does not generate seamlessly-tiled textures,
	then it is our job to pull in out-of-band data and mix it in
	with the actual pixel to get a smooth edge.
	*/
	if(!gen->isSeamless)
		{
		/*
		We mix this pixel with out-of-band values from the opposite side
		of the tile. This is a "weighted average" proportionate to the pixel's
		distance from the edge of the tile. This creates a smoothly fading
		transition from one side of the texture to the other when the edges are
		tiled together.
		*/
		float farh, farv;
		float farval1, farval2, farval3;
		float totalweight, weight, farweight1, farweight2, farweight3;
		//The farh and farv are on the opposite side of the tile.
		farh = fhpos + 1.0;
		farv = fvpos + 1.0;
		//There are three pixel values to grab off the edges.
		farval1 = tempfn(fhpos, farv, refcon);
		farval2 = tempfn(farh, fvpos, refcon);
		farval3 = tempfn(farh, farv, refcon);
		//Calculate the weight factors for each far point.
		weight = fhpos * fvpos;
		farweight1 = fhpos * (2.0 - farv);
		farweight2 = (2.0 - farh) * fvpos;
		farweight3 = (2.0 - farh) * (2.0 - farv);
		totalweight = weight + farweight1 + farweight2 + farweight3;
		//Now average all the pixels together, weighting each one by the local vs far weights.
		pixelval =
			((pixelval * weight) + (farval1 * farweight1) + (farval2 * farweight2) + (farval3 * farweight3))
					 / totalweight;
		}
	/*
	If the generator messes up and returns an out-of-range value, we clip it here.
	This way, curves that leap out of bounds simply get chopped off, instead of getting
	renormalized at the opposite end of the scale leading to big discontinuities and ugliness.
	This can mask bugs in a generator, but we aren't the generator so we don't care.
	If you're writing a generator it is your job to make your code work, and my job to
	make sure my code works even if yours doesn't.
	*/
	if(pixelval > 1.0) pixelval = 1.0;
	if(pixelval < 0.0) pixelval = 0.0;
	return pixelval;
	}
