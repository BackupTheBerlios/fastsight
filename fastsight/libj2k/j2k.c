/*
 * Copyright (c) 2001-2002, David Janssens
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * 24/04/2003: memory leak fixes by Steve Williams
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>

#include "j2k.h"
#include "cio.h"
#include "tcd.h"
#include "dwt.h"
#include "int.h"
#include "log.h"

#define J2K_MS_SOC 0xff4f
#define J2K_MS_SOT 0xff90
#define J2K_MS_SOD 0xff93
#define J2K_MS_EOC 0xffd9
#define J2K_MS_SIZ 0xff51
#define J2K_MS_COD 0xff52
#define J2K_MS_COC 0xff53
#define J2K_MS_RGN 0xff5e
#define J2K_MS_QCD 0xff5c
#define J2K_MS_QCC 0xff5d
#define J2K_MS_POC 0xff5f
#define J2K_MS_TLM 0xff55
#define J2K_MS_PLM 0xff57
#define J2K_MS_PLT 0xff58
#define J2K_MS_PPM 0xff60
#define J2K_MS_PPT 0xff61
#define J2K_MS_SOP 0xff91
#define J2K_MS_EPH 0xff92
#define J2K_MS_CRG 0xff63
#define J2K_MS_COM 0xff64

#define J2K_STATE_MHSOC 0x0001
#define J2K_STATE_MHSIZ 0x0002
#define J2K_STATE_MH 0x0004
#define J2K_STATE_TPHSOT 0x0008
#define J2K_STATE_TPH 0x0010
#define J2K_STATE_MT 0x0020

jmp_buf j2k_error;

int j2k_state;
int j2k_curtileno;
j2k_tcp_t j2k_default_tcp;
unsigned char *j2k_eot;
int j2k_sot_start;

j2k_image_t *j2k_img;
j2k_cp_t *j2k_cp;

unsigned char **j2k_tile_data;
int *j2k_tile_len;

void j2k_dump_image(j2k_image_t *img) {
	if (is_loggable(LOG_TRACE)) {
		int compno;
		log_print(LOG_TRACE, "image {\n");
		log_print(LOG_TRACE, "  x0=%d, y0=%d, x1=%d, y1=%d\n", img->x0, img->y0, img->x1, img->y1);
		log_print(LOG_TRACE, "  numcomps=%d\n", img->numcomps);
		for (compno=0; compno<img->numcomps; compno++) {
			j2k_comp_t *comp=&img->comps[compno];
			log_print(LOG_TRACE, "  comp %d {\n", compno);
			log_print(LOG_TRACE, "    dx=%d, dy=%d\n", comp->dx, comp->dy);
			log_print(LOG_TRACE, "    prec=%d\n", comp->prec);
			log_print(LOG_TRACE, "    sgnd=%d\n", comp->sgnd);
			log_print(LOG_TRACE, "  }\n");
		}
		log_print(LOG_TRACE, "}\n");
	}
}

void j2k_dump_cp(j2k_image_t *img, j2k_cp_t *cp) {
	if (is_loggable(LOG_TRACE)) {
		int tileno, compno, layno, bandno, resno, numbands;
		log_print(LOG_TRACE, "coding parameters {\n");
		log_print(LOG_TRACE, "  tx0=%d, ty0=%d\n", cp->tx0, cp->ty0);
		log_print(LOG_TRACE, "  tdx=%d, tdy=%d\n", cp->tdx, cp->tdy);
		log_print(LOG_TRACE, "  tw=%d, th=%d\n", cp->tw, cp->th);
		for (tileno=0; tileno<cp->tw*cp->th; tileno++) {
			j2k_tcp_t *tcp=&cp->tcps[tileno];
			log_print(LOG_TRACE, "  tile %d {\n", tileno);
			log_print(LOG_TRACE, "    csty=%x\n", tcp->csty);
			log_print(LOG_TRACE, "    prg=%d\n", tcp->prg);
			log_print(LOG_TRACE, "    numlayers=%d\n", tcp->numlayers);
			log_print(LOG_TRACE, "    mct=%d\n", tcp->mct);
			log_print(LOG_TRACE, "    rates=");
			for (layno=0; layno<tcp->numlayers; layno++) {
				log_print(LOG_TRACE, "%d ", tcp->rates[layno]);
			}
			log_print(LOG_TRACE, "\n");
			for (compno=0; compno<img->numcomps; compno++) {
				j2k_tccp_t *tccp=&tcp->tccps[compno];
				log_print(LOG_TRACE, "    comp %d {\n", compno);
				log_print(LOG_TRACE, "      csty=%x\n", tccp->csty);
				log_print(LOG_TRACE, "      numresolutions=%d\n", tccp->numresolutions);
				log_print(LOG_TRACE, "      cblkw=%d\n", tccp->cblkw);
				log_print(LOG_TRACE, "      cblkh=%d\n", tccp->cblkh);
				log_print(LOG_TRACE, "      cblksty=%x\n", tccp->cblksty);
				log_print(LOG_TRACE, "      qmfbid=%d\n", tccp->qmfbid);
				log_print(LOG_TRACE, "      qntsty=%d\n", tccp->qntsty);
				log_print(LOG_TRACE, "      numgbits=%d\n", tccp->numgbits);
				log_print(LOG_TRACE, "      roishift=%d\n", tccp->roishift);
				log_print(LOG_TRACE, "      stepsizes=");
				numbands=tccp->qntsty==J2K_CCP_QNTSTY_SIQNT?1:tccp->numresolutions*3-2;
				for (bandno=0; bandno<numbands; bandno++) {
					log_print(LOG_TRACE, "(%d,%d) ", tccp->stepsizes[bandno].mant, tccp->stepsizes[bandno].expn);
				}
				log_print(LOG_TRACE, "\n");
				if (tccp->csty&J2K_CCP_CSTY_PRT) {
					log_print(LOG_TRACE, "      prcw=");
					for (resno=0; resno<tccp->numresolutions; resno++) {
						log_print(LOG_TRACE, "%d ", tccp->prcw[resno]);
					}
					log_print(LOG_TRACE, "\n");
					log_print(LOG_TRACE, "      prch=");
					for (resno=0; resno<tccp->numresolutions; resno++) {
						log_print(LOG_TRACE, "%d ", tccp->prch[resno]);
					}
					log_print(LOG_TRACE, "\n");
				}
				log_print(LOG_TRACE, "    }\n");
			}
			log_print(LOG_TRACE, "  }\n");
		}
		log_print(LOG_TRACE, "}\n");
	}
}

void j2k_write_soc() {
    log_print(LOG_TRACE, "%.8x: SOC\n", cio_tell());
    cio_write(J2K_MS_SOC, 2);
}

void j2k_read_soc() {
    log_print(LOG_TRACE, "%.8x: SOC\n", cio_tell()-2);
    j2k_state=J2K_STATE_MHSIZ;
}

void j2k_write_siz() {
    int i;
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: SIZ\n", cio_tell());
    cio_write(J2K_MS_SIZ, 2);
    lenp=cio_tell();
    cio_skip(2);
    cio_write(0, 2);
    cio_write(j2k_img->x1, 4);
    cio_write(j2k_img->y1, 4);
    cio_write(j2k_img->x0, 4);
    cio_write(j2k_img->y0, 4);
    cio_write(j2k_cp->tdx, 4);
    cio_write(j2k_cp->tdy, 4);
    cio_write(j2k_cp->tx0, 4);
    cio_write(j2k_cp->ty0, 4);
    cio_write(j2k_img->numcomps, 2);
    for (i=0; i<j2k_img->numcomps; i++) {
        cio_write(j2k_img->comps[i].prec-1+(j2k_img->comps[i].sgnd<<7), 1);
        cio_write(j2k_img->comps[i].dx, 1);
        cio_write(j2k_img->comps[i].dy, 1);
    }
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_siz() {
    int len, i;
    log_print(LOG_TRACE, "%.8x: SIZ\n", cio_tell()-2);
    len=cio_read(2);
    cio_read(2);
    j2k_img->x1=cio_read(4);
    j2k_img->y1=cio_read(4);
    j2k_img->x0=cio_read(4);
    j2k_img->y0=cio_read(4);
    j2k_cp->tdx=cio_read(4);
    j2k_cp->tdy=cio_read(4);
    j2k_cp->tx0=cio_read(4);
    j2k_cp->ty0=cio_read(4);
    j2k_img->numcomps=cio_read(2);
    j2k_img->comps=(j2k_comp_t*)malloc(j2k_img->numcomps*sizeof(j2k_comp_t));
    for (i=0; i<j2k_img->numcomps; i++) {
        int tmp, w, h;
        tmp=cio_read(1);
        j2k_img->comps[i].prec=(tmp&0x7f)+1;
        j2k_img->comps[i].sgnd=tmp>>7;
        j2k_img->comps[i].dx=cio_read(1);
        j2k_img->comps[i].dy=cio_read(1);
        w=int_ceildiv(j2k_img->x1-j2k_img->x0, j2k_img->comps[i].dx);
        h=int_ceildiv(j2k_img->y1-j2k_img->y0, j2k_img->comps[i].dy);
        j2k_img->comps[i].data=(int*)malloc(sizeof(int)*w*h);
    }
    j2k_cp->tw=int_ceildiv(j2k_img->x1-j2k_img->x0, j2k_cp->tdx);
    j2k_cp->th=int_ceildiv(j2k_img->y1-j2k_img->y0, j2k_cp->tdy);
    j2k_cp->tcps=(j2k_tcp_t*)calloc(sizeof(j2k_tcp_t), j2k_cp->tw*j2k_cp->th);
    
    j2k_default_tcp.tccps=(j2k_tccp_t*)calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
 
    for (i=0; i<j2k_cp->tw*j2k_cp->th; i++) {
        j2k_cp->tcps[i].tccps=(j2k_tccp_t*)calloc(sizeof(j2k_tccp_t), j2k_img->numcomps);
    }
    j2k_tile_data=(unsigned char**)calloc(j2k_cp->tw*j2k_cp->th, sizeof(char*));
    j2k_tile_len=(int*)calloc(j2k_cp->tw*j2k_cp->th, sizeof(int));
    j2k_state=J2K_STATE_MH;
}

void j2k_write_com() {
    unsigned int i;
    int lenp, len;
    char str[256];
    log_print(LOG_DEBUG, "Creator: J2000 codec");
    log_print(LOG_TRACE, "%.8x: COM\n", cio_tell());
    cio_write(J2K_MS_COM, 2);
    lenp=cio_tell();
    cio_skip(2);
    cio_write(0, 2);
    for (i=0; i<strlen(str); i++) {
        cio_write(str[i], 1);
    }
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_com() {
    int len;
    log_print(LOG_TRACE, "%.8x: COM\n", cio_tell()-2);
    len=cio_read(2);
    cio_skip(len-2);
}

void j2k_write_cox(int compno) {
    int i;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    tcp=&j2k_cp->tcps[j2k_curtileno];
    tccp=&tcp->tccps[compno];
    cio_write(tccp->numresolutions-1, 1);
    cio_write(tccp->cblkw-2, 1);
    cio_write(tccp->cblkh-2, 1);
    cio_write(tccp->cblksty, 1);
    cio_write(tccp->qmfbid, 1);
    if (tccp->csty&J2K_CCP_CSTY_PRT) {
        for (i=0; i<tccp->numresolutions; i++) {
            cio_write(tccp->prcw[i]+(tccp->prch[i]<<4), 1);
        }
    }
}

void j2k_read_cox(int compno) {
    int i;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    tccp=&tcp->tccps[compno];
    tccp->numresolutions=cio_read(1)+1;
    tccp->cblkw=cio_read(1)+2;
    tccp->cblkh=cio_read(1)+2;
    tccp->cblksty=cio_read(1);
    tccp->qmfbid=cio_read(1);
    if (tccp->csty&J2K_CP_CSTY_PRT) {
        for (i=0; i<tccp->numresolutions; i++) {
            int tmp=cio_read(1);
            tccp->prcw[i]=tmp&0xf;
            tccp->prch[i]=tmp>>4;
        }
    }
}

void j2k_write_cod() {
    j2k_tcp_t *tcp;
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: COD\n", cio_tell());
    cio_write(J2K_MS_COD, 2);
    lenp=cio_tell();
    cio_skip(2);
    tcp=&j2k_cp->tcps[j2k_curtileno];
    cio_write(tcp->csty, 1);
    cio_write(tcp->prg, 1);
    cio_write(tcp->numlayers, 2);
    cio_write(tcp->mct, 1);
    j2k_write_cox(0);
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_cod() {
    int len, i, pos;
    j2k_tcp_t *tcp;
    log_print(LOG_TRACE, "%.8x: COD\n", cio_tell()-2);
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);
    tcp->csty=cio_read(1);
    tcp->prg=cio_read(1);
    tcp->numlayers=cio_read(2);
    tcp->mct=cio_read(1);
    pos=cio_tell();
    for (i=0; i<j2k_img->numcomps; i++) {
        tcp->tccps[i].csty=tcp->csty&J2K_CP_CSTY_PRT;
        cio_seek(pos);
        j2k_read_cox(i);
    }
}

void j2k_write_coc(int compno) {
    j2k_tcp_t *tcp;
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: COC\n", cio_tell());
    cio_write(J2K_MS_COC, 2);
    lenp=cio_tell();
    cio_skip(2);
    tcp=&j2k_cp->tcps[j2k_curtileno];
    cio_write(compno, j2k_img->numcomps<=256?1:2);
    cio_write(tcp->tccps[compno].csty, 1);
    j2k_write_cox(compno);
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_coc() {
    int len, compno;
    j2k_tcp_t *tcp;
    log_print(LOG_TRACE, "%.8x: COC\n", cio_tell()-2);
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);
    compno=cio_read(j2k_img->numcomps<=256?1:2);
    tcp->tccps[compno].csty=cio_read(1);
    j2k_read_cox(compno);
}

void j2k_write_qcx(int compno) {
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    int bandno, numbands;
    tcp=&j2k_cp->tcps[j2k_curtileno];
    tccp=&tcp->tccps[compno];
    cio_write(tccp->qntsty+(tccp->numgbits<<5), 1);
    numbands=tccp->qntsty==J2K_CCP_QNTSTY_SIQNT?1:tccp->numresolutions*3-2;
    for (bandno=0; bandno<numbands; bandno++) {
        int expn, mant;
        expn=tccp->stepsizes[bandno].expn;
        mant=tccp->stepsizes[bandno].mant;
        if (tccp->qntsty==J2K_CCP_QNTSTY_NOQNT) {
            cio_write(expn<<3, 1);
        } else {
            cio_write((expn<<11)+mant, 2);
        }
    }
}

void j2k_read_qcx(int compno, int len) {
    int tmp;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tccp;
    int bandno, numbands;
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    tccp=&tcp->tccps[compno];
    tmp=cio_read(1);
    tccp->qntsty=tmp&0x1f;
    tccp->numgbits=tmp>>5;
    numbands=tccp->qntsty==J2K_CCP_QNTSTY_SIQNT?1:(tccp->qntsty==J2K_CCP_QNTSTY_NOQNT?len-1:(len-1)/2);
    for (bandno=0; bandno<numbands; bandno++) {
        int expn, mant;
        if (tccp->qntsty==J2K_CCP_QNTSTY_NOQNT) { // WHY STEPSIZES WHEN NOQNT ?
            expn=cio_read(1)>>3;
            mant=0;
        } else {
            tmp=cio_read(2);
            expn=tmp>>11;
            mant=tmp&0x7ff;
        }
        tccp->stepsizes[bandno].expn=expn;
        tccp->stepsizes[bandno].mant=mant;
    }
}

void j2k_write_qcd() {
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: QCD\n", cio_tell());
    cio_write(J2K_MS_QCD, 2);
    lenp=cio_tell();
    cio_skip(2);
    j2k_write_qcx(0);
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_qcd() {
    int len, i, pos;
    log_print(LOG_TRACE, "%.8x: QCD\n", cio_tell()-2);
    len=cio_read(2);
    pos=cio_tell();
    for (i=0; i<j2k_img->numcomps; i++) {
        cio_seek(pos);
        j2k_read_qcx(i, len-2);
    }
}

void j2k_write_qcc(int compno) {
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: QCC\n", cio_tell());
    cio_write(J2K_MS_QCC, 2);
    lenp=cio_tell();
    cio_skip(2);
    cio_write(compno, j2k_img->numcomps<=256?1:2);
    j2k_write_qcx(compno);
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_qcc() {
    int len, compno;
    log_print(LOG_TRACE, "%.8x: QCC\n", cio_tell()-2);
    len=cio_read(2);
    compno=cio_read(j2k_img->numcomps<=256?1:2);
    j2k_read_qcx(compno, len-2-(j2k_img->numcomps<=256?1:2));
}

void j2k_read_poc() {
    int len, numpchgs, i;
    j2k_tcp_t *tcp;
    log_print(LOG_WARNING, "WARNING: POC marker segment processing not fully implemented\n");
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);
    numpchgs=(len-2)/(5+2*(j2k_img->numcomps<=256?1:2));
    for (i=0; i<numpchgs; i++) {
        int resno0, compno0, layerno1, resno1, compno1, prg;
        resno0=cio_read(1);
        compno0=cio_read(j2k_img->numcomps<=256?1:2);
        layerno1=cio_read(2);
        resno1=cio_read(1);
        compno1=cio_read(j2k_img->numcomps<=256?1:2);
        prg=cio_read(1);
        tcp->prg=prg;
    }
}

void j2k_read_crg() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: CRG marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_tlm() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: TLM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_plm() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: PLM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_plt() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: PLT marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_ppm() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: PPM marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_read_ppt() {
    int len;
    len=cio_read(2);
    log_print(LOG_WARNING, "WARNING: PPT marker segment processing not implemented\n");
    cio_skip(len-2);
}

void j2k_write_sot() {
    int lenp, len;
    log_print(LOG_TRACE, "%.8x: SOT\n", cio_tell());
    j2k_sot_start=cio_tell();
    cio_write(J2K_MS_SOT, 2);
    lenp=cio_tell();
    cio_skip(2);
    cio_write(j2k_curtileno, 2);
    cio_skip(4);
    cio_write(0, 1);
    cio_write(1, 1);
    len=cio_tell()-lenp;
    cio_seek(lenp);
    cio_write(len, 2);
    cio_seek(lenp+len);
}

void j2k_read_sot() {
    int len, tileno, totlen, partno, numparts, i;
    j2k_tcp_t *tcp;
    j2k_tccp_t *tmp;
    log_print(LOG_TRACE, "%.8x: SOT\n", cio_tell()-2);
    len=cio_read(2);
    tileno=cio_read(2);
    totlen=cio_read(4);
    partno=cio_read(1);
    numparts=cio_read(1);
    j2k_curtileno=tileno;
    j2k_eot=cio_getbp()-12+totlen;
    j2k_state=J2K_STATE_TPH;
    tcp=&j2k_cp->tcps[j2k_curtileno];
    tmp=tcp->tccps;
    *tcp=j2k_default_tcp;
    tcp->tccps=tmp;
    for (i=0; i<j2k_img->numcomps; i++) {
        tcp->tccps[i]=j2k_default_tcp.tccps[i];
    }
}

void j2k_write_sod() {
    int l, layno;
    int totlen;
    j2k_tcp_t *tcp;
    log_print(LOG_TRACE, "%.8x: SOD\n", cio_tell());
    cio_write(J2K_MS_SOD, 2);
    tcp=&j2k_cp->tcps[j2k_curtileno];
    for (layno=0; layno<tcp->numlayers; layno++) {
        tcp->rates[layno]-=cio_tell();
        log_print(LOG_TRACE, "tcp->rates[%d]=%d\n", layno, tcp->rates[layno]);
    }
    log_print(LOG_TRACE, "cio_numbytesleft=%d\n", cio_numbytesleft());
    tcd_init(j2k_img, j2k_cp);
    l=tcd_encode_tile(j2k_curtileno, cio_getbp(), cio_numbytesleft()-2);
    totlen=cio_tell()+l-j2k_sot_start;
    cio_seek(j2k_sot_start+6);
    cio_write(totlen, 4);
    cio_seek(j2k_sot_start+totlen);
    tcd_free(j2k_cp);
}

void j2k_read_sod() {
    int len;
    unsigned char *data;
    log_print(LOG_TRACE, "%.8x: SOD\n", cio_tell()-2);
    len=int_min(j2k_eot-cio_getbp(), cio_numbytesleft());
    j2k_tile_len[j2k_curtileno]+=len;
    data=(unsigned char*)realloc(j2k_tile_data[j2k_curtileno], j2k_tile_len[j2k_curtileno]);
    memcpy(data, cio_getbp(), len);
    j2k_tile_data[j2k_curtileno]=data;
    cio_skip(len);
    j2k_state=J2K_STATE_TPHSOT;
}

void j2k_read_rgn() {
    int len, compno, roisty;
    j2k_tcp_t *tcp;
    log_print(LOG_TRACE, "%.8x: RGN\n", cio_tell()-2);
    tcp=j2k_state==J2K_STATE_TPH?&j2k_cp->tcps[j2k_curtileno]:&j2k_default_tcp;
    len=cio_read(2);
    compno=cio_read(j2k_img->numcomps<=256?1:2);
    roisty=cio_read(1);
    tcp->tccps[compno].roishift=cio_read(1);
}

void j2k_write_eoc() {
    log_print(LOG_TRACE, "%.8x: EOC\n", cio_tell());
    cio_write(J2K_MS_EOC, 2);
}

void j2k_read_eoc() {
    int tileno;
    log_print(LOG_TRACE, "%.8x: EOC\n", cio_tell()-2);
    j2k_dump_image(j2k_img);
    j2k_dump_cp(j2k_img, j2k_cp);
    tcd_init(j2k_img, j2k_cp);
    for (tileno=0; tileno<j2k_cp->tw*j2k_cp->th; tileno++) {
        tcd_decode_tile(j2k_tile_data[tileno], j2k_tile_len[tileno], tileno);
    }
    j2k_state=J2K_STATE_MT;
    longjmp(j2k_error, 1);
}

void j2k_read_unk() {
    log_print(LOG_WARNING, "unknown marker\n");
}

static int logging_inited = 0;
/* initialize the logging on the first encode or decode*/
static void init_logging() {
	if (!logging_inited) {
		char *log_level = getenv("J2K_LOG_LEVEL");
		if (log_level) {
			LogLevel newLogLevel = LOG_ERROR;
			if (strstr(log_level, "FATAL") == 0) {
				newLogLevel = LOG_FATAL;
			} else if (strstr(log_level, "ERROR") == 0) {
				newLogLevel = LOG_ERROR;
			} else if (strstr(log_level, "WARNING") == 0) {
				newLogLevel = LOG_WARNING;
			} else if (strstr(log_level, "DEBUG") == 0) {
				newLogLevel = LOG_DEBUG;
			} else if (strstr(log_level, "TRACE") == 0) {
				newLogLevel = LOG_TRACE;
			}
			set_log_level(newLogLevel);
		}
       		logging_inited = 1;
    	}
}

