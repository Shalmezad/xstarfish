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


Galaxy Generator

This is a sort of cheap shot generator with an interesting effect. It uses
limiting randoms around a point to create a bursted, scattered, staticy
galaxy effect. It looks like a field of stars clustering in a brilliant core,
so bright you can't pick out the individual elements. The galaxy comes in
random sizes, and that's about the extent of it.

This is not an official part of the starfish canon because it is so predictable.
I like generators to be a little more flexible than this is.

*/

void* GalaxyInit(void);
void GalaxyExit(void* refcon);
float Galaxy(float h, float v, void* refcon);