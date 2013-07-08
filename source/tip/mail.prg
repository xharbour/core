/*
 * $Id: mail.prg 9935 2013-03-21 08:28:29Z zsaulius $
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

/* 2007-04-11, Hannes Ziegler <hz AT knowlexbase.com>
   Added method :setHeader()
   Added method :attachFile()
   Added method :detachFile()
   Added method :getFileName()
   Added method :isMultiPart()
   Added method :getMultiParts()
*/

#include "hbclass.ch"

CLASS TipMail

   DATA hHeaders
// received fields may be more than once.
   DATA aReceived INIT {}

   METHOD New( cBody, oEncoder )    Constructor
   METHOD SetBody( cBody )
   METHOD GetBody()
   METHOD GetRawBody()              INLINE ::cBody
   METHOD SetEncoder( cEncoder )

   /*
   METHOD FWrite( nFile )
   METHOD FRead( nFile )
   METHOD Send( sSocket )
   METHOD Recv( sSocket )
   */
   METHOD FromString( cString )
   METHOD ToString()

   METHOD GetFieldPart( cField )
   METHOD GetFieldOption( cField )
   METHOD SetFieldPart( cField, cValue )
   METHOD SetFieldOption( cField, cValue )

   METHOD GetContentType() INLINE ::GetFieldPart( "Content-Type" )
   METHOD GetCharEncoding() INLINE ::GetFieldOption( "Content-Type", "encoding" )

   METHOD Attach( oSubPart )
   METHOD NextAttachment()
   METHOD CountAttachments()  INLINE Len( ::aAttachments )
   METHOD GetAttachment()
   METHOD ResetAttachment()   INLINE ::nAttachPos := 1

   METHOD MakeBoundary()

   METHOD isMultiPart()
   METHOD getMultiParts()

   METHOD setHeader( cSubject, cFrom, cTo, cCC, cBCC )
   METHOD attachFile( cFileName )
   METHOD detachFile( cPath )
   METHOD getFileName()
   HIDDEN:
   DATA cBody
   DATA lBodyEncoded init .F.
   DATA oEncoder
   DATA aAttachments
   DATA nAttachPos   INIT 1

ENDCLASS

METHOD New( cBody, oEncoder ) CLASS TipMail

// Set header fileds to non-sensitive
   ::hHeaders := HSetCaseMatch( { => }, .F. )
   ::aAttachments := {}

   IF ValType( oEncoder ) $ "CO"
      ::setEncoder( oEncoder )
   ENDIF

   IF cBody != NIL
      IF ::oEncoder != NIL
         ::cBody := ::oEncoder:Encode( cBody )
         ::hHeaders[ "Content-Transfer-Encoding" ] := ::oEncoder:cName
      ELSE
         ::cBody := cBody
      ENDIF
      ::hHeaders[ "Content-Length" ] := LTrim( Str( Len( ::cBody ) ) )
   ENDIF

   RETURN Self

METHOD SetEncoder( cEnc ) CLASS TipMail

   IF HB_ISSTRING( cEnc )
      ::oEncoder := TIp_GetEncoder( cEnc )
   ELSE
      ::oEncoder := cEnc
   ENDIF
   ::hHeaders[ "Content-transfer-encoding" ] := ::oEncoder:cName

   RETURN .T.

METHOD SetBody( cBody ) CLASS TipMail

   IF ::oEncoder != NIL
      ::cBody := ::oEncoder:Encode( cBody )
      ::lBodyEncoded := .T.  //GD needed to prevent an extra crlf from being appended
   ELSE
      ::cBody := cBody
   ENDIF
//::hHeaders[ "Content-Length" ] := Ltrim( Str( Len( cBody ) ) )  //GD -not needed

   RETURN .T.

METHOD GetBody() CLASS TipMail

   IF ::cBody == NIL
      RETURN NIL
   ELSEIF ::oEncoder != NIL
      RETURN ::oEncoder:Decode( ::cBody )
   ENDIF

   RETURN ::cBody

METHOD GetFieldPart( cPart ) CLASS TipMail

   LOCAL nPos, cEnc

   nPos := HGetPos( ::hHeaders, cPart )
   IF nPos == 0
      RETURN ""
   ELSE
      cEnc := HGetValueAt( ::hHeaders, nPos )
      nPos := At( ";", cEnc )
      IF nPos != 0
         cEnc := SubStr( cEnc, 1, nPos - 1 )
      ENDIF
   ENDIF

   RETURN cEnc

