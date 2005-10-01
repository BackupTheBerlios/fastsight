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
#include <string.h>
#include <windows.h>

#include "cfgmanager.h"

int cfgmanager_init()
{
  
  HKEY key;
  DWORD disp;
  
  RegCreateKeyEx(HKEY_CURRENT_USER,
    "Software\\fastsight", 0, "",
    REG_OPTION_NON_VOLATILE, KEY_READ|KEY_WRITE, 0,
    &key, &disp);
  
  RegCloseKey(key);
  
  return 1;
  
}

char *cfgmanager_get_value(char *name)
{
  
  HKEY key;
  DWORD type, len=256;
  static char buf[256];
  
  RegOpenKeyEx(HKEY_CURRENT_USER,
    "Software\\fastsight",
    0, KEY_READ, &key);
  
  if(RegQueryValueEx(key, name, 0, &type, buf,
    &len) != ERROR_SUCCESS)
  {
    RegCloseKey(key);
    return 0;
  }
  
  RegCloseKey(key);
  return buf;
  
}

void cfgmanager_set_value(char *name, char *value)
{
  
  HKEY key;
  
  RegOpenKeyEx(HKEY_CURRENT_USER,
    "Software\\fastsight",
    0, KEY_READ|KEY_WRITE, &key);
  
  RegSetValueEx(key, name, 0, REG_SZ, (BYTE *)value,
    strlen(value));
  
  RegCloseKey(key);
  
}
