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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/sockios.h>
#include <linux/if.h>

#include "get_ip.h"

static char *get_ip_by_iface(char *dev)
{
  int f;
  struct ifreq ifr;
  struct sockaddr_in sa;
  if((f = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    return 0;
  strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
  if(ioctl(f, SIOCGIFADDR, (char*)&ifr) == -1)
    return 0;
  memcpy(&sa, &ifr.ifr_ifru.ifru_addr, sizeof(struct sockaddr_in));
  close(f);
  return inet_ntoa(sa.sin_addr);
}

char *get_ip()
{

  char buf[256], *iface;
  FILE *fp;
  
  if(!(fp = fopen("/proc/net/route", "r")))
    return "localhost";
  
  fgets(buf, 256, fp);
  fgets(buf, 256, fp);
  
  while(!feof(fp))
  {
    iface = strtok(buf, " \t");
    strtok(NULL, " \t");
    strtok(NULL, " \t");
    if(atoi(strtok(NULL, " \t")) & 2)
    {
      fclose(fp);
      return get_ip_by_iface(iface);
    }
    fgets(buf, 256, fp);
  }
  
  fclose(fp);
  return "localhost";
  
}

