/*
 * $Id: bit3.c 9692 2012-09-16 11:25:59Z andijahja $
 */

/*
 * Harbour Project source code:
 *   CT3 Number and bit manipulation functions: - NUMANDX()
 *                                              - NUMORX()
 *                                              - NUMXORX()
 *                                              - NUMNOTX()
 *                                              - NUMROLX()
 *                                              - NUMMIRRX()
 *
 * Copyright 2001 Walter Negro - FOEESITRA" <waltern@foeesitra.org.ar>
 * www - http://www.harbour-project.org
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


#include "hbapi.h"
#include "ct.h"

typedef BOOL * BOOLP;
typedef LONG * LONGP;
typedef USHORT * USHORTP;

static LONG __getparam( int iParam );
static LONG __numand( LONG wNum1, LONG wNum2 );
static LONG __numor( LONG wNum1, LONG wNum2 );
static LONG __numxor( LONG wNum1, LONG wNum2 );
static LONG __numnot( LONG wNum1, LONG wNum2 );
static LONG __numfun( int iPCount, LONG ( * operation )( LONG wNum1, LONG wNum2 ), BOOLP pbOk );
static void sizeofbits( USHORTP pusBytes, LONGP plPattern, LONGP plTestMSB );

HB_FUNC( NUMANDX )
{
   int   iPCount;
   LONG  lNumOp;
   BOOL  bOk;

   iPCount  = hb_pcount();

   lNumOp   = __numfun( iPCount, ( LONG ( * )( LONG wNum1, LONG wNum2 ) )( __numand ), &bOk );

   if( bOk )
      hb_retnl( lNumOp );
   else
      hb_ret();
}

HB_FUNC( NUMORX )
{
   int   iPCount;
   LONG  lNumOp;
   BOOL  bOk;

   iPCount  = hb_pcount();

   lNumOp   = __numfun( iPCount, ( LONG ( * )( LONG wNum1, LONG wNum2 ) )( __numor ), &bOk );

   if( bOk )
      hb_retnl( lNumOp );
   else
      hb_ret();
}

HB_FUNC( NUMXORX )
{
   int   iPCount;
   LONG  lNumOp;
   BOOL  bOk;

/*  iPCount = hb_pcount(); */

   iPCount  = 3;

   lNumOp   = __numfun( iPCount, ( LONG ( * )( LONG wNum1, LONG wNum2 ) )( __numxor ), &bOk );

   if( bOk )
      hb_retnl( lNumOp );
   else
      hb_ret();
}

HB_FUNC( NUMNOTX )
{
   int   iPCount;
   LONG  lNumOp;
   BOOL  bOk;

/*  iPCount = hb_pcount(); */

   iPCount  = 2;

   lNumOp   = __numfun( iPCount, ( LONG ( * )( LONG wNum1, LONG wNum2 ) )( __numnot ), &bOk );

   if( bOk )
      hb_retnl( lNumOp );
   else
      hb_ret();
}

HB_FUNC( NUMROLX )
{
   LONG     lNum1, lNumBak, lPattern, lTestRol;
   USHORT   usBytes, usFor, usNum2;

   if( ISNUM( 2 ) || ISCHAR( 2 ) )
   {
      lNum1    = __getparam( 2 );            /* Number to do ROL */
      usNum2   = ( USHORT ) __getparam( 3 ); /* Iterations       */

      sizeofbits( &usBytes, &lPattern, &lTestRol );

      usNum2   = usNum2 % usBytes;        /* Set usNum2 < usBytes  */

      lNumBak  = lNum1 & lPattern;        /* lNumBak contain the section
                                             to doesn't ROL               */

      for( usFor = 1; usFor <= usNum2; usFor++ )
      {
         if( lNum1 & lTestRol )  /* Test if MSB is ON */
         {
            lNum1 = lNum1 << 1;
            lNum1 = lNum1 | 1;   /* Simulate that the MSB move to LSB */
         }
         else
            lNum1 = lNum1 << 1;
      }
      /* Set the section not ROLed */
      lNum1 = ( lNum1 & ( ~lPattern ) ) | lNumBak;

      hb_retnl( lNum1 );
   }
   else
      hb_ret();
}

