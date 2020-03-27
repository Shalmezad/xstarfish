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


A collection of error codes used throughout the starfish pixel routines

*/

#ifndef __rasterliberrs__
#define __rasterliberrs__ 0

enum starfishrasterliberrs
	{
	srl_noErr = 0,
	srl_outOfBounds,
	srl_bogusBuffer,
	srl_bogusParamPtr,
	srl_mismatchedSizes,
	srl_bollixed		//data went inconsistent internally
	};
typedef enum starfishrasterliberrs srl_result;

#endif //__rasterliberrs__