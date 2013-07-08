/*
 * $Id: status.c 9597 2012-07-26 22:49:11Z andijahja $
 */

/*
 * Harbour Project source code:
 * Moving indicator for large processes
 *
 * Copyright 2000 Jose Lalin <dezac@corevia.com>
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

#include <ctype.h>

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapigt.h"

#define ST_ROW       1     // Status item display row
#define ST_COL       2     // Status item display column
#define ST_COLOR     3     // Status item color
#define ST_CURRENT   4     // Status item current position in aDisplay

#define ST_LEN       ST_CURRENT  // Length of status array

/* StatusNew( [<nRow>], [<nCol>], [<cColor>] ) --> <aStat>
*/
HB_FUNC( STATUSNEW )
{
   PHB_ITEM pReturn = hb_itemArrayNew( ST_LEN );   /* Create array */
   PHB_ITEM pItem;

   pItem = hb_itemPutNL( NULL, ( ISNUM( ST_ROW ) ? hb_parni( ST_ROW ) : 0 ) );
   hb_itemArrayPut( pReturn, ST_ROW, pItem );
   hb_itemRelease( pItem );

   pItem = hb_itemPutNL( NULL, ( ISNUM( ST_COL ) ? hb_parni( ST_COL ) : 0 ) );
   hb_itemArrayPut( pReturn, ST_COL, pItem );
   hb_itemRelease( pItem );

   pItem = hb_itemPutC( NULL, ( ISCHAR( ST_COLOR ) ? hb_parcx( ST_COLOR ) : "W+/N" ) );
   hb_itemArrayPut( pReturn, ST_COLOR, pItem );
   hb_itemRelease( pItem );

   pItem = hb_itemPutNL( NULL, 1 );
   hb_itemArrayPut( pReturn, ST_CURRENT, pItem );
   hb_itemRelease( pItem );

   hb_itemReturn( pReturn );
   hb_itemRelease( pReturn );
}

/* StatusUpdate( <aStat> ) --> nil
*/
HB_FUNC( STATUSUPDATE )
{
   PHB_ITEM pArray = hb_param( 1, HB_IT_ARRAY );

   if( pArray )
   {
      char * szDisplay  = "|/-\\";
      LONG lCurrent = hb_arrayGetNL( pArray, ST_CURRENT );
      char * szOldColor[ CLR_STRLEN ];
      PHB_ITEM pCurrent = hb_itemNew( NULL );

      lCurrent = ( ++lCurrent > 4 ? 1 : lCurrent );
      hb_itemArrayPut( pArray, ST_CURRENT, hb_itemPutNL( pCurrent, lCurrent ) );

      hb_gtGetColorStr( (char*) szOldColor );
      hb_gtSetColorStr( hb_arrayGetCPtr( pArray, ST_COLOR ) );
      hb_gtWriteAt( (USHORT) hb_arrayGetNL( pArray, ST_ROW ),
                    (USHORT) hb_arrayGetNL( pArray, ST_COL ),
                    ( BYTE * ) szDisplay + lCurrent - 1, 1 );

      hb_gtSetColorStr( (char*) szOldColor );
      hb_itemRelease( pCurrent );
   }
}
