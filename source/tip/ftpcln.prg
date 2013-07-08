/*
 * $Id: ftpcln.prg 9935 2013-03-21 08:28:29Z zsaulius $
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

/* 2007-04-19, Hannes Ziegler <hz AT knowlexbase.com>
   Added method :RMD()
   Added method :listFiles()
   Added method :MPut()
   Changed method :downloadFile() to enable display of progress
   Changed method :uploadFile() to enable display of progress
*/

/* 2007-06-01, Toninho@fwi
   Added method UserCommand( cCommand, lPasv, lReadPort, lGetReply )
*/

/* 2007-07-12, miguelangel@marchuet.net
   Added method :NoOp()
   Added method :Rest( nPos )
   Changed method :LS( cSpec )
   Changed method :List( cSpec )
   Changed method :TransferStart()
   Changed method :Stor( cFile )
   Changed method :UploadFile( cLocalFile, cRemoteFile )
   Changed method :DownloadFile( cLocalFile, cRemoteFile )

   Added support to Port transfer mode
   Added method :Port()
   Added method :SendPort()

   Cleaned unused variables.
*/

/* 2007-09-08 21:34 UTC+0100 Patrick Mast <patrick/dot/mast/at/xharbour.com>
   * source\tip\ftpcln.prg
     * Formatting
   + METHOD StartCleanLogFile()
     Starts a clean log file, overwriting current logfile.
   + METHOD fileSize( cFileSpec )
     Calculates the filesize of the given files specifications.
   + DATA cLogFile
     Holds the filename of the current logfile.
   ! Fixed logfilename in New(), now its not limited to 9999 log files anymore
   ! Fixed MGet() due to changes in HB_aTokens()
   ! Fixed listFiles() due to changes in HB_aTokens()
   ! listFiles() is still buggy. Needs to be fixed.
*/

#include "directry.ch"
#include "hbclass.ch"
#include "tip.ch"
#include "common.ch"

STATIC nPort := 16000

/**
* Inet service manager: ftp
*/

CLASS tIPClientFTP FROM tIPClient

   DATA nDataPort
   DATA cDataServer
   DATA bUsePasv
   DATA RegBytes
   DATA RegPasv
// Socket opened in response to a port command
   DATA SocketControl
   DATA SocketPortServer
   DATA cLogFile
// Culik new data to verify if we finish user command
   DATA lInUser

   METHOD New( oUrl, lTrace, oCredentials )
   METHOD Open()
   METHOD READ( nLen )
   METHOD Write( nLen )
   METHOD CLOSE()
   METHOD TransferStart()
   METHOD COMMIT()

   METHOD GetReply()
   METHOD Pasv()
   METHOD TypeI()
   METHOD TypeA()
   METHOD NoOp()
   METHOD REST( nPos )
   METHOD LIST( cSpec )
   METHOD UserCommand( cCommand, lPasv, lReadPort, lGetReply )
   METHOD Pwd()
   METHOD Cwd( cPath )
   METHOD DELE( cPath )
   METHOD Port()
   METHOD SendPort()
   METHOD Retr( cFile )
   METHOD STOR( cFile )
   METHOD QUIT()
   METHOD ScanLength()
   METHOD ReadAuxPort()
   METHOD mget()
// Method bellow contributed by  Rafa Carmona

   METHOD LS( cSpec )
   METHOD RENAME( cFrom, cTo )
// new method for file upload
   METHOD UpLoadFile( cLocalFile, cRemoteFile )
// new method to download file
   METHOD DownLoadFile( cLocalFile, cRemoteFile )
// new method to create an directory on ftp server
   METHOD MKD( cPath )

   METHOD RMD( cPath )
   METHOD listFiles( cList )
   METHOD MPut
   METHOD StartCleanLogFile()
   METHOD fileSize( cFileSpec )
   METHOD CDUP()
   DESTRUCTOR FtpClnDesTructor

ENDCLASS

PROCEDURE FtpClnDesTructor CLASS  tIPClientftp

   IF ::lTrace .AND. ::nHandle > 0
      FClose( ::nHandle )
      ::nHandle := - 1
   ENDIF

   RETURN

