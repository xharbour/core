/*
 * $Id: rtf.prg 9941 2013-03-25 17:42:55Z enricomaria $
 */

/*
 * Harbour Project source code:
 * RTF Documentation Support Code For HBDOC
 *
 * Copyright 2000-2003 Luiz Rafael Culik <culikr@uol.com.br>
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

#define CRLF HB_OSNewLine()
#xtranslate UPPERLOWER(<exp>) => (UPPER(SUBSTR(<exp>,1,1))+LOWER(SUBSTR(<exp>,2)))
#include 'hbclass.ch'
#include 'common.ch'
MEMVAR aWWW,aResult
*+��������������������������������������������������������������������
*+
*+    Class TRTF
*+
*+��������������������������������������������������������������������
*+
CLASS TRTF

   DATA cFile
   DATA nHandle
   DATA aIdh init {}
   DATA lastId init 100
   METHOD WriteHeader()
   METHOD New( cFile )
   METHOD WritePar( cPar, cIden )
   METHOD WriteParFixed( cPar )
   METHOD WriteParText( cPar, lConv )
   METHOD WriteParNoIndent( cPar )
   METHOD WriteParBox( cPar )
   METHOD WriteLink( clink )
   METHOD WriteJumpLink( clink )
   METHOD WritekLink( aLink ,lAlink)
   METHOD WriteJumpLink1( cLink, cName, cText )
   METHOD CLOSE()
   METHOD WriteParBold( cPar, lCenter )
   METHOD WriteParBoldText( cPar, cText )
   METHOD WriteTitle( cTitle, cTopic )
   METHOD WriteJumpTitle( cTitle, cTopic )
   METHOD EndPar()
   METHOD EndPage()

ENDCLASS

METHOD new( cFile ) CLASS TRTF

   IF VALTYPE( cFile ) <> NIL .AND. VALTYPE( cFile ) == "C"
      Self:cFile   := LOWER( cFile )
      Self:nHandle := FCREATE( Self:cFile )
   ENDIF
RETURN Self

METHOD WriteHeader() CLASS TRTF

   LOCAL cHeader := '{\rtf1\ansi\pard\fs20' + CRLF + ;
           '\deff5{\fonttbl' + CRLF + ;
           '{\f0\froman Tms Rmn;}' + CRLF + ;
           '{\f1\fdecor Symbol;}' + CRLF + ;
           '{\f2\fswiss Helv;}' + CRLF + ;
           '{\f3\fmodern LinePrinter;}' + CRLF + ;
           '{\f4\froman Terminal;}' + CRLF + ;
           '{\f5\froman Times New Roman;}' + CRLF + ;
           '{\f6\fswiss Arial;}' + CRLF + ;
           '{\f7\froman CG Times (WN);}' + CRLF + ;
           '{\f8\fmodern Courier;}' + CRLF + ;
           '{\f9\fmodern Modern;}' + CRLF + ;
           '{\f10\fscript Script;}' + CRLF + ;
           '{\f11\fswiss Univers (WN);}' + CRLF + ;
           '{\f12\fnil Wingdings;}' + CRLF + ;
           '{\f13\fswiss MS Sans Serif;}' + CRLF + ;
           '{\f14\fmodern\fcharset2 LotusWP Box;}' + CRLF + ;
           '}' + CRLF

   LOCAL cColortable := '{\colortbl;' + CRLF + ;
           '\red0\green0\blue0;' + CRLF + ;
           '\red0\green255\blue255;' + CRLF + ;
           '\red255\green0\blue255;' + CRLF + ;
           '\red128\green128\blue128;' + CRLF + ;
           '\red0\green128\blue0;' + CRLF + ;
           '\red0\green255\blue0;' + CRLF + ;
           '\red128\green0\blue0;' + CRLF + ;
           '\red0\green0\blue128;' + CRLF + ;
           '\red128\green128\blue0;' + CRLF + ;
           '\red128\green0\blue128;' + CRLF + ;
           '\red255\green0\blue0;' + CRLF + ;
           '\red192\green192\blue192;' + CRLF + ;
           '\red0\green128\blue128;' + CRLF + ;
           '\red255\green255\blue255;' + CRLF + ;
           '\red255\green255\blue0;' + CRLF + ;
           '}' + CRLF

   FWRITE( Self:nHandle, cHeader )

   FWRITE( Self:nHandle, cColorTable )

RETURN Self

METHOD WritePar( cPar, cIden ) CLASS TRTF

   DEFAULT ciDen TO ''
   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   FWRITE( Self:nHandle, '\par' + CRLF + '\pard\cf1\f6\fs20\b0\i0' + cIden + HB_OEMTOANSI( cPar ) + CRLF )
RETURN Self
METHOD WriteParNoIndent( cPar ) CLASS TRTF

   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   FWRITE( Self:nHandle, '\par' + CRLF + '\pard\cf1\f6\fs20\b0\i0' + HB_OEMTOANSI( cPar ) + CRLF )
RETURN Self
METHOD WriteParBox( cPar ) CLASS TRTF

   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   FWRITE( Self:nHandle, '\par' + CRLF + '\pard\cf1\f4\b0\i0\fi-426\li426' + HB_OEMTOANSI( cPar ) + CRLF )
RETURN Self
METHOD WriteParFixed( cPar ) CLASS TRTF

   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   FWRITE( Self:nHandle, '\par' + CRLF + '\pard\cf1\f8\b0\i0\keep' + cPar + CRLF )
RETURN SELF

METHOD WriteParText( cPar, lConv ) CLASS TRTF

   DEFAULT lConv TO .T.
   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   IF lConv
      FWRITE( Self:nHandle, HB_OEMTOANSI( cPar ) )
   ELSE
      FWRITE( Self:nHandle, cPar )
   ENDIF
RETURN Self

METHOD EndPar() CLASS TRTF

   FWRITE( Self:nHandle, '\par' + CRLF )
RETURN Self

METHOD WriteParBold( cPar, lCenter ) CLASS TRTF

   DEFAULT lCenter TO .F.
   cPar := STRTRAN( cPar, "{", "\{" )
   cPar := STRTRAN( cPar, "}", "\}" )
   IF lCenter
      FWRITE( Self:nHandle, '\par \pard\qc\cf1\f6\fs30\i\b\fi-426\li426 ' + ALLTRIM( HB_OEMTOANSI( cPar ) ) + CRLF )
   ELSE
      FWRITE( Self:nHandle, '\par \pard\cf1\f6\fs30\i0\b\fi-426\li426 ' + ALLTRIM( HB_OEMTOANSI( cPar ) ) + CRLF )
   ENDIF
RETURN Self

METHOD WriteParBoldText( cPar, cText ) CLASS TRTF

   cPar  := STRTRAN( cPar, "{", "\{" )
   cPar  := STRTRAN( cPar, "}", "\}" )
   cText := STRTRAN( cText, "{", "\{" )
   cText := STRTRAN( cText, "}", "\}" )

   FWRITE( Self:nHandle, '\par \pard\cf1\f6\fs20\i\b       ' + ALLTRIM( HB_OEMTOANSI( cPar ) ) + ' \b\cf1\f6\fs20\i0\b0\li300 ' + ALLTRIM( HB_OEMTOANSI( cText ) ) + CRLF )
RETURN Self

METHOD WriteTitle( cTitle, cTopic, cOne ,cCat) CLASS TRTF

   LOCAL cTemp
   LOCAL nPos
   LOCAL cWrite

   nPos := AT( "()", cTitle )

   IF nPos > 0
      cTemp := ALLTRIM( HB_OEMTOANSI( STRTRAN( cTitle, "()", "xx" ) ) )
   ELSE
      cTemp := HB_OEMTOANSI( ALLTRIM( cTitle ) )
      cTemp := STRTRAN( cTemp, "@", "x" )
   ENDIF
   nPos := AT( "#", cTitle )

   IF nPos > 0
      cTemp := ALLTRIM( HB_OEMTOANSI( STRTRAN( cTemp, "#", "\#" ) ) )
   ENDIF
   cTopic := ALLTRIM( HB_OEMTOANSI( cTopic ) )
   cTemp := StrTran( cTemp, " ","_")

   Aadd( ::aIdh, {"IDH_" + cTemp,::lastid++})
   cWrite := CRLF + ;
             '  {#{\footnote \pard\fs20 {' + "IDH_" + cTemp + ' }}}' + CRLF + ;
             '  {${\footnote \pard\fs20 {' + ALLTRIM( cTopic ) + ' }}}' + CRLF + ;
             '  {K{\footnote \pard\fs20 {' + UPPERLOWER(ALLTRIM( cTopic ))+";" + UPPERLOWER(ALLTRIM( cCat ))+ ' }}}' + CRLF + ;
             '  {A{\footnote{A} ' + UPPERLOWER(ALLTRIM( cTopic )) +' }}' + CRLF + ;
              CRLF



   /*'{\f6' + CRLF + ;*/
             /*" ; " + UPPERLOWER(cCat) +" , " +UPPERLOWER(ALLTRIM( strtran(cTopic,"()","" )))+ */
   aadd(aWww,{cTopic,"IDH_"+cTemp,cCat})
   nPos := ascan(aResult,{|a| UPPER(a) == UPPER(cCat)})
   if nPos==0
      aadd(aResult,cCat)
   endif
   FWRITE( Self:nHandle, cWrite )

   FWRITE( Self:nHandle, '\pard\cf1\f6\fs30\i0\b\keepn ' + ALLTRIM( HB_OEMTOANSI( cTopic ) ) + CRLF )
   FWRITE( Self:nHandle, '\par' + CRLF + '\pard\cf1\f6\fs20\b\i0\keepn' + " " + CRLF )
   FWRITE( Self:nHandle, '\par \pard\cf1\f6\fs30\i0\b\keepn ' + ALLTRIM( HB_OEMTOANSI( cOne ) ) + CRLF )
