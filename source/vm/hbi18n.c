/*
 * $Id: hbi18n.c 9829 2012-11-19 17:15:34Z andijahja $
 */

/*
 * xHarbour Project source code:
 * Internationalization routines
 *
 * Copyright 2003 Giancarlo Niccolai <giancarlo@niccolai.ws>
 * www - http://www.xharbour.org
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

#define HB_THREAD_OPTIMIZE_STACK

#ifdef __WIN32__
   #include <windows.h>
#endif

#include "hbsetup.h"
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbi18n.h"

#if defined( HB_OS_UNIX )
   #include <unistd.h>
#endif

/** NOTICE: Language is an application wide item,
    little thread safety required */

/** Translation table; it's an array of 2 item arrays */
static PHB_ITEM   s_i18n_table = NULL;

/** Default translation files directory */
static char       s_default_i18n_dir[ HB_PATH_MAX ];

/** And our current language */
static char       s_current_language[ HB_I18N_CODELEN + 1 ];
static char       s_current_language_name[ HB_I18N_NAMELEN + 1 ];

/** Language considered to be "untranslated", or "international" */
static char       s_base_language[ HB_I18N_CODELEN + 1 ];
static char       s_base_language_name[ HB_I18N_NAMELEN ];

/***********************************************
 * VM interface
 ***********************************************/

BOOL hb_i18nInit( char * i18n_dir, char * language )
{
   BOOL bOk = FALSE;
   /* Supposing that the user strings are compiled in English;
    * this default can be changed later
    */
   hb_strncpy( s_base_language, HB_INTERNATIONAL_CODE, HB_I18N_CODELEN );
   hb_strncpy( s_base_language_name, HB_INTERNATIONAL_NAME, HB_I18N_NAMELEN - 1 );

   if( language == NULL )
   {
      language = hb_getenv( "LANG" );

      if( language == NULL )
      {
         language = hb_getenv( "LC_ALL" );
         if ( language )
            bOk = TRUE;
      }
      else
         bOk = TRUE;
   }

   if( i18n_dir == NULL )
      hb_strncpy( s_default_i18n_dir, HB_DEFAULT_I18N_PATH, HB_PATH_MAX - 1 );
   else
      hb_strncpy( s_default_i18n_dir, i18n_dir, HB_PATH_MAX - 1 );

   /* No automatic internationalization can be found */
   if( language == NULL || ! hb_i18n_load_language( language ) )
   {
      hb_strncpy( s_current_language, s_base_language, HB_I18N_CODELEN );
      hb_strncpy( s_current_language_name, s_base_language_name, HB_I18N_NAMELEN - 1 );

      /* but we know that we don't want internationalization */
      if( language != NULL )
      {
         if ( bOk )
            hb_xfree( language ) ;

         return FALSE;
      }
   }

   if ( bOk )
      hb_xfree( language ) ;

   return TRUE;
}

void hb_i18nExit( void )
{
   if( s_i18n_table != NULL )
   {
      hb_itemRelease( s_i18n_table );
      s_i18n_table = NULL;
   }
}

/***************************************
        Low level API interface
 ***************************************/

/*
 * Find an I18N string in the current table,
 * or returns the parameter untraslated (lower level)
 */
PHB_ITEM hb_i18n_scan_table( PHB_ITEM pStr, PHB_ITEM pTable, ULONG * ulIndex )
{
   ULONG    ulLower  = 1;
   ULONG    ulHigher = ( ULONG ) hb_arrayLen( pTable );
   ULONG    ulPoint  = ( ulLower + ulHigher ) / 2;
   int      iRes;
   char *   cInt     = pStr->item.asString.value;

   while( 1 )
   {
      /* get the table row */
      PHB_ITEM pRow = hb_arrayGetItemPtr( pTable, ulPoint );

      iRes = strcmp( hb_arrayGetCPtr( pRow, 1 ), cInt );

      if( iRes == 0 )
      {
         *ulIndex = ulPoint;
         return hb_arrayGetItemPtr( pRow, 2 );
      }
      else
      {
         if( ulLower == ulHigher )
            break;

         /* last try. In pair distros, it can be also in the other node */
         else if( ulLower == ulHigher - 1 )
         {
            /* essendo matematica intera, ulPoint � per difetto, ed ha
             * gi� esaminato il punto lower
             */
            pRow = hb_arrayGetItemPtr( pTable, ulHigher );

            if( strcmp( hb_arrayGetCPtr( pRow, 1 ), cInt ) == 0 )
            {
               *ulIndex = ulHigher;
               return hb_arrayGetItemPtr( pRow, 2 );
            }
            break;
         }

         if( iRes > 0 )
            ulHigher = ulPoint;
         else
            ulLower = ulPoint;

         ulPoint = ( ulLower + ulHigher ) / 2;
      }
   }

   /* entry not found */
   return pStr;
}

