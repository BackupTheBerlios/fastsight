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
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#define recv_t char *
#define send_t const char *
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define recv_t void *
#define send_t void *
#endif

#include <FL/fl_ask.H>
#include <FL/Fl_Image.H>

#include "cfgmanager.h"
#include "interface.h"
#include "connect.h"
#include "j2kcodec.h"
#include "capture.h"
#include "filters.h"
#include "get_ip.h"

static int we_are_sending;
static int already_connecting = 0;
static int do_server_auth = 0;
static int do_recv_type = 0;
static int sock;

static char connection_key[7];
static unsigned char *recv_buf = 0;
static int recv_buf_len = 0;
static int recved_len = 0;
static int recv_len = 0;

FILE *record_file = 0;

static void setnonblocking(int s)
{
#ifdef WIN32
  u_long opts = 1;
  ioctlsocket(s, FIONBIO, &opts);
#else
  int opts;
  opts = fcntl(s,F_GETFL);
  opts = opts | O_NONBLOCK;
  fcntl(s,F_SETFL,opts);
#endif
}

static void data_can_be_sent(int fd, void *data)
{
  unsigned char *rgbimg, *j2kimg;
  int len;

  rgbimg = capture_frame();
  
  filters_apply(rgbimg);
  
  videoarea->rgbimg(rgbimg);
  videoarea->redraw();
  
  len = j2kcodec_encode(rgbimg, &j2kimg);
  
  send(fd, (send_t)&len, 4, 0);
  send(fd, (send_t)j2kimg, len, 0);
  
  Fl::remove_fd(fd);
  Fl::wait(0.2);
  Fl::add_fd(fd, FL_WRITE, data_can_be_sent, 0);
  
}

static void data_received(int fd, void *data)
{
  
  if(do_server_auth)
  {
    
    if(!recv_buf)
      recv_buf = (unsigned char *)malloc(recv_buf_len = 16);
    
    recved_len += recv(fd, (recv_t)(recv_buf+recved_len), 6-recved_len, 0);
    
    if(recved_len < 6)
      return;
    
    recv_buf[6] = 0;

    if(strcmp((char *)recv_buf, connection_key))
    {
      close(fd);
      Fl::remove_fd(fd);
      close(sock);
      fl_alert("Auth failed! Waiting the key '%s', but received '%s'.",
        connection_key, recv_buf);
      exit(1);
    }
    
    connectwnd->hide();
    mainwnd->show();
    
    if(we_are_sending)
    {
      Fl::remove_fd(fd);
      Fl::add_fd(fd, FL_WRITE, data_can_be_sent, 0);
    }
    
    recv_buf[0] = 'T';
    recv_buf[1] = we_are_sending ? 'S' : 'R';
    send(fd, (send_t)recv_buf, 2, 0);
    
    recved_len = 0;
    do_server_auth = 0;
    
    return;
    
  }
  
  if(do_recv_type)
  {
    
    if(!recv_buf)
      recv_buf = (unsigned char *)malloc(recv_buf_len = 16);
    
    recved_len += recv(fd, (recv_t)(recv_buf+recved_len), 2-recved_len, 0);
    
    if(recved_len < 2)
      return;
    
    if(recv_buf[0] != 'T')
    {
      close(fd);
      Fl::remove_fd(fd);
      close(sock);
      fl_alert("Negotiation flaw! Couldn't receive session type.");
      exit(1);
    }
    
    we_are_sending = (recv_buf[1] == 'R');
    
    connectwnd->hide();
    mainwnd->show();
    
    if(we_are_sending)
    {
      if(!capture_init())
      {
        close(fd);
        Fl::remove_fd(fd);
        close(sock);
        fl_alert("Couldn't initialize the webcam. Please check the "
          "configuration and try again.");
        exit(1);
      }
      j2kcodec_encoder_init();
      Fl::remove_fd(fd);
      Fl::add_fd(fd, FL_WRITE, data_can_be_sent, 0);
    } else {
      j2kcodec_decoder_init();
    }
    
    recved_len = 0;
    do_recv_type = 0;
    
    return;
    
  }
  
  if(!recv_len)
  {
    
    recved_len += recv(fd, (recv_t)(recv_buf+recved_len), 4-recved_len, 0);
    
    if(recved_len < 4)
      return;
    
    recv_len = *((int *)recv_buf);
    
    if(recv_len > recv_buf_len)
    {
      free(recv_buf);
      recv_buf = (unsigned char *)malloc(recv_buf_len=recv_len);
    }
    
    recved_len = 0;
    
    return;
    
  }
  
  recved_len += recv(fd, (recv_t)(recv_buf+recved_len), recv_len-recved_len, 0);
  
  if(recved_len >= recv_len)
  {
    
    unsigned char *rgbimg;
    
    recv_len = 0;
    
    if(record_file)
    {
      fwrite(&recved_len, 4, 1, record_file);
      fwrite(recv_buf, recved_len, 1, record_file);
    }
    
    j2kcodec_decode(recv_buf, recved_len, &rgbimg);
    
    filters_apply(rgbimg);
    
    videoarea->rgbimg(rgbimg);
    videoarea->redraw();
    
    recved_len = 0;
    
  }
  
}