METHOD New( oUrl, lTrace, oCredentials ) CLASS tIPClientFTP

   LOCAL cFile := "ftp"
   LOCAL n := 0

   ::lInUser := .F.
   ::super:new( oUrl, lTrace, oCredentials )
   ::nDefaultPort := 21
   ::nConnTimeout := 3000
   ::bUsePasv     := .T.
   ::nAccessMode  := TIP_RW  // a read-write protocol
   ::nDefaultSndBuffSize := 65536
   ::nDefaultRcvBuffSize := 65536

   if ::ltrace
      IF !File( "ftp.log" )
         ::nHandle := FCreate( "ftp.log" )
      ELSE
         WHILE File( cFile + LTrim( Str(Int(n ) ) ) + ".log" )
            n++
         ENDDO
         ::cLogFile := cFile + LTrim( Str( Int(n ) ) ) + ".log"
         ::nHandle := FCreate( ::cLogFile )
      ENDIF
   ENDIF

// precompilation of regex for better prestations
   ::RegBytes := hb_regexComp( "\(([0-9]+)[ )a-zA-Z]" )
   ::RegPasv :=  hb_regexComp( "([0-9]*) *, *([0-9]*) *, *([0-9]*) *, *([0-9]*) *, *([0-9]*) *, *([0-9]*)" )

   RETURN Self

METHOD StartCleanLogFile() CLASS tIPClientFTP

   FClose( ::nHandle )
   ::nHandle := FCreate( ::cLogFile )

   RETURN NIL

METHOD Open( cUrl ) CLASS tIPClientFTP

   LOCAL lRet := .F.

   IF HB_ISSTRING( cUrl )
      ::oUrl := tUrl():New( cUrl )
   ENDIF

   IF Len( ::oUrl:cUserid ) == 0 .OR. Len( ::oUrl:cPassword ) == 0
      RETURN .F.
   ENDIF

   IF .NOT. ::super:Open()
      RETURN .F.
   ENDIF

   InetSetTimeout( ::SocketCon, ::nConnTimeout )
   IF ::GetReply()
      ::InetSendall( ::SocketCon, "USER " + ::oUrl:cUserid + ::cCRLF )
      IF ::GetReply()
         ::InetSendall( ::SocketCon, "PASS " + ::oUrl:cPassword + ::cCRLF )
         // set binary by default
         IF ::GetReply() .AND. ::TypeI()
            lRet := .T.
         ENDIF
      ENDIF
   ENDIF
   IF lRet
      WHILE .T.
         ::GetReply()
         IF ::cReply == NIL
            EXIT
         ENDIF
      ENDDO
      RETURN lRet
   ENDIF

   RETURN .F.

METHOD GetReply() CLASS tIPClientFTP

   LOCAL nLen
   LOCAL cRep

   ::cReply := ::InetRecvLine( ::SocketCon, @nLen, 128 )

   cRep := ::cReply

   IF cRep == NIL
      RETURN .F.
   ENDIF

// now, if the reply has a '-' as fourth character, we need to proceed...
   WHILE ( ! Empty( cRep ) .AND. cRep[4] == '-' )
      ::cReply := ::InetRecvLine( ::SocketCon, @nLen, 128 )
      cRep := If( ValType( ::cReply ) == "C", ::cReply, "" )
   ENDDO

// 4 and 5 are error codes
   IF ::InetErrorCode( ::SocketCon ) != 0 .OR. ::cReply[1] >= '4'
      RETURN .F.
   ENDIF

   RETURN .T.

METHOD Pasv() CLASS tIPClientFTP

   LOCAL aRep

   ::InetSendall( ::SocketCon, "PASV" + ::cCRLF )
   IF .NOT. ::GetReply()
      RETURN .F.
   ENDIF
   aRep := hb_regex( ::RegPasv, ::cReply )

   IF Empty( aRep )
      RETURN .F.
   ENDIF

   ::cDataServer := aRep[2] + "." + aRep[3] + "." + aRep[4] + "." + aRep[5]
   ::nDataPort := Val( aRep[6] ) * 256 + Val( aRep[7] )

   RETURN .T.

METHOD CLOSE() CLASS tIPClientFTP

   InetSetTimeOut( ::SocketCon, ::nConnTimeout )
   if ::ltrace
      FClose( ::nHandle )
      ::nhandle := - 1
   ENDIF

   ::Quit()

   RETURN ::super:Close()

METHOD QUIT() CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "QUIT" + ::cCRLF )

   RETURN ::GetReply()

METHOD TypeI() CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "TYPE I" + ::cCRLF )

   RETURN ::GetReply()

