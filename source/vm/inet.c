/*
 * $Id: inet.c 9922 2013-02-07 12:02:27Z druzus $
 */

/*
 * xHarbour Project source code:
 * The internet protocol / TCP support
 *
 * Copyright 2002 Giancarlo Niccolai [gian@niccolai.ws]
 *                Ron Pinkas [Ron@RonPinkas.com]
 *                Marcelo Lombardo [marcelo.lombardo@newage-software.com.br]
 * www - http://www.xharbour.org
 *
 * Copyright 2007 Miguel Angel marchuet <miguelangel@marchuet.net>
 *    added dinamic system buffer
 * www - http://www.xharbour.org
 *
 * this program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
 * GNU General public License for more details.
 *
 * You should have received a copy of the GNU General public License
 * along with this software; see the file COPYING.  if not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, xHarbour license gives permission for
 * additional uses of the text contained in its release of xHarbour.
 *
 * The exception is that, if you link the xHarbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General public License.
 * Your use of that executable is in no way restricted on account of
 * linking the xHarbour library code into it.
 *
 * this exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General public License.
 *
 * this exception applies only to the code released with this xHarbour
 * explicit exception.  if you add/copy code from other sources,
 * as the General public License permits, the above exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * if you write modifications of your own for xHarbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * if you do not wish that, delete this exception notice.
 *
 */

#define _HB_API_INTERNAL_
#include "hbapi.h"
#include "hbapiitm.h"
#include "inet.h"
#include "hbstack.h"
#include <fcntl.h>
#include <errno.h>

#if defined( HB_OS_UNIX ) || defined( HB_OS_UNIX ) || defined( HB_OS_BSD ) || defined( HB_OS_OS2 )
   #include <sys/time.h>
#endif

#if defined( HB_OS_OS2 )
   #include <sys/socket.h>
   #include <sys/select.h>
   #include <sys/ioctl.h>
#endif

#if ( defined( __EXPORT__ ) && defined( HB_VM_ALL ) ) || defined( HB_THREAD_SUPPORT )
   HB_EXTERN_BEGIN
   extern HB_STACK *        _TlsGetValue( void );
   HB_EXTERN_END
   #define TlsGetValue( x ) _TlsGetValue()
#endif

#if defined( HB_OS_OS2 ) || defined( HB_OS_WIN )
/* NET_SIZE_T exists because of shortsightedness on the POSIX committee.  BSD
 * systems used "int *" as the parameter to accept(), getsockname(),
 * getpeername() et al.  Consequently many unixes took an int * for that
 * parameter.  The POSIX committee decided that "int" was just too generic and
 * had to be replaced with size_t almost everywhere.  There's no problem with
 * that when you're passing by value.  But when you're passing by reference
 * this creates a gross source incompatibility with existing programs.  On
 * 32-bit architectures it creates only a warning.  On 64-bit architectures it
 * creates broken code -- because "int *" is a pointer to a 64-bit quantity and
 * "size_t *" is frequently a pointer to a 32-bit quantity.
 *
 * Some Unixes adopted "size_t *" for the sake of POSIX compliance.  Others
 * ignored it because it was such a broken interface.  Chaos ensued.  POSIX
 * finally woke up and decided that it was wrong and created a new type
 * socklen_t.  The only useful value for socklen_t is int, and that's how
 * everyone who has a clue implements it.  It is almost always the case that
 * NET_SIZE_T should be defined to be an int, unless the system being compiled
 * for was created in the window of POSIX madness.
 */
   #define socklen_t int
#endif

#if defined( HB_OS_HPUX )
char * hstrerror( int ierr )
{
   return sprintf( "error %i", ierr );
}
#endif

#ifdef HB_OS_LINUX
#include <signal.h>
#define HB_INET_LINUX_INTERRUPT SIGUSR1 + 90
static void hb_inetLinuxSigusrHandle( int sig )
{
   /* nothing to do */
   HB_SYMBOL_UNUSED( sig );
}
#endif

/* some compilers has missing this define */
#ifndef SOCKET_ERROR
#define SOCKET_ERROR ( -1 )
#endif

/* add parens to avoid warning */
#if defined(__BORLANDC__) && (__BORLANDC__<=0x620)
   #undef  MAKEWORD
   #define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | (((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8)))
#endif

#ifndef HB_NO_DEFAULT_INET
/* JC1: we need it volatile to be minimally thread safe. */
static volatile int s_iSessions = 0;

/* Useful utility function to have a timeout; */

static int hb_selectReadSocket( HB_SOCKET_STRUCT * Socket )
{
   fd_set         set;
   struct timeval tv;

   FD_ZERO( &set );
   FD_SET( Socket->com, &set );

   if( Socket->timeout == -1 )
   {
      if( select( ( int ) Socket->com + 1, &set, NULL, NULL, NULL ) < 0 )
         return 0;
   }
   else
   {
      tv.tv_sec   = Socket->timeout / 1000;
      tv.tv_usec  = ( Socket->timeout % 1000 ) * 1000;
      if( select( ( int ) Socket->com + 1, &set, NULL, NULL, &tv ) < 0 )
         return 0;
   }

   return FD_ISSET( Socket->com, &set );
}

static int hb_selectWriteSocket( HB_SOCKET_STRUCT * Socket )
{
   fd_set         set;
   struct timeval tv;

   FD_ZERO( &set );
   FD_SET( Socket->com, &set );

   if( Socket->timeout == -1 )
   {
      if( select( ( int ) Socket->com + 1, NULL, &set, NULL, NULL ) < 0 )
         return 0;
   }
   else
   {
      tv.tv_sec   = Socket->timeout / 1000;
      tv.tv_usec  = ( Socket->timeout % 1000 ) * 1000;
      if( select( ( int ) Socket->com + 1, NULL, &set, NULL, &tv ) < 0 )
         return 0;
   }

   return FD_ISSET( Socket->com, &set );
}

int hb_selectWriteExceptSocket( HB_SOCKET_STRUCT * Socket )
{
   fd_set         set, eset;
   struct timeval tv;

   FD_ZERO( &set );
   FD_SET( Socket->com, &set );
   FD_ZERO( &eset );
   FD_SET( Socket->com, &eset );

   if( Socket->timeout == -1 )
   {
      if( select( ( int ) Socket->com + 1, NULL, &set, &eset, NULL ) < 0 )
         return 2;
   }
   else
   {
      tv.tv_sec   = Socket->timeout / 1000;
      tv.tv_usec  = ( Socket->timeout % 1000 ) * 1000;
      if( select( ( int ) Socket->com + 1, NULL, &set, &eset, &tv ) < 0 )
         return 2;
   }

   if( FD_ISSET( Socket->com, &eset ) )
      return 2;

   if( FD_ISSET( Socket->com, &set ) )
      return 1;

   return 0;
}

/*** Utilty to access host DNS */
static struct hostent * hb_getHosts( const char * name, HB_SOCKET_STRUCT * Socket )
{
   struct hostent * Host = NULL;

   /* let's see if name is an IP address; not necessary on linux */
#if defined( HB_OS_WIN ) || defined( HB_OS_OS2 )
   ULONG ulAddr;

