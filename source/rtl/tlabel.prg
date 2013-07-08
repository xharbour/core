/*
 * $Id: tlabel.prg 9935 2013-03-21 08:28:29Z zsaulius $
 */

/*
 * Harbour Project source code:
 * HBLabelForm class and __LabelForm()
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

#include "hbclass.ch"
#include "error.ch"
#include "fileio.ch"
#include "inkey.ch"

#define F_OK            0       // No error
#define F_EMPTY         -3      // File is empty

#define _LF_SAMPLES     2       // "Do you want more samples?"
#define _LF_YN          12      // "Y/N"

#define LBL_REMARK      1       // Character, remark from label file
#define LBL_HEIGHT      2       // Numeric, label height
#define LBL_WIDTH       3       // Numeric, label width
#define LBL_LMARGIN     4       // Numeric, left margin
#define LBL_LINES       5       // Numeric, lines between labels
#define LBL_SPACES      6       // Numeric, spaces between labels
#define LBL_ACROSS      7       // Numeric, number of labels across
#define LBL_FIELDS      8       // Array of Field arrays

#define LBL_COUNT       8       // Numeric, number of label fields

// Field array definitions ( one array per field )
#define LF_EXP          1       // Block, field expression
#define LF_TEXT         2       // Character, text of field expression
#define LF_BLANK        3       // Logical, compress blank fields, .T.=Yes .F.=No

#define LF_COUNT        3       // Numeric, number of elements in field array

#define BUFFSIZE        1034    // Size of label file
#define FILEOFFSET      74      // Start of label content descriptions
#define FIELDSIZE       60
#define REMARKOFFSET    2
#define REMARKSIZE      60
#define HEIGHTOFFSET    62
#define HEIGHTSIZE      2
#define WIDTHOFFSET     64
#define WIDTHSIZE       2
#define LMARGINOFFSET   66
#define LMARGINSIZE     2
#define LINESOFFSET     68
#define LINESSIZE       2
#define SPACESOFFSET    70
#define SPACESSIZE      2
#define ACROSSOFFSET    72
#define ACROSSSIZE      2

CLASS HBLabelForm

   DATA aLabelData AS ARRAY init {}
   DATA aBandToPrint AS ARRAY
   DATA cBlank AS STRING init ""
   DATA lOneMoreBand AS LOGICAL init .T.
   DATA nCurrentCol AS NUMERIC // The current column in the band
   METHOD New( cLBLName, lPrinter, cAltFile, lNoConsole, bFor, ;
      bWhile, nNext, nRecord, lRest, lSample )
   METHOD ExecuteLabel()
   METHOD SampleLabels()
   METHOD LoadLabel( cLblFile )

ENDCLASS

METHOD New( cLBLName, lPrinter, cAltFile, lNoConsole, bFor, ;
      bWhile, nNext, nRecord, lRest, lSample ) CLASS HBLabelForm

   LOCAL lPrintOn := .F.               // PRINTER status
   LOCAL lConsoleOn                    // CONSOLE status
   LOCAL cExtraFile, lExtraState       // EXTRA file status
   LOCAL xBreakVal, lBroke := .F.
   LOCAL err
   LOCAL OldMargin
   LOCAL cExt

   ::aBandToPrint := {} // ARRAY(5)
   ::nCurrentCol := 1
// Resolve parameters
   IF cLBLName == NIL
      err := ErrorNew()
      err:severity := ES_ERROR
      err:genCode := EG_ARG
      err:subSystem := "FRMLBL"
      Eval( ErrorBlock(), err )

   ELSE
      /* NOTE: CA-Cl*pper does an RTrim() on the filename here,
               but in Harbour we're using _SET_TRIMFILENAME */
      IF Set( _SET_DEFEXTENSIONS )
         hb_FNameSplit( cLBLName, NIL, NIL, @cExt )
         IF Empty( cExt )
            cLBLName += ".lbl"
         ENDIF
      ENDIF

   ENDIF

   IF lPrinter == NIL
      lPrinter := .F.
   ENDIF

   IF lSample == NIL
      lSample := .F.
   ENDIF

   // Set output devices
   IF lPrinter             // To the printer
      lPrintOn  := SET( _SET_PRINTER, lPrinter )
   ENDIF

   lConsoleOn := SET( _SET_CONSOLE )
   SET( _SET_CONSOLE, ! ( lNoConsole .OR. !lConsoleOn ) )

   IF (!Empty(cAltFile))         // To file
      lExtraState := SET( _SET_EXTRA, .T. )
      cExtraFile  := SET( _SET_EXTRAFILE, cAltFile )
   ENDIF

   OldMargin := SET( _SET_MARGIN, 0)

   BEGIN SEQUENCE

      ::aLabelData := ::LoadLabel( cLBLName )  // Load the (.lbl) into an array

      // Add to the left margin if a SET MARGIN has been defined
      ::aLabelData[ LBL_LMARGIN ] := ::aLabelData[ LBL_LMARGIN ] + OldMargin

      // Size the ::aBandToPrint array to the number of fields
      //      nLen := LEN( ::aLabelData[ LBL_FIELDS ] )

      ASIZE( ::aBandToPrint, LEN( ::aLabelData[ LBL_FIELDS ]))
      AFILL( ::aBandToPrint, SPACE( ::aLabelData[ LBL_LMARGIN ] ) )

      // Create enough space for a blank record
      ::cBlank := SPACE( ::aLabelData[ LBL_WIDTH ] + ::aLabelData[ LBL_SPACES ] )

      // Handle sample labels
      IF lSample
         ::SampleLabels()
      ENDIF

      // Execute the actual label run based on matching records
      DBEval( { || ::ExecuteLabel() }, bFor, bWhile, nNext, nRecord, lRest )

      // Print the last band if there is one
      IF ::lOneMoreBand
         // Print the band
         AEVAL( ::aBandToPrint, { | BandLine | PrintIt( BandLine ) } )

      ENDIF


   RECOVER USING xBreakVal

      lBroke := .T.

   END SEQUENCE

   // Clean up and leave
   ::aLabelData   := {}                // Recover the space
   ::aBandToPrint := {}
   ::nCurrentCol  := 1
   ::cBlank       := ""
   ::lOneMoreBand :=.T.

   // clean up
   SET( _SET_PRINTER, lPrintOn ) // Set the printer back to prior state
   SET( _SET_CONSOLE, lConsoleOn )  // Set the console back to prior state

   IF (!Empty(cAltFile))            // Set extrafile back
      SET( _SET_EXTRAFILE, cExtraFile )
      SET( _SET_EXTRA, lExtraState )
   ENDIF

   IF lBroke
      BREAK xBreakVal               // continue breaking
   ENDIF

   SET( _SET_MARGIN, OldMargin)

   RETURN Self

