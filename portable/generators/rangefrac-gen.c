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


Rangefractal Generator

Creates a range fractal. It creates a starting matrix of random values,
then interpolates using existing values as min/max points for
intermediate random values. The results can look like mountains,
clouds, clusters of vegetation, and other turbulent mixtures of two
materials.

*/

#include "rangefrac-gen.h"
#include "genutils.h"
#include <stdlib.h>
#include <math.h>

/*
Our points are stored in a big matrix.
When the caller asks for a data point, we calculate it
based on the stuff in the matrix.
*/

/*
The scale determines how many data points we calculate.
The more data points, the tighter the resolution, and the
larger the quantity of memory consumed.
The size must be an even power of 2 in order to work 
properly, so we calculate it in terms of SCALE.
SCALE should be a value 3..10.
*/
#define VALMATRIX_SCALE 8
#define VALMATRIX_SIZE (1<<VALMATRIX_SCALE)
#define VALMATRIX_MAX (VALMATRIX_SIZE - 1)

typedef struct RangefracGlobals
	{
	float data[VALMATRIX_SIZE][VALMATRIX_SIZE];
	int level[VALMATRIX_SIZE][VALMATRIX_SIZE];
	}
RangefracGlobals;

void ClearMatrix(RangefracGlobals* out);
void GenerateFractal(RangefracGlobals* out);
float CalcDistance(int matrixh, int matrixv, float desth, float destv);
float CalcWeight(int matrixh, int matrixv, float desth, float destv);
float GetMatrixVal(int matrixh, int matrixv, RangefracGlobals* glb);
int WrapH(int coord);
int WrapV(int coord);

void* RangefracInit(void)
	{
	/*
	Create a globals record which will store all of our
	persistent data.
	We also fill it out here, because I think that is a quick
	operation. If it turns out not to be quick, we will fill it
	progressively as it is requested.
	*/
	RangefracGlobals* out;
	int tempblinder;
	out = (RangefracGlobals*)malloc(sizeof(RangefracGlobals));
	if(out)
		{
		ClearMatrix(out);
		GenerateFractal(out);
		}
	return out;
	}

void RangefracExit(void* refcon)
	{
	if(refcon) free(refcon);
	}

float Rangefrac(float h, float v, void* refcon)
	{
	/*
	Locate the closest values to this one in the value
	array. Then use a proportional average based on distance
	to get the returned value.
	*/
	int smallH, smallV, bigH, bigV;
	float tweaker;
	float totalweight, totalsum;
	float localval, localweight;
	float out = 0.0;
	RangefracGlobals* glb = (RangefracGlobals*)refcon;
	if(glb)
		{
		/*
		Get each known value near the one we have been requested to retrieve.
		Calculate the distance from the requested point to each known point.
		Use the distance as a weight in an average.
		This essentially scales a small pixel map into a large one, using linear
		interpolation. It could be generalized with a little work.
		*/
		totalweight = 0;
		totalsum = 0;
		tweaker = 0.5 / VALMATRIX_SIZE;
		smallH = floor(h * VALMATRIX_SIZE - tweaker);
		smallV = floor(v * VALMATRIX_SIZE - tweaker);
		bigH = smallH + 1;
		bigV = smallV + 1;
		//TOPLEFT
		localval = GetMatrixVal(smallH, smallV, glb);
		localweight = CalcWeight(smallH, smallV, h, v);
		totalsum += (localval * localweight);
		totalweight += localweight;
		//TOPRIGHT
		localval = GetMatrixVal(bigH, smallV, glb);
		localweight = CalcWeight(bigH, smallV, h, v);
		totalsum += (localval * localweight);
		totalweight += localweight;
		//BOTLEFT
		localval = GetMatrixVal(smallH, bigV, glb);
		localweight = CalcWeight(smallH, bigV, h, v);
		totalsum += (localval * localweight);
		totalweight += localweight;
		//BOTRIGHT
		localval = GetMatrixVal(bigH, bigV, glb);
		localweight = CalcWeight(bigH, bigV, h, v);
		totalsum += (localval * localweight);
		totalweight += localweight;
		//TAKE AVERAGE
		out = totalsum / totalweight;
		}
	return out;
	}