   ulAddr = inet_addr( name );
   if( ulAddr == INADDR_NONE )
   {
      if( strcmp( "255.255.255.255", name ) == 0 )
         Host = gethostbyaddr( ( const char * ) &ulAddr, sizeof( ulAddr ), AF_INET );
   }
   else
      Host = gethostbyaddr( ( const char * ) &ulAddr, sizeof( ulAddr ), AF_INET );
#endif

   if( Host == NULL )
      Host = gethostbyname( name );

   if( Host == NULL && Socket != NULL )
   {
#if defined( HB_OS_WIN )
      HB_SOCKET_SET_ERROR2( Socket, WSAGetLastError(), "Generic error in GetHostByName()" );
      WSASetLastError( 0 );
#elif defined( HB_OS_OS2 ) || defined( HB_OS_HPUX ) || defined( __WATCOMC__ )
      HB_SOCKET_SET_ERROR2( Socket, h_errno, "Generic error in GetHostByName()" );
#else
      HB_SOCKET_SET_ERROR2( Socket, h_errno, ( char * ) hstrerror( h_errno ) );
#endif
   }
   return Host;
}

/*** Setup the non-blocking method **/

static void hb_socketSetNonBlocking( HB_SOCKET_STRUCT * Socket )
{
#ifdef HB_OS_WIN
   ULONG mode = 1;
   ioctlsocket( Socket->com, FIONBIO, &mode );

#else
   int flags = fcntl( Socket->com, F_GETFL, 0 );
   if( flags != -1 )
   {
      flags |= O_NONBLOCK;
      fcntl( Socket->com, F_SETFL, ( LONG ) flags );
   }
#endif
}

/*** Setup the blocking method **/

static void hb_socketSetBlocking( HB_SOCKET_STRUCT * Socket )
{
#ifdef HB_OS_WIN
   ULONG mode  = 0;
   ioctlsocket( Socket->com, FIONBIO, &mode );
#else
   int   flags = fcntl( Socket->com, F_GETFL, 0 );
   if( flags != -1 )
   {
      flags &= ~O_NONBLOCK;
      fcntl( Socket->com, F_SETFL, ( long ) flags );
   }
#endif
}

/*** Utility to connect to a defined remote address ***/

static int hb_socketConnect( HB_SOCKET_STRUCT * Socket )
{
   int         iErr1;

#if ! defined( HB_OS_WIN )
   int         iErrval;
   socklen_t   iErrvalLen;
#endif
   int         iOpt = 1;

   setsockopt( Socket->com, SOL_SOCKET, SO_KEEPALIVE, ( const char * ) &iOpt, sizeof( iOpt ) );

   /* we'll be using a nonblocking function */
   hb_socketSetNonBlocking( Socket );

   iErr1 = connect( Socket->com, ( struct sockaddr * ) &Socket->remote, sizeof( Socket->remote ) );
   if( iErr1 != 0 )
   {
#if defined( HB_OS_WIN )
      if( WSAGetLastError() != WSAEWOULDBLOCK )
#else
      if( errno != EINPROGRESS )
#endif
      {
         HB_SOCKET_SET_ERROR( Socket );
      }
      else
      {
         /* Now we wait for socket connection or timeout */
#if defined( HB_OS_WIN )
         iErr1 = hb_selectWriteExceptSocket( Socket );

         if( iErr1 == 2 )
         {
            HB_SOCKET_SET_ERROR2( Socket, 2, "Connection failed" );
         }
         else if( iErr1 == 1 )
         {
            /* success */
         }
#else
         if( hb_selectWriteSocket( Socket ) )
         {
            /* Connection has been completed with a failure or a success */
            iErrvalLen  = sizeof( iErrval );
            iErr1       = getsockopt( Socket->com,
                                      SOL_SOCKET,
                                      SO_ERROR,
                                      ( void * ) &iErrval,
                                      &iErrvalLen
                                      );

            if( iErr1 )
            {
               HB_SOCKET_SET_ERROR1( Socket, iErr1 );
            }
            else if( iErrval )
            {
               HB_SOCKET_SET_ERROR1( Socket, iErrval );
            }
            /* Success! */
         }
#endif
         /* Timed out */
         else
         {
            HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
         }

         /*
          * Read real buffer sizes from socket
          */
         {
            int         value;
            socklen_t   len = sizeof( value );

            if( getsockopt( Socket->com, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, &len ) != SOCKET_ERROR )
            {
               Socket->iSndBufSize = value;

               if( getsockopt( Socket->com, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, &len ) != SOCKET_ERROR )
                  Socket->iRcvBufSize = value;
               else
                  Socket->iRcvBufSize = 1400;
            }
            else
            {
               Socket->iSndBufSize  = 1400;
               Socket->iRcvBufSize  = 1400;
            }
         }
      }
   }

   hb_socketSetBlocking( Socket );

   return Socket->errorCode == 0;
}

static HB_GARBAGE_FUNC( hb_inetSocketFinalize )
{
   HB_SOCKET_STRUCT * Socket = ( HB_SOCKET_STRUCT * ) Cargo;

   if( Socket->sign != HB_SOCKET_SIGN )
   {
      hb_errInternal( HB_EI_MEMCORRUPT,
                      "hb_inetSocketFinalize: Corrupted socket item at 0x%p",
                      ( char * ) Socket, NULL );
      return;
   }

   if( Socket->com > 0 )
   {
#if defined( HB_OS_WIN )
      shutdown( Socket->com, SD_BOTH );
#elif defined( HB_OS_OS2 )
      shutdown( Socket->com, SO_RCV_SHUTDOWN + SO_SND_SHUTDOWN );
#elif ! defined( __WATCOMC__ )
      shutdown( Socket->com, SHUT_RDWR );
#endif

      HB_INET_CLOSE( Socket->com );
   }

   if( Socket->caPeriodic != NULL )
   {
      hb_itemRelease( Socket->caPeriodic );
      Socket->caPeriodic = NULL;
   }
}

/*****************************************************
 * Socket Initialization
 ***/

HB_FUNC( INETINIT )
{
   if( s_iSessions )
      s_iSessions++;
   else
   {
#if defined( HB_OS_WIN )
      WSADATA wsadata;
      WSAStartup( MAKEWORD( 1,1 ), &wsadata );
#elif defined( HB_OS_LINUX )
      signal( HB_INET_LINUX_INTERRUPT, hb_inetLinuxSigusrHandle );
#endif
      s_iSessions = 1;
   }
}

HB_FUNC( INETCLEANUP )
{
   if( --s_iSessions == 0 )
   {
#if defined( HB_OS_WIN )
      WSACleanup();
#endif
   }
}

/*****************************************************
 * Socket Creation and destruction
 ***/

HB_FUNC( INETCREATE )
{
   PHB_ITEM             pSocket = NULL;
   HB_SOCKET_STRUCT *   Socket;

   HB_SOCKET_INIT( Socket, pSocket );

   if( ISNUM( 1 ) )
      Socket->timeout = hb_parni( 1 );

   hb_itemReturnRelease( pSocket );
}

