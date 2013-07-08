/*
 * $Id: memofile.c 9724 2012-10-02 10:36:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * MEMOWRIT()/MEMOREAD() functions
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
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
#include "hbapiitm.h"
#include "hbapifs.h"

/* NOTE: CA-Cl*pper has 64K (65516 bytes exactly) limit on read, in Harbour
         this limit is extended, so we are not *strictly* compatible here.
         [vszakats] */

HB_FUNC( MEMOREAD )
{
   PHB_ITEM pFileName = hb_param( 1, HB_IT_STRING );

   if( pFileName )
   {
      HB_FHANDLE fhnd = hb_fsOpen( hb_itemGetCPtr( pFileName ), FO_READ | FO_SHARED | FO_PRIVATE );

      if( fhnd != FS_ERROR )
      {
         ULONG ulSize = hb_fsSeek( fhnd, 0, FS_END );

         if( ulSize != 0 )
         {
            void * pbyBuffer;

            /* Don't read the file terminating EOF character */

            #if ! defined( HB_OS_UNIX )
            {
               BYTE byEOF = HB_CHAR_NUL;

               hb_fsSeek( fhnd, -1, FS_END );
               hb_fsRead( fhnd, &byEOF, sizeof( BYTE ) );

               if( byEOF == HB_CHAR_EOF )
                  ulSize--;
            }
            #endif

            pbyBuffer = hb_xgrab( ulSize + sizeof( char ) );

            hb_fsSeek( fhnd, 0, FS_SET );
            hb_fsReadLarge( fhnd, pbyBuffer, ulSize );

            hb_retclen_buffer( ( char * ) pbyBuffer, ulSize );
         }
         else
            hb_retc( NULL );

         hb_fsClose( fhnd );
      }
      else
         hb_retc( NULL );
   }
   else
      hb_retc( NULL );
}

static BOOL hb_memowrit( BOOL bWriteEOF )
{
   PHB_ITEM pFileName   = hb_param( 1, HB_IT_STRING );
   PHB_ITEM pString     = hb_param( 2, HB_IT_STRING );
   BOOL     bRetVal     = FALSE;

   if( pFileName && pString )
   {
      HB_FHANDLE fhnd = hb_fsCreate( hb_itemGetCPtr( pFileName ), FC_NORMAL );

      if( fhnd != FS_ERROR )
      {
         HB_SIZE ulSize = hb_itemGetCLen( pString );

         bRetVal = ( hb_fsWriteLarge( fhnd, hb_itemGetCPtr( pString ), ulSize ) == ulSize );

         /* NOTE: CA-Cl*pper will add the EOF even if the write failed. [vszakats] */
         /* NOTE: CA-Cl*pper will not return .F. when the EOF could not be written. [vszakats] */
#if ! defined( HB_OS_UNIX )
         if( bWriteEOF )  /* if true, then write EOF */
         {
            BYTE byEOF = HB_CHAR_EOF;
            hb_fsWrite( fhnd, &byEOF, sizeof( BYTE ) );
         }
#else
         HB_SYMBOL_UNUSED( bWriteEOF );
#endif

         hb_fsClose( fhnd );
      }
   }

   return bRetVal;
}

HB_FUNC( HB_MEMOWRIT )
{
   hb_retl( hb_memowrit( FALSE ) );
}

HB_FUNC( MEMOWRIT )
{
#ifdef HB_EXTENSION
   hb_retl( hb_memowrit( hb_parinfo( 0 ) == 3 && ISLOG( 3 ) ? hb_parl( 3 ) : TRUE ) );
#else
   hb_retl( hb_memowrit( TRUE ) );
#endif
}
