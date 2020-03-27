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

*/

#ifndef __starfish_engine__
#define __starfish_engine__ 0

#include "starfish-rasterlib.h"

/*
If you choose to supply a palette to MakeStarfish, it must be
in the following format. While your palette may be longer than
MAX_PALETTE_ENTRIES, only the first MAX_PALETTE_ENTRIES records
will be used. It is not necessary to allocate more pixel records
than you actually plan to use.
*/

#define MAX_PALETTE_ENTRIES 256
typedef struct StarfishPalette
	{
	int colourcount;
	pixel colour[MAX_PALETTE_ENTRIES];
	}
StarfishPalette;

/*
Feed Starfish the parameters you want, and it returns you a brand-new
pixel buffer with a seamless, 24-bit texture inside. Yours to keep.
h is the horizontal size, v is the vertical size, both in pixels.
*/
pixbuf Starfish(int hsize, int vsize, const StarfishPalette* colours);

/*
Create a starfish texture. Then ask for its pixels, using whatever pace
and order you want. Once you are done, throw away the texture.
This lets you control scheduling yourself, instead of having to spawn
off extra threads, and requires far less memory.
The StarfishPalette is read-only, and you needn't maintain it after calling
MakeStarfish. The engine keeps its own internal copy of the palette.
If you pass NULL, the engine uses the full colour spectrum.
*/
typedef struct StarfishTexRec* StarfishRef;

StarfishRef MakeStarfish(int hsize, int vsize, const StarfishPalette* colours);
void GetStarfishPixel(int h, int v, StarfishRef texture, pixel* out);
void DumpStarfish(StarfishRef it);
int StarfishWidth(StarfishRef texture);
int StarfishHeight(StarfishRef texture);

/*
The starfish engine can be run in test mode.
In this mode, its output is one single layer, scaled from
black to white, of a specific generator.
This is useful when generator code isn't working and you don't
know exactly why. Set TEST_MODE to 1 if you want this, 0 otherwise.
In testmode, the generator number TEST_GENERATOR in the list
will be the one executed. There will be no colours
*/
#define TEST_MODE 0
#define TEST_GENERATOR 5

#endif //__starfish_engine__
