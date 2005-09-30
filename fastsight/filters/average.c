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

#include <string.h>

static unsigned char *buf = 0;

void filter_average_apply(unsigned char *frame)
{
  
  unsigned char *p1 = frame, *p2 = buf;
  int i;
  
  if(!buf)
  {
    buf = (unsigned char *)malloc(320*240*3);
    memcpy(buf, frame, 320*240*3);
    return;
  }
  
  for(i = 0; i < 320*240; i++)
  {
    p1[0] = ((3*p2[0] + p1[0]) >> 2);
    p1[1] = ((3*p2[1] + p1[1]) >> 2);
    p1[2] = ((3*p2[2] + p1[2]) >> 2);
    p1+=3; p2+=3;
  }
  
  memcpy(buf, frame, 320*240*3);
  
}
