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


Generator Utilities

A few small gadgets of use to the generator libraries.

*/

#ifndef __GENUTILS__
#define __GENUTILS__ 0

#define pi 3.141592653589

float frand(float range);
float frandge(float min, float max);
int irand(int range);
int irandge(int min, int max);
int maybe(void);

enum packmethods
	{
	scaleToFit,
	flipSignToFit,
	truncateToFit,
	slopeToFit,
	PACK_METHOD_COUNT
	};
int RandomPackMethod(void);
float PackedCos(float distance, float scale, int packmethod);

#endif		//__GENUTILS__