/**
 * Creates a standard language filename
 * i18n_dir: directory where to search for the language (NULL means default)
 * language: language name in international format i.e. "en_US" or "it_IT"
 * returns: newly allocated area where to store the filename
 */
char * hb_i18n_build_table_filename( char * i18n_dir, char * language )
{
   char * path;

   /* path? (if null, it is i18n/ subdir) */
   if( i18n_dir == NULL )
      i18n_dir = s_default_i18n_dir;

   if( strlen( i18n_dir ) > 0 )
   {
      HB_SIZE uiLen = (
         strlen( i18n_dir ) +
         strlen( language ) +
         strlen( HB_I18N_TAB_EXT ) + 3 ); /* '/', dot and '\0' */

      path = ( char * ) hb_xgrab( uiLen );
      hb_snprintf( path, ( size_t ) uiLen, "%s%c%s.%s",
                   i18n_dir,
                   HB_OS_PATH_DELIM_CHR,
                   language,
                   HB_I18N_TAB_EXT );
   }
   else
   {
      HB_SIZE uiLen = (
         strlen( language ) +
         strlen( HB_I18N_TAB_EXT ) + 2 ); /* dot and '\0' */

      path = ( char * ) hb_xgrab( uiLen );

      hb_snprintf( path, ( size_t ) uiLen, "%s.%s",
                   language,
                   HB_I18N_TAB_EXT );
   }

   return path;
}

/* read the header of a table, and puts it in a HB array */
PHB_ITEM hb_i18n_read_table_header( FHANDLE handle )
{
   PHB_ITEM             pRet;
   HB_I18N_TAB_HEADER   header;
   USHORT               nRead;

   nRead = hb_fsRead( handle, ( BYTE * ) &header, sizeof( header ) );

   if( nRead != sizeof( header ) )
      return NULL;

   /* checking signature */
   if( strcmp( header.signature, "\3HIL" ) != 0 &&
       strcmp( header.signature, "\3HIT" ) != 0 )
      return NULL;

   pRet = hb_itemNew( NULL );
   hb_arrayNew( pRet, 6 ); /* six fields. */
   hb_arraySetC( pRet, 1, header.signature );
   hb_arraySetC( pRet, 2, header.author );
   hb_arraySetC( pRet, 3, header.language );
   hb_arraySetC( pRet, 4, header.language_int );
   hb_arraySetC( pRet, 5, header.language_code );
   hb_arraySetNI( pRet, 6, header.entries );

   return pRet;
}

/* saving the table header from an xharbour array */
BOOL hb_i18n_write_table_header( FHANDLE handle, PHB_ITEM pHeader )
{
   HB_I18N_TAB_HEADER   header;
   USHORT               nWrite;

   /* strncopy prevents gpf */
   hb_strncpy( header.signature, hb_arrayGetCPtr( pHeader, 1 ), sizeof( header.signature ) );
   hb_strncpy( header.author, hb_arrayGetCPtr( pHeader, 2 ), sizeof( header.author ) );
   hb_strncpy( header.language, hb_arrayGetCPtr( pHeader, 3 ), sizeof( header.language ) );
   hb_strncpy( header.language_int, hb_arrayGetCPtr( pHeader, 4 ), sizeof( header.language_int ) );
   hb_strncpy( header.language_code, hb_arrayGetCPtr( pHeader, 5 ), sizeof( header.language_code ) );
   header.entries = hb_arrayGetNI( pHeader, 6 );

   nWrite         = hb_fsWrite( handle, ( BYTE * ) &header, sizeof( header ) );

   if( nWrite != sizeof( header ) )
      return FALSE;

   return TRUE;
}

/**
 * Reading table from disk.
 * The handle must be positioned AFTER the header.
 * if count is < 0, a self-growing array will be used. IF size of
 * data can be known in advance, this will save dynamic reallocation
 * time while loading.
 * Returns a new table in an array of array, or NULL on failure
 */

