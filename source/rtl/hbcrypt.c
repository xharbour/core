/*
 * $Id: hbcrypt.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Cryptography for xharbour
 *
 * Copyright 2003 Giancarlo Niccolai <giancarlo@niccolai.ws>
 * www - http://www.xharbour.org
 * SEE ALSO COPYRIGHT NOTICE FOR NXS BELOW.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

/***************************************************************
 * NXS aglorithm is FREE SOFTWARE. It can be reused for any
 * purpose, provided that this copyright notice is still present
 * in the software.
 *
 * This program is distributed WITHOUT ANY WARRANTY that it can
 * fit any particular need.
 *
 * NXS author is Giancarlo Niccolai <giancarlo@niccolai.ws>
 *
 * Adler 32 CRC is copyrighted by Martin Adler (see note below)
 **************************************************************/


#include "hbapi.h"
#include "hbapiitm.h"
#include "hbzlib.h"
#include "hbnxs.h"

#define BASE 65521L /* largest prime smaller than 65536 */

/* Giancarlo Niccolai's x scrambler algorithm
 * Prerequisites:
 * 1) source must be at least 8 bytes long.
 * 2) key must be at least 6 characters long.
 *    Optimal lenght is about 12 to 16 bytes. Maximum keylen is 512 bytes
 * 3) cipher must be preallocated with srclen bytes
 */

void nxs_crypt(
   const unsigned char * source, HB_SIZE srclen,
   const unsigned char * key, HB_SIZE keylen,
   BYTE * cipher )
{

   if( keylen > NXS_MAX_KEYLEN )
   {
      keylen = NXS_MAX_KEYLEN;
   }

#ifdef DEBUG_0
   HB_MEMCPY( cipher, source, srclen );
#endif

   /* pass one: scramble the source using the key */
   nxs_scramble( source, srclen, key, keylen, cipher );

   /* pass two: xor the source with the key
      threebit mutual shift is done also here */
   nxs_xorcode( cipher, srclen, key, keylen );

   /* pass three: xor the source with the cyclic key */
   nxs_xorcyclic( cipher, srclen, key, keylen );
}

/*decrypting the buffer */

void nxs_decrypt(
   const unsigned char * cipher, HB_SIZE cipherlen,
   const unsigned char * key, HB_SIZE keylen,
   BYTE * result )
{
   if( keylen > NXS_MAX_KEYLEN )
   {
      keylen = NXS_MAX_KEYLEN;
   }

   HB_MEMCPY( result, cipher, ( size_t ) cipherlen );

   /* pass one: xor the source with the cyclic key */
   nxs_xorcyclic( result, cipherlen, key, keylen );

   /* pass two: xor the source with the key
      threebit mutual shift is done also here */
   nxs_xordecode( result, cipherlen, key, keylen );

   /* pass three: unscramble the source using the key */
   nxs_unscramble( result, cipherlen, key, keylen );
}

/* This function scrambles the source using the letter ordering in the
 * key. */
void nxs_scramble(
   const unsigned char * source, HB_SIZE srclen,
   const unsigned char * key, HB_SIZE keylen,
   BYTE * cipher )
{
   int      scramble[ NXS_MAX_KEYLEN ];
   HB_SIZE  len;

   if( keylen > NXS_MAX_KEYLEN )
   {
      keylen = NXS_MAX_KEYLEN;
   }

   if( keylen > srclen )
   {
      keylen = srclen;
   }

   /* First step: find key ordering */
   nxs_make_scramble( scramble, key, keylen );

   /* Leave alone the last block */
   len      = keylen > 0 ? ( srclen / keylen ) * keylen : 0;
   nxs_partial_scramble( source, cipher, scramble, len, keylen );

   keylen   = srclen - len;
   nxs_make_scramble( scramble, key, keylen );
   nxs_partial_scramble( source + len, cipher + len, scramble, keylen, keylen );
}

void nxs_partial_scramble(
   const unsigned char * source, BYTE * cipher,
   int * scramble,
   HB_SIZE len, HB_SIZE keylen )
{
   HB_SIZE  pos;
   USHORT   kpos;

   pos   = 0l;
   kpos  = 0;
   while( pos + kpos < len )
   {
      cipher[ pos + scramble[ kpos ] ] = source[ pos + kpos ];
      kpos++;
      if( kpos >= ( USHORT ) keylen )
      {
         kpos  = 0;
         pos   += keylen;
      }
   }
}

