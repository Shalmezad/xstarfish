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


Bubble Generator

Makes a field of hemispheric bubbles. The bubbles have varying sizes,
alignments, and aspect ratios. Pixels are calculated by finding the
highest point on any intersecting bubble. The result looks something like
adding glycerine to a tub of water and blowing in it with a straw.

*/

#include "bubble-gen.h"
#include "genutils.h"
#include <stdlib.h>
#include <math.h>

#define MAX_BUBBLES 32

typedef struct BubbleData
	{
	float scale;			//by what factor should we shrink the influence of this bubble?
	float squish;			//we multiply the h by this and divide the v by it
	float angle;			//how far should we rotate this bubble's coordinate system?
	float h, v;				//coordinates for the origin of the bubble
	float boundL, boundT, boundR, boundB;	//approximate bounding box for the circle
	}
BubbleData;

typedef struct BubbleGlobals
	{
	int count;
	float scalemin, scalemax;
	float squishmin, squishmax;
	float anglemin, anglemax;
	BubbleData tip[MAX_BUBBLES];
	}
BubbleGlobals;
typedef BubbleGlobals* BubbleRef;

static float GetAllWrappedBubblesValue(float h, float v, BubbleRef it);
static float GetAllBubblesValue(float h, float v, BubbleRef it);
static float GetOneBubbleValue(float h, float v, BubbleData* bub);
static float GetSpunBubbleValue(float h, float v, BubbleData* bub);
static float GetSquishedBubbleValue(float h, float v, BubbleData* bub);
static float GetRawBubbleValue(float h, float v, BubbleData* bub);
static void InitBubble(BubbleData* bub, BubbleRef globals);
static void CalcBubbleBoundingBox(BubbleData* bub);

void* BubbleInit(void)
	{
	BubbleRef out = (BubbleRef)malloc(sizeof(BubbleGlobals));
	if(out)
		{
		int ctr;
		/*
		Pick a random number of bubbles. We have a compiled-in maximum
		number of bubbles - obviously we won't use more than that. I pseudo-
		arbitrarily chose max / 2 as the minimum, simply to see how it looked.
		Some other number less than MAX_BUBBLES could just as easily be chosen.
		The number should be small enough to be distinct from max, but large
		enough that many of the bubbles intersect each other - that's where
		the look comes from.
		*/
		out->count = irandge(MAX_BUBBLES / 4, MAX_BUBBLES);
		/*
		Pick a random minimum and maximum size. Based on empirical testing I've
		decided that 0.2 is the largest reasonable scale. Any bigger than that
		and single bubbles start to take over the entire scene. All bubble sizes
		will fall within the range we pick here. It doesn't matter if the "min"
		and "max" are actually reversed; frandge will take care of it.
		*/
		out->scalemin = frand(0.2);
		out->scalemax = frand(0.2);
		/*
		Pick random squish sizes. A squish of 1 means a perfect circle. Under 1
		means it becomes taller and narrower. Over 1 means it becomes wider and
		shorter. By setting a squishmin and squishmax for the entire bubblespace,
		we can control the "look" of the bubbles. If we want an entire space of
		tall, skinny bubbles, we can get it - or if we want one with no squishiness
		at all, we can get that too. Or we can let it be all over the map.
		Variety is a good thing, but too much of it is chaos. This is a way of
		"directing randomness" to get interesting variety.
		*/
		if(maybe())
			{
			out->squishmin = frandge(1.0, 4.0);
			if(maybe()) out->squishmin = 1.0 / out->squishmin;
			}
		else out->squishmin = 1.0;
		if(maybe())
			{
			out->squishmax = frandge(1.0, 4.0);
			if(maybe()) out->squishmax = 1.0 / out->squishmax;
			}
		else out->squishmax = 1.0;
		/*
		Some random angles. By rotating the bubbles' coordinate systems, we can make
		the squish factor turn. This makes the non-circular bubbles point in many
		strange directions. We do the same limited-random thing here as we have done
		elsewhere: the bubble-angles may be all over the map, or they may be cinched
		down into a certain direction similar to each other. This makes the field
		retain some consistency.
		Circular bubbles don't exhibit much appearance change when rotated.
		*/
		out->anglemin = frand(pi / 2.0);
		out->anglemax = frand(pi / 2.0);
		/*
		Now go through and create all of the bubbles using these data.
		*/
		for(ctr = 0; ctr < out->count; ctr++)
			{
			InitBubble(&out->tip[ctr], out);
			}
		}
	return out;
	}

void BubbleExit(void* refcon)
	{
	if(refcon) free(refcon);
	}

float Bubble(float h, float v, void* refcon)
	{
	/*
	Get the biggest value we can find out of all these bubbles.
	We will eventually do more interesting things with bubble clumps,
	points, and antibubbles, but this is just a beginning.
	*/
	BubbleRef it = (BubbleRef)refcon;
	return GetAllWrappedBubblesValue(h, v, it);
	}

