/*
 * $Id: popcln.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * xHarbour Project source code:
 * TIP Class oriented Internet protocol library
 *
 * Copyright 2003 Giancarlo Niccolai <gian@niccolai.ws>
 *
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modIFy
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
 * along with this software; see the file COPYING.  IF not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, IF you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  IF you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modIFied files, you must delete
 * this exception notice from them.
 *
 * IF you write modIFications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modIFications.
 * IF you do not wish that, delete this exception notice.
 *
 */

/* 2007-04-10, Hannes Ziegler <hz AT knowlexbase.com>
   Added method :countMail()
   Added method :retrieveAll()
*/

#include "hbclass.ch"
#include "common.ch"
/**
* Inet service manager: pop3
*/

CLASS tIPClientPOP FROM tIPClient

   METHOD New( oUrl, lTrace, oCredentials, CAFile,CaPath )
   METHOD Open()
   METHOD Close()
   METHOD Read( iLen )
   METHOD Stat()
   METHOD List()
   METHOD Retrieve( nId, nLen )
   METHOD Delete()
   METHOD Quit()
   METHOD Noop()                 // Can be called repeatedly to keep-alive the connection
   METHOD Top( nMsgId )          // Get Headers of mail (no body) to be able to quickly handle a message
   METHOD UIDL( nMsgId )         // Returns Unique ID of message n or list of unique IDs of all message inside maildrop
   METHOD GetOK()
   METHOD countMail()
   METHOD retrieveAll()
   DESTRUCTOR popDestructor
ENDCLASS


METHOD New( oUrl, lTrace, oCredentials,CAFile,CaPath ) CLASS tIPClientPOP

   local cFile :="pop3"
   local n := 0
   ::super:New( oUrl, lTrace, oCredentials, , CAFile,CaPath )

   ::nDefaultPort := 110
   ::nConnTimeout := 10000

   IF ::ltrace
      IF !file( "pop3.log" )
         ::nHandle := fcreate( "pop3.log" )
      ELSE
         while file( cFile + alltrim(str( n, 4 ) ) + ".log" )
           n++
         ENDdo
         ::nHandle := fcreate( cFile + alltrim( str( n, 4 ) ) + ".log" )
      ENDIF
   ENDIF

RETURN Self


METHOD Open( cUrl ) CLASS tIPClientPOP
   IF .not. ::super:Open( cUrl )
      RETURN .F.
   ENDIF

   IF Empty ( ::oUrl:cUserid ) .or. Empty ( ::oUrl:cPassword )
      RETURN .F.
   ENDIF
   IF ::lSSL
      InetSSLSetTimeout( ::SocketCon, ::nConnTimeout )
   ELSE
      InetSetTimeout( ::SocketCon, ::nConnTimeout )
   ENDIF
         
   IF ::GetOk()
      ::InetSENDall( ::SocketCon, "USER " + ::oUrl:cUserid + ::cCRLF )
      IF ::GetOK()
         ::InetSENDall( ::SocketCon, "PASS " + ::oUrl:cPassword + ::cCRLF )
         IF ::GetOK()
            ::isOpen := .T.
            RETURN .T.
         ENDIF
      ENDIF
   ENDIF
RETURN .F.


METHOD GetOk() CLASS tIPClientPOP
   LOCAL nLen

   ::cReply := ::InetRecvLine( ::SocketCon, @nLen, 128 )
   IF ::InetErrorCode( ::SocketCon ) != 0 .or. ::cReply[1] != '+'
      RETURN .F.
   ENDIF
RETURN .T.


METHOD Noop() CLASS tIPClientPOP
   ::InetSENDall( ::SocketCon, "NOOP" + ::cCRLF )
RETURN ::GetOk()


METHOD Close() CLASS tIPClientPOP
   IF ::lSSL
      InetSSLSetTimeOut( ::SocketCon, ::nConnTimeout )
   ELSE
      InetSetTimeOut( ::SocketCon, ::nConnTimeout )
   ENDIF

   IF ::ltrace
      fClose(::nHandle)
      ::nhandle:= -1
   ENDIF

   ::Quit()
RETURN ::super:Close()


METHOD Quit() CLASS tIPClientPOP
   ::InetSENDall( ::SocketCon, "QUIT" + ::cCRLF )
RETURN ::GetOk()


METHOD Stat() CLASS tIPClientPOP
   LOCAL nRead
   ::InetSENDall( ::SocketCon, "STAT" + ::cCRLF )
RETURN ::InetRecvLine( ::SocketCon, @nRead, 128)


METHOD Read( nLen ) CLASS tIPClientPOP
   /** Set what to read for */
   IF Empty( ::oUrl:cFile )
      RETURN ::List()
   ENDIF

   IF Val (::oUrl:cFile ) < 0
      IF ::Delete( -  Val (::oUrl:cFile ) )
         RETURN ::Quit()
      ELSE
         RETURN .F.
      ENDIF
   ENDIF

RETURN ::Retreive( Val (::oUrl:cFile ), nLen )



METHOD Top( nMsgId ) CLASS tIPClientPOP
   LOCAL nPos
   LOCAL cStr, cRet

   ::InetSENDall( ::SocketCon, "TOP " + Str( nMsgId ) + " 0 " + ::cCRLF )
   IF .not. ::GetOk()
      RETURN NIL
   ENDIF

   cRet := ""
   DO WHILE cStr != "." .and. ::InetErrorCode( ::SocketCon ) == 0
      cStr := ::InetRecvLine( ::SocketCon, @nPos, 256 )
      IF cStr != "."
         cRet += cStr + ::cCRLF
      ELSE
         ::bEof := .T.
      ENDIF

   ENDDO

   IF ::InetErrorCode( ::SocketCon ) != 0
      RETURN NIL
   ENDIF

