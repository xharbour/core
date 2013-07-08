/*
 * $Id: filesys.c 9858 2012-12-06 01:40:02Z druzus $
 */

/*
 * Harbour Project source code:
 * The FileSys API (C level)
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
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

/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
 *    hb_fsSetError()
 *    hb_fsSetDevMode()
 *    hb_fsReadLarge()
 *    hb_fsWriteLarge()
 *    hb_fsCurDirBuff()
 *
 * Copyright 1999 Jose Lalin <dezac@corevia.com>
 *    hb_fsChDrv()
 *    hb_fsCurDrv()
 *    hb_fsIsDrv()
 *    hb_fsIsDevice()
 *
 * Copyright 2000 Luiz Rafael Culik <culik@sl.conex.net>
 *            and David G. Holm <dholm@jsd-llc.com>
 *    hb_fsEof()
 *
 * Copyright 2001 Jose Gimenez (JFG) <jfgimenez@wanadoo.es>
 *                                   <tecnico.sireinsa@ctv.es>
 *    Added __WIN32__ check for any compiler to use the Win32
 *    API calls to allow openning an unlimited number of files
 *    simultaneously.
 *
 * Copyright 2003 Giancarlo Niccolai <gian@niccolai.ws>
 *    hb_fsOpenProcess()
 *    hb_fsProcessValue()
 *    hb_fsCloseProcess()
 *    Unix sensible creation flags
 *    Thread safing and optimization
 *
 * See doc/license.txt for licensing terms.
 *
 */

/* NOTE: In DOS/DJGPP under WinNT4 hb_fsSeek( fhnd, offset < 0, FS_SET ) will
         set the file pointer to the passed negative value and the subsequent
         hb_fsWrite() call will fail. In CA-Cl*pper, _fsSeek() will fail,
         the pointer will not be moved and thus the _fsWrite() call will
         successfully write the buffer to the current file position. [vszakats]

   This has been corrected by ptucker
 */

/* *nixes */
#if ! defined( _LARGEFILE64_SOURCE )
#  define _LARGEFILE64_SOURCE 1
#endif
#if ! defined( _XOPEN_SOURCE )
#  define _XOPEN_SOURCE       500
#endif

/* OS2 */
#define INCL_DOSFILEMGR       /* File Manager values */
#define INCL_DOSERRORS        /* DOS error values    */
#define INCL_DOSDATETIME      /* DATETIME functions  */

/* W32 */
#ifndef HB_OS_WIN_USED
   #define HB_OS_WIN_USED
#endif

#define HB_THREAD_OPTIMIZE_STACK

#include <string.h>
#include <ctype.h>

#include "hbapi.h"
#include "hbvm.h"
#include "hbapifs.h"
#include "hbapierr.h"
#include "hbdate.h"
#include "hb_io.h"
#include "hbset.h"

#if defined( HB_OS_UNIX )
   #include <unistd.h>
   #include <time.h>
   #include <utime.h>
   #include <signal.h>
   #include <sys/types.h>
   #include <sys/wait.h>
   #if defined( HB_OS_DARWIN )
      #include <crt_externs.h>
      #define environ ( *_NSGetEnviron() )
      #if ! defined( HB_OS_DARWIN_5 ) && defined( _POSIX_C_SOURCE )
/* these declarations are hidden in Darwin headers when
   _POSIX_C_SOURCE is defined, so declare them ourself */
struct tm * gmtime_r( const time_t *, struct tm * );
struct tm * localtime_r( const time_t *, struct tm * );
int fsync( int );
      #endif
   #elif ! defined( __WATCOMC__ )
extern char ** environ;
   #endif
#endif

#if ( defined( __DMC__ ) || defined( __BORLANDC__ ) || \
   defined( __IBMCPP__ ) || defined( _MSC_VER ) || \
   defined( __MINGW32__ ) || defined( __WATCOMC__ ) ) && \
   ! defined( HB_OS_UNIX ) && ! defined( HB_OS_WIN_CE )
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <process.h>
   #if ! defined( __POCC__ ) && ! defined( __XCC__ )
      #include <share.h>
   #endif
   #include <errno.h>
   #include <direct.h>
   #if defined( __BORLANDC__ )
      #include <dir.h>
      #include <dos.h>
   #elif defined( __WATCOMC__ )
      #include <dos.h>
   #endif

   #if defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __DMC__ )
      #include <sys/locking.h>
      #define ftruncate    _chsize
      #if defined( __MINGW32__ ) && ! defined( _LK_UNLCK )
         #define _LK_UNLCK _LK_UNLOCK
      #endif
   #else
      #define ftruncate    chsize
   #endif
   #if ! defined( HAVE_POSIX_IO )
      #define HAVE_POSIX_IO
   #endif
#elif defined( __GNUC__ ) || defined( HB_OS_UNIX )
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <errno.h>
   #if defined( __CYGWIN__ )
      #include <io.h>
   #elif defined( __DJGPP__ )
      #include <dir.h>
      #include <utime.h>
      #include <time.h>
   #endif
   #if ! defined( HAVE_POSIX_IO )
      #define HAVE_POSIX_IO
   #endif
#endif

#if defined( __MPW__ )
   #include <fcntl.h>
#endif

#if defined( HB_OS_HPUX )
extern int fdatasync( int fildes );
#elif defined( HB_OS_DOS )
   #include <dos.h>
   #include <time.h>
   #include <utime.h>
#elif defined( HB_OS_OS2 )
   #include <sys/signal.h>
   #include <sys/process.h>
   #include <sys/wait.h>
   #include <share.h>
   #ifndef SH_COMPAT
      #define SH_COMPAT SH_DENYNO
   #endif

/* 15/12/2005 - <maurilio.longo@libero.it>
                Due to a 'feature' of latest GCC on OS/2 I have to use
                DosOpen() to open files instead of LIBC sopen() since
                sopen() now adds EAs to mew files and fails if filesystem
                does not handle them. Removing this #define puts back
                LIBC code.
 */
   #define  HB_OS2_IO

#elif defined( HB_WIN32_IO )
   #include <windows.h>

   #if ! defined( INVALID_SET_FILE_POINTER ) && \
   ( defined( __DMC__ ) || defined( _MSC_VER ) || defined( __LCC__ ) )
      #define INVALID_SET_FILE_POINTER ( ( DWORD ) -1 )
   #endif
   #if ! defined( INVALID_FILE_ATTRIBUTES )
      #define INVALID_FILE_ATTRIBUTES  ( ( DWORD ) -1 )
   #endif
#endif
#if defined( HB_USE_SHARELOCKS ) && defined( HB_USE_BSDLOCKS )
   #include <sys/file.h>
#endif

#if ! defined( HB_USE_LARGEFILE64 ) && defined( HB_OS_UNIX )
   #if defined( __USE_LARGEFILE64 )
/*
 * The macro: __USE_LARGEFILE64 is set when _LARGEFILE64_SOURCE is
 * defined and efectively enables lseek64/flock64/ftruncate64 functions
 * on 32bit machines.
 */
      #define HB_USE_LARGEFILE64
   #elif defined( HB_OS_HPUX ) && defined( O_LARGEFILE )
      #define HB_USE_LARGEFILE64
   #endif
#endif

#if defined( HB_OS_HAS_DRIVE_LETTER )
/* 27/08/2004 - <maurilio.longo@libero.it>
                HB_FS_GETDRIVE() should return a number in the range 0..25 ('A'..'Z')
                HB_FS_SETDRIVE() should accept a number inside same range.

                If a particular platform/compiler returns/accepts different ranges of
                values, simply define a branch for that platform.

                NOTE: There is not an implicit "current disk", ALWAYS use

                        my_func( hb_fsCurDrv(), ...)

                      to refer to current disk
 */

#if defined( __DJGPP__ )
   #define HB_FS_GETDRIVE( n )   do { n = getdisk(); } while( 0 )
   #define HB_FS_SETDRIVE( n )   setdisk( n )

#elif defined( __WATCOMC__ )
   #define HB_FS_GETDRIVE( n )   do { _dos_getdrive( &( n ) ); --( n ); } while( 0 )
   #define HB_FS_SETDRIVE( n )   do { \
      UINT uiDummy; \
      _dos_setdrive( ( n ) + 1, &uiDummy ); \
} while( 0 )

#elif defined( HB_OS_OS2 )
   #define HB_FS_GETDRIVE( n )   do { n = _getdrive() - 'A'; } while( 0 )
   #define HB_FS_SETDRIVE( n )   _chdrive( ( n ) + 'A' )

#else
   #define HB_FS_GETDRIVE( n )   do { \
      n  = _getdrive(); \
      n  -= ( ( n ) < 'A' ) ? 1 : 'A'; \
} while( 0 )
   #define HB_FS_SETDRIVE( n )   _chdrive( ( n ) + 1 )

#endif
#endif /* HB_OS_HAS_DRIVE_LETTER */

#ifndef O_BINARY
   #define O_BINARY     0       /* O_BINARY not defined on Linux */
#endif

#ifndef O_LARGEFILE
   #define O_LARGEFILE  0       /* O_LARGEFILE is used for LFS in 32-bit Linux */
#endif

#if ! defined( HB_OS_UNIX )
   #if ! defined( S_IREAD ) && defined( S_IRUSR )
      #define S_IREAD   S_IRUSR
   #endif
   #if ! defined( S_IWRITE ) && defined( S_IWUSR )
      #define S_IWRITE  S_IWUSR
   #endif
   #if ! defined( S_IEXEC ) && defined( S_IXUSR )
      #define S_IEXEC   S_IXUSR
   #endif
#endif


#if defined( _MSC_VER ) && ! defined( __XCC__ )
#define eof             _eof
#endif


#if defined( HAVE_POSIX_IO ) || defined( HB_WIN32_IO ) || defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __LCC__ ) || defined( __DMC__ )
/* Only compilers with Posix or Posix-like I/O support are supported */
   #define HB_FS_FILE_IO
#endif

#if defined( __DMC__ ) || defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __IBMCPP__ ) || defined( __WATCOMC__ ) || defined( HB_OS_OS2 )
/* These compilers use sopen() rather than open(), because their
   versions of open() do not support combined O_ and SH_ flags */
   #define HB_FS_SOPEN
#endif

#if UINT_MAX == USHRT_MAX
   #define LARGE_MAX ( UINT_MAX - 1L )
#else
   #define HB_FS_LARGE_OPTIMIZED
#endif

static BOOL s_fUseWaitLocks = TRUE;

#if defined( HB_FS_FILE_IO )

#if defined( HB_WIN32_IO )

static HANDLE DosToWinHandle( HB_FHANDLE fHandle )
{
   if( fHandle == ( HB_FHANDLE ) HB_STDIN_HANDLE )
      return GetStdHandle( STD_INPUT_HANDLE );

   else if( fHandle == ( HB_FHANDLE ) HB_STDOUT_HANDLE )
      return GetStdHandle( STD_OUTPUT_HANDLE );

   else if( fHandle == ( HB_FHANDLE ) HB_STDERR_HANDLE )
      return GetStdHandle( STD_ERROR_HANDLE );

   else
      return ( HANDLE ) fHandle;
}

static void convert_open_flags( BOOL fCreate, ULONG ulAttr, USHORT uiFlags,
                                DWORD * dwMode, DWORD * dwShare,
                                DWORD * dwCreat, DWORD * dwAttr )
{
   if( fCreate )
   {
      *dwCreat = CREATE_ALWAYS;
      *dwMode  = GENERIC_READ | GENERIC_WRITE;
   }
   else
   {
      if( uiFlags & FO_CREAT )
      {
         if( uiFlags & FO_EXCL )
            *dwCreat = CREATE_NEW;
         else if( uiFlags & FO_TRUNC )
            *dwCreat = CREATE_ALWAYS;
         else
            *dwCreat = OPEN_ALWAYS;
      }
      else if( uiFlags & FO_TRUNC )
      {
         *dwCreat = TRUNCATE_EXISTING;
      }
      else
      {
         *dwCreat = OPEN_EXISTING;
      }

      *dwMode = 0;
      switch( uiFlags & ( FO_READ | FO_WRITE | FO_READWRITE ) )
      {
         case FO_READWRITE:
            *dwMode  |= GENERIC_READ | GENERIC_WRITE;
            break;
         case FO_WRITE:
            *dwMode  |= GENERIC_WRITE;
            break;
         case FO_READ:
            *dwMode  |= GENERIC_READ;
            break;
      }
   }

   /* shared flags */
   switch( uiFlags & ( FO_DENYREAD | FO_DENYWRITE | FO_EXCLUSIVE | FO_DENYNONE ) )
   {
      case FO_DENYREAD:
         *dwShare = FILE_SHARE_WRITE;
         break;
      case FO_DENYWRITE:
         *dwShare = FILE_SHARE_READ;
         break;
      case FO_EXCLUSIVE:
         *dwShare = 0;
         break;
      default:
         *dwShare = FILE_SHARE_WRITE | FILE_SHARE_READ;
         break;
   }

   /* file attributes flags */
   if( ulAttr == FC_NORMAL )
   {
      *dwAttr = FILE_ATTRIBUTE_NORMAL;
   }
   else if( ulAttr == FC_TEMPORARY )
   {
#if defined( HB_OS_WIN )
      *dwAttr  = FILE_ATTRIBUTE_TEMPORARY;
#else
      *dwAttr  = FILE_ATTRIBUTE_NORMAL;
#endif
   }
   else
   {
#if defined( HB_OS_WIN )
      if( ulAttr & FC_TEMPORARY )
         *dwAttr = FILE_ATTRIBUTE_TEMPORARY;
      else
         *dwAttr = FILE_ATTRIBUTE_ARCHIVE;
#else
      *dwAttr = FILE_ATTRIBUTE_ARCHIVE;
#endif
      if( ulAttr & FC_READONLY )
         *dwAttr |= FILE_ATTRIBUTE_READONLY;
      if( ulAttr & FC_HIDDEN )
         *dwAttr |= FILE_ATTRIBUTE_HIDDEN;
      if( ulAttr & FC_SYSTEM )
         *dwAttr |= FILE_ATTRIBUTE_SYSTEM;
   }
}


#elif defined( HB_OS2_IO )

static void convert_open_flags( BOOL fCreate, ULONG ulAttr, USHORT uiFlags,
                                ULONG * ulAttribute,
                                ULONG * fsOpenFlags,
                                ULONG * fsOpenMode )
{
   if( fCreate )
   {
      *fsOpenFlags   = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS;
      *fsOpenMode    = OPEN_ACCESS_READWRITE | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT;
   }
   else
   {
      if( uiFlags & FO_CREAT )
      {
         if( uiFlags & FO_EXCL )
            *fsOpenFlags = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS;
         else if( uiFlags & FO_TRUNC )
            *fsOpenFlags = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS;
         else
            *fsOpenFlags = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
      }
      else if( uiFlags & FO_TRUNC )
      {
         *fsOpenFlags = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS;
      }
      else
      {
         *fsOpenFlags = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
      }

      *fsOpenMode = 0 | OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NOINHERIT;

      switch( uiFlags & ( FO_READ | FO_WRITE | FO_READWRITE ) )
      {
         case FO_READWRITE:
            *fsOpenMode |= OPEN_ACCESS_READWRITE;
            break;
         case FO_WRITE:
            *fsOpenMode |= OPEN_ACCESS_WRITEONLY;
            break;
         case FO_READ:
            *fsOpenMode |= OPEN_ACCESS_READONLY;
            break;
      }
   }

   /* shared flags */
   switch( uiFlags & ( FO_DENYREAD | FO_DENYWRITE | FO_EXCLUSIVE | FO_DENYNONE ) )
   {
      case FO_DENYREAD:
         *fsOpenMode |= OPEN_SHARE_DENYREAD;
         break;
      case FO_DENYWRITE:
         *fsOpenMode |= OPEN_SHARE_DENYWRITE;
         break;
      case FO_EXCLUSIVE:
         *fsOpenMode |= OPEN_SHARE_DENYREADWRITE;
         break;
      default:
         *fsOpenMode |= OPEN_SHARE_DENYNONE;
         break;
   }

   /* file attributes flags */
   if( ulAttr == FC_NORMAL )
   {
      *ulAttribute = FILE_NORMAL;
   }
   else
   {
      *ulAttribute = FILE_NORMAL | FILE_ARCHIVED;

      if( ulAttr & FC_READONLY )
         *ulAttribute |= FILE_READONLY;

      if( ulAttr & FC_HIDDEN )
         *ulAttribute |= FILE_HIDDEN;

      if( ulAttr & FC_SYSTEM )
         *ulAttribute |= FILE_SYSTEM;
   }
}


#else

