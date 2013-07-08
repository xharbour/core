/*
 * $Id: sxfname.c 9723 2012-10-02 09:18:08Z andijahja $
 */

/*
 * Harbour Project source code:
 *    SIX compatible function:
 *          Sx_FNameParser()
 *
 * Copyright 2007 Przemyslaw Czerpak <druzus / at / priv.onet.pl>
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
#include "hbapifs.h"
#include "hbset.h"

HB_FUNC( SX_FNAMEPARSER )
{
   const char * szFileName = hb_parc( 1 );

   if( szFileName )
   {
      char        szPathBuf[ HB_PATH_MAX ];
      PHB_FNAME   pFileName;
      ULONG       ulLen;
      char *      pszFree;

      szFileName  = hb_fsNameConv( szFileName, &pszFree );
      pFileName   = hb_fsFNameSplit( szFileName );
      if( pszFree )
         hb_xfree( pszFree );

      if( ! ISLOG( 2 ) || ! hb_parl( 2 ) )
         pFileName->szPath = NULL;
      if( ! ISLOG( 3 ) || ! hb_parl( 3 ) )
         pFileName->szExtension = NULL;

      if( ! hb_setGetTrimFileName() )
      {
         if( pFileName->szName )
         {
            ulLen                                     = ( ULONG ) strlen( pFileName->szName );
            ulLen                                     = ( ULONG ) hb_strRTrimLen( pFileName->szName, ulLen, FALSE );
            pFileName->szName                         = hb_strLTrim( pFileName->szName, ( HB_SIZE * ) &ulLen );
            ( ( char * ) pFileName->szName )[ ulLen ] = '\0';
         }
         if( pFileName->szExtension )
         {
            ulLen                                           = ( ULONG ) strlen( pFileName->szExtension );
            ulLen                                           = ( ULONG ) hb_strRTrimLen( pFileName->szExtension, ulLen, FALSE );
            pFileName->szExtension                          = hb_strLTrim( pFileName->szExtension, ( HB_SIZE * ) &ulLen );
            ( ( char * ) pFileName->szExtension )[ ulLen ]  = '\0';
         }
      }

      hb_retc( hb_fsFNameMerge( szPathBuf, pFileName ) );
      hb_xfree( pFileName );
   }
   else
      hb_retc_null();
}
