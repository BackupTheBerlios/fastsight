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

#ifndef _FL_VIDEOAREA_H
#define _FL_VIDEOAREA_H

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Shared_Image.H>

#ifdef USE_XSHM
#include <FL/x.H>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

class Fl_VideoArea : public Fl_Widget
{
  
#ifdef USE_XSHM
  XImage *ximage_;
  XShmSegmentInfo xshminfo_;
  unsigned xshmsize_;
#endif
  
  unsigned char *rgbimg_;

  void draw();
  
  public:

    Fl_VideoArea(int X, int Y, int W, int H, const char *L = 0);
    ~Fl_VideoArea();

  void rgbimg(unsigned char *p) { rgbimg_ = p; }
  unsigned char *rgbimg() { return rgbimg_; }
  
};

#endif
