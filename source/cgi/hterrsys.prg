/*
 * $Id: hterrsys.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * Harbour Project source code:
 *    HTML output conversion
 *
 * Copyright 2000 Manos Aspradakis <maspr@otenet.gr>
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * The exception is that if you link the Harbour Runtime Library (HRL)
 * and/or the Harbour Virtual Machine (HVM) with other files to produce
 * an executable, this does not by itself cause the resulting executable
 * to be covered by the GNU General Public License. Your use of that
 * executable is in no way restricted on account of linking the HRL
 * and/or HVM code into it.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */
/*
 * The following parts are Copyright of the individual authors.
 * www - http://www.harbour-project.org
 *
 * Copyright 2000 Luiz Rafael Culik <culik@sl.conex.net>
 *    Porting this library to Harbour
 *
 * See doc/license.txt for licensing terms.
 *
 */

#include "error.ch"
#include "cgi.ch"

#define DEF_ERR_HEADER "Date : "+DTOC(Date())+"<BR>"+"Time : " + Time() + "<BR>"

// put messages to STDERR
#command ? <list,...>   =>  ?? Chr(13) + Chr(10) ; ?? <list>
#command ?? <list,...>  =>  OutErr(<list>)

   REQUEST HARDCR
   REQUEST MEMOWRIT

   STATIC sbFixCorrupt
   STATIC scErrFooter  := " "


/***
* ErrorSys()
*
* Note:  automatically executes at startup
*/
/* TOFIX: Luiz please review it
ANNOUNCE ERRORSYS

PROC Errorsys()

   
   Errorblock( { | e | DefError( e ) } )
RETURN
*/
/***
* DefError()
*/

/*
STATIC FUNC DefError( e )
   
   LOCAL i
   LOCAL cMessage   := ""
   LOCAL cErrString
   LOCAL nH         := IF( HtmlPageHandle() == NIL, 0, HtmlPageHandle() )

// by default, division by zero yields zero
   IF ( e:genCode == EG_ZERODIV )
      RETURN ( 0 )
   END

   IF ( e:genCode == EG_CORRUPTION )
      IF ValType( sbFixCorrupt ) == "B"
         Eval( sbFixCorrupt, e )
         RETURN .F.
      ELSE
         RETURN .F.
      ENDIF
   ENDIF

// for network open error, set NETERR() and subsystem default
   IF ( e:genCode == EG_OPEN .AND. ( e:osCode == 32 .OR. e:osCode == 5 ) ;
         .AND. e:canDefault )

      NetErr( .T. )
      RETURN ( .F. )                    // NOTE

   END

// for lock error during APPEND BLANK, set NETERR() and subsystem default
   IF ( e:genCode == EG_APPENDLOCK .AND. e:canDefault )

      NetErr( .T. )
      RETURN ( .F. )                    // NOTE

   END

// build error message
   cMessage += ErrorMessage( e )

// display message and traceback
   IF ( !Empty( e:osCode ) )
      cMessage += " (DOS Error   : " + NTRIM( e:osCode ) + ")"
   END

// RESET System //

   cErrString := CRLF() + "</TD></TR></TABLE>" + CRLF()
   cErrString += '<TABLE bgcolor="white" border CellPadding=1 CellSpacing=1 COLS=2 WIDTH=80%>'

   cErrString += '<TR><TD bgcolor="black" align="CENTER">'
   cErrstring += '<FONT face = "verdana" size ="5" color="white">' + CRLF()
   cErrString += "<B>ERROR REPORT</B>"
   cErrString += "</TD></TR>"

   cErrString += '<TR><TD bgcolor="blue">'
   cErrstring += '<FONT face = "verdana" size ="2" color="white">' + CRLF()
   cErrString += DEF_ERR_HEADER
   cErrString += "</TD></TR>"

   cErrString += '<TR><TD bgcolor="red">'
   cErrstring += '<FONT face ="verdana" size ="2" color="white">' + CRLF()
   cErrString += '<EM>' + cMessage + '</EM>'

   cErrString += '</TD></TR><TR><TD bgcolor="cyan">' + CRLF()
   cErrstring += '<FONT face ="verdana" size ="2" color="black">' + CRLF()
   cErrString += "ERRORCODE...... :" + NTRIM( e:GenCode ) + "<BR>" + CRLF()
   cErrString += "SUBSYSTEM..... :" + e:SubSystem + "<BR>" + CRLF()
   cErrString += "DESCRIPTION...:" + e:Description + "<BR>" + CRLF()
   cErrString += "OPERATION......:" + e:Operation + "<BR>" + CRLF()
   cErrString += "FILENAME........ :" + e:FileName + "<BR>" + CRLF()
   cErrString += "TRIES............. :" + NTRIM( e:Tries ) + CRLF()

   cErrString += '</TD></TR>'
   cErrString += '<TR><TD bgcolor="red">'
   cErrstring += '<FONT face ="verdana" size ="2" color="white">' + CRLF()
   cErrstring += '<EM>'

   i := 2

   WHILE ( !Empty( ProcName( i ) ) )

      cErrString += "Called from " + Trim( ProcName( i ) ) + ;
         "(" + NTRIM( ProcLine( i ) ) + ") <BR>" + CRLF()

      i ++
   END

   cErrstring += '</EM>'
   cErrString += '</TD></TR>'
   cErrString += '<TR><TD bgcolor="black">'
   cErrstring += '<FONT face ="verdana" size ="2" color="white">' + CRLF()
   cErrstring += "Extra Notes..."

   cErrString += "</TD>" + CRLF() + "</TR>" + CRLF() + "</TABLE>" + CRLF()
   FWrite( nH, "<BR>" + cErrString + CRLF() )
   MemoWrit( "Error.Log", HardCR( cErrString ) + CRLF() + ;
      HardCR( MemoRead( "Error.Log" ) ) )

   FWrite( nH, "</TD>" + CRLF() + "</TR>" + CRLF() + "</TABLE>" + CRLF() )

   HtmlJsCmd( nH, 'alert("There was an error processing your request:\n' + ;
      'Look at the bottom of this page for\n' + ;
      'error description and parameters...");' )
   FWrite( nH, "</FONT>" + CRLF() + "</BODY></HTML>" + CRLF() )

   CLOSE ALL

   ErrorLevel( 1 )
   QUIT

   RETURN ( .F. )
*/

FUNCTION SetCorruptFunc( bFunc )
   
   IF ValType( bFunc ) == "B"
      sbFixCorrupt := bFunc
   ENDIF

   RETURN sbFixCorrupt

FUNCTION SetErrorFooter()

   RETURN ( scErrFooter )

/***
* ErrorMessage()
*/

/*
STATIC FUNC ErrorMessage( e )
   
   LOCAL cMessage := ""

// start error message
   cMessage += IF( e:severity > ES_WARNING, "Error ", "Warning " )

// add subsystem name if available
   IF ( ValType( e:subsystem ) == "C" )
      cMessage += e:subsystem()
   ELSE
      cMessage += "???"
   END

// add subsystem's error code if available
   IF ( ValType( e:subCode ) == "N" )
      cMessage += ( "/" + NTRIM( e:subCode ) )
   ELSE
      cMessage += "/???"
   END

// add error description if available
   IF ( ValType( e:description ) == "C" )
      cMessage += ( "<BR>  " + e:description )
   END

// add either filename or operation
   IF ( !Empty( e:filename ) )
      cMessage += ( ": " + e:filename )

   ELSEIF ( !Empty( e:operation ) )
      cMessage += ( ": " + e:operation )

   END
   cMessage += CRLF()

   RETURN ( cMessage )
*/
