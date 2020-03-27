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

void* FlatwaveInit(void);
void FlatwaveExit(void* refcon);
float Flatwave(float h, float v, void* refcon);