HB_FUNC( INETCLOSE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCLOSE", 1, hb_paramError( 1 ) );
   else if( Socket->com )
   {
#if defined( HB_OS_WIN )
      shutdown( Socket->com, SD_BOTH );
#elif defined( HB_OS_OS2 )
      shutdown( Socket->com, SO_RCV_SHUTDOWN + SO_SND_SHUTDOWN );
#elif ! defined( __WATCOMC__ )
      shutdown( Socket->com, SHUT_RDWR );
#endif

      hb_retni( HB_INET_CLOSE( Socket->com ) );

      Socket->com = 0;
#ifdef HB_OS_LINUX
      kill( 0, HB_INET_LINUX_INTERRUPT );
#endif
   }
   else
      hb_retni( -1 );
}

/* Kept for backward compatibility */

HB_FUNC( INETDESTROY )
{

}

/************************************************
 * Socket data access & management
 ***/

HB_FUNC( INETSTATUS )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSTATUS", 1,
                            hb_paramError( 1 ) );
   else
      /* TODO: hb_retni( Socket->status ); */
      hb_retni( Socket->com == 0 ? -1 : 1 );
}

/* Prepared, but still not used; being in wait for comments
 * HB_FUNC( INETSTATUSDESC )
 * {
 * HB_SOCKET_STRUCT *Socket = hb_parptr( 1 );
 *
 * if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
 * {
 *    hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSTATUS", 1,
 *        hb_paramError( 1 ) );
 *    return;
 * }
 *
 * switch( Socket->status )
 * {
 *    case 0: hb_retc( "Connection not opened" ); return;
 *    case 1: hb_retc( "Connection alive" ); return;
 *    case 2: hb_retc( "Last operation error" ); return;
 *    case 3: hb_retc( "Last operation timeout" ); return;
 * }
 * }
 */

HB_FUNC( INETERRORCODE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETERRORCODE", 1,
                            hb_paramError( 1 ) );
   else
      hb_retni( Socket->errorCode );
}

HB_FUNC( INETERRORDESC )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETERRORDESC", 1,
                            hb_paramError( 1 ) );
      return;
   }

   hb_retc( Socket->errorDesc );
}

HB_FUNC( INETCLEARERROR )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCLEARERROR", 1, hb_paramError( 1 ) );
   else
      HB_SOCKET_ZERO_ERROR( Socket );
}

HB_FUNC( INETCOUNT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCOUNT", 1,
                            hb_paramError( 1 ) );
   else
      hb_retni( Socket->count );
}

HB_FUNC( INETADDRESS )
{
   HB_SOCKET_STRUCT *   Socket = HB_PARSOCKET( 1 );
   char *               addr;

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETADDRESS", 1,
                            hb_paramError( 1 ) );
   else
   {
      addr = inet_ntoa( Socket->remote.sin_addr );
      hb_retc( addr );
   }
}

HB_FUNC( INETPORT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETPORT", 1,
                            hb_paramError( 1 ) );
   else
      hb_retni( ntohs( Socket->remote.sin_port ) );
}

HB_FUNC( INETFD )
{
   HB_SOCKET_STRUCT * socket = HB_PARSOCKET( 1 );

   if( socket )
   {
      hb_retnint( socket->com );

      if( hb_parl( 2 ) )
         socket->com = ( HB_SOCKET_T ) -1;
   }
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETFD", 1, hb_paramError( 1 ) );
}

HB_FUNC( INETSETTIMEOUT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket != NULL && Socket->sign == HB_SOCKET_SIGN && ISNUM( 2 ) )
      Socket->timeout = hb_parni( 2 );
   else
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSETTIMEOUT", 2,
                            hb_paramError( 1 ), hb_paramError( 2 ) );
}

HB_FUNC( INETGETTIMEOUT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETTIMEOUT", 1,
                            hb_paramError( 1 ) );
   else
      hb_retni( Socket->timeout );
}


HB_FUNC( INETCLEARTIMEOUT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCLEARTIMEOUT", 1,
                            hb_paramError( 1 ) );
   else
      Socket->timeout = -1;
}

HB_FUNC( INETSETTIMELIMIT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN || ! ISNUM( 2 ) )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSETTIMELIMIT", 2,
                            hb_paramError( 1 ), hb_paramError( 2 ) );
   else
      Socket->timelimit = hb_parnl( 2 );
}

HB_FUNC( INETGETTIMELIMIT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETTIMELIMIT", 1,
                            hb_paramError( 1 ) );
   else
      hb_retni( Socket->timelimit );
}

HB_FUNC( INETCLEARTIMELIMIT )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCLEARTIMELIMIT", 1,
                            hb_paramError( 1 ) );
   else
      Socket->timelimit = -1;
}

HB_FUNC( INETSETPERIODCALLBACK )
{
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 1 );
   PHB_ITEM             pArray   = hb_param( 2, HB_IT_ARRAY );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSETPERIODCALLBACK", 2,
                            hb_paramError( 1 ), hb_paramError( 2 ) );
   else
   {
      if( Socket->caPeriodic )
         hb_itemRelease( Socket->caPeriodic );
      Socket->caPeriodic = hb_arrayClone( pArray, NULL );
   }
}


HB_FUNC( INETGETPERIODCALLBACK )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETPERIODCALLBACK", 1,
                            hb_paramError( 1 ) );
   else
   {
      if( Socket->caPeriodic == NULL )
         hb_ret();
      else
         hb_itemReturn( Socket->caPeriodic );
   }
}

HB_FUNC( INETCLEARPERIODCALLBACK )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCLEARPERIODCALLBACK", 1,
                            hb_paramError( 1 ) );
   else if( Socket->caPeriodic )
   {
      hb_itemRelease( Socket->caPeriodic );
      Socket->caPeriodic = NULL;
   }
}

HB_FUNC( INETGETSNDBUFSIZE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETSNDBUFSIZE", 1,
                            hb_paramError( 1 ) );
   else
   {
      int         value;
      socklen_t   len = sizeof( value );
      getsockopt( Socket->com, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, &len );
      Socket->iSndBufSize = value;
      hb_retni( value );
   }
}

HB_FUNC( INETGETRCVBUFSIZE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETRCVBUFSIZE", 1,
                            hb_paramError( 1 ) );
   else
   {
      int         value;
      socklen_t   len = sizeof( value );
      getsockopt( Socket->com, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, &len );
      Socket->iRcvBufSize = value;
      hb_retni( value );
   }
}

HB_FUNC( INETSETSNDBUFSIZE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSETSNDBUFSIZE", 1, hb_paramError( 1 ) );
   else
   {
      int value = hb_parni( 2 );
      setsockopt( Socket->com, SOL_SOCKET, SO_SNDBUF, ( char * ) &value, sizeof( value ) );
      Socket->iSndBufSize = value;
      hb_retni( value );
   }
}

HB_FUNC( INETSETRCVBUFSIZE )
{
   HB_SOCKET_STRUCT * Socket = HB_PARSOCKET( 1 );

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSETRCVBUFSIZE", 1, hb_paramError( 1 ) );
   else
   {
      int value = hb_parni( 2 );
      setsockopt( Socket->com, SOL_SOCKET, SO_RCVBUF, ( char * ) &value, sizeof( value ) );
      Socket->iRcvBufSize = value;
      hb_retni( value );
   }
}

/********************************************************************
 * TCP receive and send functions
 ***/

