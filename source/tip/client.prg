/*
 * $Id: client.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * xHarbour Project source code:
 * TIP Class oriented Internet protocol library
 *
 * Copyright 2003 Giancarlo Niccolai <gian@niccolai.ws>
 *
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

/* 2004-01-13
  Enhaced tip cliente to conenct to secure smtp servers by Luiz Rafael Culik
*/

/* 2007-03-29, Hannes Ziegler
   Adapted all :new() method(s) so that tIPClient becomes the
   abstract super class for TIpClientFtp, TIpClientHttp, TIpClientPop and TIpClientSmtp

   Added Methods :INetErrorDesc(), :lastErrorCode() and :lastErrorMessage()
   Removed method :data() since it calls an undeclared method :getOk()
   :data() is used in TIpClientSmtp

   Fixed bug in :readToFile()

*/

/* 2007-06-01, Toninho@fwi
   Added data ::nWrite to work like ::nRead
*/
/* 2009-06-29, Luiz Rafael Culik( luiz at xharbour dot com dot br
   Added support for proxy connection
*/


#include "hbclass.ch"
#include "error.ch"
#include "fileio.ch"
#include "tip.ch"
#include "common.ch"

#define RCV_BUF_SIZE Int( ::InetRcvBufSize( ::SocketCon ) / 2 )
#define SND_BUF_SIZE Int( ::InetSndBufSize( ::SocketCon ) / 2 )

/**
* Inet Client class
*/

CLASS tIPClient

   CLASSDATA   bInitSocks  INIT .F.
   CLASSDATA   cCRLF       INIT InetCRLF()
   DATA oUrl                                 // url to wich to connect
   DATA oCredentials                         // credential needed to access the service
   DATA nStatus                              // basic status
   DATA SocketCon
   DATA lTrace
   DATA nHandle   INIT - 1

   DATA nDefaultRcvBuffSize
   DATA nDefaultSndBuffSize

   /* Input stream length */
   DATA nLength
   /* Input stream data read by the app*/
   DATA nRead
   /* Last physical read amount */
   DATA nLastRead

   DATA nDefaultPort
   DATA nConnTimeout
   DATA bInitialized

   DATA cReply
   DATA nAccessMode
   DATA nWrite
   DATA nLastWrite

   DATA bEof
   DATA isOpen INIT .F.

   /** Gauge control; it can be a codeblock or a function pointer. */
   DATA exGauge

   DATA Cargo

// Data For proxy connection
   DATA cProxyHost
   DATA nProxyPort
   DATA cProxyUser
   DATA cProxyPassword
   METHOD SetProxy()
   METHOD Openproxy( cServer, nPort, cProxy, nProxyPort, cResp, cUserName, cPassWord, nTimeOut, cUserAgent )
   METHOD ReadHTTPProxyResponse()

   METHOD New( oUrl, lTrace, oCredentials )
   METHOD Open()

   METHOD READ( iLen )
   METHOD ReadToFile( cFile, nMode, nSize )
   METHOD Write( cData, iLen, bCommit )
   METHOD COMMIT()
   METHOD WriteFromFile( cFile )
   METHOD Reset()
   METHOD CLOSE()
   /*   METHOD Data( cData ) */
// commented: calls undeclared METHOD :getOk

   METHOD lastErrorCode() INLINE ::nLastError
   METHOD lastErrorMessage( SocketCon ) INLINE ::INetErrorDesc( SocketCon )

   METHOD InetRcvBufSize( SocketCon, nSizeBuff )
   METHOD InetSndBufSize( SocketCon, nSizeBuff )

   PROTECTED:
   DATA nLastError INIT 0

   /* Methods to log data if needed */
   METHOD InetRecv( SocketCon, cStr1, len )
   METHOD InetRecvLine( SocketCon, nLen, size )
   METHOD InetRecvAll( SocketCon, cStr1, len )
   METHOD InetCount( SocketCon )
   METHOD InetSendAll( SocketCon, cData, nLen )
   METHOD InetErrorCode( SocketCon )
   METHOD InetErrorDesc( SocketCon )
   METHOD InetConnect( cServer, nPort, SocketCon )

   METHOD Log()

ENDCLASS

