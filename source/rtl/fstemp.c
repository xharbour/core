/*
 * $Id: fstemp.c 9922 2013-02-07 12:02:27Z druzus $
 */

/*
 * Harbour Project source code:
 * HB_FTEMPCREATE() function
 *
 * Copyright 2000-2001 Jose Lalin <dezac@corevia.com>
 *                     Viktor Szakats <viktor.szakats@syenar.hu>
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

#ifndef HB_OS_WIN_USED
   #define HB_OS_WIN_USED
#endif

#include "hbapi.h"
#include "hbapifs.h"
#include "hbmath.h"
#include "hbset.h"

#if defined( HB_OS_UNIX )
#include <stdlib.h>
#include <unistd.h>  /* We need for mkstemp() on BSD */
#endif

#if ! defined( HB_WIN32_IO )
static BOOL fsGetTempDirByCase( char * pszName, const char * pszTempDir, HB_BOOL fTrans )
{
   BOOL fOK = FALSE;

   if( pszTempDir && *pszTempDir != '\0' )
   {
      HB_SYMBOL_UNUSED( fTrans );

      hb_strncpy( pszName, pszTempDir, HB_PATH_MAX - 1 );

      switch( hb_setGetDirCase() )
      {
         case HB_SET_CASE_LOWER:
            hb_strLower( pszName, strlen( pszName ) );
            fOK = strcmp( pszName, pszTempDir ) == 0;
            break;
         case HB_SET_CASE_UPPER:
            hb_strUpper( pszName, strlen( pszName ) );
            fOK = strcmp( pszName, pszTempDir ) == 0;
            break;
         default:
            fOK   = TRUE;
            break;
      }
   }

   if( fOK )
   {
#  if defined( __DJGPP__ )
      /* convert '/' to '\' */
      char * pszDelim;
      while( ( pszDelim = strchr( pszName, '/' ) ) != NULL )
         *pszDelim = '\\';
#  endif
      if( ! hb_fsDirExists( pszTempDir ) )
         fOK = HB_FALSE;
   }

   return fOK;
}
#endif

static HB_FHANDLE hb_fsCreateTempLow( const char * pszDir, const char * pszPrefix, ULONG ulAttr, char * pszName, const char * pszExt )
{
   /* less attemps */
   int         iAttemptLeft = 99, iLen;
   HB_FHANDLE  fd;

   do
   {
      pszName[ 0 ] = '\0';

      if( pszDir && pszDir[ 0 ] != '\0' )
      {
         hb_strncpy( pszName, pszDir, HB_PATH_MAX - 1 );
      }
      else
      {
#if defined( HB_WIN32_IO )
         if( ! GetTempPathA( ( DWORD ) ( HB_PATH_MAX - 1 ), ( LPSTR ) pszName ) )
         {
            pszName[ 0 ]   = '.';
            pszName[ 1 ]   = '\0';
         }
#else
         char * pszTmpDir = hb_getenv( "TMPDIR" );

         if( ! fsGetTempDirByCase( pszName, pszTmpDir, HB_FALSE ) )
         {
#ifdef P_tmpdir
            if( ! fsGetTempDirByCase( pszName, P_tmpdir, HB_TRUE ) )
#endif
            {
               pszName[ 0 ]   = '.';
               pszName[ 1 ]   = '\0';
            }
         }
         if( pszTmpDir )
            hb_xfree( pszTmpDir );
#endif
      }

      if( pszName[ 0 ] != '\0' )
      {
         HB_SIZE len = strlen( pszName );

         if( pszName[ len - 1 ] != HB_OS_PATH_DELIM_CHR )
         {
            pszName[ len ]       = HB_OS_PATH_DELIM_CHR;
            pszName[ len + 1 ]   = '\0';
         }
      }

      if( pszPrefix )
         hb_strncat( pszName, pszPrefix, HB_PATH_MAX - 1 );

      iLen = ( int ) strlen( pszName );
      if( iLen > ( HB_PATH_MAX - 1 ) - 6 )
         return FS_ERROR;

#if ! defined( __WATCOMC__ ) && ( defined( HB_OS_UNIX ) || defined( HB_OS_BSD ) )
      if( hb_setGetFileCase() != HB_SET_CASE_LOWER &&
          hb_setGetFileCase() != HB_SET_CASE_UPPER &&
          hb_setGetDirCase() != HB_SET_CASE_LOWER &&
          hb_setGetDirCase() != HB_SET_CASE_UPPER &&
          pszExt == NULL )
      {
         hb_strncat( pszName, "XXXXXX", HB_PATH_MAX - 1 );
         fd = ( HB_FHANDLE ) mkstemp( pszName );
         hb_fsSetIOError( fd != ( HB_FHANDLE ) -1, 0 );
      }
      else
#endif
      {
         int      i, n;
         double   d = hb_random_num(), x;

         for( i = 0; i < 6; i++ )
         {
            d                 = d * 36;
            n                 = ( int ) d;
            d                 = modf( d, &x );
            pszName[ iLen++ ] = ( char ) ( n + ( n > 9 ? 'a' - 10 : '0' ) );
         }
         pszName[ iLen ] = '\0';
         if( pszExt )
            hb_strncat( pszName, pszExt, HB_PATH_MAX - 1 );
         hb_fsNameConv( pszName, NULL );
         fd = hb_fsCreateEx( pszName, ulAttr, FO_EXCLUSIVE | FO_EXCL );
      }

      if( fd != ( HB_FHANDLE ) FS_ERROR )
         return fd;
   }
   while( --iAttemptLeft );

   return FS_ERROR;
}