PHB_ITEM hb_i18n_read_table( FHANDLE handle, int count )
{
   char     szStrLen[ 9 ];
   int      nStrLen, nRead;
   int      i, j;
   PHB_ITEM pTable;

   szStrLen[ 8 ]  = '\0';
   pTable         = hb_itemNew( NULL );

   if( count > 0 )
      hb_arrayNew( pTable, count );
   else
      hb_arrayNew( pTable, 0 ); /* we'll add them */

   for( i = 1; count == -1 || i <= count; i++ )
   {
      HB_ITEM ArrRow;

      ArrRow.type = HB_IT_NIL;
      hb_arrayNew( &ArrRow, 2 );

      for( j = 1; j <= 2; j++ )
      {
         nRead = hb_fsRead( handle, ( BYTE * ) szStrLen, 8 );

         if( nRead == 8 )
         {
            nStrLen = atoi( szStrLen );

            /* zero is included */
            if( nStrLen > 0 )  /* sanitizing unwritten strings */
            {
               char * str = ( char * ) hb_xgrab( nStrLen );
               nRead = hb_fsRead( handle, ( BYTE * ) str, ( USHORT ) nStrLen );
               /* using trailing zero as file integrity check
                * (zero is the last character read, so we check nStrLen-1)
                */
               if( nRead != nStrLen || str[ nStrLen - 1 ] != 0 )
               {
                  hb_xfree( str );
                  hb_itemClear( &ArrRow );
                  hb_itemRelease( pTable );
                  return NULL;
               }
               else
                  hb_itemPutCRaw( hb_arrayGetItemPtr( &ArrRow, j ), str, nStrLen - 1 );
            }
         }
         /* correct file termination upon unknown size request? */
         else if( nRead == 0 && count < 0 )
         {
            hb_itemClear( &ArrRow );
            return pTable;
         }
         else
         {
            hb_itemClear( &ArrRow );
            hb_itemRelease( pTable );
            return NULL;
         }
      }

      /* saves our row here */
      if( count > 0 )
         hb_arraySetForward( pTable, i, &ArrRow );
      else
         hb_arrayAddForward( pTable, &ArrRow );
   }

   return pTable;
}


PHB_ITEM hb_i18n_read_memory_table( BYTE * pData, int count )
{
   char     szStrLen[ 9 ];
   int      nStrLen;
   int      i, j;
   PHB_ITEM pTable;


/* TODO: unknown size table not supported, since resource size is available only
 *       aligned to 16 bytes.
 */

   if( count < 0 )
   {
      TraceLog( NULL, "hb_i18n_read_memory_table unknown size request not supported\n" );
      return NULL;
   }

   szStrLen[ 8 ]  = '\0';
   pTable         = hb_itemNew( NULL );

   if( count > 0 )
      hb_arrayNew( pTable, count );
   else
      hb_arrayNew( pTable, 0 ); /* we'll add them */

   for( i = 1; count == -1 || i <= count; i++ )
   {
      HB_ITEM ArrRow;

      ArrRow.type = HB_IT_NIL;
      hb_arrayNew( &ArrRow, 2 );

      for( j = 1; j <= 2; j++ )
      {
         HB_MEMCPY( szStrLen, pData, 8 );
         pData    += 8;
         nStrLen  = atoi( szStrLen );

         /* zero is included */
         if( nStrLen > 0 )  /* sanitizing unwritten strings */
         {
            char * str = ( char * ) hb_xgrab( nStrLen );
            HB_MEMCPY( str, pData, nStrLen );
            pData += nStrLen;
            /* using trailing zero as memory integrity check
             * (zero is the last character read, so we check nStrLen-1)
             */
            if( str[ nStrLen - 1 ] != 0 )
            {
               hb_xfree( str );
               hb_itemClear( &ArrRow );
               hb_itemRelease( pTable );
               return NULL;
            }
            else
               hb_itemPutCRaw( hb_arrayGetItemPtr( &ArrRow, j ), str, nStrLen - 1 );
         }
      }

      /* saves our row here */
      if( count > 0 )
         hb_arraySetForward( pTable, i, &ArrRow );
      else
         hb_arrayAddForward( pTable, &ArrRow );
   }

   return pTable;
}

/**
 * Saving table data to disk
 */

