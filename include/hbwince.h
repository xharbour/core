/*
 * $Id: hbwince.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 *
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

#ifndef HB_OS_WIN_CE_H_
#define HB_OS_WIN_CE_H_

#if defined(HB_OS_WIN)

HB_EXTERN_BEGIN

#if defined(HB_OS_WIN_CE)
#  undef  HB_OS_HAS_DRIVE_LETTER

/* defined(__CEGCC__) || defined(__MINGW32CE__) */

#if defined(__MINGW32CE__) && 0
typedef long clock_t;
extern clock_t clock( void );
#endif

extern int remove( const char *filename );
extern int system( const char *string );
extern char *strerror( int errnum );

#if defined( HB_OS_WIN_USED ) && defined( _MSC_VER )

   #ifndef MAX_COMPUTERNAME_LENGTH
      #define MAX_COMPUTERNAME_LENGTH           31
      #define SEM_FAILCRITICALERRORS            0x0001
      #define FILE_TYPE_CHAR                    0x0002
      #define STD_INPUT_HANDLE                  ((DWORD)-10)
      #define STD_OUTPUT_HANDLE                 ((DWORD)-11)
      #define STD_ERROR_HANDLE                  ((DWORD)-12)
      #define LOCKFILE_FAIL_IMMEDIATELY         0x00000001
      #define LOCKFILE_EXCLUSIVE_LOCK           0x00000002
      #define OEM_FIXED_FONT                    SYSTEM_FONT
      #define WM_NCMOUSEMOVE                    0x00A0
      #define WM_QUERYENDSESSION                0x0011
      #define WM_ENTERIDLE                      0x0121
      #define SM_CMOUSEBUTTONS                  43
      #define PROOF_QUALITY                     2
      #define LR_LOADFROMFILE                   0x0010
      #ifndef DRIVE_UNKNOWN
         #define DRIVE_UNKNOWN                     0
      #endif
   #endif

   DWORD WINAPI GetEnvironmentVariableA( LPCSTR name, LPSTR value, DWORD size );
   LPSTR WINAPI GetEnvironmentStringsA( void );
   BOOL WINAPI GetProcessTimes( HANDLE hprocess,
                                LPFILETIME lpCreationTime, LPFILETIME lpExitTime,
                                LPFILETIME lpKernelTime, LPFILETIME lpUserTime );
   BOOL WINAPI GetUserNameA( LPSTR buffer, LPDWORD len );
   BOOL WINAPI GetComputerNameA( LPSTR buffer, LPDWORD len );
   DWORD WINAPI GetCurrentDirectoryA( DWORD len, LPSTR buffer );
   BOOL WINAPI SetCurrentDirectoryA( LPCSTR dirname );
   BOOL WINAPI LockFile( HANDLE hFile,
                         DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
                         DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh );
   BOOL WINAPI LockFileEx( HANDLE hFile,
                           DWORD dwFlags, DWORD dwReserved,
                           DWORD nNumberOfBytesToLockLow,
                           DWORD nNumberOfBytesToLockHigh, LPOVERLAPPED lpOverlapped );
   BOOL WINAPI UnlockFile( HANDLE hFile,
                           DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh,
                           DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh );
   BOOL WINAPI UnlockFileEx( HANDLE hFile, DWORD dwReserved,
                             DWORD nNumberOfBytesToUnlockLow,
                             DWORD nNumberOfBytesToUnlockHigh, LPOVERLAPPED lpOverlapped );
   BOOL WINAPI GetVolumeInformationA( LPCSTR p1, LPSTR p2, DWORD p3, PDWORD p4,
                                      PDWORD p5, PDWORD p6, LPSTR p7, DWORD p8 );
   UINT WINAPI SetErrorMode( UINT mode );
   HANDLE WINAPI CreateFileA( LPCSTR filename, DWORD access,
                       DWORD sharing, LPSECURITY_ATTRIBUTES sa,
                       DWORD creation, DWORD attributes, HANDLE tmplt );
   BOOL WINAPI MoveFileA( LPCSTR fn1, LPCSTR fn2 );
   BOOL WINAPI DeleteFileA( LPCSTR path );
   BOOL WINAPI RemoveDirectoryA( LPCSTR path );
   BOOL WINAPI CreateDirectoryA( LPCSTR path, LPSECURITY_ATTRIBUTES attr );
   BOOL WINAPI SetFileAttributesA( LPCSTR filename, DWORD attr );
   BOOL WINAPI CharToOemBuffA( LPCSTR src, LPSTR dst, DWORD len );
   BOOL WINAPI OemToCharBuffA( LPCSTR src, LPSTR dst, DWORD len );
   HANDLE WINAPI FindFirstFileA( LPCSTR path, WIN32_FIND_DATAA * data );
   BOOL WINAPI FindNextFileA( HANDLE handle, WIN32_FIND_DATAA * data );
   BOOL WINAPI GetVersionExA( OSVERSIONINFOA * v );
   HANDLE WINAPI GetStdHandle( DWORD nStdHandle );
   DWORD WINAPI GetFileType( HANDLE handle );
   HMODULE WINAPI GetModuleHandleA( LPCSTR modulename );
   HINSTANCE WINAPI LoadLibraryA( LPCSTR libname );
   DWORD WINAPI GetTempPathA( DWORD size, LPSTR buffer );
   UINT WINAPI GetTempFileNameA( LPCSTR tmpdir, LPCSTR prefix, UINT unique, LPSTR filename );
   BOOL WINAPI GetDiskFreeSpaceA( LPCSTR path, PDWORD pdwSectorsPerCluster,
                                  PDWORD pdwBytesPerSector,
                                  PDWORD pdwNumberOfFreeClusters, PDWORD pdwTotalNumberOfClusters );
   BOOL WINAPI Beep( DWORD dwFreq, DWORD dwDurat );
   int WINAPI SetTextCharacterExtra( HDC hdc, int i );
   BOOL WINAPI GetKeyboardState( PBYTE p );
   BOOL WINAPI SetKeyboardState( PBYTE p );

   int WINAPI FrameRect( HDC hDC, CONST RECT *lprc, HBRUSH hbr );
   BOOL WINAPI FloodFill( HDC hdc, int x, int y, COLORREF color);
   BOOL  WINAPI Arc( HDC hdc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

   #if defined( __POCC__ ) || defined( __XCC__ )
      #ifndef GlobalAlloc
      #define GlobalAlloc(flags, cb)      LocalAlloc(flags, cb)
      #endif
      #ifndef GlobalLock
      #define GlobalLock(lp)              LocalLock(lp)
      #endif
      #ifndef GlobalUnlock
      #define GlobalUnlock(lp)            LocalUnlock(lp)
      #endif
      #ifndef GlobalSize
      #define GlobalSize(lp)              LocalSize(lp)
      #endif
      #ifndef GlobalFree
      #define GlobalFree(h)               LocalFree(h)
      #endif
      #ifndef GlobalReAlloc
      #define GlobalReAlloc(h, cb, flags) LocalReAlloc(h, cb, flags)
      #endif
      #ifndef GlobalHandle
      #define GlobalHandle(lp)            LocalHandle(lp)
      #endif
      #ifndef GlobalFlags
      #define GlobalFlags(lp)             LocalFlags(lp)
   #endif
   #endif
#endif /* HB_OS_WIN_USED && _MSC_VER */

#endif /* HB_OS_WIN_CE */

extern wchar_t * hb_mbtowc( const char *srcA );
extern char * hb_wctomb( const wchar_t *srcW );
extern wchar_t * hb_mbntowc( const char *srcA, unsigned long ulLen );
extern char * hb_wcntomb( const wchar_t *srcW, unsigned long ulLen );
extern void hb_mbtowccpy( wchar_t *dstW, const char *srcA, unsigned long ulLen );
extern void hb_mbtowcset( wchar_t *dstW, const char *srcA, unsigned long ulLen );
extern void hb_wctombget( char *dstA, const wchar_t *srcW, unsigned long ulLen );

#if defined(UNICODE)

   #define HB_TCHAR_CPTO(d,s,l)        hb_mbtowccpy(d,s,l)
   #define HB_TCHAR_GETFROM(d,s,l)     hb_wctombget(d,s,l)
   #define HB_TCHAR_SETTO(d,s,l)       hb_mbtowcset(d,s,l)
   #define HB_TCHAR_CONVTO(s)          hb_mbtowc(s)
   #define HB_TCHAR_CONVFROM(s)        hb_wctomb(s)
   #define HB_TCHAR_CONVNTO(s,l)       hb_mbntowc(s,l)
   #define HB_TCHAR_CONVNFROM(s,l)     hb_wcntomb(s,l)
   #define HB_TCHAR_CONVNREV(d,s,l)    do { hb_wctombget(d,s,l); hb_xfree(s); } while( 0 )
   #define HB_TCHAR_FREE(s)            hb_xfree(s)

#else

   #define HB_TCHAR_CPTO(d,s,l)        hb_strncpy(d,s,l)
   #define HB_TCHAR_SETTO(d,s,l)       memcpy(d,s,l)
   #define HB_TCHAR_GETFROM(d,s,l)     memcpy(d,s,l)
   #define HB_TCHAR_CONVTO(s)          ((char *)(s))
   #define HB_TCHAR_CONVFROM(s)        ((char *)(s))
   #define HB_TCHAR_CONVNTO(s,l)       ((char *)(s))
   #define HB_TCHAR_CONVNFROM(s,l)     ((char *)(s))
   #define HB_TCHAR_CONVNREV(d,s,l)    do { ; } while( 0 )
   #define HB_TCHAR_FREE(s)            HB_SYMBOL_UNUSED(s)

#endif /* UNICODE */

HB_EXTERN_END

#endif /* HB_OS_WIN */

#endif /* HB_OS_WIN_CE_H_ */