METHOD New( oUrl, lTrace, oCredentials ) CLASS tIPClient

   LOCAL oErr

   DEFAULT lTrace TO .F.

   ::lTrace       := lTrace

   IF .NOT. ::bInitSocks
      InetInit()
      ::bInitSocks := .T.
   ENDIF

   IF HB_ISSTRING( oUrl )
      oUrl := tUrl():New( oUrl )
   ENDIF

   IF .NOT. oURL:cProto IN "ftp,http,pop,smtp"
      oErr := ErrorNew()
      oErr:Args          := { Self, oURL:cProto }
      oErr:CanDefault    := .F.
      oErr:CanRetry      := .F.
      oErr:CanSubstitute := .T.
      oErr:Description   := "unsupported protocol"
      oErr:GenCode       := EG_UNSUPPORTED
      oErr:Operation     := ::className() + ":new()"
      oErr:Severity      := ES_ERROR
      oErr:SubCode       := 1081
      oErr:SubSystem     := "BASE"
      Eval( ErrorBlock(), oErr )
   ENDIF

   ::oUrl         := oUrl
   ::oCredentials := oCredentials
   ::nStatus      := 0
   ::bInitialized := .F.
   ::nWrite       := 0
   ::nLastWrite   := 0
   ::nLength      := - 1
   ::nRead        := 0
   ::nLastRead    := 0
   ::bEof         := .F.

   RETURN self

METHOD Open( cUrl ) CLASS tIPClient

   LOCAL nPort
   LOCAL cResp := ""

   IF HB_ISSTRING( cUrl )
      ::oUrl := tUrl():New( cUrl )
   ENDIF

   IF ::oUrl:nPort == - 1
      nPort := ::nDefaultPort
   ELSE
      nPort := ::oUrl:nPort
   ENDIF
   ::SocketCon := InetCreate()
   InetSetTimeout( ::SocketCon, ::nConnTimeout )
   IF !Empty( ::cProxyHost )
      IF !::openProxy( ::oUrl:cServer, nport, ::cProxyHost, ::nProxyPort, @cResp, ::cProxyUser , ::cProxyPassword, ::nConnTimeout, '' )
         RETURN .F.
      ENDIF
   ELSE
      ::InetConnect( ::oUrl:cServer, nPort, ::SocketCon )

      IF ::InetErrorCode( ::SocketCon ) != 0
         RETURN .F.
      ENDIF
   ENDIF
   ::isOpen := .T.

   RETURN .T.

METHOD CLOSE() CLASS tIPClient

   LOCAL nRet := - 1

   IF .NOT. Empty( ::SocketCon )

      nRet := InetClose( ::SocketCon )

      ::SocketCon := nil
      ::isOpen := .F.
   ENDIF

   RETURN( nRet )

METHOD Reset() CLASS tIPClient

   ::bInitialized := .F.
   ::bEof := .F.

   RETURN .T.

METHOD COMMIT() CLASS tIPClient

   RETURN .T.

METHOD READ( nLen ) CLASS tIPClient

   LOCAL cStr0, cStr1

   IF ::nLength > 0 .AND. ::nLength == ::nRead
      RETURN NIL
   ENDIF

   IF Empty( nLen ) .OR. nLen < 0 .OR. ( ::nLength > 0 .AND. nLen > ::nLength - ::nRead )
      nLen := ::nLength - ::nRead
   ENDIF

   IF Empty( nLen ) .OR. nLen < 0
      // read till end of stream
      cStr1 := Space( RCV_BUF_SIZE )
      cStr0 := ""
      ::nLastRead := ::InetRecv( ::SocketCon, @cStr1, RCV_BUF_SIZE )
      DO WHILE ::nLastRead > 0
         ::nRead += ::nLastRead
         cStr0 += SubStr( cStr1, 1, ::nLastRead )
         ::nLastRead := ::InetRecv( ::SocketCon, @cStr1, RCV_BUF_SIZE )
      ENDDO
      ::bEof := .T.
   ELSE
      // read an amount of data
      cStr0 := Space( nLen )

      // S.R. if len of file is less than RCV_BUF_SIZE InetRecvAll return 0
      //      ::nLastRead := InetRecvAll( ::SocketCon, @cStr0, nLen )

      ::InetRecvAll( ::SocketCon, @cStr0, nLen )
      ::nLastRead := ::InetCount( ::SocketCon )
      ::nRead += ::nLastRead

      IF ::nLastRead != nLen
         ::bEof := .T.
         cStr0 := SubStr( cStr0, 1, ::nLastRead )
         // S.R.         RETURN NIL
      ENDIF

      IF ::nRead == ::nLength
         ::bEof := .T.
      ENDIF

   ENDIF

   RETURN cStr0