static void convert_open_flags( BOOL fCreate, ULONG ulAttr, USHORT uiFlags,
                                int * flags, unsigned * mode,
                                int * share, int * attr )
{
   HB_TRACE( HB_TR_DEBUG, ( "convert_open_flags(%d, %lu, %hu, %p, %p, %p, %p)", fCreate, ulAttr, uiFlags, flags, mode, share, attr ) );

   /* file access mode */
#if defined( HB_OS_UNIX )
   *mode = HB_FA_POSIX_ATTR( ulAttr );
   if( *mode == 0 )
   {
      *mode = ( ulAttr & FC_HIDDEN ) ? S_IRUSR : ( S_IRUSR | S_IRGRP | S_IROTH );
      if( ! ( ulAttr & FC_READONLY ) )
      {
         if( *mode & S_IRUSR )
            *mode |= S_IWUSR;
         if( *mode & S_IRGRP )
            *mode |= S_IWGRP;
         if( *mode & S_IROTH )
            *mode |= S_IWOTH;
      }
      if( ulAttr & FC_SYSTEM )
      {
         if( *mode & S_IRUSR )
            *mode |= S_IXUSR;
         if( *mode & S_IRGRP )
            *mode |= S_IXGRP;
         if( *mode & S_IROTH )
            *mode |= S_IXOTH;
      }
   }
#else
   *mode = S_IREAD |
           ( ( ulAttr & FC_READONLY ) ? 0 : S_IWRITE ) |
           ( ( ulAttr & FC_SYSTEM ) ? S_IEXEC : 0 );
#endif

   /* dos file attributes */
#if defined( HB_FS_DOSATTR )
   if( ulAttr == FC_NORMAL )
   {
      *attr = _A_NORMAL;
   }
   else
   {
      *attr = _A_ARCH;
      if( ulAttr & FC_READONLY )
         *attr |= _A_READONLY;
      if( ulAttr & FC_HIDDEN )
         *attr |= _A_HIDDEN;
      if( ulAttr & FC_SYSTEM )
         *attr |= _A_SYSTEM;
   }
#else
   *attr = 0;
#endif

   if( fCreate )
   {
      *flags = O_RDWR | O_CREAT | O_TRUNC | O_BINARY | O_LARGEFILE |
               ( ( uiFlags & FO_EXCL ) ? O_EXCL : 0 );
   }
   else
   {
      *attr    = 0;
      *flags   = O_BINARY | O_LARGEFILE;
      switch( uiFlags & ( FO_READ | FO_WRITE | FO_READWRITE ) )
      {
         case FO_READ:
            *flags   |= O_RDONLY;
            break;
         case FO_WRITE:
            *flags   |= O_WRONLY;
            break;
         case FO_READWRITE:
            *flags   |= O_RDWR;
            break;
         default:
            /* this should not happen and it's here to force default OS behavior */
            *flags |= ( O_RDONLY | O_WRONLY | O_RDWR );
            break;
      }

      if( uiFlags & FO_CREAT )
         *flags |= O_CREAT;
      if( uiFlags & FO_TRUNC )
         *flags |= O_TRUNC;
      if( uiFlags & FO_EXCL )
         *flags |= O_EXCL;
   }

   /* shared flags (HB_FS_SOPEN) */
#if defined( _MSC_VER ) || defined( __DMC__ )
   if( ( uiFlags & FO_DENYREAD ) == FO_DENYREAD )
      *share = _SH_DENYRD;
   else if( uiFlags & FO_EXCLUSIVE )
      *share = _SH_DENYRW;
   else if( uiFlags & FO_DENYWRITE )
      *share = _SH_DENYWR;
   else if( uiFlags & FO_DENYNONE )
      *share = _SH_DENYNO;
   else
      *share = _SH_COMPAT;
#elif ! defined( HB_OS_UNIX )
   if( ( uiFlags & FO_DENYREAD ) == FO_DENYREAD )
      *share = SH_DENYRD;
   else if( uiFlags & FO_EXCLUSIVE )
      *share = SH_DENYRW;
   else if( uiFlags & FO_DENYWRITE )
      *share = SH_DENYWR;
   else if( uiFlags & FO_DENYNONE )
      *share = SH_DENYNO;
   else
      *share = SH_COMPAT;
#else
   *share = 0;
#endif

   HB_TRACE( HB_TR_INFO, ( "convert_open_flags: flags=0x%04x, mode=0x%04x, share=0x%04x, attr=0x%04x", *flags, *mode, *share, *attr ) );

}
#endif

static USHORT convert_seek_flags( USHORT uiFlags )
{
   /* by default FS_SET is set */
   USHORT result_flags = SEEK_SET;

   HB_TRACE( HB_TR_DEBUG, ( "convert_seek_flags(%hu)", uiFlags ) );

   if( uiFlags & FS_RELATIVE )
      result_flags = SEEK_CUR;

   if( uiFlags & FS_END )
      result_flags = SEEK_END;

   return result_flags;
}

#endif


/*
 * filesys.api functions:
 */

HB_FHANDLE hb_fsGetOsHandle( HB_FHANDLE hFileHandle )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_fsGetOsHandle(%p)", ( void * ) ( HB_PTRDIFF ) hFileHandle ) );

#if defined( HB_WIN32_IO )
   return ( HB_FHANDLE ) DosToWinHandle( hFileHandle );
#else
   return hFileHandle;
#endif
}

HB_FHANDLE hb_fsPOpen( const char * pFilename, const char * pMode )
{
#if defined( HB_OS_UNIX )
   HB_THREAD_STUB
#endif

   HB_FHANDLE hFileHandle = FS_ERROR;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsPOpen(%p, %s)", pFilename, pMode ) );

#if defined( HB_OS_UNIX ) && ! defined( __CYGWIN__ )
   {
      HB_FHANDLE  hPipeHandle[ 2 ], hNullHandle;
      pid_t       pid;
      char *      pbyTmp;
      BOOL        bRead;
      ULONG       ulLen;
      int         iMaxFD;

      ulLen = strlen( pFilename );
      if( pMode && ( *pMode == 'r' || *pMode == 'w' ) )
         bRead = ( *pMode == 'r' );
      else
      {
         if( pFilename[ 0 ] == '|' )
            bRead = FALSE;
         else if( pFilename[ ulLen - 1 ] == '|' )
            bRead = TRUE;
         else
            bRead = FALSE;
      }

      if( pFilename[ 0 ] == '|' )
      {
         ++pFilename;
         --ulLen;
      }
      if( pFilename[ ulLen - 1 ] == '|' )
      {
         pbyTmp            = hb_strdup( pFilename );
         pbyTmp[ --ulLen ] = 0;
         pFilename         = pbyTmp;
      }
      else
         pbyTmp = NULL;

      //JC1: unlocking the stack to allow cancelation points
      hb_vmUnlock();

      if( pipe( hPipeHandle ) == 0 )
      {
         if( ( pid = fork() ) != -1 )
         {
            if( pid != 0 )
            {
               if( bRead )
               {
                  close( hPipeHandle[ 1 ] );
                  hFileHandle = hPipeHandle[ 0 ];
               }
               else
               {
                  close( hPipeHandle[ 0 ] );
                  hFileHandle = hPipeHandle[ 1 ];
               }
            }
            else
            {
               char * argv[ 4 ];
               argv[ 0 ]   = ( char * ) "sh";
               argv[ 1 ]   = ( char * ) "-c";
               argv[ 2 ]   = ( char * ) pFilename;
               argv[ 3 ]   = ( char * ) 0;
               hNullHandle = open( "/dev/null", O_RDWR );
               if( bRead )
               {
                  close( hPipeHandle[ 0 ] );
                  dup2( hPipeHandle[ 1 ], 1 );
                  dup2( hNullHandle, 0 );
                  dup2( hNullHandle, 2 );
               }
               else
               {
                  close( hPipeHandle[ 1 ] );
                  dup2( hPipeHandle[ 0 ], 0 );
                  dup2( hNullHandle, 1 );
                  dup2( hNullHandle, 2 );
               }
               iMaxFD = sysconf( _SC_OPEN_MAX );
               if( iMaxFD < 3 )
                  iMaxFD = 1024;
               for( hNullHandle = 3; hNullHandle < iMaxFD; ++hNullHandle )
                  close( hNullHandle );
               setuid( getuid() );
               setgid( getgid() );
               execve( "/bin/sh", argv, environ );
               exit( 1 );
            }
         }
         else
         {
            close( hPipeHandle[ 0 ] );
            close( hPipeHandle[ 1 ] );
         }
      }
      hb_fsSetIOError( hFileHandle != FS_ERROR, 0 );
      hb_vmLock();

      if( pbyTmp )
         hb_xfree( pbyTmp );
   }
#else

   HB_SYMBOL_UNUSED( pFilename );
   HB_SYMBOL_UNUSED( pMode );

   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return hFileHandle;
}

#ifndef HB_WIN32_IO

int s_parametrize( char * out, const char * in )
{
   int count = 0;  // we'll have at least one token

   // removes leading spaces
   while( *in && isspace( ( BYTE ) *in ) )
      in++;
   if( ! *in )
      return 0;

   while( *in )
   {
      if( *in == '\"' || *in == '\'' )
      {
         char quote = *in;
         in++;
         while( *in && *in != quote )
         {
            if( *in == '\\' )
            {
               in++;
            }
            if( *in )
            {
               *out = *in;
               out++;
               in++;
            }
         }
         if( *in )
         {
            in++;
         }
         if( *in )
         {
            *out = '\0';
         }
         // out++ will be done later; if in is done,
         // '\0' will be added at loop exit.
      }
      else if( ! isspace( ( BYTE ) *in ) )
      {
         *out = *in;
         in++;
         out++;
      }
      else
      {
         *out = '\0';
         count++;
         while( *in && isspace( ( BYTE ) *in ) )
            in++;
         out++;
      }
   }
   *out = '\0';
   count++;

   return count;
}

char ** s_argvize( char * params, int size )
{
   int      i;
   char **  argv = ( char ** ) hb_xgrab( sizeof( char * ) * ( size + 1 ) );

   for( i = 0; i < size; i++ )
   {
      argv[ i ] = params;
      while( *params )
         params++;
      params++;
   }
   return argv;
}

#endif

/*
   JC1: Process Control functions
   hb_fsOpenProcess creates a process and get the control of the 4 main
   standard handlers. The handlers are returned in HB_FHANDLE pointers;
   each of them can be 0 if the owner process don't want to get
   the ownership of that specific handler.

   The process return a resource identificator that allows to
   wait for the child process to be stopped. Incidentally, the
   type of the process resource identificator is the same as
   a file handle in all the systems were are using this functions:
   in windows it is a HANDLE, while in unix is a PID_T (that is
   an integer == file handle in all the GNU derived unces).

   On success, a valid FHandle is returned, and FError returns
   zero. On error, -1 is returned and FError() returns nonzero.
 */

