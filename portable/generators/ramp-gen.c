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


Rampgen - a simple module that just spits out a test pattern
It exists because it is very predictable :-) It should not go in any
working version of starfish

*/

#include "ramp-gen.h"
#include <math.h>

void* RampgenInit(void)
	{
	return NULL;
	}

void RampgenExit(void* refcon)
	{
	return;
	}

float Rampgen(float h, float v, void* refcon)
	{
	/*
	Calculate a grey value according to the position.
	Actually, for now, we just spit out medium grey...
	*/
	float tempv;
	float hpower, vpower;
#if 1
	return exp(h * v);
#endif
#if 0
	/*
	This algorithm assumes that you are dealing with a range
	from 0..1 in each direction, and that the origin is (0.5, 0.5).
	It returns 0 in the centre, 1 along all of the edges. Along the
	way, it curves gently. The slope of the curve is determined by
	the darkfactor. Darkfactors lower than 1 create a convex curve:
	most of the domain returns high values, sloping rapidly to low values.
	Darkfactors higher than 1 create a concave surface: the centre remains
	1, but it tails off to low values rather quickly.
	The "natural domain" of this function is 0 through 1/4. The peak value
	will always equal SCALEFACTOR/4. Thus a scalefactor of 4 results in a range
	of 0..1.
	The useful range for DARKFACTOR in edge-smoothing applications seems
	to be 0.35 (very little overlap) through 0.8 (extremely high overlap).
	*/
	#define DARKFACTOR 0.4		//lower values lighter, higher values darker, 0..1
	#define SCALEFACTOR 4
	if(h >= 0.5) h = 1 - h;
	if(v >= 0.5) v = 1 - v;
	return powf(h * v * SCALEFACTOR, DARKFACTOR);
#endif
#if 0
	float slope, nearleg;
	float fardist, neardist;
	//Calculate the angle from here to our known point.
	slope = v / h;
	if(slope < 1.0) slope = 1.0 / slope;
	nearleg = 1 / slope;
	//Now calculate the distance from centre to edgepoint.
	fardist = hypotf(nearleg, 1.0);
	neardist = hypotf(h, v);
	//The result is the ratio of neardist to fardist
	return fardist;
#endif
#if 0
	return fabs(0.5 - v) / fabs(0.5 -h);
#endif
#if 0
	return ((0.5 + fabs(0.5 - h)) * (0.5 + fabs(0.5 - v)))*1.0;
#endif
#if 0
	return (cos(hypotf(h - 0.3, v - 0.6) * 20.0) + 1.0) / 2.0;
	//Function to sum 4-way weighting and see what happens.
	//This one makes a pretty picture, but I don't think it's what I want.
#endif
#if 0
	if(h>0.5) h = 1.0 - h;
	if(v>0.5) v = 1.0 - v;
	return	(
					(powf(h * v, 0.5)) +
					(powf(fabs(1.0 - h) * v, 0.5)) +
					(powf(h * fabs(1.0 - v), 0.5)) +
					(powf(fabs(1.0 - h) * fabs(1.0 - v), 0.5))
					);
#endif
#if 0
	if(h>0.5) h = 1.0 - h;
	if(v>0.5) v = 1.0 - v;
	//Function to calculate weight for local pixel:
	return 0.5 + powf(h * v, 0.5);
#endif
#if 0
	if(h>0.5) h = 1.0 - h;
	if(v>0.5) v = 1.0 - v;
	//Function for SUM of remote pixels:
	return 0.5 - powf(h * v, 0.5);
#endif
#if 0
	/*
	Whoa! The following function produces some incredible turbulence
	and a really neat cross in the centre... check it out sometime
	*/
	//this works with or without the if(h>0.5) h = 1.0 - h; stuff.
	hpower = powf(fabs(0.5 - h), 1.0+fabs(0.5-h));
	vpower = powf(fabs(0.5 - v), 1.0+fabs(0.5-v));
	return powf(hpower * vpower, -2);
#endif
	/*
	*/
	}