static void server_connected(int fd, void *data)
{
  
  int connection = accept(fd, NULL, NULL);
  setnonblocking(connection);
  
  Fl::add_fd(connection, FL_READ, data_received, 0);
  Fl::remove_fd(fd);
  
}

void connect_create_server()
{
  
  struct sockaddr_in server_address;
  char *s, buf[128];
  int port;
  
  if(already_connecting)
    return;
  
  if(!(s = cfgmanager_get_value("listenport")))
    cfgmanager_set_value("listenport", s = "7000");
  port = atoi(s);
  
  we_are_sending = chksend->value();
  
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return;
  
  if(we_are_sending)
  {
    if(!capture_init())
    {
      fl_alert("Couldn't initialize the webcam. Please check the "
        "configuration and try again.");
      return;
    }
    j2kcodec_encoder_init();
  } else {
    j2kcodec_decoder_init();
  }
  
  already_connecting = 1;
  
  setnonblocking(sock);
  
  memset((char *)&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);
  server_address.sin_port = htons(port++);
  
  while(bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    server_address.sin_port = htons(port++);
  port--;

  srand(time(0));
  sprintf(connection_key, "%02x%02x%02x", rand()%256, rand()%256, rand()%256);
  
  sprintf(buf, "%s:%d:%s", get_ip(), port, connection_key);
  txtcode->value(buf);
  
  do_server_auth = 1;
  
  listen(sock, 1);
  
  Fl::add_fd(sock, FL_READ, server_connected, 0);
  
}

void connect_create_client()
{
  
  struct sockaddr_in remote;
  struct hostent *hp;
  char buf[128], *shost, *sport, *sauth;
  int port;
    
  if(already_connecting)
    return;
  
  if(!strlen((char *)txtcode->value()))
    return;
  
  already_connecting = 1;
  
  strncpy(buf, (char *)txtcode->value(), 128);
  shost = strtok(buf, ":");
  sport = strtok(0, ":");
  sauth = strtok(0, ":");
  
  port = atoi(sport);
  
  if(!(hp = gethostbyname(shost)))
  {
    fl_alert("Couldn't resolve hostname. Change the"
      "connection string and try again.");
    already_connecting = 0;
    return;
  }
  
  memset((char *)&remote, 0, sizeof(remote));
  remote.sin_family = AF_INET;
  remote.sin_port = htons(port);
  memcpy((char *)&remote.sin_addr, hp->h_addr, hp->h_length);
  
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    already_connecting = 0;
    return;
  }
  
  if(connect(sock, (struct sockaddr *)&remote, sizeof(remote)) < 0)
  {
    fl_alert("Couldn't connect to the host. Change the"
      "connection string and try again.");
    close(sock);
    already_connecting = 0;
    return;
  }
  
  strncpy(connection_key, sauth, 6);
  send(sock, connection_key, 6, 0);
  
  do_recv_type = 1;

  setnonblocking(sock);
  
  Fl::add_fd(sock, FL_READ, data_received, 0);
  
}