LIBJ2K_API int j2k_encode(j2k_image_t *img, j2k_cp_t *cp, unsigned char *dest, int len) {

    int tileno, compno;
    /* Initialize the logging */
    init_logging();
    if (setjmp(j2k_error)) {
        return 0;
    }
    cio_init(dest, len);
    j2k_img=img;
    j2k_cp=cp;
    j2k_dump_cp(j2k_img, j2k_cp);
    j2k_write_soc();
    j2k_write_siz();
    j2k_write_com();
    for (tileno=0; tileno<cp->tw*cp->th; tileno++) {
        j2k_curtileno=tileno;
        j2k_write_sot();
        j2k_write_cod();
        j2k_write_qcd();
        for (compno=1; compno<img->numcomps; compno++) {
            j2k_write_coc(compno);
            j2k_write_qcc(compno);
        }
        j2k_write_sod();
    }
    j2k_write_eoc();
    return cio_tell();
}

typedef struct {
    int id;
    int states;
    void (*handler)();
} j2k_dec_mstabent_t;

j2k_dec_mstabent_t j2k_dec_mstab[]={
    {J2K_MS_SOC, J2K_STATE_MHSOC, j2k_read_soc},
    {J2K_MS_SOT, J2K_STATE_MH|J2K_STATE_TPHSOT, j2k_read_sot},
    {J2K_MS_SOD, J2K_STATE_TPH, j2k_read_sod},
    {J2K_MS_EOC, J2K_STATE_TPHSOT, j2k_read_eoc},
    {J2K_MS_SIZ, J2K_STATE_MHSIZ, j2k_read_siz},
    {J2K_MS_COD, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_cod},
    {J2K_MS_COC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_coc},
    {J2K_MS_RGN, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_rgn},
    {J2K_MS_QCD, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_qcd},
    {J2K_MS_QCC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_qcc},
    {J2K_MS_POC, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_poc},
    {J2K_MS_TLM, J2K_STATE_MH, j2k_read_tlm},
    {J2K_MS_PLM, J2K_STATE_MH, j2k_read_plm},
    {J2K_MS_PLT, J2K_STATE_TPH, j2k_read_plt},
    {J2K_MS_PPM, J2K_STATE_MH, j2k_read_ppm},
    {J2K_MS_PPT, J2K_STATE_TPH, j2k_read_ppt},
    {J2K_MS_SOP, 0, 0},
    {J2K_MS_CRG, J2K_STATE_MH, j2k_read_crg},
    {J2K_MS_COM, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_com},
    {0, J2K_STATE_MH|J2K_STATE_TPH, j2k_read_unk}
};