RETURN Self

METHOD WriteJumpTitle( cTitle, cTopic ) CLASS TRTF

   LOCAL cTemp
   LOCAL nPos
   LOCAL cWrite

   nPos := AT( "()", cTitle )

   IF nPos > 0
      cTemp := ALLTRIM( HB_OEMTOANSI( STRTRAN( cTitle, "()", "xx" ) ) )
   ELSE
      cTemp := HB_OEMTOANSI( ALLTRIM( cTitle ) )
      cTemp := STRTRAN( cTemp, "@", "x" )
   ENDIF

   cTopic := ALLTRIM( HB_OEMTOANSI( cTopic ) )

   cWrite :=  CRLF + ;
             '  #{\footnote \pard\fs20 ' + "IDH_" + cTemp + ' }' + CRLF + ;
             '  ${\footnote \pard\fs20 ' + ALLTRIM( cTopic ) + ' }' + CRLF + ;
             CRLF

   FWRITE( Self:nHandle, cWrite )

   Self:WriteParBold( cTopic )

RETURN Self
METHOD EndPage() CLASS TRTF

   FWRITE( Self:nHandle, "\par " + CRLF + '\page' + CRLF )
RETURN Self

METHOD CLOSE() CLASS TRTF

   //   FWRITE( Self:nHandle, '\page' + CRLF )

   FWRITE( Self:nHandle, '}' + CRLF )

   FCLOSE( Self:nHandle )

