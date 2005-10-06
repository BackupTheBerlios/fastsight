/*
 * fastsight
 * 
 * Copyright (c) 2005 Paulo Matias
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Fl_VideoArea.H"
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Fl_VideoArea::Fl_VideoArea(int X, int Y, int W, int H, const char *L)
  : Fl_Widget(X, Y, 320, 240, L)
{
#ifdef USE_XSHM
  ximage_ = (XImage *)0;
  xshmsize_ = 0;
#endif
  rgbimg_ = 0;
}

Fl_VideoArea::~Fl_VideoArea()
{
  
#ifdef USE_XSHM
  if(ximage_)
  {
    XDestroyImage(ximage_);
    ximage_ = (XImage *)0;
  }

  if(xshmsize_)
  {
    XShmDetach(fl_display, &xshminfo_);
    if(shmdt(xshminfo_.shmaddr))
      perror("shmdt failed");
    if(shmctl(xshminfo_.shmid, IPC_RMID, 0))
      perror("shmctl failed");
  }
#endif

}

void Fl_VideoArea::draw()
{
  
#ifdef USE_XSHM
  
  static bool	xshm_check = false;
  static bool	xshm_do_init;
  
  if(!xshm_check)
  {
    xshm_do_init = true;
    if(fl_visual->depth != 24 || fl_visual->red_mask != 0xff0000)
      xshm_do_init = false;
    else
      xshm_do_init = XShmQueryExtension(fl_display) != 0;
    xshm_check = true;
  }
  
  if(xshm_do_init)
  {
    if(!ximage_)
    {
      ximage_ = XShmCreateImage(fl_display, fl_visual->visual, fl_visual->depth,
        ZPixmap, 0, &xshminfo_, 320, 240);
      if((ximage_->bytes_per_line / ximage_->width) != 4)
      {
        XDestroyImage(ximage_);
        ximage_ = (XImage *)0;
        xshm_do_init = false;
      }
    }
    if(!xshmsize_ && ximage_)
    {
      memset(&xshminfo_, 0, sizeof(xshminfo_));
      xshmsize_ = ximage_->height * ximage_->bytes_per_line;
      xshminfo_.shmid   = shmget(IPC_PRIVATE, xshmsize_, IPC_CREAT | 0777);
      xshminfo_.shmaddr = ximage_->data = (char *)shmat(xshminfo_.shmid, 0, 0);
      xshminfo_.readOnly = False;
      XShmAttach(fl_display, &xshminfo_);
      xshm_do_init = false;
    }
  }
  
#endif

  if(rgbimg_)
  {
    
#ifdef USE_XSHM
    
    if(ximage_)
    {
      
      unsigned char	*src;
      unsigned *dst;
      int i;
      
      for(i=320*240,src=rgbimg_,dst=(unsigned *)ximage_->data;i>0;i--)
      {
        *dst++ = (((src[0] << 8) | src[1]) << 8) | src[2];
        src += 3;
      }
      
      XShmPutImage(fl_display, fl_window, fl_gc, ximage_, 0, 0, x(), y(),
        320, 240, False);
      
      XFlush(fl_display);
      
    }
    else
      
#endif
    
    {
      fl_draw_image(rgbimg_, x(), y(), 320, 240);
    }
    
  }
  else
  {
    fl_color(color());
    fl_rectf(x(), y(), 320, 240);
  }

}