j2k_dec_mstabent_t *j2k_dec_mstab_lookup(int id) {
    j2k_dec_mstabent_t *e;
    for (e=j2k_dec_mstab; e->id!=0; e++) {
        if (e->id==id) {
            break;
        }
    }
    return e;
}

// j2k_release is used to free up all working data allocated
// during the j2k_decode operation.

extern tcd_image_t tcd_image;
 
LIBJ2K_API void j2k_release(j2k_image_t *img, j2k_cp_t *cp) 
{
   int c,b,r, tileno;
   
   if (cp)
   {
      for (tileno=0; tileno<cp->tw*cp->th; tileno++)
      {
         tcd_tile_t *tile = &tcd_image.tiles[tileno];

         if (j2k_tile_data[tileno])
            free(j2k_tile_data[tileno]);

         if (j2k_tile_len)
            free(j2k_tile_len);

         for (c=0;c<tile->numcomps;c++)
         {
            tcd_tilecomp_t *tc = &tile->comps[c];

            if (tc->data)
               free(tc->data);

            if (tc->resolutions)
            {
               for (r=0;r<tc->numresolutions;r++)
               {
                  tcd_resolution_t *res = &tc->resolutions[r];

                  for (b=0;b<res->numbands;b++)
                  {
                     tcd_band_t *band = &res->bands[b];

                     if (band->precincts)
                     {
                         if (band->precincts->cblks)
                            free(band->precincts->cblks);

                         if (band->precincts->incltree)
                         {
                            if (band->precincts->incltree->nodes)
                              free(band->precincts->incltree->nodes);

                            free(band->precincts->incltree);
                         }

                         if (band->precincts->imsbtree)
                         {
                             if (band->precincts->imsbtree->nodes)
                                free(band->precincts->imsbtree->nodes);

                            free(band->precincts->imsbtree);
                         }

                         free(band->precincts);
                     }
                  }
               }

               free(tc->resolutions);
            }
         }

         free(tile->comps);
      }

      free(tcd_image.tiles);
      tcd_image.tiles = NULL;

      if (cp->tcps)
      {
         if (cp->tcps->tccps)
            free(cp->tcps->tccps);

         free(cp->tcps);
      }
      
      free(cp);
         
      if (j2k_tile_data)
         free(j2k_tile_data);
   }

   if (img)
   {
      for (c=0; c< img->numcomps; c++)
      {
         if (img->comps[c].data)
            free(img->comps[c].data);
      }

      if (img->comps)
         free (img->comps);

      free(img);
   }

   if (j2k_default_tcp.tccps)
   {
      free(j2k_default_tcp.tccps);
      j2k_default_tcp.tccps = NULL;
   }
}



