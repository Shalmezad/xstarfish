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


Starfish

Coswave
This is the original texture from Starfish's venerable ancestor.
This was cool enough by itself, but when you combine it with the
trippy-cool edge wrapping code in starfish, it creates *really*
neat turbulent lumpy patterns. Very smooth.
This is an astonishingly versatile generator, as the simplest
formulae often are.
OK, *I* think it's cool.

Stop laughing already.

*/

#include "genutils.h"
#include "coswave-gen.h"
#include <stdlib.h>
#include <math.h>

enum waveaccelmethods
	{
	accelNone,
	accelLinear,
	accelSine,
	ACCEL_METHOD_COUNT
	};

typedef struct CoswaveGlobals
	{
	float originH, originV;
	float wavescale;
	float squish, sqangle, distortion;
	int packmethod;
	int accelmethod;
	float accel;
	}
CoswaveGlobals;

void* CoswaveInit(void)
	{
	/*
	We don't actually care about the pixel size of this texture, though I suppose
	we *could* use it for some kind of scalefactor correction.
	All we do here is pick some random values, put them in a global record,
	and return it.
	*/
	CoswaveGlobals* out = NULL;
	out = (CoswaveGlobals*)malloc(sizeof(CoswaveGlobals));
	if(out)
		{
		out->originH = frand(1);
		out->originV = frand(1);
		out->packmethod = RandomPackMethod();
		/*
		I once attempted to make the coswave shift its scale over time, much like
		the spinflake generator does with its twist. I wasn't particularly succesful.
		But I did happen upon a *beautifully* bizarre twist to the generator which is
		really strange but not terribly useful. So I fire it once in every 64 generations
		or so, which is just infrequent enough that the viewer really goes "what the hell
		is THAT" when they see it.
		It's chaotic moirŽness, sorta - the wavescale increases by the exponent of the
		distance. At some point, the wavescale becomes less than one pixel, and then chaos
		begins to happen. Odd eddies show up, turbulences become visible, and a bit of static
		shines through here and there. It's quite beautiful in an abstract sort of way.
		*/
		if(maybe() && maybe() && maybe() && maybe() && maybe() && maybe())
			{
			out->accelmethod = accelLinear;
			out->accel = frand(2.0) + 1.0;
			}
		else out->accelmethod = accelNone;
		/*
		Packmethods flipsign and truncate effectively double the wavescale,
		because they turn both peaks and valleys into peaks. So we use a lower
		wavescale, then double it with the scaleToFit method to put it in range
		with the other packmethods.
		*/
		out->wavescale = frand(25) + 1.0;
		if(out->packmethod == scaleToFit) out->wavescale *= 2;
		/*
		We don't like waves that are always perfect circles; they're too
		predictable. So we "squish" them a bit. We choose a squish factor, which
		serves as a multiplier. Currently wave scale modifications can range from
		half length to double length. It would be fun to widen this sometime and 
		see what happened.
		The squish angle determines the "direction" of the squish effect. The
		strength of the squish is determined by the sine of the difference between
		the angle between the current point and the origin, and the sqangle.
		*/
		out->squish = frand(2.0) + 0.5;
		if(maybe()) out->squish = -out->squish;
		out->sqangle = frand(pi);
		out->distortion = frand(1.5) + 0.5;
		}
	return out;
	}

void CoswaveExit(void* refcon)
	{
	//If we successfully created a globals record, throw it away now.
	if(refcon) free(refcon);
	}

float Coswave(float h, float v, void* refcon)
	{
	float out = 0.0;
	CoswaveGlobals* glb = (CoswaveGlobals*)refcon;
	if(glb)
		{
		/*
		Calculate this point's distance from the origin.
		Perform a cosine on the point.
		Then move the results of the cosine into the appropriate range.
		*/
		float hypotenuse, hypangle;
		float rawcos, compwavescale;
		//Rotate the axes of this shape.
		h -= glb->originH;
		v -= glb->originV;
		hypangle = atan((v / h) * glb->distortion) + glb->sqangle;
		hypotenuse = hypot(h, v);
		h = (cos(hypangle) * hypotenuse);
		v = (sin(hypangle) * hypotenuse);
		//Calculate the squished distance from the origin to the desired point.
		hypotenuse = hypot(h * glb->squish, v / glb->squish);
		//Scale the wavescale according to our accelerator function.
		switch(glb->accelmethod)
			{
			case accelNone:	
				compwavescale = glb->wavescale;
				break;
			case accelLinear:
				compwavescale = powf(glb->wavescale, hypotenuse * glb->accel);
				break;
			}
		//Now map our cosine function along that distance.
		out = PackedCos(hypotenuse, compwavescale, glb->packmethod);
		}
	return out;
	}
