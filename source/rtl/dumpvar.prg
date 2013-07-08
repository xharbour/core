/*
 * $Id: dumpvar.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * Harbour Project source code:
 * Dumpvar function to display var contents
 *
 * Copyright 2003 Francesco Saverio Giudice <info@fsgiudice.com>
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

#include "common.ch"
#include "hbclass.ch"

#define  CRLF HB_OsNewLine()

/*
 * (C) 2003 - Francesco Saverio Giudice
 *
 * Send to hb_OutDebug() more parameters
 *
*/

PROCEDURE __OutDebug( ... )

   LOCAL xVal

   FOR EACH xVal IN hb_AParams()
      hb_OutDebug( hb_DumpVar( xVal ) )
   NEXT

   RETURN

/*
 * (C) 2003 - Francesco Saverio Giudice
 *
 * return a string containing a dump of a variable
 *
 *
 * 24/09/2006 - FSG
 * - Added recursion limit
 * - Added front function with limited parameters and removed support for TAssociative Array
*/

FUNCTION HB_DumpVar( xVar, lRecursive, nMaxRecursionLevel )

   LOCAL nRecursionLevel := 1
   LOCAL nIndent         := 0

//TraceLog( "HB_DumpVariable: xVar, lAssocAsObj, lRecursive", xVar, lAssocAsObj, lRecursive )

   DEFAULT nMaxRecursionLevel TO 0

   RETURN __HB_DumpVar( xVar, , lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )

STATIC FUNCTION __HB_DumpVar( xVar, lAssocAsObj, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )

   LOCAL cType := ValType( xVar )
   LOCAL cString := "", cKey

   DEFAULT lAssocAsObj        TO FALSE
   DEFAULT lRecursive         TO FALSE

//TraceLog( "Recursion: xVar, nRecursionLevel, nMaxRecursionLevel", xVar, nRecursionLevel, nMaxRecursionLevel )