/* NOTE: The buffer must be at least HB_PATH_MAX chars long */
#if ! defined( HB_OS_UNIX )

static BOOL hb_fsTempName( char * pszBuffer, const char * pszDir, const char * pszPrefix )
{
   BOOL fResult;

#if defined( HB_WIN32_IO )
   {
      char cTempDir[ HB_PATH_MAX ];

      if( pszDir && pszDir[ 0 ] != '\0' )
         hb_strncpy( cTempDir, pszDir, sizeof( cTempDir ) - 1 );
      else
      {
         if( ! GetTempPathA( ( DWORD ) HB_PATH_MAX, cTempDir ) )
         {
            hb_fsSetIOError( FALSE, 0 );
            return FALSE;
         }
      }
      cTempDir[ HB_PATH_MAX - 1 ]   = '\0';

      fResult                       = GetTempFileNameA( ( LPCSTR ) cTempDir, pszPrefix ? ( LPCSTR ) pszPrefix : ( LPCSTR ) "xht", 0, ( LPSTR ) pszBuffer );
   }
#else

   /* TODO: Implement these: */
   HB_SYMBOL_UNUSED( pszDir );
   HB_SYMBOL_UNUSED( pszPrefix );

   /* TOFIX: The spec says to reserve L_tmpnam number of characters for the
             passed buffer. It will be needed to fix HB_PATH_MAX - 1 to be
             at least this large. */

   pszBuffer[ 0 ] = '\0';
   fResult        = ( tmpnam( pszBuffer ) != NULL );

#endif

   hb_fsSetIOError( fResult, 0 );
   return fResult;
}

/* NOTE: The pszName buffer must be at least HB_PATH_MAX chars long */

HB_FHANDLE hb_fsCreateTemp( const char * pszDir, const char * pszPrefix, ULONG ulAttr, char * pszName )
{
   USHORT nAttemptLeft = 999;

   while( --nAttemptLeft )
   {
      if( hb_fsTempName( pszName, pszDir, pszPrefix ) )
      {
         HB_FHANDLE fhnd = hb_fsCreateEx( pszName, ulAttr, FO_EXCLUSIVE | FO_EXCL );

         /* This function may fail, if the generated filename got
            used between generation and the file creation. */

         if( fhnd != FS_ERROR )
            return fhnd;
      }
      else
      {
         /* Don't attempt to retry if the filename generator is
            failing for some reason. */
         break;
      }
   }

   return FS_ERROR;
}
#else

HB_FHANDLE hb_fsCreateTemp( const char * pszDir, const char * pszPrefix, ULONG ulAttr, char * pszName )
{
   return hb_fsCreateTempLow( pszDir, pszPrefix, ulAttr, pszName, NULL );
}

#endif

HB_FUNC( HB_FTEMPCREATE )
{
   char szName[ HB_PATH_MAX ];

   hb_retnint( ( HB_NHANDLE ) hb_fsCreateTemp( hb_parc( 1 ),
                                               hb_parc( 2 ),
                                               ( ULONG ) ( ISNUM( 3 ) ? ( ULONG ) hb_parnl( 3 ) : FC_NORMAL ),
                                               szName ) );

   hb_storc( szName, 4 );
}

