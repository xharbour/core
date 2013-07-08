/*
 * $Id: httpcln.prg 9279 2011-02-14 18:06:32Z druzus $
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

#include "hbclass.ch"
#include "tip.ch"

/**
* Inet service manager: http
*/

CLASS tIPClientHTTP FROM tIPClient
   DATA cMethod
   DATA nReplyCode
   DATA cReplyDescr
   DATA nVersion     INIT  1
   DATA nSubversion  INIT  0
   DATA bChunked
   DATA hHeaders     INIT  {=>}
   DATA hCookies     INIT  {=>}
   DATA hFields      INIT  {=>}
   DATA cUserAgent   INIT  "Mozilla/3.0 compatible"
   DATA cAuthMode    INIT ""
   DATA cBoundary
   DATA aAttachments init {}

   METHOD New( oUrl,lTrace, oCredentials)
   METHOD Get( cQuery )
   METHOD Post( cPostData, cQuery )
   METHOD ReadHeaders()
   METHOD Read( nLen )
   METHOD UseBasicAuth()      INLINE   ::cAuthMode := "Basic"
   Method ReadAll()
   Method SetCookie
   Method GetCookies
   Method Boundary
   METHOD Attach(cName,cFileName,cType)
   Method PostMultiPart
   Method WriteAll( cFile )
   DESTRUCTOR httpClnDestructor
HIDDEN:
   METHOD StandardFields()

ENDCLASS

METHOD New( oUrl,lTrace, oCredentials, CAFile,CaPath ) CLASS tIPClientHTTP
   local cFile := "httpcln"
   local n := 0

   ::super:new( oUrl, lTrace, oCredentials,, CAFile,CaPath  )
   if ::oUrl:cProto  == "https"
      ::nDefaultPort := 443
   else
      ::nDefaultPort := 80
   endif
   ::nConnTimeout := 5000
   ::bChunked     := .F.
   if ::ltrace
      if !file( "httpcln.log" )
         ::nHandle := fcreate( "httpcln.log" )
      else
         while file( cFile + alltrim( str( n, 4 ) ) + ".log")
           n++
         enddo
         ::nHandle := fcreate( cFile + alltrim( str( n, 4 ) ) + ".log")
      endif
   endif


   HSetCaseMatch( ::hHeaders, .F. )
RETURN Self


METHOD Get( cQuery ) CLASS tIPClientHTTP
   IF .not. HB_IsString( cQuery )
      cQuery := ::oUrl:BuildQuery()
   ENDIF

   ::InetSendall( ::SocketCon, "GET " + cQuery + " HTTP/1.1" + ::cCRLF )
   ::StandardFields()
   ::InetSendall( ::SocketCon, ::cCRLF )
   IF ::InetErrorCode( ::SocketCon ) ==  0
      RETURN ::ReadHeaders()
   ENDIF
RETURN .F.