HB_FHANDLE hb_fsOpenProcess( const char * pFilename, HB_FHANDLE * fhStdin,
                             HB_FHANDLE * fhStdout, HB_FHANDLE * fhStderr,
                             BOOL bBackground, ULONG * ProcessID )
{
   HB_FHANDLE hRet = ( HB_FHANDLE ) FS_ERROR;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsOpenProcess(%s, %p, %p, %p )", pFilename, fhStdin, fhStdout, fhStderr ) );

#if defined( HB_OS_UNIX ) || ( defined( HB_OS_WIN ) && ! defined( HB_WIN32_IO ) ) || defined ( HB_OS_OS2 )
   {
   #ifndef MAXFD
      #define MAXFD 1024
   #endif

      HB_FHANDLE  hPipeIn[ 2 ], hPipeOut[ 2 ], hPipeErr[ 2 ];
      HB_FHANDLE  hNull;
      char **     argv;
      int         size;
      char *      command;

   #ifdef HB_OS_WIN
      int         pid;
      #define pipe( x ) _pipe( x, 2048, _O_BINARY )
   #else
      pid_t       pid;
   #endif

      if( fhStdin != 0 && pipe( hPipeIn ) != 0 )
      {
         hb_fsSetIOError( FALSE, 0 );
         return ( HB_FHANDLE ) -1;
 #ifdef HB_OS_OS2
      }
      else
      {
         setmode( hPipeIn[ 0 ], O_BINARY );
         setmode( hPipeIn[ 1 ], O_BINARY );
 #endif
      }

      if( fhStdout != 0 && pipe( hPipeOut ) != 0 )
      {
         hb_fsSetIOError( FALSE, 0 );

         if( fhStdin != 0 )
         {
            close( hPipeIn[ 0 ] );
            close( hPipeIn[ 1 ] );
         }

         return ( HB_FHANDLE ) -1;
 #ifdef HB_OS_OS2
      }
      else
      {
         setmode( hPipeOut[ 0 ], O_BINARY );
         setmode( hPipeOut[ 1 ], O_BINARY );
 #endif
      }

      if( fhStderr != 0 )
      {
         if( fhStderr != fhStdout )
         {
            if( pipe( hPipeErr ) != 0 )
            {
               hb_fsSetIOError( FALSE, 0 );

               if( fhStdin != 0 )
               {
                  close( hPipeIn[ 0 ] );
                  close( hPipeIn[ 1 ] );
               }
               if( fhStdout != 0 )
               {
                  close( hPipeOut[ 0 ] );
                  close( hPipeOut[ 1 ] );
               }

               return ( HB_FHANDLE ) -1;
       #ifdef HB_OS_OS2
            }
            else
            {
               setmode( hPipeErr[ 0 ], O_BINARY );
               setmode( hPipeErr[ 1 ], O_BINARY );
       #endif
            }
         }
      }

   #if defined( HB_OS_WIN ) || defined( HB_OS_OS2 )
      {
         int   oldstdin, oldstdout, oldstderr;
         int   iFlags;

         hNull       = open( "NUL:", O_RDWR );

         oldstdin    = dup( 0 );
         oldstdout   = dup( 1 );
         oldstderr   = dup( 2 );

         if( fhStdin != 0 )
         {
            dup2( hPipeIn[ 0 ], 0 );
         }
         else if( bBackground )
         {
            dup2( hNull, 0 );
         }

         if( fhStdout != 0 )
         {
            dup2( hPipeOut[ 1 ], 1 );
         }
         else if( bBackground )
         {
            dup2( hNull, 1 );
         }

         if( fhStderr != 0 )
         {
            if( fhStderr != fhStdout )
            {
               dup2( hPipeErr[ 1 ], 2 );
            }
            else
            {
               dup2( hPipeOut[ 1 ], 2 );
            }
         }
         else if( bBackground )
         {
            dup2( hNull, 2 );
         }

         command        = ( char * ) hb_xgrab( strlen( pFilename ) + 2 );
         size           = s_parametrize( command, pFilename );
         argv           = s_argvize( command, size );
         argv[ size ]   = 0;

      #if defined( __BORLANDC__ ) || defined( __WATCOMC__ ) || defined( __GNUC__ )
         iFlags         = P_NOWAIT;
         pid            = spawnvp( iFlags, argv[ 0 ], argv );
      #else
         iFlags         = _P_NOWAIT;
         pid            = _spawnvp( iFlags, argv[ 0 ], argv );
      #endif

      #ifdef HB_OS_OS2
         *ProcessID  = ( ULONG ) pid;
      #else
         *ProcessID  = ( DWORD ) pid;
      #endif

         hb_xfree( command );
         hb_xfree( argv );

         dup2( oldstdin, 0 );
         dup2( oldstdout, 1 );
         dup2( oldstderr, 2 );
      }

      if( pid < 0 )
  #else
      if( ( pid = fork() ) == -1 )
  #endif
      {
         hb_fsSetIOError( FALSE, 0 );
         // closing unused handles should be nice
         // TODO: check fs_Popen to close handles.
         if( fhStdin != 0 )
         {
            close( hPipeIn[ 0 ] );
            close( hPipeIn[ 1 ] );
         }

         if( fhStdout != 0 )
         {
            close( hPipeOut[ 0 ] );
            close( hPipeOut[ 1 ] );
         }

         if( fhStderr != 0 && fhStderr != fhStdout )
         {
            close( hPipeErr[ 0 ] );
            close( hPipeErr[ 1 ] );
         }

         return ( HB_FHANDLE ) -1;
      }

      if( pid != 0 )
      {
         *ProcessID = ( ULONG ) pid;

         // I am the father
         if( fhStdin != NULL )
         {
            *fhStdin = hPipeIn[ 1 ];
            close( hPipeIn[ 0 ] );
         }

         if( fhStdout != NULL )
         {
            *fhStdout = hPipeOut[ 0 ];
            close( hPipeOut[ 1 ] );
         }

         if( fhStderr != NULL && fhStderr != fhStdout )
         {
            *fhStderr = hPipeErr[ 0 ];
            close( hPipeErr[ 1 ] );
         }

         // father is done.
         hb_fsSetError( 0 );
         hRet = ( HB_FHANDLE ) pid;

      }

      // I am che child
 #ifndef HB_OS_WIN
      else
      {
         command        = ( char * ) hb_xgrab( strlen( pFilename ) + 2 );
         size           = s_parametrize( command, pFilename );
         argv           = s_argvize( command, size );
         argv[ size ]   = NULL;

/*
      // temporary solution
      char *argv[4];
      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = ( char * ) pFilename;
      argv[3] = ( char * ) 0; */
         // drop uncontrolled streams

         /* Initialize hNull to make compiler happy ;-) */
         hNull = bBackground ? open( "/dev/null", O_RDWR ) : FS_ERROR;

         // does father wants to control us?
         if( fhStdin != NULL )
         {
            close( hPipeIn[ 1 ] ); // we don't write to stdin
            dup2( hPipeIn[ 0 ], 0 );
         }
         else if( bBackground )
         {
            dup2( hNull, 0 );
         }

         if( fhStdout != NULL )
         {
            close( hPipeOut[ 0 ] );
            dup2( hPipeOut[ 1 ], 1 );
         }
         else if( bBackground )
         {
            dup2( hNull, 1 );
         }

         if( fhStderr != NULL )
         {
            if( fhStdout != fhStderr )
            {
               close( hPipeErr[ 0 ] );
               dup2( hPipeErr[ 1 ], 2 );
            }
            else
            {
               dup2( 1, 2 );
            }
         }
         else if( bBackground )
         {
            dup2( hNull, 2 );
         }

         if( bBackground )
         {
            close( hNull );
         }

         /*
            for( hNull = 3; hNull < MAXFD; ++hNull )
            close(hNull);
          */

         // ????
         /*
          * This disable SUID and SGID, I added it for security to hb_fsPOpen
          * Just simply program can work using seteuid()/setegid() to access
          * database file what can cause that users cannot access database
          * file directly - only from program. When you run external program
          * with setuid(geteuid())/setgid(getegid()) then it inherits UID and
          * GID so is able to operate with their privileges. If this external
          * program is called without absolute path (beginning from "/") then
          * user can set his own PATH variable or change directory before
          * running xHarbour binaries to take control over EUID/EGID resources
          * Take a decision is it's important - maybe it should be set
          * by a parameter? Druzus.
          */
         /*
            setuid(getuid());
            setgid(getgid());*/

         execvp( argv[ 0 ], argv );
      }
   #endif
   }
#elif defined( HB_WIN32_IO )
   {
      STARTUPINFO          si;
      PROCESS_INFORMATION  proc;
      ULONG                ulSize;
      // int iSize;
      // DWORD iRet;
      DWORD                iFlags = 0;
      char                 fullCommand[ 1024 ], cmdName[ 256 ];
      char *               completeCommand, * pos;
      char *               filePart;
      SECURITY_ATTRIBUTES  secatt;

      HANDLE               hPipeInRd   = INVALID_HANDLE_VALUE, hPipeInWr = INVALID_HANDLE_VALUE;
      HANDLE               hPipeOutRd  = INVALID_HANDLE_VALUE, hPipeOutWr = INVALID_HANDLE_VALUE;
      HANDLE               hPipeErrRd  = INVALID_HANDLE_VALUE, hPipeErrWr = INVALID_HANDLE_VALUE;

      // prepare security attributes
      secatt.nLength                = sizeof( secatt );
      secatt.lpSecurityDescriptor   = NULL;
      secatt.bInheritHandle         = TRUE;

      hb_fsSetError( 0 ); // reset error

      if( fhStdin != NULL )
      {
         if( ! CreatePipe( &hPipeInRd, &hPipeInWr, &secatt, 0 ) )
         {
            hb_fsSetIOError( FALSE, 0 );
            return hRet;
         }
      }

      if( fhStdout != NULL )
      {
         if( ! CreatePipe( &hPipeOutRd, &hPipeOutWr, &secatt, 0 ) )
         {
            hb_fsSetIOError( FALSE, 0 );
            goto ret_close_1;
         }
      }

      if( fhStderr != NULL )
      {
         if( fhStderr == fhStdout )
         {
            hPipeErrRd  = hPipeOutRd;
            hPipeErrWr  = hPipeOutWr;
         }

         if( ! CreatePipe( &hPipeErrRd, &hPipeErrWr, &secatt, 0 ) )
         {
            hb_fsSetIOError( FALSE, 0 );
            goto ret_close_2;
         }
      }

      // parameters are included in the command string
      pos = ( char * ) pFilename;
      while( *pos && *pos != ' ' && *pos != '\\' )
      {
         pos++;
      }

      ulSize = ( unsigned ) ( pos - ( char * ) pFilename );
      if( ulSize > 254 || *pos == '\\' )
      {
         // absolute path. We are ok
         hb_strncpy( fullCommand, pFilename, 1023 );
         fullCommand[ 1023 ] = '\0';
      }
      else
      {
         HB_MEMCPY( cmdName, pFilename, ulSize );
         cmdName[ ulSize + 1 ] = 0;
         // find the command in the path
         if( ! SearchPath( NULL, cmdName, NULL, 1024, fullCommand, &filePart ) )
         {
            hb_xstrcpy( fullCommand, cmdName, 0 );
         }

      }

      if( *pos && *pos != '\\' )
      {
         HB_SIZE uiLen = strlen( fullCommand ) + strlen( pos ) + 2;
         completeCommand = ( char * ) hb_xgrab( uiLen );
         hb_snprintf( completeCommand, ( size_t ) uiLen, "%s %s", fullCommand, pos + 1 );
      }
      else
      {
         completeCommand = ( char * ) hb_xgrab( strlen( fullCommand ) + 1 );
         hb_xstrcpy( completeCommand, fullCommand, 0 );
      }

      memset( &si, 0, sizeof( si ) );
      si.cb = sizeof( si );

      if( bBackground )
      {
         // using show_hide AND using invalid handlers for unused streams
         si.dwFlags     = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
         si.wShowWindow = SW_HIDE;

         si.hStdInput   = hPipeInRd;
         si.hStdOutput  = hPipeOutWr;
         si.hStdError   = hPipeErrWr;

         iFlags         |= DETACHED_PROCESS;
      }
      else
      {
         si.dwFlags = STARTF_USESTDHANDLES;

         if( fhStdin != NULL )
         {
            si.hStdInput = hPipeInRd;
         }
         else
         {
            si.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
         }

         if( fhStdout != NULL )
         {
            si.hStdOutput = hPipeOutWr;
         }
         else
         {
            si.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
         }

         if( fhStderr != NULL )
         {
            si.hStdError = hPipeErrWr;
         }
         else
         {
            si.hStdError = GetStdHandle( STD_ERROR_HANDLE );
         }
      }

      if( ! CreateProcess( NULL,
                           completeCommand,
                           NULL,
                           NULL,
                           TRUE, //Inerhit handles!
                           iFlags,
                           NULL,
                           NULL,
                           &si,
                           &proc
                           ) )
      {
         hb_fsSetIOError( FALSE, 0 );
         hb_xfree( completeCommand );
         goto ret_close_3;
      }
      else
      {
         hb_xfree( completeCommand );
         hb_fsSetError( 0 );
         hRet        = ( HB_FHANDLE ) proc.hProcess;
         *ProcessID  = proc.dwProcessId;

         if( fhStdin != NULL )
         {
            *fhStdin = ( HB_FHANDLE ) hPipeInWr;
            CloseHandle( hPipeInRd );
         }
         if( fhStdout != NULL )
         {
            *fhStdout = ( HB_FHANDLE ) hPipeOutRd;
            CloseHandle( hPipeOutWr );
         }
         if( fhStderr != NULL )
         {
            *fhStderr = ( HB_FHANDLE ) hPipeErrRd;
            CloseHandle( hPipeErrWr );
         }

         CloseHandle( proc.hThread ); // unused
      }

      return hRet;

 ret_close_3:
      if( hPipeErrRd != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeErrRd );
      }
      if( hPipeErrWr != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeErrWr );
      }

 ret_close_2:
      if( hPipeOutRd != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeOutRd );
      }
      if( hPipeOutWr != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeOutWr );
      }

 ret_close_1:
      if( hPipeInRd != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeInRd );
      }
      if( hPipeInWr != INVALID_HANDLE_VALUE )
      {
         CloseHandle( hPipeInWr );
      }
   }

#else

   HB_SYMBOL_UNUSED( pFilename );
   HB_SYMBOL_UNUSED( fhStdin );
   HB_SYMBOL_UNUSED( fhStdout );
   HB_SYMBOL_UNUSED( fhStderr );
   HB_SYMBOL_UNUSED( bBackground );
   HB_SYMBOL_UNUSED( ProcessID );

   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return hRet;
}

/*
   See if a process is still being executed. If bWait is true,
   the function will wait for the process to finish  before
   to return. When the process is terminated
   with a signal, the signal is returned as -signum. Notice that
   this does not apply to Windows.
 */

int hb_fsProcessValue( HB_FHANDLE fhProc, BOOL bWait )
{
   HB_THREAD_STUB
   int iRetStatus;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsProcessValue(%d, %d )", fhProc, bWait ) );

   hb_fsSetError( 0 );

#if defined( HB_OS_UNIX ) || defined( HB_OS_OS2 )
   {
      int iStatus;

      if( fhProc > 0 )
      {
         if( ! bWait )
         {
            iRetStatus = waitpid( ( pid_t ) fhProc, &iStatus, WNOHANG );
         }
         else
         {
            hb_vmUnlock();
            iRetStatus = waitpid( ( pid_t ) fhProc, &iStatus, 0 );
            hb_vmLock();
         }
      }
      else
      {
         iRetStatus = 0;
      }

#ifdef ERESTARTSYS
      if( iRetStatus < 0 && errno != ERESTARTSYS )
#else
      if( iRetStatus < 0 )
#endif
      {
         hb_fsSetIOError( FALSE, 0 );
         iRetStatus = -2;
      }
      else if( iRetStatus == 0 )
      {
         iRetStatus = -1;
      }
      else
      {
         if( WIFEXITED( iStatus ) )
         {
            iRetStatus = WEXITSTATUS( iStatus );
         }
         else
         {
            iRetStatus = 0;
         }
      }
   }
#elif defined( HB_OS_WIN ) && ! defined( HB_WIN32_IO )
   {
      int iPid;

      HB_SYMBOL_UNUSED( bWait );

      hb_vmUnlock();
   #ifdef __BORLANDC__
      iPid  = cwait( &iRetStatus, ( int ) fhProc, 0 );
   #else
      iPid  = _cwait( &iRetStatus, ( int ) fhProc, 0 );
   #endif
      hb_vmLock();

      if( iPid != ( int ) fhProc )
      {
         iRetStatus = -1;
      }
   }
#elif defined( HB_WIN32_IO )
   {
      DWORD dwTime;
      DWORD dwResult;

      if( ! bWait )
      {
         dwTime = 0;
      }
      else
      {
         dwTime = INFINITE;
      }

      hb_vmUnlock();
      dwResult = WaitForSingleObject( DosToWinHandle( fhProc ), dwTime );
      hb_vmLock();

      if( dwResult == WAIT_OBJECT_0 )
      {
         BOOL fResult = GetExitCodeProcess( DosToWinHandle( fhProc ), &dwResult );

         hb_fsSetIOError( fResult, 0 );
         iRetStatus = fResult ? ( int ) dwResult : -2;
      }
      else
      {
         iRetStatus = -1;
      }
   }
#else

   HB_SYMBOL_UNUSED( fhProc );
   HB_SYMBOL_UNUSED( bWait );

   hb_fsSetError( ( USHORT ) FS_ERROR );
   iRetStatus = -1;
#endif

   return iRetStatus;
}

/*
   Closes a process (that is, kill the application running the
   process); the handle is still valid until you
   catch it with hb_fsProcessValue. If bGentle is true, then
   a kind termination request is sent to the process, else
   the process is just killed.
   Retiurn
 */

BOOL hb_fsCloseProcess( HB_FHANDLE fhProc, BOOL bGentle )
{
   BOOL bRet;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCloseProcess(%d, %d )", fhProc, bGentle ) );

#if defined( HB_OS_UNIX ) || defined( HB_OS_OS2 )
   if( fhProc > 0 )
   {
      int iSignal = bGentle ? SIGTERM : SIGKILL;
      bRet = ( kill( ( pid_t ) fhProc, iSignal ) == 0 );
      hb_fsSetIOError( bRet, 0 );
   }
   else
   {
      bRet = FALSE;
      hb_fsSetError( ( USHORT ) FS_ERROR );
   }
#elif defined( HB_WIN32_IO )
   bRet = ( TerminateProcess( DosToWinHandle( fhProc ), bGentle ? 0 : 1 ) != 0 );
   hb_fsSetIOError( bRet, 0 );
#elif defined( HB_OS_WIN )
   {
      HANDLE hProc;

      hProc = OpenProcess( PROCESS_TERMINATE, FALSE, fhProc );

      if( hProc != NULL )
      {
         bRet = ( TerminateProcess( hProc, bGentle ? 0 : 1 ) != 0 );
      }
      else
      {
         bRet = FALSE;
      }
      hb_fsSetIOError( bRet, 0 );
   }
#else

   HB_SYMBOL_UNUSED( fhProc );
   HB_SYMBOL_UNUSED( bGentle );
   bRet = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif
   return bRet;
}


HB_FHANDLE hb_fsOpen( const char * pFilename, USHORT uiFlags )
{
   HB_THREAD_STUB

   HB_FHANDLE  hFileHandle;
   char *      pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsOpen(%p, %hu)", pFilename, uiFlags ) );

   pFilename = hb_fsNameConv( pFilename, &pszFree );

   // Unlocking stack to allow cancelation points
   hb_vmUnlock();

#if defined( HB_WIN32_IO )
   {
      DWORD    dwMode, dwShare, dwCreat, dwAttr;
      HANDLE   hFile;

      convert_open_flags( FALSE, FC_NORMAL, uiFlags, &dwMode, &dwShare, &dwCreat, &dwAttr );

      hFile = ( HANDLE ) CreateFileA( ( char * ) pFilename, dwMode, dwShare,
                                      NULL, dwCreat, dwAttr, NULL );
      hb_fsSetIOError( hFile != ( HANDLE ) INVALID_HANDLE_VALUE, 0 );

      hFileHandle = ( HB_FHANDLE ) hFile;
   }

#elif defined( HB_OS2_IO )

   {
      ULONG    ulAttribute, fsOpenFlags, fsOpenMode;
      APIRET   rc;
      ULONG    ulAction;
      HFILE    hFile = FS_ERROR;

      /* On OS/2 it has only 6 parameters */
      convert_open_flags( FALSE, FC_NORMAL, uiFlags, &ulAttribute, &fsOpenFlags, &fsOpenMode );

      rc = DosOpen( ( PSZ ) pFilename,
                    &hFile,
                    &ulAction,
                    0L,
                    ulAttribute,
                    fsOpenFlags,
                    fsOpenMode,
                    0L );

      /* On OS/2 errors 0..99 have the same meaning they had on DOS */
      hb_fsSetError( rc );

      if( rc == NO_ERROR )
      {
         hFileHandle = _imphandle( hFile );
         // Defaults to O_TEXT inside LIBC
         setmode( hFileHandle, O_BINARY );
      }
      else
      {
         hFileHandle = FS_ERROR;
      }
   }
#elif defined( HB_FS_FILE_IO )
   {
      int      flags, share, attr;
      unsigned mode;

      convert_open_flags( FALSE, FC_NORMAL, uiFlags, &flags, &mode, &share, &attr );
#if defined( _MSC_VER ) || defined( __DMC__ )
      if( share )
         hFileHandle = _sopen( pFilename, flags, share, mode );
      else
         hFileHandle = _open( pFilename, flags, mode );
#elif defined( HB_FS_SOPEN )
      if( share )
         hFileHandle = sopen( pFilename, flags, share, mode );
      else
         hFileHandle = open( pFilename, flags, mode );
#else
      hFileHandle = open( pFilename, flags | share, mode );
#endif
      hb_fsSetIOError( hFileHandle != FS_ERROR, 0 );
   }
#else

   hFileHandle = FS_ERROR;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   if( pszFree )
      hb_xfree( pszFree );

   return hFileHandle;
}