RETURN cRet



METHOD List() CLASS tIPClientPOP
   LOCAL nPos
   LOCAL cStr, cRet

   ::InetSENDall( ::SocketCon, "LIST" + ::cCRLF )
   IF .not. ::GetOk()
      RETURN NIL
   ENDIF

   cRet := ""
   DO WHILE cStr != "." .and. ::InetErrorCode( ::SocketCon ) == 0
      cStr := ::InetRecvLine( ::SocketCon, @nPos, 256 )
      IF cStr != "."
         cRet += cStr + ::cCRLF
      ELSE
         ::bEof := .T.
      ENDIF

   ENDDO

   IF ::InetErrorCode( ::SocketCon ) != 0
      RETURN NIL
   ENDIF

RETURN cRet



METHOD UIDL( nMsgId ) CLASS tIPClientPOP

   LOCAL nPos
   LOCAL cStr, cRet

   IF ! Empty( nMsgId )
      ::InetSENDall( ::SocketCon, "UIDL " + Str( nMsgId ) + ::cCRLF )
   ELSE
      ::InetSENDall( ::SocketCon, "UIDL" + ::cCRLF )
   ENDIF

   IF .not. ::GetOk()
      RETURN NIL
   ENDIF

   IF ! Empty( nMsgId )

      // +OK Space(1) nMsg Space(1) UID
      RETURN SubStr(::cReply, Rat(Space(1), ::cReply) + 1)

   ELSE

      cRet := ""
      DO WHILE cStr != "." .and. ::InetErrorCode( ::SocketCon ) == 0
         cStr := ::InetRecvLine( ::SocketCon, @nPos, 256 )
         IF cStr != "."
            cRet += cStr + ::cCRLF
         ELSE
            ::bEof := .T.
         ENDIF

      ENDDO

   ENDIF

   IF ::InetErrorCode( ::SocketCon ) != 0
      RETURN NIL
   ENDIF

RETURN cRet



METHOD Retrieve( nId, nLen ) CLASS tIPClientPOP

   LOCAL nPos
   LOCAL cRet, nRetLen, cBuffer, nRead
   LOCAL cEOM := ::cCRLF + "." + ::cCRLF        // END Of Mail
   DEFAULT nLen to 0

   IF .not. ::bInitialized
      ::InetSENDall( ::SocketCon, "RETR "+ Str( nId ) + ::cCRLF )
      IF .not. ::GetOk()
         ::bEof := .T.
         RETURN NIL
      ENDIF
      ::bInitialized := .T.
   ENDIF

   cRet := ""
   nRetLen := 0
   nRead := 0

   /* 04/05/2004 - <maurilio.longo@libero.it>
      Instead of receiving a single char at a time until after we have the full mail, let's receive as
      much as we can and stop when we reach EOM (END of mail :)) sequence. This way is _a lot_ faster
   */
   ::bEof := .F.
   DO WHILE ::InetErrorCode( ::SocketCon ) == 0 .AND. ! ::bEof

      cBuffer :=  Space(1024) 

      nRead := ::InetRecv( ::SocketCon, @cBuffer, 1024 )

      cRet += Left( cBuffer, nRead )

      /* 24/11/2005 - <maurilio.longo@libero.it>
                      "- Len( cEOM )" to be sure to always find a full EOM,
                      otherwise IF response breaks EOM in two, it will never
                      be found
      */
      IF ( nPos := At( cEOM, cRet, Max( nRetLen - Len( cEOM ), 1 ) ) ) <> 0
         // Remove ".CRLF"
         cRet := Left( cRet, nPos + 1 )
         ::bEof := .T.

      ELSEIF ! Empty( nLen ) .AND. nLen < Len( cRet )
         EXIT

      ELSE
         nRetLen += nRead

      ENDIF

   ENDDO

   IF ::InetErrorCode( ::SocketCon ) != 0
      RETURN NIL
   ENDIF

       // Remove byte-stuffed termination octet(s) IF any
RETURN StrTran( cRet, ::cCRLF + "..", ::cCRLF + "." )


METHOD Delete( nId ) CLASS tIPClientPOP
   ::InetSENDall( ::SocketCon, "DELE " + AllTrim( Str( nId ) ) +  ::cCRLF )
RETURN ::GetOk()



METHOD countMail CLASS TIpClientPop
   LOCAL aMails
   IF ::isOpen
      ::reset()
      aMails := HB_ATokens( StrTran( ::list(), Chr(13),''), Chr(10) )
      RETURN Len( aMails ) 
   ENDIF
RETURN -1


METHOD retrieveAll( lDelete )
   LOCAL aMails, i, imax, cMail

   IF Valtype( lDelete ) <> "L"
      lDelete := .F.
   ENDIF

   IF .NOT. ::isOpen
      RETURN NIL
   ENDIF

   imax := ::countMail()
   aMails := Array( imax )

   FOR i:=1 TO imax
      ::reset()
      cMail := ::retrieve( i )
      aMails[i] := TIpMail():new()
      aMails[i]:fromString( cMail )

      IF lDelete
         ::reset()
         ::delete(i)
      ENDIF
   NEXT

RETURN aMails

PROCEDURE popDestructor CLASS tIPClientPOP
   IF ::ltrace .and. ::nhandle > -1    
      fClose( ::nHandle )
      ::nhandle := -1 
   ENDIF
RETURN

