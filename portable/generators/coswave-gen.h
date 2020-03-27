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

*/

void* CoswaveInit(void);
void CoswaveExit(void* refcon);
float Coswave(float h, float v, void* refcon);