METHOD TypeA() CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "TYPE A" + ::cCRLF )

   RETURN ::GetReply()

METHOD NoOp() CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "NOOP" + ::cCRLF )

   RETURN ::GetReply()

METHOD REST( nPos ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "REST " + Str( If( Empty( nPos ), 0, nPos ),,, .T. ) + ::cCRLF )

   RETURN ::GetReply()

METHOD CWD( cPath ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "CWD " + cPath + ::cCRLF )

   RETURN ::GetReply()

METHOD CDUP(  ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "CDUP " + ::cCRLF )

   RETURN ::GetReply()

METHOD PWD() CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "PWD"  + ::cCRLF )
   IF .NOT. ::GetReply()
      RETURN .F.
   ENDIF
   ::cReply := SubStr( ::cReply, At( '"', ::cReply ) + 1, ;
      RAt( '"', ::cReply ) - At( '"', ::cReply ) - 1 )

   RETURN .T.

METHOD DELE( cPath ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "DELE " + cPath + ::cCRLF )

   RETURN ::GetReply()

// scan last reply for an hint of length

METHOD ScanLength() CLASS tIPClientFTP

   LOCAL aBytes

   aBytes := hb_regex( ::RegBytes, ::cReply )
   IF .NOT. Empty( aBytes )
      ::nLength := Val( aBytes[2] )
   ENDIF

   RETURN .T.

METHOD TransferStart() CLASS tIPClientFTP

   LOCAL skt

   ::SocketControl := ::SocketCon

   IF ::bUsePasv
      skt := InetConnectIP( ::cDataServer, ::nDataPort )
      IF skt != NIL .AND. ::InetErrorCode( skt ) == 0
         // Get the start message from the control connection
         IF ! ::GetReply()
            InetClose( skt )
            RETURN .F.
         ENDIF

         InetSetTimeout( skt, ::nConnTimeout )
         /* Set internal socket send buffer to 64k,
         * this should fix the speed problems some users have reported
         */
         IF ! Empty( ::nDefaultSndBuffSize )
            ::InetSndBufSize( skt, ::nDefaultSndBuffSize )
         ENDIF

         IF ! Empty( ::nDefaultRcvBuffSize )
            ::InetRcvBufSize( skt, ::nDefaultRcvBuffSize )
         ENDIF

         ::SocketCon := skt
      ENDIF
   ELSE
      ::SocketCon := InetAccept( ::SocketPortServer )
      IF Empty( ::SocketCon )
         ::bInitialized := .F.
         ::SocketCon := ::SocketControl
         ::GetReply()
         RETURN .F.
      ENDIF
      InetSetRcvBufSize( ::SocketCon, 65536 )
      InetSetSndBufSize( ::SocketCon, 65536 )
   ENDIF

   RETURN .T.

METHOD COMMIT() CLASS tIPClientFTP

   InetClose( ::SocketCon )
   ::SocketCon := ::SocketControl
   ::bInitialized := .F.

   IF .NOT. ::GetReply()
      RETURN .F.
   ENDIF

// error code?
   IF ::cReply[1] == "5"
      RETURN .F.
   ENDIF

   RETURN .T.

METHOD LIST( cSpec ) CLASS tIPClientFTP

   LOCAL cStr

   IF cSpec = nil
      cSpec := ''
   ELSE
      cSpec := ' ' + cSpec
   ENDIF
   IF ::bUsePasv
      IF .NOT. ::Pasv()
         //::bUsePasv := .F.
         RETURN .F.
      ENDIF
   ENDIF

   IF .NOT. ::bUsePasv
      IF .NOT. ::Port()
         RETURN .F.
      ENDIF
   ENDIF

   ::InetSendAll( ::SocketCon, "LIST" + cSpec + ::cCRLF )
   cStr := ::ReadAuxPort()
   ::bEof := .F.

   RETURN cStr

METHOD UserCommand( cCommand, lPasv, lReadPort, lGetReply ) CLASS tIPClientFTP

   DEFAULT cCommand  TO ""
   DEFAULT lPasv     TO .T.
   DEFAULT lReadPort TO .T.
   DEFAULT lGetReply TO .F.

   if ::bUsePasv .AND. lPasv .AND. !::Pasv()
      RETURN .F.
   ENDIF

   ::InetSendAll( ::SocketCon, cCommand )

   IF lReadPort
      lReadPort := ::ReadAuxPort()
   ENDIF

   IF lGetReply
      lGetReply := ::GetReply()
   ENDIF

   RETURN .T.