METHOD GetFieldOption( cPart, cOption ) CLASS TipMail

   LOCAL nPos, aMatch
   LOCAL cEnc

   nPos := HGetPos( ::hHeaders, cPart )
   IF nPos == 0
      RETURN ""
   ELSE
      cEnc := HGetValueAt( ::hHeaders, nPos )
      // Case insensitive check
      aMatch := hb_regex( ";\s*" + cOption + "\s*=\s*([^;]*)", cEnc, .F. )
      IF aMatch != NIL
         cEnc := aMatch[2]
      ELSE
         RETURN ""
      ENDIF
   ENDIF

   RETURN cEnc

METHOD SetFieldPart( cPart, cValue ) CLASS TipMail

   LOCAL nPos, cEnc

   nPos := HGetPos( ::hHeaders, cPart )
   IF nPos == 0
      ::hHeaders[ cPart ] := cValue
   ELSE
      cEnc := HGetValueAt( ::hHeaders, nPos )
      nPos := At( ";", cEnc )
      IF nPos == 0
         ::hHeaders[ cPart ] := cValue
      ELSE
         ::hHeaders[ cPart ] := cValue + SubStr( cEnc, nPos )
      ENDIF
   ENDIF

   RETURN .T.

METHOD SetFieldOption( cPart, cOption, cValue ) CLASS TipMail

   LOCAL nPos, aMatch
   LOCAL cEnc

   nPos := HGetPos( ::hHeaders, cPart )
   IF nPos == 0
      RETURN .F.
   ELSE
      cEnc := HGetValueAt( ::hHeaders, nPos )
      aMatch := hb_regex( "(.*?;\s*)" + cOption + "\s*=[^;]*(.*)?", cEnc, .F. )
      IF Empty( aMatch )
         ::hHeaders[ cPart ] := cEnc += "; " + cOption + '="' + cValue + '"'
      ELSE
         ::hHeaders[ cPart ] := aMatch[2] + cOption + '="' + ;
            cValue + '"' + aMatch[3]
      ENDIF
   ENDIF

   RETURN .T.

METHOD Attach( oSubPart ) CLASS TipMail

   IF HB_ISOBJECT( oSubPart ) .AND. oSubPart:ClassName == "TIPMAIL"
      // reset wrong content-type
      IF At( "multipart/", Lower( ::GetFieldPart("Content-Type" ) ) ) == 0
         ::hHeaders[ "Content-Type" ] := "multipart/mixed"
      ENDIF

      AAdd( ::aAttachments, oSubPart )
      RETURN .T.
   ELSE
      Alert( "TipMail:Attach() must be called with another TipMail object" )
   ENDIF

   RETURN .F.

METHOD NextAttachment() CLASS TipMail

   IF ::nAttachPos > Len( ::aAttachments )
      RETURN NIL
   ELSE
      ::nAttachPos ++
   ENDIF

   RETURN ::aAttachments[ ::nAttachPos - 1 ]

METHOD GetAttachment() CLASS TipMail

   IF ::nAttachPos > Len( ::aAttachments )
      RETURN NIL
   ENDIF

   RETURN ::aAttachments[ ::nAttachPos ]

METHOD ToString() CLASS TipMail

   LOCAL cBoundary, cElem, i
   LOCAL cRet := ""

// this is a multipart message; we need a boundary
   IF Len( ::aAttachments ) > 0
      ::hHeaders[ "Mime-Version" ] := "1.0"
   ENDIF

   IF Len( ::aAttachments ) > 0
      //Reset failing content type
      IF At( "multipart/", Lower( ::GetFieldPart("Content-Type" ) ) ) == 0
         ::hHeaders[ "Content-Type" ] := "multipart/mixed"
      ENDIF

      // have we got it already?
      cBoundary := ::GetFieldOption( "Content-Type", "Boundary" )
      IF Empty( cBoundary )
         cBoundary := ::MakeBoundary()
         IF .NOT. ::SetFieldOption( "Content-Type", "Boundary", cBoundary )
            ::hHeaders[ "Content-Type" ] := ;
               'multipart/mixed; boundary="' + cBoundary + '"'
         ENDIF
      ENDIF
   ENDIF

