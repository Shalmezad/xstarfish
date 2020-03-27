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

Spinflake Generator. Embosses rotationally symmetrical shapes.
Mad ppatter! 1.6 introduced an attempt at creating crystalline,
pseudo-organic leaf/snowflake like shapes. It was all vector based,
not terribly smooth, and (in my opinion) did not work all that well.
This takes an even older graphics algorithm I worked out, tweaks it
a bit, and uses it to create a similar effect.

The basic idea is that the generator picks an origin point and some
parameters for a sine wave. It then lays a second sine wave on top
of the first; their periods do not have to match. The result is a
perturbed, vaguely symmetrical shape radiating from a point.

This gives us a line. For every point in the problem domain, we extend
a ray from the origin, through the point, until we hit the line.
We then calculate the distance from the origin and the distance from
the origin to the line, along that ray. The Z value is the distance from
the point to the line, scaled proportionally to the distance from the line
to the origin.

If the point is outside the line, the value is 1/(distance + 1). 

Or something like that.

Spinflake does its own texture-wrapping. The default wrapping just
kills the contrast; we don't like that, so we wrap it our own way.

*/

#include "spinflake-gen.h"
#include "genutils.h"
#include <math.h>
#include <stdlib.h>

enum sinepositivizingmethods
	{
	sineCompressMethod,
	sineTruncateMethod,
	sineAbsoluteMethod,
	sineSawbladeMethod,
	MAX_SINEPOS_METHODS
	};

enum twirlmethods
	{
	twirlNoneMethod,
	twirlCurveMethod,
	twirlSineMethod,
	twirlAccelMethod,
	MAX_TWIRL_METHODS
	};

#define MIDPOINT 0.5
#define MAX_TWIRL 14
#define MAX_SINEAMP 4.0
#define MAX_FLORETS 3

typedef struct floret
	{
	int sineposmethod;
	int backward;
	int spines;
	float spineradius;
	float twirlbase, twirlspeed, twirlamp, twirlmod;
	int twirlmethod;
	}
floret;

typedef struct spinflake
	{
	float originH, originV;
	float radius, squish, twist;
	int averageflorets;
	int florets;
	floret layer[MAX_FLORETS];
	}
spinflake;

typedef struct SpinflakeGlobals
	{
	int spinflakes;
	spinflake flake[1];
	}
SpinflakeGlobals;

void InitFloret(floret* it);
void InitSpinflake(spinflake* it);
static float calctheta(float h, float v, spinflake* it);
static float calcwave(float theta, float origindist, floret* it);
static float chopsin(float theta, floret* glb);
static float rawpoint(float h, float v, spinflake* glb);
static float vtiledpoint(float h, float v, SpinflakeGlobals* glb);

void InitFloret(floret* it)
	{
	/*
	Pick a random packing method for the sine wave.
	We have several ways to use the sine function's range to
	produce a 0..1 value.
	*/
	it->sineposmethod = (rand() * MAX_SINEPOS_METHODS) / RAND_MAX;
	//If backward is true, we will flip the sine wave over.
	it->backward = maybe();
	//Pick a random number of "spines" on the wave, from 1 to 16.
	it->spines = (rand() * 15 / RAND_MAX) + 1;
	//All modes but absolute-method require an even number of spines.
	if(it->sineposmethod != sineAbsoluteMethod && (it->spines & 1)) it->spines++;
	//Pick a height for the spines, similar to the range of the main radius.
	it->spineradius = frand(0.5);
	//Instead of aligning to the Y axis, twirl the flake a bit.
	it->twirlbase = frand(pi);
	//We use different methods to twirl the flake for unique effects.
	it->twirlmethod = (rand() * MAX_TWIRL_METHODS) / RAND_MAX;
	switch(it->twirlmethod)
		{
		case twirlNoneMethod:
			break;
		case twirlSineMethod:
			it->twirlspeed = frand(MAX_TWIRL * pi);
			it->twirlamp = frand(MAX_SINEAMP * 2) - MAX_SINEAMP;
			it->twirlmod = frand(1.0) - 0.5;
			break;
		case twirlCurveMethod:
			it->twirlspeed = frand(MAX_TWIRL * 2) - MAX_TWIRL;
			it->twirlamp = frand(MAX_SINEAMP * 2) - MAX_SINEAMP;
			break;
		}
	}

void InitSpinflake(spinflake* it)
	{
	/*
	Pick a random location and size for this spinflake.
	Then calculate up some florets to add - without any, it would be
	an ordinary (and boring) circle. We like more complicated shapes.
	*/
	int ctr;
	//Pick somewhere for the flake to radiate from.
	it->originH = frand(1);
	it->originV = frand(1);
	//Pick a random radius for our main circle.
	it->radius = frand(0.5);
	//Squish it horizontally/vertically a bit. Just a small bit.
	it->squish = 0.25 + frand(2.75);
	it->twist = frand(pi);
	//Flip a coin - should we average out the values of our florets, or merely combine?
	it->averageflorets = maybe();
	//Now fill out our florets.
	it->florets = ((rand() * MAX_FLORETS) / RAND_MAX) + 1;
	for(ctr = 0; ctr < it->florets; ctr++)
		{
		InitFloret(&it->layer[ctr]);
		}
	}