static void s_inetRecvInternal( char * szFuncName, int iMode )
{
   HB_SOCKET_STRUCT *   Socket   = ( HB_SOCKET_STRUCT * ) hb_parptr( 1 );
   PHB_ITEM             pBuffer  = hb_param( 2, HB_IT_STRING );
   char *               Buffer;
   int                  iLen, iMaxLen, iReceived, iBufferLen;
   int                  iTimeElapsed;

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN || pBuffer == NULL || ! ISBYREF( 2 ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, szFuncName, 2, hb_paramError( 1 ), hb_paramError( 2 ) );
      return;
   }

   pBuffer  = hb_itemUnShare( pBuffer );
   Buffer   = hb_itemGetCPtr( pBuffer );
   iLen     = ( int ) hb_itemGetCLen( pBuffer );

   if( ISNIL( 3 ) )
      iMaxLen = iLen;
   else
   {
      iMaxLen = hb_parni( 3 );
      if( iLen < iMaxLen )
         iMaxLen = iLen;
   }

   iReceived      = 0;
   iTimeElapsed   = 0;
   HB_SOCKET_ZERO_ERROR( Socket );

   do
   {
      if( iMode == 1 )
         iBufferLen = ( Socket->iRcvBufSize > iMaxLen - iReceived ) ? iMaxLen - iReceived : Socket->iRcvBufSize;
      else
         iBufferLen = iMaxLen;

      HB_STACK_UNLOCK;
      HB_TEST_CANCEL_ENABLE_ASYN;

      if( hb_selectReadSocket( Socket ) )
      {
         iLen = recv( Socket->com, Buffer + iReceived, iBufferLen, MSG_NOSIGNAL );

         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;

         if( iLen > 0 )
            iReceived += iLen;

         /* Called from InetRecv()? */
         if( iMode == 0 )
            break;
      }
      else
      {
         /* timed out; let's see if we have to run a cb routine */
         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;

         iTimeElapsed += Socket->timeout;

         /* if we have a caPeriodic, timeLimit is our REAL timeout */
         if( Socket->caPeriodic != NULL )
         {
            hb_execFromArray( Socket->caPeriodic );

            /* do we continue?
               if( ! hb_itemGetL( &HB_VM_STACK.Return ) || (Socket->timelimit != -1 && iTimeElapsed >= Socket->timelimit ) )
             */
            if( ! hb_itemGetL( hb_stackReturnItem() ) || ( Socket->timelimit != -1 && iTimeElapsed >= Socket->timelimit ) )
            {
               HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" )
               hb_retni( iReceived );
               return;
            }

            /* Declare success to continue loop */
            iLen = 1;
         }
         else /* the timeout has gone, and we have no recovery routine */
         {
            HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" )
            hb_retni( iReceived );
            return;
         }
      }
   }
   while( iReceived < iMaxLen && iLen > 0 );

   Socket->count = iReceived;

   if( iLen == 0 )
   {
      HB_SOCKET_SET_ERROR2( Socket, -2, "Connection closed" );
      hb_retni( iLen );
   }
   else if( iLen < 0 )
   {
      HB_SOCKET_SET_ERROR( Socket );
      hb_retni( iLen );
   }
   else
      /* TraceLog( NULL, "Received: %i Len: %i\n", iReceived, iBufferLen );
       */
      hb_retni( iReceived );
}

HB_FUNC( INETRECV )
{
   s_inetRecvInternal( "INETRECV", 0 );
}


HB_FUNC( INETRECVALL )
{
   s_inetRecvInternal( "INETRECVALL", 1 );
}

static void s_inetRecvPattern( char * szFuncName, char * szPattern )
{
   HB_SOCKET_STRUCT *   Socket      = HB_PARSOCKET( 1 );
   PHB_ITEM             pResult     = hb_param( 2, HB_IT_BYREF );
   PHB_ITEM             pMaxSize    = hb_param( 3, HB_IT_NUMERIC );
   PHB_ITEM             pBufferSize = hb_param( 4, HB_IT_NUMERIC );

   char                 cChar = 0;
   char *               Buffer;
   int                  iAllocated, iBufferSize, iMax;
   int                  iLen  = 0;
   int                  iPos  = 0, iTimeElapsed;
   ULONG                ulPatPos;


   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, szFuncName, 4, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
      return;
   }

   if( pBufferSize )
      iBufferSize = hb_itemGetNI( pBufferSize );
   else
      iBufferSize = 80;

   if( pMaxSize )
      iMax = hb_itemGetNI( pMaxSize );
   else
      iMax = 0;

   HB_SOCKET_ZERO_ERROR( Socket );

   Buffer         = ( char * ) hb_xgrab( iBufferSize );
   iAllocated     = iBufferSize;
   iTimeElapsed   = 0;

   ulPatPos       = 0;
   do
   {
      if( iPos == iAllocated - 1 )
      {
         iAllocated  += iBufferSize;
         Buffer      = ( char * ) hb_xrealloc( Buffer, iAllocated );
      }

      HB_STACK_UNLOCK;
      HB_TEST_CANCEL_ENABLE_ASYN;

      if( hb_selectReadSocket( Socket ) )
      {
         iLen = recv( Socket->com, &cChar, 1, MSG_NOSIGNAL );
         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;
      }
      else
      {
         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;
         iTimeElapsed += Socket->timeout;

         if( Socket->caPeriodic != NULL )
         {
            hb_execFromArray( Socket->caPeriodic );
            /* do we continue?
               if ( hb_itemGetL( &HB_VM_STACK.Return ) &&
             */
            if( hb_itemGetL( hb_stackReturnItem() ) &&
                ( Socket->timelimit == -1 || iTimeElapsed < Socket->timelimit ) )
               continue;
         }

         /* this signals timeout */
         iLen = -2;
      }

      if( iLen > 0 )
      {
         /* verify endsequence recognition automata status */
         if( cChar == szPattern[ ulPatPos ] )
         {
            ulPatPos++;
            if( ! szPattern[ ulPatPos ] )
               break;
         }
         else
            ulPatPos = 0;

         Buffer[ iPos++ ] = cChar;
      }
      else
         break;
   }
   while( iMax == 0 || iPos < iMax );

   if( iLen <= 0 )
   {
      if( pResult )
         hb_itemPutNI( pResult, iLen );

      if( iLen == 0 )
      {
         HB_SOCKET_SET_ERROR2( Socket, -2, "Connection closed" );
      }
      else if( iLen == -2 )
      {
         HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
      }
      else
         HB_SOCKET_SET_ERROR( Socket );

      hb_xfree( ( void * ) Buffer );
      hb_ret();
   }
   else
   {
      if( iMax == 0 || iPos < iMax )
      {
         iPos--;
         Socket->count = iPos;

         if( pResult )
            hb_itemPutNI( pResult, iPos );

         hb_retclenAdopt( Buffer, iPos );
      }
      else
      {
         HB_SOCKET_SET_ERROR2( Socket, -3, "Buffer overrun" );

         if( pResult )
            hb_itemPutNI( pResult, -2 );

         hb_xfree( ( void * ) Buffer );
         hb_retc( NULL );
      }
   }
}

HB_FUNC( INETRECVLINE )
{
   s_inetRecvPattern( "INETRECVLINE", "\r\n" );
}