HB_FUNC( NUMMIRRX )
{
   LONG     lNum1, lPattern, lTestMSB, lNumBak, lMirror = 0;
   USHORT   usBytes, usFor;

   if( ISNUM( 2 ) || ISCHAR( 2 ) )
   {
      lNum1 = __getparam( 2 );

      sizeofbits( &usBytes, &lPattern, &lTestMSB );

      lNumBak = lNum1 & lPattern;

      for( usFor = 1; usFor <= usBytes; usFor++ )
      {
         if( lNum1 & 1 )
         {

            lMirror  = lMirror << 1;   /* if the LSB of lNum1 == 1 then */
            lMirror  = lMirror | 1;    /* set the LSB of lMirror = 1    */
         }
         else
            lMirror = lMirror << 1;

         lNum1 = lNum1 >> 1;

      }
      lMirror = ( lMirror & ( ~lPattern ) ) | lNumBak;

      hb_retnl( lMirror );
   }
   else
      hb_ret();

}

static LONG __getparam( int iParam )
{
   if( ISCHAR( iParam ) )
      return ( LONG ) hb_hextonum( hb_parcx( iParam ) );
   else
      return hb_parnl( iParam );
}

static LONG __numand( LONG lNum1, LONG lNum2 )
{
   return lNum1 & lNum2;
}

static LONG __numor( LONG lNum1, LONG lNum2 )
{
   return lNum1 | lNum2;
}


static LONG __numxor( LONG lNum1, LONG lNum2 )
{
   return lNum1 ^ lNum2;
}

static LONG __numnot( LONG lNum1, LONG lNum2 )
{
   HB_SYMBOL_UNUSED( lNum2 );
   return ~lNum1;
}

static LONG __numfun( int iPCount, LONG ( * operation )( LONG wNum1, LONG wNum2 ), BOOLP pbOk )
{
   LONG     lNumOp = 0;
   LONG     lNum1, lNum2;
   LONG     lPattern, lTestMSB;
   USHORT   usBytes;
   int      iFor;

   if( ISNUM( 1 ) || ISNIL( 1 ) )
   {
      sizeofbits( &usBytes, &lPattern, &lTestMSB );

      if( ISNUM( 2 ) || ISCHAR( 2 ) )
      {
         lNum1 = __getparam( 2 );

         if( iPCount == 2 )
            /*  If unary operation: NOT                           */
            lNumOp = ( *operation )( lNum1, 0 );

         else
         {

            for( iFor = 3; iFor <= iPCount; iFor++ )
            {
               if( ISNUM( iFor ) || ISCHAR( iFor ) )
               {
                  lNum2 = __getparam( iFor );


                  /*  Call to operation: AND, OR, XOR                   */
                  lNumOp = ( *operation )( lNum1, lNum2 );

               }
               else
               {
                  /*  If error in parameter then return -1              */
                  *pbOk = FALSE;
                  return -1;
               }

               /*  Copy result to first parameter if multi operation */
               lNum1 = lNumOp;
            }

         }

      }
      else
      {

         /*  If error in parameter then return -1              */
         *pbOk = FALSE;
         return -1;
      }

      /*  Return result of operation */
      lNumOp   = ( lNumOp & lTestMSB ) ? lNumOp | lPattern : lNumOp & ( ~lPattern );

      *pbOk    = TRUE;

      return lNumOp;
   }
   else
   {
      *pbOk = FALSE;
      return -1;
   }
}

static void sizeofbits( USHORTP pusBytes, LONG * plPattern, LONG * plTestMSB )
{

   *pusBytes = ( ( ISNIL( 1 ) || hb_parni( 1 ) == 0 ) ? sizeof( int ) * 8
                 : ( USHORT ) hb_parni( 1 ) );

   if( *pusBytes > sizeof( LONG ) * 8 )
      *pusBytes = *pusBytes % ( sizeof( LONG ) * 8 );

   *plPattern  = *pusBytes == ( sizeof( LONG ) * 8 ) ? 0 : ( -1 ) << *pusBytes;
   *plTestMSB  = *pusBytes == 0 ? 0 : 1 << ( *pusBytes - 1 );
}