/* Reversing scramble process */
void nxs_unscramble(
   BYTE * cipher, HB_SIZE cipherlen,
   const unsigned char * key, HB_SIZE keylen )
{
   int      scramble[ NXS_MAX_KEYLEN ];
   HB_SIZE  len;

   if( keylen > NXS_MAX_KEYLEN )
   {
      keylen = NXS_MAX_KEYLEN;
   }

   if( keylen > cipherlen )
   {
      keylen = cipherlen;
   }

   /* First step: find key ordering */
   nxs_make_scramble( scramble, key, keylen );

   /* Leave alone the last block */
   len      = keylen > 0 ? ( cipherlen / keylen ) * keylen : 0;
   nxs_partial_unscramble( cipher, scramble, len, keylen );

   keylen   = cipherlen - len;
   nxs_make_scramble( scramble, key, keylen );
   nxs_partial_unscramble( cipher + len, scramble, keylen, keylen );
}


void nxs_partial_unscramble(
   BYTE * cipher,
   int * scramble,
   HB_SIZE len, HB_SIZE keylen )
{
   HB_SIZE  pos;
   USHORT   kpos;
   BYTE     buf[ NXS_MAX_KEYLEN ];

   pos   = 0l;
   kpos  = 0;
   while( pos + kpos < len )
   {
      buf[ kpos ] = cipher[ pos + scramble[ kpos ] ];
      kpos++;
      if( kpos >= ( USHORT ) keylen )
      {
         HB_MEMCPY( cipher + pos, buf, ( size_t ) keylen );
         kpos  = 0;
         pos   += keylen;
      }
   }
}

/* pass two: xor the source with the key
   threebit mutual shift is done also here */
void nxs_xorcode(
   BYTE * cipher, HB_SIZE cipherlen,
   const unsigned char * key, HB_SIZE keylen )
{
   ULONG    pos      = 0l;
   USHORT   keypos   = 0;
   BYTE     c_bitrest;

   c_bitrest = cipher[ 0 ] >> 5;

   while( pos < cipherlen )
   {
      cipher[ pos ] <<= 3;

      if( keypos == ( USHORT ) keylen - 1 || pos == cipherlen - 1 )
      {
         cipher[ pos ] |= c_bitrest;
      }
      else
      {
         cipher[ pos ] |= cipher[ pos + 1 ] >> 5;
      }

      cipher[ pos ] ^= key[ keypos ];
      keypos++;
      pos++;

      if( keypos == ( USHORT ) keylen )
      {
         keypos      = 0;
         c_bitrest   = cipher[ pos ] >> 5;
      }
   }
}

void nxs_xordecode(
   BYTE * cipher, HB_SIZE cipherlen,
   const unsigned char * key, HB_SIZE keylen )
{
   ULONG    pos      = 0l;
   USHORT   keypos   = 0;
   BYTE     c_bitrest, c_bitleft;

   /* A very short block? */
   if( keylen > cipherlen - pos )
   {
      keylen = ( USHORT ) ( cipherlen - pos );
   }
   c_bitleft = ( cipher[ keylen - 1 ] ^ key[ keylen - 1 ] ) << 5;

   while( pos < cipherlen )
   {
      cipher[ pos ]  ^= key[ keypos ];

      c_bitrest      = cipher[ pos ] << 5;
      cipher[ pos ]  >>= 3;
      cipher[ pos ]  |= c_bitleft;
      c_bitleft      = c_bitrest;

      keypos++;
      pos++;

      if( keypos == ( USHORT ) keylen )
      {
         keypos = 0;
         /* last block */
         if( keylen > cipherlen - pos )
         {
            keylen = ( USHORT ) ( cipherlen - pos );
         }

         c_bitleft = ( cipher[ pos + keylen - 1 ] ^ key[ keylen - 1 ] ) << 5;
      }
   }
}

