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

#ifndef _J2KCODEC_H
#define _J2KCODEC_H

#ifdef __cplusplus
extern "C"
{
#endif

int j2kcodec_encoder_init();
int j2kcodec_encode(unsigned char *src, unsigned char **dest);

int j2kcodec_decoder_init();
int j2kcodec_decode(unsigned char *src, int len, unsigned char **dest);

#ifdef __cplusplus
}
#endif
  
#endif