METHOD Post( cPostData, cQuery ) CLASS tIPClientHTTP
   LOCAL cData, nI, cTmp,y

   IF HB_IsHash( cPostData )
      cData := ""
      FOR nI := 1 TO Len( cPostData )
         cTmp := HGetKeyAt( cPostData, nI )
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp +"="
         cTmp := HGetValueAt( cPostData, nI )
         cTmp := CStr( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp + "&"
      NEXT
      cData := left( cData, len( cData ) - 1 )
   elseIF HB_IsArray( cPostData )
      cData := ""
      y:=Len(cPostData)
      FOR nI := 1 TO y
         cTmp := cPostData[ nI ,1]
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp +"="
         cTmp := cPostData[ nI,2]
         cTmp := CStr( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp
         IF nI!=y
            cData+="&"
         ENDIF
      NEXT

   ELSEIF HB_IsString( cPostData )
      cData := cPostData
   ELSE
      Alert( "TipClientHTTP_PostRequest: Invalid parameters" )
      RETURN .F.
   ENDIF

   IF .not. HB_IsString( cQuery )
      cQuery := ::oUrl:BuildQuery()
   ENDIF

   ::InetSendall( ::SocketCon, "POST " + cQuery + " HTTP/1.1" + ::cCRLF )
   ::StandardFields()

   IF .not. "Content-Type" IN ::hFields
      ::InetSendall( ::SocketCon, e"Content-Type: application/x-www-form-urlencoded\r\n" )
   ENDIF

   ::InetSendall( ::SocketCon, "Content-Length: " + ;
         LTrim(Str( Len( cData ) ) ) + ::cCRLF )

   // End of header
   ::InetSendall( ::SocketCon, ::cCRLF )

   IF ::InetErrorCode( ::SocketCon  ) ==  0
      ::InetSendall( ::SocketCon, cData )
      ::bInitialized := .T.
      RETURN ::ReadHeaders()
/*   else
      alert("Post InetErrorCode:"+winsockerrorcode(::InetErrorCode( ::SocketCon  )))*/
   ENDIF
RETURN .F.

METHOD StandardFields() CLASS tIPClientHTTP
   LOCAL iCount
   LOCAL oEncoder,cCookies

   ::InetSendall( ::SocketCon, "Host: " + ::oUrl:cServer + ::cCRLF )
   ::InetSendall( ::SocketCon, "User-agent: " + ::cUserAgent + ::cCRLF )
   ::InetSendall( ::SocketCon, "Connection: close" + ::cCRLF )

   // Perform a basic authentication request
   IF ::cAuthMode == "Basic" .and. .not. ("Authorization" in ::hFields)
      oEncoder := TIPEncoderBase64():New()
      oEncoder:bHttpExcept := .T.
      ::InetSendall( ::SocketCon, "Authorization: Basic " +;
          oEncoder:Encode(  ::oUrl:cUserID + ":" + ::oUrl:cPassword ) + ::cCRLF )
   ENDIF


   // send cookies
   cCookies:=::getCookies()
   IF ! Empty( cCookies )
      ::InetSendall( ::SocketCon, "Cookie: " + cCookies+::cCRLF)
   ENDIF

   //Send optional Fields
   FOR iCount := 1 TO Len( ::hFields )
      ::InetSendall( ::SocketCon, HGetKeyAt( ::hFields, iCount ) +;
         ": " + HGetValueAt( ::hFields, iCount ) + ::cCRLF )
   NEXT

RETURN .T.



METHOD ReadHeaders(lClear) CLASS tIPClientHTTP
   LOCAL cLine, nPos, aVersion
   LOCAL aHead

   // Now reads the fields and set the content lenght
   cLine := ::InetRecvLine( ::SocketCon, @nPos, 500 )
   IF Empty( cLine )
      // In case of timeout or error on receiving
      RETURN .F.
   ENDIF

   // Get Protocol version
   aVersion := HB_Regex( "^HTTP/(.)\.(.) ([0-9][0-9][0-9]) +(.*)$", cLine )
   ::cReply := cLine

   IF aVersion == NIL
      ::nVersion := 0
      ::nSubversion := 9
      ::nReplyCode := 0
      ::cReplyDescr := ""
   ELSE
      ::nVersion := Val(aVersion[2])
      ::nSubversion := Val( aVersion[3] )
      ::nReplyCode := val( aVersion[4] )
      ::cReplyDescr := aVersion[5]
   ENDIF

   ::nLength := -1
   ::bChunked := .F.
   cLine := ::InetRecvLine( ::SocketCon, @nPos, 500 )
   IF !lClear=.f. .AND. !empty(::hHeaders)
      ::hHeaders:={=>}
   ENDIF
   DO WHILE ::InetErrorCode( ::SocketCon ) == 0 .and. .not. Empty( cLine )
      aHead := HB_RegexSplit( ":", cLine,,, 1 )
      IF aHead == NIL .or. Len( aHead ) != 2
         cLine := ::InetRecvLine( ::SocketCon, @nPos, 500 )
         LOOP
      ENDIF

      ::hHeaders[ aHead[1] ] := LTrim(aHead[2])
      DO CASE

         // RFC 2068 forces to discard content length on chunked encoding
         CASE lower( aHead[1] ) == "content-length" .and. .not. ::bChunked
            cLine := Substr( cLine, 16 )
            ::nLength := Val( cLine )

         // as above
         CASE lower( aHead[1] ) == "transfer-encoding"
            IF At( "chunked", lower( cLine ) ) > 0
               ::bChunked := .T.
               ::nLength := -1
            ENDIF
         CASE lower( aHead[1] ) == "set-cookie"
            ::setCookie(aHead[2])

      ENDCASE
      cLine := ::InetRecvLine( ::SocketCon, @nPos, 500 )
   ENDDO
   IF ::InetErrorCode( ::SocketCon ) != 0
      RETURN .F.
   ENDIF
RETURN .T.


METHOD Read( nLen ) CLASS tIPClientHTTP
   LOCAL cData, nPos, cLine, aHead

   IF .not. ::bInitialized
      ::bInitialized := .T.
      IF .not. ::Get()
         RETURN NIL
      ENDIF
   ENDIF

   /* On HTTP/1.1 protocol, content lenght can be in hex format before each chunk.
      The chunk header is read each time nLength is -1; While reading the chunk,
      nLenght is set to nRead plus the expected chunk size. After reading the
      chunk, the footer is discarded, and nLenght is reset to -1.
   */
   IF ::nLength == -1 .and. ::bChunked
      cLine := ::InetRecvLine( ::SocketCon, @nPos, 1024 )

      IF Empty( cLine )
         RETURN NIL
      ENDIF

      // if this is the last chunk ...
      IF cLine == "0"

         // read the footers.
         cLine := ::InetRecvLine( ::SocketCon, @nPos, 1024 )
         DO WHILE .not. Empty( cLine )
            // add Headers to footers
            aHead := HB_RegexSplit( ":", cLine,,, 1 )
            IF aHead != NIL
               ::hHeaders[ aHead[1] ] := LTrim(aHead[2])
            ENDIF

            cLine := ::InetRecvLine( ::SocketCon, @nPos, 1024 )
         ENDDO

         // we are done
         ::bEof := .T.
         RETURN NIL
      ENDIF

      // A normal chunk here

      // Remove the extensions
      nPos := at( ";", cLine )
      IF nPos > 0
         cLine := Substr( cLine, 1, nPos - 1 )
      ENDIF

      // Convert to length
      // Set length so that super::Read reads in at max cLine bytes.
      ::nLength := HexToNum( cLine ) + ::nRead

   ENDIF

   // nLen is normalized by super:read()
   cData := ::super:Read( nLen )

   // If bEof is set with chunked encoding, this means that the whole chunk has been read;
   IF ::bEof .and. ::bChunked
      ::bEof := .F.
      ::nLength := -1
     //chunked data is followed by a blank line
      cLine := ::InetRecvLine( ::SocketCon, @nPos, 1024 )

   ENDIF

RETURN cData

METHOD ReadAll() CLASS tIPClientHTTP

   local cOut:='', cChunk
   IF .not. ::bInitialized
      ::bInitialized := .T.
      IF .not. ::Get()
         RETURN NIL
      ENDIF
   ENDIF
   IF ::bChunked
      cChunk:=::read()
      while cChunk!=nil
         cOut+=cChunk
      // ::nLength:=-1
         cChunk:=::read()
      end
   else
      return(::read())
   endif
   return(cOut)

METHOD setCookie(cLine) CLASS tIPClientHTTP
   //docs from http://www.ietf.org/rfc/rfc2109.txt
   local aParam
   local cHost, cPath, cName, cValue, aElements, cElement
   local cDefaultHost:=::oUrl:cServer, cDefaultPath:=::oUrl:cPath
   local x,y
   IF empty(cDefaultPath)
      cDefaultPath:='/'
   ENDIF
   //this function currently ignores expires, secure and other tags that may be in the cookie for now...
//   ?'Setting COOKIE:',cLine
   aParam := HB_RegexSplit( ";", cLine )
   cName:=cValue:=''
   cHost:=cDefaultHost
   cPath:=cDefaultPath
   y:=len(aParam)
   FOR x:=1 to y
      aElements := HB_RegexSplit( "=", aParam[x], 1)
      IF len(aElements)==2
         IF x=1
            cName:=alltrim(aElements[1])
            cValue:=alltrim(aElements[2])
         else
            cElement:=upper(alltrim(aElements[1]))
            do case
            //case cElement=='EXPIRES'
            case cElement=='PATH'
               cPath:=alltrim(aElements[2])
            case cElement=='DOMAIN'
               cHost:=alltrim(aElements[2])
            endcase
         ENDIF
      ENDIF
   next
   IF !empty(cName)
      //cookies are stored in hashes as host.path.name
      //check if we have a host hash yet
      if !HHASKEY(::hCookies,cHost)
         ::hCookies[cHost]:={=>}
      endif
      if !HHASKEY(::hCookies[cHost],cPath)
         ::hCookies[cHost][cPath]:={=>}
      endif
      ::hCookies[cHost][cPath][cName]:=cValue
   ENDIF
return NIL

METHOD getcookies(cHost,cPath) CLASS tIPClientHTTP
   local x,y,aDomKeys:={},aKeys,z,cKey,aPathKeys,nPath
   local a, b, cOut := '', c, d

   IF cHost=nil
      cHost:=::oUrl:cServer
   ENDIF
   IF cPath=nil
      cPath:=::oUrl:cPath
      IF empty(cPath)
         cPath:='/'
      ENDIF
   ENDIF
   IF empty(cHost)
      return(cOut)
   ENDIF

   //tail matching the domain
   aKeys:=hgetkeys(::hCookies)
   y:=len(aKeys)
   z:=len(cHost)
   cHost:=upper(cHost)
   FOR x := 1 TO y
      cKey:=upper(aKeys[x])
      IF upper(right(cKey,z))==cHost.and.(len(cKey)=z .OR. substr(aKeys[x],0-z,1)=='.')
         aadd(aDomKeys,aKeys[x])
      ENDIF
   NEXT
   //more specific paths should be sent before lesser generic paths.
   asort(aDomKeys,,, {|cX,cY| len(cX) > len(cY)} )
   y:=len(aDomKeys)
   //now that we have the domain matches we have to do path matchine
   nPath:=len(cPath)
   FOR x := 1 TO y
      aKeys:=hgetkeys(::hCookies[aDomKeys[x]])
      aPathKeys:={}
      b:=len(aKeys)
      FOR  a:= 1 TO b
         cKey:=aKeys[a]
         z:=len(cKey)
         IF cKey=='/'.or.(z<=nPath.and.substr(cKey,1,nPath)==cKey)
            aadd(aPathKeys,aKeys[a])
         ENDIF
      NEXT
      asort(aPathKeys,,, {|cX,cY| len(cX) > len(cY)} )
      b:=len(aPathKeys)
      FOR a := 1 TO b
         aKeys:=hgetkeys(::hCookies[aDomKeys[x]][aPathKeys[a]])
         d:=len(aKeys)
         FOR c := 1 TO d
            IF !empty(cOut)
               cOut+='; '
            ENDIF
            cOut+=aKeys[c]+'='+::hCookies[aDomKeys[x]][aPathKeys[a]][aKeys[c]]
         NEXT
      NEXT
   NEXT
return(cOut)

METHOD Boundary(nType) CLASS tIPClientHTTP
   /*
   nType: 0=as found as the separator in the stdin stream
         1=as found as the last one in the stdin stream
         2=as found in the CGI enviroment
   Examples:
   -----------------------------41184676334  //in the body or stdin stream
   -----------------------------41184676334--   //last one of the stdin stream
---------------------------41184676334 //in the header or CGI envirnment
   */

   local cBound:=::cBoundary
   LOCAL i
   IF nType=nil
      nType=0
   ENDIF
   IF empty(cBound)
      cBound:=replicate('-',27)+space(11)
      FOR i := 28 TO 38
         cBound[i] := str(int(HB_Random(0, 9 )),1,0)
      NEXT
      ::cBoundary:=cBound
   endif
   cBound:=if(nType<2,'--','')+cBound+if(nType=1,'--','')
   RETURN(cBound)

METHOD Attach(cName,cFileName,cType) CLASS tIPClientHTTP
   aadd(::aAttachments,{cName,cFileName,cType})
return(nil)

METHOD PostMultiPart( cPostData, cQuery ) CLASS tIPClientHTTP
   LOCAL cData:="", nI, cTmp,y,cBound:=::boundary()
   local cCrlf:=::cCRlf,oSub
   local nPos
   local cFilePath,cName,cFile,cType
   local nFile,cBuf,nBuf,nRead

   IF empty(cPostData)
   elseif HB_IsHash( cPostData )
      FOR nI := 1 TO Len( cPostData )
         cTmp := HGetKeyAt( cPostData, nI )
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cBound+cCrlf+'Content-Disposition: form-data; name="'+cTmp +'"'+cCrlf+cCrLf
         cTmp := HGetValueAt( cPostData, nI )
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp+cCrLf
      NEXT
   elseIF HB_IsArray( cPostData )
      y:=Len(cPostData)
      FOR nI := 1 TO y
         cTmp := cPostData[ nI ,1]
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cBound+cCrlf+'Content-Disposition: form-data; name="'+cTmp +'"'+cCrlf+cCrLf
         cTmp := cPostData[ nI,2]
         cTmp := CStr( cTmp )
         cTmp := AllTrim( cTmp )
         cTmp := TipEncoderUrl_Encode( cTmp )
         cData += cTmp+cCrLf
      NEXT

   ELSEIF HB_IsString( cPostData )
      cData := cPostData
   ENDIF
   FOR each oSub in ::aAttachments
      cName:=oSub[1]
      cFile:=oSub[2]
      cType:=oSub[3]
      cTmp:=strtran(cFile,'/','\')
      if ( nPos := rat( "\", cTmp ) ) != 0
          cFilePath := substr( cTmp, 1, nPos )
      elseif ( nPos := rat( ":", cTmp ) ) != 0
          cFilePath := substr( cTmp, 1, nPos )
      else
          cFilePath := ""
      endif
      cTmp:=substr(cFile,Len(cFilePath)+1)
      IF empty(cType)
         cType:='text/html'
      ENDIF
      cData += cBound+cCrlf+'Content-Disposition: form-data; name="'+cName +'"; filename="'+cTmp+'"'+cCrlf+'Content-Type: '+cType+cCrLf+cCrLf
      //hope this is not a big file....
      nFile:=fopen(cFile)
      nbuf:=8192
      nRead:=nBuf
      cBuf:=space(nBuf)
      while nRead=nBuf
         //nRead=FRead( nFile,@cBuf,nBuf)
         cBuf:=FReadstr( nFile,nBuf)
         nRead:=len(cBuf)
/*         IF nRead<nBuf
            cBuf:=pad(cBuf,nRead)
         ENDIF
*/
         cData+=cBuf
      end
      fClose(nFile)
      cData+=cCrlf
   NEXT
   cData+=cBound+'--'+cCrlf
   IF .not. HB_IsString( cQuery )
      cQuery := ::oUrl:BuildQuery()
   ENDIF

   ::InetSendall( ::SocketCon, "POST " + cQuery + " HTTP/1.1" + ::cCRLF )
   ::StandardFields()

   IF .not. "Content-Type" IN ::hFields
      ::InetSendall( ::SocketCon, e"Content-Type: multipart/form-data; boundary="+::boundary(2)+::cCrlf )
   ENDIF

   ::InetSendall( ::SocketCon, "Content-Length: " + ;
         LTrim(Str( Len( cData ) ) ) + ::cCRLF )
   // End of header
   ::InetSendall( ::SocketCon, ::cCRLF )

   IF ::InetErrorCode( ::SocketCon  ) ==  0
      ::InetSendall( ::SocketCon, cData )
      ::bInitialized := .T.
      RETURN ::ReadHeaders()
/*   else
      alert("Post InetErrorCode:"+winsockerrorcode(::InetErrorCode( ::SocketCon  )))*/
   ENDIF
RETURN .F.

METHOD WriteAll( cFile ) CLASS tIPClientHTTP

   local nFile
   local lSuccess
   local nLen
   local cStream

   cStream := ::ReadAll()

   nLen := len( cStream )

   nFile := fcreate( cFile )

   if nFile != 0
      lSuccess := ( fwrite( nFile, cStream, nLen ) == nLen )
      fclose( nFile )
   else
      lSuccess := .f.
   endif

   RETURN lSuccess


PROCEDURE httpClnDestructor CLASS tIPClientHTTP
   if ::ltrace .and. ::nhandle > -1    
      fClose( ::nHandle )
      ::nhandle := -1 
   endif


RETURN