HB_FHANDLE hb_fsCreateTempEx( char * pszName, const char * pszDir, const char * pszPrefix, const char * pszExt, ULONG ulAttr )
{
   return hb_fsCreateTempLow( pszDir, pszPrefix, ulAttr, pszName, pszExt );
}

HB_FUNC( HB_FTEMPCREATEEX )
{
   char szName[ HB_PATH_MAX ];

   hb_retnint( ( HB_NHANDLE ) hb_fsCreateTempEx( szName,
                                                 hb_parc( 2 ),
                                                 hb_parc( 3 ),
                                                 hb_parc( 4 ),
                                                 ( ULONG ) ( ISNUM( 5 ) ? ( ULONG ) hb_parnl( 5 ) : FC_NORMAL ) ) );

   hb_storc( szName, 1 );
}

/* NOTE: pszTempDir must be at least HB_PATH_MAX long. */
HB_ERRCODE hb_fsTempDir( char * pszTempDir )
{
   HB_ERRCODE nResult = ( HB_ERRCODE ) FS_ERROR;

   pszTempDir[ 0 ] = '\0';

#if defined( HB_OS_UNIX )
   {
      char * pszTempDirEnv = hb_getenv( "TMPDIR" );

      if( fsGetTempDirByCase( pszTempDir, pszTempDirEnv, HB_FALSE ) )
         nResult = 0;
#ifdef P_tmpdir
      else if( fsGetTempDirByCase( pszTempDir, P_tmpdir, HB_TRUE ) )
         nResult = 0;
#endif
      else if( fsGetTempDirByCase( pszTempDir, "/tmp", HB_TRUE ) )
         nResult = 0;

      if( pszTempDirEnv )
         hb_xfree( pszTempDirEnv );
   }
#elif defined( HB_OS_WIN )
   {
      TCHAR lpDir[ HB_PATH_MAX ];

      if( GetTempPath( HB_PATH_MAX, lpDir ) )
      {
         nResult = 0;
         lpDir[ HB_PATH_MAX - 1 ] = TEXT( '\0' );
         // HB_OSSTRDUP2( lpDir, pszTempDir, HB_PATH_MAX - 1 );
         hb_strncpy( pszTempDir, lpDir, HB_PATH_MAX - 1 );
      }
   }
#else
   {
#if !defined( HB_OS_OS2 )
      char szBuffer[ L_tmpnam ];

      if( tmpnam( szBuffer ) != NULL )
      {
         PHB_FNAME pTempName = hb_fsFNameSplit( szBuffer );
         if( fsGetTempDirByCase( pszTempDir, pTempName->szPath, HB_TRUE ) )
            nResult = 0;
         hb_xfree( pTempName );
      }
      if( nResult != 0 )
#endif
      {
         static const char * env_tmp[] = { "TEMP", "TMP", "TMPDIR", NULL };

         const char ** tmp = env_tmp;

         while( *tmp && nResult != 0 )
         {
            char * pszTempDirEnv = hb_getenv( *tmp++ );

            if( pszTempDirEnv )
            {
               if( fsGetTempDirByCase( pszTempDir, pszTempDirEnv, HB_FALSE ) )
                  nResult = 0;
               hb_xfree( pszTempDirEnv );
            }
         }
      }
   }
#endif

   if( nResult == 0 && pszTempDir[ 0 ] != '\0' )
   {
      int len = ( int ) strlen( pszTempDir );
      if( pszTempDir[ len - 1 ] != HB_OS_PATH_DELIM_CHR &&
          len < HB_PATH_MAX - 1 )
      {
         pszTempDir[ len ] = HB_OS_PATH_DELIM_CHR;
         pszTempDir[ len + 1 ] = '\0';
      }
   }
   else
   {
      pszTempDir[ 0 ] = '.';
      pszTempDir[ 1 ] = HB_OS_PATH_DELIM_CHR;
      pszTempDir[ 2 ] = '\0';
   }

   return nResult;
}

HB_FUNC( HB_DIRTEMP )
{
   char szTempDir[ HB_PATH_MAX ];

   if( hb_fsTempDir( szTempDir ) != ( HB_ERRCODE ) FS_ERROR )
      hb_retc( szTempDir );
   else
      hb_retc_null();
}