METHOD ReadAuxPort( cLocalFile ) CLASS tIPClientFTP

   LOCAL cRet, cList := "", nFile := 0

   IF .NOT. ::TransferStart()
      RETURN NIL
   END
   IF !Empty( cLocalFile )
      nFile := FCreate( cLocalFile )
   ENDIF
   cRet := ::super:Read( 512 )
   WHILE cRet != NIL .AND. Len( cRet ) > 0
      IF nFile > 0
         FWrite( nFile, cRet )
      ELSE
         cList += cRet
      ENDIF
      cRet := ::super:Read( 512 )
   END

   InetClose( ::SocketCon )
   ::SocketCon := ::SocketControl
   IF ::GetReply()
      IF nFile > 0
         FClose( nFile )
         RETURN( .T. )
      ENDIF
      RETURN cList
   ENDIF

   RETURN NIL

METHOD STOR( cFile ) CLASS tIPClientFTP

   IF ::bUsePasv
      IF .NOT. ::Pasv()
         //::bUsePasv := .F.
         RETURN .F.
      ENDIF
   ENDIF

   ::InetSendall( ::SocketCon, "STOR " + cFile + ::cCRLF )

// It is important not to delete these lines in order not to disrupt the timing of
// the responses, which can lead to failures in transfers.
   IF ! ::bUsePasv
      ::GetReply()
   ENDIF

   RETURN ::TransferStart()

METHOD Port() CLASS tIPClientFTP

   ::SocketPortServer := InetCreate( ::nConnTimeout )
   nPort ++
   DO WHILE nPort < 24000
      InetServer( nPort, ::SocketPortServer )
      IF ::InetErrorCode( ::SocketPortServer ) == 0
         RETURN ::SendPort()
      ENDIF
      nPort ++
   ENDDO

   RETURN .F.

METHOD SendPort() CLASS tIPClientFTP

   LOCAL cAddr
   LOCAL cPort, nPort

   cAddr := InetGetHosts( NetName() )[1]
   cAddr := StrTran( cAddr, ".", "," )
   nPort := InetPort( ::SocketPortServer )
   cPort := "," + AllTrim( Str( Int( nPort / 256 ) ) ) +  "," + AllTrim( Str( Int( nPort % 256 ) ) )

   ::InetSendall( ::SocketCon, "PORT " + cAddr + cPort  + ::cCRLF )

   RETURN ::GetReply()

METHOD READ( nLen ) CLASS tIPClientFTP

   LOCAL cRet

   IF .NOT. ::bInitialized

      IF .NOT. Empty( ::oUrl:cPath )

         IF .NOT. ::CWD( ::oUrl:cPath )

            ::bEof := .T.  // no data for this transaction
            RETURN .F.

         ENDIF

      ENDIF

      IF Empty( ::oUrl:cFile )

         RETURN ::List()

      ENDIF

      IF .NOT. ::Retr( ::oUrl:cFile )

         ::bEof := .T.  // no data for this transaction
         RETURN .F.

      ENDIF

      // now channel is open
      ::bInitialized := .T.

   ENDIF

   cRet := ::super:Read( nLen )

   IF cRet == NIL

      ::Commit()
      ::bEof := .T.

   ENDIF

   RETURN cRet

//
// FTP transfer wants commit only at end.
//

METHOD Write( cData, nLen ) CLASS tIPClientFTP

   IF .NOT. ::bInitialized

      IF Empty( ::oUrl:cFile )

         RETURN - 1

      ENDIF

      IF .NOT. Empty( ::oUrl:cPath )

         IF .NOT. ::CWD( ::oUrl:cPath )
            RETURN - 1
         ENDIF

      ENDIF

      IF .NOT. ::Stor( ::oUrl:cFile )
         RETURN - 1
      ENDIF

      // now channel is open
      ::bInitialized := .T.
   ENDIF

   RETURN ::super:Write( cData, nLen, .F. )

/*
 * HZ: What's cLocalFile good for? It's unused
 */

METHOD Retr( cFile ) CLASS tIPClientFTP

   IF ::bUsePasv
      IF .NOT. ::Pasv()
         //::bUsePasv := .F.
         RETURN .F.
      ENDIF
   ENDIF

   ::InetSendAll( ::SocketCon, "RETR " + cFile + ::cCRLF )

   IF ::TransferStart()
      ::ScanLength()
      RETURN .T.
   ENDIF

   RETURN .F.

