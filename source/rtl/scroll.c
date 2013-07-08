/*
 * $Id: scroll.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * SCROLL() function
 *
 * Copyright 1999 David G. Holm <dholm@jsd-llc.com>
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
#include "hbapigt.h"

/* Scrolls a screen region */

HB_FUNC( SCROLL )
{
   int   iMaxRow  = hb_gtMaxRow();
   int   iMaxCol  = hb_gtMaxCol();

   int   iTop;
   int   iLeft;
   int   iBottom;
   int   iRight;

   /* Enforce limits of (0,0) to (MAXROW(),MAXCOL()) */

   iTop = hb_parni( 1 ); /* Defaults to zero on bad type */
   if( iTop < 0 )
   {
      iTop = 0;
   }
   else if( iTop > iMaxRow )
   {
      iTop = iMaxRow;
   }

   iLeft = hb_parni( 2 ); /* Defaults to zero on bad type */
   if( iLeft < 0 )
   {
      iLeft = 0;
   }
   else if( iLeft > iMaxCol )
   {
      iLeft = iMaxCol;
   }

   if( ISNUM( 3 ) )
   {
      iBottom = hb_parni( 3 );
      if( iBottom < iTop )
      {
         return;
      }
      else if( iBottom > iMaxRow )
      {
         iBottom = iMaxRow;
      }
   }
   else
   {
      iBottom = iMaxRow;
   }

   if( ISNUM( 4 ) )
   {
      iRight = hb_parni( 4 );
      if( iRight < iLeft )
      {
         return;
      }
      else if( iRight > iMaxCol )
      {
         iRight = iMaxCol;
      }
   }
   else
   {
      iRight = iMaxCol;
   }

   hb_gtScroll( ( USHORT ) iTop,
                ( USHORT ) iLeft,
                ( USHORT ) iBottom,
                ( USHORT ) iRight,
                ( SHORT ) hb_parni( 5 ),     /* Defaults to zero on bad type */
                ( SHORT ) hb_parni( 6 ) );   /* Defaults to zero on bad type */
}

//-------------------------------------------------------------------//

HB_FUNC( SCROLLFIXED )
{
   hb_gtScroll( ( USHORT ) hb_parni( 1 ),
                ( USHORT ) hb_parni( 2 ),
                ( USHORT ) hb_parni( 3 ),
                ( USHORT ) hb_parni( 4 ),
                ( SHORT ) hb_parni( 5 ),     /* Defaults to zero on bad type */
                ( SHORT ) hb_parni( 6 ) );   /* Defaults to zero on bad type */
}


HB_FUNC( HB_SCROLL )
{
   int   iMaxRow  = hb_gtMaxRow();
   int   iMaxCol  = hb_gtMaxCol();

   int   iTop;
   int   iLeft;
   int   iBottom;
   int   iRight;
   int   iColor;
   int   iChar;

   /* Enforce limits of (0,0) to (MAXROW(),MAXCOL()) */

   iTop = hb_parni( 1 ); /* Defaults to zero on bad type */
   if( iTop < 0 )
      iTop = 0;
   else if( iTop > iMaxRow )
      iTop = iMaxRow;

   iLeft = hb_parni( 2 ); /* Defaults to zero on bad type */
   if( iLeft < 0 )
      iLeft = 0;
   else if( iLeft > iMaxCol )
      iLeft = iMaxCol;

   if( ISNUM( 3 ) )
   {
      iBottom = hb_parni( 3 );
      if( iBottom < 0 )
         iBottom = 0;
      else if( iBottom > iMaxRow )
         iBottom = iMaxRow;
   }
   else
      iBottom = iMaxRow;

   if( ISNUM( 4 ) )
   {
      iRight = hb_parni( 4 );
      if( iRight < 0 )
         iRight = 0;
      else if( iRight > iMaxCol )
         iRight = iMaxCol;
   }
   else
      iRight = iMaxCol;

   if( ISNUM( 7 ) )
      iColor = hb_parni( 7 );
   else if( ISCHAR( 7 ) )
      iColor = hb_gtColorToN( hb_parc( 7 ) );
   else
      iColor = -1;

   if( ISNUM( 8 ) )
      iChar = hb_parni( 8 );
   else if( ISCHAR( 8 ) )
      iChar = ( UCHAR ) hb_parc( 8 )[ 0 ];
   else
      iChar = -1;

   hb_gtScrollEx( iTop,
                  iLeft,
                  iBottom,
                  iRight,
                  iColor,
                  iChar,
                  hb_parni( 5 ),    /* Defaults to zero on bad type */
                  hb_parni( 6 ) );  /* Defaults to zero on bad type */
}