HB_FHANDLE hb_fsCreate( const char * pFilename, ULONG ulAttr )
{
   HB_THREAD_STUB
   HB_FHANDLE  hFileHandle;
   char *      pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCreate(%p, %lu)", pFilename, ulAttr ) );

   pFilename = hb_fsNameConv( pFilename, &pszFree );

   hb_vmUnlock();

#if defined( HB_WIN32_IO )
   {
      DWORD    dwMode, dwShare, dwCreat, dwAttr;
      HANDLE   hFile;

      convert_open_flags( TRUE, ulAttr, FO_EXCLUSIVE, &dwMode, &dwShare, &dwCreat, &dwAttr );

      hFile = ( HANDLE ) CreateFileA( ( LPCTSTR ) pFilename, dwMode, dwShare,
                                      NULL, dwCreat, dwAttr, NULL );
      hb_fsSetIOError( hFile != ( HANDLE ) INVALID_HANDLE_VALUE, 0 );

      hFileHandle = ( HB_FHANDLE ) hFile;
   }

#elif defined( HB_OS2_IO )

   {
      ULONG    ulAttribute, fsOpenFlags, fsOpenMode;
      APIRET   rc;
      ULONG    ulAction;
      HFILE    hFile = FS_ERROR;

      /* On OS/2 it has only 6 parameters */
      convert_open_flags( TRUE, ulAttr, FO_EXCLUSIVE, &ulAttribute, &fsOpenFlags, &fsOpenMode );

      rc = DosOpen( ( PSZ ) pFilename,
                    &hFile,
                    &ulAction,
                    0L,
                    ulAttribute,
                    fsOpenFlags,
                    fsOpenMode,
                    0L );

      /* On OS/2 errors 0..99 have the same meaning they had on DOS */
      hb_fsSetError( rc );

      if( rc == NO_ERROR )
      {
         hFileHandle = _imphandle( hFile );
         // Defaults to O_TEXT inside LIBC
         setmode( hFileHandle, O_BINARY );
      }
      else
      {
         hFileHandle = FS_ERROR;
      }
   }

#elif defined( HB_FS_FILE_IO )
   {
      int      flags, share, attr;
      unsigned mode;
      convert_open_flags( TRUE, ulAttr, FO_EXCLUSIVE, &flags, &mode, &share, &attr );

#if defined( HB_FS_DOSCREAT )
      hFileHandle = _creat( pFilename, attr );
#elif defined( HB_FS_SOPEN )
      hFileHandle = open( pFilename, flags, mode );
#else
      hFileHandle = open( pFilename, flags | share, mode );
#endif
      hb_fsSetIOError( hFileHandle != FS_ERROR, 0 );
   }
#else

   hFileHandle = FS_ERROR;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   if( pszFree )
      hb_xfree( pszFree );

   return hFileHandle;
}

/* Derived from hb_fsCreate()

   NOTE: The default opening mode differs from the one used in hb_fsCreate()
         [vszakats]
 */

HB_FHANDLE hb_fsCreateEx( const char * pFilename, ULONG ulAttr, USHORT uiFlags )
{
   HB_THREAD_STUB
   HB_FHANDLE  hFileHandle;
   char *      pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCreateEx(%p, %lu, %hu)", pFilename, ulAttr, uiFlags ) );

   pFilename = hb_fsNameConv( pFilename, &pszFree );

   hb_vmUnlock();

#if defined( HB_WIN32_IO )
   {
      DWORD    dwMode, dwShare, dwCreat, dwAttr;
      HANDLE   hFile;

      convert_open_flags( TRUE, ulAttr, uiFlags, &dwMode, &dwShare, &dwCreat, &dwAttr );

      hFile = ( HANDLE ) CreateFileA( ( LPCTSTR ) pFilename, dwMode, dwShare,
                                      NULL, dwCreat, dwAttr, NULL );
      hb_fsSetIOError( hFile != ( HANDLE ) INVALID_HANDLE_VALUE, 0 );

      hFileHandle = ( HB_FHANDLE ) hFile;
   }

#elif defined( HB_OS2_IO )

   {
      ULONG    ulAttribute, fsOpenFlags, fsOpenMode;
      APIRET   rc;
      ULONG    ulAction;
      HFILE    hFile = FS_ERROR;

      /* On OS/2 it has only 6 parameters */
      convert_open_flags( TRUE, ulAttr, uiFlags, &ulAttribute, &fsOpenFlags, &fsOpenMode );

      rc = DosOpen( ( PSZ ) pFilename,
                    &hFile,
                    &ulAction,
                    0L,
                    ulAttribute,
                    fsOpenFlags,
                    fsOpenMode,
                    0L );

      /* On OS/2 errors 0..99 have the same meaning they had on DOS */
      hb_fsSetError( rc );

      if( rc == NO_ERROR )
      {
         hFileHandle = _imphandle( hFile );
         // Defaults to O_TEXT inside LIBC
         setmode( hFileHandle, O_BINARY );
      }
      else
      {
         hFileHandle = FS_ERROR;
      }
   }
#elif defined( HB_FS_FILE_IO )
   {
      int      flags, share, attr;
      unsigned mode;
      convert_open_flags( TRUE, ulAttr, uiFlags, &flags, &mode, &share, &attr );

#if defined( HB_FS_SOPEN )
      hFileHandle = open( pFilename, flags, mode );
#else
      hFileHandle = open( pFilename, flags | share, mode );
#endif
      hb_fsSetIOError( hFileHandle != FS_ERROR, 0 );
   }
#else

   hFileHandle = FS_ERROR;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   if( pszFree )
      hb_xfree( pszFree );

   return hFileHandle;
}

void hb_fsClose( HB_FHANDLE hFileHandle )
{
   HB_THREAD_STUB
   HB_TRACE( HB_TR_DEBUG, ( "hb_fsClose(%p)", ( void * ) ( HB_PTRDIFF ) hFileHandle ) );

#if defined( HB_FS_FILE_IO )

   hb_vmUnlock();

   #if defined( HB_WIN32_IO )
   hb_fsSetIOError( CloseHandle( DosToWinHandle( hFileHandle ) ), 0 );
   #else
   hb_fsSetIOError( close( hFileHandle ) == 0, 0 );
   #endif
   hb_vmLock();

#else

   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif
}

BOOL hb_fsSetDevMode( HB_FHANDLE hFileHandle, USHORT uiDevMode )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_fsSetDevMode(%p, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, uiDevMode ) );

   /* TODO: HB_WIN32_IO support */

#if defined( __BORLANDC__ ) || defined( __IBMCPP__ ) || defined( __DJGPP__ ) || \
   defined( __CYGWIN__ ) || defined( __WATCOMC__ ) || defined( HB_OS_OS2 )
   {
      int iRet = 0;

#if defined( HB_WIN32_IO )
      if( hFileHandle != ( HB_FHANDLE ) 0 &&
          hFileHandle != ( HB_FHANDLE ) 1 &&
          hFileHandle != ( HB_FHANDLE ) 2 )
         iRet = -1;
      else
#endif
      switch( uiDevMode )
      {
         case FD_BINARY:
            iRet = setmode( ( HB_NHANDLE ) hFileHandle, O_BINARY );
            break;

         case FD_TEXT:
            iRet = setmode( ( HB_NHANDLE ) hFileHandle, O_TEXT );
            break;
      }

      hb_fsSetIOError( iRet != -1, 0 );

      return iRet != -1;
   }
#elif ( defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __DMC__ ) ) && \
   ! defined( HB_OS_WIN_CE )
   {
      int iRet = 0;

#if defined( HB_WIN32_IO )
      if( hFileHandle != ( HB_FHANDLE ) 0 &&
          hFileHandle != ( HB_FHANDLE ) 1 &&
          hFileHandle != ( HB_FHANDLE ) 2 )
         iRet = -1;
      else
#endif
      switch( uiDevMode )
      {
         case FD_BINARY:
            iRet = _setmode( ( int ) ( HB_NHANDLE ) hFileHandle, _O_BINARY );
            break;

         case FD_TEXT:
            iRet = _setmode( ( int ) ( HB_NHANDLE ) hFileHandle, _O_TEXT );
            break;
      }

      hb_fsSetIOError( iRet != -1, 0 );

      return iRet != -1;
   }
#elif defined( HB_OS_UNIX ) || defined( HB_OS_WIN_CE )

   HB_SYMBOL_UNUSED( hFileHandle );

   if( uiDevMode == FD_TEXT )
   {
      hb_fsSetError( ( USHORT ) FS_ERROR );
      return FALSE;
   }

   hb_fsSetError( 0 );
   return TRUE;

#else

   HB_SYMBOL_UNUSED( hFileHandle );
   HB_SYMBOL_UNUSED( uiDevMode );
   hb_fsSetError( ( USHORT ) FS_ERROR );
   return FALSE;

#endif
}

