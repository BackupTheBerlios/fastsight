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
#include <windows.h>
#include <winsock.h>
#include "get_ip.h"

char *get_ip()
{
  
  struct hostent *host;
	char localhost[129];
	long unsigned add;
	static char ip[46];
  int i;
  
  if(gethostname(localhost, 128) < 0)
		return "localhost";
  
  if((host = gethostbyname(localhost)) == NULL)
    return "localhost";
  
  for(i = 0; host->h_addr_list[i]; i++)
  {
    memcpy(&add, host->h_addr_list[0], 4);
    add = htonl(add);
    sprintf(ip, "%lu.%lu.%lu.%lu",
          ((add >> 24) & 255),
          ((add >> 16) & 255),
          ((add >>  8) & 255),
          add & 255);
    if(strcmp(ip, "127.0.0.1"))
      break;
  }
  
	return ip;
  
}
