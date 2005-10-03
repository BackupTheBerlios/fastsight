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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev.h>
#include "capture.h"
#include "capturepriv.h"
#include "cfgmanager.h"

typedef enum {
  IO_METHOD_READ,
  IO_METHOD_MMAP,
} io_method;

static unsigned char *v4l_map;
static int v4l_fd = -1;
static io_method v4l_io;

static void capture_shutdown()
{
  close(v4l_fd);
}

int capture_init()
{
  
  struct video_capability vcap;
  struct video_channel    vc;
  struct video_picture    vp;
  struct video_window     vw;
  struct video_mbuf       mb;
  
  int channel, brightness, contrast;
  char *s;
  
  if(!(s = cfgmanager_get_value("videochan")))
    s = "0";
  channel    = atoi(s);
  if(!(s = cfgmanager_get_value("brightness")))
    s = "32768";
  brightness = atoi(s);
  if(!(s = cfgmanager_get_value("contrast")))
    s = "32768";
  contrast   = atoi(s);
  if(!(s = cfgmanager_get_value("videodev")))
    s = "/dev/video0";
  
  if((v4l_fd = open(s, O_RDONLY)) < 0)
  {
    perror("open");
    return 0;
  }
  
  if(ioctl(v4l_fd, VIDIOCGCAP, &vcap) < 0)
  {
    perror("VIDIOCGCAP");
    close(v4l_fd);
    return 0;
  }

  if(channel >= vcap.channels)
  {
    fprintf(stderr, "capture: invalid channel\n");
    close(v4l_fd);
    return 0;
  }
  
  if(ioctl(v4l_fd, VIDIOCGPICT, &vp) < 0)
  {
    perror("VIDIOCGPICT");
    close(v4l_fd);
    return 0;
  }
  
  vc.channel = channel;
  vc.type = VIDEO_TYPE_CAMERA;
  vc.norm = 0;
  
  if(ioctl(v4l_fd, VIDIOCSCHAN, &vc) < 0){
    perror("VIDIOCSCHAN");
    close(v4l_fd);
    return 0;
  }
  
  if(ioctl(v4l_fd, VIDIOCGWIN, &vw) < 0){
    perror("VIDIOCGWIN");
    close(v4l_fd);
    return 0;
  }
  
  vw.x=0;
  vw.y=0;
  vw.width=320;
  vw.height=240;
  
  if(ioctl(v4l_fd, VIDIOCSWIN, &vw) < 0)
  {
    perror("VIDIOCSWIN");
    close(v4l_fd);
    return 0;
  }
  
  vp.depth = 24;
  vp.brightness = brightness;
  vp.contrast = contrast;
  vp.palette = VIDEO_PALETTE_RGB24;
  
  if(ioctl(v4l_fd, VIDIOCSPICT, &vp))
    perror("VIDIOCSPICT");
  
  v4l_map = (unsigned char *)-1;
  
  if(ioctl(v4l_fd, VIDIOCGMBUF, &mb) < 0)
    v4l_io = IO_METHOD_READ;
  else
  {
    v4l_io = IO_METHOD_MMAP;
    v4l_map = (unsigned char *)mmap(0, mb.size,
      PROT_READ, MAP_SHARED, v4l_fd, 0);
    if((unsigned char *)-1 == (unsigned char *)v4l_map)
      v4l_io = IO_METHOD_READ; /* fallback to read method */
  }
  
  if(v4l_io == IO_METHOD_MMAP)
    v4l_map += mb.offsets[0];
  else
    v4l_map = (unsigned char *)malloc(320*240*3);
  
  atexit(capture_shutdown);
  
  return 1;
  
}

unsigned char *capture_frame()
{
  
  struct video_mmap       mm;
  unsigned char t, *p = v4l_map;
  int i;
  
  if(v4l_io == IO_METHOD_MMAP)
  {
    mm.frame  = 0;
    mm.width  = 320;
    mm.height = 240;
    mm.format = VIDEO_PALETTE_RGB24;
    if(ioctl(v4l_fd, VIDIOCMCAPTURE, &mm) < 0)
      perror("VIDIOCMCAPTURE");
    if(ioctl(v4l_fd, VIDIOCSYNC, &mm.frame) < 0)
      perror("VIDIOCSYNC");
  } else {
    while(read(v4l_fd, v4l_map, 320*240*3) <= 0);
  }
  
  /* Convert BGR -> RGB */
  i = 320*240;
  while(i > 0)
  {
    t = p[2]; p[2] = p[0]; p[0] = t;
    p += 3;
    i--;
  }
  
  return v4l_map;
  
  
}

void capture_set_brightness(int value)
{
  
  struct video_picture    vp;
    
  if(v4l_fd < 0)
    return;
  
  if(ioctl(v4l_fd, VIDIOCGPICT, &vp) < 0)
  {
    perror("VIDIOCGPICT");
    return;
  }
  
  vp.brightness = value;
  
  if(ioctl(v4l_fd, VIDIOCSPICT, &vp))
    perror("VIDIOCSPICT");
  
}

void capture_set_contrast(int value)
{
  
  struct video_picture    vp;
 
  if(v4l_fd < 0)
    return;
  
  if(ioctl(v4l_fd, VIDIOCGPICT, &vp) < 0)
  {
    perror("VIDIOCGPICT");
    return;
  }
  
  vp.contrast = value;
  
  if(ioctl(v4l_fd, VIDIOCSPICT, &vp))
    perror("VIDIOCSPICT");
  
}