BOOL hb_fsGetFileTime( const char * pszFileName, LONG * plJulian, LONG * plMillisec )
{
   HB_THREAD_STUB
   BOOL fResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsGetFileTime(%s, %p, %p)", pszFileName, plJulian, plMillisec ) );

   fResult = FALSE;

#if defined( HB_WIN32_IO )
   {
      HB_FHANDLE hFile = hb_fsOpen( pszFileName, FO_READ | FO_SHARED );

      if( hFile != FS_ERROR )
      {
         FILETIME    ft, local_ft;
         SYSTEMTIME  st;

         hb_vmUnlock();
         if( GetFileTime( DosToWinHandle( hFile ), NULL, NULL, &ft ) &&
             FileTimeToLocalFileTime( &ft, &local_ft ) &&
             FileTimeToSystemTime( &local_ft, &st ) )
         {
            *plJulian   = hb_dateEncode( st.wYear, st.wMonth, st.wDay );
            *plMillisec = hb_timeStampEncode( st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );

            fResult     = TRUE;
         }
         hb_fsSetIOError( fResult, 0 );
         hb_vmLock();
         hb_fsClose( hFile );
      }
   }
#elif defined( HB_OS_UNIX ) || defined( HB_OS_OS2 ) || defined( HB_OS_DOS ) || defined( __GNUC__ )
   {
      struct stat sStat;
      char *      pszFree;

      pszFileName = hb_fsNameConv( pszFileName, &pszFree );

      hb_vmUnlock();
      if( stat( pszFileName, &sStat ) == 0 )
      {
         time_t      ftime;
         struct tm   ft;

         ftime = sStat.st_mtime;
#   if _POSIX_C_SOURCE < 199506L || defined( HB_OS_DARWIN_5 )
         ft    = *localtime( &ftime );
#   else
         localtime_r( &ftime, &ft );
#   endif

         *plJulian   = hb_dateEncode( ft.tm_year + 1900, ft.tm_mon + 1, ft.tm_mday );
         *plMillisec = hb_timeStampEncode( ft.tm_hour, ft.tm_min, ft.tm_sec, 0 );

         fResult     = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();

      if( pszFree )
         hb_xfree( pszFree );
   }
#else
   {
      int TODO; /* TODO: for given platform */

      HB_SYMBOL_UNUSED( pszFileName );
      HB_SYMBOL_UNUSED( plJulian );
      HB_SYMBOL_UNUSED( plMillisec );
   }
#endif

   return fResult;
}

BOOL hb_fsGetAttr( const char * pszFileName, ULONG * pulAttr )
{
   BOOL     fResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsGetAttr(%s, %p)", pszFileName, pulAttr ) );

   fResult     = FALSE;
   pszFileName = hb_fsNameConv( pszFileName, &pszFree );

#if defined( HB_OS_WIN )
   {
      HB_THREAD_STUB
      DWORD dwAttr;

      hb_vmUnlock();
      dwAttr = GetFileAttributesA( pszFileName );

      if( dwAttr != INVALID_FILE_ATTRIBUTES )
      {
         *pulAttr = hb_fsAttrFromRaw( dwAttr );
         fResult  = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#elif defined( HB_OS_DOS )
   hb_vmUnlock();
   {
#if defined( __DJGPP__ ) || defined( __BORLANDC__ )
      int            attr  = _chmod( pszFileName, 0, 0 );
      if( attr != -1 )
#else
      unsigned int   attr  = 0;
      if( _dos_getfileattr( pszFileName, &attr ) == 0 )
#endif
      {
         *pulAttr = hb_fsAttrFromRaw( attr );
         fResult  = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
   }
   hb_vmLock();
#elif defined( HB_OS_OS2 )
   {
      FILESTATUS3 fs3;
      APIRET      ulrc;

      hb_vmUnlock();
      ulrc = DosQueryPathInfo( ( PCSZ ) pszFileName, FIL_STANDARD, &fs3, sizeof( fs3 ) );
      if( ulrc == NO_ERROR )
      {
         *pulAttr = hb_fsAttrFromRaw( fs3.attrFile );
         fResult  = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#elif defined( HB_OS_UNIX )
   {
      struct stat sStat;

      hb_vmUnlock();
      if( stat( pszFileName, &sStat ) == 0 )
      {
         *pulAttr = hb_fsAttrFromRaw( sStat.st_mode );
         fResult  = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#else
   {
      int TODO; /* TODO: for given platform */

      HB_SYMBOL_UNUSED( pszFileName );
      HB_SYMBOL_UNUSED( pulAttr );
   }
#endif

   if( pszFree )
      hb_xfree( pszFree );

   return fResult;
}

BOOL hb_fsSetFileTime( const char * pszFileName, LONG lJulian, LONG lMillisec )
{
   BOOL  fResult;
   int   iYear, iMonth, iDay;
   int   iHour, iMinute, iSecond, iMSec;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsSetFileTime(%s, %ld, %ld)", pszFileName, lJulian, lMillisec ) );

   hb_dateDecode( lJulian, &iYear, &iMonth, &iDay );
   hb_timeStampDecode( lMillisec, &iHour, &iMinute, &iSecond, &iMSec );

#if defined( HB_OS_WIN ) && ! defined( __CYGWIN__ )
   {
      HB_THREAD_STUB
      HB_FHANDLE hFile = hb_fsOpen( pszFileName, FO_READWRITE | FO_SHARED );

      fResult = hFile != FS_ERROR;
      if( fResult )
      {
         FILETIME    ft, local_ft;
         SYSTEMTIME  st;

         hb_vmUnlock();
         if( lJulian <= 0 || lMillisec < 0 )
            GetLocalTime( &st );
         else
            memset( &st, 0, sizeof( st ) );

         if( lJulian > 0 )
         {
            st.wYear    = ( WORD ) iYear;
            st.wMonth   = ( WORD ) iMonth;
            st.wDay     = ( WORD ) iDay;
         }
         if( lMillisec >= 0 )
         {
            st.wHour          = ( WORD ) iHour;
            st.wMinute        = ( WORD ) iMinute;
            st.wSecond        = ( WORD ) iSecond;
            st.wMilliseconds  = ( WORD ) iMSec;
         }
         SystemTimeToFileTime( &st, &local_ft );
         LocalFileTimeToFileTime( &local_ft, &ft );
         fResult = SetFileTime( DosToWinHandle( hFile ), NULL, &ft, &ft ) != 0;
         hb_fsSetIOError( fResult, 0 );
         hb_vmLock();
         hb_fsClose( hFile );
      }
   }
#elif defined( HB_OS_OS2 )
   {
      FILESTATUS3 fs3;
      APIRET      ulrc;
      char *      pszFree;

      pszFileName = hb_fsNameConv( pszFileName, &pszFree );

      hb_vmUnlock();
      ulrc        = DosQueryPathInfo( ( PCSZ ) pszFileName, FIL_STANDARD, &fs3, sizeof( fs3 ) );
      if( ulrc == NO_ERROR )
      {
         FDATE fdate;
         FTIME ftime;

         if( lJulian <= 0 || lMillisec < 0 )
         {
            DATETIME dt;

            DosGetDateTime( &dt );

            fdate.year     = dt.year - 1980;
            fdate.month    = dt.month;
            fdate.day      = dt.day;
            ftime.hours    = dt.hours;
            ftime.minutes  = dt.minutes;
            ftime.twosecs  = dt.seconds / 2;
         }
         if( lJulian > 0 )
         {
            fdate.year  = iYear - 1980;
            fdate.month = iMonth;
            fdate.day   = iDay;
         }
         if( lMillisec >= 0 )
         {
            ftime.hours    = iHour;
            ftime.minutes  = iMinute;
            ftime.twosecs  = iSecond / 2;
         }

         fs3.fdateCreation = fs3.fdateLastAccess = fs3.fdateLastWrite = fdate;
         fs3.ftimeCreation = fs3.ftimeLastAccess = fs3.ftimeLastWrite = ftime;
         ulrc              = DosSetPathInfo( ( PCSZ ) pszFileName, FIL_STANDARD,
                                             &fs3, sizeof( fs3 ), DSPI_WRTTHRU );
      }
      fResult = ulrc == NO_ERROR;
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
      if( pszFree )
         hb_xfree( pszFree );
   }
#elif defined( HB_OS_UNIX ) || defined( HB_OS_DOS )
   {
      char * pszFree;

      pszFileName = hb_fsNameConv( pszFileName, &pszFree );

      hb_vmUnlock();
      if( lJulian <= 0 && lMillisec )
      {
         fResult = utime( pszFileName, NULL ) == 0;
      }
      else
      {
         struct utimbuf buf;
         struct tm      new_value;
         time_t         tim;

         if( lJulian <= 0 || lMillisec < 0 )
         {
            time_t current_time;

            current_time   = time( NULL );
#   if _POSIX_C_SOURCE < 199506L || defined( HB_OS_DARWIN_5 )
            new_value      = *localtime( &current_time );
#   else
            localtime_r( &current_time, &new_value );
#   endif
         }
         else
            memset( &new_value, 0, sizeof( new_value ) );

         if( lJulian > 0 )
         {
            new_value.tm_year = iYear - 1900;
            new_value.tm_mon  = iMonth - 1;
            new_value.tm_mday = iDay;
         }
         if( lMillisec >= 0 )
         {
            new_value.tm_hour = iHour;
            new_value.tm_min  = iMinute;
            new_value.tm_sec  = iSecond;
         }
         tim         = mktime( &new_value );
#   if _POSIX_C_SOURCE < 199506L || defined( HB_OS_DARWIN_5 )
         new_value   = *gmtime( &tim );
#   else
         gmtime_r( &tim, &new_value );
#   endif
         buf.actime  = buf.modtime = mktime( &new_value );
         fResult     = utime( ( char * ) pszFileName, &buf ) == 0;
      }
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
      if( pszFree )
         hb_xfree( pszFree );
   }
#else
   {
      int TODO; /* To force warning */

      fResult = FALSE;
      hb_fsSetError( ( USHORT ) FS_ERROR );
   }
#endif

   return fResult;
}

BOOL hb_fsSetAttr( const char * pszFileName, ULONG ulAttr )
{
   BOOL     fResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsSetAttr(%s, %lu)", pszFileName, ulAttr ) );

   pszFileName = hb_fsNameConv( pszFileName, &pszFree );

#if defined( HB_OS_WIN )
   {
      HB_THREAD_STUB
      DWORD dwFlags = FILE_ATTRIBUTE_ARCHIVE;

      if( ulAttr & HB_FA_READONLY )
         dwFlags |= FILE_ATTRIBUTE_READONLY;
      if( ulAttr & HB_FA_HIDDEN )
         dwFlags |= FILE_ATTRIBUTE_HIDDEN;
      if( ulAttr & HB_FA_SYSTEM )
         dwFlags |= FILE_ATTRIBUTE_SYSTEM;
      if( ulAttr & HB_FA_NORMAL )
         dwFlags |= FILE_ATTRIBUTE_NORMAL;
      hb_vmUnlock();
      fResult = SetFileAttributes( pszFileName, dwFlags );
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#elif defined( HB_OS_OS2 )
   {
      FILESTATUS3 fs3;
      APIRET      ulrc;
      ULONG       ulOsAttr = FILE_NORMAL;

      if( ulAttr & HB_FA_READONLY )
         ulOsAttr |= FILE_READONLY;
      if( ulAttr & HB_FA_HIDDEN )
         ulOsAttr |= FILE_HIDDEN;
      if( ulAttr & HB_FA_SYSTEM )
         ulOsAttr |= FILE_SYSTEM;
      if( ulAttr & HB_FA_ARCHIVE )
         ulOsAttr |= FILE_ARCHIVED;

      hb_vmUnlock();
      ulrc = DosQueryPathInfo( ( PCSZ ) pszFileName, FIL_STANDARD, &fs3, sizeof( fs3 ) );
      if( ulrc == NO_ERROR )
      {
         fs3.attrFile   = ulOsAttr;
         ulrc           = DosSetPathInfo( ( PCSZ ) pszFileName, FIL_STANDARD,
                                          &fs3, sizeof( fs3 ), DSPI_WRTTHRU );
      }
      fResult = ulrc == NO_ERROR;
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#elif defined( HB_OS_DOS )

   ulAttr   &= ~( HB_FA_ARCHIVE | HB_FA_HIDDEN | HB_FA_READONLY | HB_FA_SYSTEM );
   hb_vmUnlock();
#  if defined( __DJGPP__ ) || defined( __BORLANDC__ )
   fResult  = _chmod( pszFileName, 1, ulAttr ) != -1;
#  else
   fResult  = _dos_setfileattr( pszFileName, ulAttr ) != -1;
#  endif
   hb_fsSetIOError( fResult, 0 );
   hb_vmLock();

#elif defined( HB_OS_UNIX )
   {
      int iAttr = HB_FA_POSIX_ATTR( ulAttr );
      if( iAttr == 0 )
      {
         iAttr = ( ulAttr & HB_FA_HIDDEN ) ? S_IRUSR : ( S_IRUSR | S_IRGRP | S_IROTH );
         if( ! ( ulAttr & HB_FA_READONLY ) )
         {
            if( iAttr & S_IRUSR )
               iAttr |= S_IWUSR;
            if( iAttr & S_IRGRP )
               iAttr |= S_IWGRP;
            if( iAttr & S_IROTH )
               iAttr |= S_IWOTH;
         }
         if( ulAttr & HB_FA_SYSTEM )
         {
            if( iAttr & S_IRUSR )
               iAttr |= S_IXUSR;
            if( iAttr & S_IRGRP )
               iAttr |= S_IXGRP;
            if( iAttr & S_IROTH )
               iAttr |= S_IXOTH;
         }
      }
      hb_vmUnlock();
      fResult = chmod( pszFileName, iAttr ) != -1;
      hb_fsSetIOError( fResult, 0 );
      hb_vmLock();
   }
#else
   {
      int TODO; /* To force warning */

      fResult = FALSE;
      hb_fsSetError( ( USHORT ) FS_ERROR );
   }
#endif

   if( pszFree )
      hb_xfree( pszFree );

   return fResult;
}

USHORT hb_fsRead( HB_FHANDLE hFileHandle, void * pBuff, USHORT uiCount )
{
   HB_THREAD_STUB
   USHORT uiRead;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsRead(%p, %p, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, uiCount ) );

#if defined( HB_FS_FILE_IO )

   hb_vmUnlock();

   #if defined( HB_WIN32_IO )
   {
      DWORD dwRead;
      BOOL  fResult;

      // allowing async cancelation here
      fResult  = ReadFile( DosToWinHandle( hFileHandle ), pBuff, ( DWORD ) uiCount, &dwRead, NULL );
      hb_fsSetIOError( fResult, 0 );

      uiRead   = fResult ? ( USHORT ) dwRead : 0;
   }
   #else
   // allowing async cancelation here
   uiRead = read( hFileHandle, pBuff, uiCount );
   hb_fsSetIOError( uiRead != ( USHORT ) -1, 0 );
   #endif

   if( uiRead == ( USHORT ) -1 )
      uiRead = 0;

   hb_vmLock();
#else

   uiRead = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return uiRead;
}

USHORT hb_fsWrite( HB_FHANDLE hFileHandle, const void * pBuff, USHORT uiCount )
{
   HB_THREAD_STUB
   USHORT uiWritten;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsWrite(%p, %p, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, uiCount ) );

#if defined( HB_FS_FILE_IO )

   hb_vmUnlock();

   #if defined( HB_WIN32_IO )
   {
      DWORD dwWritten = 0;
      BOOL  fResult;

      if( uiCount )
      {
         fResult = WriteFile( DosToWinHandle( hFileHandle ), pBuff, uiCount, &dwWritten, NULL );
      }
      else
      {
         dwWritten   = 0;
         fResult     = SetEndOfFile( DosToWinHandle( hFileHandle ) );
      }
      hb_fsSetIOError( fResult, 0 );

      uiWritten = fResult ? ( USHORT ) dwWritten : 0;
   }
   #else

   if( uiCount )
   {
      uiWritten = write( hFileHandle, pBuff, uiCount );
      hb_fsSetIOError( uiWritten != ( USHORT ) -1, 0 );
      if( uiWritten == ( USHORT ) -1 )
         uiWritten = 0;
   }
   else
   {
#if defined( HB_USE_LARGEFILE64 )
      hb_fsSetIOError( ftruncate64( hFileHandle, lseek64( hFileHandle, 0L, SEEK_CUR ) ) != -1, 0 );
#else
      hb_fsSetIOError( ftruncate( hFileHandle, lseek( hFileHandle, 0L, SEEK_CUR ) ) != -1, 0 );
#endif
      uiWritten = 0;
   }

   #endif

   hb_vmLock();

#else

   uiWritten = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return uiWritten;
}

HB_SIZE hb_fsReadLarge( HB_FHANDLE hFileHandle, void * pBuff, HB_SIZE ulCount )
{
   HB_THREAD_STUB
   ULONG ulRead;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsReadLarge(%p, %p, %lu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, ulCount ) );

#if defined( HB_FS_FILE_IO )

   hb_vmUnlock();

   #if defined( HB_WIN32_IO )
   {
      hb_fsSetIOError( ReadFile( DosToWinHandle( hFileHandle ),
                                 pBuff, ( DWORD ) ulCount, &ulRead, NULL ), 0 );
   }
   #elif defined( HB_FS_LARGE_OPTIMIZED )
   {
      ulRead = read( hFileHandle, pBuff, ulCount );
      hb_fsSetIOError( ulRead != ( ULONG ) -1, 0 );
      if( ulRead == ( ULONG ) -1 )
         ulRead = 0;
   }
   #else
   {
      ULONG    ulLeftToRead   = ulCount;
      USHORT   uiToRead;
      USHORT   uiRead;
      void *   pPtr           = pBuff;

      ulRead = 0;

      while( ulLeftToRead )
      {
         /* Determine how much to read this time */
         if( ulLeftToRead > ( ULONG ) INT_MAX )
         {
            uiToRead       = INT_MAX;
            ulLeftToRead   -= ( ULONG ) uiToRead;
         }
         else
         {
            uiToRead       = ( USHORT ) ulLeftToRead;
            ulLeftToRead   = 0;
         }

         // allowing async cancelation here

         uiRead = read( hFileHandle, pPtr, uiToRead );
         /* -1 on bad hFileHandle
             0 on disk full
          */

         if( uiRead == 0 )
            break;

         if( uiRead == ( USHORT ) -1 )
         {
            uiRead = 0;
            break;
         }

         ulRead   += ( ULONG ) uiRead;
         pPtr     += uiRead;
      }
      hb_fsSetIOError( ulLeftToRead == 0, 0 );
   }
   #endif

   hb_vmLock();

#else

   ulRead = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return ulRead;
}

HB_SIZE hb_fsWriteLarge( HB_FHANDLE hFileHandle, const void * pBuff, HB_SIZE ulCount )
{
   HB_THREAD_STUB
   ULONG ulWritten;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsWriteLarge(%p, %p, %lu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, ulCount ) );

#if defined( HB_FS_FILE_IO )

   #if defined( HB_WIN32_IO )
   {
      ulWritten = 0;
      hb_vmUnlock();
      if( ulCount )
      {
         hb_fsSetIOError( WriteFile( DosToWinHandle( hFileHandle ), pBuff, ( DWORD ) ulCount, &ulWritten, NULL ), 0 );
      }
      else
      {
         hb_fsSetIOError( SetEndOfFile( DosToWinHandle( hFileHandle ) ), 0 );
      }
      hb_vmLock();
   }
   #else
   hb_vmUnlock();

   if( ulCount )
      #if defined( HB_FS_LARGE_OPTIMIZED )
   {
      ulWritten = write( hFileHandle, pBuff, ulCount );
      hb_fsSetIOError( ulWritten != ( ULONG ) -1, 0 );
      if( ulWritten == ( ULONG ) -1 )
         ulWritten = 0;
   }
      #else
   {
      ULONG          ulLeftToWrite  = ulCount;
      USHORT         uiToWrite;
      USHORT         uiWritten;
      const void *   pPtr           = pBuff;

      ulWritten = 0;

      while( ulLeftToWrite )
      {
         /* Determine how much to write this time */
         if( ulLeftToWrite > ( ULONG ) INT_MAX )
         {
            uiToWrite      = INT_MAX;
            ulLeftToWrite  -= ( ULONG ) uiToWrite;
         }
         else
         {
            uiToWrite      = ( USHORT ) ulLeftToWrite;
            ulLeftToWrite  = 0;
         }

         uiWritten = write( hFileHandle, pPtr, uiToWrite );

         /* -1 on bad hFileHandle
             0 on disk full
          */

         if( uiWritten == 0 )
            break;

         if( uiWritten == ( USHORT ) -1 )
         {
            uiWritten = 0;
            break;
         }

         ulWritten   += ( ULONG ) uiWritten;
         pPtr        += uiWritten;
      }
      hb_fsSetIOError( ulLeftToWrite == 0, 0 );
   }
      #endif
   else
   {
#if defined( HB_USE_LARGEFILE64 )
      hb_fsSetIOError( ftruncate64( hFileHandle, lseek64( hFileHandle, 0L, SEEK_CUR ) ) != -1, 0 );
#else
      hb_fsSetIOError( ftruncate( hFileHandle, lseek( hFileHandle, 0L, SEEK_CUR ) ) != -1, 0 );
#endif
      ulWritten = 0;
   }

   hb_vmLock();

   #endif

#else

   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return ulWritten;
}

HB_SIZE hb_fsReadAt( HB_FHANDLE hFileHandle, void * pBuff, HB_SIZE ulCount, HB_FOFFSET llOffset )
{
   HB_THREAD_STUB
   ULONG ulRead;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsReadAt(%p, %p, %lu, %" PFHL "i)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, ulCount, llOffset ) );

#if defined( HB_FS_FILE_IO )

#  if defined( HB_OS_UNIX ) && ! defined( __WATCOMC__ )
   {
      hb_vmUnlock();
#     if defined( HB_USE_LARGEFILE64 )
      ulRead   = pread64( hFileHandle, pBuff, ulCount, llOffset );
#     else
      ulRead   = pread( hFileHandle, pBuff, ulCount, llOffset );
#     endif
      hb_fsSetIOError( ulRead != ( ULONG ) -1, 0 );
      if( ulRead == ( ULONG ) -1 )
         ulRead = 0;
      hb_vmLock();
   }
#  else
#     if defined( HB_WIN32_IO )
   if( hb_iswinnt() )
   {
      OVERLAPPED Overlapped;
      hb_vmUnlock();
      memset( &Overlapped, 0, sizeof( Overlapped ) );
      Overlapped.Offset       = ( DWORD ) ( llOffset & 0xFFFFFFFF ),
      Overlapped.OffsetHigh   = ( DWORD ) ( llOffset >> 32 ),
      hb_fsSetIOError( ReadFile( DosToWinHandle( hFileHandle ),
                                 pBuff, ( DWORD ) ulCount, &ulRead, &Overlapped ), 0 );
      hb_vmLock();
   }
   else
   #endif
   {
      hb_vmUnlock();
      /* TOFIX: this is not atom operation. It has to be fixed for RDD
       *        file access with shared file handles in aliased work areas
       */
      if( hb_fsSeekLarge( hFileHandle, llOffset, FS_SET ) == llOffset )
         ulRead = ( ULONG ) hb_fsReadLarge( hFileHandle, pBuff, ulCount );
      else
         ulRead = 0;
      hb_vmLock();
   }
#  endif

#else

   ulRead = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return ulRead;
}

HB_SIZE hb_fsWriteAt( HB_FHANDLE hFileHandle, const void * pBuff, HB_SIZE ulCount, HB_FOFFSET llOffset )
{
   HB_THREAD_STUB
   ULONG ulWritten;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsWriteAt(%p, %p, %lu, %" PFHL "i)", ( void * ) ( HB_PTRDIFF ) hFileHandle, pBuff, ulCount, llOffset ) );

#if defined( HB_FS_FILE_IO )

   #if defined( HB_OS_UNIX ) && ! defined( __WATCOMC__ )
   {
      hb_vmUnlock();
      #if defined( HB_USE_LARGEFILE64 )
      ulWritten   = pwrite64( hFileHandle, pBuff, ulCount, llOffset );
      #else
      ulWritten   = pwrite( hFileHandle, pBuff, ulCount, llOffset );
      #endif
      hb_fsSetIOError( ulWritten != ( ULONG ) -1, 0 );
      if( ulWritten == ( ULONG ) -1 )
         ulWritten = 0;
      hb_vmLock();
   }
   #else
   #if defined( HB_WIN32_IO )
   if( hb_iswinnt() )
   {
      OVERLAPPED Overlapped;
      hb_vmUnlock();
      memset( &Overlapped, 0, sizeof( Overlapped ) );
      Overlapped.Offset       = ( DWORD ) ( llOffset & 0xFFFFFFFF ),
      Overlapped.OffsetHigh   = ( DWORD ) ( llOffset >> 32 ),
      hb_fsSetIOError( WriteFile( DosToWinHandle( hFileHandle ),
                                  pBuff, ( DWORD ) ulCount, &ulWritten, &Overlapped ), 0 );
      hb_vmLock();
   }
   else
   #endif
   {
      hb_vmUnlock();
      /* TOFIX: this is not atom operation. It has to be fixed for RDD
       *        file access with shared file handles in aliased work areas
       */
      if( hb_fsSeekLarge( hFileHandle, llOffset, FS_SET ) == llOffset )
         ulWritten = ( ULONG ) hb_fsWriteLarge( hFileHandle, pBuff, ulCount );
      else
         ulWritten = 0;
      hb_vmLock();
   }
   #endif

#else

   ulWritten = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return ulWritten;
}

BOOL hb_fsTruncAt( HB_FHANDLE hFileHandle, HB_FOFFSET llOffset )
{
   HB_THREAD_STUB
   BOOL fResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsReadAt(%p, %" PFHL "i)", ( void * ) ( HB_PTRDIFF ) hFileHandle, llOffset ) );

#if defined( HB_FS_FILE_IO )

   hb_vmUnlock();
   #if defined( HB_WIN32_IO )
   {
      ULONG ulOffsetLow    = ( ULONG ) ( llOffset & ULONG_MAX ),
            ulOffsetHigh   = ( ULONG ) ( llOffset >> 32 );

      /* This is not atom operation anyhow if someone want to truncate
       * file then he has to made necessary synchronizations in upper level
       * code. We have such situation in our RDD drivers and for us such
       * version is enough. [druzus]
       */
      ulOffsetLow = SetFilePointer( DosToWinHandle( hFileHandle ),
                                    ulOffsetLow, ( PLONG ) &ulOffsetHigh,
                                    ( DWORD ) SEEK_SET );
      if( ( ( ( HB_FOFFSET ) ulOffsetHigh << 32 ) | ulOffsetLow ) == llOffset )
         fResult = SetEndOfFile( DosToWinHandle( hFileHandle ) );
      else
         fResult = FALSE;
   }
   #elif defined( HB_USE_LARGEFILE64 )
   fResult  = ftruncate64( hFileHandle, llOffset ) != -1;
   #else
   fResult  = ftruncate( hFileHandle, llOffset ) != -1;
   #endif
   hb_fsSetIOError( fResult, 0 );
   hb_vmLock();

#else

   fResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return fResult;
}

void hb_fsCommit( HB_FHANDLE hFileHandle )
{
   HB_THREAD_STUB
   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCommit(%p)", ( void * ) ( HB_PTRDIFF ) hFileHandle ) );

   hb_vmUnlock();

#if defined( HB_OS_WIN )
   {
      // allowing async cancelation here
      #if defined( HB_WIN32_IO )
      hb_fsSetIOError( FlushFileBuffers( ( HANDLE ) DosToWinHandle( hFileHandle ) ), 0 );
      #else
         #if defined( __WATCOMC__ )
      hb_fsSetIOError( fsync( hFileHandle ) == 0, 0 );
         #else
      hb_fsSetIOError( _commit( hFileHandle ) == 0, 0 );
         #endif
      #endif
   }

#elif defined( HB_OS_OS2 )

   {
      hb_fsSetIOError( DosResetBuffer( hFileHandle ) == 0, 0 );
   }

#elif defined( HB_OS_UNIX )

   /* NOTE: close() functions releases all lock regardles if it is an
    * original or duplicated file handle
    */
   #if defined( _POSIX_SYNCHRONIZED_IO ) && _POSIX_SYNCHRONIZED_IO + 0 > 0
   /* faster - flushes data buffers only, without updating directory info
    */
   hb_fsSetIOError( fdatasync( hFileHandle ) == 0, 0 );
   #else
   /* slower - flushes all file data buffers and i-node info
    */
   hb_fsSetIOError( fsync( hFileHandle ) == 0, 0 );
   #endif

#elif defined( __WATCOMC__ )

   hb_fsSetIOError( fsync( hFileHandle ) == 0, 0 );

#elif defined( HB_FS_FILE_IO ) && ! defined( HB_OS_OS2 ) && ! defined( HB_OS_UNIX )

   /* This hack is very dangerous. POSIX standard define that if _ANY_
      file handle is closed all locks set by the process on the file
      pointed by this descriptor are removed. It doesn't matter they
      were done using different descriptor. It means that we now clean
      all locks on hFileHandle with the code below if the OS is POSIX
      compilant. I vote to disable it.
    */
   {
      int   dup_handle;
      BOOL  fResult = FALSE;

      dup_handle = dup( hFileHandle );
      if( dup_handle != -1 )
      {
         close( dup_handle );
         fResult = TRUE;
      }
      hb_fsSetIOError( fResult, 0 );
   }

#else

   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();
}

BOOL hb_fsLock( HB_FHANDLE hFileHandle, HB_SIZE ulStart,
                HB_SIZE ulLength, USHORT uiMode )
{
   HB_THREAD_STUB
   BOOL        bResult;

#if defined( HB_WIN32_IO )
   static BOOL s_bInit = 0, s_bWinNt;
#endif

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsLock(%p, %lu, %lu, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, ulStart, ulLength, uiMode ) );

   hb_vmUnlock();

#if defined( HB_WIN32_IO )
   if( ! s_bInit )
   {
      s_bInit  = TRUE;
      s_bWinNt = hb_iswinnt();
   }
   switch( uiMode & FL_MASK )
   {
      case FL_LOCK:
         if( s_bWinNt )
         {
            OVERLAPPED  sOlap;
            DWORD       dwFlags;
            memset( &sOlap, 0, sizeof( OVERLAPPED ) );
            sOlap.Offset   = ( ULONG ) ulStart;
            dwFlags        = ( uiMode & FLX_SHARED ) ? 0 : LOCKFILE_EXCLUSIVE_LOCK;
            if( ! s_fUseWaitLocks || ! ( uiMode & FLX_WAIT ) )
            {
               dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;
            }
            bResult = LockFileEx( DosToWinHandle( hFileHandle ), dwFlags, 0, ( DWORD ) ulLength, 0, &sOlap );
         }
         else
         {
            bResult = LockFile( DosToWinHandle( hFileHandle ), ( DWORD ) ulStart, 0, ( DWORD ) ulLength, 0 );
         }
         break;
      case FL_UNLOCK:
         if( s_bWinNt )
         {
            OVERLAPPED sOlap;
            memset( &sOlap, 0, sizeof( OVERLAPPED ) );
            sOlap.Offset   = ( ULONG ) ulStart;
            bResult        = UnlockFileEx( DosToWinHandle( hFileHandle ), 0, ( DWORD ) ulLength, 0, &sOlap );
         }
         else
         {
            bResult = UnlockFile( DosToWinHandle( hFileHandle ), ( DWORD ) ulStart, 0, ( DWORD ) ulLength, 0 );
         }
         break;

      default:
         bResult = FALSE;
   }
   hb_fsSetIOError( bResult, 0 );
#elif defined( HB_OS_OS2 )
   {
      struct _FILELOCK fl, ful;

      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:

            fl.lOffset  = ulStart;
            fl.lRange   = ulLength;
            ful.lOffset = 0;
            ful.lRange  = 0;

            /* lock region, 2 seconds timeout, exclusive access - no atomic */
            bResult     = ( DosSetFileLocks( hFileHandle, &ful, &fl, 2000L, 0L ) == 0 );
            break;

         case FL_UNLOCK:

            fl.lOffset  = 0;
            fl.lRange   = 0;
            ful.lOffset = ulStart;
            ful.lRange  = ulLength;

            /* unlock region, 2 seconds timeout, exclusive access - no atomic */
            bResult     = ( DosSetFileLocks( hFileHandle, &ful, &fl, 2000L, 0L ) == 0 );
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );
   }
#elif defined( _MSC_VER ) || defined( __DMC__ )
   {
      ULONG ulOldPos;

      ulOldPos = lseek( hFileHandle, 0L, SEEK_CUR );
      lseek( hFileHandle, ulStart, SEEK_SET );
      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:
            bResult = ( locking( hFileHandle, _LK_NBLCK, ulLength ) == 0 );
            break;

         case FL_UNLOCK:
            bResult = ( locking( hFileHandle, _LK_UNLCK, ulLength ) == 0 );
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );
      lseek( hFileHandle, ulOldPos, SEEK_SET );
   }
#elif defined( __MINGW32__ )
   {
      ULONG ulOldPos;

      ulOldPos = lseek( hFileHandle, 0L, SEEK_CUR );
      lseek( hFileHandle, ulStart, SEEK_SET );
      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:
            bResult = ( _locking( hFileHandle, _LK_LOCK, ulLength ) == 0 );
            break;

         case FL_UNLOCK:
            bResult = ( _locking( hFileHandle, _LK_UNLCK, ulLength ) == 0 );
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );
      lseek( hFileHandle, ulOldPos, SEEK_SET );
   }
#elif defined( HB_OS_UNIX )
   {
      /* TODO: check for append locks (SEEK_END)
       */
      struct flock lock_info;

      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:

            lock_info.l_type     = ( uiMode & FLX_SHARED ) ? F_RDLCK : F_WRLCK;
            lock_info.l_start    = ulStart;
            lock_info.l_len      = ulLength;
            lock_info.l_whence   = SEEK_SET; /* start from the beginning of the file */
            lock_info.l_pid      = 0;

            bResult              = ( fcntl( hFileHandle,
                                            ( uiMode & FLX_WAIT ) ? F_SETLKW : F_SETLK,
                                            &lock_info ) >= 0 );
            break;

         case FL_UNLOCK:

            lock_info.l_type     = F_UNLCK; /* unlock */
            lock_info.l_start    = ulStart;
            lock_info.l_len      = ulLength;
            lock_info.l_whence   = SEEK_SET;
            lock_info.l_pid      = 0;

            bResult              = ( fcntl( hFileHandle, F_SETLK, &lock_info ) >= 0 );
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );
   }