void ClearMatrix(RangefracGlobals* out)
	{
	/*
	Clear out all the flags in the matrix.
	This makes sure we don't accidentally use garbage data.
	*/
	int h, v;
	for(v = 0; v < VALMATRIX_SIZE; v++)
		{
		for(h = 0; h < VALMATRIX_SIZE; h++)
			{
			out->level[h][v] = 0;
			}
		}
	}

void GenerateFractal(RangefracGlobals* out)
	{
	/*
	Walk through the matrix.
	For each point, search its neighbors. For each neighboring point
	of higher level than current, compare its value against the current
	min and max. If the neighboring point exceeds min or max, use its
	value as the new min or max. Repeat.
	*/
	int h, v;
	int step;
	for(step = VALMATRIX_SIZE / 2; step > 0; step /= 2)
		{
		for(v = 0; v < VALMATRIX_SIZE; v += step)
			{
			for(h = 0; h < VALMATRIX_SIZE; h += step)
				{
				float max, min, val;
				//See if we need to calculate this pixel at all.
				if(out->level[h][v] < step)
					{
					//Go hunting for the highest and lowest values among this pixel's neighbors.
					max = 0.0;
					min = 1.0;
					//Top left
					if(out->level[WrapH(h - step)][WrapV(v - step)] > step)
						{
						val = out->data[WrapH(h - step)][WrapV(v - step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Top
					if(out->level[h][WrapV(v - step)] > step)
						{
						val = out->data[h][WrapV(v - step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Top right
					if(out->level[WrapH(h + step)][WrapV(v - step)] > step)
						{
						val = out->data[WrapH(h + step)][WrapV(v - step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Left
					if(out->level[WrapH(h - step)][v] > step)
						{
						val = out->data[WrapH(h - step)][v];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Right
					if(out->level[WrapH(h + step)][v] > step)
						{
						val = out->data[WrapH(h + step)][v];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Bottom left
					if(out->level[WrapH(h - step)][WrapV(v + step)] > step)
						{
						val = out->data[WrapH(h - step)][WrapV(v + step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Bottom
					if(out->level[h][WrapV(v + step)] > step)
						{
						val = out->data[h][WrapV(v + step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					//Bottom right
					if(out->level[WrapH(h + step)][WrapV(v + step)] > step)
						{
						val = out->data[WrapH(h + step)][WrapV(v + step)];
						if(val < min) min = val;
						if(val > max) max = val;
						}
					val = frand(max - min) + min;
					if(step >= VALMATRIX_SIZE / 2)
						{
						/*
						The first pieces of data are always picked completely at random,
						because they have no neighbors to influence their decisions.
						But these data are the extremes of the image - no values can be
						any larger or smaller than them. So we "push" them out a little
						bit by rounding them to integer values, then averaging them with
						their original values. This gives us whiter whites and blacker
						blacks, without forcing the first data to be pure white or black.
						*/
						int valint;
						valint = (val > 0.5) ? 1 : 0;
						val = (valint + val) / 2.0;
						}
					out->data[h][v] = val;
					out->level[h][v] = step;
					}
				}
			}
		}
	}

float CalcDistance(int matrixh, int matrixv, float desth, float destv)
	{
	return hypotf(matrixh - (desth * VALMATRIX_SIZE),
		    matrixv - (destv * VALMATRIX_SIZE));
	}

float CalcWeight(int matrixh, int matrixv, float desth, float destv)
	{
	float out;
	out = 1 - CalcDistance(matrixh, matrixv, desth, destv);
	if(out < 0.0) out = 0.0;
	return out;
	}

float GetMatrixVal(int matrixh, int matrixv, RangefracGlobals* glb)
	{
	return glb->data[WrapH(matrixh)][WrapV(matrixv)];
	}

int WrapH(int coord)
	{
	while(coord > VALMATRIX_MAX) coord -= VALMATRIX_SIZE;
	while(coord < 0) coord += VALMATRIX_SIZE;
	return coord;
	}

int WrapV(int coord)
	{
	while(coord > VALMATRIX_MAX) coord -= VALMATRIX_SIZE;
	while(coord < 0) coord += VALMATRIX_SIZE;
	return coord;
	}