/* pass three: xor the source with the cyclic key */
void nxs_xorcyclic(
   BYTE * cipher, HB_SIZE cipherlen,
   const unsigned char * key, HB_SIZE keylen )
{
   HB_SIZE  pos = 0l, crcpos = 0l;
   HB_SIZE  crc1, crc2, crc3;
   HB_SIZE  crc1l, crc2l, crc3l;

   /* Build the cyclic key seed */
   crc1  = keylen >= 2 ? adler32( 0, key + 0, ( uInt ) keylen - 2 ) : 1;
   crc2  = keylen >= 4 ? adler32( 0, key + 2, ( uInt ) keylen - 4 ) : 1;
   crc3  = keylen >= 2 ? adler32( 0, key + 1, ( uInt ) keylen - 2 ) : 1;

   crc1l = crc1 = nxs_cyclic_sequence( crc1 );
   crc2l = crc2 = nxs_cyclic_sequence( crc2 );
   crc3l = crc3 = nxs_cyclic_sequence( crc3 );

   while( pos < cipherlen )
   {
      if( crcpos < 4 )
      {
         /* this ensures portability across platforms */
         cipher[ pos ]  ^= ( BYTE ) ( crc1l % 256 );
         crc1l          /= 256l;
      }
      else if( crcpos < 8 )
      {
         cipher[ pos ]  ^= ( BYTE ) ( crc2l % 256 );
         crc2l          /= 256l;
      }
      else
      {
         cipher[ pos ]  ^= ( BYTE ) ( crc3l % 256 );
         crc3l          /= 256l;
      }
      crcpos++;
      pos++;

      if( crcpos == 12 )
      {
         crcpos   = 0;
         crc1l    = crc1 = nxs_cyclic_sequence( crc1 );
         crc2l    = crc2 = nxs_cyclic_sequence( crc2 );
         crc3l    = crc3 = nxs_cyclic_sequence( crc3 );
      }
   }
}

HB_SIZE nxs_cyclic_sequence( HB_SIZE input )
{
   HB_SIZE  first    = input & 0xffff;
   HB_SIZE  second   = input >> 16;
   HB_SIZE  ret      = ( ( second * BASE * BASE ) & 0xffff ) |
                       ( ( first * BASE * BASE ) & 0xffff0000 );

   return ret;
}


void nxs_make_scramble( int * scramble, const unsigned char * key, HB_SIZE keylen )
{
   ULONG i, j, tmp;

   for( i = 0; i < keylen; i++ )
   {
      scramble[ i ] = i;
   }

   for( i = 0; i < keylen; i++ )
   {
      for( j = i + 1; j < keylen; j++ )
      {
         if( key[ scramble[ j ] ] < key[ scramble[ i ] ] )
         {
            tmp            = scramble[ j ];
            scramble[ j ]  = scramble[ i ];
            scramble[ i ]  = tmp;
            j              = i;
         }
      }
   }
}

/*
 * END OF NXS
 */

/***********************************
 * XHarbour implementation
 ************************************/

/*****
 * Encrypt a text using a key
 * Usage:
 * HB_Crypt( cSource, cKey ) --> cCipher
 */

HB_FUNC( HB_CRYPT )
{
   PHB_ITEM pSource  = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pKey     = hb_param( 2, HB_IT_ANY );

   BYTE *   cRes     = ( BYTE * ) hb_xgrab( hb_itemGetCLen( pSource ) + 8 );

   nxs_crypt(
      ( BYTE * ) hb_itemGetCPtr( pSource ), hb_itemGetCLen( pSource ),
      ( BYTE * ) hb_itemGetCPtr( pKey ), hb_itemGetCLen( pKey ),
      cRes );

   hb_retclenAdopt( ( char * ) cRes, hb_itemGetCLen( pSource ) );
}

/*****
 * Decrypt a text using a key
 * Usage:
 * HB_Decrypt( cCrypt, cKey ) --> cSource
 */

HB_FUNC( HB_DECRYPT )
{
   PHB_ITEM pSource  = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pKey     = hb_param( 2, HB_IT_ANY );

   BYTE *   cRes     = ( BYTE * ) hb_xgrab( hb_itemGetCLen( pSource ) + 8 );

   nxs_decrypt(
      ( BYTE * ) hb_itemGetCPtr( pSource ), hb_itemGetCLen( pSource ),
      ( BYTE * ) hb_itemGetCPtr( pKey ), hb_itemGetCLen( pKey ),
      cRes );

   hb_retclenAdopt( ( char * ) cRes, hb_itemGetCLen( pSource ) );
}