#elif defined( HAVE_POSIX_IO ) && ! defined( __IBMCPP__ ) && ( ! defined( __GNUC__ ) || defined( __DJGPP__ ) )

   switch( uiMode & FL_MASK )
   {
      case FL_LOCK:
         bResult = ( lock( hFileHandle, ulStart, ulLength ) == 0 );
         break;

      case FL_UNLOCK:
         bResult = ( unlock( hFileHandle, ulStart, ulLength ) == 0 );
         break;

      default:
         bResult = FALSE;
   }
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   return bResult;
}

BOOL hb_fsLockLarge( HB_FHANDLE hFileHandle, HB_FOFFSET ulStart,
                     HB_FOFFSET ulLength, USHORT uiMode )
{
   BOOL bResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsLockLarge(%p, %" PFHL "u, %" PFHL "i, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, ulStart, ulLength, uiMode ) );

#if defined( HB_WIN32_IO )
   {
      HB_THREAD_STUB

      DWORD dwOffsetLo     = ( DWORD ) ( ulStart & 0xFFFFFFFF ),
            dwOffsetHi     = ( DWORD ) ( ulStart >> 32 ),
            dwLengthLo     = ( DWORD ) ( ulLength & 0xFFFFFFFF ),
            dwLengthHi     = ( DWORD ) ( ulLength >> 32 );

      static BOOL s_bInit  = 0, s_bWinNt;

      if( ! s_bInit )
      {
         s_bInit  = TRUE;
         s_bWinNt = hb_iswinnt();
      }

      hb_vmUnlock();

      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:
            if( s_bWinNt )
            {
               OVERLAPPED  sOlap;
               DWORD       dwFlags;

               dwFlags = ( ( uiMode & FLX_SHARED ) ? 0 : LOCKFILE_EXCLUSIVE_LOCK );
               if( ! s_fUseWaitLocks || ! ( uiMode & FLX_WAIT ) )
               {
                  dwFlags |= LOCKFILE_FAIL_IMMEDIATELY;
               }

               memset( &sOlap, 0, sizeof( OVERLAPPED ) );
               sOlap.Offset      = dwOffsetLo;
               sOlap.OffsetHigh  = dwOffsetHi;

               bResult           = LockFileEx( DosToWinHandle( hFileHandle ), dwFlags, 0,
                                               dwLengthLo, dwLengthHi, &sOlap );
            }
            else
            {
               bResult = LockFile( DosToWinHandle( hFileHandle ),
                                   dwOffsetLo, dwOffsetHi,
                                   dwLengthLo, dwLengthHi );
            }
            break;

         case FL_UNLOCK:
            if( s_bWinNt )
            {
               OVERLAPPED sOlap;

               memset( &sOlap, 0, sizeof( OVERLAPPED ) );
               sOlap.Offset      = dwOffsetLo;
               sOlap.OffsetHigh  = dwOffsetHi;

               bResult           = UnlockFileEx( DosToWinHandle( hFileHandle ), 0,
                                                 dwLengthLo, dwLengthHi, &sOlap );
            }
            else
            {
               bResult = UnlockFile( DosToWinHandle( hFileHandle ),
                                     dwOffsetLo, dwOffsetHi,
                                     dwLengthLo, dwLengthHi );
            }
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );

      hb_vmLock();
   }
#elif defined( HB_USE_LARGEFILE64 )
   {
      HB_THREAD_STUB

      struct flock64 lock_info;

      hb_vmUnlock();

      switch( uiMode & FL_MASK )
      {
         case FL_LOCK:

            lock_info.l_type     = ( uiMode & FLX_SHARED ) ? F_RDLCK : F_WRLCK;
            lock_info.l_start    = ulStart;
            lock_info.l_len      = ulLength;
            lock_info.l_whence   = SEEK_SET; /* start from the beginning of the file */
            lock_info.l_pid      = 0;

            bResult              = ( fcntl( hFileHandle,
                                            ( uiMode & FLX_WAIT ) ? F_SETLKW64 : F_SETLK64,
                                            &lock_info ) != -1 );
            break;

         case FL_UNLOCK:

            lock_info.l_type     = F_UNLCK; /* unlock */
            lock_info.l_start    = ulStart;
            lock_info.l_len      = ulLength;
            lock_info.l_whence   = SEEK_SET;
            lock_info.l_pid      = 0;

            bResult              = ( fcntl( hFileHandle, F_SETLK64, &lock_info ) != -1 );
            break;

         default:
            bResult = FALSE;
      }
      hb_fsSetIOError( bResult, 0 );

      hb_vmLock();
   }
#else
   bResult = hb_fsLock( hFileHandle, ( ULONG ) ulStart, ( ULONG ) ulLength, uiMode );
#endif

   return bResult;
}

ULONG hb_fsSeek( HB_FHANDLE hFileHandle, LONG lOffset, USHORT uiFlags )
{
   HB_THREAD_STUB
   /* Clipper compatibility: under clipper, ulPos is returned as it was
      before; on error it is not changed. This is not thread compliant,
      but does not cares as MT prg are required to test FError(). */
   /* This is nothing compilat, this static var is not bound with file
      handle and it will cause overwriting database files and many other
      bad side effect if seek fails - it's one of the most serious bug
      I've found so far in filesys.c - ulPos _CANNOT_BE_ static, Druzus */
   ULONG ulPos;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsSeek(%p, %ld, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, lOffset, uiFlags ) );

   hb_vmUnlock();

#if defined( HB_FS_FILE_IO )
   {
      USHORT Flags = convert_seek_flags( uiFlags );

   #if defined( HB_OS_OS2 )
      {
         APIRET ret;

         /* This DOS hack creates 2GB file size limit, Druzus */
         if( lOffset < 0 && Flags == SEEK_SET )
         {
            ret = 1;
            hb_fsSetError( 25 ); /* 'Seek Error' */
         }
         else
         {
            ret = DosSetFilePtr( hFileHandle, lOffset, Flags, &ulPos );
            /* TODO: what we should do with this error code? Is it DOS compatible? */
            hb_fsSetError( ( USHORT ) ret );
         }
         if( ret != 0 )
         {
            /* FIXME: it should work if DosSetFilePtr is lseek compatible
               but maybe OS2 has DosGetFilePtr too, if not then remove this
               comment, Druzus */
            if( DosSetFilePtr( hFileHandle, 0, SEEK_CUR, &ulPos ) != 0 )
            {
               ulPos = 0;
            }
         }
      }
   #elif defined( HB_WIN32_IO )
      /* This DOS hack creates 2GB file size limit, Druzus */
      if( lOffset < 0 && Flags == SEEK_SET )
      {
         ulPos = ( ULONG ) INVALID_SET_FILE_POINTER;
         hb_fsSetError( 25 ); /* 'Seek Error' */
      }
      else
      {
         ulPos = ( DWORD ) SetFilePointer( DosToWinHandle( hFileHandle ), lOffset, NULL, ( DWORD ) Flags );
         hb_fsSetIOError( ( DWORD ) ulPos != INVALID_SET_FILE_POINTER, 0 );
      }

      if( ( DWORD ) ulPos == INVALID_SET_FILE_POINTER )
      {
         ulPos = ( DWORD ) SetFilePointer( DosToWinHandle( hFileHandle ), 0, NULL, SEEK_CUR );
      }
   #else
      /* This DOS hack creates 2GB file size limit, Druzus */
      if( lOffset < 0 && Flags == SEEK_SET )
      {
         ulPos = ( ULONG ) -1;
         hb_fsSetError( 25 ); /* 'Seek Error' */
      }
      else
      {
         ulPos = lseek( hFileHandle, lOffset, Flags );
         hb_fsSetIOError( ulPos != ( ULONG ) -1, 0 );
      }
      if( ulPos == ( ULONG ) -1 )
      {
         ulPos = lseek( hFileHandle, 0L, SEEK_CUR );
         if( ulPos == ( ULONG ) -1 )
         {
            ulPos = 0;
         }
      }
   #endif
   }