METHOD MGET( cSpec, cLocalPath ) CLASS tIPClientFTP

   LOCAL cStr, cfile, aFiles

   IF cSpec == nil
      cSpec := ''
   ENDIF
   IF cLocalPath = nil
      cLocalPath := ''
   ENDIF
   IF ::bUsePasv
      IF .NOT. ::Pasv()
         //::bUsePasv := .F.
         RETURN .F.
      ENDIF
   ENDIF

   ::InetSendAll( ::SocketCon, "NLST " + cSpec + ::cCRLF )
   cStr := ::ReadAuxPort()

   IF !Empty( cStr )
      aFiles := hb_ATokens( StrTran( cStr,Chr(13 ),'' ), Chr( 10 ) )
      FOR EACH cFile in aFiles
         IF !Empty( cFile ) //PM:09-08-2007 Needed because of the new HB_aTokens()
            ::downloadfile( cLocalPath + Trim( cFile ), Trim( cFile ) )
         ENDIF
      NEXT

   ENDIF

   RETURN cStr

METHOD MPUT( cFileSpec, cAttr ) CLASS tIPClientFTP

   LOCAL cPath, cFile, cExt, aFile, aFiles
   LOCAL cStr := ""

   IF ValType( cFileSpec ) <> "C"
      RETURN 0
   ENDIF

   hb_FNameSplit( cFileSpec, @cPath, @cFile, @cExt  )

   aFiles := Directory( cPath + cFile + cExt, cAttr )

   FOR EACH aFile in aFiles
      IF ::uploadFile( cPath + aFile[F_NAME], aFile[F_NAME] )
         cStr += INetCrlf() + aFile[F_NAME]
      ENDIF
   NEXT

   RETURN SubStr( cStr, 3 )

METHOD UpLoadFile( cLocalFile, cRemoteFile ) CLASS tIPClientFTP

   LOCAL cPath := ""
   LOCAL cFile := ""
   LOCAL cExt  := ""

   hb_FNameSplit( cLocalFile, @cPath, @cFile, @cExt  )

   DEFAULT cRemoteFile TO cFile + cExt

   ::bEof := .F.
   ::oUrl:cFile := cRemoteFile

   IF ! ::bInitialized

      IF Empty( ::oUrl:cFile )
         RETURN .F.
      ENDIF

      IF ! Empty( ::oUrl:cPath )

         IF ! ::CWD( ::oUrl:cPath )
            RETURN .F.
         ENDIF

      ENDIF

      IF ! ::bUsePasv .AND. ! ::Port()
         RETURN .F.
      ENDIF

      IF ! ::Stor( ::oUrl:cFile )
         RETURN .F.
      ENDIF

      // now channel is open
      ::bInitialized := .T.
   ENDIF

   RETURN ::WriteFromFile( cLocalFile )

METHOD LS( cSpec ) CLASS tIPClientFTP

   LOCAL cStr

   IF cSpec == nil
      cSpec := ''
   ENDIF

   IF ::bUsePasv .AND. ! ::Pasv()
      //::bUsePasv := .F.
      RETURN .F.
   ENDIF

   IF ! ::bUsePasv .AND. ! ::Port()
      RETURN .F.
   ENDIF

   ::InetSendAll( ::SocketCon, "NLST " + cSpec + ::cCRLF )
   IF ::GetReply()
      cStr := ::ReadAuxPort()
   ELSE
      cStr := ''
   ENDIF

   RETURN cStr

   /*Rename a traves del ftp */

METHOD RENAME( cFrom, cTo ) CLASS tIPClientFTP

   LOCAL lResult  := .F.

   ::InetSendAll( ::SocketCon, "RNFR " + cFrom + ::cCRLF )

   IF ::GetReply()

      ::InetSendAll( ::SocketCon, "RNTO " + cTo + ::cCRLF )
      lResult := ::GetReply()

   ENDIF

   RETURN lResult