static float GetAllWrappedBubblesValue(float h, float v, BubbleRef it)
	{
	/*
	Calculate nine values from the array of bubbles, corresponding to
	the main tile and each of its neighboring imaginary tiles.
	This lets the edges of the bubbles spill over and affect neighbouring
	tiles, creating the illusion of an infinitely tiled seamless space.
	We damp down the influence of neighbouring tiles proportionate to their
	distance from the edge of the main tile. This is to prevent really huge
	bubbles that cover multiple tiles from breaking the smooth edges.
	*/
	float current, best = 0;
	current = best = GetAllBubblesValue(h, v, it);
	current = GetAllBubblesValue(h + 1.0, v, it) * (1.0 - h);			//right
	if(current > best) best = current;
	current = GetAllBubblesValue(h - 1.0, v, it) * (h);					//left
	if(current > best) best = current;
	current = GetAllBubblesValue(h, v + 1.0, it) * (1.0 - v);			//bottom
	if(current > best) best = current;
	current = GetAllBubblesValue(h, v - 1.0, it) * (v);					//top
	if(current > best) best = current;
	current = GetAllBubblesValue(h + 1.0, v + 1.0, it) * (1.0 - h) * (1.0 - v);	//bottom right
	if(current > best) best = current;
	current = GetAllBubblesValue(h + 1.0, v - 1.0, it) * (1.0 - h) * (v); //top right
	if(current > best) best = current;
	current = GetAllBubblesValue(h - 1.0, v + 1.0, it) * (h) * (1.0 - v); //bottom left
	if(current > best) best = current;
	current = GetAllBubblesValue(h - 1.0, v - 1.0, it) * (h) * (v);		//bottom right
	if(current > best) best = current;
	return best;
	}

static float GetAllBubblesValue(float h, float v, BubbleRef it)
	{
	/*
	Get the biggest lump we can from this array of bubbles.
	We just scan through the list, compare the point with each bubble,
	and return the best match we can find.
	*/
	int ctr;
	float current, best;
	best = 0;
	for(ctr = 0; ctr < it->count; ctr++)
		{
		current = GetOneBubbleValue(h, v, &it->tip[ctr]);
		if(current > best) best = current;
		}
	return best;
	}

static float GetOneBubbleValue(float h, float v, BubbleData* bub)
	{
	return GetSpunBubbleValue(h, v, bub);
	}

static float GetSpunBubbleValue(float h, float v, BubbleData* bub)
	{
	/*
	Rotate the h and v values around the origin of the bubble according
	to the bubble's angle. Then pass the new h and v on to the squisher.
	*/
	float hypangle, hypotenuse;
	float distance, transverse;
	//Move the coordinates into bubble-relative coordinates.
	h -= bub->h;
	v -= bub->v;
	//Calculate the distance from the new origin to this point.
	hypotenuse = hypotf(h, v);
	/*
	Draw a line from the origin to this point. Get the angle this line
	forms with the horizontal. Then add the amount this bubble is rotated.
	*/
	hypangle = atan(v / h) + bub->angle;
	//The next line is magic. I don't quite understand it.
	if(h < 0) hypangle += pi;
	//We have the angle and the hypotenuse. Take the sine and cosine to get
	//the new horizontal and vertical distances in the new coordinate system.
	transverse = (cos(hypangle) * hypotenuse) + bub->h;
	distance = (sin(hypangle) * hypotenuse) + bub->h;
	//That's it. Pass in the transverse and distance values as the new h and v.
	return GetSquishedBubbleValue(transverse, distance, bub);
	}

static float GetSquishedBubbleValue(float h, float v, BubbleData* bub)
	{
	/*
	Perform the h, v compensation here. We multiply the h by the squish
	value and divide the v by it. So if squish is less than zero, the effect
	is reversed. Very simple little effect that gets non-spherical bubbles.
	*/
	h = bub->h + ((h - bub->h) * bub->squish);
	v = bub->v + ((v - bub->v) / bub->squish);
	return GetRawBubbleValue(h, v, bub);
	}

static float GetRawBubbleValue(float h, float v, BubbleData* bub)
	{
	/*
	Calculate the value of this point inside this bubble. If the point
	is outside the bubble, this will return a negative number. If the point
	is on the bubble's radius, this will return zero. Otherwise, this will return
	a number between zero and 1.
	*/
	float hypotenuse;
	hypotenuse = hypotf(h - bub->h, v - bub->v);
	return 1.0 - hypotenuse * hypotenuse / bub->scale;
	}

static void InitBubble(BubbleData* bub, BubbleRef globals)
	{
	/*
	Come up with some reasonable values for this bubble.
	The limits for these values are determined by certain fields in the globals.
	Limits on minimum and maximum size, squish, angle, and proximity are
	specified there. We just make sure the bubble fits within those values.
	*/
	/*
	There is no proximity limit yet.
	Bubbles can be positioned anywhere in the field.
	*/
	bub->h = frand(1.0);
	bub->v = frand(1.0);
	/*
	The bubble's scale must be in line with the scale limits for this field.
	This can force a bubble-scene to be uniform or allow it to be diverse.
	*/
	bub->scale = frandge(globals->scalemin, globals->scalemax);
	/*
	The bubble's squish factor has to fit the bubblefield, too. This is what
	determines its height-to-width aspect ratio.
	*/
	bub->squish = frandge(globals->squishmin, globals->squishmax);
	/*
	Each bubble needs an angle. This determines the rotation of its coordinate
	system around the bubble's origin, relative to the bubblefield. It's what
	makes squished bubbles point in different directions.
	*/
	bub->angle = frandge(globals->anglemin, globals->anglemax);
	/*
	We've set up all the bubble's information. Now give it a bounding box so we can
	do quicker hit tests.
	*/
	CalcBubbleBoundingBox(bub);
	}

static void CalcBubbleBoundingBox(BubbleData* bub)
	{
	/*
	Calculate the greatest and least coordinate values this bubble is able to hit in
	both horizontal and vertical axes. This is useful for hit-testing; we can quickly
	exclude circles that a given point doesn't hit.
	*/
	bub->boundL = bub->h - (bub->scale);
	bub->boundR = bub->h + (bub->scale);
	bub->boundT = bub->v - (bub->scale);
	bub->boundB = bub->v + (bub->scale);
	}
