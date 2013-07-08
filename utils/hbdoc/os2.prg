/*
 * $Id: os2.prg 9941 2013-03-25 17:42:55Z enricomaria $
 */

/*
 * Harbour Project source code:
 * OS/2 IPF Documentation Support Code For HBDOC
 *
 * Copyright 2000 Luiz Rafael Culik <Culik@sl.conex.net>
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

#define CRLF hb_osnewline()

#include 'hbclass.ch'
#include 'common.ch'

*+��������������������������������������������������������������������
*+
*+    Class TOs2
*+
*+��������������������������������������������������������������������
*+
CLASS TOs2

   DATA cFile
   DATA nHandle
   DATA aLinkRef
   DATA nRef
   DATA aHeadRef
   DATA aIndRef 
   METHOD New( cFile )
   METHOD WritePar( cPar )
   METHOD WritePar2( cPar,ctag )
   METHOD WriteLink( cLink )
   METHOD ScanLink( cLink )
   METHOD ScanRef( cRef )
   METHOD ScanInd( cRef )    
   METHOD WriteJumpLink( cLink, cName, cText )
   METHOD CLOSE()
   METHOD WriteText( cPar )
   METHOD WriteParBold( cPar ,lMarg)
   METHOD WriteTitle( cTopic, cTitle ,cCategorie)
   METHOD DostoOs2Text( cText )
   METHOD WriteJumpTitle( cTitle, cTopic )
ENDCLASS

METHOD New( cFile ) CLASS TOs2

   IF Self:aLinkRef == NIL
      Self:aLinkRef := {}
      Self:aHeadRef := {}
      Self:aIndRef := {}
      Self:nRef     := 1
   ENDIF

   IF VALTYPE( cFile ) <> NIL .AND. VALTYPE( cFile ) == "C"
      Self:cFile   := LOWER( cFile )
      Self:nHandle := FCREATE( Self:cFile )
   ENDIF

   FWRITE( Self:nHandle, ':userdoc.' + CRLF )

   FWRITE( Self:nHandle, ':docprof toc=123456.' + CRLF )
   FWRITE( Self:nHandle, ':title.' + "Harbour Reference Guide" + CRLF )

   FWRITE( Self:nHandle, '.* HBDOC generated IPF Source File.' + CRLF )

   FWRITE( Self:nHandle, '.* HBDOC Document Source Extractor, (c)1999-2000 Luiz Rafael Culik.' + CRLF )

RETURN Self

METHOD WritePar( cPar ) CLASS TOs2

   FWRITE( Self:nHandle,  Self:DostoOs2Text( cPar ) + CRLF)

RETURN Self

METHOD WritePar2( cBuffer,cTag,cStyle ) CLASS TOs2
   Local aLines
   LOCAL nPos,cLine
   Default cStyle to "Default"
if at("-",cBuffer)>0

Endif
      cBuffer   := STRTRAN( cBuffer, '<b>', ':hp2.' )
      cBuffer   := STRTRAN( cBuffer, '</b>', ':ehp2.' )
      cBuffer   := STRTRAN( cBuffer, '<par>', '' )
      cBuffer   := STRTRAN( cBuffer, '</par>', '' )
      cBuffer   := self:DostoOs2Text(cBuffer)

      aLines:=FormatStringBuffer(cBuffer)

      For nPos:=1 to LEN(aLines)
        cLine:=aLines[nPos]
        If nPos==1
            If !empty(cLine) .and. cStyle <>"Syntax"
                FWRITE( Self:nHandle,cTag+  cLine  + CRLF)
            elseIf !empty(cLine) .and. cStyle =="Syntax"
                FWRITE( Self:nHandle,cTag+ ":hp2."+ cLine  +":ehp2." +CRLF)
            Endif
        Else

            If !empty(cLine) .and. cStyle <>"Syntax"
                FWRITE( Self:nHandle, cLine  + CRLF)
            elseIf !empty(cLine) .and. cStyle =="Syntax"
                FWRITE( Self:nHandle, ":hp2."+ cLine  +":ehp2." +CRLF)
            Endif
        Endif
      NEXT
/*
if cStyle=="Syntax"
    ? cBuffer
endif    
         DO WHILE !lendPar
            IF nPos == 0
               cLine := SUBSTR( cBuffer, 1, 231 )
               nPos  := RAT( " ", cLine )
               IF nPos > 0                 
                  cLine := SUBSTR( cBuffer, 1, nPos )
               ENDIF
            If !empty(cLine) .and. cStyle <>"Syntax"
                FWRITE( Self:nHandle,cTag+  cLine  + CRLF)
            elseIf !empty(cLine) .and. cStyle =="Syntax"
                FWRITE( Self:nHandle,cTag+ ":hp2."+ cLine  +":ehp2." +CRLF)
            Endif
            if nLen<255
                lEndpar:=.T.
                exit
            endif
            ELSE
               cLine :=  SUBSTR( cBuffer, curPos, 231 )

               IF AT( '</par>', cLine ) > 0
                  lEndPar := .T.
                  cLine   := STRTRAN( cLine, " </par>", "" )
               ENDIF
               nPos := RAT( " ", cLine )
               IF nPos > 0
                  cLine :=  SUBSTR( cBuffer, curpos, nPos )
                  nPos -= 1  
               ELSE
                  IF cLine == "</par>"
                     cLine := ''
                  ENDIF

            ENDIF
            If !empty(cLine) .and. cStyle <>"Syntax"
                FWRITE( Self:nHandle,cTag+  cLine  + CRLF)
            elseIf !empty(cLine) .and. cStyle =="Syntax"
                FWRITE( Self:nHandle,cTag+ ":hp2."+ cLine  +":ehp2." +CRLF)
            Endif
            
            ENDIF

            curPos += nPos
? Curpos
         ENDDO
*/   