METHOD ReadToFile( cFile, nMode, nSize ) CLASS tIPClient

   LOCAL nFout
   LOCAL cData
   LOCAL nSent

   IF Empty ( nMode )
      nMode := FC_NORMAL
   ENDIF

   nSent := 0

   IF !Empty( ::exGauge )
      hb_ExecFromArray( ::exGauge, { nSent, nSize, Self } )
   ENDIF

   ::nRead   := 0
   ::nStatus := 1

   DO WHILE ::InetErrorCode( ::SocketCon ) == 0 .AND. .NOT. ::bEof
      cData := ::Read( RCV_BUF_SIZE )
      IF cData == NIL
         IF nFout != NIL
            FClose( nFout )
         ENDIF
         IF ::InetErrorCode( ::SocketCon ) > 0
            RETURN .F.
         ELSE
            RETURN .T.
         ENDIF
      ENDIF
      IF nFout == NIL
         nFout := FCreate( cFile, nMode )
         IF nFout < 0
            ::nStatus := 0
            RETURN .F.
         ENDIF
      ENDIF

      IF FWrite( nFout, cData ) < 0
         FClose( nFout )
         RETURN .F.
      ENDIF

      nSent += Len( cData )
      IF !Empty( ::exGauge )
         hb_ExecFromArray( ::exGauge, { nSent, nSize, Self } )
      ENDIF

   ENDDO

   IF nSent > 0
      ::Commit()
   ENDIF

   ::nStatus := 2
   FClose( nFout )

   RETURN .T.

METHOD WriteFromFile( cFile ) CLASS tIPClient

   LOCAL nFin
   LOCAL cData
   LOCAL nLen
   LOCAL nSize, nSent, nBufSize

   ::nWrite  := 0
   ::nStatus := 0
   nFin := FOpen( cFile, FO_READ )
   IF nFin < 0
      RETURN .F.
   ENDIF
   nSize := FSeek( nFin, 0, 2 )
   FSeek( nFin, 0 )

   nBufSize := SND_BUF_SIZE

// allow initialization of the gauge
   nSent := 0
   IF ! Empty( ::exGauge )
      hb_ExecFromArray( ::exGauge, { nSent, nSize, Self } )
   ENDIF

   ::nStatus := 1
   cData := Space( nBufSize )
   nLen := FRead( nFin, @cData, nBufSize )
   DO WHILE nLen > 0
      IF ::Write( @cData, nLen ) != nLen
         FClose( nFin )
         RETURN .F.
      ENDIF
      nSent += nLen
      IF ! Empty( ::exGauge )
         hb_ExecFromArray( ::exGauge, { nSent, nSize, Self } )
      ENDIF
      nLen := FRead( nFin, @cData, nBufSize )
   ENDDO

// it may happen that the file has lenght 0
   IF nSent > 0
      ::Commit()
   ENDIF

   ::nStatus := 2
   FClose( nFin )

   RETURN .T.


/*
HZ: METHOD :getOk() is not declared in TIpClient

METHOD Data( cData ) CLASS tIPClient
   ::InetSendall( ::SocketCon, "DATA" + ::cCRLF )
   IF .not. ::GetOk()
      RETURN .F.
   ENDIF
   ::InetSendall(::SocketCon, cData + ::cCRLF + "." + ::cCRLF )
RETURN ::GetOk()
*/

METHOD Write( cData, nLen, bCommit ) CLASS tIPClient

   IF Empty( nLen )
      nLen := Len( cData )
   ENDIF

   ::nLastWrite := ::InetSendall( ::SocketCon,  cData , nLen )

   IF .NOT. Empty( bCommit )

      ::Commit()

   ENDIF

   ::nWrite += ::nLastWrite

   RETURN ::nLastWrite

