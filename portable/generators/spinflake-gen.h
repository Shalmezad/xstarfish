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

Snowflake Generator. Embosses rotationally symmetrical shapes.
Mad ppatter! 1.6 introduced an attempt at creating crystalline,
pseudo-organic leaf/snowflake like shapes. It was all vector based,
not terribly smooth, and (in my opinion) did not work all that well.
This takes a graphics algorithm I used in an even older program (Flakie, 
which generated snowflake-like shapes) and adds a third dimension for
even more interesting effect.

The basic idea is that the generator picks an origin point, a radius,
and parameters for a sine wave. It then lays the wave on top of the
circle for a doily/snowflake/wave effect.

This gives us a curve. For every point in the domain, we extend
a ray from the origin, through the point, until we hit the curve.
We then calculate the distance from the origin and the distance from
the origin to the curve, along that ray. The Z value is the distance from
the point to the curve, scaled proportionally to the distance from the curve
to the origin. If the point is outside the curve, the value is
1 - (1 / (1 - the distance)), which makes the value start at zero and grow
slowly to infinity.

*/

void* SpinflakeInit(void);
void SpinflakeExit(void* refcon);
float Spinflake(float h, float v, void* refcon);