RETURN Self

METHOD WriteText( cPar ) CLASS TOs2
   FWRITE( Self:nHandle, cPar + CRLF )
RETURN Self


METHOD WriteParBold( cPar ,lMarg) CLASS TOs2
DEFAULT lMarg to .t.
    IF lMarg
      FWRITE( Self:nHandle,".br"+CRLF+ ":hp2." + SELF:DosToOs2Text( cPar ) + ':ehp2.'+CRLF +".br" + CRLF +":p."+CRLF+':lm margin=6.' +CRLF )
    Else
      FWRITE( Self:nHandle,":hp2." + SELF:DosToOs2Text( cPar ) + ':ehp2.'+CRLF +".br")
    Endif

RETURN Self

METHOD WriteTitle( cTopic, cTitle ,cCategory) CLASS TOs2

   LOCAL nItem
   Local lHead:=.F.
   LOCAL nrItem,nIItem
   LOCAL cRefCateg
   LOCAL cIndCateg   
   cTopic := ALLTRIM( cTopic )
   cRefCateg:=SetCateg(cCategory )
   cIndCateg:=SetInd(cCategory)

   IF Self:Scanlink( cTopic ) == 0
      nItem := ASCAN( Self:aLinkRef, { | a | upper(a[ 1 ]) == upper(cTopic )} )
   ELSE             // Just in case that nItem>0 so the Link is already referenced
      nItem := ASCAN( Self:aLinkRef, { | a | upper(a[ 1 ]) == upper(cTopic) } )
   ENDIF
//   FWRITE( Self:nHandle, ':h1 res=' + ALLTRIM( STR( nItem ) ) + '.' + cTopic + CRLF )
   If Self:ScanInd(cIndCateg)==0
      niItem := ASCAN( Self:aIndRef, { | a | upper(a) == upper(cIndCateg )} )
      FWRITE( Self:nHandle, ':h1 ' + ::aIndRef[niItem] + "."+ UPPER( cCategory ) + CRLF)
      lHead := .T.  
   ELSE             // Just in case that nItem>0 so the Link is already referenced
      niItem := ASCAN( Self:aIndRef, { | a | upper(a) == upper(cIndCateg) } )
   ENDIF
   IF niItem>0 .AND.       lHead 
      FWRITE( Self:nHandle, ':h2 '+ ' res=' + ALLTRIM( STR( nItem ) ) + '.' + cTopic  + CRLF  )
   elseIF     niItem>0 .AND.       !lHead
      FWRITE( Self:nHandle, ':h2 id='+ ::aIndRef[niItem] + ' res=' + ALLTRIM( STR( nItem ) ) + '.' + cTopic  + CRLF  )
   Endif
   If Self:ScanRef(cRefCateg)==0
      nrItem := ASCAN( Self:aHeadRef, { | a | upper(a) == upper(cRefCateg )} )
      FWRITE( Self:nHandle, ':i1 id=' + ::aHeadRef[nrItem] + "."+ UPPER( cCategory ) + CRLF)
   ELSE             // Just in case that nItem>0 so the Link is already referenced
      nrItem := ASCAN( Self:aHeadRef, { | a | upper(a) == upper(cRefCateg) } )
   ENDIF

   if nritem>0

      FWRITE( Self:nHandle, ':i2 refid=' + ::aHeadRef[nrItem] + "." + UPPER( cTopic ) + CRLF )
   Endif
   cTopic := ::DosToOs2Text(cTopic)
   cTitle := ::DosToOs2Text(cTitle)

   FWRITE( Self:nHandle, ":p." + cTitle + CRLF +".br"+CRLF)

RETURN Self

METHOD CLOSE() CLASS TOs2

   FWRITE( Self:nHandle, ':euserdoc.' + CRLF )

   FCLOSE( Self:nHandle )

RETURN Self

METHOD WriteLink( cLink ) CLASS TOs2

   LOCAL nItem

   IF Self:Scanlink( cLink ) == 0
      nItem := ASCAN( Self:aLinkRef, { | a | upper(a[ 1 ]) == upper(cLink) } )                // Again.
   ELSE
      nItem := ASCAN( Self:aLinkRef, { | a | upper(a[ 1 ]) == upper(cLink) } )

   ENDIF

   IF nItem = 0
      nItem := Self:nRef
   ENDIF

   FWRITE( Self:nHandle, ":link reftype=hd res=" + ALLTRIM( STR( nItem ) ) + "." + Self:aLinkRef[ nItem, 1 ] + ":elink." + CRLF )

   FWRITE( Self:nHandle, ".br" + CRLF )