METHOD ExecuteLabel() CLASS HBLabelForm
   LOCAL nField, nMoreLines, aBuffer := {}, cBuffer
   LOCAL v
   LOCAL aLabel
   LOCAL cBand

   // Load the current record into aBuffer
   FOR EACH aLabel IN ::aLabelData[ LBL_FIELDS ]

      if ( aLabel <> NIL )

         v := Eval( aLabel[ LF_EXP ] )

         cBuffer := PadR( v, ::aLabelData[ LBL_WIDTH ] )
         cBuffer := cBuffer + Space( ::aLabelData[ LBL_SPACES ] )

         if ( aLabel[ LF_BLANK ] )
            if ( !Empty( cBuffer ) )
               AADD( aBuffer, cBuffer )
            end
         else
            AADD( aBuffer, cBuffer )
         endif

      else

         AADD( aBuffer, NIL )

      end

   NEXT

   ASIZE( aBuffer, LEN( ::aLabelData[ LBL_FIELDS ] ) )

   // Add aBuffer to ::aBandToPrint
//   nField := 1
   FOR EACH cBand IN ::aBandToPrint
      IF aBuffer[ HB_EnumIndex() ] == NIL
         cBand += ::cBlank
      ELSE
         cBand += aBuffer[ HB_EnumIndex() ]
      ENDIF
