/*
 * $Id: thtml.prg 9726 2012-10-02 19:56:17Z andijahja $
 */
/*
 * Harbour Project source code:
 * HTML Classes
 *
 * Copyright 2007 Hannes Ziegler <hz/at/knowleXbase.com>
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
 *
 */

#include "common.ch"
#include "error.ch"
#include "hbclass.ch"
#include "thtml.ch"

// A Html document can have more than 16 nesting levels.
// The current implementation of FOR EACH is not suitable for the HTML classes

#define  FOR_EACH_NESTING_LIMIT_IS_ONLY_16_AND_FAR_TOO_SMALL

// Directives for a light weight html parser
#xtrans  P_PARSER( <c> )       =>    {<c>,HB_String2Pointer(<c>),0,Len(<c>),0}
#define  P_STR                 1    // the string to parse
#define  P_PTR                 2    // pointer to the string
#define  P_POS                 3    // current parser position
#define  P_LEN                 4    // length of string
#define  P_END                 5    // last parser position

#xtrans  :p_str                =>   \[P_STR]
#xtrans  :p_ptr                =>   \[P_PTR]
#xtrans  :p_pos                =>   \[P_POS]
#xtrans  :p_len                =>   \[P_LEN]
#xtrans  :p_end                =>   \[P_END]

#xtrans  P_SEEK( <a>, <c> )    =>   (<a>:p_end:=<a>:p_pos, <a>:p_pos:=AtI(<c>,<a>:p_str,<a>:p_end+1))
#xtrans  P_PEEK( <a>, <c> )    =>   (<a>:p_end:=<a>:p_pos,PStrCompi( <a>:p_ptr, <a>:p_pos, <c> ))
#xtrans  P_NEXT( <a> )         =>   (<a>:p_end:=<a>:p_pos, <a>:p_str\[++<a>:p_pos])
#xtrans  P_PREV( <a> )         =>   (<a>:p_end:=<a>:p_pos, <a>:p_str\[--<a>:p_pos])

// Directives for a light weight stack
#define  S_DATA                1    // array holding data elements
#define  S_NUM                 2    // number of occupied data elements
#define  S_SIZE                3    // total size of data array
#define  S_STEP                4    // number of elements for auto sizing

#xtrans  S_STACK()             =>   S_STACK(64)
#xtrans  S_STACK( <n> )        =>   {Array(<n>),0,<n>,Max(32,Int(<n>/2))}
#xtrans  S_GROW( <a> )         =>   (IIF(++<a>\[S_NUM]><a>\[S_SIZE],ASize(<a>\[S_DATA],(<a>\[S_SIZE]+=<a>\[S_STEP])),<a>))
#xtrans  S_SHRINK( <a> )       =>   (IIF(<a>\[S_NUM]>0 .AND. --<a>\[S_NUM]\<<a>\[S_SIZE]-<a>\[S_STEP],ASize(<a>\[S_DATA],<a>\[S_SIZE]-=<a>\[S_STEP]),<a>))
#xtrans  S_COMPRESS( <a> )     =>   (ASize(<a>\[S_DATA],<a>\[S_SIZE]:=<a>\[S_NUM]))
#xtrans  S_PUSH(<a>,<x>)       =>   (S_GROW(<a>),<a>\[S_DATA,<a>\[S_NUM]]:=<x>)
#xtrans  S_POP(<a>,@<x>)       =>   (<x>:=<a>\[S_DATA,<a>\[S_NUM]],<a>\[S_DATA,<a>\[S_NUM]]:=NIL,S_SHRINK(<a>))
#xtrans  S_POP(<a>)            =>   (<a>\[S_DATA,<a>\[S_NUM]]:=NIL,S_SHRINK(<a>))
#xtrans  S_TOP(<a>)            =>   (<a>\[S_DATA,<a>\[S_NUM]])


   STATIC  saHtmlAttr                  // data for HTML attributes
   STATIC  shTagTypes                  // data for HTML tags
   STATIC  saHtmlAnsiEntities          // HTML character entities (ANSI character set)
   STATIC  slInit      := .F.          // initilization flag for HTML data

// #define _DEBUG_
#ifdef _DEBUG_
#xtrans HIDDEN:  =>  EXPORTED:   // debugger can't see HIDDEN iVars
#endif

/*
 * Class for handling an entire HTML document
 */

CLASS THtmlDocument

   HIDDEN:
   DATA oIterator
   DATA nodes

   EXPORTED:
   DATA root    READONLY
   DATA head    READONLY
   DATA body    READONLY
   DATA changed INIT .T.

   METHOD new( cHtmlString )
   METHOD readFile( cFileName )
   METHOD writeFile( cFileName )

   METHOD collect()
   METHOD toString( nIndent )
   METHOD getNode( cTagName )
   METHOD getNodes( cTagName )
   METHOD findFirst( cName, cAttrib, cValue, cData )
   METHOD findFirstRegex( cName, cAttrib, cValue, cData )
   METHOD findNext() INLINE ::oIterator:Next()

ENDCLASS

// accepts a HTML formatted string

METHOD new( cHtmlString ) CLASS THtmlDocument

   LOCAL cEmptyHtmlDoc, oNode, oSubNode, oErrNode, aHead, aBody, nMode := 0

   TEXT INTO cEmptyHtmlDoc
   <! DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" >
   < html >
   < head >
   </ head >
   < body >
   </ body >
   </ html >
   ENDTEXT

   IF ValType( cHtmlString ) <> "C"
      ::root := THtmlNode():new( cEmptyHtmlDoc )
   ELSE
      IF .NOT. "<html" $ Lower( Left( cHtmlString, 4096 ) )
         ::root := THtmlNode():new( cEmptyHtmlDoc )
         nMode := 1
      ELSE
         ::root := THtmlNode():new( cHtmlString )
      ENDIF
   ENDIF

   ::root:document := self
   ::head := ::getNode( "head" )
   ::body := ::getNode( "body" )

   IF ::head == NIL .AND. ::body == NIL
      // A HTML document consists of <html>, <head> and <body> tags
      // Although they are optional, the THtmlDocument class enforces them
      // so that the instance variables :head and :body are always available
      aHead := {}
      aBody := {}
      FOR EACH oSubNode IN ::root:htmlContent
         IF oSubNode:isType( CM_HEAD )
            AAdd( aHead, oSubNode )
         ELSE
            AAdd( aBody, oSubNode )
         ENDIF
      NEXT

      ::root    := THtmlNode():new( cEmptyHtmlDoc )
      ::root:document := self
      ::changed := .T.
      ::head    := ::getNode( "head" )
      ::body    := ::getNode( "body" )

      FOR EACH oSubNode IN aHead
         IF oSubNode:isType( CM_HEAD )
            ::head:addNode( oSubNode )
         ELSE
            ::body:addNode( oSubNode )
         ENDIF
      NEXT

      FOR EACH oSubNode IN aBody
         IF Lower( oSubNode:htmlTagName ) $ "html,head,body"
            // This node is an error in the HTML string.
            // We gracefully add its subnodes to the <body> tag
            FOR EACH oErrNode IN oSubNode:htmlContent
               ::body:addNode( oErrNode )
            NEXT
         ELSE
            IF oSubNode:isType( CM_HEAD )
               oSubNode:delete()
               ::head:addNode( oSubNode )
            ELSE
               ::body:addNode( oSubNode )
            ENDIF
         ENDIF
      NEXT

   ELSEIF ::head == NIL
      ::head := ::body:insertBefore( THtmlNode():new( ::body, "head" ) )

   ELSEIF ::body == NIL
      ::head := ::head:insertAfter( THtmlNode():new( ::head, "body" ) )

   ENDIF

   IF nMode == 1
      oNode := THtmlNode():new( cHtmlString )

      FOR EACH oSubNode IN oNode:htmlContent
         IF oSubNode:isType( CM_HEAD )
            ::head:addNode( oSubNode )
         ELSE
            ::body:addNode( oSubNode )
         ENDIF
      NEXT
   ENDIF

   RETURN self

// Builds a HTML formatted string

METHOD toString() CLASS THtmlDocument

   RETURN ::root:toString()

// reads HTML file and parses it into tree of objects

METHOD readFile( cFileName ) CLASS THtmlDocument

   IF ! File( cFileName )
      RETURN .F.
   ENDIF
   ::changed := .T.
   ::new( MemoRead( cFileName ) )

   RETURN .T.

// writes the entire tree of HTML objects into a file

METHOD writeFile( cFileName ) CLASS THtmlDocument

   LOCAL cHtml := ::toString()
   LOCAL nFileHandle := FCreate( cFileName )

   IF FError() <> 0
      RETURN .F.
   ENDIF

   FWrite( nFileHandle, cHtml, Len( cHtml ) )
   FClose( nFileHandle )
   ::changed := .F.

   RETURN ( FError() == 0 )

// builds a one dimensional array of all nodes contained in the HTML document

METHOD collect() CLASS THtmlDocument

   IF ::changed
      ::nodes   := ::root:collect()
      ::changed := .F.
   ENDIF

   RETURN ::nodes

// returns the first tag matching the passed tag name

METHOD getNode( cTagName ) CLASS THtmlDocument

   LOCAL oNode

   IF ::changed
      ::collect()
   ENDIF

   FOR EACH oNode IN ::nodes
      IF Lower( oNode:htmlTagName ) == Lower( cTagName )
         RETURN oNode
      ENDIF
   NEXT

   RETURN NIL

// returns all tags matching the passed tag name

METHOD getNodes( cTagName ) CLASS THtmlDocument

   LOCAL oNode, stack := S_STACK()

   IF ::changed
      ::collect()
   ENDIF

   FOR EACH oNode IN ::nodes
      IF Lower( oNode:htmlTagName ) == Lower( cTagName )
         S_PUSH( stack, oNode )
      ENDIF
   NEXT

   S_COMPRESS( stack )

   RETURN stack[S_DATA]

// finds the first HTML tag matching the search criteria

METHOD findFirst( cName, cAttrib, cValue, cData ) CLASS THtmlDocument

   ::oIterator := THtmlIteratorScan():New( self )

   RETURN ::oIterator:Find( cName, cAttrib, cValue, cData )

// finds the first HTML tag matching the RegEx search criteria

METHOD findFirstRegex( cName, cAttrib, cValue, cData ) CLASS THtmlDocument

   ::oIterator := THtmlIteratorRegex():New( self )

   RETURN ::oIterator:Find( cName, cAttrib, cValue, cData )


/*
 * Abstract super class for THtmlIteratorScan and THtmlIteratorScanRegEx
 *
 * (Adopted from TXMLIterator -> source\rtl\TXml.prg)
 */

CLASS THtmlIterator

   METHOD New( oNodeTop )           CONSTRUCTOR
   METHOD NEXT()
   METHOD Rewind()
   METHOD Find( cName, cAttribute, cValue, cData )

   METHOD GetNode()                 INLINE   ::oNode
   METHOD SetContext()
   METHOD Clone()

   HIDDEN:
   DATA cName
   DATA cAttribute
   DATA cValue
   DATA cData
   DATA oNode
   DATA oTop
   DATA aNodes
   DATA nCurrent
   DATA nLast
   METHOD MatchCriteria()

ENDCLASS

// accepts a THtmlNode or THtmlDocument object

METHOD New( oHtml ) CLASS THtmlIterator

   IF oHtml:isDerivedFrom ( "THtmlDocument" )
      ::oNode := oHtml:root
      ::aNodes := oHtml:nodes
   ELSE
      ::oNode  := oHtml
      ::aNodes := ::oNode:collect()
   ENDIF

   ::oTop     := ::oNode
   ::nCurrent := 1
   ::nLast    := Len( ::aNodes )

   RETURN Self

METHOD rewind CLASS THtmlIterator

   ::oNode := ::oTop
   ::nCurrent := 0

   RETURN self

METHOD Clone() CLASS THtmlIterator

   LOCAL oRet

   oRet            := THtmlIterator():New( ::oTop )
   oRet:cName      := ::cName
   oRet:cAttribute := ::cAttribute
   oRet:cValue     := ::cValue
   oRet:cData      := ::cData
   oRet:nCurrent   := 0
   oRet:nLast      := Len( ::aNodes )
   oRet:aNodes     := ::aNodes

   RETURN oRet

METHOD SetContext() CLASS THtmlIterator

   ::oTop          := ::oNode
   ::aNodes        := ::oNode:collect()
   ::nCurrent      := 0
   ::nLast         := Len( ::aNodes )

   RETURN Self

METHOD Find( cName, cAttribute, cValue, cData ) CLASS THtmlIterator

   ::cName         := cName
   ::cAttribute    := cAttribute
   ::cValue        := cValue
   ::cData         := cData

   IF ::nLast == 0
      ::nCurrent := 0
      RETURN NIL
   ENDIF

   IF ::MatchCriteria( ::oNode )
      RETURN ::oNode
   ENDIF

   RETURN ::Next()

METHOD NEXT() CLASS THtmlIterator

   LOCAL oFound, lExit := .F.

   DO WHILE .NOT. lExit
      TRY
         oFound := ::aNodes[ ++::nCurrent ]
         IF ::MatchCriteria( oFound )
            ::oNode := oFound
            lExit := .T.
         ENDIF
      CATCH
         lExit      := .T.
         oFound     := NIL
         ::nCurrent := 0
      END
   ENDDO

   RETURN oFound