BOOL hb_i18n_write_table( FHANDLE handle, PHB_ITEM pTable )
{
   char  szStrLen[ 9 ];
   int   nStrLen;
   ULONG i, j;

   for( i = 1; i <= hb_arrayLen( pTable ); i++ )
   {
      PHB_ITEM pRow = hb_arrayGetItemPtr( pTable, i );

      for( j = 1; j <= 2; j++ )
      {
         /* including also trailing 0 */
         nStrLen = ( int ) hb_arrayGetCLen( pRow, j ) + 1;

         if( nStrLen == 1 )
         {
            hb_snprintf( szStrLen, sizeof( szStrLen ), "%8d", 0 );

            if( hb_fsWrite( handle, ( BYTE * ) szStrLen, 8 ) != 8 )
               return FALSE;

            continue;
         }

         hb_snprintf( szStrLen, sizeof( szStrLen ), "%8d", nStrLen );

         if( hb_fsWrite( handle, ( BYTE * ) szStrLen, 8 ) != 8 )
            return FALSE;

         if( hb_fsWrite( handle, ( BYTE * ) hb_arrayGetCPtr( pRow, j ), ( USHORT ) nStrLen ) != nStrLen )
            return FALSE;
      }
   }
   return TRUE;
}

/**
 * load language is a shortcut to
 * 1) get the canonical filename for a language code
 * 2) open it
 * 3) load the header and check consistency
 * 4) load table into s_i18n_table
 */

BOOL hb_i18n_load_language( char * language )
{
   char *               path;
   FHANDLE              handle;
   HB_I18N_TAB_HEADER   header;
   PHB_ITEM             pTable;

   path     = hb_i18n_build_table_filename( NULL, language );
   handle   = hb_fsOpen( path, FO_READ ); /* on error will fail on next op */
   hb_xfree( path );

   if( handle == FS_ERROR )
   {
#ifdef __WIN32__
      HRSRC    hRes;
      HGLOBAL  hMem;
      BYTE *   pRes;

      hRes = FindResource( NULL, language, "I18N" );

      if( ! hRes )
         return FALSE;

      hMem = LoadResource( NULL, hRes );

      if( ! hMem )
         return FALSE;

      pRes = ( BYTE * ) LockResource( hMem );
      HB_MEMCPY( &header, pRes, sizeof( header ) );

      /* checking signature */
      if( strcmp( header.signature, "\3HIL" ) != 0 &&
          strcmp( header.signature, "\3HIT" ) != 0 )
         return FALSE;

      pTable = hb_i18n_read_memory_table( pRes + sizeof( header ), header.entries );

      if( pTable != NULL )
      {
         hb_strncpy( s_current_language, language, HB_I18N_CODELEN );
         hb_strncpy( s_current_language_name, header.language, HB_I18N_NAMELEN );

         if( s_i18n_table != NULL )
            hb_itemRelease( s_i18n_table );

         s_i18n_table = pTable;
         return TRUE;
      }
#endif
      return FALSE;
   }

   if( hb_fsRead( handle, ( BYTE * ) &header, sizeof( header ) ) != sizeof( header ) )
   {
      hb_fsClose( handle );
      return FALSE;
   }

   /* checking signature */
   if( strcmp( header.signature, "\3HIL" ) != 0 &&
       strcmp( header.signature, "\3HIT" ) != 0 )
   {
      hb_fsClose( handle );
      return FALSE;
   }

   pTable = hb_i18n_read_table( handle, header.entries );

   if( pTable != NULL )
   {
      hb_strncpy( s_current_language, language, HB_I18N_CODELEN );
      hb_strncpy( s_current_language_name, header.language, HB_I18N_NAMELEN );

      if( s_i18n_table != NULL )
         hb_itemRelease( s_i18n_table );

      s_i18n_table = pTable;
      return TRUE;
   }

   return FALSE;
}

/*
 * XHARBOUR MIDDLE LEVEL API
 * These functions are meant to be used
 * by utilities willing to build/manage
 * dictionary files.
 */

/**
 * Loads a table in an array
 * HB_I18nLoadTable( cPath ) --> aResult.
 * HB_I18nLoadTable( nFileHandle ) --> aResult.
 *
 * On success returns a two item array, where the header is
 * the first item, and the table is the second. On failure,
 * returns NIL
 */