//      nField++
   NEXT

   IF ::nCurrentCol == ::aLabelData[ LBL_ACROSS ]

      // trim
      FOR EACH aLabel IN ::aBandToPrint
         aLabel := Trim( aLabel )
      NEXT


      ::lOneMoreBand := .F.
      ::nCurrentCol  := 1

      // Print the band
      AEVAL( ::aBandToPrint, { | BandLine | PrintIt( BandLine ) } )

      nMoreLines := ::aLabelData[ LBL_HEIGHT ] - LEN( ::aBandToPrint )
      IF nMoreLines > 0
         FOR nField := 1 TO nMoreLines
            PrintIt()
         NEXT
      ENDIF
      IF ::aLabelData[ LBL_LINES ] > 0

         // Add the spaces between the label lines
         FOR nField := 1 TO ::aLabelData[ LBL_LINES ]
            PrintIt()
         NEXT

      ENDIF

      // Clear out the band
      AFILL( ::aBandToPrint, SPACE( ::aLabelData[ LBL_LMARGIN ] ) )
   ELSE
      ::lOneMoreBand := .T.
      ::nCurrentCol :=  ::nCurrentCol + 1
   ENDIF

   RETURN Self

METHOD SampleLabels() CLASS HBLabelForm
   LOCAL nGetKey, lMoreSamples := .T., nField
   LOCAL aBand := {}

   // Create the sample label row
   ASIZE( aBand, ::aLabelData[ LBL_HEIGHT ] )
   AFILL( aBand, SPACE( ::aLabelData[ LBL_LMARGIN ] ) +;
              REPLICATE( REPLICATE( "*", ;
              ::aLabelData[ LBL_WIDTH ] ) + ;
              SPACE( ::aLabelData[ LBL_SPACES ] ), ;
              ::aLabelData[ LBL_ACROSS ] ) )

   // Prints sample labels
   DO WHILE lMoreSamples

      // Print the samples
      AEVAL( aBand, { | BandLine | PrintIt( BandLine ) } )

      IF ::aLabelData[ LBL_LINES ] > 0
         // Add the spaces between the label lines
         FOR nField := 1 TO ::aLabelData[ LBL_LINES ]
            PrintIt()
         NEXT nField
      ENDIF

      // Prompt for more
      @ ROW(), 0 SAY NationMsg(_LF_SAMPLES)+" ("+Nationmsg(_LF_YN)+")"
      nGetKey := INKEY(0)
      @ ROW(), COL() SAY CHR(nGetKey)
      IF ROW() == MAXROW()
         SCROLL( 0, 0, MAXROW(), MAXCOL(), 1 )
         @ MAXROW(), 0 SAY ""
      ELSE
         @ ROW()+1, 0 SAY ""
      ENDIF
      IF IsNegative(CHR(nGetKey))   // Don't give sample labels
         lMoreSamples := .F.
      ENDIF
   ENDDO

   RETURN Self

