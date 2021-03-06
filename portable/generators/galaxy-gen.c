/*

Copyright �1999 Mars Saxman
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


Galaxy Generator

This works, and makes convincing-looking, perfectly spherical galaxies.
It isn't too interesting or flexible, though. More of a novelty than
a useful Starfish engine.

*/

#include "galaxy-gen.h"
#include "genutils.h"
#include <stdlib.h>
#include <math.h>


typedef struct GalaxyGlobals
	{
	float rangemin;
	float rangemax;
	}
GalaxyGlobals;
typedef GalaxyGlobals* GalaxyRef;

void* GalaxyInit(void)
	{
	GalaxyRef out = (GalaxyRef)malloc(sizeof(GalaxyGlobals));
	if(out)
		{
		out->rangemin = frandge(0.0, 4.0);
		out->rangemax = frandge(1.0, 48.0);
		}
	return out;
	}

void GalaxyExit(void* refcon)
	{
	if(refcon) free(refcon);
	}

float Galaxy(float h, float v, void* refcon)
	{
	float dist;
	GalaxyRef it = (GalaxyRef)refcon;
	dist = hypot(h - 0.5, v - 0.5);
	return 1.0 - dist  * dist * frandge(it->rangemin, it->rangemax);
	}