// Begin output the fields
// Presenting them in a "well-known" order
   IF "Return-Path" IN ::hHeaders
      cRet += "Return-Path: " + ::hHeaders[ "Return-Path" ] + e"\r\n"
   ENDIF
   IF "Delivered-To" IN ::hHeaders
      cRet += "Delivered-To: " + ::hHeaders[ "Delivered-To" ] + e"\r\n"
   ENDIF
   FOR EACH cElem IN ::aReceived
      cRet += "Received: " + cElem + e"\r\n"
   NEXT
   IF "Date" IN ::hHeaders
      cRet += "Date: " + ::hHeaders[ "Date" ] + e"\r\n"
   ENDIF
   IF "From" IN ::hHeaders
      cRet += "From: " + ::hHeaders[ "From" ] + e"\r\n"
   ENDIF
   IF "To" IN ::hHeaders
      cRet += "To: " + ::hHeaders[ "To" ] + e"\r\n"
   ENDIF
   IF "Subject" IN ::hHeaders
      cRet += "Subject: " + ::hHeaders[ "Subject" ] + e"\r\n"
   ENDIF
   IF Len( ::aAttachments ) > 0
      cRet += "Mime-Version:" + ::hHeaders[ "Mime-Version" ] + e"\r\n"
   ENDIF

   FOR i := 1 TO Len( ::hHeaders )
      cElem := Lower( HGetKeyAt( ::hHeaders, i ) )
      IF !( cElem == "return-path" ) .AND. ;
            !( cElem == "delivered-to" ) .AND. ;
            !( cElem == "date" ) .AND. ;
            !( cElem == "from" ) .AND. ;
            !( cElem == "to" ) .AND. ;
            !( cElem == "subject" ) .AND. ;
            !( cElem == "mime-version" )
         cRet += HGetKeyAt( ::hHeaders, i ) + ": " + ;
            HGetValueAt( ::hHeaders, i ) + e"\r\n"
      ENDIF
   NEXT

// end of Header
   cRet += e"\r\n"

//Body
   IF .NOT. Empty( ::cBody )
      IF Empty( ::aAttachments )
         //cRet += ::cBody +if(lAttachment,'', e"\r\n")
         cRet += ::cBody + if( ::lBodyEncoded, '', e"\r\n" )
      ELSE
         //GD - if there are attachements the body of the message has to be treated as an attachment.
         cRet += "--" + cBoundary + e"\r\n"
         cRet += "Content-Type: text/plain; charset=ISO-8859-1; format=flowed" + e"\r\n"
         cRet += "Content-Transfer-Encoding: 7bit" + e"\r\n"
         cRet += "Content-Disposition: inline" + e"\r\n" + e"\r\n"
         cRet += ::cBody + e"\r\n"
      ENDIF

   ENDIF

   IF .NOT. Empty( ::aAttachments )
      //Eventually go with mime multipart
      FOR i := 1 TO Len( ::aAttachments )
         cRet += "--" + cBoundary + e"\r\n"
         cRet += ::aAttachments[i]:ToString()
      NEXT
      cRet += "--" + cBoundary + "--" + e"\r\n"
   ENDIF

   RETURN cRet

METHOD FromString( cMail, cBoundary, nPos ) CLASS TipMail

   LOCAL oSubSection, cSubBoundary
   LOCAL nLinePos, nSplitPos, nBodyPos
   LOCAL cValue, cLastField

   IF Len( ::aAttachments ) > 0
      ::aAttachments := {}
   ENDIF

   IF Len( ::hHeaders ) > 0
      ::hHeaders := HSetCaseMatch( { => } , .F. )
   ENDIF

   IF Len( ::aReceived ) > 0
      ::aReceived := {}
   ENDIF

// Part 1: parsing header
   IF nPos == NIL
      nPos := 1
   ENDIF

   IF cMail == NIL
      cMail := ""
   ENDIF

   nLinePos := At( e"\r\n", cMail, nPos )
   DO WHILE nLinePos > nPos
      // going on with last field?
      IF ( cMail[ nPos ] == " " .OR. cMail[ nPos ] == e"\t" );
            .AND. cLastField != NIL
         cValue := LTrim( SubStr( cMail, nPos, nLinePos - nPos ) )
         IF Lower( cLastField ) == "received"
            ::aReceived[Len(::aReceived)] += " " + cValue
         ELSE
            ::hHeaders[ cLastField ] += " " + cValue
         ENDIF

      ELSE
         nSplitPos := At( ":", cMail, nPos )
         cLastField := SubStr( cMail, nPos, nSplitPos - nPos )
         cValue := LTrim( SubStr( cMail, nSplitPos + 1, nLinePos - nSplitPos - 1 ) )
         IF Lower( cLastField ) == "received"
            AAdd( ::aReceived, cValue )
         ELSE
            ::hHeaders[ cLastField ] := cValue
         ENDIF
      ENDIF

      nPos := nLinePos + 2
      nLinePos := At( e"\r\n", cMail, nPos )
      //Prevents malformed body to affect us
      IF cBoundary != NIL .AND. At( "--" + cBoundary, cMail, nPos ) == 1
         RETURN 0
      ENDIF
   ENDDO

