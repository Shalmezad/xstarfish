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

 struct display_info_struct
 {
     Display *display;
     int screen;
     int depth;
     int bpp;
     int width;
     int height;
     Window rootwin;
     GC gc;
     XImage *image;
 };
typedef struct display_info_struct display_info;

int compose(int i, int shift)
{
  return (shift<0) ? (i>>(-shift)) : (i<<shift);
}

void fillimage(StarfishRef tex, display_info *di, int xzoom, int yzoom)
{
  int x,y;
  int dx,dy,i;
  unsigned long value;
  int redshift,greenshift,blueshift;
  pixel pixel, oldpix;
  float xstep,ystep,pixr,pixg,pixb,dr,dg,db;

  x=di->image->red_mask; redshift=-8;
  while(x) { x/=2; redshift++; }
  x=di->image->green_mask; greenshift=-8;
  while(x) { x/=2; greenshift++; }
  x=di->image->blue_mask; blueshift=-8;
  while(x) { x/=2; blueshift++; }

  dy=0;
  xstep=1.0/xzoom;
  ystep=1.0/yzoom;
  for (y=0; y<di->height; y++)
  {
    dx=0;
    for (x=0; x<di->width; x++)
    {
      GetStarfishPixel(x, y, tex, &pixel);
      value  = compose(pixel.red,redshift) & di->image->red_mask;
      value += compose(pixel.green,greenshift) & di->image->green_mask;
      value += compose(pixel.blue,blueshift) & di->image->blue_mask;
      // we want the end of the zoomed area to get the pixel...
      XPutPixel(di->image,dx+xzoom-1,dy,value);
      // so that we can generate the interpolated pixels to it's left.
      if(xzoom>1) {
        if(x==0) GetStarfishPixel(di->width-1, y, tex, &oldpix);
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
          value  = compose(pixr,redshift) & di->image->red_mask;
          value += compose(pixg,greenshift) & di->image->green_mask;
          value += compose(pixb,blueshift) & di->image->blue_mask;
          XPutPixel(di->image,dx+i,dy,value);
        }
        memcpy(&oldpix, &pixel, sizeof(pixel));
      }
      dx+=xzoom;
    }
    if(y>0) {
      do {
        for(dx=0; dx<(di->width*xzoom); dx++) {
          if(y==di->height) value=XGetPixel(di->image,dx,0);
          else value=XGetPixel(di->image,dx,dy);
          pixr=(value&di->image->red_mask)>>redshift;
          pixg=(value&di->image->green_mask)>>greenshift;
          pixb=(value&di->image->blue_mask)>>blueshift;
          value=XGetPixel(di->image,dx,dy-yzoom);
          dr=(value&di->image->red_mask)>>redshift;
          dg=(value&di->image->green_mask)>>greenshift;
          db=(value&di->image->blue_mask)>>blueshift;
          dr=(pixr-dr)*ystep;
          dg=(pixg-dg)*ystep;
          db=(pixb-db)*ystep;
          for(i=1; i<=yzoom; i++) {
            pixr-=dr;
            pixg-=dg;
            pixb-=db;
            value  = compose(pixr,redshift) & di->image->red_mask;
            value += compose(pixg,greenshift) & di->image->green_mask;
            value += compose(pixb,blueshift) & di->image->blue_mask;
            XPutPixel(di->image,dx,dy-i,value);
          }
        }
      } while(y==di->height-1 && y++ && (dy+=yzoom));
    }
    dy+=yzoom;
  }
}

void XSetWindowBackgroundImage(display_info *di)
{
  Pixmap out;
  if (out = XCreatePixmap(di->display, di->rootwin,
                          di->image->width, di->image->height,
                          di->depth))
  {
    XPutImage(di->display, out, di->gc, di->image, 0,0, 0,0,
              di->image->width, di->image->height);
    XSetWindowBackgroundPixmap(di->display, di->rootwin, out);
    XFreePixmap(di->display, out);
    //Force the entire window to redraw itself. This shows our pixmap.
    XClearWindow(di->display, di->rootwin);
  }
}

void mainloop(StarfishRef tex, display_info *displays, int xzoom, int yzoom)
{
  char *buf;
  int bpl;
  int n_pmf;
  int i;
  XPixmapFormatValues * pmf;
  display_info *di_counter;
  int j = 0;

  for(di_counter = &displays[j]; di_counter->display != 0;
      j++, di_counter = &displays[j])
  {
      pmf = XListPixmapFormats (di_counter->display, &n_pmf);
      if (pmf)
      {
          for (i = 0; i < n_pmf; i++)
          {
              if (pmf[i].depth == di_counter->depth)
              {
                  int pad, pad_bytes;
                  di_counter->bpp = pmf[i].bits_per_pixel;
                  bpl = di_counter->width * xzoom * di_counter->bpp / 8;
                  pad = pmf[i].scanline_pad;
                  pad_bytes = pad / 8;
                  /* make bpl a whole multiple of pad/8 */
                  bpl = (bpl + pad_bytes - 1) & ~(pad_bytes - 1);
                  buf=malloc(di_counter->height*bpl*yzoom);
                  di_counter->image = XCreateImage(
                      di_counter->display,
                      DefaultVisual(di_counter->display,
                                    di_counter->screen),
                      di_counter->depth, ZPixmap,0,buf,
                      di_counter->width*xzoom,
                      di_counter->height*yzoom, pad,bpl);
                  if(!di_counter->image)
                  {
                      puts("xstarfish: XCreateImage failed");
                      return;
                  }
                  break;
              }
          }
          XFree ((char *) pmf);
      }

      if (! XInitImage(di_counter->image))
          return;
      fillimage(tex, di_counter, xzoom, yzoom);
      XSetWindowBackgroundImage(di_counter);
      XDestroyImage(di_counter->image);
  }
}

void SetXDesktop(StarfishRef tex, const char* displayname,
                 int xzoom, int yzoom)
{
  display_info *displays;
  Display *display;
  int screen_count;
  int i;
  if (! (display = XOpenDisplay(displayname)))
  {
    fprintf(stderr, "xstarfish: Failed to open display\n");
    return;
  }
  screen_count = ScreenCount(display);
  displays = malloc(sizeof(display_info) * (screen_count + 1));

  displays[screen_count].display = 0;

  for(i = 0; i < screen_count; i++)
  {
      displays[i].display = display;
      displays[i].image = 0;
      displays[i].screen = i;
      displays[i].depth = DefaultDepth(displays[i].display,
                                       displays[i].screen);
      displays[i].rootwin = RootWindow(displays[i].display,
                                       displays[i].screen);
      displays[i].gc = DefaultGC(displays[i].display, displays[i].screen);
      displays[i].width = StarfishWidth(tex);
      displays[i].height = StarfishHeight(tex);
  }

  mainloop(tex, displays, xzoom, yzoom);
  XCloseDisplay(displays[0].display);
  return;
}