HB_FUNC( INETRECVENDBLOCK )
{
   HB_SOCKET_STRUCT *   Socket      = HB_PARSOCKET( 1 );
   PHB_ITEM             pProto      = hb_param( 2, HB_IT_ARRAY | HB_IT_STRING );
   PHB_ITEM             pResult     = hb_param( 3, HB_IT_BYREF );
   PHB_ITEM             pMaxSize    = hb_param( 4, HB_IT_NUMERIC );
   PHB_ITEM             pBufferSize = hb_param( 5, HB_IT_NUMERIC );

   char                 cChar          = 0;
   char *               Buffer;
   char **              Proto;
   int                  iAllocated, iBufferSize, iMax;
   int                  iLen;
   int                  iPos           = 0;
   int                  iPosProto;
   int                  iTimeElapsed   = 0;
   int                  iprotos;
   int                  i;
   int *                iprotosize;
   int                  ifindproto = 0;
   BOOL                 bProtoFound;


   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETRECVENDBLOCK", 5,
                            hb_paramError( 1 ), hb_paramError( 2 ),
                            hb_paramError( 3 ), hb_paramError( 4 ), hb_paramError( 5 ) );
      return;
   }

   if( pProto )
   {
      if( HB_IS_ARRAY( pProto ) )
      {
         iprotos = ( int ) hb_arrayLen( pProto );
         if( iprotos <= 0 )
         {
            hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETRECVENDBLOCK", 5,
                                  hb_paramError( 1 ), hb_paramError( 2 ),
                                  hb_paramError( 3 ), hb_paramError( 4 ), hb_paramError( 5 ) );
            return;
         }

         Proto       = ( char ** ) hb_xgrab( sizeof( char * ) * iprotos );
         iprotosize  = ( int * ) hb_xgrab( sizeof( int ) * iprotos );

         for( i = 0; i < iprotos; i++ )
         {
            PHB_ITEM pItem = hb_arrayGetItemPtr( pProto, i + 1 );
            Proto[ i ]        = hb_itemGetCPtr( pItem );
            iprotosize[ i ]   = ( int ) hb_itemGetCLen( pItem );
         }
      }
      else
      {
         Proto             = ( char ** ) hb_xgrab( sizeof( char * ) );
         iprotosize        = ( int * ) hb_xgrab( sizeof( int ) );
         Proto[ 0 ]        = hb_itemGetCPtr( pProto );
         iprotosize[ 0 ]   = ( int ) hb_itemGetCLen( pProto );
         iprotos           = 1;
      }
   }
   else
   {
      Proto             = ( char ** ) hb_xgrab( sizeof( char * ) );
      iprotosize        = ( int * ) hb_xgrab( sizeof( int ) );
      Proto[ 0 ]        = ( char * ) "\r\n";
      iprotos           = 1;
      iprotosize[ 0 ]   = 2;
   }

   iBufferSize = pBufferSize ? hb_itemGetNI( pBufferSize ) : 80;
   iMax        = pMaxSize ? hb_itemGetNI( pMaxSize ) : 0;

   HB_SOCKET_ZERO_ERROR( Socket );

   Buffer      = ( char * ) hb_xgrab( iBufferSize );
   iAllocated  = iBufferSize;

   do
   {
      if( iPos == iAllocated - 1 )
      {
         iAllocated  += iBufferSize;
         Buffer      = ( char * ) hb_xrealloc( Buffer, iAllocated );
      }

      iLen = 0;

      HB_STACK_UNLOCK;
      HB_TEST_CANCEL_ENABLE_ASYN;

      if( hb_selectReadSocket( Socket ) )
      {
         iLen = recv( Socket->com, &cChar, 1, MSG_NOSIGNAL );
         HB_DISABLE_ASYN_CANC;

      }
      else
      {
         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;
         iTimeElapsed += Socket->timeout;
         if( Socket->caPeriodic != NULL )
         {
            hb_execFromArray( Socket->caPeriodic );

            /* if( hb_itemGetL( &HB_VM_STACK.Return ) && */
            if( hb_itemGetL( hb_stackReturnItem() ) &&
                ( Socket->timelimit == -1 || iTimeElapsed < Socket->timelimit ) )
               continue;
         }

         iLen = -2;
      }

      if( iLen > 0 )
      {
         int protos;
         bProtoFound = 0;

         for( protos = 0; protos < iprotos; protos++ )
         {
            if( cChar == Proto[ protos ][ iprotosize[ protos ] - 1 ] && iprotosize[ protos ] <= iPos )
            {
               bProtoFound = 1;
               for( iPosProto = 0; iPosProto < ( iprotosize[ protos ] - 1 ); iPosProto++ )
               {
                  if( Proto[ protos ][ iPosProto ] != Buffer[ ( iPos - iprotosize[ protos ] ) + iPosProto + 1 ] )
                  {
                     bProtoFound = 0;
                     break;
                  }
               }
               if( bProtoFound )
               {
                  ifindproto = protos;
                  break;
               }
            }
         }

         if( bProtoFound )
            break;

         Buffer[ iPos++ ] = cChar;
      }
      else
         break;
   }
   while( iMax == 0 || iPos < iMax );

   if( iLen <= 0 )
   {
      if( pResult )
         hb_itemPutNI( pResult, iLen );

      if( iLen == 0 )
      {
         HB_SOCKET_SET_ERROR2( Socket, -2, "Connection closed" );
      }
      else if( iLen == -2 )
      {
         HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
      }
      else
         HB_SOCKET_SET_ERROR( Socket );

      hb_xfree( ( void * ) Buffer );
      hb_retc( NULL );
   }
   else
   {
      if( iMax == 0 || iPos < iMax )
      {
         Socket->count = iPos;

         if( pResult )
            hb_itemPutNI( pResult, iPos - ( iprotosize[ ifindproto ] - 1 ) );

         hb_retclenAdopt( Buffer, iPos - ( iprotosize[ ifindproto ] - 1 ) );
      }
      else
      {
         HB_SOCKET_SET_ERROR2( Socket, -1, "Buffer overrun" );

         if( pResult )
            hb_itemPutNI( pResult, -2 );

         hb_xfree( ( void * ) Buffer );
         hb_retc( NULL );
      }
   }

   hb_xfree( Proto );
   hb_xfree( iprotosize );
}

HB_FUNC( INETDATAREADY )
{
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 1 );
   int                  iLen;
   fd_set               rfds;
   struct timeval       tv       = { 0, 0 };

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN || ( hb_pcount() == 2 && ! ISNUM( 2 ) ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETDATAREADY", 2, hb_paramError( 1 ), hb_paramError( 2 ) );
      return;
   }

   HB_SOCKET_ZERO_ERROR( Socket );

   /* Watch our socket. */
   if( hb_pcount() == 2 )
   {
      iLen        = hb_parni( 2 );
      tv.tv_sec   = iLen / 1000;
      tv.tv_usec  = ( iLen % 1000 ) * 1000;
   }

   FD_ZERO( &rfds );
   FD_SET( Socket->com, &rfds );

   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   iLen = select( ( int ) Socket->com + 1, &rfds, NULL, NULL, &tv );

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;
   /* Don't rely on the value of tv now! */

   if( iLen < 0 )
      HB_SOCKET_SET_ERROR( Socket );

   hb_retni( iLen );
}