// now we may have a body or a multipart message; multipart
// messages may also have a "fake" body, that is usually not
// displayed, between their headers and the first multipart
// boundary.

   IF "Content-Transfer-Encoding" IN ::hHeaders
      ::oEncoder := TIp_GetEncoder( ::hHeaders["Content-Transfer-Encoding"] )
   ENDIF

// se if we have subparts:
   IF At( "multipart/", Lower( ::GetFieldPart("Content-Type" ) ) ) > 0
      cSubBoundary := ::GetFieldOption( "Content-Type", "Boundary" )
      //strip " on boundary
      IF cSubBoundary[1] == '"'
         cSubBoundary := SubStr( cSubBoundary, 2, Len( cSubBoundary ) - 2 )
      ENDIF
   ENDIF

   nPos := nLinePos + 2
   nBodyPos := nPos
   nLinePos := At( e"\r\n", cMail, nPos )

   DO WHILE nLinePos >= nPos
      // Avoid useless tests for empty lines
      IF nLinePos == nPos
         nPos += 2
         nLinePos := At( e"\r\n", cMail, nPos )
         LOOP
      ENDIF

      //have we met the boundary?
      IF cBoundary != NIL .AND. At( "--" + cBoundary, cMail, nPos ) == nPos
         EXIT
      ENDIF

      //Have we met a section?
      IF cSubBoundary != NIL .AND. ;
            At( "--" + cSubBoundary, cMail, nPos ) == nPos

         //is it the last subsection?
         IF At( "--", cMail, nPos + Len( cSubBoundary ) + 2, nLinePos ) > 0
            EXIT
         ENDIF

         // set our body
         IF nBodyPos > 0
            ::cBody := SubStr( cMail, nBodyPos, nPos - nBodyPos )
            nBodyPos := 0
         ENDIF

         // Add our subsection
         oSubSection := TipMail():New()
         nPos := oSubSection:FromString( cMail, cSubBoundary, ;
            nLinePos + 2 )

         IF nPos > 0
            AAdd( ::aAttachments, oSubSection )
         ELSE
            RETURN 0
         ENDIF
         // I must stay on the boundary found by the subsection to
         // enter in this part of the loop again.

      ELSE
         //nPos := nLinePos + 2
         /* 04/05/2004 - <maurilio.longo@libero.it>
            Instead of testing every single line of mail until we find next boundary, if there is a boundary we
            jump to it immediatly, this saves thousands of EOL test and makes splitting of a string fast
         */
         nPos := iif( ! Empty( cSubBoundary ), At( "--" + cSubBoundary, cMail, nPos ), iif( ! Empty(cBoundary ), At("--" + cBoundary, cMail, nPos ), nLinePos + 2 ) )
      ENDIF

      nLinePos := At( e"\r\n", cMail, nPos )
   ENDDO

// set our body if needed
   IF nBodyPos > 0
      ::cBody := SubStr( cMail, nBodyPos, nPos - nBodyPos )
   ENDIF

   RETURN nPos

METHOD MakeBoundary() CLASS TipMail

   LOCAL cBound := "=_0" + Space( 17 )
   LOCAL i

   FOR i := 4 TO 20
      cBound[i] := Chr( hb_Random( 0, 25 ) + Asc( "A" ) )
   NEXT

   cBound += "_TIP_" + StrTran( DToC( Date() ), "/", "" ) + ;
      "_" + StrTran( Time(), ":", "" )

   RETURN cBound