void* SpinflakeInit(void)
	{
	/*
	Create a globals record and fill out all appropriate random values.
	*/
	SpinflakeGlobals* out;
	out = (SpinflakeGlobals*)malloc(sizeof(SpinflakeGlobals));
	if(out)
		{
		InitSpinflake(&out->flake[0]);
		}
	return out;
	}

void SpinflakeExit(void* refcon)
	{
	/*
	If we have a valid globals record, throw it away now.
	*/
	if(refcon)
		{
		free(refcon);
		}
	}

float Spinflake(float h, float v, void* refcon)
	{
	float out;
	SpinflakeGlobals* glb = (SpinflakeGlobals*)refcon;
	if(glb)
		{
		float point, farpoint, weight, farweight;
		point = vtiledpoint(h, v, glb);
		if(h > 0.5)
			{
			farpoint = vtiledpoint(h - 1.0, v, glb);
			farweight = (h - 0.5) * 2.0;
			weight = 1.0 - farweight;
			out = (point * weight) + (farpoint * farweight);
			}
		else
			{
			out = point;
			}
		}
	return out;
	}

static float chopsin(float theta, floret* glb)
	{
	float out = 0;
	out = sin(theta);
	switch(glb->sineposmethod)
		{
		case sineCompressMethod:
			out = (out + 1.0) / 2.0;
			break;
		case sineAbsoluteMethod:
			out = fabs(out);
			break;
		case sineTruncateMethod:
			if(out < 0) out += 1.0;
			break;
		case sineSawbladeMethod:
			theta = fmod(theta / 4.0, pi / 2.0);
			if(theta < 0) theta += (pi / 2.0);
			out = sin(theta);
			break;
		}
	if(glb->backward)
		{
		out = 1.0 - out;
		}
	return out;
	}

static float vtiledpoint(float h, float v, SpinflakeGlobals* glb)
	{
	/*
	Return one data point, seamlessly-fused vertically.
	*/
	float out;
	float point, farpoint, weight, farweight, totalweight;
	point = rawpoint(h, v, &glb->flake[0]);
	if(v > 0.5)
		{
		farpoint = rawpoint(h, v - 1.0, &glb->flake[0]);
		farweight = (v - 0.5) * 2.0;
		weight = 1.0 - farweight;
		out = (point * weight) + (farpoint * farweight);
		}
	else
		{
		out = point;
		}
	return out;
	}

static float rawpoint(float h, float v, spinflake* glb)
	{
	/*
	Calculate one raw data point.
	This does the calculations without worrying about seamless-tile wrapping.
	*/
	float pointangle, origindist, edgedist, proportiondist;
	float out;
	/*
	Rotate the point around our origin. This lets the squashed bulge-points on
	the sides of the squished spinflake point in random directions - not just aligned
	with the cartesian axes.
	*/
	float hypangle;
	h -= glb->originH;
	v -= glb->originV;
	hypangle = atan(v / h) + glb->twist;
	origindist = hypotf(h, v);
	h = (cos(hypangle) * origindist);
	v = (sin(hypangle) * origindist);
	//Calculate the distance from the origin to this point. Again.
	origindist = hypotf(h * glb->squish, v / glb->squish);
	//If we are at the origin, there is no need to do the computations.
	if(origindist)
		{
		int ctr;
		//The edge is (currently) a circle some radius units away.
		//Compute the angle this point represents to the origin.
		pointangle = calctheta(h, v, glb);
		edgedist = glb->radius;
		for(ctr = 0; ctr < glb->florets; ctr++)
			{
			edgedist += calcwave(pointangle, origindist, &glb->layer[ctr]);
			}
		if(glb->averageflorets) edgedist /= glb->florets;
		//Our return value is the distance from the edge, proportionate
		//to the distance from the origin to the edge.
		proportiondist = ((edgedist - origindist) / edgedist);
		//If the value is >=0, we are inside the shape. Otherwise, we're outside it.
		if(proportiondist >= 0) out = sqrt(proportiondist);
			else out = 1.0 - (1.0 / (1 - proportiondist));
		}
	else out = 1.0;
	return out;
	}

static float calcwave(float theta, float dist, floret* it)
	{
	/*
	Calculate the distance from centre this floret adds to the mix
	at the particular angle supplied.
	This is where we incorporate the floret's spines and twirling.
	Oddly, a spinflake's florets don't have to twirl in unison. This
	can get really interesting. If it doesn't work, migrate the twirl back
	to the spinflake instead.
	*/
	float cosparam;
	switch(it->twirlmethod)
		{
		case twirlCurveMethod:
			cosparam = theta * it->spines + it->twirlbase
						+ (dist * (it->twirlspeed + (dist * it->twirlamp)));
			break;
		case twirlSineMethod:
			cosparam = (theta * it->spines + it->twirlbase)
						+ (sin(dist * it->twirlspeed) * (it->twirlamp + (dist * it->twirlamp)));
			break;
		case twirlNoneMethod:
		default:
			//no twirl at all
			cosparam = theta * it->spines + it->twirlbase;
			break;
		}
	return chopsin(cosparam, it) * it->spineradius;
	}

static float calctheta(float h, float v, spinflake* it)
	{
	/*
	Calculate the angle this point presents to the origin of the spinflake.
	We assume that the point is relative to the origin of the spinflake.
	We do this, because that's how rawpoint is coded. :-) 
	*/
	return atan(v / h);
	}
