/*

Starfish X desktop setter
Copyright (c) 2000 Mars Saxman
Portions derived from Zut 1.5 and are copyright (C) 1999 Sebastien Loisel
Thanks to Mr. Loisel for providing his code as free software.
Thanks to Adrian Bridgett for fixing many bugs.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include "starfish-engine.h"

Display *display;
int screen,depth,bpp,width,height;
Window rootwin;
GC gc;
XImage *image=0;

int compose(int i, int shift)
{
  return (shift<0) ? (i>>(-shift)) : (i<<shift); 
}

void fillimage(StarfishRef tex, int xzoom, int yzoom)
{
  int x,y;
  int dx,dy,i;
  unsigned long value;
  int redshift,greenshift,blueshift;
  pixel pixel, oldpix;
  float xstep,ystep,pixr,pixg,pixb,dr,dg,db;
     
  x=image->red_mask; redshift=-8;
  while(x) { x/=2; redshift++; }
  x=image->green_mask; greenshift=-8;
  while(x) { x/=2; greenshift++; }
  x=image->blue_mask; blueshift=-8;
  while(x) { x/=2; blueshift++; }
     
  dy=0;
  xstep=1.0/xzoom;
  ystep=1.0/yzoom;
  for (y=0; y<height; y++)
  {
    dx=0;
    for (x=0; x<width; x++)
    {
      GetStarfishPixel(x, y, tex, &pixel);
      value  = compose(pixel.red,redshift) & image->red_mask;
      value += compose(pixel.green,greenshift) & image->green_mask;
      value += compose(pixel.blue,blueshift) & image->blue_mask;
      // we want the end of the zoomed area to get the pixel...
      XPutPixel(image,dx+xzoom-1,dy,value);
      // so that we can generate the interpolated pixels to it's left.
      if(xzoom>1) {
        if(x==0) GetStarfishPixel(width-1, y, tex, &oldpix);
        dr=(pixel.red-oldpix.red)*xstep;
        dg=(pixel.green-oldpix.green)*xstep;
        db=(pixel.blue-oldpix.blue)*xstep;
        pixr=pixel.red;
        pixg=pixel.green;
        pixb=pixel.blue;
        for(i=xzoom-2; i>=0; i--) {
          pixr-=dr;
          pixg-=dg;
          pixb-=db;
          value  = compose(pixr,redshift) & image->red_mask;
          value += compose(pixg,greenshift) & image->green_mask;
          value += compose(pixb,blueshift) & image->blue_mask;
          XPutPixel(image,dx+i,dy,value);
        }
        memcpy(&oldpix, &pixel, sizeof(pixel));
      }
      dx+=xzoom;
    }
    if(y>0) {
      do {
        for(dx=0; dx<(width*xzoom); dx++) {
          if(y==height) value=XGetPixel(image,dx,0);
          else value=XGetPixel(image,dx,dy);
          pixr=(value&image->red_mask)>>redshift;
          pixg=(value&image->green_mask)>>greenshift;
          pixb=(value&image->blue_mask)>>blueshift;
          value=XGetPixel(image,dx,dy-yzoom);
          dr=(value&image->red_mask)>>redshift;
          dg=(value&image->green_mask)>>greenshift;
          db=(value&image->blue_mask)>>blueshift;
          dr=(pixr-dr)*ystep;
          dg=(pixg-dg)*ystep;
          db=(pixb-db)*ystep;
          for(i=1; i<=yzoom; i++) {
            pixr-=dr;
            pixg-=dg;
            pixb-=db;
            value  = compose(pixr,redshift) & image->red_mask;
            value += compose(pixg,greenshift) & image->green_mask;
            value += compose(pixb,blueshift) & image->blue_mask;
            XPutPixel(image,dx,dy-i,value);
          }
        }
      } while(y==height-1 && y++ && (dy+=yzoom));
    }
    dy+=yzoom;
  }
}

void XSetWindowBackgroundImage(Display* display, Drawable window, XImage* image)
{
  Pixmap out;
  if (out = XCreatePixmap(display, window, image->width, image->height, depth))
  {
    XPutImage(display, out, gc, image, 0,0, 0,0, image->width, image->height);
    XSetWindowBackgroundPixmap(display, window, out);
    XFreePixmap(display, out);
    //Force the entire window to redraw itself. This shows our pixmap.
    XClearWindow(display, window);
  }
}

void mainloop(StarfishRef tex, int xzoom, int yzoom)
{
  char *buf;
  int bpl;
  int n_pmf;
  int i;
  XPixmapFormatValues * pmf;   
     
  pmf = XListPixmapFormats (display, &n_pmf);
  if (pmf)
  {
    for (i = 0; i < n_pmf; i++)
    {
      if (pmf[i].depth == depth)
      {
        int pad, pad_bytes;
        bpp = pmf[i].bits_per_pixel;
	bpl = width * xzoom * bpp / 8;
	pad = pmf[i].scanline_pad;
	pad_bytes = pad / 8;
	/* make bpl a whole multiple of pad/8 */
	bpl = (bpl + pad_bytes - 1) & ~(pad_bytes - 1);
        buf=malloc(height*bpl*yzoom);
        image=XCreateImage(display,DefaultVisual(display,screen), depth,
       	                   ZPixmap,0,buf,width*xzoom,height*yzoom,pad,bpl);
        if(!image)
        {
          puts("xstarfish: XCreateImage failed");
          return;
        }
	break;
      }
    }
    XFree ((char *) pmf);
  }    

  if (! XInitImage(image))
    return;
  fillimage(tex, xzoom, yzoom);
  XSetWindowBackgroundImage(display, rootwin, image);
  XDestroyImage(image);
}

void SetXDesktop(StarfishRef tex, const char* displayname, int xzoom, int yzoom)
{
  if (! (display = XOpenDisplay(displayname)))
  {
    fprintf(stderr, "xstarfish: Failed to open display\n");
    return;
  }
  screen = DefaultScreen(display);
  depth = DefaultDepth(display, screen);
  rootwin = RootWindow(display, screen);
  gc=DefaultGC(display,screen);
  width = StarfishWidth(tex);
  height = StarfishHeight(tex);
  mainloop(tex, xzoom, yzoom);
  XCloseDisplay(display);
  return;
}