METHOD MatchCriteria( /*oNode*/ ) CLASS THtmlIterator

   RETURN .T.


/********************************************
   Iterator scan class
*********************************************/

CLASS THtmlIteratorScan FROM THtmlIterator

   METHOD New( oNodeTop ) CONSTRUCTOR
   HIDDEN:
   METHOD MatchCriteria( oFound )

ENDCLASS

METHOD New( oNodeTop ) CLASS THtmlIteratorScan

   ::Super:New( oNodeTop )

   RETURN Self

METHOD MatchCriteria( oFound ) CLASS THtmlIteratorScan

   LOCAL xData

   IF ::cName != NIL .AND. ( Lower( ::cName ) != Lower( oFound:htmlTagName ) )
      RETURN .F.
   ENDIF

   IF ::cAttribute != NIL .AND. .NOT. HHasKey( oFound:getAttributes(), ::cAttribute )
      RETURN .F.
   ENDIF

   IF ::cValue != NIL
      xData := oFound:getAttributes()
      IF HScan( xData, {| /*xKey*/, cValue| Lower( ::cValue ) == Lower( cValue ) } ) == 0
         RETURN .F.
      ENDIF
   ENDIF

   IF ::cData != NIL
      xData := oFound:getText( " " )
      IF Empty( xData ) .OR. ( AllTrim( ::cData )  <> AllTrim( xData )  )
         RETURN .F.
      ENDIF
   ENDIF

   RETURN .T.

/********************************************
   Iterator regex class
*********************************************/

CLASS THtmlIteratorRegex FROM THtmlIterator

   METHOD New( oNodeTop ) CONSTRUCTOR
   HIDDEN:
   METHOD MatchCriteria( oFound )

ENDCLASS

METHOD New( oNodeTop ) CLASS THtmlIteratorRegex

   ::Super:New( oNodeTop )

   RETURN Self

METHOD MatchCriteria( oFound ) CLASS THtmlIteratorRegex

   LOCAL xData

   IF ::cName != NIL .AND. .NOT. ( Lower( ::cName ) LIKE Lower( oFound:htmlTagName ) )
      RETURN .F.
   ENDIF

   IF ::cAttribute != NIL .AND. ;
         hScan( oFound:getAttributes(), {|cKey| cKey LIKE Lower( ::cAttribute ) } ) == 0
      RETURN .F.
   ENDIF

   IF ::cValue != NIL .AND. ;
         hScan( oFound:getAttributes(), {| /*xKey*/, cValue| cValue LIKE ::cValue } ) == 0
      RETURN .F.
   ENDIF

   IF ::cData != NIL
      xData := oFound:getText( " " )
      IF Empty( xData ) .OR. .NOT. ( AllTrim( xData ) HAS AllTrim( ::cData ) )
         RETURN .F.
      ENDIF
   ENDIF

   RETURN .T.

/*
 * Class representing a HTML node tree.
 * It parses a HTML formatted string
 */

CLASS THtmlNode

   HIDDEN:
   DATA root
   DATA _document
   DATA parent
   DATA htmlContent

   METHOD parseHtml
   METHOD parseHtmlFixed

   METHOD _getTextNode
   METHOD _setTextNode

   METHOD keepFormatting

   EXPORTED:

   DATA htmlTagName     READONLY
   DATA htmlEndTagName  READONLY
   DATA htmlTagType     READONLY
   DATA htmlAttributes  READONLY

   METHOD new( oParent, cTagName, cAttrib, cContent )

   METHOD isType( nCM_TYPE )
   ACCESS isEmpty()
   ACCESS isInline()
   ACCESS isOptional()
   ACCESS isNode()
   ACCESS ISBLOCK()

   METHOD addNode( oTHtmlNode )
   METHOD insertAfter( oTHtmlNode )
   METHOD insertBefore( oTHtmlNode )
   METHOD DELETE()

// Messages from TXmlNode
   MESSAGE insertBelow METHOD addNode
   MESSAGE unlink      METHOD DELETE

   METHOD firstNode()
   METHOD lastNode()

   ACCESS nextNode()
   ACCESS prevNode()

   ACCESS siblingNodes()  INLINE iif( ::parent == NIL, NIL, ::parent:htmlContent )
   ACCESS childNodes()    INLINE iif( ::isNode(), ::htmlContent, NIL )
   ACCESS parentNode()    INLINE ::parent
   ACCESS document()      INLINE iif( ::root == NIL, NIL, ::root:_document )

   METHOD toString( nIndent )
   METHOD attrToString()

   METHOD collect()
   METHOD getText( cCRLF )

   METHOD getAttribute( cAttrName )
   METHOD getAttributes()

   METHOD setAttribute( cAttrName, cAttrValue )
   METHOD setAttributes( cHtmlAttr )

   METHOD delAttribute( cAttrName )
   METHOD delAttributes()

   METHOD isAttribute()

   ACCESS TEXT    INLINE ::_getTextNode()
   ASSIGN TEXT( x ) INLINE ::_setTextNode( x )

   ACCESS attr    INLINE ::getAttributes()
   ASSIGN attr( x ) INLINE ::setAttributes( x )

   METHOD pushNode  OPERATOR +
   METHOD popNode   OPERATOR -

   METHOD findNodeByTagName
   METHOD findNodesByTagName

   ERROR HANDLER noMessage
   METHOD noAttribute

ENDCLASS

METHOD new( oParent, cTagName, cAttrib, cContent ) CLASS THtmlNode

   IF .NOT. slInit
      THtmlInit( .T. )
   ENDIF

   IF ValType( oParent ) == "C"
      // a HTML string is passed -> build new tree of objects
      IF Chr( 9 ) $ oParent
         oParent := StrTran( oParent, Chr( 9 ), Chr( 32 ) )
      ENDIF
      ::root           := self
      ::htmlTagName    := "_root_"
      ::htmlTagType    := THtmlTagType( "_root_" )
      ::htmlContent    := {}
      ::parseHtml( P_PARSER( oParent ) )
   ELSEIF ValType( oParent ) == "O"
      // a HTML object is passed -> we are in the course of building an object tree
      ::root           := oParent:root
      ::parent         := oParent
      IF ValType( cAttrib ) == "C"
         IF cAttrib[-1] == "/"
            cAttrib[-1] := " "
            ::htmlEndTagName := "/"
            ::htmlAttributes := Trim( cAttrib )
         ELSE
            ::htmlAttributes := cAttrib
         ENDIF
      ELSE
         ::htmlAttributes := cAttrib
      ENDIF
      ::htmlTagName    := cTagName
      ::htmlTagType    := THtmlTagType( cTagName )
      ::htmlContent    := iif( cContent == NIL, {}, cContent )
   ELSE
      RETURN ::error( "Parameter error", ::className(), ":new()", EG_ARG, hb_AParams() )
   ENDIF

   RETURN self

METHOD isType( nType ) CLASS THtmlNode

   LOCAL lRet

   TRY
      lRet := ( ( ::htmlTagType[2] & nType ) > 0 )
   CATCH
      lRet := .F.
   END

   RETURN lRet

// checks if this is a node that is always empty and never has HTML text, e.g. <img>,<link>,<meta>

METHOD isEmpty CLASS THtmlNode

   RETURN ( ( ::htmlTagType[2] & CM_EMPTY ) > 0 )

// checks if this is a node that may occur inline, eg. <b>,<font>

METHOD isInline CLASS THtmlNode

   RETURN ( ( ::htmlTagType[2] & CM_INLINE ) > 0 )

// checks if this is a node that may appear without a closing tag, eg. <p>,<tr>,<td>

METHOD isOptional CLASS THtmlNode

   RETURN ( ( ::htmlTagType[2] & CM_OPT ) > 0 )

// checks if this is a node (leafs contain no further nodes, e.g. <br>,<hr>,_text_)

METHOD isNode CLASS THtmlNode

   RETURN ( ValType( ::htmlContent ) == "A" .AND. Len( ::htmlContent ) > 0 )

// checks if this is a block node that must be closed with an ending tag: eg: <table></table>, <ul></ul>

METHOD isBlock CLASS THtmlNode

   RETURN ( ( ::htmlTagType[2] & CM_BLOCK ) > 0 )

// checks if this is a node whose text line formatting must be preserved: <pre>,<script>,<textarea>

METHOD keepFormatting CLASS THtmlNode

   RETURN "<" + Lower( ::htmlTagName ) + ">" $ ( "<pre>,<script>,<textarea>" )

// parses a HTML string and builds a tree of THtmlNode objects

METHOD parseHtml( parser ) CLASS THtmlNode

   LOCAL nLastPos := parser:p_pos
   LOCAL lRewind  := .F.
   LOCAL oThisTag, oNextTag, oLastTag
   LOCAL cTagName, cAttr, nStart, nEnd, nPos, cText

   IF .NOT. "<" $ parser:p_Str
      // Plain text
      ::addNode( THtmlNode():new( self, "_text_", , parser:p_Str ) )
      RETURN self
   ENDIF

   oThisTag := self

   DO WHILE P_SEEK( parser, "<" ) > 0
      nStart   := parser:p_pos
      P_SEEK( parser, ">" )
      nEnd     := parser:p_pos
      cAttr    := SubStr( parser:p_Str, nStart, nEnd - nStart + 1 )
      cText    := LTrim( SubStr( parser:p_str, nLastPos + 1, nStart - nLastPos - 1 ) )
      cTagName := CutStr( " ", @cAttr )

      IF .NOT. cText == ""
         IF Left( cText, 2 ) == "</"
            // ending tag of previous node
            cText := Lower( AllTrim( SubStr( CutStr( ">", @cText ), 3 ) ) )
            oLastTag := oThisTag:parent
            DO WHILE oLastTag <> NIL .AND. Lower( oLastTag:htmlTagName ) <> cText
               oLastTag := oLastTag:parent
            ENDDO
            IF oLastTag <> NIL
               oLastTag:htmlEndTagName := "/" + oLastTag:htmlTagName
            ENDIF

         ELSEIF Chr( 10 ) $ cText
            cText := Trim( cText )
            nPos := Len( cText ) + 1
            DO WHILE nPos > 0 .AND. cText[--nPos] $ Chr( 9 ) + Chr( 10 ) + Chr( 13 ) ; ENDDO
            oThisTag:addNode( THtmlNode():new( oThisTag, "_text_", , Left(cText,nPos ) ) )
         ELSE
            oThisTag:addNode( THtmlNode():new( oThisTag, "_text_", , cText ) )
         ENDIF
      ENDIF

      IF cTagName == "<"
         // <  tagName>
         cAttr    := LTrim( cAttr )
         cTagName += CutStr( " ", @cAttr )
      ENDIF
      cTagName := StrTran( cTagName, ">", "" )
      cTagName := AllTrim( SubStr( cTagName, 2 ) )

      SWITCH cTagName[1]
      CASE "!"
         // comment or PI
         oThisTag:addNode( THtmlNode():new( oThisTag, cTagName, Left(cAttr,Len(cAttr ) - 1 ) ) )
         EXIT

      CASE "/"
         // end tag
         IF Lower( "/" + oThisTag:htmlTagName ) == Lower( cTagName )
            oThisTag:htmlEndTagName := cTagName

         ELSE

            oNextTag := oThisTag:parent
            DO WHILE oNextTag <> NIL .AND. Lower( oNextTag:htmlTagName ) <> Lower( SubStr( cTagName,2 ) )
               oNextTag := oNextTag:parent
            ENDDO

            IF oNextTag == NIL
               // orphaned end tag with no opening tag
               LOOP
            ELSE
               // node that opened the end tag
               oNextTag:htmlEndTagName := cTagName
               oThisTag := oNextTag
            ENDIF

         ENDIF

         lRewind := .T.
         EXIT

         DEFAULT

         IF oThisTag:isOptional()
            // this tag has no closing tag
            // a new opening tag is found
            DO CASE
            CASE ( Lower( cTagName ) == Lower( oThisTag:htmlTagName ) )
               // the next tag is the same like this tag
               // ( e.g. <p>|<tr>|<td>|<li>)
               lRewind := .T.
            CASE ( Lower( cTagName ) == Lower( oThisTag:parent:htmlTagName ) ) .AND. .NOT. oThisTag:isType( CM_LIST )
               // the next tag is the same like the parent tag
               // ( e.g. this is <td> and the next tag is <tr> )
               lRewind := .T.
            CASE ( Lower( ::htmlTagName ) $ "dd,dt" )
               // <dl><dt><dd> is a unique special case
               IF ( Lower( cTagName ) $ "dd,dt" )
                  // next tag is <dt> or <dd>
                  lRewind := .T.
               ENDIF
            ENDCASE

            IF lRewind
               // go back to previous node
               parser:p_pos := nStart - 1
            ENDIF
         ENDIF

         IF .NOT. lRewind
            IF cAttr == ""
               // tag has no attributes
               oNextTag := THtmlNode():new( oThisTag, cTagName )
            ELSE
               // attribute string has ">" at the end. Remove ">"
               oNextTag := THtmlNode():new( oThisTag, cTagName, Left( cAttr,Len(cAttr ) - 1 ) )
            ENDIF

            oThisTag:addNode( oNextTag )

            IF .NOT. oThisTag:isOptional() .AND. Lower( oThisTag:htmlTagName ) == Lower( ctagName )
               oThisTag:htmlEndTagName := "/" + oThisTag:htmlTagName
            ENDIF

            IF oNextTag:keepFormatting()
               // do not spoil formatting of Html text
               oNextTag:parseHtmlFixed( parser )

            ELSEIF .NOT. oNextTag:isEmpty()
               // parse into node list of new tag
               oThisTag := oNextTag

            ENDIF
         ENDIF
      END

      IF lRewind
         oThisTag := oThisTag:parent
         lRewind := .F.

         IF oThisTag == NIL
            oThisTag := self
            nLastPos := parser:p_len
            EXIT
         ENDIF
      ENDIF

      nLastPos := parser:p_pos
      IF nLastPos == 0
         EXIT
      ENDIF
   ENDDO

   IF nLastPos > 0 .AND. nLastPos < parser:p_len
      oThisTag:addNode( THtmlNode():new( self, "_text_", , SubStr( parser:p_str, nLastPos + 1 ) ) )
   ENDIF

   RETURN self