LIBJ2K_API int j2k_decode(unsigned char *src, int len, j2k_image_t **img, j2k_cp_t **cp) {

   // Initialize globals ...
   j2k_eot = NULL;
   j2k_img = NULL;
   j2k_cp  = NULL;
   j2k_tile_data = NULL;
   j2k_tile_len = NULL;
   j2k_default_tcp.tccps = NULL;
   
   /* Initialize the logging */
   init_logging();

   if (setjmp(j2k_error)) {
        if (j2k_state!=J2K_STATE_MT) {
            log_print(LOG_ERROR, "WARNING: incomplete bitstream\n");
            return 0;
        }
        return cio_numbytes();
    }
    j2k_img=(j2k_image_t*)malloc(sizeof(j2k_image_t));
    j2k_cp=(j2k_cp_t*)malloc(sizeof(j2k_cp_t));
    *img=j2k_img;
    *cp=j2k_cp;
    j2k_state=J2K_STATE_MHSOC;
    cio_init(src, len);
    for (;;) {
        j2k_dec_mstabent_t *e;
        int id=cio_read(2);
        if (id>>8!=0xff) {
            log_print(LOG_WARNING, "%.8x: expected a marker instead of %x\n", cio_tell()-2, id);
            return 0;
        }
        e=j2k_dec_mstab_lookup(id);
        if (!(j2k_state & e->states)) {
            log_print(LOG_WARNING, "%.8x: unexpected marker %x\n", cio_tell()-2, id);
            return 0;
        }
        if (e->handler) {
            (*e->handler)();
        }
    }
}