static void s_inetSendInternal( char * szFuncName, int iMode )
{
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 1 );
   PHB_ITEM             pBuffer  = hb_param( 2, HB_IT_STRING );
   char *               Buffer;
   int                  iLen, iSent, iSend, iBufferLen;

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN || pBuffer == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, szFuncName, 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
      return;
   }

   Buffer   = hb_itemGetCPtr( pBuffer );
   iSend    = ( int ) hb_itemGetCLen( pBuffer );
   if( ISNUM( 3 ) )
   {
      iLen = hb_parni( 3 );
      if( iLen < iSend )
         iSend = iLen;
   }
   iSent = 0;

   HB_SOCKET_ZERO_ERROR( Socket );

   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   iLen = 0;
   while( iSent < iSend )
   {
      if( iMode == 1 )
         iBufferLen = Socket->iSndBufSize > iSend - iSent ? iSend - iSent : Socket->iSndBufSize;
      else
         iBufferLen = iSend;

      iLen = 0;

      if( hb_selectWriteSocket( Socket ) )
         iLen = send( Socket->com, Buffer + iSent, iBufferLen, MSG_NOSIGNAL );

      if( iLen > 0 )
         iSent += iLen;
      else if( iLen == 0 )
      {
         HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
         break;
      }
      else
      {
         HB_SOCKET_SET_ERROR( Socket );
         break;
      }

      if( iMode == 0 )
         break;
   }

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;

   Socket->count = iSent;

   if( iLen > 0 )
      hb_retni( iSent );
   else
      hb_retni( -1 );
}

HB_FUNC( INETSEND )
{
   s_inetSendInternal( "INETSEND", 0 );
}

HB_FUNC( INETSENDALL )
{
   s_inetSendInternal( "INETSENDALL", 1 );
}

/*******************************************
 * Name resolution interface functions
 ***/

HB_FUNC( INETGETHOSTS )
{
   const char *      szHost   = hb_parc( 1 );
   struct hostent *  Host;
   char **           cHosts;
   int               iCount   = 0;

   if( szHost == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETHOSTS", 1, hb_paramError( 1 ) );
      return;
   }

   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   Host = hb_getHosts( szHost, NULL );

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;

   if( Host )
   {
      cHosts = Host->h_addr_list;
      while( *cHosts )
      {
         iCount++;
         cHosts++;
      }
   }

   if( iCount == 0 )
      hb_reta( 0 );
   else
   {
      PHB_ITEM pHosts = hb_itemArrayNew( iCount );
      iCount   = 0;
      cHosts   = Host->h_addr_list;
      while( *cHosts )
      {
         hb_arraySetC( pHosts, ++iCount,
                       inet_ntoa( *( ( struct in_addr * ) *cHosts ) ) );
         cHosts++;
      }
      hb_itemReturnRelease( pHosts );
   }
}

HB_FUNC( INETGETALIAS )
{
   const char *      szHost   = hb_parc( 1 );
   struct hostent *  Host;
   char **           cHosts;
   int               iCount   = 0;

   if( szHost == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETGETALIAS", 1, hb_paramError( 1 ) );
      return;
   }

   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   Host = hb_getHosts( szHost, NULL );

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;

   if( Host )
   {
      cHosts = Host->h_aliases;
      while( *cHosts )
      {
         iCount++;
         cHosts++;
      }
   }

   if( iCount == 0 )
      hb_reta( 0 );
   else
   {
      PHB_ITEM pHosts = hb_itemArrayNew( iCount );
      iCount   = 0;
      cHosts   = Host->h_aliases;
      while( *cHosts )
      {
         hb_arraySetC( pHosts, ++iCount,
                       inet_ntoa( *( ( struct in_addr * ) *cHosts ) ) );
         cHosts++;
      }
      hb_itemReturnRelease( pHosts );
   }
}

/**********************************************
 * Server Specific functions
 ****/

HB_FUNC( INETSERVER )
{
   PHB_ITEM             pSocket  = hb_param( 2, HB_IT_POINTER );
   HB_SOCKET_STRUCT *   Socket   = ( pSocket ? ( HB_SOCKET_STRUCT * ) hb_itemGetPtr( pSocket ) : NULL ); /* hb_parptr(2); */
   int                  iPort;
   int                  iOpt     = 1;
   int                  iListen;
   BOOL                 bRelease = FALSE;

   /* Parameter error checking */
   if( ! ISNUM( 1 ) || ( Socket != NULL && Socket->sign != HB_SOCKET_SIGN ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETSERVER", 2,
                            hb_paramError( 1 ), hb_paramError( 2 ) );
      return;
   }

   if( Socket != NULL )
   {
      HB_SOCKET_ZERO_ERROR( Socket );
   }
   else
   {
      HB_SOCKET_INIT( Socket, pSocket );
      bRelease = TRUE;
   }

   /* Creates comm socket */
#if defined( HB_OS_WIN )
   Socket->com = socket( AF_INET, SOCK_STREAM, 0 );
#else
   Socket->com = socket( PF_INET, SOCK_STREAM, 0 );
#endif

   if( Socket->com == ( HB_SOCKET_T ) -1 )
   {
      HB_SOCKET_SET_ERROR( Socket );
      Socket->com = 0;
      hb_itemReturn( pSocket );
      if( bRelease )
         hb_itemRelease( pSocket );
      return;
   }

   /* we'll be using only nonblocking sockets */
   /* hb_socketSetNonBlocking( Socket ); */

   /* Reusable socket; under unix, do not wait it is unused */
   setsockopt( Socket->com, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &iOpt, sizeof( iOpt ) );

   iPort                      = htons( ( u_short ) hb_parni( 1 ) );

   Socket->remote.sin_family  = AF_INET;
   Socket->remote.sin_port    = ( USHORT ) iPort;

   if( ! ISCHAR( 2 ) )
      Socket->remote.sin_addr.s_addr = INADDR_ANY;
   else
      Socket->remote.sin_addr.s_addr = inet_addr( hb_parcx( 2 ) );

   iListen = ISNUM( 3 ) ? hb_parni( 3 ) : 10;

   if( bind( Socket->com, ( struct sockaddr * ) &Socket->remote, sizeof( Socket->remote ) ) )
   {
      HB_SOCKET_SET_ERROR( Socket );
      HB_INET_CLOSE( Socket->com );
   }
   else if( listen( Socket->com, iListen ) )
   {
      HB_SOCKET_SET_ERROR( Socket );
      HB_INET_CLOSE( Socket->com );
   }

   hb_itemReturn( pSocket );

   if( bRelease )
      hb_itemRelease( pSocket );
}

HB_FUNC( INETACCEPT )
{
#if ! defined( EAGAIN )
#  define EAGAIN -1
#endif
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 1 );
   HB_SOCKET_STRUCT *   NewSocket;
   HB_SOCKET_T          incoming = 0;
   int                  iError   = EAGAIN;
   struct sockaddr_in   si_remote;
#if defined( _XOPEN_SOURCE_EXTENDED )
   socklen_t            Len;
#elif defined( HB_OS_WIN )
   int                  Len;
#else
   UINT                 Len;