METHOD setHeader( cSubject, cFrom, cTo, cCC, cBCC ) CLASS TipMail

   LOCAL aTo, aCC, aBCC, i, imax

   IF ValType( csubject ) <> "C"
      cSubject := ""
   ENDIF

   IF ValType( cFrom ) <> "C"
      RETURN .F.
   ENDIF

   IF ValType( cTo ) == "C"
      aTo := { cTo }
   ELSEIF ValType( cTo ) == "A"
      aTo := cTo
   ENDIF

   IF ValType( cCC ) == "C"
      aCC := { cCC }
   ELSEIF ValType( cCC ) == "A"
      aCC := cCC
   ENDIF

   IF ValType( cBCC ) == "C"
      aBCC := { cBCC }
   ELSEIF ValType( cBCC ) == "A"
      aBCC := cBCC
   ENDIF

   IF aTO == NIL
      RETURN .F.
   ENDIF

   IF .NOT. ::setFieldPart( "Subject", cSubject )
      RETURN .F.
   ENDIF

   IF .NOT. ::setFieldPart( "From"   , cFrom    )
      RETURN .F.
   ENDIF

   cTo  := aTO[1]
   imax := Len( aTO )
   FOR i := 2 TO imax
      cTo += "," + InetCrlf() + Chr( 9 ) + aTo[i]
   NEXT

   IF .NOT. ::setFieldPart( "To", cTo )
      RETURN .F.
   ENDIF

   IF aCC != NIL
      cCC  := aCC[1]
      imax := Len( aCC )
      FOR i := 2 TO imax
         cCC += "," + InetCrlf() + Chr( 9 ) + aCC[i]
      NEXT

      IF .NOT. ::setFieldPart( "Cc", cCC )
         RETURN .F.
      ENDIF
   ENDIF

   IF aBCC != NIL
      cBCC  := aBCC[1]
      imax := Len( aBCC )
      FOR i := 2 TO imax
         cBCC += "," + InetCrlf() + Chr( 9 ) + aBCC[i]
      NEXT

      IF .NOT. ::setFieldPart( "Bcc", cBCC )
         RETURN .F.
      ENDIF
   ENDIF

   RETURN .T.

METHOD attachFile( cFileName ) CLASS TipMail

   LOCAL cContent := MemoRead( cFileName )
   LOCAL cFname, cFext
   LOCAL cMimeType
   LOCAL cDelim   := hb_osPathSeparator()
   LOCAL oAttach

   IF Empty( cContent )
      RETURN .F.
   ENDIF

   hb_FNameSplit( cFileName, , @cFname, @cFext )
   cMimeType := HB_SetMimeType( cFileName, cFname, cFext )

   IF ".html" in Lower( cFext ) .OR. ".htm" in Lower( cFext )
      cMimeType += "; charset=ISO-8859-1"
   ENDIF