// return if there is limit in recursion
   IF nMaxRecursionLevel > 0 .AND. ;
         nRecursionLevel > nMaxRecursionLevel
      RETURN AsString( xVar )
   ENDIF

   DO CASE
   CASE cType == "O"

      IF !lAssocAsObj .AND. xVar:ClassName == "TASSOCIATIVEARRAY"
         cString += Space( nIndent ) + "Type='Associative' -> " + CRLF
         // Keys extraction.
         IF Len( xVar:Keys ) > 0
            cString += Space( nIndent ) + "{" + CRLF
            FOR EACH cKey IN xVar:Keys
               cString += Space( nIndent ) + " '" + cKey + "' => " + asString( xVar:SendKey( cKey ) ) + ", " + CRLF
               IF lRecursive .AND. ValType( xVar:SendKey( cKey ) ) $ "AOH"
                  cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
                  cString += __HB_DumpVar( xVar:SendKey( cKey ), , lRecursive, nIndent + 3, nRecursionLevel + 1, nMaxRecursionLevel )
                  cString := SubStr( cString, 1, Len( cString ) - 2 ) + ", " + CRLF
               ENDIF
            NEXT
            cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
            cString += Space( nIndent ) + "}" + CRLF
         ENDIF
      ELSE
         cString += Space( nIndent ) + "<" + xVar:ClassName + " Object>" + CRLF
         cString += Space( nIndent ) + " | " + CRLF
         cString += Space( nIndent ) + " +- PRIVATE/HIDDEN:" + CRLF
         cString += DShowProperties( xVar, HB_OO_CLSTP_HIDDEN, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
         cString += Space( nIndent ) + " +- PROTECTED:" + CRLF
         cString += DShowProperties( xVar, HB_OO_CLSTP_PROTECTED, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
         cString += Space( nIndent ) + " +- EXPORTED/VISIBLE/PUBLIC:" + CRLF
         cString += DShowProperties( xVar, HB_OO_CLSTP_EXPORTED, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
         cString += Space( nIndent ) + " +- PUBLISHED:" + CRLF
         cString += DShowProperties( xVar, HB_OO_CLSTP_PUBLISHED, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
         cString += Space( nIndent ) + " +----------->" + CRLF
      ENDIF

   CASE cType == "A"
      IF nRecursionLevel == 1
         cString += Space( nIndent ) + "Type='A' -> { Array of " + LTrim( Str( Len( xVar ) ) ) + " Items }" + CRLF
      ENDIF
      IF nMaxRecursionLevel > 0 .AND. nRecursionLevel > nMaxRecursionLevel
         cString += AsString( xVar )
      ELSE
         cString += DShowArray( xVar, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
      ENDIF

   CASE cType == "H"
      IF nRecursionLevel == 1
         cString += Space( nIndent ) + "Type='H' -> { Hash of " + LTrim( Str( Len( xVar ) ) ) + " Items }" + CRLF
      ENDIF
      IF nMaxRecursionLevel > 0 .AND. nRecursionLevel > nMaxRecursionLevel
         cString += AsString( xVar )
      ELSE
         cString += DShowHash( xVar, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )
      ENDIF

   OTHERWISE
      cString +=  Space( nIndent ) + AsString( xVar ) + CRLF
   ENDCASE

   RETURN cString

STATIC FUNCTION DShowProperties( oVar, nScope, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )

   LOCAL xProp, aProps
   LOCAL aMethods, aMth
   LOCAL lOldScope
   LOCAL cString := ""

   DEFAULT nIndent TO 0

   IF ValType( oVar ) == "O"
      lOldScope := __SetClassScope( .F. )
      aMethods  := __objGetMsgFullList( oVar, .F. , HB_MSGLISTALL, nScope )
      aProps    := __objGetValueFullList( oVar, NIL, nScope )
      __SetClassScope( lOldScope )

      IF Len( aProps ) > 0
         cString += Space( nIndent ) + " |  +- >> Begin Data    ------" + CRLF
         FOR EACH xProp IN aProps
            cString += Space( nIndent ) + " |  +- " + PadR( xProp[ HB_OO_DATA_SYMBOL ], 20 ) + " [" + DecodeType( xProp[ HB_OO_DATA_TYPE ] ) +  "] [" + DecodeScope( xProp[ HB_OO_DATA_SCOPE ] ) + "] " + ValType( xProp[ HB_OO_DATA_VALUE ] ) + " => " + AsString( xProp[ HB_OO_DATA_VALUE ] ) + CRLF
            IF lRecursive .AND. ValType( xProp[ HB_OO_DATA_VALUE ] ) $ "AO"
               cString += __HB_DumpVar( xProp[ HB_OO_DATA_VALUE ], , lRecursive, nIndent + 3, nRecursionLevel + 1, nMaxRecursionLevel ) + CRLF
            ENDIF
         NEXT
         cString += Space( nIndent ) + " |  +- >> End   Data    ------" + CRLF
         cString += Space( nIndent ) + " |   " + CRLF
      ENDIF
      IF Len( aMethods ) > 0
         cString += Space( nIndent ) + " |  +- >> Begin Methods ------" + CRLF
         FOR EACH aMth IN aMethods
            cString += Space( nIndent ) + " |  +- " + PadR( aMth[HB_OO_DATA_SYMBOL], 20 ) + " [" + DecodeType( aMth[HB_OO_DATA_TYPE] ) + "]" + " [" + DecodeScope( aMth[HB_OO_DATA_SCOPE] ) +  "] " + CRLF
         NEXT
         cString += Space( nIndent ) + " |  +- >> End   Methods ------" + CRLF
         cString += Space( nIndent ) + " |     " + CRLF
      ENDIF
   ENDIF
   IF Empty( cString )
      cString := Space( nIndent ) + " | " + CRLF
   ENDIF

   RETURN cString

STATIC FUNCTION DShowArray( aVar, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )

   LOCAL xVal, nChar
   LOCAL cString := ""

   DEFAULT nIndent TO 0

//TraceLog( "DShowArray: aVar, lRecursive", aVar, lRecursive )

   IF ValType( aVar ) == "A"
      nChar := Len( LTrim( Str( Len( aVar ) ) ) )  // return number of chars to display that value
      // i.e. if Len( aVar ) == 99, then nChar := 2
      cString += Space( nIndent ) + "{" + CRLF
      FOR EACH xVal IN aVar
         cString += Space( nIndent ) + " [" + LTrim( StrZero( HB_EnumIndex(), nChar ) ) + "] => " + AsString( xVal ) + ", " + CRLF
         IF lRecursive .AND. ValType( xVal ) $ "AOH"
            cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
            cString += __HB_DumpVar( xVal, , lRecursive, nIndent + 3, nRecursionLevel + 1, nMaxRecursionLevel )
            cString := SubStr( cString, 1, Len( cString ) - 2 ) + ", " + CRLF
         ENDIF
      NEXT
      IF Len( aVar ) > 0
         cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
      ENDIF
      cString += Space( nIndent ) + "}" + CRLF
   ENDIF

   RETURN cString

STATIC FUNCTION DShowHash( hVar, lRecursive, nIndent, nRecursionLevel, nMaxRecursionLevel )

   LOCAL xVal, xKey, aKeys
   LOCAL cString := ""

   DEFAULT nIndent TO 0

//TraceLog( "DShowHash: hVar, ValType( hVar ), lRecursive", hVar, ValType( hVar ), ValToPrg( hVar ), lRecursive )

   IF ValType( hVar ) == "H"
      aKeys := HGetKeys( hVar )
      cString += Space( nIndent ) + "{" + CRLF
      FOR EACH xKey IN aKeys
         xVal := hVar[ xKey ]
         cString += Space( nIndent ) + " [" + LTrim( AsString( xKey ) ) + "] => " + AsString( xVal ) + ", " + CRLF
         IF lRecursive .AND. ValType( xVal ) $ "AOH"
            cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
            cString += __HB_DumpVar( xVal, , lRecursive, nIndent + 3, nRecursionLevel + 1, nMaxRecursionLevel )
            cString := SubStr( cString, 1, Len( cString ) - 2 ) + ", " + CRLF
         ENDIF
      NEXT
      IF Len( aKeys ) > 0
         cString := SubStr( cString, 1, Len( cString ) - 4 ) + CRLF
      ENDIF
      cString += Space( nIndent ) + "}" + CRLF
   ENDIF

   RETURN cString

STATIC FUNCTION DecodeScope( nScope AS NUMERIC )

   LOCAL cString := ""

   IF hb_bitAnd( nScope, HB_OO_CLSTP_EXPORTED  ) # 0  //   1
      cString += "Ex,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_PUBLISHED ) # 0  //   2
      cString += "Pu,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_PROTECTED ) # 0  //   4
      cString += "Pr,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_HIDDEN    ) # 0  //   8
      cString += "Hi,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_CTOR      ) # 0  //  16
      cString += "Ct,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_READONLY  ) # 0  //  32
      cString += "Ro,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_SHARED    ) # 0  //  64
      cString += "Sh,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_CLASS     ) # 0  // 128
      cString += "Cl,"
   ENDIF
   IF hb_bitAnd( nScope, HB_OO_CLSTP_SUPER     ) # 0  // 256
      cString += "Su,"
   ENDIF

   IF cString[-1] == ","
      cString := SubStr( cString, 1, Len( cString ) - 1 )
   ENDIF

   RETURN PadR( cString, 18 )

STATIC FUNCTION DecodeType( nType AS NUMERIC )

   LOCAL cString := ""

   DO CASE
   CASE nType == HB_OO_MSG_METHOD      // 0
      cString += "Method"
   CASE nType == HB_OO_MSG_DATA        // 1
      cString += "Data"
   CASE nType == HB_OO_MSG_CLASSDATA   // 2
      cString += "Clsdata"
   CASE nType == HB_OO_MSG_INLINE      // 3
      cString += "Inline"
   CASE nType == HB_OO_MSG_VIRTUAL     // 4
      cString += "Virtual"
   CASE nType == HB_OO_MSG_SUPER       // 5
      cString += "Super"
   CASE nType == HB_OO_MSG_ONERROR     // 6
      cString += "OnError"
   CASE nType == HB_OO_MSG_DESTRUCTOR  // 7
      cString += "Destructor"
   CASE nType == HB_OO_PROPERTY        // 8
      cString += "Property"
   CASE nType == HB_OO_MSG_PROPERTY    // 9
      cString += "MsgPrp"
   CASE nType == HB_OO_MSG_CLASSPROPERTY  // 10
      cString += "ClsPrp"
   ENDCASE

   RETURN PadR( cString, 7 )

STATIC FUNCTION asString( x )

   LOCAL v := ValType( x )

   DO CASE
   CASE v == "C"
      RETURN '"' + x + '"'
   OTHERWISE
      RETURN cStr( x )
   END CASE

   RETURN( x )