METHOD InetSendAll( SocketCon, cData, nLen ) CLASS tIPClient

   LOCAL nRet

   IF Empty( nLen )
      nLen := Len( cData )
   ENDIF

   nRet := InetSendAll( SocketCon, cData, nLen )

   if ::lTrace
      ::Log( SocketCon, nlen, cData, nRet )
   ENDIF

   RETURN nRet

METHOD InetCount( SocketCon ) CLASS tIPClient

   LOCAL nRet

   nRet := InetCount( SocketCon )

   if ::lTrace
      ::Log( SocketCon, nRet )
   ENDIF

   RETURN nRet

METHOD InetRecv( SocketCon, cStr1, len ) CLASS tIPClient

   LOCAL nRet

   nRet := InetRecv( SocketCon, @cStr1, len )

   if ::lTrace

      ::Log( SocketCon, "", len, iif( nRet >= 0, cStr1, nRet ) )

   ENDIF

   RETURN nRet

METHOD InetRecvLine( SocketCon, nLen, size ) CLASS tIPClient

   LOCAL cRet

   cRet := InetRecvLine( SocketCon, @nLen, size )

   if ::lTrace

      ::Log( SocketCon, "", size, cRet )

   ENDIF

   RETURN cRet

METHOD InetRecvAll( SocketCon, cStr1, len ) CLASS tIPClient

   LOCAL nRet

   nRet := InetRecvAll( SocketCon, @cStr1, len )

   if ::lTrace

      ::Log( SocketCon, "", len, iif( nRet >= 0, cStr1, nRet ) )

   ENDIF

   RETURN nRet

METHOD InetErrorCode( SocketCon ) CLASS tIPClient

   LOCAL nRet

   ::nLastError := nRet := InetErrorCode( SocketCon )

   if ::lTrace

      ::Log( SocketCon, nRet )

   ENDIF

   RETURN nRet

METHOD InetErrorDesc( SocketCon ) CLASS tIPClient

   LOCAL cMsg := ""

   DEFAULT SocketCon TO ::SocketCon

   IF .NOT. Empty( SocketCon )

      cMsg := InetErrorDesc( SocketCon )

   ENDIF

   RETURN cMsg

   /* BROKEN, should test number of parameters and act accordingly, see doc\inet.txt */

METHOD InetConnect( cServer, nPort, SocketCon ) CLASS tIPClient

   InetConnect( cServer, nPort, SocketCon )

   IF ! Empty( ::nDefaultSndBuffSize )
      ::InetSndBufSize( SocketCon, ::nDefaultSndBuffSize )
   ENDIF
   
   IF ! Empty( ::nDefaultRcvBuffSize )
      ::InetRcvBufSize( SocketCon, ::nDefaultRcvBuffSize )
   ENDIF

   if ::lTrace

      ::Log( cServer, nPort, SocketCon )

   ENDIF

   RETURN Nil

   /* Methods to manage buffers */

METHOD InetRcvBufSize( SocketCon, nSizeBuff ) CLASS tIPClient

   IF ! Empty( nSizeBuff )
      InetSetRcvBufSize( SocketCon, nSizeBuff )
   ENDIF

   RETURN InetGetRcvBufSize( SocketCon )

METHOD InetSndBufSize( SocketCon, nSizeBuff ) CLASS tIPClient

   IF ! Empty( nSizeBuff )
      InetSetSndBufSize( SocketCon, nSizeBuff )
   ENDIF

   RETURN InetGetSndBufSize( SocketCon )

/* Called from another method with list of parameters and, as last parameter, return code
   of function being logged.
   Example, I want to log MyFunc( a, b, c ) which returns m,
            ::Log( a, b, c, m )
*/

