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
 
#ifndef _FILTERS_H
#define _FILTERS_H

#ifdef __cplusplus
extern "C"
{
#endif

int filters_init();
void filters_apply(unsigned char *frame);
void filters_set_enabled(void *filter_func, int enabled);

void filter_average_apply(unsigned char *frame);
void filter_range_apply(unsigned char *frame);
void filter_grayscale_apply(unsigned char *frame);
  
#ifdef __cplusplus
}
#endif
  
#endif