// parses a HTML string without any changes to indentation and line breaks

METHOD parseHtmlFixed( parser ) CLASS THtmlNode

   LOCAL nStart, nEnd

// keep entire Html text within tag
   nStart := parser:p_pos + 1
   P_SEEK( parser, "<" )

   IF P_NEXT( parser ) == "!" .AND. P_NEXT( parser ) == "["
      // <![CDATA[   ]]>
      P_SEEK( parser, "]]>" )
   ENDIF

   IF .NOT. P_PEEK( parser, "/" + ::htmlTagName )
      // seek  <  /endtag>
      P_SEEK( parser, "/" + ::htmlTagName )
   ENDIF

// back to "<"
   DO WHILE P_PREV( parser ) <> "<" ; ENDDO

   nEnd  := parser:p_pos
   ::addNode( THtmlNode():new( self, "_text_", , SubStr( parser:p_Str, nStart, nEnd - nStart ) ) )

   ::htmlEndTagName := "/" + ::htmlTagName
   P_SEEK( parser, ">" )

   RETURN self

// adds a new CHILD node to the current one

METHOD addNode( oTHtmlNode ) CLASS THtmlNode

   IF oTHtmlNode:parent <> NIL .AND. .NOT. oTHtmlNode:parent == self
      oTHtmlNode:delete()
   ENDIF

   oTHtmlNode:parent := self
   oTHtmlNode:root   := ::root

   AAdd( ::htmlContent, oTHtmlNode )

   IF ::root <> NIL .AND. ::root:_document <> NIL
      ::root:_document:changed := .T.
   ENDIF

   RETURN oTHtmlNode

// inserts a SIBLING node before the current one

METHOD insertBefore( oTHtmlNode ) CLASS THtmlNode

   IF ::parent == NIL
      RETURN ::error( "Cannot insert before root node", ::className(), ":insertBefore()", EG_ARG, hb_AParams() )
   ENDIF

   IF oTHtmlNode:parent <> NIL .AND. .NOT. oTHtmlNode:parent == self
      oTHtmlNode:delete()
   ENDIF

   oTHtmlNode:parent := ::parent
   oTHtmlNode:root   := ::root

   IF ::root <> NIL .AND. ::root:_document <> NIL
      ::root:_document:changed := .T.
   ENDIF

   IF ValType( ::parent:htmlContent ) == "A"
      AIns( ::parent:htmlContent, 1, oTHtmlNode, .T. )
   ENDIF

   RETURN oTHtmlNode

// inserts a SIBLING node after the current one

METHOD insertAfter( oTHtmlNode ) CLASS THtmlNode

   LOCAL nPos

   IF oTHtmlNode:parent <> NIL .AND. .NOT. oTHtmlNode:parent == self
      oTHtmlNode:delete()
   ENDIF

   oTHtmlNode:parent := ::parent
   oTHtmlNode:root   := ::root

   IF ::root <> NIL .AND. ::root:_document <> NIL
      ::root:_document:changed := .T.
   ENDIF

   nPos := AScan( ::parent:htmlContent, self ) + 1

   IF nPos > Len( ::parent:htmlContent )
      ::parent:addNode( oTHtmlNode )
   ELSE
      AIns( ::parent:htmlContent, nPos, oTHtmlNode, .T. )
   ENDIF

   RETURN oTHtmlNode

// deletes this node from the object tree

METHOD DELETE()  CLASS THtmlNode

   LOCAL nPos

   IF ::parent == NIL
      RETURN self
   ENDIF

   IF ::root <> NIL .AND. ::root:_document <> NIL
      ::root:_document:changed := .T.
   ENDIF

   IF ValType( ::parent:htmlContent ) == "A"
      nPos := AScan( ::parent:htmlContent, self )
      ADel( ::parent:htmlContent, nPos, .T. )
   ENDIF

   ::parent := NIL
   ::root   := NIL

   RETURN self

// returns first node in subtree (.F.) or first node of entire tree (.T.)

METHOD firstNode( lRoot ) CLASS THtmlNode

   IF ValType( lRoot ) <> "L"
      lRoot := .F.
   ENDIF

   IF lRoot
      RETURN ::root:htmlContent[1]
   ELSEIF ::htmlTagName == "_text_"
      RETURN ::parent:htmlContent[1]
   ENDIF

   RETURN iif( Empty( ::htmlContent ), NIL, ::htmlContent[1] )

// returns last node in subtree (.F.) or last node of entire tree (.T.)

METHOD lastNode( lRoot ) CLASS THtmlNode

   LOCAL aNodes

   IF ValType( lRoot ) <> "L"
      lRoot := .F.
   ENDIF
   IF ::htmlTagName == "_text_"
      RETURN ::parent:lastNode( lRoot )
   ENDIF
   aNodes := iif( lRoot, ::root:collect(), ::collect() )

   RETURN ATail( aNodes )

// returns next node

METHOD nextNode() CLASS THtmlNode

   LOCAL nPos, aNodes

   IF ::htmlTagName == "_root_"
      RETURN ::htmlContent[1]
   ENDIF

   IF ::htmlTagName <> "_text_" .AND. .NOT. Empty( ::htmlContent )
      RETURN ::htmlContent[1]
   ENDIF

   nPos := AScan( ::parent:htmlContent, {|o| o == self } )

   IF nPos <  Len( ::parent:htmlContent )
      RETURN ::parent:htmlContent[nPos+1]
   ENDIF

   aNodes := ::parent:parent:collect()
   nPos   := AScan( aNodes, {|o| o == self } )

   RETURN iif( nPos == Len( aNodes ), NIL, aNodes[nPos+1] )

// returns previous node

METHOD prevNode() CLASS THtmlNode

   LOCAL nPos, aNodes

   IF ::htmlTagName == "_root_"
      RETURN NIL
   ENDIF

   aNodes := ::parent:collect( self )
   nPos   := AScan( aNodes, {|o| o == self } )

   RETURN iif( nPos == 1, ::parent, aNodes[nPos-1] )

// creates HTML code for this node

METHOD toString( nIndent ) CLASS THtmlNode

   LOCAL cIndent, cHtml := "", oNode, i, imax

   IF ::htmlTagName == "_text_"
      // a leaf has no child nodes
      RETURN ::htmlContent
   ENDIF

   IF nIndent == NIL
      nIndent := - 1
   ENDIF

   cIndent := iif( ::keepFormatting(), "", Space( Max(0,nIndent ) ) )

   IF .NOT. ::htmlTagName == "_root_"
      // all nodes but the root node have a HTML tag
      IF .NOT. ::isInline() .OR. ::htmlTagName == "!--"
         cHtml += cIndent
      ELSEIF ::keepFormatting()
         cHtml += Chr( 13 ) + Chr( 10 )
      ENDIF
      cHtml += "<" + ::htmlTagName + ::attrToString()

      IF .NOT. ::htmlEndTagName == "/"
         cHtml += ">"
      ENDIF
   ENDIF

   IF ValType( ::htmlContent ) == "A"

#ifdef FOR_EACH_NESTING_LIMIT_IS_ONLY_16_AND_FAR_TOO_SMALL
      imax := Len( ::htmlContent )
      FOR i := 1 TO imax
         oNode := ::htmlContent[i]
         IF .NOT. oNode:isInline() .OR. oNode:htmlTagName == "!--"
            cHtml += Chr( 13 ) + Chr( 10 )
         ENDIF
         cHtml += oNode:toString( nIndent + 1 )
      NEXT
#else
      FOR EACH oNode IN ::htmlContent
         IF .NOT. oNode:isInline() .OR. oNode:htmlTagName == "!--"
            cHtml += Chr( 13 ) + Chr( 10 )
         ENDIF
         cHtml += oNode:toString( nIndent + 1 )
      NEXT
#endif

   ELSEIF ValType( ::htmlContent ) == "C"
      cHtml += ::htmlContent
   ENDIF

   IF ::htmlEndTagName <> NIL
      IF ::isInline() .OR. ::keepFormatting() .OR. ::isType( CM_HEADING ) .OR. ::isType( CM_HEAD )
         RETURN cHtml += iif( ::htmlEndTagName == "/", " />", "<" + ::htmlEndTagName + ">" )
      ENDIF
      IF cHtml[-1] <> Chr( 10 )
         cHtml += Chr( 13 ) + Chr( 10 )
      ENDIF
      RETURN cHtml += cIndent + iif( ::htmlEndTagName == "/", " />", "<" + ::htmlEndTagName + ">" )
   ELSEIF ::htmlTagName $ "!--,br"
      RETURN cHtml += Chr( 13 ) + Chr( 10 ) + cIndent
   ENDIF

   RETURN cHtml

// Builds the attribute string

METHOD attrToString() CLASS THtmlNode

   LOCAL aAttr, cAttr

   IF ::htmlAttributes == NIL
      cAttr := ""

   ELSEIF ValType( ::htmlAttributes ) == "C"
      cAttr := " " + ::htmlAttributes

   ELSE
      // attributes are parsed into a Hash
      TRY
         aAttr := HB_Exec( ::htmlTagType[1] )
      CATCH
         // Tag has no attributes
         aAttr := {}
      END
      cAttr := ""
      HEval( ::htmlAttributes, {|cKey, cValue| cAttr += __AttrToStr( cKey, cValue, aAttr, self ) } )
   ENDIF

   RETURN cAttr

STATIC FUNCTION __AttrToStr( cName, cValue, aAttr, oTHtmlNode )

   LOCAL nPos

   IF ( nPos := AScan( aAttr, {|a| a[1] == Lower( cName ) } ) ) == 0
      // Tag doesn't have this attribute
      RETURN oTHtmlNode:error( "Invalid HTML attribute for: <" + oTHtmlNode:htmlTagName + ">", oTHtmlNode:className(), cName, EG_ARG, { cName, cValue } )
   ENDIF

   IF aAttr[nPos,2] == HTML_ATTR_TYPE_BOOL
      RETURN " " + cName
   ENDIF

   RETURN " " + cName + "=" + '"' + cValue + '"'

// collects all (sub)nodes of the tree in a one dimensional array

METHOD collect( oEndNode ) CLASS THtmlNode

   LOCAL stack, oSubNode

   stack := S_STACK()

   IF ::htmlTagName == "_root_"
      FOR EACH oSubNode IN ::htmlContent
         __CollectTags( oSubNode, stack, oEndNode )
      NEXT
   ELSE
      __CollectTags( self, stack, oEndNode )
   ENDIF

   S_COMPRESS( stack )

   RETURN stack[S_DATA]

#ifdef FOR_EACH_NESTING_LIMIT_IS_ONLY_16_AND_FAR_TOO_SMALL

STATIC FUNCTION __CollectTags( oTHtmlNode, stack, oEndNode )

   LOCAL i, imax

   S_PUSH( stack, oTHtmlNode )

   IF oTHtmlNode:isNode() .AND. .NOT. oTHtmlNode == oEndNode
      imax := Len( oTHtmlNode:htmlContent )
      FOR i := 1 TO imax
         __CollectTags( oTHtmlNode:htmlContent[i], stack, oEndNode )
      NEXT
   ENDIF

   RETURN stack

#else

// $HZ BUG: Unrecoverable error 9000: FOR EACH excessive nesting!

STATIC FUNCTION __CollectTags( oTHtmlNode, stack, oEndNode )

   LOCAL oSubNode

   S_PUSH( stack, oTHtmlNode )

   IF oTHtmlNode:isNode() .AND. .NOT. oTHtmlNode == oEndNode
      FOR EACH oSubNode IN oTHtmlNode:htmlContent
         __CollectTags( oSubNode, stack, oEndNode )
      NEXT
   ENDIF

   RETURN stack

#endif

// Retrieves the textual content of a node

METHOD getText( cEOL ) CLASS THtmlNode

   LOCAL cText := ""
   LOCAL oNode, i, imax

   IF cEOL == NIL
      cEOL := hb_osNewLine()
   ENDIF

   IF ::htmlTagName == "_text_"
      RETURN Trim( ::htmlContent ) + cEOL
   ENDIF