RETURN Self

METHOD ScanLink( cLink ) CLASS TOs2

   LOCAL nItem

   nItem := ASCAN( Self:aLinkRef, { | a | Upper(a[ 1 ] )== upper(cLink) } )

   IF nItem == 0
      AADD( Self:aLinkRef, { upper(cLink), Self:nRef } )
      Self:nRef ++
   ENDIF

RETURN nItem
METHOD ScanRef( cLink ) CLASS TOs2

   LOCAL nItem

   nItem := ASCAN( Self:aHeadRef, { | a | Upper(a)== upper(cLink) } )

   IF nItem == 0
      AADD( Self:aHeadRef,  upper(cLink))
   ENDIF

RETURN nItem

METHOD ScanInd( cLink ) CLASS TOs2

   LOCAL nItem

   nItem := ASCAN( Self:aIndRef, { | a | Upper(a)== upper(cLink) } )

   IF nItem == 0
      AADD( Self:aIndRef,  upper(cLink))
   ENDIF

RETURN nItem

METHOD DosToOs2Text( cText ) CLASS TOs2

   LOCAL cReturn

   cReturn := STRTRAN( cText, '&', "&amp." )

   cReturn := STRTRAN( cReturn, '"', "&cdq." )
   if at(":hp2.",cReturn)==0 .or. at(":ehp2.",cReturn)==0
       cReturn := STRTRAN( cReturn, ':', "&colon." )
    endif
   cReturn := STRTRAN( cReturn, ',', "&comma." )

   cReturn := STRTRAN( cReturn, '_', "&us." )
   cReturn := STRTRAN( cReturn, '~', "&tilde." )
   cReturn := STRTRAN( cReturn, '|', "&splitvbar." )

   cReturn := STRTRAN( cReturn, '/', "&slash." )
   cReturn := STRTRAN( cReturn,"<&slash.par>","</par>")
   cReturn := STRTRAN( cReturn, ';', "&semi." )
   cReturn := STRTRAN( cReturn, ')', "&rpar." )
   cReturn := STRTRAN( cReturn, ']', "&rbrk.." )
   cReturn := STRTRAN( cReturn, '}', "&rbrc." )
   cReturn := STRTRAN( cReturn, '(', "&lpar." )
   cReturn := STRTRAN( cReturn, '[', "&lbrk." )
   cReturn := STRTRAN( cReturn, '{', "&lbrc." )
   cReturn := STRTRAN( cReturn, '=', "&eq." )
   cReturn := STRTRAN( cReturn, '$', "&dollar." )
   cReturn := STRTRAN( cReturn, "-", "&minus." )
RETURN cReturn

METHOD WriteJumpTitle( cTitle, cTopic ) CLASS TOs2

   LOCAL cWrite

cTitle = cTitle

   cTopic := ALLTRIM( HB_OEMTOANSI( cTopic ) )

   cWrite := ':fn id=' + cTopic + '.'

   FWRITE( Self:nHandle, cWrite )

   Self:WriteParBold( cTopic )

RETURN Self
METHOD WriteJumpLink( cLink, cText ) CLASS TOs2

   FWRITE( Self:nHandle, "       :link refid=" + ALLTRIM( HB_OEMTOANSI( cLink ) ) + "reftype=fn." + cLink + ":elink." + cText + CRLF )

RETURN Self

Static function SetCateg(cRef)
Local cReturn
cReturn:=Alltrim(left(cRef,5))
cReturn+="X"
Return cReturn
Static function SetInd(cRef)
Local cReturn
cReturn:=Alltrim(left(cRef,4))
cReturn+="Y"
Return cReturn

Static FUNCTION FormatStringBuffer(cBuffer)
Local nLen,nPos,aLine:={}
Local cLine
nLen:=Len(cBuffer)

WHILE nLen>230
    If nLen>230
        cLine:=Substr(cBuffer,1,230)
        nPos:=RAT(" ",cLine)
        IF nPos>0
            cLine:=Substr(cBuffer,1,nPos)
            cBuffer:=Strtran(cBuffer,cLine,"")
            AADD(aLine,alltrim(cLine))
            nLen:=Len(cBuffer)
        Endif
       if at('&minus.',cLine)>0 .or. at('&eq.',cLine)>0
        nPos:=RAT(".",cLine)
        IF nPos>0
            cLine:=Substr(cBuffer,1,nPos)
            cBuffer:=Strtran(cBuffer,cLine,"")
            AADD(aLine,alltrim(cLine))
            nLen:=Len(cBuffer)
        Endif
       Endif
    Endif
ENDDO
IF nLen<=230
    aadd(aLine,ALLTRIM(cBuffer))
ENDIF
RETURN aLine
*+ EOF: OS2.PRG