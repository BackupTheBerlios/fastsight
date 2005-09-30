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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfgmanager.h"

static char cfgmanager_file[256];

int cfgmanager_init()
{
  char *home = getenv("HOME");
  FILE *fp;
  if(home[strlen(home)-1] == '/')
    home[strlen(home)-1] = 0;
  sprintf(cfgmanager_file, "%s/.fastsight", home);
  if(!(fp = fopen(cfgmanager_file, "r")))
  {
    if(!(fp = fopen(cfgmanager_file, "w")))
      return 0;
  }
  fclose(fp);
  return 1;
}

char *cfgmanager_get_value(char *name)
{
  static char buf[256];
  char *n, *v;
  FILE *fp = fopen(cfgmanager_file, "r");
  while(!feof(fp))
  {
    buf[0] = 0;
    fgets(buf, 256, fp);
    if(!buf[0])
      continue;
    n = strtok(buf, "=");
    if(!strcmp(n, name))
    {
      v = strtok(0, "\n");
      fclose(fp);
      return v;
    }
  }
  fclose(fp);
  return 0;
}

void cfgmanager_set_value(char *name, char *value)
{
  char buf[256], tmp[256], *n, *v, chg=0;
  FILE *fp, *fpo;
  sprintf(tmp, "%s.tmp", cfgmanager_file);
  fpo = fopen(tmp, "w");
  fp = fopen(cfgmanager_file, "r");
  while(!feof(fp))
  {
    buf[0] = 0;
    fgets(buf, 256, fp);
    if(!buf[0])
      continue;
    n = strtok(buf, "=");
    if(!strcmp(n, name))
    {
      v = value;
      chg = 1;
    } else {
      v = strtok(0, "\n");
    }
    sprintf(buf, "%s=%s\n", n, v);
    fputs(buf, fpo);
  }
  if(!chg)
  {
    sprintf(buf, "%s=%s\n", name, value);
    fputs(buf, fpo);
  }
  fclose(fp);
  fclose(fpo);
  remove(cfgmanager_file);
  rename(tmp, cfgmanager_file);
}
