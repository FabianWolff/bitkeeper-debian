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

/* Implements ECC over Z/pZ for curve y^2 = x^3 - 3x + b
 *
 * All curves taken from NIST recommendation paper of July 1999
 * Available at http://csrc.nist.gov/cryptval/dss.htm
 */
#include "tomcrypt.h"

/**
  @file ecc_make_key.c
  ECC Crypto, Tom St Denis
*/  

#ifdef MECC

/**
  Make a new ECC key 
  @param prng         An active PRNG state
  @param wprng        The index of the PRNG you wish to use
  @param keysize      The keysize for the new key (in octets from 20 to 65 bytes)
  @param key          [out] Destination of the newly created key
  @return CRYPT_OK if successful, upon error all allocated memory will be freed
*/
int ecc_make_key(prng_state *prng, int wprng, int keysize, ecc_key *key)
{
   int            x, err;
   ecc_point     *base;
   void          *prime;
   unsigned char *buf;

   LTC_ARGCHK(key != NULL);
   LTC_ARGCHK(ltc_mp.name != NULL);

   /* good prng? */
   if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
      return err;
   }

   /* find key size */
   for (x = 0; (keysize > ltc_ecc_sets[x].size) && (ltc_ecc_sets[x].size != 0); x++);
   keysize = ltc_ecc_sets[x].size;

   if (keysize > ECC_MAXSIZE || ltc_ecc_sets[x].size == 0) {
      return CRYPT_INVALID_KEYSIZE;
   }
   key->idx = x;

   /* allocate ram */
   base = NULL;
   buf  = XMALLOC(ECC_MAXSIZE);
   if (buf == NULL) {
      return CRYPT_MEM;
   }

   /* make up random string */
   if (prng_descriptor[wprng].read(buf, (unsigned long)keysize, prng) != (unsigned long)keysize) {
      err = CRYPT_ERROR_READPRNG;
      goto LBL_ERR2;
   }

   /* setup the key variables */
   if ((err = mp_init_multi(&key->pubkey.x, &key->pubkey.y, &key->pubkey.z, &key->k, &prime, NULL)) != CRYPT_OK) {
      goto done;
   }
   base = ltc_ecc_new_point();
   if (base == NULL) {
      mp_clear_multi(key->pubkey.x, key->pubkey.y, key->pubkey.z, key->k, prime, NULL);
      err = CRYPT_MEM;
      goto done;
   }

   /* read in the specs for this key */
   if ((err = mp_read_radix(prime, (char *)ltc_ecc_sets[key->idx].prime, 16)) != CRYPT_OK)      { goto done; }
   if ((err = mp_read_radix(base->x, (char *)ltc_ecc_sets[key->idx].Gx, 16)) != CRYPT_OK)       { goto done; }
   if ((err = mp_read_radix(base->y, (char *)ltc_ecc_sets[key->idx].Gy, 16)) != CRYPT_OK)       { goto done; }
   mp_set(base->z, 1);
   if ((err = mp_read_unsigned_bin(key->k, (unsigned char *)buf, keysize)) != CRYPT_OK)         { goto done; }

   /* make the public key */
   if ((err = ltc_mp.ecc_ptmul(key->k, base, &key->pubkey, prime, 1)) != CRYPT_OK)              { goto done; }
   key->type = PK_PRIVATE;

   /* free up ram */
   err = CRYPT_OK;
done:
   ltc_ecc_del_point(base);
   mp_clear(prime);
LBL_ERR2:
#ifdef LTC_CLEAN_STACK
   zeromem(buf, ECC_MAXSIZE);
#endif

   XFREE(buf);

   return err;
}

#endif
/* $Source: /cvs/libtom/libtomcrypt/src/pk/ecc/ecc_make_key.c,v $ */
/* $Revision: 1.6 $ */
/* $Date: 2006/03/31 14:15:35 $ */

