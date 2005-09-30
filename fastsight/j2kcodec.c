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
#include <math.h>
#include "j2k.h"
#include "j2kcodec.h"
#include "cfgmanager.h"

#define ir (0)
#define width (320)
#define height (240)

static unsigned char *enc_buf, *dec_buf;
int enc_rate;

static double dwt_norms_97[4][10]={
    {1.000, 1.965, 4.177, 8.403, 16.90, 33.84, 67.69, 135.3, 270.6, 540.9},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.080, 3.865, 8.307, 17.18, 34.71, 69.59, 139.3, 278.6, 557.2}
};

static int floorlog2(int a) {
    int l;
    for (l=0; a>1; l++) {
        a>>=1;
    }
    return l;
}

static void encode_stepsize(int stepsize, int numbps, int *expn, int *mant) {
    int p, n;
    p=floorlog2(stepsize)-13;
    n=11-floorlog2(stepsize);
    *mant=(n<0?stepsize>>-n:stepsize<<n)&0x7ff;
    *expn=numbps-p;
}

static void calc_explicit_stepsizes(j2k_tccp_t *tccp, int prec) {
    int numbands, bandno;
    numbands=3*tccp->numresolutions-2;
    for (bandno=0; bandno<numbands; bandno++) {
        double stepsize;

        int resno, level, orient, gain;
        resno=bandno==0?0:(bandno-1)/3+1;
        orient=bandno==0?0:(bandno-1)%3+1;
        level=tccp->numresolutions-1-resno;
        gain=tccp->qmfbid==0?0:(orient==0?0:(orient==1||orient==2?1:2));
        if (tccp->qntsty==J2K_CCP_QNTSTY_NOQNT) {
            stepsize=1.0;
        } else {
            double norm=dwt_norms_97[orient][level];
            stepsize=(1<<(gain+1))/norm;
        }
        encode_stepsize((int)floor(stepsize*8192.0), prec+gain, &tccp->stepsizes[bandno].expn, &tccp->stepsizes[bandno].mant);
    }
}

int j2kcodec_encoder_init()
{
  
  enc_rate = atoi(cfgmanager_get_value("j2krate"));
  enc_buf = (unsigned char *)malloc(enc_rate + 2);
  
  return 1;
  
}

int j2kcodec_encode(unsigned char *src, unsigned char **dest)
{
  
  j2k_image_t img;
  j2k_cp_t cp;
  j2k_tcp_t *tcp;
  j2k_tccp_t *tccp;
  
  unsigned char *p;
  int i, len;
  
  *dest = enc_buf;
  
  memset(&cp, sizeof(j2k_cp_t), 0);
  memset(&img, sizeof(j2k_image_t), 0);
  
  cp.tx0 = 0; cp.ty0 = 0;
  cp.tw = 1; cp.th = 1;
  cp.tcps = (j2k_tcp_t*)malloc(sizeof(j2k_tcp_t));
  tcp = &cp.tcps[0];

  tcp->numlayers = 1;
  tcp->rates[0] = enc_rate;
  
  img.x0 = 0; img.y0 = 0; img.x1 = width; img.y1 = height;
  img.numcomps = 3;
  img.comps = (j2k_comp_t*)malloc(img.numcomps*sizeof(j2k_comp_t));
  
  for(i = 0; i < img.numcomps; i++)
  {
    img.comps[i].data = (int*)malloc(width*height*sizeof(int));
    img.comps[i].prec = 8;
    img.comps[i].sgnd = 0;
    img.comps[i].dx = 1;
    img.comps[i].dy = 1;
  }
  
  p = src;
  for(i = 0; i < width * height; i++)
  {
    img.comps[0].data[i] = p[0];
    img.comps[1].data[i] = p[1];
    img.comps[2].data[i] = p[2];
    p+=3;
  }
  
  cp.tdx = img.x1 - img.x0;
  cp.tdy = img.y1 - img.y0;
  
  tcp->csty=0;
  tcp->prg=0;
  tcp->mct = img.numcomps==3?1:0;
  tcp->tccps = (j2k_tccp_t*)malloc(img.numcomps*sizeof(j2k_tccp_t));
  
  for(i = 0; i < img.numcomps; i++)
  {
    tccp = &tcp->tccps[i];
    tccp->csty = 0;
    tccp->numresolutions = 6;
    tccp->cblkw = 6;
    tccp->cblkh = 6;
    tccp->cblksty = 0;

    tccp->qmfbid = ir?0:1;
    tccp->qntsty = ir?J2K_CCP_QNTSTY_SEQNT:J2K_CCP_QNTSTY_NOQNT;
    tccp->numgbits = 2;
    tccp->roishift = 0;
    calc_explicit_stepsizes(tccp, img.comps[i].prec);
  }
  
  len = j2k_encode(&img, &cp, enc_buf, enc_rate + 2);
  
  free(tcp->tccps);
  for(i = 0; i < img.numcomps; i++)
    free(img.comps[i].data);
  free(img.comps);
  free(cp.tcps);
  
  return len;
  
}

int j2kcodec_decoder_init()
{
  
  dec_buf = (unsigned char *)malloc(width*height*3);
  
  return 1;
  
}

int j2kcodec_decode(unsigned char *src, int len, unsigned char **dest)
{
  
  j2k_image_t *img;
  j2k_cp_t *cp;
  int i;
  
  unsigned char *p = dec_buf;
  
  *dest = dec_buf;
  
  if(!j2k_decode(src, len, &img, &cp))
    return 0;
  
  for(i=0; i<width*height; i++)
  {
    p[0] = img->comps[0].data[i];
    p[1] = img->comps[1].data[i];
    p[2] = img->comps[2].data[i];
    p+=3;
  }
  
  j2k_release(img, cp);
  
  return 1;
  
}