HB_FUNC( HB_I18NLOADTABLE )
{
   HB_THREAD_STUB_API
   PHB_ITEM pParam = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pHeader;
   PHB_ITEM pTable;
   HB_ITEM  ArrRet;
   FHANDLE  handle;

   if( pParam == NULL ||
       ( ! HB_IS_STRING( pParam ) && ! HB_IS_NUMERIC( pParam ) )
       )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   if( HB_IS_STRING( pParam ) )
      handle = hb_fsOpen( pParam->item.asString.value, FO_READ );
   else
      handle = hb_itemGetNI( pParam );

   pHeader = hb_i18n_read_table_header( handle );

   if( pHeader != NULL )
   {
      pTable = hb_i18n_read_table( handle, hb_arrayGetNI( pHeader, 6 ) );
      if( pTable != NULL )
      {
         ArrRet.type = HB_IT_NIL;
         hb_arrayNew( &ArrRet, 2 );
         hb_arraySetForward( &ArrRet, 1, pHeader );
         hb_arraySetForward( &ArrRet, 2, pTable );
         hb_itemReturnForward( &ArrRet );
         hb_itemRelease( pTable );
      }
      else
         hb_ret();

      hb_itemRelease( pHeader );
   }
   else
      hb_ret();

   if( HB_IS_STRING( pParam ) )
      hb_fsClose( handle );
}

/**
 * Sorts a table in a new array.
 * HB_I18nSortTable( aTable ) --> aSorted
 **/
HB_FUNC( HB_I18NSORTTABLE )
{
   PHB_ITEM pTable = hb_param( 1, HB_IT_ARRAY );
   PHB_ITEM pTemp;
   HB_ITEM  ArrResult;
   char *   key;
   ULONG    i, pos;

   if( pTable == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   if( hb_arrayLen( pTable ) == 0 )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Parameter must be non empty", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   /* Creating a table wide as the target */
   ArrResult.type = HB_IT_NIL;
   hb_arrayNew( &ArrResult, hb_arrayLen( pTable ) );
   /* setting first element */
   hb_arraySet( &ArrResult, 1, hb_arrayGetItemPtr( pTable, 1 ) );
   /* TODO: Use fixed len table and do quicksort algo. */

   for( i = 2; i <= hb_arrayLen( pTable ); i++ )
   {
      pTemp = hb_arrayGetItemPtr( pTable, i );
      key   = pTemp->item.asString.value;
      pos   = 1;

      while( pos < i )
      {
         if( strcmp( hb_arrayGetCPtr( pTable, pos ), key ) >= 0 )
            break;

         pos++;
      }

      if( pos <= i )
         hb_arrayIns( &ArrResult, pos );

      hb_arraySet( &ArrResult, pos, pTemp );
   }

   hb_itemReturnForward( &ArrResult );
}

/**
 * Saves a table to disk
 * HB_I18nSaveTable( cPath, aHeader, aTable ) --> lResult
 * HB_I18nLoadTable( nFile, aHeader, aTable ) --> lResult
 * Returns true on success.
 */
HB_FUNC( HB_I18NSAVETABLE )
{
   HB_THREAD_STUB_API
   PHB_ITEM pParam   = hb_param( 1, HB_IT_ANY );
   PHB_ITEM pHeader  = hb_param( 2, HB_IT_ARRAY );
   PHB_ITEM pTable   = hb_param( 3, HB_IT_ARRAY );
   FHANDLE  handle;

   if( pParam == NULL ||
       ( ! HB_IS_STRING( pParam ) && ! HB_IS_NUMERIC( pParam ) )
       )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            3, hb_param( 1, HB_IT_ANY ),
                            hb_param( 2, HB_IT_ANY ),
                            hb_param( 3, HB_IT_ANY ) );
      return;
   }

   if( HB_IS_STRING( pParam ) )
   {
      handle = hb_fsCreate( pParam->item.asString.value, FC_NORMAL );

      /* an opening failure will cause following operations to fail */
      if( handle == FS_ERROR )
      {
         hb_retl( FALSE );
         return;
      }
   }
   else
      handle = hb_itemGetNL( pParam );

   if( hb_i18n_write_table_header( handle, pHeader ) &&
       hb_i18n_write_table( handle, pTable ) )
      hb_retl( TRUE );
   else
      hb_retl( FALSE );

   if( HB_IS_STRING( pParam ) )
      hb_fsClose( handle );
}

/***************************************
   XHARBOUR HIGH LEVEL API
***************************************/

