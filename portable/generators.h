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

Individual generator algorithms work in floating-point space. The
generator works in pixelspace.

There are two ways to use this library. The easy way: Call Generate
with a set of dimensions and your choice of generator. It will return
a greybuf containing a texture created by that generator. Or there
is the hard, but more configurable way: Create a layer, then ask it
for pixel values at your leisure. When you're done asking your questions,
throw the layer away. Or use it again, we don't really care.

*/

#ifndef __starfish_generators__
#define __starfish_generators__ 0

#include "greymap.h"

//Opaque reference to all of the available generators.
typedef struct GeneratorList* GenListRef;
//Reference to a generator instance with its package of settings.
typedef struct LayerRec* LayerRef;

//Seek out and load up all available generators.
GenListRef LoadGenerators(void);
//Unload all generators and release the memory they occupied.
void UnloadGenerators(GenListRef list);
//How many generators are available?
int CountGenerators(GenListRef list);
//Create a texture of appropriate dimensions from this generator.
greybuf Generate(int ctr, int h, int v, GenListRef list);

//Create a layer for later inspection.
LayerRef MakeLayer(int ctr, int h, int v, GenListRef list);
//Get a pixel value from the layer. If out of bounds, returns MIN_CHANVAL.
channelval GetLayerPixel(int h, int v, LayerRef it);
//We are done with this layer; throw it away.
void DumpLayer(LayerRef it);

//If ROLL_TEXTURE is set true, the generator will randomize the origin
//point for each layer.
#define ROLL_TEXTURE 1

#endif //__starfish_generators__