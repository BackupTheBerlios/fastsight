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
#include "filters.h"

static void *filter_funcs[] =
{
  filter_average_apply,
  filter_range_apply,
  filter_grayscale_apply,  
  0
};

#define n_filters ((sizeof(filter_funcs)/sizeof(void *))-1)

static char filter_enabled[n_filters+1];

int filters_init()
{
  memset(filter_enabled, 0, n_filters+1);
  return 1;
}

void filters_apply(unsigned char *frame)
{
  int i;
  void (*apply_func)(unsigned char *frame);
  for(i = 0; filter_funcs[i]; i++)
  {
    if(filter_enabled[i])
    {
      apply_func = filter_funcs[i];
      apply_func(frame);
    }
  }
}

void filters_set_enabled(void *filter_func, int enabled)
{
  int i;
  for(i = 0; filter_funcs[i]; i++)
  {
    if(filter_funcs[i] == filter_func)
    {
      filter_enabled[i] = enabled;
      break;
    }
  }
}

