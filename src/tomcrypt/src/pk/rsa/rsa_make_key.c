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
#include "tomcrypt.h"

/**
  @file rsa_make_key.c
  RSA key generation, Tom St Denis
*/  

#ifdef MRSA

/** 
   Create an RSA key
   @param prng     An active PRNG state
   @param wprng    The index of the PRNG desired
   @param size     The size of the modulus (key size) desired (octets)
   @param e        The "e" value (public key).  e==65537 is a good choice
   @param key      [out] Destination of a newly created private key pair
   @return CRYPT_OK if successful, upon error all allocated ram is freed
*/
int rsa_make_key(prng_state *prng, int wprng, int size, long e, rsa_key *key)
{
   void *p, *q, *tmp1, *tmp2, *tmp3;
   int    err;

   LTC_ARGCHK(ltc_mp.name != NULL);
   LTC_ARGCHK(key != NULL);

   if ((size < (MIN_RSA_SIZE/8)) || (size > (MAX_RSA_SIZE/8))) {
      return CRYPT_INVALID_KEYSIZE;
   }

   if ((e < 3) || ((e & 1) == 0)) {
      return CRYPT_INVALID_ARG;
   }

   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   if ((err = mp_init_multi(&p, &q, &tmp1, &tmp2, &tmp3, NULL)) != CRYPT_OK) {
      return err;
   }

   /* make primes p and q (optimization provided by Wayne Scott) */
   if ((err = mp_set_int(tmp3, e)) != CRYPT_OK) { goto error; }            /* tmp3 = e */

   /* make prime "p" */
   do {
       if ((err = rand_prime( p, size/2, prng, wprng)) != CRYPT_OK)  { goto done; }
       if ((err = mp_sub_d( p, 1,  tmp1)) != CRYPT_OK)               { goto error; }  /* tmp1 = p-1 */
       if ((err = mp_gcd( tmp1,  tmp3,  tmp2)) != CRYPT_OK)          { goto error; }  /* tmp2 = gcd(p-1, e) */
   } while (mp_cmp_d( tmp2, 1) != 0);                                                /* while e divides p-1 */

   /* make prime "q" */
   do {
       if ((err = rand_prime( q, size/2, prng, wprng)) != CRYPT_OK) { goto done; }
       if ((err = mp_sub_d( q, 1,  tmp1)) != CRYPT_OK)               { goto error; } /* tmp1 = q-1 */
       if ((err = mp_gcd( tmp1,  tmp3,  tmp2)) != CRYPT_OK)          { goto error; } /* tmp2 = gcd(q-1, e) */
   } while (mp_cmp_d( tmp2, 1) != 0);                                               /* while e divides q-1 */

   /* tmp1 = lcm(p-1, q-1) */
   if ((err = mp_sub_d( p, 1,  tmp2)) != CRYPT_OK)                  { goto error; } /* tmp2 = p-1 */
                                                                   /* tmp1 = q-1 (previous do/while loop) */
   if ((err = mp_lcm( tmp1,  tmp2,  tmp1)) != CRYPT_OK)             { goto error; } /* tmp1 = lcm(p-1, q-1) */

   /* make key */
   if ((err = mp_init_multi(&key->e, &key->d, &key->N, &key->dQ, &key->dP,
                     &key->qP, &key->p, &key->q, NULL)) != CRYPT_OK) {
      goto error;
   }

   if ((err = mp_set_int( key->e, e)) != CRYPT_OK)                     { goto error2; } /* key->e =  e */
   if ((err = mp_invmod( key->e,  tmp1,  key->d)) != CRYPT_OK)         { goto error2; } /* key->d = 1/e mod lcm(p-1,q-1) */
   if ((err = mp_mul( p,  q,  key->N)) != CRYPT_OK)                    { goto error2; } /* key->N = pq */

   /* optimize for CRT now */
   /* find d mod q-1 and d mod p-1 */
   if ((err = mp_sub_d( p, 1,  tmp1)) != CRYPT_OK)                     { goto error2; } /* tmp1 = q-1 */
   if ((err = mp_sub_d( q, 1,  tmp2)) != CRYPT_OK)                     { goto error2; } /* tmp2 = p-1 */
   if ((err = mp_mod( key->d,  tmp1,  key->dP)) != CRYPT_OK)           { goto error2; } /* dP = d mod p-1 */
   if ((err = mp_mod( key->d,  tmp2,  key->dQ)) != CRYPT_OK)           { goto error2; } /* dQ = d mod q-1 */
   if ((err = mp_invmod( q,  p,  key->qP)) != CRYPT_OK)                { goto error2; } /* qP = 1/q mod p */

   if ((err = mp_copy( p,  key->p)) != CRYPT_OK)                       { goto error2; }
   if ((err = mp_copy( q,  key->q)) != CRYPT_OK)                       { goto error2; }

   /* set key type (in this case it's CRT optimized) */
   key->type = PK_PRIVATE;

   /* return ok and free temps */
   err       = CRYPT_OK;
   goto done;
error2:
   mp_clear_multi( key->d,  key->e,  key->N,  key->dQ,  key->dP,
                   key->qP,  key->p,  key->q, NULL);
error:
done:
   mp_clear_multi( tmp3,  tmp2,  tmp1,  p,  q, NULL);
   return err;
}

#endif

/* $Source: /cvs/libtom/libtomcrypt/src/pk/rsa/rsa_make_key.c,v $ */
/* $Revision: 1.12 $ */
/* $Date: 2006/03/31 14:15:35 $ */
