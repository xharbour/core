/*
 * $Id: hbrandom.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Random number generator routine
 *
 * Copyright 2003 Giancarlo Niccolai <gian@niccolai.ws>
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

#include <hbapi.h>
#include <hbmath.h>
#include <stdlib.h>
#include <time.h>

#ifndef HB_OS_WIN
#include <float.h>
#endif

/* Globally available data, no need to MT it */
static volatile int s_fInit = 0;

/*
 * HB_RANDOM
 *
 * HB_RANDOM() --> returns a real value n so that 0 <= n < 1
 * HB_RANDOM( x ) --> returns a real number n so that 0 <= n < x
 * HB_RANDOM( x, y) --> Returns a  real number n so that  x <= n < y
 */
HB_FUNC( HB_RANDOM )
{
   double dRnd = hb_random_num();

   if( ! ISNUM( 1 ) )
      hb_retnd( dRnd );
   else if( ! ISNUM( 2 ) )
      hb_retnd( dRnd * hb_parnd( 1 ) );
   else
   {
      double   dX = hb_parnd( 2 );
      double   dY = hb_parnd( 1 );
      if( dX > dY )
      {
         double dZ = dY;
         dY = dX;
         dX = dZ;
      }
      hb_retnd(  dRnd * ( dY - dX ) + dX );
   }
}

/*
 * HB_RANDOMINT
 *
 * HB_RANDOMINT() --> returns 0 or 1, evenly distributed
 * HB_RANDOMINT( N ) --> returns an integer between 1 and N (inclusive)
 * HB_RANDOMINT( x, y) --> Returns a real number between x and y (inclusive)
 * The integer returned is of the longest type available
 */
HB_FUNC( HB_RANDOMINT )
{
   double dRnd = hb_random_num();

   if( ! ISNUM( 1 ) )
      hb_retni( dRnd >= 0.5 ? 0 : 1 );
   else if( ! ISNUM( 2 ) )
      hb_retnint( ( HB_LONG ) ( 1 + ( dRnd * hb_parnint( 1 ) ) ) );
   else
   {
      HB_LONG  lX = hb_parnint( 1 );
      HB_LONG  lY = hb_parnint( 2 );
      if( lX > lY )
      {
         HB_LONG lZ = lY;
         lY = lX;
         lX = lZ;
      }
      hb_retnint( ( HB_LONG ) ( lX + ( dRnd * ( lY - lX + 1 ) ) ) );
   }
}

HB_FUNC( HB_RANDOMSEED )
{
   srand( ISNUM( 1 ) ? ( unsigned ) hb_parni( 1 ) : ( unsigned ) time( NULL ) );
   s_fInit = TRUE;
}

/* Returns a double value between 0 and 1 */
double hb_random_num( void )
{
   double d1, d2;

   if( ! s_fInit )
   {
      srand( ( unsigned ) time( NULL ) );
      s_fInit = TRUE;
   }

   d1 = ( double ) rand();
   d2 = ( double ) RAND_MAX;
#ifdef HB_OS_WIN
   /* It seems that on win32 platform there some weirdness about EPSILON value so
      that a float division using an epsilon smaller than 1e-10 may be rounded.
      Must dig if it's a borland lib bug or a windows problem.
    */
   d2 += 0.001;
#else
   d2 += DBL_EPSILON;
#endif

   return d1 / d2;
}