#endif

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETACCEPT", 1, hb_paramError( 1 ) );
      return;
   }

   Len = sizeof( struct sockaddr_in );

   /*
    * Accept can (and should) be asynchronously stopped by closing the
    * accepting socket. this will make the wait to terminate, and the
    * calling program will be notivfied through the status of the
    * returned socket.
    */

   HB_SOCKET_ZERO_ERROR( Socket );

   HB_STACK_UNLOCK;
   /* Connection incoming */
   while( iError == EAGAIN )
   {
      HB_TEST_CANCEL_ENABLE_ASYN;

      if( hb_selectReadSocket( Socket ) )
      {
         /* On error (e.g. async connection closed) , com will be -1 and
            errno == 22 (invalid argument ) */
         incoming = accept( Socket->com, ( struct sockaddr * ) &si_remote, &Len );

         HB_DISABLE_ASYN_CANC;

         if( incoming == ( HB_SOCKET_T ) -1 )
         {
#if defined( HB_OS_WIN )
            iError   = WSAGetLastError();
#else
            iError   = errno;
#endif
         }
         else
            iError = 0;
      }
      /* Timeout expired */
      else
      {
         iError = -1;
         HB_DISABLE_ASYN_CANC;
      }
   }

   HB_STACK_LOCK;

   if( iError == -1 )
   {
      HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
      hb_ret();
   }
   else if( iError > 0 )
   {
      HB_SOCKET_SET_ERROR1( Socket, iError );
      hb_ret();
   }
   else
   {
      PHB_ITEM pSocket = NULL;
      /* we'll be using only nonblocking sockets */
      HB_SOCKET_INIT( NewSocket, pSocket );
      HB_MEMCPY( &NewSocket->remote, &si_remote, Len );
      NewSocket->com = incoming;
      /* hb_socketSetNonBlocking( NewSocket ); */
      hb_itemReturnRelease( pSocket );
   }
}

/**********************************************
 * Client specific (connection functions)
 ****/

HB_FUNC( INETCONNECT )
{
   const char *         szHost   = hb_parc( 1 );
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 3 );
   PHB_ITEM             pSocket  = NULL;
   struct hostent *     Host;
   int                  iPort;

   if( szHost == NULL || ! ISNUM( 2 ) || ( Socket == NULL && ! ISNIL( 3 ) ) || ( Socket != NULL && Socket->sign != HB_SOCKET_SIGN ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCONNECT", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
      return;
   }

   if( Socket != NULL )
   {
      HB_SOCKET_ZERO_ERROR( Socket );
   }
   else
      HB_SOCKET_INIT( Socket, pSocket );

   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   Host = hb_getHosts( szHost, Socket );

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;

   /* error had been set by get hosts */

   if( Host != NULL )
   {
      /* Creates comm socket */
#if defined( HB_OS_WIN )
      Socket->com = socket( AF_INET, SOCK_STREAM, 0 );
#else
      Socket->com = socket( PF_INET, SOCK_STREAM, 0 );
#endif

      if( Socket->com == ( HB_SOCKET_T ) -1 )
      {
         HB_SOCKET_SET_ERROR( Socket );
      }
      else
      {
         iPort                            = htons( ( u_short ) hb_parni( 2 ) );

         Socket->remote.sin_family        = AF_INET;
         Socket->remote.sin_port          = ( USHORT ) iPort;
         Socket->remote.sin_addr.s_addr   = ( *( UINT * ) Host->h_addr_list[ 0 ] );

         HB_STACK_UNLOCK;
         HB_TEST_CANCEL_ENABLE_ASYN;

         hb_socketConnect( Socket );

         HB_DISABLE_ASYN_CANC;
         HB_STACK_LOCK;
      }
   }

   if( pSocket )
      hb_itemReturnRelease( pSocket );
   else
      hb_itemReturn( hb_param( 3, HB_IT_ANY ) );
}

