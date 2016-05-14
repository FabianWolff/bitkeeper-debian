/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.com
 */

/**
   @file gcm_mult_h.c
   GCM implementation, do the GF mult, by Tom St Denis
*/
#include "tomcrypt.h"

#if defined(GCM_MODE)
/**
  GCM multiply by H
  @param gcm   The GCM state which holds the H value
  @param I     The value to multiply H by
 */
void gcm_mult_h(gcm_state *gcm, unsigned char *I)
{
   unsigned char T[16];
#ifdef GCM_TABLES
   int x, y;
   XMEMCPY(T, &gcm->PC[0][I[0]][0], 16);
   for (x = 1; x < 16; x++) {
#ifdef LTC_FAST
       for (y = 0; y < 16; y += sizeof(LTC_FAST_TYPE)) {
           *((LTC_FAST_TYPE *)(T + y)) ^= *((LTC_FAST_TYPE *)(&gcm->PC[x][I[x]][y]));
       }
#else
       for (y = 0; y < 16; y++) {
           T[y] ^= gcm->PC[x][I[x]][y];
       }
#endif
   }
#else     
   gcm_gf_mult(gcm->H, I, T); 
#endif
   XMEMCPY(I, T, 16);
}
#endif

/* $Source: /cvs/libtom/libtomcrypt/src/encauth/gcm/gcm_mult_h.c,v $ */
/* $Revision: 1.3 $ */
/* $Date: 2006/03/31 14:15:35 $ */
