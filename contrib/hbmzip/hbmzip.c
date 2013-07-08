/*
 * $Id: hbmzip.c 9773 2012-10-19 08:29:51Z andijahja $
 */

/*
 * Harbour Project source code:
 *    Wrapper functions for minizip library
 *    Some higher level zip archive functions
 *
 * Copyright 2008 Mindaugas Kavaliauskas <dbtopas.at.dbtopas.lt>
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

#define INCL_DOSFILEMGR

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbdate.h"
#include "hbset.h"
#include "zip.h"
#include "unzip.h"

#if defined( HB_OS_WIN )
   #include "windows.h"
#elif defined( HB_OS_UNIX )
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <unistd.h>
   #include <time.h>
   #include <utime.h>
#elif defined( HB_OS_DOS )
   #if defined(__DJGPP__) || defined(__RSX32__) || defined(__GNUC__)
      #include "hb_io.h"
      #include <sys/param.h>
      #if defined( HB_OS_DOS )
         #include <time.h>
         #include <utime.h>
      #endif
   #endif
#endif

#define HB_Z_IOBUF_SIZE       ( 1024 * 16 )
#define hb_storclen_buffer    hb_storclenAdopt

static HB_GARBAGE_FUNC( hb_zipfile_destructor )
{
   zipFile*  phZip = (zipFile*) Cargo;

   if( * phZip )
   {
      zipClose( * phZip, NULL );
      * phZip = NULL;
   }
}

static zipFile hb_zipfileParam( int iParam )
{
   zipFile*  phZip = ( zipFile* ) hb_parptrGC( hb_zipfile_destructor, iParam );

   if( phZip && * phZip )
      return *phZip;

   hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
   return NULL;
}


static HB_GARBAGE_FUNC( hb_unzipfile_destructor )
{
   unzFile*  phUnzip = (unzFile*) Cargo;

   if( * phUnzip )
   {
      unzClose( * phUnzip );
      * phUnzip = NULL;
   }
}

static unzFile hb_unzipfileParam( int iParam )
{
   unzFile*  phUnzip = ( unzFile* ) hb_parptrGC( hb_unzipfile_destructor, iParam );

   if( phUnzip && * phUnzip )
      return *phUnzip;

   hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
   return NULL;
}

/* HB_ZipOpen( cFileName, [ iMode = HB_ZIP_CREATE ], [ @cGlobalComment ] ) --> hZip */
HB_FUNC( HB_ZIPOPEN )
{
   const char* szFileName = hb_parc( 1 );

   if( szFileName )
   {
      zipcharpc pszGlobalComment = NULL;
      zipFile hZip = zipOpen2( szFileName, ISNUM( 2 ) ? hb_parni( 2 ) : APPEND_STATUS_CREATE,
                               &pszGlobalComment, NULL );
      if( hZip )
      {
         zipFile* phZip = (zipFile*) hb_gcAlloc( sizeof( zipFile ), hb_zipfile_destructor );

         * phZip = hZip;
         hb_retptrGC( phZip );

         if( pszGlobalComment )
            hb_storc( ( char * ) pszGlobalComment, 3 );
      }
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_ZipClose( hZip, [ cGlobalComment ] ) --> nError */
HB_FUNC( HB_ZIPCLOSE )
{
   zipFile*  phZip = ( zipFile* ) hb_parptrGC( hb_zipfile_destructor, 1 );

   if( phZip && * phZip )
   {
      zipFile  hZip = * phZip;

      * phZip = NULL;
      hb_retni( zipClose( hZip, hb_parc( 2 ) ) );
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/* HB_ZipFileCreate( hZip, cZipName, dDate, cTime, nInternalAttr, nExternalAttr,
                     [ nMethod = HB_ZLIB_METHOD_DEFLATE ],
                     [ nLevel = HB_ZLIB_COMPRESSION_DEFAULT ],
                     [ cPassword, ulFileCRC32 ], [ cComment ] ) --> nError */
HB_FUNC( HB_ZIPFILECREATE )
{
   const char* szZipName = hb_parc( 2 );

   if( szZipName )
   {
      zipFile hZip = hb_zipfileParam( 1 );

      if( hZip )
      {
         int iMethod = ISNUM( 7 ) ? hb_parni( 7 ) : Z_DEFLATED;
         int iLevel = ISNUM( 8 ) ? hb_parni( 8 ) : Z_DEFAULT_COMPRESSION;
         int iY, iM, iD;

         zip_fileinfo zfi;

         memset( &zfi, 0, sizeof( zfi ) );

         hb_dateDecode( hb_pardl( 3 ), &iY, &iM, &iD );
         zfi.tmz_date.tm_year = iY;
         zfi.tmz_date.tm_mon = iM - 1;
         zfi.tmz_date.tm_mday = iD;

         hb_timeStrGet( hb_parc( 4 ), &iY, &iM, &iD, NULL );
         zfi.tmz_date.tm_hour = iY;
         zfi.tmz_date.tm_min = iM;
         zfi.tmz_date.tm_sec = iD;

         /* Missed Internal and External File Attributes */
         /* Begin: 2008-01-19 andresreyesh */

         zfi.internal_fa = hb_parnl( 5 );
         zfi.external_fa = hb_parnl( 6 );

         /* End: 2008-01-19 andresreyesh */

         hb_retni( zipOpenNewFileInZip3( hZip, szZipName, &zfi,
                                         NULL, 0, NULL, 0,
                                         hb_parc( 11 ), iMethod, iLevel, 0,
                                         -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                         hb_parc( 9 ), hb_parnl( 10 ) ) );
      }
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_ZipFileWrite( hZip, cData [, nLen ] ) --> nError */
HB_FUNC( HB_ZIPFILEWRITE )
{
   const char* pData = hb_parc( 2 );

   if( pData )
   {
      zipFile hZip = hb_zipfileParam( 1 );
      ULONG   ulLen = (ULONG) hb_parclen( 2 );

      if( ISNUM( 3 ) && (ULONG) hb_parnl( 3 ) < ulLen )
         ulLen = (ULONG) hb_parnl( 3 );

      if( hZip )
         hb_retni( zipWriteInFileInZip( hZip, (void*) pData, ulLen ) );
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_ZipFileClose( hZip ) --> nError */
HB_FUNC( HB_ZIPFILECLOSE )
{
   zipFile hZip = hb_zipfileParam( 1 );

   if( hZip )
      hb_retni( zipCloseFileInZip( hZip ) );
}


/* HB_UnzipOpen( cFileName ) --> hUnzip */
HB_FUNC( HB_UNZIPOPEN )
{
   const char* szFileName = hb_parc( 1 );

   if( szFileName )
   {
      unzFile hUnzip = unzOpen( szFileName );
      if( hUnzip )
      {
         unzFile*  phUnzip = (unzFile*) hb_gcAlloc( sizeof( unzFile ), hb_unzipfile_destructor );

         * phUnzip = hUnzip;
         hb_retptrGC( phUnzip );
      }
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_UnzipClose( hUnzip ) --> nError */
HB_FUNC( HB_UNZIPCLOSE )
{
   unzFile*  phUnzip = ( unzFile* ) hb_parptrGC( hb_unzipfile_destructor, 1 );

   if( phUnzip && * phUnzip )
   {
      unzFile  hUnzip = * phUnzip;

      * phUnzip = NULL;
      hb_retni( unzClose( hUnzip ) );
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_UnzipGlobalInfo( hUnzip, @nEntries, @cGlobalComment ) --> nError */
HB_FUNC( HB_UNZIPGLOBALINFO )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
   {
      unz_global_info ugi;
      int iResult;

      iResult = unzGetGlobalInfo( hUnzip, &ugi );

      hb_retni( iResult );

      if( iResult == UNZ_OK )
      {
         hb_storni( ugi.number_entry, 2 );
         if( ISBYREF( 3 ) )
         {
            if( ugi.size_comment > 0 )
            {
               char * pszComment = ( char * ) hb_xgrab( ugi.size_comment + 1 );

               iResult = unzGetGlobalComment( hUnzip, pszComment, ugi.size_comment );
               if( iResult < 0 )
               {
                  hb_xfree( pszComment );
                  hb_storc( NULL, 3 );
                  hb_retni( iResult );
               }
               else
               {
                  pszComment[ iResult ] = '\0';
                  if( !hb_storclen_buffer( pszComment, ugi.size_comment, 3 ) )
                     hb_xfree( pszComment );
               }
            }
         }
      }
      else
      {
         hb_storni( 0, 2 );
         hb_storc( NULL, 3 );
      }
   }
}


/*  HB_UnzipFileFirst( hUnzip ) --> nError */
HB_FUNC( HB_UNZIPFILEFIRST )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( unzGoToFirstFile( hUnzip ) );
}


/*  HB_UnzipFileNext( hUnzip ) --> nError */
HB_FUNC( HB_UNZIPFILENEXT )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( unzGoToNextFile( hUnzip ) );
}


/*  HB_UnzipFilePos( hUnzip ) --> nPosition */
HB_FUNC( HB_UNZIPFILEPOS )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retnint( unzGetOffset( hUnzip ) );
}


/*  HB_UnzipFileGoto( hUnzip, nPosition ) --> nError */
HB_FUNC( HB_UNZIPFILEGOTO )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( unzSetOffset( hUnzip, (ULONG) hb_parnint( 2 ) ) );
}


/*  HB_UnzipFileInfo( hUnzip, @cZipName, @dDate, @cTime,
                      @nInternalAttr, @nExternalAttr,
                      @nMethod, @nSize, @nCompressedSize,
                      @lCrypted, @cComment ) --> nError */
HB_FUNC( HB_UNZIPFILEINFO )
{
   unzFile hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
   {
      char           szFileName[ HB_PATH_MAX ];
      unz_file_info  ufi;
      int            iResult;
      char           buf[ 16 ];

      iResult = unzGetCurrentFileInfo( hUnzip, &ufi, szFileName, HB_PATH_MAX - 1,
                                       NULL, 0, NULL, 0 );
      hb_retni( iResult );

      if( iResult == UNZ_OK )
      {
         szFileName[ HB_PATH_MAX - 1 ] = '\0';
         hb_storc( szFileName, 2 );
         hb_stord( ufi.tmu_date.tm_year, ufi.tmu_date.tm_mon + 1, ufi.tmu_date.tm_mday, 3 );



         hb_snprintf( buf, sizeof( buf ), "%02d:%02d:%02d", ufi.tmu_date.tm_hour,
                   ufi.tmu_date.tm_min, ufi.tmu_date.tm_sec );
         hb_storc( buf, 4 );
         hb_stornl( ufi.internal_fa, 5 );
         hb_stornl( ufi.external_fa, 6 );
         hb_stornl( ufi.compression_method, 7 );
         hb_stornl( ufi.uncompressed_size, 8 );
         hb_stornl( ufi.compressed_size, 9 );
         hb_storl( ( ufi.flag & 1 ) != 0, 10 );

         if( ufi.size_file_comment > 0 && ISBYREF( 11 ) )
         {
            char * pszComment = ( char * ) hb_xgrab( ufi.size_file_comment + 1 );

            iResult = unzGetCurrentFileInfo( hUnzip, NULL, NULL, 0, NULL, 0,
                                             pszComment, ufi.size_file_comment );
            pszComment[ ufi.size_file_comment ] = '\0';
            if( iResult != UNZ_OK )
            {
               hb_xfree( pszComment );
               hb_storc( NULL, 11 );
            }
            else if( !hb_storclen_buffer( pszComment, ufi.size_file_comment, 11 ) )
               hb_xfree( pszComment );
         }
      }
      else
      {
         hb_storc( NULL, 2 );
         hb_stordl( 0, 3 );
         hb_storc( NULL, 4 );
         hb_stornl( 0, 5 );
         hb_stornl( 0, 6 );
         hb_stornl( 0, 7 );
         hb_stornl( 0, 8 );
         hb_stornl( 0, 9 );
         hb_storl( FALSE, 10 );
         hb_storc( NULL, 11 );
      }
   }
}


/*  HB_UnzipFileOpen( hUnzip, [ cPassword ] ) --> nError */
HB_FUNC( HB_UNZIPFILEOPEN )
{
   unzFile  hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( unzOpenCurrentFilePassword( hUnzip, hb_parc( 2 ) ) );
}


/* HB_UnzipFileRead( hUnzip, @cBuf [, nLen ] ) --> nRead */
HB_FUNC( HB_UNZIPFILEREAD )
{
   PHB_ITEM  pBuffer = hb_param( 2, HB_IT_STRING );
   char *    buffer;
   HB_SIZE   ulSize;

   if( pBuffer && ISBYREF( 2 ) &&
       hb_itemGetWriteCL( pBuffer, &buffer, &ulSize ) )
   {
      unzFile hUnzip = hb_unzipfileParam( 1 );

      if( hUnzip )
      {
         int       iResult;

         if( ISNUM( 3 ) )
         {
            ULONG ulRead = (ULONG) hb_parnl( 3 );
            if( ulRead < ulSize )
               ulSize = ulRead;
         }

         iResult = unzReadCurrentFile( hUnzip, buffer, (unsigned int) ulSize );
         hb_retnl( iResult );
      }
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


/*  HB_UnzipFileClose( hUnzip ) --> nError */
HB_FUNC( HB_UNZIPFILECLOSE )
{
   unzFile hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( unzCloseCurrentFile( hUnzip ) );
}


/*
 *
 * Higher level functions - not a wrappers of minizip code
 *
 */

static BOOL hb_zipGetFileInfo( const char* szFileName, ULONG * pulCRC, BOOL * pfText )
{
   FILE * file;
   BOOL fText = pfText != NULL, fResult = FALSE;
   ULONG ulCRC = 0;

   file = hb_fopen( szFileName, "rb" );
   if( file )
   {
      unsigned char * pString = ( unsigned char * ) hb_xgrab( HB_Z_IOBUF_SIZE );
      ULONG ulRead, u;

      do
      {
         ulRead = ( ULONG ) fread( pString, 1, HB_Z_IOBUF_SIZE, file );
         if( ulRead > 0 )
         {
            ulCRC = crc32( ulCRC, pString, ulRead );
            if( fText )
            {
               for( u = 0; u < ulRead; ++u )
               {
                  if( pString[ u ] < 0x20 ?
                      ( pString[ u ] != HB_CHAR_HT &&
                        pString[ u ] != HB_CHAR_LF &&
                        pString[ u ] != HB_CHAR_CR &&
                        pString[ u ] != HB_CHAR_EOF ) :
                      ( pString[ u ] >= 0x7f && pString[ u ] < 0xA0 &&
                        pString[ u ] != ( unsigned char ) HB_CHAR_SOFT1 ) )
                  {
                     fText = FALSE;
                     break;
                  }
               }
            }
         }
      }
      while( ulRead == HB_Z_IOBUF_SIZE );

      fResult = ferror( file ) == 0;

      hb_xfree( pString );
      fclose( file );
   }

   if( pulCRC )
      *pulCRC = ulCRC;
   if( pfText )
      *pfText = fText;

   return fResult;
}


/*  HB_zipFileCRC32( cFileName ) --> nError */
HB_FUNC( HB_ZIPFILECRC32 )
{
   const char * szFileName = hb_parc( 1 );

   if( szFileName )
   {
      ULONG ulCRC = 0;
      if( !hb_zipGetFileInfo( szFileName, &ulCRC, NULL ) )
         ulCRC = 0;
      hb_retnint( ulCRC );
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}

static ULONG hb_translateExtAttr( const char* szFileName, ULONG ulExtAttr )
{
   int iLen;

   iLen = ( int ) strlen( szFileName );
   if( iLen > 4 )
   {
      if( hb_stricmp( szFileName - 4, ".exe" ) == 0 ||
          hb_stricmp( szFileName - 4, ".com" ) == 0 ||
          hb_stricmp( szFileName - 4, ".bat" ) == 0 ||
          hb_stricmp( szFileName - 4, ".cmd" ) == 0 ||
          hb_stricmp( szFileName - 3, ".sh" ) == 0 )
      {
         ulExtAttr |= 0x00490000; /* --x--x--x */
      }
   }

   if( ulExtAttr | HB_FA_READONLY )
      ulExtAttr |= 0x01240000;  /* r--r--r-- */
   else
      ulExtAttr |= 0x01B60000;  /* rw-rw-rw- */

   if( ulExtAttr & HB_FA_DIRECTORY )
      ulExtAttr |= 0x40000000;
   else
      ulExtAttr |= 0x80000000;

   return ulExtAttr;
}

static int hb_zipStoreFile( zipFile hZip, const char* szFileName, const char* szName, const char* szPassword, const char* szComment )
{
   char          * szZipName, * pString;
   FHANDLE       hFile;
   ULONG         ulLen, ulExtAttr;
   zip_fileinfo  zfi;
   int           iResult;
   BOOL          fError;
   BOOL          fText;
   ULONG         ulCRC;

   if( szName )
   {
      /* change path separators to '/' */
      szZipName = hb_strdup( szName );

      ulLen = (ULONG) strlen( szZipName );
      pString = szZipName;
      while( ulLen-- )
      {
         if( pString[ ulLen ] == '\\' )
            pString[ ulLen ] = '/';
      }
   }
   else
   {
      /* get file name */
      szZipName = hb_strdup( szFileName );

      ulLen = (ULONG) strlen( szZipName );
      pString = szZipName;

      while( ulLen-- )
      {
         if( pString[ ulLen ] == '/' || pString[ ulLen ] == '\\' )
         {
            memmove( szZipName, &pString[ ulLen + 1 ], strlen( szZipName ) - ulLen );
            break;
         }
      }
   }

   memset( &zfi, 0, sizeof( zfi ) );
   fError = FALSE;

#if defined( HB_OS_WIN )
   {
      ulExtAttr = GetFileAttributesA( szFileName );

      if( (LONG) ulExtAttr != -1 )
      {
         ulExtAttr = GetFileAttributesA( szFileName ) &
                     ( FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
                       FILE_ATTRIBUTE_SYSTEM   | FILE_ATTRIBUTE_DIRECTORY |
                       FILE_ATTRIBUTE_ARCHIVE );

         ulExtAttr = hb_translateExtAttr( szFileName, ulExtAttr );
      }
      else
         fError = TRUE;
   }
#elif defined( HB_OS_UNIX )
   {
      struct stat statbuf;
      struct tm   st;

      ulExtAttr = 0;

      if( stat( szFileName, &statbuf ) == 0 )
      {
         if( S_ISDIR( statbuf.st_mode ) )
         {
            ulExtAttr |= 0x40000000;
            ulExtAttr |= 0x10; /* FILE_ATTRIBUTE_DIRECTORY */
         }
         else
         {
            ulExtAttr |= 0x80000000;
            ulExtAttr |= 0x20; /* FILE_ATTRIBUTE_ARCHIVE */
         }

         ulExtAttr |= ( ( statbuf.st_mode & S_IXOTH ) ? 0x00010000 : 0 ) |
                      ( ( statbuf.st_mode & S_IWOTH ) ? 0x00020000 : 0 ) |
                      ( ( statbuf.st_mode & S_IROTH ) ? 0x00040000 : 0 ) |
                      ( ( statbuf.st_mode & S_IXGRP ) ? 0x00080000 : 0 ) |
                      ( ( statbuf.st_mode & S_IWGRP ) ? 0x00100000 : 0 ) |
                      ( ( statbuf.st_mode & S_IRGRP ) ? 0x00200000 : 0 ) |
                      ( ( statbuf.st_mode & S_IXUSR ) ? 0x00400000 : 0 ) |
                      ( ( statbuf.st_mode & S_IWUSR ) ? 0x00800000 : 0 ) |
                      ( ( statbuf.st_mode & S_IRUSR ) ? 0x01000000 : 0 );

#   if _POSIX_C_SOURCE < 199506L || defined( HB_OS_DARWIN_5 )
         st = *localtime( &statbuf.st_mtime );
#   else
         localtime_r( &statbuf.st_mtime, &st );
#   endif

         zfi.tmz_date.tm_sec = st.tm_sec;
         zfi.tmz_date.tm_min = st.tm_min;
         zfi.tmz_date.tm_hour = st.tm_hour;
         zfi.tmz_date.tm_mday = st.tm_mday;
         zfi.tmz_date.tm_mon = st.tm_mon;
         zfi.tmz_date.tm_year = st.tm_year;
      }
      else
         fError = TRUE;
   }
#elif defined( HB_OS_DOS )
   {
#  if defined(__DJGPP__) || defined(__RSX32__) || defined(__GNUC__)
      int attr;

      attr = _chmod( szFileName, 0, 0 );
      if( attr != -1 )
#else
      ULONG attr;

      if( hb_fsGetAttr( ( BYTE * ) szFileName, &attr ) )
#endif
      {
         ulExtAttr = attr & ( HB_FA_READONLY | HB_FA_HIDDEN | HB_FA_SYSTEM |
                               HB_FA_DIRECTORY | HB_FA_ARCHIVE );

         ulExtAttr = hb_translateExtAttr( szFileName, ulExtAttr );
      }
      else
         fError = TRUE;
   }
#elif defined( HB_OS_OS2 )
   {
      FILESTATUS3 fs3;
      APIRET ulrc;
      ULONG ulAttr;

      ulrc = DosQueryPathInfo( szName, FIL_STANDARD, &fs3, sizeof( fs3 ) );
      if( ulrc == NO_ERROR )
      {
         ulAttr = 0;
         if( fs3.attrFile & FILE_READONLY )
            ulAttr |= HB_FA_READONLY;
         if( fs3.attrFile & FILE_HIDDEN )
            ulAttr |= HB_FA_HIDDEN;
         if( fs3.attrFile & FILE_SYSTEM )
            ulAttr |= HB_FA_SYSTEM;
         if( fs3.attrFile & FILE_DIRECTORY )
            ulAttr |= HB_FA_DIRECTORY;
         if( fs3.attrFile & FILE_ARCHIVED )
            ulAttr |= HB_FA_ARCHIVE;

         ulExtAttr = ulAttr;
         ulExtAttr = hb_translateExtAttr( szFileName, ulExtAttr );

         zfi.tmz_date.tm_sec = fs3.ftimeLastWrite.twosecs * 2;
         zfi.tmz_date.tm_min = fs3.ftimeLastWrite.minutes;
         zfi.tmz_date.tm_hour = fs3.ftimeLastWrite.hours;
         zfi.tmz_date.tm_mday = fs3.fdateLastWrite.day;
         zfi.tmz_date.tm_mon = fs3.fdateLastWrite.month;
         zfi.tmz_date.tm_year = fs3.fdateLastWrite.year + 1980;
      }
      else
         fError = TRUE;
   }
#else
   {
      ULONG attr;

      if( !hb_fsGetAttr( ( BYTE * ) szFileName, &attr ) )
         ulExtAttr = 0x81B60020;  /* FILE_ATTRIBUTE_ARCHIVE | rw-rw-rw- */
   }
#endif

   if( fError )
   {
      hb_xfree( szZipName );
      return -200;
   }

   fText = FALSE;
   ulCRC = 0;

   zfi.external_fa = ulExtAttr;
   /* TODO: zip.exe test: 0 for binary file, 1 for text. Does not depend on
      extension. We should analyse content of file to determine this??? */
   zfi.internal_fa = 0;

   if( ulExtAttr & 0x40000000 )
   {
      iResult = zipOpenNewFileInZip3( hZip, szZipName, &zfi, NULL, 0, NULL, 0, szComment,
                                      Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0,
                                      -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                      szPassword, ulCRC );
      if( iResult == 0 )
         zipCloseFileInZip( hZip );
   }
   else
   {
      hFile = hb_fsOpen( szFileName, FO_READ );

      if( hFile != FS_ERROR )
      {
#if defined( HB_OS_WIN )
         {
            FILETIME    ftutc, ft;
            SYSTEMTIME  st;

            if( GetFileTime( ( HANDLE ) hb_fsGetOsHandle( hFile ), NULL, NULL, &ftutc ) &&
                FileTimeToLocalFileTime( &ftutc, &ft ) &&
                FileTimeToSystemTime( &ft, &st ) )
            {
               zfi.tmz_date.tm_sec = st.wSecond;
               zfi.tmz_date.tm_min = st.wMinute;
               zfi.tmz_date.tm_hour = st.wHour;
               zfi.tmz_date.tm_mday = st.wDay;
               zfi.tmz_date.tm_mon = st.wMonth - 1;
               zfi.tmz_date.tm_year = st.wYear;
            }
         }
#endif
         if( szPassword )
         {
            if( hb_zipGetFileInfo( szFileName, &ulCRC, &fText ) )
            {
               zfi.internal_fa = fText ? 1 : 0;
            }
         }

         iResult = zipOpenNewFileInZip3( hZip, szZipName, &zfi, NULL, 0, NULL, 0, szComment,
                                         Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0,
                                         -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                         szPassword, ulCRC );
         if( iResult == 0 )
         {
            pString = ( char* ) hb_xgrab( HB_Z_IOBUF_SIZE );
            while( ( ulLen = (ULONG) hb_fsReadLarge( hFile, (BYTE*) pString, HB_Z_IOBUF_SIZE ) ) > 0 )
            {
               zipWriteInFileInZip( hZip, pString, ulLen );
            }
            hb_xfree( pString );

            zipCloseFileInZip( hZip );
         }
         hb_fsClose( hFile );
      }
      else
      {
         iResult = -200 - hb_fsError();
      }
   }
   hb_xfree( szZipName );
   return iResult;
}


/*  HB_ZipStoreFile( hZip, cFileName, [ cZipName ], [ cPassword ], [ cComment ] ) --> nError */
HB_FUNC( HB_ZIPSTOREFILE )
{
   const char* szFileName = hb_parc( 2 );

   if( szFileName )
   {
      zipFile hZip = hb_zipfileParam( 1 );

      if( hZip )
         hb_retni( hb_zipStoreFile( hZip, szFileName, hb_parc( 3 ), hb_parc( 4 ), hb_parc( 5 ) ) );
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}


static int hb_unzipExtractCurrentFile( unzFile hUnzip, const char* szFileName, const char* szPassword )
{
   char           szName[ HB_PATH_MAX ];
   ULONG          ulPos, ulLen;
   char           cSep, * pString;
   unz_file_info  ufi;
   int            iResult;
   FHANDLE        hFile;

   iResult = unzGetCurrentFileInfo( hUnzip, &ufi, szName, HB_PATH_MAX - 1,
                                    NULL, 0, NULL, 0 );
   if( iResult != UNZ_OK )
      return iResult;

   iResult = unzOpenCurrentFilePassword( hUnzip, szPassword );

   if( iResult != UNZ_OK )
      return iResult;

   if( szFileName )
   {
      hb_strncpy( szName, szFileName, sizeof( szName ) - 1 );
   }

   ulLen = (ULONG) strlen( szName );

   /* Test shows that files in subfolders can be stored to zip file without
      explicitly adding folder. So, let's create a requred path */

   ulPos = 1;
   while( ulPos < ulLen )
   {
      cSep = szName[ ulPos ];

      /* allow both path separators, ignore terminating path separator */
      if( ( cSep == '\\' || cSep == '/' ) && ulPos < ulLen - 1 )
      {
         szName[ ulPos ] = '\0';
         hb_fsMkDir( szName );
         szName[ ulPos ] = cSep;
      }
      ulPos++;
   }

   if( ufi.external_fa & 0x40000000 ) /* DIRECTORY */
   {
      hb_fsMkDir( szName );
      iResult = UNZ_OK;
   }
   else
   {
      hFile = hb_fsCreate( szName, FC_NORMAL );

      if( hFile != FS_ERROR )
      {
         pString = (char*) hb_xgrab( HB_Z_IOBUF_SIZE );

         while( ( iResult = unzReadCurrentFile( hUnzip, pString, HB_Z_IOBUF_SIZE ) ) > 0 )
         {
            hb_fsWriteLarge( hFile, (BYTE*) pString, (ULONG) iResult );
         }
         hb_xfree( pString );

#if defined( HB_OS_WIN )
         {
            FILETIME    ftutc, ft;
            SYSTEMTIME  st;

            st.wSecond = (WORD) ufi.tmu_date.tm_sec;
            st.wMinute = (WORD) ufi.tmu_date.tm_min;
            st.wHour = (WORD) ufi.tmu_date.tm_hour;
            st.wDay = (WORD) ufi.tmu_date.tm_mday;
            st.wMonth = (WORD) ufi.tmu_date.tm_mon + 1;
            st.wYear = (WORD) ufi.tmu_date.tm_year;
            st.wMilliseconds = 0;

            if( SystemTimeToFileTime( &st, &ft ) &&
                LocalFileTimeToFileTime( &ft, &ftutc ) )
            {
               SetFileTime( ( HANDLE ) hb_fsGetOsHandle( hFile ), &ftutc, &ftutc, &ftutc );
            }
         }
#endif

         hb_fsClose( hFile );
      }
      else
      {
         iResult = -200 - hb_fsError();
      }
   }
   unzCloseCurrentFile( hUnzip );


#if defined( HB_OS_WIN )
   {
      SetFileAttributesA( szName, ufi.external_fa & 0xFF );
   }
#elif defined( HB_OS_UNIX ) || defined( __DJGPP__ )
   {
      struct utimbuf    utim;
      struct tm         st;
      time_t            tim;

#  if defined( __DJGPP__ )
      _chmod( szName, 1, ufi.external_fa & 0xFF );
#  else
      ULONG ulAttr = ufi.external_fa & 0xFFFF0000;

      if( ( ulAttr & 0xFFFF0000 ) == 0 )
         ulAttr = hb_translateExtAttr( szName, ulAttr );

      chmod( szName, ( ( ulAttr & 0x00010000 ) ? S_IXOTH : 0 ) |
                     ( ( ulAttr & 0x00020000 ) ? S_IWOTH : 0 ) |
                     ( ( ulAttr & 0x00040000 ) ? S_IROTH : 0 ) |
                     ( ( ulAttr & 0x00080000 ) ? S_IXGRP : 0 ) |
                     ( ( ulAttr & 0x00100000 ) ? S_IWGRP : 0 ) |
                     ( ( ulAttr & 0x00200000 ) ? S_IRGRP : 0 ) |
                     ( ( ulAttr & 0x00400000 ) ? S_IXUSR : 0 ) |
                     ( ( ulAttr & 0x00800000 ) ? S_IWUSR : 0 ) |
                     ( ( ulAttr & 0x01000000 ) ? S_IRUSR : 0 ) );
#  endif
      memset( &st, 0, sizeof( st ) );

      st.tm_sec = ufi.tmu_date.tm_sec;
      st.tm_min = ufi.tmu_date.tm_min;
      st.tm_hour = ufi.tmu_date.tm_hour;
      st.tm_mday = ufi.tmu_date.tm_mday;
      st.tm_mon = ufi.tmu_date.tm_mon;
      st.tm_year = ufi.tmu_date.tm_year - 1900;

      tim = mktime( &st );
#   if _POSIX_C_SOURCE < 199506L || defined( HB_OS_DARWIN_5 )
      st = *gmtime( &tim );
#   else
      gmtime_r( &tim, &st );
#   endif
      utim.actime = utim.modtime = mktime( &st );

      utime( szName, &utim );
   }
#elif defined( HB_OS_DOS )

#  if defined(__RSX32__) || defined(__GNUC__)
      _chmod( szName, 1, ufi.external_fa & 0xFF );
#  else
      hb_fsSetAttr( ( BYTE * ) szName, ufi.external_fa & 0xFF );
#  endif

#elif defined( HB_OS_OS2 )
   {
      FILESTATUS3 fs3;
      APIRET ulrc;
      ULONG ulAttr = FILE_NORMAL;
      int iAttr = ufi.external_fa & 0xFF;

      if( iAttr & HB_FA_READONLY )
         ulAttr |= FILE_READONLY;
      if( iAttr & HB_FA_HIDDEN )
         ulAttr |= FILE_HIDDEN;
      if( iAttr & HB_FA_SYSTEM )
         ulAttr |= FILE_SYSTEM;
      if( iAttr & HB_FA_ARCHIVE )
         ulAttr |= FILE_ARCHIVED;

      ulrc = DosQueryPathInfo( szName, FIL_STANDARD, &fs3, sizeof( fs3 ) );
      if( ulrc == NO_ERROR )
      {
         FDATE   fdate;
         FTIME   ftime;

         fdate.year = ufi.tmu_date.tm_year - 1980;
         fdate.month = ufi.tmu_date.tm_mon;
         fdate.day = ufi.tmu_date.tm_mday;
         ftime.hours = ufi.tmu_date.tm_hour;
         ftime.minutes = ufi.tmu_date.tm_min;
         ftime.twosecs = ufi.tmu_date.tm_sec / 2;

         fs3.attrFile = ulAttr;

         fs3.fdateCreation = fs3.fdateLastAccess = fs3.fdateLastWrite = fdate;
         fs3.ftimeCreation = fs3.ftimeLastAccess = fs3.ftimeLastWrite = ftime;
         ulrc = DosSetPathInfo( szName, FIL_STANDARD,
                                &fs3, sizeof( fs3 ), DSPI_WRTTHRU );
      }
   }
#else
   {
      hb_fsSetAttr( ( BYTE * ) szName, ufi.external_fa );
   }
#endif

   return iResult;
}


/*  HB_UnzipExtractCurrentFile( hZip, [ cFileName ], [ cPassword ] ) --> nError */
HB_FUNC( HB_UNZIPEXTRACTCURRENTFILE )
{
   unzFile hUnzip = hb_unzipfileParam( 1 );

   if( hUnzip )
      hb_retni( hb_unzipExtractCurrentFile( hUnzip, hb_parc( 2 ), hb_parc( 3 ) ) );
}


static int hb_zipDeleteFile( const char* szZipFile, const char* szFileMask )
{
   char szTempFile[ HB_PATH_MAX ];
   char szCurrFile[ HB_PATH_MAX ];
   PHB_FNAME pFileName;
   FHANDLE hFile;
   unzFile hUnzip;
   zipFile hZip;
   unz_global_info ugi;
   unz_file_info ufi;
   zip_fileinfo zfi;
   char* pszGlobalComment = NULL;
   char* pszFileComment = NULL;
   void* pExtraField = NULL;
   void* pLocalExtraField = NULL;
   int iFilesLeft = 0;
   int iFilesDel = 0;
   int iExtraFieldLen;
   int method;
   int level;
   int iResult;

   /* open source file */
   hUnzip = unzOpen( szZipFile );
   if( hUnzip == NULL )
      return UNZ_ERRNO;

   pFileName = hb_fsFNameSplit( szZipFile );
   hFile = hb_fsCreateTemp( pFileName->szPath, NULL, FC_NORMAL, szTempFile );
   hZip = NULL;
   if( hFile != FS_ERROR )
   {
      hb_fsClose( hFile );
      hZip = zipOpen( szTempFile, APPEND_STATUS_CREATE );
   }
   hb_xfree( pFileName );

   if( hZip == NULL )
   {
      unzClose( hUnzip );
      return UNZ_ERRNO;
   }

   iResult = unzGetGlobalInfo( hUnzip, &ugi );
   if( iResult == UNZ_OK )
   {
      if( ugi.size_comment > 0 )
      {
         pszGlobalComment = ( char * ) hb_xgrab( ugi.size_comment + 1 );
         if( ( uLong ) unzGetGlobalComment( hUnzip, pszGlobalComment,
                                 ugi.size_comment ) != ugi.size_comment )
            iResult = UNZ_ERRNO;
         pszGlobalComment[ ugi.size_comment ] = '\0';
      }
      if( iResult == UNZ_OK )
         iResult = unzGoToFirstFile( hUnzip );
   }

   while( iResult == UNZ_OK )
   {
      iResult = unzGetCurrentFileInfo( hUnzip, &ufi, szCurrFile, HB_PATH_MAX - 1, NULL, 0, NULL, 0 );
      if( iResult != UNZ_OK )
         break;

      if( hb_strMatchFile( szCurrFile, szFileMask ) )
         iFilesDel++;
      else
      {
         if( ufi.size_file_extra )
            pExtraField = ( char * ) hb_xgrab( ufi.size_file_extra );
         if( ufi.size_file_comment )
            pszFileComment = ( char * ) hb_xgrab( ufi.size_file_comment + 1 );

         iResult = unzGetCurrentFileInfo( hUnzip, NULL, NULL, 0,
                                          pExtraField, ufi.size_file_extra,
                                          pszFileComment, ufi.size_file_comment );
         if( ufi.size_file_comment )
            pszFileComment[ ufi.size_file_comment ] = '\0';
         if( iResult != UNZ_OK )
            break;

         iResult = unzOpenCurrentFile2( hUnzip, &method, &level, 1 );
         if( iResult != UNZ_OK )
            break;

         iExtraFieldLen = unzGetLocalExtrafield( hUnzip, NULL, 0 );
         if( iExtraFieldLen < 0 )
         {
            iResult = UNZ_ERRNO;
            break;
         }
         else if( iExtraFieldLen > 0 )
         {
            pLocalExtraField = hb_xgrab( iExtraFieldLen );
            if( unzGetLocalExtrafield( hUnzip, pLocalExtraField, iExtraFieldLen ) != iExtraFieldLen )
            {
               iResult = UNZ_ERRNO;
               break;
            }
         }

         memset( &zfi, 0, sizeof( zfi ) );
         memcpy( &zfi.tmz_date, &ufi.tmu_date, sizeof( tm_unz ) );
         zfi.dosDate = ufi.dosDate;
         zfi.internal_fa = ufi.internal_fa;
         zfi.external_fa = ufi.external_fa;

         iResult = zipOpenNewFileInZip2( hZip, szCurrFile, &zfi, pLocalExtraField, iExtraFieldLen, pExtraField, ufi.size_file_extra, pszFileComment, method, level, 1 );
         if( iResult != UNZ_OK )
            break;

         if( ufi.compressed_size )
         {
            BYTE * buffer = ( BYTE * ) hb_xgrab( HB_Z_IOBUF_SIZE );
            uLong ulLeft = ufi.compressed_size;

            while( ulLeft > 0 )
            {
               int iRead = HB_MIN( ulLeft, HB_Z_IOBUF_SIZE );
               iResult = unzReadCurrentFile( hUnzip, ( voidp ) buffer, iRead );
               if( iResult < 0 )
                  break;
               if( iResult != iRead )
               {
                  iResult = UNZ_ERRNO;
                  break;
               }
               iResult = zipWriteInFileInZip( hZip, ( voidp ) buffer, iRead );
               if( iResult != UNZ_OK )
                  break;
               ulLeft -= iRead;
            }
            hb_xfree( buffer );
            if( iResult != UNZ_OK )
               break;
         }

         iResult = zipCloseFileInZipRaw( hZip, ufi.uncompressed_size, ufi.crc );
         if( iResult != UNZ_OK )
            break;

         iResult = unzCloseCurrentFile( hUnzip );
         if( iResult != UNZ_OK )
            break;

         if( pExtraField )
         {
            hb_xfree( pExtraField );
            pExtraField = NULL;
         }
         if( pszFileComment )
         {
            hb_xfree( pszFileComment );
            pszFileComment = NULL;
         }
         if( pLocalExtraField )
         {
            hb_xfree( pLocalExtraField );
            pLocalExtraField = NULL;
         }
         iFilesLeft++;
      }
      iResult = unzGoToNextFile( hUnzip );
   }

   if( pExtraField )
      hb_xfree( pExtraField );
   if( pszFileComment )
      hb_xfree( pszFileComment );
   if( pLocalExtraField )
      hb_xfree( pLocalExtraField );

   if( iFilesDel == 0 )
      iResult = UNZ_ERRNO;
   else if( iResult == UNZ_END_OF_LIST_OF_FILE )
      iResult = UNZ_OK;

   if( iResult != UNZ_OK )
      zipClose( hZip, NULL );
   else
      iResult = zipClose( hZip, pszGlobalComment );
   unzClose( hUnzip );
   if( pszGlobalComment )
      hb_xfree( pszGlobalComment );

   if( iResult != UNZ_OK )
      hb_fsDelete( szTempFile );
   else
   {
      hb_fsDelete( szZipFile );

      if( iFilesLeft == 0 )
         hb_fsDelete( szTempFile );
      else if( !hb_fsRename( szTempFile, szZipFile ) )
         iResult = UNZ_ERRNO;
   }

   return iResult;
}

/*  HB_ZipDeleteFile( cZipFile, cFileMask ) --> nError */
HB_FUNC( HB_ZIPDELETEFILE )
{
   const char * szZipFile  = hb_parc( 1 );
   const char * szFileMask = hb_parc( 2 );

   if( szZipFile && szFileMask )
      hb_retni( hb_zipDeleteFile( szZipFile, szFileMask ) );
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS );
}