HB_FUNC( INETCONNECTIP )
{
   const char *         szHost   = hb_parc( 1 );
   HB_SOCKET_STRUCT *   Socket   = HB_PARSOCKET( 3 );
   PHB_ITEM             pSocket  = NULL;
   int                  iPort    = hb_parni( 2 );

   if( szHost == NULL || iPort == 0 || ( Socket == NULL && ! ISNIL( 3 ) ) || ( Socket != NULL && Socket->sign != HB_SOCKET_SIGN ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETCONNECTIP", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
      return;
   }

   if( Socket != NULL )
   {
      HB_SOCKET_ZERO_ERROR( Socket );
   }
   else
      HB_SOCKET_INIT( Socket, pSocket );

   /* Creates comm socket */
#if defined( HB_OS_WIN )
   Socket->com = socket( AF_INET, SOCK_STREAM, 0 );
#else
   Socket->com = socket( PF_INET, SOCK_STREAM, 0 );
#endif

   if( Socket->com == ( HB_SOCKET_T ) -1 )
   {
      HB_SOCKET_SET_ERROR( Socket );
   }
   else
   {
      iPort                            = htons( ( u_short ) iPort );

      Socket->remote.sin_family        = AF_INET;
      Socket->remote.sin_port          = ( USHORT ) iPort;
      Socket->remote.sin_addr.s_addr   = inet_addr( szHost );

      HB_STACK_UNLOCK;
      HB_TEST_CANCEL_ENABLE_ASYN;

      hb_socketConnect( Socket );

      HB_DISABLE_ASYN_CANC;
      HB_STACK_LOCK;
   }

   if( pSocket )
      hb_itemReturnRelease( pSocket );
   else
      hb_itemReturn( hb_param( 3, HB_IT_ANY ) );
}

/***********************************************************
 * Datagram functions
 ************************************************************/

HB_FUNC( INETDGRAMBIND )
{
   HB_SOCKET_STRUCT *   Socket;
   PHB_ITEM             pSocket  = NULL;
   int                  iPort    = hb_parni( 1 );
   int                  iOpt     = 1;

   /* Parameter error checking */
   if( iPort == 0 || ( hb_pcount() == 4 && ! ISCHAR( 4 ) ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETDGRAMBIND", 4, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ), hb_paramError( 4 ) );
      return;
   }

   HB_SOCKET_INIT( Socket, pSocket );

   /* Creates comm socket */
#if defined( HB_OS_WIN )
   Socket->com = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
#else
   Socket->com = socket( PF_INET, SOCK_DGRAM, 0 );
#endif

   if( Socket->com == ( HB_SOCKET_T ) -1 )
   {
      HB_SOCKET_SET_ERROR( Socket );
      Socket->com = 0;
      hb_itemReturnRelease( pSocket );
      return;
   }

   /* Reusable socket; under unix, do not wait it is unused */
   setsockopt( Socket->com, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &iOpt, sizeof( iOpt ) );

   /* Setting broadcast if needed. */
   if( hb_parl( 3 ) )
   {
      iOpt = 1;
      setsockopt( Socket->com, SOL_SOCKET, SO_BROADCAST, ( const char * ) &iOpt, sizeof( iOpt ) );
   }

   /* Binding here */
   iPort                      = htons( ( u_short ) iPort );

   Socket->remote.sin_family  = AF_INET;
   Socket->remote.sin_port    = ( USHORT ) iPort;

   if( ! ISCHAR( 2 ) )
      Socket->remote.sin_addr.s_addr = INADDR_ANY;
   else
      Socket->remote.sin_addr.s_addr = inet_addr( hb_parcx( 2 ) );

   if( bind( Socket->com, ( struct sockaddr * ) &Socket->remote, sizeof( Socket->remote ) ) )
   {
      HB_SOCKET_SET_ERROR( Socket );
      HB_INET_CLOSE( Socket->com );
   }

   if( hb_pcount() == 4 )
   {
#ifndef IP_ADD_MEMBERSHIP
#     define IP_ADD_MEMBERSHIP 5      /* which header should this be in? */
#endif

      /* this structure should be define in a header file.  The MS SDK indicates that */
      /* it is in Ws2tcpip.h but I'm not sure I know where it should go in xHb */
      struct ip_mreq
      {
         struct in_addr imr_multiaddr;    /* IP multicast address of group */
         struct in_addr imr_interface;    /* local IP address of interface */
      };

      struct ip_mreq mreq;

      mreq.imr_multiaddr.s_addr  = inet_addr( hb_parc( 4 ) ); /* HELLO_GROUP */
      mreq.imr_interface.s_addr  = htonl( INADDR_ANY );

#ifndef IPPROTO_IP
      /*
       * some systems may not have this definitions, it should
       * be 0 what works with TCP/UDP sockets or explicitly set
       * to IPPROTO_TCP/IPPROTO_UDP
       */
#     define IPPROTO_IP 0
#endif

      if( setsockopt( Socket->com, IPPROTO_IP, IP_ADD_MEMBERSHIP, ( const char * ) &mreq, sizeof( mreq ) ) < 0 )
      {
         HB_SOCKET_SET_ERROR( Socket );
         Socket->com = 0;
         hb_itemReturnRelease( pSocket );
         return;
      }
   }

   hb_itemReturnRelease( pSocket );
}

HB_FUNC( INETDGRAM )
{
   HB_SOCKET_STRUCT *   Socket;
   PHB_ITEM             pSocket  = NULL;
   int                  iOpt     = 1;

   HB_SOCKET_INIT( Socket, pSocket );

   /* Creates comm socket */
#if defined( HB_OS_WIN )
   Socket->com = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
#else
   Socket->com = socket( PF_INET, SOCK_DGRAM, 0 );
#endif

   if( Socket->com == ( HB_SOCKET_T ) -1 )
   {
      HB_SOCKET_SET_ERROR( Socket );
      Socket->com = 0;
      hb_itemReturnRelease( pSocket );
      return;
   }

   /* Setting broadcast if needed. */
   if( hb_parl( 1 ) )
   {
      iOpt = 1;
      setsockopt( Socket->com, SOL_SOCKET, SO_BROADCAST, ( const char * ) &iOpt, sizeof( iOpt ) );
   }
   /* we'll be using non blocking sockets in all functions */
   /* hb_socketSetNonBlocking( Socket ); */

   hb_itemReturnRelease( pSocket );
}

HB_FUNC( INETDGRAMSEND )
{
   HB_SOCKET_STRUCT *   Socket      = HB_PARSOCKET( 1 );
   const char *         szAddress   = hb_parcx( 2 );
   int                  iPort       = hb_parni( 3 );
   PHB_ITEM             pBuffer     = hb_param( 4, HB_IT_STRING );
   int                  iLen;
   char *               szBuffer;

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN ||
       szAddress == NULL || iPort == 0 || pBuffer == NULL )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETDGRAMSEND", 5,
                            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ),
                            hb_paramError( 4 ), hb_paramError( 5 ) );
      return;
   }

   Socket->remote.sin_family        = AF_INET;
   Socket->remote.sin_port          = htons( ( u_short ) iPort );
   Socket->remote.sin_addr.s_addr   = inet_addr( szAddress );
   szBuffer                         = hb_itemGetCPtr( pBuffer );

   if( ISNUM( 5 ) )
      iLen = hb_parni( 5 );
   else
      iLen = ( int ) hb_itemGetCLen( pBuffer );

   HB_SOCKET_ZERO_ERROR( Socket );

   Socket->count = 0;
   HB_STACK_UNLOCK;
   HB_TEST_CANCEL_ENABLE_ASYN;

   if( hb_selectWriteSocket( Socket ) )
   {
      Socket->count = sendto( Socket->com, szBuffer, iLen, 0,
                              ( const struct sockaddr * ) &Socket->remote, sizeof( Socket->remote ) );
   }

   HB_DISABLE_ASYN_CANC;
   HB_STACK_LOCK;

   hb_retni( Socket->count );

   if( Socket->count == 0 )
   {
      HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
   }
   else if( Socket->count < 0 )
   {
      Socket->count = 0;
      HB_SOCKET_SET_ERROR( Socket );
   }
}

HB_FUNC( INETDGRAMRECV )
{
   HB_SOCKET_STRUCT *   Socket         = HB_PARSOCKET( 1 );
   PHB_ITEM             pBuffer        = hb_param( 2, HB_IT_STRING );
   int                  iTimeElapsed   = 0;
   int                  iLen, iMaxLen;
   char *               Buffer;
   BOOL                 fRepeat;

#if defined( HB_OS_WIN )
   int                  iDtLen   = sizeof( struct sockaddr );
#else
   socklen_t            iDtLen   = ( socklen_t ) sizeof( struct sockaddr );
#endif

   if( Socket == NULL || Socket->sign != HB_SOCKET_SIGN ||
       pBuffer == NULL || ! ISBYREF( 2 ) )
   {
      hb_errRT_BASE_SubstR( EG_ARG, 3012, NULL, "INETDGRAMRECV", 3,
                            hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) );
      return;
   }

   pBuffer  = hb_itemUnShare( pBuffer );
   Buffer   = hb_itemGetCPtr( pBuffer );

   if( ISNUM( 3 ) )
      iMaxLen = hb_parni( 3 );
   else
      iMaxLen = ( int ) hb_itemGetCLen( pBuffer );

   HB_SOCKET_ZERO_ERROR( Socket );

   do
   {
      fRepeat  = FALSE;
      HB_STACK_UNLOCK;
      HB_TEST_CANCEL_ENABLE_ASYN;
      iLen     = -2;
      if( hb_selectReadSocket( Socket ) )
         iLen = recvfrom( Socket->com, Buffer, iMaxLen, 0,
                          ( struct sockaddr * ) &Socket->remote, &iDtLen );
      HB_DISABLE_ASYN_CANC;
      HB_STACK_LOCK;

      iTimeElapsed += Socket->timeout;
      if( Socket->caPeriodic != NULL )
      {
         hb_execFromArray( Socket->caPeriodic );
         /* do we continue? */
         fRepeat = hb_parl( -1 ) &&
                   ( Socket->timelimit == -1 || iTimeElapsed < Socket->timelimit );
      }
   }
   while( fRepeat );

   if( iLen == -2 )
   {
      HB_SOCKET_SET_ERROR2( Socket, -1, "Timeout" );
      Socket->count  = 0;
      iLen           = -1;
   }
   else if( iLen == 0 )
   {
      HB_SOCKET_SET_ERROR2( Socket, -2, "Connection closed" );
      Socket->count = 0;
   }
   else if( iLen < 0 )
   {
      HB_SOCKET_SET_ERROR( Socket );
      Socket->count = 0;
   }
   else
      Socket->count = iLen;

   hb_retni( iLen );
}

/***********************************************************
 * Generic utility(?) functions
 ************************************************************/

HB_FUNC( INETCRLF )
{
   hb_retc( "\r\n" );
}

HB_FUNC( INETISSOCKET )
{
   hb_retl( HB_PARSOCKET( 1 ) != NULL );
}

#endif
