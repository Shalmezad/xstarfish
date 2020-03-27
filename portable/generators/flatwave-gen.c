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

Flatwave - produces linear waves at arbitrary angles. Like Coswave, but
produces linear ("flat") waves instead of waves oriented around a point.
The flatwave module will eventually generate several waves at a time,
interfering with each other and making interesting effects.

*/

#include <math.h>
#include <stdlib.h>
#include "genutils.h"
#include "flatwave-gen.h"

enum interference_methods
	{
	intfMostExtreme,
	intfLeastExtreme,
	intfMax,
	intfMin,
	intfAverage,
	MAX_INTERFERENCE_METHODS
	};
	
enum accel_methods
	{
	accelNone,
	accelWave,
	MAX_ACCEL_METHODS
	};

/*
A wave is a curve on a line.
Each wave may have different scaling
and display packing options.
*/
typedef struct WaveRec
	{
	float scale;
	int packmethod;
	int accelmethod;
	float accelscale;
	float accelamp;
	int accelpack;
	}
WaveRec;

/*
A wavepacket is a group of waves on the same line.
A wavepacket has an origin and an angle. All waves
in the packet are calculated relative to that line.
*/
typedef struct WavePacketRec
	{
	float originH, originV;
	float angle;
	WaveRec wave;
	}
WavePacketRec;

/*
The FlatwaveRec contains all the information
about a flatwave layer. This equals a list of
wavepackets and a description of the way to
interfere them with each other.
*/
#define MAX_WAVE_PACKETS 3
typedef struct FlatwaveRec
	{
	int packets;
	int interferencemethod;
	WavePacketRec packet[MAX_WAVE_PACKETS + 1];
	}
FlatwaveRec;

void InitWavePacket(WavePacketRec* it);
void InitWave(WaveRec* it);
float CalcWavePacket(float h, float v, WavePacketRec* it);
float CalcWave(float distance, float transverse, WaveRec* it);

void* FlatwaveInit(void)
	{
	/*
	All of the information we use to create an image lives in a FlatwaveRec.
	Allocate one such record and fill in appropriate random values.
	*/
	FlatwaveRec* out = (FlatwaveRec*)malloc(sizeof(FlatwaveRec));
	if(out)
		{
		int ctr;
		//Pick a random number of packets, from 1 through MAX_WAVE_PACKETS.
		out->packets = irand(MAX_WAVE_PACKETS) + 1;
		out->interferencemethod = (rand() * MAX_INTERFERENCE_METHODS) / RAND_MAX;
		for(ctr = 0; ctr <= out->packets; ctr++)
			{
			InitWavePacket(&out->packet[ctr]);
			}
		}
	return out;
	}

void InitWavePacket(WavePacketRec* it)
	{
	/*
	Pick a location and angle for this wave packet.
	Then create a wave.
	*/
	it->originH = frand(1.0);
	it->originV = frand(1.0);
	it->angle = frand(pi);
	InitWave(&it->wave);
	}

void InitWave(WaveRec* it)
	{
	/*
	Set up this wave.
	Pick a scaling factor and display packing method.
	*/
	it->scale = frandge(2.0, 30.0);
	it->packmethod = RandomPackMethod();
	if(it->packmethod == scaleToFit) it->scale *= 2.0;
	it->accelmethod = irand(MAX_ACCEL_METHODS);
	switch(it->accelmethod)
		{
		case accelNone:
			//No setup to do. Exit.
			break;
		case accelWave:
			//Make a sine wave to squiggle this one sideways.
			it->accelscale = frandge(2.0, 30.0);
			it->accelamp = frand(0.1);
			it->accelpack = RandomPackMethod();
			break;
		}
	}

void FlatwaveExit(void* refcon)
	{
	if(refcon) free(refcon);
	}

float Flatwave(float h, float v, void* refcon)
	{
	/*
	Turn the angle from the origin to this point into a right triangle.
	Compute the legs of this triangle. We will use these legs to determine
	where on the linear wave this point happens to fall.
	*/
	float hypangle, hypotenuse;
	float distance, transverse;
	float out = 0.5;
	FlatwaveRec* glb = (FlatwaveRec*)refcon;
	if(glb)
		{
		int ctr;
		float layer;
		switch(glb->interferencemethod)
			{
			case intfMostExtreme:
				out = 0.5;
				break;
			case intfLeastExtreme:
				out = 0;
				break;
			case intfMax:
				out = 0;
				break;
			case intfMin:
				out = 1;
				break;
			case intfAverage:
				out = 0;
				break;
			}
		for(ctr = 0; ctr <= glb->packets; ctr++)
			{
			layer = CalcWavePacket(h, v, &glb->packet[ctr]);
			if(glb->packets > 1)
				{
				switch(glb->interferencemethod)
					{
					case intfMostExtreme:
						/*
						Is this value's distance from 0.5 greater than the existing
						value's distance from 0.5?
						*/
						if(fabs(layer - 0.5) > fabs(out - 0.5)) out = layer;
						break;
					case intfLeastExtreme:
						/*
						Is this value closer to the median than the existing value?
						*/
						if(fabs(layer - 0.5) < fabs(out - 0.5)) out = layer;
						break;
					case intfMax:
						//Is this value closer to 1 than the existing value?
						if(layer > out) out = layer;
						break;
					case intfMin:
						//Is this value closer to zero than the existing one was?
						if(out > layer) out = layer;
						break;
					case intfAverage:
						//Sum all the values up and compute the average at the end.
						out += layer;
						break;
					default:
						//Beats me what to do with this case. It should never happen.
						out = layer;
					}
				}
			else out = layer;
			}
		//If we are in average mode, do the averaging now.
		if(glb->interferencemethod == intfAverage) out /= glb->packets;
		}
	return out;
	}

float CalcWavePacket(float h, float v, WavePacketRec* it)
	{
	/*
	Calculate the value returned by this wave packet.
	We find the origin of the wave and determine how far away and
	at what angle this point lies from that origin.
	Then we feed the distance & traverse values we get into each
	wave. We combine the results with any of several interference schemes.
	*/
	float hypangle, hypotenuse;
	float distance, transverse;
	float out = 0.5;
	//Re-centre the point on our wave's origin.
	h -= it->originH;
	v -= it->originV;
	//Now figure the length from the origin to this point.
	hypotenuse = hypotf(h, v);
	//Find the angle of the line from this point to the origin.
	hypangle = atan(v / h) + it->angle;
	if(h < 0) hypangle += pi;
	//Using the angle and the hypotenuse, we can figure out the individual legs.
	transverse = (cos(hypangle) * hypotenuse);
	distance = (sin(hypangle) * hypotenuse);
	//Our return value, for now, is just the value of our wave.
	out = CalcWave(distance, transverse, &it->wave);
	return out;
	}

float CalcWave(float distance, float transverse, WaveRec* it)
	{
	/*
	We have a distance and a transverse value for this wave.
	Use them to calculate the value of the wave at this point.
	Then pack the results to fit in the 0..1 allowed output scale.
	*/
	float out;
	switch(it->accelmethod)
		{
		case accelNone:
			//No acceleration. Do nothing.
			break;
		case accelWave:
			distance += (PackedCos(transverse, it->accelscale, it->accelpack) * it->accelamp);
			break;
		}
	out = PackedCos(distance, it->scale, it->packmethod);
	return out;
	}