METHOD LoadLabel( cLblFile ) CLASS HBLabelForm
   LOCAL i                                // Counters
   LOCAL cBuff      := SPACE(BUFFSIZE)    // File buffer
   LOCAL nHandle                          // File handle
   LOCAL nReadCount                       // Bytes read from file
   LOCAL nOffset    := FILEOFFSET         // Offset into file
   LOCAL nFileError                       // File error
   LOCAL cFieldText                       // Text expression container
   LOCAL err                              // error object

   LOCAL cDefPath          // contents of SET DEFAULT string
   LOCAL cPath

   // Create and initialize default label array
   LOCAL aLabel[ LBL_COUNT ]
   aLabel[ LBL_REMARK ]  := SPACE(60)      // Label remark
   aLabel[ LBL_HEIGHT ]  := 5              // Label height
   aLabel[ LBL_WIDTH ]   := 35             // Label width
   aLabel[ LBL_LMARGIN ] := 0              // Left margin
   aLabel[ LBL_LINES ]   := 1              // Lines between labels
   aLabel[ LBL_SPACES ]  := 0              // Spaces between labels
   aLabel[ LBL_ACROSS ]  := 1              // Number of labels across
   aLabel[ LBL_FIELDS ]  := {}             // Array of label fields

   // Open the label file
   nHandle := FOPEN( cLblFile )

   IF ( ! EMPTY( nFileError := FERROR() ) ) .AND. !( "\" IN cLblFile .OR. ":" IN cLblFile )

      // Search through default path; attempt to open label file
      cDefPath := SET( _SET_DEFAULT )
      cDefPath := STRTRAN( cDefPath, ",", ";" )

      FOR EACH cPath IN ListAsArray( cDefPath, ";" )
         nHandle := FOPEN( cPath + "\" + cLblFile )
         // if no error is reported, we have our label file
         IF EMPTY( nFileError := FERROR() )
            EXIT

         ENDIF

      NEXT

   ENDIF

   // File error
   IF nFileError != F_OK
      err := ErrorNew()
      err:severity := ES_ERROR
      err:genCode := EG_OPEN
      err:subSystem := "FRMLBL"
      err:osCode := nFileError
      err:filename := cLblFile
      Eval(ErrorBlock(), err)
   ENDIF

   // If we got this far, assume the label file is open and ready to go
   // and so go ahead and read it
   nReadCount := FREAD( nHandle, @cBuff, BUFFSIZE )

   // READ ok?
   IF nReadCount == 0
      nFileError := F_EMPTY             // File is empty
   ELSE
      nFileError := FERROR()            // Check for DOS errors
   ENDIF

   IF nFileError == 0

      // Load label dimension into aLabel
      aLabel[ LBL_REMARK ] := SUBSTR(cBuff, REMARKOFFSET, REMARKSIZE)
      aLabel[ LBL_HEIGHT ] := BIN2W(SUBSTR(cBuff, HEIGHTOFFSET, HEIGHTSIZE))
      aLabel[ LBL_WIDTH  ] := BIN2W(SUBSTR(cBuff, WIDTHOFFSET, WIDTHSIZE))
      aLabel[ LBL_LMARGIN] := BIN2W(SUBSTR(cBuff, LMARGINOFFSET, LMARGINSIZE))
      aLabel[ LBL_LINES  ] := BIN2W(SUBSTR(cBuff, LINESOFFSET, LINESSIZE))
      aLabel[ LBL_SPACES ] := BIN2W(SUBSTR(cBuff, SPACESOFFSET, SPACESSIZE))
      aLabel[ LBL_ACROSS ] := BIN2W(SUBSTR(cBuff, ACROSSOFFSET, ACROSSSIZE))

      FOR i := 1 TO aLabel[ LBL_HEIGHT ]

         // Get the text of the expression
         cFieldText := TRIM( SUBSTR( cBuff, nOffset, FIELDSIZE ) )
         nOffset :=nOffSet + 60

         IF !EMPTY( cFieldText )

            AADD( aLabel[ LBL_FIELDS ], {} )

            // Field expression
            AADD( aLabel[ LBL_FIELDS, i ], &( "{ || " + cFieldText + "}" ) )

            // Text of field
            AADD( aLabel[ LBL_FIELDS, i ], cFieldText )

            // Compression option
            AADD( aLabel[ LBL_FIELDS, i ], .T. )

         ELSE

            AADD( aLabel[ LBL_FIELDS ], NIL )

         ENDIF

      NEXT

      // Close file
      FCLOSE( nHandle )
   ENDIF
   RETURN aLabel



FUNCTION __LabelForm( cLBLName, lPrinter, cAltFile, lNoConsole, bFor, ;
                       bWhile, nNext, nRecord, lRest, lSample )

   RETURN HBLabelForm():New( cLBLName, lPrinter, cAltFile, lNoConsole, bFor, ;
                          bWhile, nNext, nRecord, lRest, lSample )

STATIC PROCEDURE PrintIt( cString )

   IF cString == NIL
      cString := ""
   ENDIF
   QQOUT( cString )
   QOUT()

   RETURN

STATIC FUNCTION ListAsArray( cList, cDelimiter )

   LOCAL nPos
   LOCAL aList := {}                  // Define an empty array
   LOCAL lDelimLast := .F.

   IF cDelimiter == NIL
      cDelimiter := ","
   ENDIF

   DO WHILE LEN(cList) <> 0

      nPos := AT(cDelimiter, cList)

      IF nPos == 0
         nPos := LEN(cList)
      ENDIF

      IF ( SUBSTR( cList, nPos, 1 ) == cDelimiter )
         lDelimLast := .T.
         AADD(aList, SUBSTR(cList, 1, nPos - 1)) // Add a new element
      ELSE
         lDelimLast := .F.
         AADD(aList, SUBSTR(cList, 1, nPos)) // Add a new element
      ENDIF

      cList := SUBSTR(cList, nPos + 1)

   ENDDO

   IF lDelimLast
      AADD(aList, "")
   ENDIF

   RETURN aList                       // Return the array