#else
   hb_fsSetError( 25 );
   ulPos = 0;
#endif

   hb_vmLock();

   return ulPos;
}

HB_FOFFSET hb_fsSeekLarge( HB_FHANDLE hFileHandle, HB_FOFFSET llOffset, USHORT uiFlags )
{
   HB_FOFFSET llPos;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsSeekLarge(%p, %" PFHL "i, %hu)", ( void * ) ( HB_PTRDIFF ) hFileHandle, llOffset, uiFlags ) );

#if defined( HB_WIN32_IO )
   {
      HB_THREAD_STUB

      USHORT   Flags          = convert_seek_flags( uiFlags );

      ULONG    ulOffsetLow    = ( ULONG ) ( llOffset & ULONG_MAX ),
               ulOffsetHigh   = ( ULONG ) ( llOffset >> 32 );

      hb_vmUnlock();

      if( llOffset < 0 && Flags == SEEK_SET )
      {
         llPos = ( HB_FOFFSET ) INVALID_SET_FILE_POINTER;
         hb_fsSetError( 25 ); /* 'Seek Error' */
      }
      else
      {
         ulOffsetLow = SetFilePointer( DosToWinHandle( hFileHandle ),
                                       ulOffsetLow, ( PLONG ) &ulOffsetHigh,
                                       ( DWORD ) Flags );
         llPos       = ( ( HB_FOFFSET ) ulOffsetHigh << 32 ) | ulOffsetLow;
         hb_fsSetIOError( llPos != ( HB_FOFFSET ) INVALID_SET_FILE_POINTER, 0 );
      }

      if( llPos == ( HB_FOFFSET ) INVALID_SET_FILE_POINTER )
      {
         ulOffsetHigh   = 0;
         ulOffsetLow    = SetFilePointer( DosToWinHandle( hFileHandle ),
                                          0, ( PLONG ) &ulOffsetHigh, SEEK_CUR );
         llPos          = ( ( HB_FOFFSET ) ulOffsetHigh << 32 ) | ulOffsetLow;
      }

      hb_vmLock();
   }
#elif defined( HB_USE_LARGEFILE64 )
   {
      HB_THREAD_STUB

      USHORT Flags = convert_seek_flags( uiFlags );

      hb_vmUnlock();

      if( llOffset < 0 && Flags == SEEK_SET )
      {
         llPos = ( HB_FOFFSET ) -1;
         hb_fsSetError( 25 ); /* 'Seek Error' */
      }
      else
      {
         llPos = lseek64( hFileHandle, llOffset, Flags );
         hb_fsSetIOError( llPos != ( HB_FOFFSET ) -1, 0 );
      }

      if( llPos == ( HB_FOFFSET ) -1 )
      {
         llPos = lseek64( hFileHandle, 0L, SEEK_CUR );
      }

      hb_vmLock();
   }
#else
   llPos = ( HB_FOFFSET ) hb_fsSeek( hFileHandle, ( LONG ) llOffset, uiFlags );
#endif

   return llPos;
}

ULONG hb_fsTell( HB_FHANDLE hFileHandle )
{
   HB_TRACE( HB_TR_DEBUG, ( "hb_fsTell(%p)", ( void * ) ( HB_PTRDIFF ) hFileHandle ) );

   return hb_fsSeek( hFileHandle, 0, FS_RELATIVE );
}

BOOL hb_fsDelete( const char * pFilename )
{
   HB_THREAD_STUB
   BOOL     bResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsDelete(%s)", pFilename ) );

   pFilename = hb_fsNameConv( pFilename, &pszFree );

   hb_vmUnlock();


#if defined( HB_OS_WIN )

   bResult = DeleteFileA( pFilename );
   hb_fsSetIOError( bResult, 0 );

#elif defined( HAVE_POSIX_IO )

   bResult = ( remove( pFilename ) == 0 );
   hb_fsSetIOError( bResult, 0 );

#elif defined( _MSC_VER ) || defined( __MINGW32__ ) || defined( __DMC__ )

   bResult = ( remove( pFilename ) == 0 );
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   if( pszFree )
      hb_xfree( pszFree );

   return bResult;
}

BOOL hb_fsRename( const char * pOldName, const char * pNewName )
{
   HB_THREAD_STUB
   BOOL     bResult;
   char *   pszFreeOld, * pszFreeNew;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsRename(%s, %s)", pOldName, ( char * ) pNewName ) );

   pOldName = hb_fsNameConv( pOldName, &pszFreeOld );
   pNewName = hb_fsNameConv( pNewName, &pszFreeNew );

   hb_vmUnlock();


#if defined( HB_OS_WIN )

   bResult = MoveFileA( pOldName, pNewName );
   hb_fsSetIOError( bResult, 0 );

#elif defined( HB_FS_FILE_IO )

   bResult = ( rename( pOldName, pNewName ) == 0 );
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif


   hb_vmLock();

   if( pszFreeOld )
      hb_xfree( pszFreeOld );
   if( pszFreeNew )
      hb_xfree( pszFreeNew );

   return bResult;
}

BOOL hb_fsMkDir( const char * pDirname )
{
   HB_THREAD_STUB
   BOOL     bResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsMkDir(%s)", pDirname ) );

   pDirname = hb_fsNameConv( pDirname, &pszFree );

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsMkDir(%s)", ( char * ) pDirname ) );

   hb_vmUnlock();

#if defined( HB_OS_WIN )

   bResult = CreateDirectoryA( pDirname, NULL );
   hb_fsSetIOError( bResult, 0 );

#elif defined( HAVE_POSIX_IO ) || defined( __MINGW32__ )

#  if ! defined( HB_OS_UNIX ) && \
   ( defined( __WATCOMC__ ) || defined( __BORLANDC__ ) || \
   defined( __IBMCPP__ ) || defined( __MINGW32__ ) )
   bResult  = ( mkdir( ( char * ) pDirname ) == 0 );
#  else
   bResult  = ( mkdir( pDirname, S_IRWXU | S_IRWXG | S_IRWXO ) == 0 );
#  endif
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();
   if( pszFree )
      hb_xfree( pszFree );

   return bResult;
}

BOOL hb_fsChDir( const char * pDirname )
{
   HB_THREAD_STUB
   BOOL     bResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsChDir(%s)", pDirname ) );

   pDirname = hb_fsNameConv( pDirname, &pszFree );

   hb_vmUnlock();

#if defined( HB_OS_WIN )

   bResult = SetCurrentDirectoryA( pDirname );
   hb_fsSetIOError( bResult, 0 );

#elif defined( HAVE_POSIX_IO ) || defined( __MINGW32__ )

   bResult = ( chdir( ( char * ) pDirname ) == 0 );
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();
   if( pszFree )
      hb_xfree( pszFree );

   return bResult;
}

BOOL hb_fsRmDir( const char * pDirname )
{
   HB_THREAD_STUB
   BOOL     bResult;
   char *   pszFree;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsRmDir(%s)", pDirname ) );

   pDirname = hb_fsNameConv( pDirname, &pszFree );

   hb_vmUnlock();

#if defined( HB_OS_WIN )

   bResult = RemoveDirectoryA( pDirname );
   hb_fsSetIOError( bResult, 0 );

#elif defined( HAVE_POSIX_IO ) || defined( __MINGW32__ )

   bResult = ( rmdir( pDirname ) == 0 );
   hb_fsSetIOError( bResult, 0 );

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();
   if( pszFree )
      hb_xfree( pszFree );

   return bResult;
}

/* NOTE: This is not thread safe function, it's there for compatibility. */
/* NOTE: 0 = current drive, 1 = A, 2 = B, 3 = C, etc. */

const char * hb_fsCurDir( USHORT uiDrive )
{
   static char pbyDirBuffer[ HB_PATH_MAX ];

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCurDir(%hu)", uiDrive ) );

   hb_fsCurDirBuff( uiDrive, pbyDirBuffer, HB_PATH_MAX );

   return pbyDirBuffer;
}

/* NOTE: Thread safe version of hb_fsCurDir() */
/* NOTE: 0 = current drive, 1 = A, 2 = B, 3 = C, etc. */

USHORT hb_fsCurDirBuff( USHORT uiDrive, char * pbyBuffer, HB_SIZE ulLen )
{
   HB_THREAD_STUB
   USHORT   uiCurDrv = uiDrive, usError;
   BOOL     fResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCurDirBuff(%hu)", uiDrive ) );

   pbyBuffer[ 0 ] = '\0';

   /*
    * do not cover this code by HB_OS_HAS_DRIVE_LETTER macro
    * It will allow us to add drive emulation in hb_fsCurDrv()/hb_fsChDrv()
    * and hb_fsNameConv()
    */
#if ! ( defined( __GNUC__ ) && ( defined( HB_OS_OS2 ) || ! defined( __MINGW32__ ) ) )
   if( uiDrive )
   {
      uiCurDrv = hb_fsCurDrv() + 1;
      if( uiDrive != uiCurDrv )
         hb_fsChDrv( ( BYTE ) ( uiDrive - 1 ) );
   }
#endif

   hb_vmUnlock();

#if defined( HB_OS_WIN )

   fResult = GetCurrentDirectoryA( ( DWORD ) ulLen, ( char * ) pbyBuffer );
   hb_fsSetIOError( fResult, 0 );

#elif defined( HB_OS_OS2 ) && defined( __GNUC__ )

   fResult = ( _getcwd1( ( char * ) pbyBuffer, uiDrive + 'A' - 1 ) == 0 );
   hb_fsSetIOError( fResult, 0 );

#elif defined( HAVE_POSIX_IO )

   fResult = ( getcwd( ( char * ) pbyBuffer, ulLen ) != NULL );
   hb_fsSetIOError( fResult, 0 );

#elif defined( __MINGW32__ )

   fResult = ( _getdcwd( uiDrive, pbyBuffer, ulLen ) != NULL );
   hb_fsSetIOError( fResult, 0 );

#else

   fResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   hb_vmLock();

   usError = hb_fsError();

   if( uiDrive != uiCurDrv )
   {
      hb_fsChDrv( ( BYTE ) ( uiCurDrv - 1 ) );
      hb_fsSetError( usError );
   }

   pbyBuffer[ ulLen - 1 ] = '\0';

   if( usError == 0 && pbyBuffer[ 0 ] )
   {
      char * pbyStart = pbyBuffer;

      /* Strip the leading drive spec, and leading backslash if there's one. */
      /* NOTE: A trailing underscore is not returned on this platform,
               so we don't need to strip it. [vszakats] */

      ulLen = strlen( pbyBuffer );

#if defined( HB_OS_HAS_DRIVE_LETTER )
      if( pbyStart[ 1 ] == HB_OS_DRIVE_DELIM_CHR )
      {
         pbyStart += 2;
         ulLen    -= 2;
      }
#endif
      if( strchr( HB_OS_PATH_DELIM_CHR_LIST, pbyStart[ 0 ] ) )
      {
         pbyStart++;
         ulLen--;
      }

      /* Strip the trailing (back)slash if there's one */
      if( ulLen && strchr( HB_OS_PATH_DELIM_CHR_LIST, pbyStart[ ulLen - 1 ] ) )
         ulLen--;

      if( ulLen && pbyBuffer != pbyStart )
         memmove( pbyBuffer, pbyStart, ( size_t ) ulLen );

      pbyBuffer[ ulLen ] = '\0';
   }

   return usError;
}

/* NOTE: 0=A:, 1=B:, 2=C:, 3=D:, ... */

USHORT hb_fsChDrv( BYTE nDrive )
{
#if defined( HB_OS_HAS_DRIVE_LETTER )
   HB_THREAD_STUB
#endif

   USHORT uiResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsChDrv(%d)", ( int ) nDrive ) );

#if defined( HB_OS_HAS_DRIVE_LETTER )
   {
      /* 'unsigned int' _have to_ be used in Watcom */
      UINT uiSave, uiNewDrive;

      hb_vmUnlock();

      HB_FS_GETDRIVE( uiSave );
      HB_FS_SETDRIVE( nDrive );
      HB_FS_GETDRIVE( uiNewDrive );

      if( ( UINT ) nDrive == uiNewDrive )
      {
         uiResult = 0;
         hb_fsSetError( 0 );
      }
      else
      {
         HB_FS_SETDRIVE( uiSave );

         uiResult = ( USHORT ) FS_ERROR;
         hb_fsSetError( ( USHORT ) FS_ERROR );
      }
      hb_vmLock();
   }
#else

   HB_SYMBOL_UNUSED( nDrive );
   uiResult = ( USHORT ) FS_ERROR;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return uiResult;
}

/* NOTE: 0=A:, 1=B:, 2=C:, 3=D:, ... */

BYTE hb_fsCurDrv( void )
{
#if defined( HB_OS_UNIX ) || ( defined( HB_OS_HAS_DRIVE_LETTER ) || ( defined( HB_OS_WIN ) && ! defined( HB_OS_WIN_CE ) ) )
   HB_THREAD_STUB
#endif
   /* 'unsigned int' _have to_ be used in Watcom */
   UINT uiResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCurDrv()" ) );

#if defined( HB_OS_HAS_DRIVE_LETTER )

   hb_vmUnlock();
   HB_FS_GETDRIVE( uiResult );
   hb_fsSetError( 0 );
   hb_vmLock();

#else

   uiResult = 0;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return ( BYTE ) uiResult; /* Return the drive number, base 0. */
}

/* NOTE: 0=A:, 1=B:, 2=C:, 3=D:, ... */

/* TOFIX: This isn't fully compliant because CA-Cl*pper doesn't access
          the drive before checking. hb_fsIsDrv only returns TRUE
          if there is a disk in the drive. */

USHORT hb_fsIsDrv( BYTE nDrive )
{
#if defined( HB_OS_HAS_DRIVE_LETTER ) || ( defined( HB_OS_WIN ) && ! defined( HB_OS_WIN_CE ) )
   HB_THREAD_STUB
#endif

   USHORT uiResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsIsDrv(%d)", ( int ) nDrive ) );

#if defined( HB_OS_WIN ) && ! defined( HB_OS_WIN_CE )
   {
      char  buffer[ 4 ];
      UINT  type;

      buffer[ 0 ] = nDrive + 'A';
      buffer[ 1 ] = ':';
      buffer[ 2 ] = '\\';
      buffer[ 3 ] = '\0';

      hb_vmUnlock();
      type        = GetDriveTypeA( buffer );
      hb_vmLock();
      uiResult    = ( type == DRIVE_UNKNOWN || type == DRIVE_NO_ROOT_DIR ) ? F_ERROR : 0;
      hb_fsSetError( 0 );
   }
#elif defined( HB_OS_HAS_DRIVE_LETTER )
   {
      /* 'unsigned int' _have to_ be used in Watcom
       */
      UINT uiSave, uiNewDrive;

      hb_vmUnlock();

      HB_FS_GETDRIVE( uiSave );
      HB_FS_SETDRIVE( nDrive );
      HB_FS_GETDRIVE( uiNewDrive );
      if( ( UINT ) nDrive != uiNewDrive )
      {
         uiResult = ( USHORT ) FS_ERROR;
         hb_fsSetError( ( USHORT ) FS_ERROR );
      }
      else
      {
         uiResult = 0;
         hb_fsSetError( 0 );
      }
      HB_FS_SETDRIVE( uiSave );

      hb_vmLock();
   }
#else

   HB_SYMBOL_UNUSED( nDrive );
   uiResult = ( USHORT ) FS_ERROR;
   hb_fsSetError( ( USHORT ) FS_ERROR );

#endif

   return uiResult;
}

BOOL hb_fsIsDevice( HB_FHANDLE hFileHandle )
{
   BOOL bResult;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsIsDevice(%p)", ( void * ) ( HB_PTRDIFF ) hFileHandle ) );

#if defined( HB_OS_WIN )

   hb_vmUnlock();
   bResult = GetFileType( DosToWinHandle( hFileHandle ) ) == FILE_TYPE_CHAR;
   hb_fsSetIOError( bResult, 0 );
   hb_vmLock();

#elif defined( HB_FS_FILE_IO )

   hb_vmUnlock();
#if defined( _MSC_VER ) || defined( __MINGW32__ )
   bResult  = _isatty( hFileHandle ) != 0;
#else
   bResult  = isatty( hFileHandle ) != 0;
#endif
   hb_fsSetIOError( bResult, 0 );
   hb_vmLock();

#else

   bResult = FALSE;
   hb_fsSetError( ( USHORT ) FS_ERROR );
   HB_SYMBOL_UNUSED( hFileHandle );

#endif

   return bResult;
}

/* convert file name for hb_fsExtOpen
 * caller must free the returned buffer
 */