/* Translate an xharbour string if possible, or return it untraslated */
HB_FUNC( I18N ) /* we get a license over HB_ naming convention for this */
{
   PHB_ITEM pStr = hb_param( 1, HB_IT_STRING );
   PHB_ITEM pRet;
   ULONG    ulIndex;

   if( pStr == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "I18N must be called on a string", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   if( s_i18n_table == NULL )
   {
      hb_itemReturn( pStr );
      return;
   }

   if( ( pRet = hb_i18n_scan_table( pStr, s_i18n_table, &ulIndex ) )->type == HB_IT_NIL )
      hb_itemReturn( pStr );
   else
      hb_itemReturn( pRet );
}

/* Sets default directory */
HB_FUNC( HB_I18NSETPATH )
{
   PHB_ITEM pStr = hb_param( 1, HB_IT_STRING );

   if( pStr == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   hb_strncpy( s_default_i18n_dir, hb_parc( 1 ), HB_PATH_MAX - 1 );
}

/* Gets default directory */
HB_FUNC( HB_I18NGETPATH )
{
   HB_THREAD_STUB_API
   hb_retc( s_default_i18n_dir );
}

/* Sets current language (return true on success) */
HB_FUNC( HB_I18NSETLANGUAGE )
{
   HB_THREAD_STUB_API

   char *   language;
   PHB_ITEM pStr = hb_param( 1, HB_IT_STRING );

   if( pStr == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            1, hb_param( 1, HB_IT_ANY ) );
      return;
   }

   language = pStr->item.asString.value;

   if( strcmp( s_base_language, language ) == 0 )
   {
      if( s_i18n_table != NULL )
      {
         hb_itemRelease( s_i18n_table );
         s_i18n_table = NULL;
         hb_strncpy( s_current_language, s_base_language, HB_I18N_CODELEN );
         hb_strncpy( s_current_language_name, s_base_language_name,
                     HB_I18N_NAMELEN - 1 );
      }
      hb_retl( TRUE );
   }
   else
      hb_retl( hb_i18n_load_language( language ) );
}

HB_FUNC( HB_I18NGETLANGUAGE )
{
   HB_THREAD_STUB_API
   hb_retc( s_current_language );
}

HB_FUNC( HB_I18NGETLANGUAGENAME )
{
   HB_THREAD_STUB_API
   hb_retc( s_current_language_name );
}

HB_FUNC( HB_I18NGETBASELANGUAGE )
{
   HB_THREAD_STUB_API
   hb_retc( s_base_language );
}

HB_FUNC( HB_I18NGETBASELANGUAGENAME )
{
   HB_THREAD_STUB_API
   hb_retc( s_base_language_name );
}

HB_FUNC( HB_I18NSETBASELANGUAGE )
{
   const char *   szCode   = hb_parc( 1 );
   const char *   szName   = hb_parc( 2 );
   BOOL           bChange  = FALSE;

   if( szCode == NULL || szName == NULL )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            2, hb_paramError( 1 ), hb_paramError( 2 ) );

   if( strcmp( s_base_language, s_current_language ) == 0 )
      bChange = TRUE;

   hb_strncpy( s_base_language, szCode, HB_I18N_CODELEN );
   hb_strncpy( s_base_language_name, szName, HB_I18N_NAMELEN );

   if( bChange )
   {
      hb_strncpy( s_current_language, szCode, HB_I18N_CODELEN );
      hb_strncpy( s_current_language_name, szName, HB_I18N_NAMELEN );
   }
}

HB_FUNC( HB_I18NINITIALIZED )
{
   HB_THREAD_STUB_API
   hb_retl( s_i18n_table != NULL );
}

/*******************************************
 * Publishing the hashtable search algorithm
 */
HB_FUNC( HB_HFIND )
{
   HB_THREAD_STUB_API
   PHB_ITEM pHash    = hb_param( 1, HB_IT_ARRAY );
   PHB_ITEM pKey     = hb_param( 2, HB_IT_STRING );
   PHB_ITEM pRet;
   PHB_ITEM pIndex;
   ULONG    ulIndex  = 0;

   if( pHash == NULL || pKey == NULL )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, "Wrong parameter format", NULL,
                            2, hb_paramError( 1 ), hb_paramError( 2 ) );

   pRet = hb_i18n_scan_table( pKey, pHash, &ulIndex );

   if( hb_pcount() >= 3 && ISBYREF( 3 ) )
   {
      pIndex = hb_param( 3, HB_IT_ANY );
      hb_itemPutNL( pIndex, ulIndex );
   }

   /* The key scan returns the same pKey if not found */
   if( pRet != pKey )
      hb_itemReturn( pRet );
   else
      hb_ret();
}