#ifdef FOR_EACH_NESTING_LIMIT_IS_ONLY_16_AND_FAR_TOO_SMALL

   imax := Len( ::htmlContent )
   FOR i := 1 TO imax
      oNode := ::htmlContent[i]
      cText += oNode:getText( cEOL )
      IF Lower( ::htmlTagName ) $ "td,th" .AND. AScan( ::parent:htmlContent, {|o| o == self } ) < Len( ::parent:htmlContent )
         // leave table rows in one line, cells separated by Tab
         cText := SubStr( cText, 1, Len( cText ) - Len( cEol ) )
         cText += Chr( 9 )
      ENDIF
   NEXT

#else

   FOR EACH oNode IN ::htmlContent
      cText += oNode:getText( cEOL )
      IF Lower( ::htmlTagName ) $ "td,th" .AND. AScan( ::parent:htmlContent, {|o| o == self } ) < Len( ::parent:htmlContent )
         // leave table rows in one line, cells separated by Tab
         cText := SubStr( cText, 1, Len( cText ) - Len( cEol ) )
         cText += Chr( 9 )
      ENDIF
   NEXT

#endif

   RETURN cText

// Returns the value of an HTML attribute

METHOD getAttribute( cName ) CLASS THtmlNode

   LOCAL hHash := ::getAttributes()
   LOCAL cValue

   IF ValType( hHash ) <> "H"
      RETURN hHash
   ENDIF

   TRY
      cValue := hHash[cName]
   CATCH
      cValue := NIL
   END

   RETURN cValue

// Returns all HTML attributes as a Hash

METHOD getAttributes() CLASS THtmlNode

   IF ::htmlTagType[1] == NIL
      // Tag has no valid attributes
      RETURN NIL

   ELSEIF ::htmlTagName[1] == "!"
      // <!DOCTYPE > and <!-- comments --> have no HTML attributes
      RETURN ::htmlAttributes

   ELSEIF ::htmlAttributes == NIL
      ::htmlAttributes := Hash()
      HSetCaseMatch( ::htmlAttributes, .F. )

   ELSEIF ValType( ::htmlAttributes ) == "C"
      IF ::htmlAttributes == "/"
         ::htmlAttributes := Hash()
         HSetCaseMatch( ::htmlAttributes, .F. )
      ELSE
         ::htmlAttributes := __ParseAttr( P_PARSER( AllTrim(::htmlAttributes ) ) )
      ENDIF
   ENDIF

   RETURN ::htmlAttributes

// HTML attribute parser

STATIC FUNCTION __ParseAttr( parser )

   LOCAL cChr, nMode := 1 // 1=name, 2=value
   LOCAL aAttr := { "", "" }
   LOCAL hHash := Hash()
   LOCAL nStart, nEnd
   LOCAL lIsQuoted := .F.

   HSetCaseMatch( hHash, .F. )

   DO WHILE .NOT. ( cChr := P_NEXT( parser ) ) == ""

      SWITCH cChr
      CASE "="
         lIsQuoted := .F.

         IF nMode == 2
            aAttr[2] += "="
         ELSE
            nMode := 2
         ENDIF
         EXIT

      CASE " "
         IF nMode == 1
            IF .NOT. aAttr[1] == ""
               hHash[ aAttr[1] ] := aAttr[2]
               aAttr[1] := ""
               aAttr[2] := ""
            ENDIF
            LOOP
         ENDIF

         nMode := iif( lIsQuoted, 2, 1 )
         hHash[ aAttr[1] ] := aAttr[2]
         aAttr[1] := ""
         aAttr[2] := ""

         DO WHILE P_NEXT( parser ) == " "
         ENDDO

         IF parser:p_pos > Len( parser:p_str )
            RETURN hHash
         ENDIF

         parser:p_end := parser:p_pos
         parser:p_pos --
         EXIT

      CASE '"'
      CASE "'"
         lIsQuoted := .T.
         parser:p_end := parser:p_pos
         parser:p_pos ++

         nStart := parser:p_pos

         IF parser:p_str[nStart] == cChr
            // empty value ""
            hHash[ aAttr[1] ] := ""
            parser:p_end := parser:p_pos
            parser:p_pos --
         ELSE
            P_SEEK( parser, cChr )
            nEnd := parser:p_pos

            IF nEnd > 0
               aAttr[2] := SubStr( parser:p_str, nStart, nEnd - nStart )
            ELSE
               aAttr[2] := SubStr( parser:p_str, nStart )
            ENDIF

            hHash[ aAttr[1] ] := aAttr[2]
         ENDIF

         aAttr[1] := ""
         aAttr[2] := ""
         nMode := 1

         IF nEnd == 0
            RETURN hHash
         ENDIF

         EXIT

         DEFAULT
         aAttr[nMode] += cChr
      END
   ENDDO

   IF .NOT. aAttr[1] == ""
      hHash[ aAttr[1] ] := aAttr[2]
   ENDIF

   RETURN hHash

// Sets one attribute and value

METHOD setAttribute( cName, cValue ) CLASS THtmlNode

   LOCAL aAttr
   LOCAL nPos
   LOCAL nType
   LOCAL hHash := ::getAttributes()

   IF ValType( hHash ) <> "H"
      // Tag doesn't have any attribute
      RETURN ::error( "Invalid HTML attribute for: <" + ::htmlTagName + ">", ::className(), cName, EG_ARG, { cName, cValue } )
   ENDIF

   TRY
      aAttr := HB_Exec( ::htmlTagType[1] )
   CATCH
      // Tag has no attributes
      aAttr := {}
   END

   IF ( nPos := AScan( aAttr, {|a| a[1] == Lower( cName ) } ) ) == 0
      // Tag doesn't have this attribute
      RETURN ::error( "Invalid HTML attribute for: <" + ::htmlTagName + ">", ::className(), cName, EG_ARG, { cName, cValue } )
   ENDIF

   nType := aAttr[nPos,2]
   IF nType == HTML_ATTR_TYPE_BOOL
      hHash[cName] := ""
   ELSE
      hHash[cName] := cValue
   ENDIF

   RETURN hHash[cName]

// Sets all attribute and values

METHOD setAttributes( cHtml ) CLASS THtmlNode

   ::htmlAttributes := cHtml

   RETURN ::getAttributes()

// Removes one attribute

METHOD delAttribute( cName ) CLASS THtmlNode

   LOCAL xVal := ::getAttribute( cName )
   LOCAL lRet := .F.

   IF xVal <> NIL
      TRY
         HDel( ::htmlAttributes, cName )
         lRet := .T.
      CATCH
         lRet := .F.
      END
   ENDIF

   RETURN lRet

// Removes all attributes

METHOD delAttributes() CLASS THtmlNode

   ::htmlAttributes := NIL

   RETURN .T.

// Checks for the existence of an attribute

METHOD isAttribute( cName ) CLASS THtmlNode

   LOCAL lRet

   TRY
      lRet := HHasKey( ::getAttributes(), cName )
   CATCH
      lRet := .F.
   END

   RETURN lRet

// Error handling

METHOD noMessage( ... ) CLASS THtmlNode

   LOCAL aParams := hb_AParams()

   RETURN ::noAttribute( __GetMessage(), aParams )

// Non existent message -> returns and/or creates Tag or Attribute

METHOD noAttribute( cName, aValue ) CLASS THtmlNode

   LOCAL oNode

   cName := Lower( cName )

   IF cName[1] == "_"
      cName := SubStr( cName, 2 )
   ENDIF

   IF HHasKey( shTagTypes, cName )
      // message identifies a html tag
      oNode := ::findNodeByTagName( cName )

      IF oNode == NIL
         oNode := THtmlNode():new( self, cName )
         IF .NOT. oNode:isOptional() .AND. .NOT. oNode:isEmpty()
            oNode:htmlEndTagName := "/" + cName
         ENDIF
         ::addNode( oNode )
      ENDIF

      RETURN oNode

   ELSEIF cName[-1] == "s" .AND. HHasKey( shTagTypes, Left( cName, Len(cName ) - 1 ) )
      // message is the plural of a html tag -> oNode:forms -> Array of <FORM> tags
      RETURN ::findNodesByTagName( Left( cName, Len(cName ) - 1 ), ATail( aValue ) )
   ENDIF

   IF ! Empty( aValue )
      RETURN ::setAttribute( cName, aValue[1] )
   ENDIF

   RETURN ::getAttribute( cName )

// finds the first node in tree with this name

METHOD findNodeByTagName( cName ) CLASS THtmlNode

   LOCAL aNodes := ::collect()
   LOCAL oNode

   FOR EACH oNode IN aNodes
      IF Lower( oNode:htmlTagName ) == Lower( cName )
         RETURN oNode
      ENDIF
   NEXT

   RETURN NIL

// collects all nodes in tree with this name

METHOD findNodesByTagName( cName, nOrdinal ) CLASS THtmlNode

   LOCAL aNodes := ::collect()
   LOCAL oNode
   LOCAL aRet := {}

   FOR EACH oNode IN aNodes
      IF Lower( oNode:htmlTagName ) == Lower( cName )
         AAdd( aRet, oNode )
      ENDIF
   NEXT

   IF ValType( nOrdinal ) == "N"
      IF nOrdinal < 1 .OR. nOrdinal > Len( aRet )
         RETURN NIL
      ENDIF
      RETURN aRet[nOrdinal]
   ENDIF

   RETURN aRet

// returns the text node of this node

METHOD _getTextNode() CLASS THtmlNode

   IF ::htmlTagName == "_text_"
      RETURN self
   ENDIF

   ::addNode( THtmlNode():new( self, "_text_", , "" ) )

   RETURN ATail( ::htmlContent )

// assigns text to a text node of this node

METHOD _setTextNode( cText ) CLASS THtmlNode

   LOCAL oNode   := ::_getTextNode()

   cText   := LTrim( hb_ValToStr( cText ) )

   DO WHILE "<" $ cText
      cText := StrTran( cText, "<", "&lt;" )
   ENDDO

   DO WHILE ">" $ cText
      cText := StrTran( cText, ">", "&gt;" )
   ENDDO

   oNode:htmlContent := iif( cText == "", "&nbsp;", cText )

   RETURN self

// called by "+" operator
// Creates a new node of the specified tag name and raises error if cTagName is invalid

METHOD pushNode( cTagName ) CLASS THtmlNode

   LOCAL oNode
   LOCAL cAttr := AllTrim( cTagName )
   LOCAL cName := CutStr( " ", @cAttr )

   IF ::isEmpty()
      RETURN ::error( "Cannot add HTML tag to: <" + ::htmlTagName + ">", ::className(), "+", EG_ARG, { cName } )
   ENDIF

   IF .NOT. HHasKey( shTagTypes, cName )
      IF cName[1] == "/" .AND. HHasKey( shTagTypes, SubStr( cName,2 ) )
         IF .NOT. Lower( SubStr( cName,2 ) ) == Lower( ::htmlTagName )
            RETURN ::error( "Not a valid closing HTML tag for: <" + ::htmlTagName + ">", ::className(), "-", EG_ARG, { cName } )
         ENDIF
         RETURN self:parent
      ENDIF
      RETURN ::error( "Invalid HTML tag", ::className(), "+", EG_ARG, { cName } )
   ENDIF

   IF LTrim( cAttr ) == ""
      cAttr := NIL
   ENDIF

   oNode := THtmlNode():new( self, cName, cAttr )
   IF .NOT. oNode:isOptional() .AND. .NOT. oNode:isEmpty()
      oNode:htmlEndTagName := "/" + cName
   ENDIF
   ::addNode( oNode )

   RETURN oNode

// called by "-" operator
// returns the parent of this node and raises error if cName is an invalid closing tag

METHOD popNode( cName ) CLASS THtmlNode

   cName := LTrim( cName )

   IF cName[1] == "/"
      cName := SubStr( cName, 2 )
   ENDIF

   IF .NOT. Lower( cName ) == Lower( ::htmlTagName )
      RETURN ::error( "Invalid closing HTML tag for: <" + ::htmlTagName + ">", ::className(), "-", EG_ARG, { cName } )
   ENDIF

   RETURN self:parent

// Generic parsing function

STATIC FUNCTION CutStr( cCut, cString )

   LOCAL cLeftPart, i := At( cCut, cString )

   IF i > 0
      cLeftPart := Left  ( cString, i - 1 )
      cString   := SubStr( cString, i + Len( cCut ) )
   ELSE
      cLeftPart := cString
      cString   := ""
   ENDIF

   RETURN cLeftPart

FUNCTION THtmlInit( lInit )

   IF ValType( lInit ) == "L" .AND. .NOT. lInit
      saHtmlAttr         := NIL
      shTagTypes         := NIL
      saHtmlAnsiEntities := NIL
      RETURN .NOT. ( slInit := .F. )
   ELSEIF .NOT. slInit
      saHtmlAttr := Array( HTML_ATTR_COUNT )
      _Init_Html_AnsiCharacterEntities()
      _Init_Html_Attributes()
      _Init_Html_TagTypes()
      slInit := .T.
   ENDIF

   RETURN .T.

FUNCTION THtmlCleanup()

   RETURN THtmlInit( .F. )