METHOD Log( ... ) CLASS tIPClient

   LOCAL xVar
   LOCAL cMsg := DToS( Date() ) + "-" + Time() + Space( 2 ) + ;
      SubStr( ProcName( 1 ), RAt( ":", ProcName( 1 ) ) ) + ;
      "( "

   FOR EACH xVar in hb_AParams()

      // Preserves CRLF on result
      IF hb_EnumIndex() < PCount()
         cMsg += StrTran( StrTran( AllTrim( CStr( xVar ) ), Chr( 13 ) ), Chr( 10 ) )
      ELSE
         cMsg += CStr( xVar )
      ENDIF

      cMsg += iif ( hb_EnumIndex() < PCount() - 1, ", ", "" )

      IF hb_EnumIndex() == PCount() - 1
         cMsg += " )" + hb_osNewLine() + ">> "

      ELSEIF hb_EnumIndex() == PCount()
         cMsg += " <<" + hb_osNewLine() + hb_osNewLine()

      ENDIF

   NEXT

   FWrite( ::nHandle, cMsg )

   RETURN Self

METHOD SetProxy( cProxyHost, nProxyPort, cProxyUser, cProxyPassword )  CLASS tIPClient

   ::cProxyHost     := cProxyHost
   ::nProxyPort     := nProxyPort
   ::cProxyUser     := cProxyUser
   ::cProxyPassword := cProxyPassword

   RETURN Self

METHOD Openproxy( cServer, nPort, cProxy, nProxyPort, cResp, cUserName, cPassWord, nTimeOut, cUserAgent )

//Local pSocket
   LOCAL cLine
   LOCAL cRequest := ""
   LOCAL cPass
   LOCAL cEncoded
   LOCAL lRet := .T.
   LOCAL nResponseCode
   LOCAL sResponseCode, nFirstSpace

   ::InetConnect( cProxy, nProxyPort, ::SocketCon )
   IF ::InetErrorCode( ::SocketCon ) == 0
      Try
         cLine := sprintf( 'CONNECT %s:%d HTTP/1', cServer, nPort ) + Chr( 13 ) + Chr( 10 )
         cRequest += cLine
         IF !Empty( cUserName )
            cPass := sprintf( '%s:%s', cUserName, cPassWord )
            cEncoded := hb_base64( cPass, Len( cPass ) )
            cLine := sprintf( "Proxy-authorization: Basic %s", cEncoded ) + Chr( 13 ) + Chr( 10 )
            cRequest += cLine
         ENDIF
         IF !Empty( cUserAgent )
            cLine := sprintf( "User-Agent: %s", cUserAgent ) + Chr( 13 ) + Chr( 10 )
            cRequest += cLine
         ENDIF
         cRequest += Chr( 13 ) + Chr( 10 )
         ::InetSendAll( ::SocketCon, cRequest )
         cResp := ''
         ::ReadHTTPProxyResponse( nTimeOut, @cResp, ::SocketCon )
         nFirstSpace := At( " ", cResp )
         IF ( nFirstSpace != 0 )
            sResponseCode := Right( cResp, Len( cResp ) - nFirstSpace )
            nResponseCode = Val( sResponseCode )
            IF ( nResponseCode != 200 )
               Throw( ErrorNew( "INETCONNECTPROXY", 0, 4000, ProcName(), "Connection refused" ) )
            ENDIF
         ENDIF
      catch
         ::close( )
         lRet := .F.
      END
   
   ENDIF

   RETURN  lRet

METHOD ReadHTTPProxyResponse( dwTimeout, sResponse )

   LOCAL  bMoreDataToRead := .T.
   LOCAL nLength, nData
   LOCAL szResponse

   HB_SYMBOL_UNUSED( dwTimeout )
   WHILE bMoreDataToRead
  
      szResponse := Space( 1 )
      nData := inetRecv( ::SocketCon, @szResponse, 1 )
      IF ( nData == 0 )
         throw( ErrorNew( "INETCONNECTPROXY" ,0,4000,ProcName(),"Disconnected" ) )
      ENDIF
      sResponse += szResponse

      nLength = Len( sResponse )
      IF nLength >= 4
         bMoreDataToRead := !( ( SubStr( sResponse, nLength - 3 , 1 ) == Chr( 13 ) ) .AND. ( SubStr( sResponse, nLength - 2, 1 ) == Chr( 10 ) ) .AND. ;
            ( SubStr( sResponse, nLength - 1, 1 ) == Chr( 13 ) ) .AND. ( SubStr( sResponse, nLength, 1 ) == Chr( 10 ) ) )
      ENDIF
   ENDDO

   RETURN nil