//   oAttach   := TIPMail():new( cContent, "base64" )
   IF Lower( cFileName ) LIKE ".+\.(vbd|asn|asz|asd|pqi|tsp|exe|sml|ofml)"    .OR. ;
         Lower( cFileName ) LIKE ".+\.(pfr|frl|spl|gz||stk|ips|ptlk|hqx|mbd)"    .OR. ;
         Lower( cFileName ) LIKE ".+\.(mfp|pot|pps|ppt|ppz|doc|n2p|bin|class)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(lha|lzh|lzx|dbf|cdx|dbt|fpt|ntx|oda)"     .OR. ;
         Lower( cFileName ) LIKE ".+\.(axs|zpa|pdf|ai|eps|ps|shw|qrt|rtc|rtf)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(smp|dst|talk|tbk|vmd|vmf|wri|wid|rrf)"    .OR. ;
         Lower( cFileName ) LIKE ".+\.(wis|ins|tmv|arj|asp|aabaam|aas|bcpio)"    .OR. ;
         Lower( cFileName ) LIKE ".+\.(vcd|chat|cnc|coda|page|z|con|cpio|pqf)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(csh|cu|csm|dcr|dir|dxr|swa|dvi|evy|ebk)"  .OR. ;
         Lower( cFileName ) LIKE ".+\.(gtar|hdf|map|phtml|php3|ica|ipx|ips|js)"  .OR. ;
         Lower( cFileName ) LIKE ".+\.(latex|bin|mif|mpl|mpire|adr|wlt|nc|cdf)"  .OR. ;
         Lower( cFileName ) LIKE ".+\.(npx|nsc|pgp|css|sh||shar|swf|spr|sprite)" .OR. ;
         Lower( cFileName ) LIKE ".+\.(sit|sca|sv4cpio|sv4crc|tar|tcl|tex)"      .OR. ;
         Lower( cFileName ) LIKE ".+\.(texinfo|texi|tlk|t|tr|roff|man|mems)"     .OR. ;
         Lower( cFileName ) LIKE ".+\.(alt|che|ustar|src|xls|xlt|zip|au|snd)"    .OR. ;
         Lower( cFileName ) LIKE ".+\.(es|gsm|gsd|rmf|tsi|vox|wtx|aif|aiff)"     .OR. ;
         Lower( cFileName ) LIKE ".+\.(aifc|cht|dus|mid|midi|mp3|mp2|m3u|ram)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(ra|rpm|stream|rmf|vqf|vql|vqe|wav|wtx)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(mol|pdb|dwf|ivr|cod|cpi|fif|gif|ief)"     .OR. ;
         Lower( cFileName ) LIKE ".+\.(jpeg|jpg|jpe|rip|svh|tiff|tif|mcf|svf)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(dwg|dxf|wi|ras|etf|fpx|fh5|fh4|fhc|dsf)"  .OR. ;
         Lower( cFileName ) LIKE ".+\.(pnm|pbm|pgm|ppm|rgb|xbm|xpm|xwd|dig)"     .OR. ;
         Lower( cFileName ) LIKE ".+\.(push|wan|waf||afl|mpeg|mpg|mpe|qt|mov)"   .OR. ;
         Lower( cFileName ) LIKE ".+\.(viv|vivo|asf|asx|avi|movie|vgm|vgx)"      .OR. ;
         Lower( cFileName ) LIKE ".+\.(xdr|vgp|vts|vtts|3dmf|3dm|qd3d|qd3)"      .OR. ;
         Lower( cFileName ) LIKE ".+\.(xml)"      .OR. ;
         Lower( cFileName ) LIKE ".+\.(docm|docx|dotm|dotx|potm|potx|ppam|ppsm|ppsx|pptm|pptx|xlam|xlsb|xlsm|xlsx|xltm|xltx)" .OR. ;
         Lower( cFileName ) LIKE ".+\.(odt,ott,oth,odm,odg,otg,odp,otp,ods,ots,odc,odf,odb,odi,oxt)" .OR. ;
         Lower( cFileName ) LIKE ".+\.(svr|wrl|wrz|vrt|7z)" .OR. Empty( cFExt )
         
      oAttach   := TIPMail():new( cContent, "base64" )
   ELSE
      oAttach   := TIPMail():new( cContent, "7-bit" )
   ENDIF

   cFileName := SubStr( cFileName, RAt( cFileName, cDelim ) + 1 )

   oAttach:setFieldPart  ( "Content-Type", cMimeType )
   oAttach:setFieldOption( "Content-Type", "name", cFname + cFext )

   oAttach:setFieldPart  ( "Content-Disposition", "attachment" )
   oAttach:setFieldOption( "Content-Disposition", "filename", cFname + cFext )

   RETURN ::attach( oAttach )

METHOD detachFile( cPath ) CLASS TipMail

   LOCAL cContent  := ::getBody()
   LOCAL cFileName := ::getFileName()
   LOCAL cDelim    := hb_osPathSeparator()
   LOCAL nFileHandle

   IF Empty( cFileName )
      RETURN .F.
   ENDIF

   IF ValType( cPath ) == "C"
      cFileName := StrTran( cPath + cDelim + cFileName, cDelim + cDelim, cDelim )
   ENDIF

   nFileHandle := FCreate( cFileName )
   IF FError() != 0
      RETURN .F.
   ENDIF

   FWrite( nFileHandle, cContent )

   FClose( nFileHandle )

   RETURN FError() == 0

METHOD getFileName() CLASS TipMail

   LOCAL cName

   IF "attachment" $ Lower( ::getFieldPart( "Content-Disposition" ) )
      cName := ::getFieldOption( "Content-Disposition", "filename" )
   ELSE
      cName := ::getFieldOption( "Content-Type", "name" )
   ENDIF

   RETURN StrTran( cName, '"', '' )

METHOD isMultiPart CLASS TipMail

   RETURN "multipart/" $ Lower( ::GetFieldPart( "Content-Type" ) )

METHOD getMultiParts( aParts ) CLASS TipMail

   LOCAL oSubPart, lReset := .F.

   ::resetAttachment()

   IF aParts == NIL
      aParts := {}
   ENDIF

   DO WHILE ( oSubPart := ::nextAttachment() ) != NIL
      lReset := .T.
      AAdd( aParts, oSubPart )
      IF oSubPart:countAttachments() > 0
         oSubPart:getMultiparts( aParts )
      ENDIF
   ENDDO

   IF lReset
      ::resetAttachment()
   ENDIF

   RETURN aParts

