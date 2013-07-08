/*
 * $Id: hbgete.c 9949 2013-04-10 08:11:11Z zsaulius $
 */

/*
 * Harbour Project source code:
 * environment variables access
 *
 * Copyright 2001-2002 Antonio Linares <alinares@fivetech.com>
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

/* NOTE: Notice that this code is needed as ANSI C getenv() crashes
         so badly when used from a Windows DLL. */

/* For OS/2 */
#define INCL_DOSMISC

#define HB_OS_WIN_USED

#include "hbapi.h"
#include "hbexemem.h"

#if defined( __HB_COMPILER__ )
   #include "hbcomp.h"
#endif

/* NOTE: Warning, this function _may_ return NULL as a result if
         the environment variable reading fails form some reason.
         If the return value is not NULL, the caller must free
         the pointer. */

char * hb_getenv( const char * szName )
{
   char * pszBuffer = NULL;

#if defined( HB_OS_WIN )

   {
      DWORD size = GetEnvironmentVariable( szName, NULL, 0 );

      if( size != 0 )
      {
         pszBuffer = ( char * ) hb_xgrab( size );
         GetEnvironmentVariable( szName, pszBuffer, size );
      }
   }

#elif defined( HB_OS_OS2 )

   {
      PSZ EnvValue = "";

      if( DosScanEnv( szName, &EnvValue ) == NO_ERROR )
      {
         pszBuffer = ( char * ) hb_xgrab( strlen( EnvValue ) + 1 );
         strcpy( pszBuffer, ( char * ) EnvValue );
      }
   }

#else

   {
      char * pszTemp = getenv( szName );

      if( pszTemp != NULL )
      {
         pszBuffer = ( char * ) hb_xgrab( strlen( pszTemp ) + 1 );
         strcpy( pszBuffer, pszTemp );
      }
   }

#endif

   return pszBuffer;
}


#define SYS_ENVVARS      TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" )
#define USER_ENV_SUBKEY  TEXT( "Environment" )

/* Sets the contents of the specified environment variable for the current process/system
*/
HB_BOOL hb_setenv( const char * szName, const char * szValue, HB_BOOL fSys )
{

#if defined( HB_OS_WIN )
   HB_BOOL fSuccess = SetEnvironmentVariable( szName, szValue );

   if( fSuccess && fSys )
   {
      HKEY hKey;

      fSuccess = HB_FALSE;
      if( szValue && szValue[ 0 ] == '\0' )
         szValue = NULL;

      if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, SYS_ENVVARS, 0, KEY_READ | KEY_WRITE, &hKey ) == ERROR_SUCCESS )
      {
         HB_BOOL fRet = RegQueryValueEx( hKey, szName, NULL, NULL, NULL, NULL );

         if( fRet == ERROR_SUCCESS || ( fRet == ERROR_FILE_NOT_FOUND && szValue ) )
         {
            if( szValue )
               RegSetValueEx( hKey, szName, 0, REG_SZ, ( LPBYTE ) szValue, strlen( szValue ) + 1 );
            else
               RegDeleteValue( hKey, szName );

            SendMessage( HWND_BROADCAST, WM_SETTINGCHANGE, (WPARAM) NULL, (LPARAM) USER_ENV_SUBKEY );
            fSuccess = HB_TRUE;
         }
         RegCloseKey( hKey );
      }
   }

   return fSuccess;

#else

   HB_SYMBOL_UNUSED( szName );
   HB_SYMBOL_UNUSED( szValue );
   HB_SYMBOL_UNUSED( fSys );

   return HB_FALSE;

#endif

}