FUNCTION THtmlTagType( cTagName )

   LOCAL aType

   IF shTagTypes == NIL
      THtmlInit()
   ENDIF

   TRY
      aType := shTagTypes[ cTagName ]
   CATCH
      aType := shTagTypes[ "_text_" ]
   END

   RETURN aType

FUNCTION THtmlIsValid( cTagName, cAttrName )

   LOCAL lRet := .T. , aValue

   IF shTagTypes == NIL
      THtmlInit()
   ENDIF

   TRY
      aValue := shTagTypes[ cTagName ]
      IF cAttrName <> NIL
         aValue := HB_Exec( aValue[1] )
         lRet   := ( AScan( aValue, {|a| Lower(a[1] ) == Lower( cAttrName ) } ) > 0 )
      ENDIF
   CATCH
      lRet := .F.
   END

   RETURN lRet

/*
  HTML Tag data are adopted for xHarbour from Tidy.exe (www.sourceforge.net/tidy)
*/

STATIC PROCEDURE _Init_Html_TagTypes

   shTagTypes := Hash()

   HSetCaseMatch( shTagTypes, .F. )

   shTagTypes[ "_root_"     ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "_text_"     ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "!--"        ] := { NIL                         , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "a"          ] := { ( @THtmlAttr_A() )          , ( CM_INLINE )                                   }
   shTagTypes[ "abbr"       ] := { ( @THtmlAttr_ABBR() )       , ( CM_INLINE )                                   }
   shTagTypes[ "acronym"    ] := { ( @THtmlAttr_ACRONYM() )    , ( CM_INLINE )                                   }
   shTagTypes[ "address"    ] := { ( @THtmlAttr_ADDRESS() )    , ( CM_BLOCK )                                    }
   shTagTypes[ "align"      ] := { NIL                         , ( CM_BLOCK )                                    }
   shTagTypes[ "applet"     ] := { ( @THtmlAttr_APPLET() )     , ( CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM )         }
   shTagTypes[ "area"       ] := { ( @THtmlAttr_AREA() )       , ( CM_BLOCK|CM_EMPTY )                           }
   shTagTypes[ "b"          ] := { ( @THtmlAttr_B() )          , ( CM_INLINE )                                   }
   shTagTypes[ "base"       ] := { ( @THtmlAttr_BASE() )       , ( CM_HEAD|CM_EMPTY )                            }
   shTagTypes[ "basefont"   ] := { ( @THtmlAttr_BASEFONT() )   , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "bdo"        ] := { ( @THtmlAttr_BDO() )        , ( CM_INLINE )                                   }
   shTagTypes[ "bgsound"    ] := { NIL                         , ( CM_HEAD|CM_EMPTY )                            }
   shTagTypes[ "big"        ] := { ( @THtmlAttr_BIG() )        , ( CM_INLINE )                                   }
   shTagTypes[ "blink"      ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "blockquote" ] := { ( @THtmlAttr_BLOCKQUOTE() ) , ( CM_BLOCK )                                    }
   shTagTypes[ "body"       ] := { ( @THtmlAttr_BODY() )       , ( CM_HTML|CM_OPT|CM_OMITST )                    }
   shTagTypes[ "br"         ] := { ( @THtmlAttr_BR() )         , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "button"     ] := { ( @THtmlAttr_BUTTON() )     , ( CM_INLINE )                                   }
   shTagTypes[ "caption"    ] := { ( @THtmlAttr_CAPTION() )    , ( CM_TABLE )                                    }
   shTagTypes[ "center"     ] := { ( @THtmlAttr_CENTER() )     , ( CM_BLOCK )                                    }
   shTagTypes[ "cite"       ] := { ( @THtmlAttr_CITE() )       , ( CM_INLINE )                                   }
   shTagTypes[ "code"       ] := { ( @THtmlAttr_CODE() )       , ( CM_INLINE )                                   }
   shTagTypes[ "col"        ] := { ( @THtmlAttr_COL() )        , ( CM_TABLE|CM_EMPTY )                           }
   shTagTypes[ "colgroup"   ] := { ( @THtmlAttr_COLGROUP() )   , ( CM_TABLE|CM_OPT )                             }
   shTagTypes[ "comment"    ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "dd"         ] := { ( @THtmlAttr_DD() )         , ( CM_DEFLIST|CM_OPT|CM_NO_INDENT )              }
   shTagTypes[ "del"        ] := { ( @THtmlAttr_DEL() )        , ( CM_INLINE|CM_BLOCK|CM_MIXED )                 }
   shTagTypes[ "dfn"        ] := { ( @THtmlAttr_DFN() )        , ( CM_INLINE )                                   }
   shTagTypes[ "dir"        ] := { ( @THtmlAttr_DIR() )        , ( CM_BLOCK|CM_OBSOLETE )                        }
   shTagTypes[ "div"        ] := { ( @THtmlAttr_DIV() )        , ( CM_BLOCK )                                    }
   shTagTypes[ "dl"         ] := { ( @THtmlAttr_DL() )         , ( CM_BLOCK )                                    }
   shTagTypes[ "dt"         ] := { ( @THtmlAttr_DT() )         , ( CM_DEFLIST|CM_OPT|CM_NO_INDENT )              }
   shTagTypes[ "em"         ] := { ( @THtmlAttr_EM() )         , ( CM_INLINE )                                   }
   shTagTypes[ "embed"      ] := { NIL                         , ( CM_INLINE|CM_IMG|CM_EMPTY )                   }
   shTagTypes[ "fieldset"   ] := { ( @THtmlAttr_FIELDSET() )   , ( CM_BLOCK )                                    }
   shTagTypes[ "font"       ] := { ( @THtmlAttr_FONT() )       , ( CM_INLINE )                                   }
   shTagTypes[ "form"       ] := { ( @THtmlAttr_FORM() )       , ( CM_BLOCK )                                    }
   shTagTypes[ "frame"      ] := { ( @THtmlAttr_FRAME() )      , ( CM_FRAMES|CM_EMPTY )                          }
   shTagTypes[ "frameset"   ] := { ( @THtmlAttr_FRAMESET() )   , ( CM_HTML|CM_FRAMES )                           }
   shTagTypes[ "h1"         ] := { ( @THtmlAttr_H1() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "h2"         ] := { ( @THtmlAttr_H2() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "h3"         ] := { ( @THtmlAttr_H3() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "h4"         ] := { ( @THtmlAttr_H4() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "h5"         ] := { ( @THtmlAttr_H5() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "h6"         ] := { ( @THtmlAttr_H6() )         , ( CM_BLOCK|CM_HEADING )                         }
   shTagTypes[ "head"       ] := { ( @THtmlAttr_HEAD() )       , ( CM_HTML|CM_OPT|CM_OMITST )                    }
   shTagTypes[ "hr"         ] := { ( @THtmlAttr_HR() )         , ( CM_BLOCK|CM_EMPTY )                           }
   shTagTypes[ "html"       ] := { ( @THtmlAttr_HTML() )       , ( CM_HTML|CM_OPT|CM_OMITST )                    }
   shTagTypes[ "i"          ] := { ( @THtmlAttr_I() )          , ( CM_INLINE )                                   }
   shTagTypes[ "iframe"     ] := { ( @THtmlAttr_IFRAME() )     , ( CM_INLINE )                                   }
   shTagTypes[ "ilayer"     ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "img"        ] := { ( @THtmlAttr_IMG() )        , ( CM_INLINE|CM_IMG|CM_EMPTY )                   }
   shTagTypes[ "input"      ] := { ( @THtmlAttr_INPUT() )      , ( CM_INLINE|CM_IMG|CM_EMPTY )                   }
   shTagTypes[ "ins"        ] := { ( @THtmlAttr_INS() )        , ( CM_INLINE|CM_BLOCK|CM_MIXED )                 }
   shTagTypes[ "isindex"    ] := { ( @THtmlAttr_ISINDEX() )    , ( CM_BLOCK|CM_EMPTY )                           }
   shTagTypes[ "kbd"        ] := { ( @THtmlAttr_KBD() )        , ( CM_INLINE )                                   }
   shTagTypes[ "keygen"     ] := { NIL                         , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "label"      ] := { ( @THtmlAttr_LABEL() )      , ( CM_INLINE )                                   }
   shTagTypes[ "layer"      ] := { NIL                         , ( CM_BLOCK )                                    }
   shTagTypes[ "legend"     ] := { ( @THtmlAttr_LEGEND() )     , ( CM_INLINE )                                   }
   shTagTypes[ "li"         ] := { ( @THtmlAttr_LI() )         , ( CM_LIST|CM_OPT|CM_NO_INDENT )                 }
   shTagTypes[ "link"       ] := { ( @THtmlAttr_LINK() )       , ( CM_HEAD|CM_EMPTY )                            }
   shTagTypes[ "listing"    ] := { ( @THtmlAttr_LISTING() )    , ( CM_BLOCK|CM_OBSOLETE )                        }
   shTagTypes[ "map"        ] := { ( @THtmlAttr_MAP() )        , ( CM_INLINE )                                   }
   shTagTypes[ "marquee"    ] := { NIL                         , ( CM_INLINE|CM_OPT )                            }
   shTagTypes[ "menu"       ] := { ( @THtmlAttr_MENU() )       , ( CM_BLOCK|CM_OBSOLETE )                        }
   shTagTypes[ "meta"       ] := { ( @THtmlAttr_META() )       , ( CM_HEAD|CM_EMPTY )                            }
   shTagTypes[ "multicol"   ] := { NIL                         , ( CM_BLOCK )                                    }
   shTagTypes[ "nextid"     ] := { ( @THtmlAttr_NEXTID() )     , ( CM_HEAD|CM_EMPTY )                            }
   shTagTypes[ "nobr"       ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "noembed"    ] := { NIL                         , ( CM_INLINE )                                   }
   shTagTypes[ "noframes"   ] := { ( @THtmlAttr_NOFRAMES() )   , ( CM_BLOCK|CM_FRAMES )                          }
   shTagTypes[ "nolayer"    ] := { NIL                         , ( CM_BLOCK|CM_INLINE|CM_MIXED )                 }
   shTagTypes[ "nosave"     ] := { NIL                         , ( CM_BLOCK )                                    }
   shTagTypes[ "noscript"   ] := { ( @THtmlAttr_NOSCRIPT() )   , ( CM_BLOCK|CM_INLINE|CM_MIXED )                 }
   shTagTypes[ "object"     ] := { ( @THtmlAttr_OBJECT() )     , ( CM_OBJECT|CM_HEAD|CM_IMG|CM_INLINE|CM_PARAM ) }
   shTagTypes[ "ol"         ] := { ( @THtmlAttr_OL() )         , ( CM_BLOCK )                                    }
   shTagTypes[ "optgroup"   ] := { ( @THtmlAttr_OPTGROUP() )   , ( CM_FIELD|CM_OPT )                             }
   shTagTypes[ "option"     ] := { ( @THtmlAttr_OPTION() )     , ( CM_FIELD|CM_OPT )                             }
   shTagTypes[ "p"          ] := { ( @THtmlAttr_P() )          , ( CM_BLOCK|CM_OPT )                             }
   shTagTypes[ "param"      ] := { ( @THtmlAttr_PARAM() )      , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "plaintext"  ] := { ( @THtmlAttr_PLAINTEXT() )  , ( CM_BLOCK|CM_OBSOLETE )                        }
   shTagTypes[ "pre"        ] := { ( @THtmlAttr_PRE() )        , ( CM_BLOCK )                                    }
   shTagTypes[ "q"          ] := { ( @THtmlAttr_Q() )          , ( CM_INLINE )                                   }
   shTagTypes[ "rb"         ] := { ( @THtmlAttr_RB() )         , ( CM_INLINE )                                   }
   shTagTypes[ "rbc"        ] := { ( @THtmlAttr_RBC() )        , ( CM_INLINE )                                   }
   shTagTypes[ "rp"         ] := { ( @THtmlAttr_RP() )         , ( CM_INLINE )                                   }
   shTagTypes[ "rt"         ] := { ( @THtmlAttr_RT() )         , ( CM_INLINE )                                   }
   shTagTypes[ "rtc"        ] := { ( @THtmlAttr_RTC() )        , ( CM_INLINE )                                   }
   shTagTypes[ "ruby"       ] := { ( @THtmlAttr_RUBY() )       , ( CM_INLINE )                                   }
   shTagTypes[ "s"          ] := { ( @THtmlAttr_S() )          , ( CM_INLINE )                                   }
   shTagTypes[ "samp"       ] := { ( @THtmlAttr_SAMP() )       , ( CM_INLINE )                                   }
   shTagTypes[ "script"     ] := { ( @THtmlAttr_SCRIPT() )     , ( CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE )         }
   shTagTypes[ "select"     ] := { ( @THtmlAttr_SELECT() )     , ( CM_INLINE|CM_FIELD )                          }
   shTagTypes[ "server"     ] := { NIL                         , ( CM_HEAD|CM_MIXED|CM_BLOCK|CM_INLINE )         }
   shTagTypes[ "servlet"    ] := { NIL                         , ( CM_OBJECT|CM_IMG|CM_INLINE|CM_PARAM )         }
   shTagTypes[ "small"      ] := { ( @THtmlAttr_SMALL() )      , ( CM_INLINE )                                   }
   shTagTypes[ "spacer"     ] := { NIL                         , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "span"       ] := { ( @THtmlAttr_SPAN() )       , ( CM_INLINE )                                   }
   shTagTypes[ "strike"     ] := { ( @THtmlAttr_STRIKE() )     , ( CM_INLINE )                                   }
   shTagTypes[ "strong"     ] := { ( @THtmlAttr_STRONG() )     , ( CM_INLINE )                                   }
   shTagTypes[ "style"      ] := { ( @THtmlAttr_STYLE() )      , ( CM_HEAD )                                     }
   shTagTypes[ "sub"        ] := { ( @THtmlAttr_SUB() )        , ( CM_INLINE )                                   }
   shTagTypes[ "sup"        ] := { ( @THtmlAttr_SUP() )        , ( CM_INLINE )                                   }
   shTagTypes[ "table"      ] := { ( @THtmlAttr_TABLE() )      , ( CM_BLOCK )                                    }
   shTagTypes[ "tbody"      ] := { ( @THtmlAttr_TBODY() )      , ( CM_TABLE|CM_ROWGRP|CM_OPT )                   }
   shTagTypes[ "td"         ] := { ( @THtmlAttr_TD() )         , ( CM_ROW|CM_OPT|CM_NO_INDENT )                  }
   shTagTypes[ "textarea"   ] := { ( @THtmlAttr_TEXTAREA() )   , ( CM_INLINE|CM_FIELD )                          }
   shTagTypes[ "tfoot"      ] := { ( @THtmlAttr_TFOOT() )      , ( CM_TABLE|CM_ROWGRP|CM_OPT )                   }
   shTagTypes[ "th"         ] := { ( @THtmlAttr_TH() )         , ( CM_ROW|CM_OPT|CM_NO_INDENT )                  }
   shTagTypes[ "thead"      ] := { ( @THtmlAttr_THEAD() )      , ( CM_TABLE|CM_ROWGRP|CM_OPT )                   }
   shTagTypes[ "title"      ] := { ( @THtmlAttr_TITLE() )      , ( CM_HEAD )                                     }
   shTagTypes[ "tr"         ] := { ( @THtmlAttr_TR() )         , ( CM_TABLE|CM_OPT )                             }
   shTagTypes[ "tt"         ] := { ( @THtmlAttr_TT() )         , ( CM_INLINE )                                   }
   shTagTypes[ "u"          ] := { ( @THtmlAttr_U() )          , ( CM_INLINE )                                   }
   shTagTypes[ "ul"         ] := { ( @THtmlAttr_UL() )         , ( CM_BLOCK )                                    }
   shTagTypes[ "var"        ] := { ( @THtmlAttr_VAR() )        , ( CM_INLINE )                                   }
   shTagTypes[ "wbr"        ] := { NIL                         , ( CM_INLINE|CM_EMPTY )                          }
   shTagTypes[ "xmp"        ] := { ( @THtmlAttr_XMP() )        , ( CM_BLOCK|CM_OBSOLETE )                        }

   RETURN


/*
  HTML Tag attribute data are adopted for xHarbour from Tidy.exe (www.sourceforge.net/tidy)
*/

STATIC PROCEDURE _Init_Html_Attributes

// attribute    NAME                TYPE
   saHtmlAttr[ HTML_ATTR_ABBR             ] := { "abbr"             , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ACCEPT           ] := { "accept"           , HTML_ATTR_TYPE_XTYPE     }
   saHtmlAttr[ HTML_ATTR_ACCEPT_CHARSET   ] := { "accept-charset"   , HTML_ATTR_TYPE_CHARSET   }
   saHtmlAttr[ HTML_ATTR_ACCESSKEY        ] := { "accesskey"        , HTML_ATTR_TYPE_CHARACTER }
   saHtmlAttr[ HTML_ATTR_ACTION           ] := { "action"           , HTML_ATTR_TYPE_ACTION    }
   saHtmlAttr[ HTML_ATTR_ADD_DATE         ] := { "add_date"         , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ALIGN            ] := { "align"            , HTML_ATTR_TYPE_ALIGN     }
   saHtmlAttr[ HTML_ATTR_ALINK            ] := { "alink"            , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_ALT              ] := { "alt"              , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ARCHIVE          ] := { "archive"          , HTML_ATTR_TYPE_URLS      }
   saHtmlAttr[ HTML_ATTR_AXIS             ] := { "axis"             , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_BACKGROUND       ] := { "background"       , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_BGCOLOR          ] := { "bgcolor"          , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_BGPROPERTIES     ] := { "bgproperties"     , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_BORDER           ] := { "border"           , HTML_ATTR_TYPE_BORDER    }
   saHtmlAttr[ HTML_ATTR_BORDERCOLOR      ] := { "bordercolor"      , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_BOTTOMMARGIN     ] := { "bottommargin"     , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_CELLPADDING      ] := { "cellpadding"      , HTML_ATTR_TYPE_LENGTH    }
   saHtmlAttr[ HTML_ATTR_CELLSPACING      ] := { "cellspacing"      , HTML_ATTR_TYPE_LENGTH    }
   saHtmlAttr[ HTML_ATTR_CHAR             ] := { "char"             , HTML_ATTR_TYPE_CHARACTER }
   saHtmlAttr[ HTML_ATTR_CHAROFF          ] := { "charoff"          , HTML_ATTR_TYPE_LENGTH    }
   saHtmlAttr[ HTML_ATTR_CHARSET          ] := { "charset"          , HTML_ATTR_TYPE_CHARSET   }
   saHtmlAttr[ HTML_ATTR_CHECKED          ] := { "checked"          , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_CITE             ] := { "cite"             , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_CLASS            ] := { "class"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_CLASSID          ] := { "classid"          , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_CLEAR            ] := { "clear"            , HTML_ATTR_TYPE_CLEAR     }
   saHtmlAttr[ HTML_ATTR_CODE             ] := { "code"             , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_CODEBASE         ] := { "codebase"         , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_CODETYPE         ] := { "codetype"         , HTML_ATTR_TYPE_XTYPE     }
   saHtmlAttr[ HTML_ATTR_COLOR            ] := { "color"            , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_COLS             ] := { "cols"             , HTML_ATTR_TYPE_COLS      }
   saHtmlAttr[ HTML_ATTR_COLSPAN          ] := { "colspan"          , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_COMPACT          ] := { "compact"          , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_CONTENT          ] := { "content"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_COORDS           ] := { "coords"           , HTML_ATTR_TYPE_COORDS    }
   saHtmlAttr[ HTML_ATTR_DATA             ] := { "data"             , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_DATAFLD          ] := { "datafld"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_DATAFORMATAS     ] := { "dataformatas"     , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_DATAPAGESIZE     ] := { "datapagesize"     , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_DATASRC          ] := { "datasrc"          , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_DATETIME         ] := { "datetime"         , HTML_ATTR_TYPE_DATE      }
   saHtmlAttr[ HTML_ATTR_DECLARE          ] := { "declare"          , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_DEFER            ] := { "defer"            , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_DIR              ] := { "dir"              , HTML_ATTR_TYPE_TEXTDIR   }
   saHtmlAttr[ HTML_ATTR_DISABLED         ] := { "disabled"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_ENCODING         ] := { "encoding"         , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ENCTYPE          ] := { "enctype"          , HTML_ATTR_TYPE_XTYPE     }
   saHtmlAttr[ HTML_ATTR_EVENT            ] := { "event"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_FACE             ] := { "face"             , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_FOR              ] := { "for"              , HTML_ATTR_TYPE_IDREF     }
   saHtmlAttr[ HTML_ATTR_FRAME            ] := { "frame"            , HTML_ATTR_TYPE_TFRAME    }
   saHtmlAttr[ HTML_ATTR_FRAMEBORDER      ] := { "frameborder"      , HTML_ATTR_TYPE_FBORDER   }
   saHtmlAttr[ HTML_ATTR_FRAMESPACING     ] := { "framespacing"     , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_GRIDX            ] := { "gridx"            , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_GRIDY            ] := { "gridy"            , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_HEADERS          ] := { "headers"          , HTML_ATTR_TYPE_IDREFS    }
   saHtmlAttr[ HTML_ATTR_HEIGHT           ] := { "height"           , HTML_ATTR_TYPE_LENGTH    }
   saHtmlAttr[ HTML_ATTR_HREF             ] := { "href"             , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_HREFLANG         ] := { "hreflang"         , HTML_ATTR_TYPE_LANG      }
   saHtmlAttr[ HTML_ATTR_HSPACE           ] := { "hspace"           , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_HTTP_EQUIV       ] := { "http-equiv"       , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ID               ] := { "id"               , HTML_ATTR_TYPE_IDDEF     }
   saHtmlAttr[ HTML_ATTR_ISMAP            ] := { "ismap"            , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_LABEL            ] := { "label"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_LANG             ] := { "lang"             , HTML_ATTR_TYPE_LANG      }
   saHtmlAttr[ HTML_ATTR_LANGUAGE         ] := { "language"         , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_LAST_MODIFIED    ] := { "last_modified"    , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_LAST_VISIT       ] := { "last_visit"       , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_LEFTMARGIN       ] := { "leftmargin"       , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_LINK             ] := { "link"             , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_LONGDESC         ] := { "longdesc"         , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_LOWSRC           ] := { "lowsrc"           , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_MARGINHEIGHT     ] := { "marginheight"     , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_MARGINWIDTH      ] := { "marginwidth"      , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_MAXLENGTH        ] := { "maxlength"        , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_MEDIA            ] := { "media"            , HTML_ATTR_TYPE_MEDIA     }
   saHtmlAttr[ HTML_ATTR_METHOD           ] := { "method"           , HTML_ATTR_TYPE_FSUBMIT   }
   saHtmlAttr[ HTML_ATTR_METHODS          ] := { "methods"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_MULTIPLE         ] := { "multiple"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_N                ] := { "n"                , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_NAME             ] := { "name"             , HTML_ATTR_TYPE_NAME      }
   saHtmlAttr[ HTML_ATTR_NOHREF           ] := { "nohref"           , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_NORESIZE         ] := { "noresize"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_NOSHADE          ] := { "noshade"          , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_NOWRAP           ] := { "nowrap"           , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_OBJECT           ] := { "object"           , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_ONAFTERUPDATE    ] := { "onafterupdate"    , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONBEFOREUNLOAD   ] := { "onbeforeunload"   , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONBEFOREUPDATE   ] := { "onbeforeupdate"   , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONBLUR           ] := { "onblur"           , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONCHANGE         ] := { "onchange"         , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONCLICK          ] := { "onclick"          , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONDATAAVAILABLE  ] := { "ondataavailable"  , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONDATASETCHANGED ] := { "ondatasetchanged" , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONDATASETCOMPLETE] := { "ondatasetcomplete", HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ] := { "ondblclick"       , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONERRORUPDATE    ] := { "onerrorupdate"    , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONFOCUS          ] := { "onfocus"          , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ] := { "onkeydown"        , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ] := { "onkeypress"       , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONKEYUP          ] := { "onkeyup"          , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONLOAD           ] := { "onload"           , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ] := { "onmousedown"      , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ] := { "onmousemove"      , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ] := { "onmouseout"       , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ] := { "onmouseover"      , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ] := { "onmouseup"        , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONRESET          ] := { "onreset"          , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONROWENTER       ] := { "onrowenter"       , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONROWEXIT        ] := { "onrowexit"        , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONSELECT         ] := { "onselect"         , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONSUBMIT         ] := { "onsubmit"         , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_ONUNLOAD         ] := { "onunload"         , HTML_ATTR_TYPE_SCRIPT    }
   saHtmlAttr[ HTML_ATTR_PROFILE          ] := { "profile"          , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_PROMPT           ] := { "prompt"           , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_RBSPAN           ] := { "rbspan"           , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_READONLY         ] := { "readonly"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_REL              ] := { "rel"              , HTML_ATTR_TYPE_LINKTYPES }
   saHtmlAttr[ HTML_ATTR_REV              ] := { "rev"              , HTML_ATTR_TYPE_LINKTYPES }
   saHtmlAttr[ HTML_ATTR_RIGHTMARGIN      ] := { "rightmargin"      , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_ROWS             ] := { "rows"             , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_ROWSPAN          ] := { "rowspan"          , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_RULES            ] := { "rules"            , HTML_ATTR_TYPE_TRULES    }
   saHtmlAttr[ HTML_ATTR_SCHEME           ] := { "scheme"           , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_SCOPE            ] := { "scope"            , HTML_ATTR_TYPE_SCOPE     }
   saHtmlAttr[ HTML_ATTR_SCROLLING        ] := { "scrolling"        , HTML_ATTR_TYPE_SCROLL    }
   saHtmlAttr[ HTML_ATTR_SDAFORM          ] := { "sdaform"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_SDAPREF          ] := { "sdapref"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_SDASUFF          ] := { "sdasuff"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_SELECTED         ] := { "selected"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_SHAPE            ] := { "shape"            , HTML_ATTR_TYPE_SHAPE     }
   saHtmlAttr[ HTML_ATTR_SHOWGRID         ] := { "showgrid"         , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_SHOWGRIDX        ] := { "showgridx"        , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_SHOWGRIDY        ] := { "showgridy"        , HTML_ATTR_TYPE_BOOL      }
   saHtmlAttr[ HTML_ATTR_SIZE             ] := { "size"             , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_SPAN             ] := { "span"             , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_SRC              ] := { "src"              , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_STANDBY          ] := { "standby"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_START            ] := { "start"            , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_STYLE            ] := { "style"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_SUMMARY          ] := { "summary"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_TABINDEX         ] := { "tabindex"         , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_TARGET           ] := { "target"           , HTML_ATTR_TYPE_TARGET    }
   saHtmlAttr[ HTML_ATTR_TEXT             ] := { "text"             , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_TITLE            ] := { "title"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_TOPMARGIN        ] := { "topmargin"        , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_TYPE             ] := { "type"             , HTML_ATTR_TYPE_TYPE      }
   saHtmlAttr[ HTML_ATTR_UNKNOWN          ] := { "unknown!"         , HTML_ATTR_TYPE_UNKNOWN   }
   saHtmlAttr[ HTML_ATTR_URN              ] := { "urn"              , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_USEMAP           ] := { "usemap"           , HTML_ATTR_TYPE_URL       }
   saHtmlAttr[ HTML_ATTR_VALIGN           ] := { "valign"           , HTML_ATTR_TYPE_VALIGN    }
   saHtmlAttr[ HTML_ATTR_VALUE            ] := { "value"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_VALUETYPE        ] := { "valuetype"        , HTML_ATTR_TYPE_VTYPE     }
   saHtmlAttr[ HTML_ATTR_VERSION          ] := { "version"          , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_VLINK            ] := { "vlink"            , HTML_ATTR_TYPE_COLOR     }
   saHtmlAttr[ HTML_ATTR_VSPACE           ] := { "vspace"           , HTML_ATTR_TYPE_NUMBER    }
   saHtmlAttr[ HTML_ATTR_WIDTH            ] := { "width"            , HTML_ATTR_TYPE_LENGTH    }
   saHtmlAttr[ HTML_ATTR_WRAP             ] := { "wrap"             , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_XMLNS            ] := { "xmlns"            , HTML_ATTR_TYPE_PCDATA    }
   saHtmlAttr[ HTML_ATTR_XML_LANG         ] := { "xml:lang"         , HTML_ATTR_TYPE_LANG      }
   saHtmlAttr[ HTML_ATTR_XML_SPACE        ] := { "xml:space"        , HTML_ATTR_TYPE_PCDATA    }

   RETURN

STATIC FUNCTION THtmlAttr_A()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_CHARSET          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COORDS           ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HREF             ], ;
      saHtmlAttr[ HTML_ATTR_HREFLANG         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_METHODS          ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_REL              ], ;
      saHtmlAttr[ HTML_ATTR_REV              ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SHAPE            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TARGET           ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_URN              ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_ABBR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_ACRONYM()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_ADDRESS()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_APPLET()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_ALT              ], ;
      saHtmlAttr[ HTML_ATTR_ARCHIVE          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_CODE             ], ;
      saHtmlAttr[ HTML_ATTR_CODEBASE         ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_HSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_OBJECT           ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_AREA()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_ALT              ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COORDS           ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HREF             ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NOHREF           ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SHAPE            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TARGET           ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_B()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BASE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_HREF             ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_TARGET           ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BASEFONT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_COLOR            ], ;
      saHtmlAttr[ HTML_ATTR_FACE             ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_SIZE             ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BDO()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BIG()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BLOCKQUOTE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CITE             ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BODY()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALINK            ], ;
      saHtmlAttr[ HTML_ATTR_BACKGROUND       ], ;
      saHtmlAttr[ HTML_ATTR_BGCOLOR          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_LINK             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONLOAD           ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ONUNLOAD         ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TEXT             ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VLINK            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_CLEAR            ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_BUTTON()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_VALUE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_CAPTION()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_CENTER()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_CITE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_CODE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_COL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SPAN             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_COLGROUP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SPAN             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DD()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DEL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CITE             ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DATETIME         ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DFN()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DIR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COMPACT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DIV()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COMPACT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_DT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_EM()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_FIELDSET()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_FONT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COLOR            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_FACE             ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_SIZE             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_FORM()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCEPT           ], ;
      saHtmlAttr[ HTML_ATTR_ACCEPT_CHARSET   ], ;
      saHtmlAttr[ HTML_ATTR_ACTION           ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ENCTYPE          ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_METHOD           ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ONRESET          ], ;
      saHtmlAttr[ HTML_ATTR_ONSUBMIT         ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SDASUFF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TARGET           ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_FRAME()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_FRAMEBORDER      ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LONGDESC         ], ;
      saHtmlAttr[ HTML_ATTR_MARGINHEIGHT     ], ;
      saHtmlAttr[ HTML_ATTR_MARGINWIDTH      ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_NORESIZE         ], ;
      saHtmlAttr[ HTML_ATTR_SCROLLING        ], ;
      saHtmlAttr[ HTML_ATTR_SRC              ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_FRAMESET()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COLS             ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONLOAD           ], ;
      saHtmlAttr[ HTML_ATTR_ONUNLOAD         ], ;
      saHtmlAttr[ HTML_ATTR_ROWS             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H1()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H2()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H3()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H4()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H5()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_H6()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_HEAD()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_PROFILE          ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_HR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NOSHADE          ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SIZE             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_HTML()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_VERSION          ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_I()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_IFRAME()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_FRAMEBORDER      ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LONGDESC         ], ;
      saHtmlAttr[ HTML_ATTR_MARGINHEIGHT     ], ;
      saHtmlAttr[ HTML_ATTR_MARGINWIDTH      ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_SCROLLING        ], ;
      saHtmlAttr[ HTML_ATTR_SRC              ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_IMG()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_ALT              ], ;
      saHtmlAttr[ HTML_ATTR_BORDER           ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_HSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ISMAP            ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_LONGDESC         ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SRC              ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_USEMAP           ], ;
      saHtmlAttr[ HTML_ATTR_VSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_INPUT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCEPT           ], ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_ALT              ], ;
      saHtmlAttr[ HTML_ATTR_CHECKED          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ISMAP            ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_MAXLENGTH        ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCHANGE         ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ONSELECT         ], ;
      saHtmlAttr[ HTML_ATTR_READONLY         ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SIZE             ], ;
      saHtmlAttr[ HTML_ATTR_SRC              ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_USEMAP           ], ;
      saHtmlAttr[ HTML_ATTR_VALUE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_INS()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CITE             ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DATETIME         ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_ISINDEX()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_PROMPT           ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_KBD()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_LABEL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_FOR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_LEGEND()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_LI()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_VALUE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_LINK()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CHARSET          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HREF             ], ;
      saHtmlAttr[ HTML_ATTR_HREFLANG         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_MEDIA            ], ;
      saHtmlAttr[ HTML_ATTR_METHODS          ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_REL              ], ;
      saHtmlAttr[ HTML_ATTR_REV              ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TARGET           ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_URN              ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_LISTING()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_MAP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_MENU()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COMPACT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_META()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CONTENT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HTTP_EQUIV       ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_SCHEME           ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_NEXTID()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_N                ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_NOFRAMES()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_NOSCRIPT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_OBJECT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_ARCHIVE          ], ;
      saHtmlAttr[ HTML_ATTR_BORDER           ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_CLASSID          ], ;
      saHtmlAttr[ HTML_ATTR_CODEBASE         ], ;
      saHtmlAttr[ HTML_ATTR_CODETYPE         ], ;
      saHtmlAttr[ HTML_ATTR_DATA             ], ;
      saHtmlAttr[ HTML_ATTR_DECLARE          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_HSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STANDBY          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_USEMAP           ], ;
      saHtmlAttr[ HTML_ATTR_VSPACE           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_OL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COMPACT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_START            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_OPTGROUP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LABEL            ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_OPTION()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LABEL            ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SELECTED         ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALUE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_P()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_PARAM()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_VALUE            ], ;
      saHtmlAttr[ HTML_ATTR_VALUETYPE        ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_PLAINTEXT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_PRE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XML_SPACE        ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_Q()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CITE             ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RB()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RBC()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_RBSPAN           ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RTC()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_RUBY()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_S()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SAMP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SCRIPT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CHARSET          ], ;
      saHtmlAttr[ HTML_ATTR_DEFER            ], ;
      saHtmlAttr[ HTML_ATTR_EVENT            ], ;
      saHtmlAttr[ HTML_ATTR_FOR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANGUAGE         ], ;
      saHtmlAttr[ HTML_ATTR_SRC              ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_XML_SPACE        ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SELECT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_MULTIPLE         ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCHANGE         ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_SIZE             ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SMALL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SPAN()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_STRIKE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_STRONG()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_STYLE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_MEDIA            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XML_SPACE        ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SUB()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_SUP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TABLE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_BGCOLOR          ], ;
      saHtmlAttr[ HTML_ATTR_BORDER           ], ;
      saHtmlAttr[ HTML_ATTR_CELLPADDING      ], ;
      saHtmlAttr[ HTML_ATTR_CELLSPACING      ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DATAPAGESIZE     ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_FRAME            ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_RULES            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_SUMMARY          ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TBODY()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TD()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ABBR             ], ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_AXIS             ], ;
      saHtmlAttr[ HTML_ATTR_BGCOLOR          ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COLSPAN          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HEADERS          ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NOWRAP           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ROWSPAN          ], ;
      saHtmlAttr[ HTML_ATTR_SCOPE            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TEXTAREA()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ACCESSKEY        ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COLS             ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_DISABLED         ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NAME             ], ;
      saHtmlAttr[ HTML_ATTR_ONBLUR           ], ;
      saHtmlAttr[ HTML_ATTR_ONCHANGE         ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONFOCUS          ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ONSELECT         ], ;
      saHtmlAttr[ HTML_ATTR_READONLY         ], ;
      saHtmlAttr[ HTML_ATTR_ROWS             ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TABINDEX         ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TFOOT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TH()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ABBR             ], ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_AXIS             ], ;
      saHtmlAttr[ HTML_ATTR_BGCOLOR          ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COLSPAN          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_HEADERS          ], ;
      saHtmlAttr[ HTML_ATTR_HEIGHT           ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_NOWRAP           ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_ROWSPAN          ], ;
      saHtmlAttr[ HTML_ATTR_SCOPE            ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_WIDTH            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_THEAD()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TITLE()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_ALIGN            ], ;
      saHtmlAttr[ HTML_ATTR_BGCOLOR          ], ;
      saHtmlAttr[ HTML_ATTR_CHAR             ], ;
      saHtmlAttr[ HTML_ATTR_CHAROFF          ], ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_VALIGN           ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_TT()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_U()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_UL()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_COMPACT          ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_TYPE             ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_VAR()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_CLASS            ], ;
      saHtmlAttr[ HTML_ATTR_DIR              ], ;
      saHtmlAttr[ HTML_ATTR_ID               ], ;
      saHtmlAttr[ HTML_ATTR_LANG             ], ;
      saHtmlAttr[ HTML_ATTR_ONCLICK          ], ;
      saHtmlAttr[ HTML_ATTR_ONDBLCLICK       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYDOWN        ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYPRESS       ], ;
      saHtmlAttr[ HTML_ATTR_ONKEYUP          ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEDOWN      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEMOVE      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOUT       ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEOVER      ], ;
      saHtmlAttr[ HTML_ATTR_ONMOUSEUP        ], ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_STYLE            ], ;
      saHtmlAttr[ HTML_ATTR_TITLE            ], ;
      saHtmlAttr[ HTML_ATTR_XML_LANG         ], ;
      saHtmlAttr[ HTML_ATTR_XMLNS            ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

STATIC FUNCTION THtmlAttr_XMP()

   RETURN { ;
      saHtmlAttr[ HTML_ATTR_SDAFORM          ], ;
      saHtmlAttr[ HTML_ATTR_SDAPREF          ], ;
      saHtmlAttr[ HTML_ATTR_UNKNOWN          ]  ;
      }

// Converts an HTML formatted text string to the ANSI character set

FUNCTION HtmlToAnsi( cHtmlText )

   LOCAL aEntity

   IF saHtmlAnsiEntities == NIL
      _Init_Html_AnsiCharacterEntities()
   ENDIF

   FOR EACH aEntity IN saHtmlAnsiEntities
      IF aEntity[2] $ cHtmlText
         cHtmlText := StrTran( cHtmlText, aEntity[2], aEntity[1] )
      ENDIF
   NEXT
   IF "&nbsp;" $ cHtmlText
      cHtmlText := StrTran( cHtmlText, "&nbsp;", " " )
   ENDIF

   RETURN cHtmlText

// Converts an HTML formatted text string to the OEM character set

FUNCTION HtmlToOem( cHtmlText )

   RETURN hb_ANSIToOEM( HtmlToAnsi( cHtmlText ) )

// Inserts HTML character entities into an ANSI text string

FUNCTION AnsiToHtml( cAnsiText )

   LOCAL cHtmlText := ""
   LOCAL parser    := P_PARSER( cAnsiText )
   LOCAL nStart    := 1
   LOCAL aEntity, cEntity, cText, cChr, nEnd

   IF saHtmlAnsiEntities == NIL
      _Init_Html_AnsiCharacterEntities()
   ENDIF

// Convert to Html but ignore all html character entities
   DO WHILE P_SEEK( parser, "&" ) > 0
      nEnd  := parser:p_pos
      cText := SubStr( parser:p_str, nStart, nEnd - nStart )

      DO WHILE .NOT. ( ( cChr := P_NEXT(parser ) ) $ "; " ) .AND. Asc( cChr ) > 0 .AND. .NOT. parser:p_pos == 0; ENDDO

      IF Asc( cChr ) == 0
         LOOP
      ENDIF

      SWITCH cChr
      CASE ";"
         // HTML character entity found
         nStart  := nEnd
         nEnd    := parser:p_pos + 1
         cEntity := SubStr( parser:p_str, nStart, nEnd - nStart )
         parser:p_end := parser:p_pos
         parser:p_pos ++
         EXIT
      CASE " "
         // "&" character found
         cHtmlText += cText
         nStart    := nEnd
         nEnd      := parser:p_pos + 1
         cText     := SubStr( parser:p_str, nStart, nEnd - nStart )
         nStart    := nEnd
         cHtmlText += "&amp;" + SubStr( cText, 2 )
         LOOP
      END

      nStart := parser:p_pos
      FOR EACH aEntity IN saHtmlAnsiEntities
         IF aEntity[1] $ cText
            cText := StrTran( cText, aEntity[1], aEntity[2] )
         ENDIF
      NEXT

      cHtmlText += cText + cEntity
   ENDDO

   cText := SubStr( parser:p_str, nStart )
   FOR EACH aEntity IN saHtmlAnsiEntities
      IF aEntity[1] $ cText
         cText := StrTran( cText, aEntity[1], aEntity[2] )
      ENDIF
   NEXT
   cHtmlText += cText

   RETURN cHtmlText

// Inserts HTML character entities into an OEM text string

FUNCTION OemToHtml( cOemText )

   RETURN AnsiToHtml( hb_OEMToANSI( cOemText ) )


/*
 * This function returs the HTML character entities that are exchangeable between ANSI and OEM character sets
 *
 * NOTE: The array below is coded with an ANSI editor and does not *DISPLAY* properly with an OEM editor
 *
 * ***  DO NOT CHANGE THE CODE BELOW WITH AN OEM EDITOR ***
 */

STATIC PROCEDURE _Init_Html_AnsiCharacterEntities

   saHtmlAnsiEntities := ;
      { ;
      { "&", "&amp;"    }, ;      //  ampersand
      { "<", "&lt;"     }, ;      //  less-than sign
      { ">", "&gt;"     }, ;      //  greater-than sign
      { "�", "&cent;"   }, ;      //  cent sign
      { "�", "&pound;"  }, ;      //  pound sign
      { "�", "&yen;"    }, ;      //  yen sign
      { "�", "&brvbar;" }, ;      //  broken bar
      { "�", "&sect;"   }, ;      //  section sign
      { "�", "&copy;"   }, ;      //  copyright sign
      { "�", "&reg;"    }, ;      //  registered sign
      { "�", "&deg;"    }, ;      //  degree sign
      { "�", "&iquest;" }, ;      //  inverted question mark
      { "�", "&Agrave;" }, ;      //  Latin capital letter a with grave
      { "�", "&Aacute;" }, ;      //  Latin capital letter a with acute
      { "�", "&Acirc;"  }, ;      //  Latin capital letter a with circumflex
      { "�", "&Atilde;" }, ;      //  Latin capital letter a with tilde
      { "�", "&Auml;"   }, ;      //  Latin capital letter a with diaeresis
      { "�", "&Aring;"  }, ;      //  Latin capital letter a with ring above
      { "�", "&AElig;"  }, ;      //  Latin capital letter ae
      { "�", "&Ccedil;" }, ;      //  Latin capital letter c with cedilla
      { "�", "&Egrave;" }, ;      //  Latin capital letter e with grave
      { "�", "&Eacute;" }, ;      //  Latin capital letter e with acute
      { "�", "&Ecirc;"  }, ;      //  Latin capital letter e with circumflex
      { "�", "&Euml;"   }, ;      //  Latin capital letter e with diaeresis
      { "�", "&Igrave;" }, ;      //  Latin capital letter i with grave
      { "�", "&Iacute;" }, ;      //  Latin capital letter i with acute
      { "�", "&Icirc;"  }, ;      //  Latin capital letter i with circumflex
      { "�", "&Iuml;"   }, ;      //  Latin capital letter i with diaeresis
      { "�", "&ETH;"    }, ;      //  Latin capital letter eth
      { "�", "&Ntilde;" }, ;      //  Latin capital letter n with tilde
      { "�", "&Ograve;" }, ;      //  Latin capital letter o with grave
      { "�", "&Oacute;" }, ;      //  Latin capital letter o with acute
      { "�", "&Ocirc;"  }, ;      //  Latin capital letter o with circumflex
      { "�", "&Otilde;" }, ;      //  Latin capital letter o with tilde
      { "�", "&Ouml;"   }, ;      //  Latin capital letter o with diaeresis
      { "�", "&Oslash;" }, ;      //  Latin capital letter o with stroke
      { "�", "&Ugrave;" }, ;      //  Latin capital letter u with grave
      { "�", "&Uacute;" }, ;      //  Latin capital letter u with acute
      { "�", "&Ucirc;"  }, ;      //  Latin capital letter u with circumflex
      { "�", "&Uuml;"   }, ;      //  Latin capital letter u with diaeresis
      { "�", "&Yacute;" }, ;      //  Latin capital letter y with acute
      { "�", "&THORN;"  }, ;      //  Latin capital letter thorn
      { "�", "&szlig;"  }, ;      //  Latin small letter sharp s (German Eszett)
      { "�", "&agrave;" }, ;      //  Latin small letter a with grave
      { "�", "&aacute;" }, ;      //  Latin small letter a with acute
      { "�", "&acirc;"  }, ;      //  Latin small letter a with circumflex
      { "�", "&atilde;" }, ;      //  Latin small letter a with tilde
      { "�", "&auml;"   }, ;      //  Latin small letter a with diaeresis
      { "�", "&aring;"  }, ;      //  Latin small letter a with ring above
      { "�", "&aelig;"  }, ;      //  Latin lowercase ligature ae
      { "�", "&ccedil;" }, ;      //  Latin small letter c with cedilla
      { "�", "&egrave;" }, ;      //  Latin small letter e with grave
      { "�", "&eacute;" }, ;      //  Latin small letter e with acute
      { "�", "&ecirc;"  }, ;      //  Latin small letter e with circumflex
      { "�", "&euml;"   }, ;      //  Latin small letter e with diaeresis
      { "�", "&igrave;" }, ;      //  Latin small letter i with grave
      { "�", "&iacute;" }, ;      //  Latin small letter i with acute
      { "�", "&icirc;"  }, ;      //  Latin small letter i with circumflex
      { "�", "&iuml;"   }, ;      //  Latin small letter i with diaeresis
      { "�", "&eth;"    }, ;      //  Latin small letter eth
      { "�", "&ntilde;" }, ;      //  Latin small letter n with tilde
      { "�", "&ograve;" }, ;      //  Latin small letter o with grave
      { "�", "&oacute;" }, ;      //  Latin small letter o with acute
      { "�", "&ocirc;"  }, ;      //  Latin small letter o with circumflex
      { "�", "&otilde;" }, ;      //  Latin small letter o with tilde
      { "�", "&ouml;"   }, ;      //  Latin small letter o with diaeresis
      { "�", "&oslash;" }, ;      //  Latin small letter o with stroke
      { "�", "&ugrave;" }, ;      //  Latin small letter u with grave
      { "�", "&uacute;" }, ;      //  Latin small letter u with acute
      { "�", "&ucirc;"  }, ;      //  Latin small letter u with circumflex
      { "�", "&uuml;"   }, ;      //  Latin small letter u with diaeresis
      { "�", "&yacute;" }, ;      //  Latin small letter y with acute
      { "�", "&thorn;"  }, ;      //  Latin small letter thorn
      { "�", "&yuml;"   }, ;      //  Latin small letter y with diaeresis
      { "^", "&circ;"   }, ;      //  modifier letter circumflex accent
      { "~", "&tilde;"  }  ;      //  small tilde
      }


#pragma BEGINDUMP
#include <ctype.h>
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbfast.h"

HB_EXTERN_BEGIN
extern ULONG  hb_strAtI( const char * szSub, ULONG ulSubLen, const char * szText, ULONG ulLen ); /* returns an index to a sub-string within another string (case insensitive)*/
HB_EXTERN_END
// extern int      hb_strnicmp( const char * s1, const char * s2, ULONG ulLen ); /* compare two string without regards to case, limited by length */

// Case insensitive string comparison to optimize this expression:
// IF Lower( <cSubStr> ) == Lower( SubStr( <cString>, <nStart>, Len(<cSubStr>) ) )
//  <cString> must be provided as a pointer to the character string containing a substring
//  <nStart> is the numeric position to start comparison in <cString>
//  <cSubStr> is the character string to compare with characters in <cString>, beginning at <nStart>

HB_FUNC( PSTRCOMPI )
{
   PHB_ITEM pPointer = hb_param( 1, HB_IT_POINTER );
   PHB_ITEM pStart   = hb_param( 2, HB_IT_NUMERIC );
   PHB_ITEM pSubstr  = hb_param( 3, HB_IT_STRING  );

   if( (pPointer && pStart && pSubstr) )
   {
      char * pcBase = (char*) hb_itemGetPtr( pPointer ) ;
      char * pcSub  = pSubstr->item.asString.value ;
      HB_SIZE uSublen = pSubstr->item.asString.length ;
      ULONG uStart  = hb_itemGetNL( pStart ) ;
      int iCompare  = hb_strnicmp( pcBase+uStart-1 , pcSub, uSublen ) ;

      hb_retl( (LONG) iCompare==0 ) ;
   }
   else
   {
      hb_errRT_BASE_SubstR( EG_ARG, 1099, NULL, "PStrCompI", 3, hb_paramError( 1 ), hb_paramError( 2 ), hb_paramError( 3 ) ) ;
   }
}

// Case insensitive At() function
    HB_FUNC( ATI )
    {
       PHB_ITEM pSub = hb_param( 1, HB_IT_STRING );
       PHB_ITEM pText = hb_param( 2, HB_IT_STRING );
       PHB_ITEM pStart = hb_param( 3, HB_IT_NUMERIC );
       PHB_ITEM pEnd = hb_param( 4, HB_IT_NUMERIC );

       if( pText && pSub )
       {
          LONG lStart = pStart ? hb_itemGetNL( pStart ) : 1;
          LONG lEnd = pEnd ? hb_itemGetNL( pEnd ) : ( LONG ) pText->item.asString.length;
          ULONG ulPos;

          if( lStart < 0 )
          {
             lStart += ( LONG ) pText->item.asString.length;

             if( lStart < 0 )
             {
                lStart = 0;
             }
          }
          else if( lStart )
          {
             lStart--;
          }

          if( lEnd < 0 )
          {
             lEnd += ( LONG ) pText->item.asString.length + 1;
          }

          if( lEnd > ( LONG ) pText->item.asString.length )
          {
             lEnd = ( LONG ) pText->item.asString.length;
          }

          // Stop searching if starting past beyond end.
          if( lStart >= lEnd )
          {
             //TraceLog( NULL, "Start: %i End: %i\n", lStart, lEnd );
             hb_retnl( 0 );
             return;
          }

          //TraceLog( NULL, "Search >%s< for >%s< from %i to %i\n", pText->item.asString.value, pSub->item.asString.value, lStart, lEnd );

          ulPos = hb_strAtI( pSub->item.asString.value, ( ULONG ) pSub->item.asString.length, pText->item.asString.value + lStart, lEnd - lStart );

          hb_retnl( ulPos ? ulPos + lStart : 0 );
       }
       else
       {
          hb_errRT_BASE_SubstR( EG_ARG, 1108, NULL, "AT", 2, hb_paramError( 1 ), hb_paramError( 2 ) );
       }
    }

// Case insensitive hb_strAt() function
ULONG hb_strAtI( const char * szSub, ULONG ulSubLen, const char * szText, ULONG ulLen )
{
   HB_TRACE(HB_TR_DEBUG, ("hb_strAt(%s, %lu, %s, %lu)", szSub, ulSubLen, szText, ulLen));

   if( ulSubLen > 0 && ulLen >= ulSubLen )
   {
      ULONG ulPos = 0;
      ULONG ulSubPos = 0;

      while( ulPos < ulLen && ulSubPos < ulSubLen )
      {
         if( tolower( (BYTE) szText[ ulPos ] ) == tolower( (BYTE) szSub[ ulSubPos ] ) )
         {
            ulSubPos++;
            ulPos++;
         }
         else if( ulSubPos )
         {
            /* Go back to the first character after the first match,
               or else tests like "22345" $ "012223456789" will fail. */
            ulPos -= ( ulSubPos - 1 );
            ulSubPos = 0;
         }
         else
            ulPos++;
      }

      return ( ulSubPos < ulSubLen ) ? 0 : ( ulPos - ulSubLen + 1 );
   }
   else
      return 0;
}
#pragma ENDDUMP