char * hb_fsExtName( const char * pFilename, const char * pDefExt, USHORT uiExFlags, const char * pPaths )
{
   HB_PATHNAMES * pNextPath;
   PHB_FNAME      pFilepath;
   BOOL           fIsFile = FALSE;
   char *         szPath;

   szPath      = ( char * ) hb_xgrab( HB_PATH_MAX );

   pFilepath   = hb_fsFNameSplit( pFilename );

   if( pDefExt && ( ( uiExFlags & FXO_FORCEEXT ) || ! pFilepath->szExtension ) )
   {
      pFilepath->szExtension = pDefExt;
   }

   if( pFilepath->szPath )
   {
      hb_fsFNameMerge( szPath, pFilepath );
   }
   else if( uiExFlags & FXO_DEFAULTS )
   {
      const char * szDefault = hb_setGetDefault();
      if( szDefault )
      {
         pFilepath->szPath = szDefault;
         hb_fsFNameMerge( szPath, pFilepath );
         fIsFile           = hb_fsFile( szPath );
      }
      if( ! fIsFile && hb_setGetPath() )
      {
         pNextPath = hb_setGetFirstSetPath();
         while( ! fIsFile && pNextPath )
         {
            pFilepath->szPath = pNextPath->szPath;
            hb_fsFNameMerge( szPath, pFilepath );
            fIsFile           = hb_fsFile( szPath );
            pNextPath         = pNextPath->pNext;
         }
      }
      if( ! fIsFile )
      {
         pFilepath->szPath = szDefault ? szDefault : NULL;
         hb_fsFNameMerge( szPath, pFilepath );
      }
   }
   else if( pPaths && *pPaths )
   {
      HB_PATHNAMES * pSearchPath = NULL;
      hb_fsAddSearchPath( pPaths, &pSearchPath );
      pNextPath = pSearchPath;
      while( ! fIsFile && pNextPath )
      {
         pFilepath->szPath = pNextPath->szPath;
         hb_fsFNameMerge( szPath, pFilepath );
         fIsFile           = hb_fsFile( szPath );
         pNextPath         = pNextPath->pNext;
      }
      hb_fsFreeSearchPath( pSearchPath );
      if( ! fIsFile )
      {
         pFilepath->szPath = NULL;
         hb_fsFNameMerge( szPath, pFilepath );
      }
   }
   else
   {
      hb_fsFNameMerge( szPath, pFilepath );
   }
   hb_xfree( pFilepath );

   return szPath;
}

HB_FHANDLE hb_fsExtOpen( const char * pFilename, const char * pDefExt,
                         USHORT uiExFlags, const char * pPaths,
                         PHB_ITEM pError )
{
   HB_FHANDLE  hFile;
   USHORT      uiFlags;
   char *      szPath;

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsExtOpen(%s, %s, %hu, %p, %p)", pFilename, pDefExt, uiExFlags, pPaths, pError ) );

#if 0
   #define FXO_TRUNCATE    0x0100   /* Create (truncate if exists) */
   #define FXO_APPEND      0x0200   /* Create (append if exists) */
   #define FXO_UNIQUE      0x0400   /* Create unique file FO_EXCL ??? */
   #define FXO_FORCEEXT    0x0800   /* Force default extension */
   #define FXO_DEFAULTS    0x1000   /* Use SET command defaults */
   #define FXO_DEVICERAW   0x2000   /* Open devices in raw mode */
   /* xHarbour extension */
   #define FXO_SHARELOCK   0x4000   /* emulate DOS SH_DENY* mode in POSIX OS */
   #define FXO_COPYNAME    0x8000   /* copy final szPath into pFilename */

   hb_errGetFileName( pError );
#endif

   szPath   = hb_fsExtName( pFilename, pDefExt, uiExFlags, pPaths );

   uiFlags  = uiExFlags & 0xff;
   if( uiExFlags & ( FXO_TRUNCATE | FXO_APPEND | FXO_UNIQUE ) )
   {
      uiFlags |= FO_CREAT;
      if( uiExFlags & FXO_UNIQUE )
         uiFlags |= FO_EXCL;
#if ! defined( HB_USE_SHARELOCKS )
      else if( uiExFlags & FXO_TRUNCATE )
         uiFlags |= FO_TRUNC;
#endif
   }

   hFile = hb_fsOpen( szPath, uiFlags );

#if defined( HB_USE_SHARELOCKS )
   if( hFile != FS_ERROR && uiExFlags & FXO_SHARELOCK )
   {
#if defined( HB_USE_BSDLOCKS )
      int iLock;
      if( ( uiFlags & ( FO_READ | FO_WRITE | FO_READWRITE ) ) == FO_READ ||
          ( uiFlags & ( FO_DENYREAD | FO_DENYWRITE | FO_EXCLUSIVE ) ) == 0 )
         iLock = LOCK_SH | LOCK_NB;
      else
         iLock = LOCK_EX | LOCK_NB;
      hb_vmUnlock();
      iLock = flock( hFile, iLock );
      hb_vmLock();
      if( iLock != 0 )
#else
      USHORT uiLock;
      if( ( uiFlags & ( FO_READ | FO_WRITE | FO_READWRITE ) ) == FO_READ ||
          ( uiFlags & ( FO_DENYREAD | FO_DENYWRITE | FO_EXCLUSIVE ) ) == 0 )
         uiLock = FL_LOCK | FLX_SHARED;
      else
         uiLock = FL_LOCK | FLX_EXCLUSIVE;

      if( ! hb_fsLockLarge( hFile, HB_SHARELOCK_POS, HB_SHARELOCK_SIZE, uiLock ) )
#endif
      {
         hb_fsClose( hFile );
         hFile = FS_ERROR;
         /*
          * fix for neterr() support and Clipper compatibility,
          * should be revised with a better multi platform solution.
          */
         hb_fsSetError( ( uiExFlags & FXO_TRUNCATE ) ? 5 : 32 );
      }
      else if( uiExFlags & FXO_TRUNCATE )
      {
         /* truncate the file only if properly locked */
         hb_fsSeek( hFile, 0, FS_SET );
         hb_fsWrite( hFile, NULL, 0 );
         if( hb_fsError() != 0 )
         {
            hb_fsClose( hFile );
            hFile = FS_ERROR;
            hb_fsSetError( 5 );
         }
      }
   }
#elif 1
   /*
    * Temporary fix for neterr() support and Clipper compatibility,
    * should be revised with a better solution.
    */
   if( ( uiExFlags & ( FXO_TRUNCATE | FXO_APPEND | FXO_UNIQUE ) ) == 0 &&
       hb_fsError() == 5 )
   {
      hb_fsSetError( 32 );
   }
#endif

   if( pError )
   {
      hb_errPutFileName( pError, szPath );
      if( hFile == FS_ERROR )
      {
         hb_errPutOsCode( pError, hb_fsError() );
         hb_errPutGenCode( pError, ( USHORT ) ( ( uiExFlags & FXO_TRUNCATE ) ? EG_CREATE : EG_OPEN ) );
      }
   }

   if( uiExFlags & FXO_COPYNAME && hFile != FS_ERROR )
      hb_strncpy( ( char * ) pFilename, szPath, HB_PATH_MAX - 1 );

   hb_xfree( szPath );
   return hFile;
}

BOOL hb_fsEof( HB_FHANDLE hFileHandle )
{
   BOOL fResult;

   hb_vmUnlock();

#if defined( __DJGPP__ ) || defined( __CYGWIN__ ) || \
   defined( HB_WIN32_IO ) || defined( HB_OS_WIN_CE ) || \
   defined( HB_OS_UNIX )

   HB_THREAD_STUB
   {
      HB_FOFFSET  curPos;
      HB_FOFFSET  endPos;
      HB_FOFFSET  newPos;

      curPos = hb_fsSeekLarge( hFileHandle, 0L, SEEK_CUR );
      if( curPos != -1 )
      {
         endPos   = hb_fsSeekLarge( hFileHandle, 0L, SEEK_END );
         newPos   = hb_fsSeekLarge( hFileHandle, curPos, SEEK_SET );
         fResult  = ( endPos != -1 && newPos == curPos );
      }
      else
      {
         endPos   = -1;
         fResult  = FALSE;
      }
      hb_fsSetIOError( fResult, 0 );

      fResult = ! fResult || curPos == endPos;
   }
#else
   fResult = eof( hFileHandle ) != 0;
   hb_fsSetIOError( fResult, 0 );
#endif

   hb_vmLock();

   return fResult;
}

char * hb_fsCurDirEx( USHORT uiDrive )
{
   static char s_byDirBuffer[ HB_PATH_MAX ];

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCurDir(%hu)", uiDrive ) );

   hb_fsCurDirBuffEx( uiDrive, s_byDirBuffer, HB_PATH_MAX );

   return ( char * ) s_byDirBuffer;
}

USHORT hb_fsCurDirBuffEx( USHORT uiDrive, char * pbyBuffer, HB_SIZE ulLen )
{
#if defined( HB_OS_WIN )
   HB_THREAD_STUB
#endif

   HB_TRACE( HB_TR_DEBUG, ( "hb_fsCurDirBuffEx(%hu)", uiDrive ) );

   HB_SYMBOL_UNUSED( uiDrive );

   pbyBuffer[ 0 ] = '\0';

#if defined( HB_OS_WIN )
   {
      DWORD dwResult;

      dwResult = GetCurrentDirectoryA( ( DWORD ) ulLen, ( char * ) pbyBuffer );
      hb_fsSetIOError( dwResult != 0, 0 );
      hb_vmLock();
   }

#elif defined( HB_OS_OS2 )

   hb_fsSetIOError( ( _getcwd1( ( char * ) pbyBuffer, uiDrive + 'A' ) != 0 ), 0 );

#elif defined( HAVE_POSIX_IO )

   hb_fsSetIOError( getcwd( ( char * ) pbyBuffer, ulLen ) != NULL, 0 );

#elif defined( __MINGW32__ )

   hb_fsSetIOError( _getdcwd( uiDrive, pbyBuffer, ulLen ) != NULL, 0 );

#else

   hb_fsSetError( ( USHORT ) FS_ERROR );
   return ( USHORT ) FS_ERROR;

#endif

   /* Strip the leading drive spec, and leading backslash if there's one. */

   if( hb_fsError() != 0 )
   {
      return hb_fsError();
   }
   else
   {
      char *   pbyStart = pbyBuffer;
      HB_SIZE  ulPthLen;

      /* NOTE: A trailing underscore is not returned on this platform,
               so we don't need to strip it. [vszakats] */

#if defined( HB_OS_HAS_DRIVE_LETTER )
      if( pbyStart[ 1 ] == HB_OS_DRIVE_DELIM_CHR )
      {
         pbyStart += 2;
      }
#endif

      if( pbyBuffer != pbyStart )
      {
         memmove( pbyBuffer, pbyStart, ( size_t ) ulLen );
      }

      /* Strip the trailing (back)slash if there's one */
      ulPthLen = strlen( ( char * ) pbyBuffer );

      if( strchr( HB_OS_PATH_DELIM_CHR_LIST, pbyBuffer[ ulPthLen - 1 ] ) )
      {
         pbyBuffer[ ulPthLen ] = '\0';
      }
      else
      {
         hb_xstrcat( ( char * ) pbyBuffer, HB_OS_PATH_DELIM_CHR_STRING, 0 );
      }

      return 0; // if it reaches here, it is right.
   }
}

const char * hb_fsNameConv( const char * szFileName, char ** pszFree )
{
   int            iFileCase, iDirCase;
   const char *   pszCP;
   char           cDirSep;
   BOOL           fTrim;

/*
   Convert file and dir case. The allowed SET options are:
      LOWER - Convert all caracters of file to lower
      UPPER - Convert all caracters of file to upper
      MIXED - Leave as is

   The allowed environment options are:
      FILECASE - define the case of file
      DIRCASE - define the case of path
      DIRSEPARATOR - define separator of path (Ex. "/")
      TRIMFILENAME - strip trailing and leading spaces (also from extension)
 */

   if( pszFree )
      *pszFree = NULL;
   fTrim       = hb_setGetTrimFileName();
   cDirSep     = ( char ) hb_setGetDirSeparator();
   iFileCase   = hb_setGetFileCase();
   iDirCase    = hb_setGetDirCase();
   pszCP       = hb_setGetOSCODEPAGE();
   if( pszCP && *pszCP == 0 )
      pszCP = NULL;

   if( fTrim ||
       cDirSep != HB_OS_PATH_DELIM_CHR ||
       iFileCase != HB_SET_CASE_MIXED ||
       iDirCase != HB_SET_CASE_MIXED ||
       pszCP )
   {
      PHB_FNAME   pFileName;
      HB_SIZE     ulLen;

      if( pszFree )
      {
         szFileName = *pszFree = hb_strncpy( ( char * ) hb_xgrab( HB_PATH_MAX ),
                                             szFileName, HB_PATH_MAX - 1 );
      }

      if( cDirSep != HB_OS_PATH_DELIM_CHR )
      {
         char * p = ( char * ) szFileName;
         while( *p )
         {
            if( *p == cDirSep )
               *p = HB_OS_PATH_DELIM_CHR;
            p++;
         }
      }

      pFileName = hb_fsFNameSplit( szFileName );

      /* strip trailing and leading spaces */
      if( fTrim )
      {
         if( pFileName->szName )
         {
            ulLen                                     = strlen( pFileName->szName );
            ulLen                                     = hb_strRTrimLen( pFileName->szName, ulLen, FALSE );
            pFileName->szName                         = hb_strLTrim( pFileName->szName, &ulLen );
            ( ( char * ) pFileName->szName )[ ulLen ] = '\0';
         }
         if( pFileName->szExtension )
         {
            ulLen                                           = strlen( pFileName->szExtension );
            ulLen                                           = hb_strRTrimLen( pFileName->szExtension, ulLen, FALSE );
            pFileName->szExtension                          = hb_strLTrim( pFileName->szExtension, &ulLen );
            ( ( char * ) pFileName->szExtension )[ ulLen ]  = '\0';
         }
      }

      /* FILECASE */
      if( iFileCase == HB_SET_CASE_LOWER )
      {
         if( pFileName->szName )
            hb_strLower( ( char * ) pFileName->szName, strlen( pFileName->szName ) );
         if( pFileName->szExtension )
            hb_strLower( ( char * ) pFileName->szExtension, strlen( pFileName->szExtension ) );
      }
      else if( iFileCase == HB_SET_CASE_UPPER )
      {
         if( pFileName->szName )
            hb_strUpper( ( char * ) pFileName->szName, strlen( pFileName->szName ) );
         if( pFileName->szExtension )
            hb_strUpper( ( char * ) pFileName->szExtension, strlen( pFileName->szExtension ) );
      }

      /* DIRCASE */
      if( pFileName->szPath )
      {
         if( iDirCase == HB_SET_CASE_LOWER )
            hb_strLower( ( char * ) pFileName->szPath, strlen( pFileName->szPath ) );
         else if( iDirCase == HB_SET_CASE_UPPER )
            hb_strUpper( ( char * ) pFileName->szPath, strlen( pFileName->szPath ) );
      }

      hb_fsFNameMerge( ( char * ) szFileName, pFileName );
      hb_xfree( pFileName );

      if( pszCP )
      {
         const char * pszPrev = szFileName;
         ulLen       = HB_PATH_MAX;
         szFileName  = hb_osEncodeCP( szFileName, pszFree, &ulLen );
         if( pszFree == NULL && szFileName != pszPrev )
         {
            hb_strncpy( ( char * ) pszPrev, szFileName, HB_PATH_MAX - 1 );
            hb_xfree( ( void * ) szFileName );
            szFileName = pszPrev;
         }
      }
   }

   return szFileName;
}

#ifdef HB_LEGACY_LEVEL

BYTE * hb_fileNameConv( char * szFileName )
{
   BOOL     fFree;
   BYTE *   szNew;

   szNew = hb_fsNameConv( ( BYTE * ) szFileName, &fFree );
   if( fFree )
   {
      hb_strncpy( szFileName, ( char * ) szNew, strlen( szFileName ) );
      hb_xfree( szNew );
   }

   return ( BYTE * ) szFileName;
}

#endif

static BOOL hb_fsDisableWaitLocks( int iSet )
{
   BOOL fRetVal = s_fUseWaitLocks;

   if( iSet >= 0 )
      s_fUseWaitLocks = ( iSet == 0 );

   return fRetVal;
}

HB_FUNC( HB_DISABLEWAITLOCKS )
{
   HB_THREAD_STUB

   hb_retl( hb_fsDisableWaitLocks( ISLOG( 1 ) ? ( hb_parl( 1 ) ? 1 : 0 ) : -1 ) );
}

HB_FUNC( DISABLEWAITLOCKS )
{
   HB_FUNC_EXEC( HB_DISABLEWAITLOCKS );
}