METHOD DownLoadFile( cLocalFile, cRemoteFile ) CLASS tIPClientFTP

   LOCAL cPath := ""
   LOCAL cFile := ""
   LOCAL cExt  := ""

   hb_FNameSplit( cLocalFile, @cPath, @cFile, @cExt  )


   DEFAULT cRemoteFile TO cFile + cExt

   ::bEof := .F.
   ::oUrl:cFile := cRemoteFile

   IF ! ::bInitialized

      IF ! Empty( ::oUrl:cPath ) .AND. ! ::CWD( ::oUrl:cPath )
         ::bEof := .T.  // no data for this transaction
         RETURN .F.
      ENDIF

      IF ! ::bUsePasv .AND. ! ::Port()
         RETURN .F.
      ENDIF

      IF ! ::Retr( ::oUrl:cFile )
         ::bEof := .T.  // no data for this transaction
         RETURN .F.
      ENDIF

      // now channel is open
      ::bInitialized := .T.

   ENDIF

   RETURN ::ReadToFile( cLocalFile, , ::nLength )

// Create a new folder

METHOD MKD( cPath ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "MKD " + cPath + ::cCRLF )

   RETURN ::GetReply()

// Delete an existing folder

METHOD RMD( cPath ) CLASS tIPClientFTP

   ::InetSendall( ::SocketCon, "RMD " + cPath + ::cCRLF )

   RETURN ::GetReply()

// Return total file size for <cFileSpec>

METHOD fileSize( cFileSpec ) CLASS tIPClientFTP

   LOCAL aFiles := ::ListFiles( cFileSpec ), nSize := 0, n

   FOR n = 1 TO Len( aFiles )
      nSize += Val( aFiles[n][F_SIZE] ) // Should [7] not be [F_SIZE] ?
   NEXT

   RETURN nSize

// Parse the :list() string into a Directory() compatible 2-dim array

METHOD listFiles( cFileSpec ) CLASS tIPClientFTP

   LOCAL aMonth := { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
   LOCAL cList, aList, aFile, cEntry, nStart, nEnd
   LOCAL cYear, cMonth, cDay, cTime

   cList := ::list( cFileSpec )

   IF Empty( cList )
      RETURN {}
   ENDIF

   aList := hb_ATokens( StrTran( cList, Chr(13 ),'' ), Chr( 10 ) )

   FOR EACH cEntry IN aList

      IF Empty( cEntry ) //PM:09-08-2007 Needed because of the new HB_aTokens()

         ADel( aList, HB_EnumIndex(), .T. )

      ELSE

         aFile         := Array( F_LEN + 3 )
         nStart        := 1
         nEnd          := At( Chr( 32 ), cEntry, nStart )

         // file permissions (attributes)
         aFile[F_ATTR] := SubStr( cEntry, nStart, nEnd - nStart )
         nStart        := nEnd

         // # of links
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         aFile[F_LEN+1] := Val( SubStr( cEntry, nStart, nEnd - nStart ) )
         nStart        := nEnd

         // owner name
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         aFile[F_LEN+2] := SubStr( cEntry, nStart, nEnd - nStart )
         nStart        := nEnd

         // group name
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         aFile[F_LEN+3] := SubStr( cEntry, nStart, nEnd - nStart )
         nStart        := nEnd

         // file size
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         aFile[F_SIZE] := Val( SubStr( cEntry, nStart, nEnd - nStart ) )
         nStart        := nEnd

         // Month
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         cMonth        := SubStr( cEntry, nStart, nEnd - nStart )
         cMonth        := PadL( AScan( aMonth, cMonth ), 2, "0" )
         nStart        := nEnd

         // Day
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         cDay          := SubStr( cEntry, nStart, nEnd - nStart )
         cDay          := PadL( Val( cDay ), 2, "0" )
         nStart        := nEnd

         // year
         DO WHILE cEntry[++nStart] == " " ; ENDDO
         nEnd          := At( Chr( 32 ), cEntry, nStart )
         cYear         := SubStr( cEntry, nStart, nEnd - nStart )
         nStart        := nEnd

         IF ":" $ cYear
            cTime := cYear
            cYear := Str( Year( Date() ), 4, 0 )
         ELSE
            cTime := ""
         ENDIF

         // file name
         DO WHILE cEntry[++nStart] == " " ; ENDDO

         aFile[F_NAME] := SubStr( cEntry, nStart )
         aFile[F_DATE] := SToD( cYear + cMonth + cDay )
         aFile[F_TIME] := cTime

         aList[ HB_EnumIndex() ] := aFile

      ENDIF

   NEXT

   RETURN aList
