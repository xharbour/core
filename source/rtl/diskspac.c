/*
 * $Id: diskspac.c 9858 2012-12-06 01:40:02Z druzus $
 */

/*
 * Harbour Project source code:
 * DISKSPACE() function
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

/* NOTE: DISKSPACE() supports larger disks than 2GB. CA-Cl*pper will always
         return a (LONG) value, Harbour may return a (double) for large
         values, the decimal places are always set to zero, though. */

/* NOTE: For OS/2. Must be ahead of any and all #include statements */
#define INCL_BASE
#define INCL_DOSERRORS

#define HB_OS_WIN_USED

#include "hbapi.h"
#include "hbapierr.h"
#include "hbapifs.h"

#if defined( HB_OS_BSD )
#  include <sys/param.h>
#  include <sys/mount.h>
#elif defined( HB_OS_SUNOS )
#  include <sys/statvfs.h>
#elif defined( HB_OS_UNIX )
#  if defined( __WATCOMC__ )
#     include <sys/stat.h>
#  else
#     include <sys/vfs.h>
#  endif
#endif

HB_FUNC( DISKSPACE )
{
   USHORT   uiDrive  = ISNUM( 1 ) ? ( USHORT ) hb_parni( 1 ) : 0;
   double   dSpace   = 0.0;
   BOOL     bError   = FALSE;

#if defined( HB_OS_DOS )

   {
      union REGS regs;

      regs.HB_XREGS.dx  = uiDrive;
      regs.h.ah         = 0x36;
      HB_DOS_INT86( 0x21, &regs, &regs );

      if( regs.HB_XREGS.ax != 0xFFFF )
         dSpace = ( double ) regs.HB_XREGS.bx *
                  ( double ) regs.HB_XREGS.ax *
                  ( double ) regs.HB_XREGS.cx;
      else
         bError = TRUE;
   }

#elif defined( HB_OS_WIN )

   {
      typedef BOOL ( WINAPI * P_GDFSE )( LPCTSTR, PULARGE_INTEGER,
                                         PULARGE_INTEGER, PULARGE_INTEGER );

      char     szPath[ 4 ];
      P_GDFSE  pGetDiskFreeSpaceEx;
      UINT     uiErrMode;

      /* Get the default drive */

      if( uiDrive == 0 )
      {
         USHORT uiErrorOld = hb_fsError();

         uiDrive = hb_fsCurDrv() + 1;

         hb_fsSetError( uiErrorOld );
      }

      szPath[ 0 ] = ( char ) ( uiDrive + 'A' - 1 );
      szPath[ 1 ] = ':';
      szPath[ 2 ] = '\\';
      szPath[ 3 ] = '\0';

      uiErrMode   = SetErrorMode( SEM_FAILCRITICALERRORS );

      SetLastError( 0 );

      pGetDiskFreeSpaceEx = ( P_GDFSE ) GetProcAddress( GetModuleHandle( "kernel32.dll" ),
                                                        "GetDiskFreeSpaceExA" );

      if( pGetDiskFreeSpaceEx )
      {
         ULARGE_INTEGER i64FreeBytesToCaller,
                        i64TotalBytes,
                        i64FreeBytes,
                        i64RetVal;

         if( pGetDiskFreeSpaceEx( szPath,
                                  ( PULARGE_INTEGER ) &i64FreeBytesToCaller,
                                  ( PULARGE_INTEGER ) &i64TotalBytes,
                                  ( PULARGE_INTEGER ) &i64FreeBytes ) )
         {
            HB_MEMCPY( &i64RetVal, &i64FreeBytesToCaller, sizeof( ULARGE_INTEGER ) );

            #if ( defined( __GNUC__ ) || defined( _MSC_VER ) || defined( __LCC__ ) ) && ! defined( __RSXNT__ )

            dSpace = ( double ) i64RetVal.LowPart +
                     ( double ) i64RetVal.HighPart +
                     ( double ) i64RetVal.HighPart *
                     ( double ) 0xFFFFFFFF;

            #else

            /* NOTE: Borland doesn't seem to deal with the un-named
                     struct that is part of ULARGE_INTEGER
                     [pt] */

            dSpace = ( double ) i64RetVal.u.LowPart +
                     ( double ) i64RetVal.u.HighPart +
                     ( double ) i64RetVal.u.HighPart *
                     ( double ) 0xFFFFFFFF;

            #endif
         }
      }
      else
      {
         DWORD dwSectorsPerCluster;
         DWORD dwBytesPerSector;
         DWORD dwNumberOfFreeClusters;
         DWORD dwTotalNumberOfClusters;

         SetLastError( 0 );

         if( GetDiskFreeSpace( szPath,
                               &dwSectorsPerCluster,
                               &dwBytesPerSector,
                               &dwNumberOfFreeClusters,
                               &dwTotalNumberOfClusters ) )
            dSpace = ( double ) dwNumberOfFreeClusters *
                     ( double ) dwSectorsPerCluster *
                     ( double ) dwBytesPerSector;
      }

      SetErrorMode( uiErrMode );

      if( GetLastError() != 0 )
         bError = TRUE;
   }

#elif defined( HB_OS_OS2 )

   {
      struct _FSALLOCATE fsa;

      /* Query level 1 info from filesystem */
      if( DosQueryFSInfo( uiDrive, 1, &fsa, sizeof( fsa ) ) == 0 )
         dSpace = ( double ) fsa.cUnitAvail *
                  ( double ) fsa.cSectorUnit *
                  ( double ) fsa.cbSector;
      else
         bError = TRUE;
   }

#elif defined( HB_OS_UNIX )

   {
      const char *szName = ISCHAR( 1 ) ? hb_parc( 1 ) : "/";

#if defined( __WATCOMC__ )
      struct stat st;
      if( stat( szName, &st ) == 0 )
         dSpace = ( double ) st.st_blocks * ( double ) st.st_blksize;
#else
#  if defined( HB_OS_SUNOS )
      struct statvfs st;
      if( statvfs( szName, &st ) == 0 )
#  else
      struct statfs  st;
      if( statfs( szName, &st ) == 0 )
#  endif
         dSpace = ( double ) st.f_blocks * ( double ) st.f_bsize;
#endif
      else
         bError = TRUE;

      HB_SYMBOL_UNUSED( uiDrive );
   }

#else

   {
      HB_SYMBOL_UNUSED( uiDrive );
   }

#endif

   if( bError )
      hb_errRT_BASE_Ext1( EG_OPEN, 2018, NULL, NULL, 0, EF_CANDEFAULT, 1, hb_paramError( 1 ) );

   hb_retnlen( dSpace, -1, 0 );
}
