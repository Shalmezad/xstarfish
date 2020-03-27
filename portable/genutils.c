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


Utilities common to generators
I wrote these once and don't want to have to write them again.

*/

#include <math.h>
#include <stdlib.h>
#include "genutils.h"

float frand(float range)
	{
	float denom, max;
	denom = rand();
	max = RAND_MAX;
	denom /= max;
	return range * denom;
	}

float frandge(float min, float max)
	{
	return frand(max - min) + min;
	}

int irand(int range)
	{
	return frand(range);
	}

int irandge(int min, int max)
	{
	return irand(max - min) + min;
	}

static int maybeshifter = 0;
int maybe(void)
	{
	//Flip a coin. Returns a zero/nonzero answer with equal probability.
	if(maybeshifter > 31) maybeshifter = 0;
	return (rand() >> maybeshifter++) & 1;
	}

int RandomPackMethod(void)
	{
	/*
	Pick a pack method at random
	and return it to the caller. Presumably, they will save it somewhere.
	*/
	return (rand() * PACK_METHOD_COUNT) / RAND_MAX;
	}

float PackedCos(float distance, float scale, int packmethod)
	{
	/*
	Many of the generators use a scheme where a wave is applied over
	a line. Since the range of a cosine wave is -1..0..1 rather than the
	simpler 0..1 expected by Starfish, we have to devise some way of packing
	the curve into the available range. These methods live in PackedCos, where
	they can be shared between all modules using such schemes.
	In addition, when new pack methods are devised, they can be added to the
	entire Starfish generator set simply by placing them in here.
	*/
	float rawcos, out;
	rawcos = cos(distance * scale);
	switch(packmethod)
		{
		case flipSignToFit:
			//When the scale goes negative, turn it positive.
			out = (rawcos >= 0) ? rawcos : -rawcos;
			break;
		case truncateToFit:
			//When the scale goes negative, add 1 to it to bring it in range
			out = (rawcos >= 0) ? rawcos : rawcos + 1;
			break;
		case scaleToFit:
			//Compress the -1..0..1 range of the normal cosine into 0..1
			out = (rawcos + 1.0) / 2.0;
			break;
		case slopeToFit:
			//use only the first half of the cycle. A saw-edge effect.
			out = (cos(fmod(distance * scale, pi)) + 1.0) / 2.0;
			break;
		default:
			//just to show me something's wrong
			out = 0.5;
		}
	return out;
	}
