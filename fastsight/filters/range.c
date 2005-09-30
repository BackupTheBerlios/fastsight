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

/* Based in the range filter from sdlcam
 * http://sdlcam.raphnet.net/
 */

#include <string.h>

#define RANGE_IGNORE	300

static unsigned char pallete_r[256];
static unsigned char pallete_g[256];
static unsigned char pallete_b[256];

void filter_range_apply(unsigned char *frame)
{
  
  unsigned int Level_R[256];
  unsigned int Level_G[256];
  unsigned int Level_B[256];
  unsigned int Level_med, Level_avg = 0;
	unsigned int lower_r=255, higher_r=0,range_r;
	unsigned int lower_g=255, higher_g=0,range_g;
	unsigned int lower_b=255, higher_b=0,range_b;
	unsigned int summe;
  unsigned char *p;
  int i;
  
  memset(Level_R, 0, sizeof(Level_R));
  memset(Level_G, 0, sizeof(Level_G));
  memset(Level_B, 0, sizeof(Level_B));
  
  p = frame;
  for(i = 0; i < 320*240; i++)
  {
    Level_R[*p++]++;
    Level_G[*p++]++;
    Level_B[*p++]++;
  }
  
  Level_med = 0;
  i = 255;
  do
  {
	  Level_avg += Level_R[i];
	  Level_avg += Level_G[i];
	  Level_avg += Level_B[i];
    Level_med += (Level_R[i]+Level_G[i]+Level_B[i])*i;
  } while(--i);
  
  if(Level_avg != 0)
    Level_med /= Level_avg;
  
	/* Get the Minimum for R G B */
	for(summe=0,lower_r =0;lower_r  < 255 && summe < RANGE_IGNORE ;lower_r ++) summe+=Level_R[lower_r ];
	for(summe=0,lower_g =0;lower_g  < 255 && summe < RANGE_IGNORE ;lower_g ++) summe+=Level_G[lower_g ];
	for(summe=0,lower_b =0;lower_b  < 255 && summe < RANGE_IGNORE ;lower_b ++) summe+=Level_B[lower_b ];
	/* Get the Maximum for R G B but not lower than Minimum */
	for(summe=0,higher_r=255;higher_r > lower_r && summe < RANGE_IGNORE ;higher_r--) summe+=Level_R[higher_r];
	for(summe=0,higher_g=255;higher_g > lower_g && summe < RANGE_IGNORE ;higher_g--) summe+=Level_G[higher_g];
	for(summe=0,higher_b=255;higher_b > lower_b && summe < RANGE_IGNORE ;higher_b--) summe+=Level_B[higher_b];
	/* Since higher_r >= lower_r minimum is 0 */
	range_r   = higher_r - lower_r;
	range_g   = higher_g - lower_g;
	range_b   = higher_b - lower_b;
	Level_med = Level_med - (lower_r+lower_g+lower_b)/3;
  
	/* only range correction if range > 0 , else only shifting */
	if (range_r) for(i=0;i<256;i++) pallete_r[i]=(i<=lower_r)?0:((i>=higher_r)?255:((i-lower_r)<<8)/range_r);
	else 	       for(i=0;i<256;i++)	pallete_r[i]=(i<=lower_r)?0:((i>=higher_r)?255: (i-lower_r));
	if (range_g) for(i=0;i<256;i++) pallete_g[i]=(i<=lower_g)?0:((i>=higher_g)?255:((i-lower_g)<<8)/range_g);
	else 	       for(i=0;i<256;i++)	pallete_g[i]=(i<=lower_g)?0:((i>=higher_g)?255: (i-lower_g));
	if (range_b) for(i=0;i<256;i++) pallete_b[i]=(i<=lower_b)?0:((i>=higher_b)?255:((i-lower_b)<<8)/range_b);
	else	       for(i=0;i<256;i++) pallete_b[i]=(i<=lower_b)?0:((i>=higher_b)?255: (i-lower_b));

	p = frame;
	for(i = 0; i < 320*240; i++)
  {
		p[0] = pallete_r[p[0]];
		p[1] = pallete_g[p[1]];
		p[2] = pallete_b[p[2]];
		p += 3;
  }
  
}