RETURN Self

METHOD WriteLink( cLink ) CLASS TRTF

   FWRITE( Self:nHandle, '\par \pard\cf1\fs20       {\f6\uldb ' + ALLTRIM( HB_OEMTOANSI( cLink ) ) + '}{\v\f6 ' + "IDH_" + IF( AT( "()", cLink ) > 0, ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "()", "xx" ) ) ), ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "@", "x" ) ) ) ) + '}' + CRLF )

RETURN Self


METHOD WriteJumpLink( cLink, cName, cText ) CLASS TRTF

   FWRITE( Self:nHandle, '\par \pard\cf1\fs20       {\f6\uldb ' + ALLTRIM( HB_OEMTOANSI( cName ) ) + '}{\v\f6 ' + "IDH_" + IF( AT( "()", cLink ) > 0, ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "()", "xx" ) ) ), ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "@", "x" ) ) ) ) + '}' + cText + CRLF )

RETURN Self

METHOD WriteJumpLink1( cLink, cName, cText ) CLASS TRTF

   FWRITE( Self:nHandle, '\par \pard\cf1\fs20       {\f6\ul ' + ALLTRIM( HB_OEMTOANSI( cName ) ) + '}{\v\f6 ' + "IDH_" + IF( AT( "()", cLink ) > 0, ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "()", "xx" ) ) ), ALLTRIM( HB_OEMTOANSI( STRTRAN( cLink, "@", "x" ) ) ) ) + '}' + cText + CRLF )

RETURN Self

METHOD WritekLink( aLink ,lAlink) CLASS TRTF
Local cItem:=' '
Local nPos
Local nSize:=Len(aLink)

(lAlink)

if nSize >2
For nPos:=1 to nSize
    if nPos==nSize
        cItem+= UPPERLOWER(aLink[nPos])
    else
        cItem+= UPPERLOWER(aLink[nPos])
        cItem+=";"
    endif
next
cItem:=Alltrim(cItem)
   FWRITE( Self:nHandle, '\par \pard\cf1\fs20       \{button , ALink('+UPPER(cItem) + ', 2) \}{\f6\uldb Related Topic }'+'{\v\f6 %!ALink(" '+cItem + '", 2) }'+ CRLF )
else
For nPos:=1 to nSize
    FWRITE( Self:nHandle, '\par \pard\cf1\fs20       {\f6\uldb '+aLink[nPos] +' }{\v\f6 !KLink(" '+UPPERLOWER(aLink[nPos]) + '", 2) }'+ CRLF )
next
endif
RETURN Self

*+ EOF: RTF.PRG
