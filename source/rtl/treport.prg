/*
 * $Id: treport.prg 9941 2013-03-25 17:42:55Z enricomaria $
 */

/*
 * Harbour Project source code:
 * HBreportForm class and __ReportForm()
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

#define _RFRM_PAGENO            3       // "Page No."
#define _RFRM_SUBTOTAL          4       // "** Subtotal **"
#define _RFRM_SUBSUBTOTAL       5       // "* Subsubtotal *"
#define _RFRM_TOTAL             6       // "*** Total ***"

#define RPT_HEADER      1       // Array of header strings
#define RPT_WIDTH       2       // Numeric, report page width
#define RPT_LMARGIN     3       // Numeric, report page offset
#define RPT_RMARGIN     4       // NIL, Not used
#define RPT_LINES       5       // Numeric, number of lines per page
#define RPT_SPACING     6       // Numeric, single=1, double=2
#define RPT_BEJECT      7       // Logical, eject before 1st page, .T.=Yes .F.=No
#define RPT_AEJECT      8       // Logical, eject after last page, .T.=Yes .F.=No
#define RPT_PLAIN       9       // Logical, plain report, .T.=Yes .F.=No
#define RPT_SUMMARY     10      // Logical, no detail lines, .T.=Yes .F.=No
#define RPT_COLUMNS     11      // Array of Column arrays
#define RPT_GROUPS      12      // Array of Group arrays
#define RPT_HEADING     13      // Character, heading for the report

#define RPT_COUNT       13      // Number of elements in the Report array


// Column array definitions ( one array per column definition )
#define RCT_EXP         1       // Block, contains compiled column expression
#define RCT_TEXT        2       // Character, contains text column expression
#define RCT_TYPE        3       // Character, type of expression
#define RCT_HEADER      4       // Array of column heading strings
#define RCT_WIDTH       5       // Numeric, column width including decimals and
                                // decimal point
#define RCT_DECIMALS    6       // Numeric, number of decimal places
#define RCT_TOTAL       7       // Logical, total this column, .T.=Yes .F.=No
#define RCT_PICT        8       // Character, picture string

#define RCT_COUNT       8       // Number of elements in the Column array


// Group array definitions ( one array per group definition )
#define RGT_EXP         1       // Block, contains compiled group expression
#define RGT_TEXT        2       // Character, contains text group expression
#define RGT_TYPE        3       // Character, type of expression
#define RGT_HEADER      4       // Character, column heading string
#define RGT_AEJECT      5       // Logical, eject after group, .T.=Yes .F.=No

#define RGT_COUNT       5       // Number of elements in the Group array

#define SIZE_FILE_BUFF          1990    // Size of report file
#define SIZE_LENGTHS_BUFF       110
#define SIZE_OFFSETS_BUFF       110
#define SIZE_EXPR_BUFF          1440
#define SIZE_FIELDS_BUFF        300
#define SIZE_PARAMS_BUFF        24

// Definitions for offsets into the FILE_BUFF string
#define LENGTHS_OFFSET          5       // Start of expression length array
#define OFFSETS_OFFSET          115     // Start of expression position array
#define EXPR_OFFSET             225     // Start of expression data area
#define FIELDS_OFFSET           1665    // Start of report columns (fields)
#define PARAMS_OFFSET           1965    // Start of report parameters block

// These are offsets into the FIELDS_BUFF string to actual values
// Values are added to a block offset FLD_OFFSET that is moved in
// increments of 12
#define FIELD_WIDTH_OFFSET      1
#define FIELD_TOTALS_OFFSET     6
#define FIELD_DECIMALS_OFFSET   7

// These are offsets into FIELDS_BUFF which are used to 'point' into
// the EXPR_BUFF string which contains the textual data
#define FIELD_CONTENT_EXPR_OFFSET       9
#define FIELD_HEADER_EXPR_OFFSET        11

// These are actual offsets into the PARAMS_BUFF string which
// are used to 'point' into the EXPR_BUFF string
#define PAGE_HDR_OFFSET         1
#define GRP_EXPR_OFFSET         3
#define SUB_EXPR_OFFSET         5
#define GRP_HDR_OFFSET          7
#define SUB_HDR_OFFSET          9

// These are actual offsets into the PARAMS_BUFF string to actual values
#define PAGE_WIDTH_OFFSET       11
#define LNS_PER_PAGE_OFFSET     13
#define LEFT_MRGN_OFFSET        15
#define RIGHT_MGRN_OFFSET       17
#define COL_COUNT_OFFSET        19
#define DBL_SPACE_OFFSET        21
#define SUMMARY_RPT_OFFSET      22
#define PE_OFFSET               23
#define OPTION_OFFSET           24

CLASS HBReportForm

   DATA aReportData AS ARRAY init {}
   DATA aReportTotals AS ARRAY init {}
   DATA aGroupTotals AS ARRAY init {}
   DATA nPageNumber AS NUMERIC
   DATA nLinesLeft AS NUMERIC
   DATA lFirstPass AS LOGICAL
   DATA lFormFeeds AS LOGICAL
   DATA nMaxLinesAvail AS NUMERIC
   DATA cExprBuff AS STRING
   DATA cOffsetsBuff AS STRING
   DATA cLengthsBuff AS STRING

   METHOD NEW( cFrmName AS STRING, lPrinter AS LOGICAL ,cAltFile AS STRING, lNoConsole AS LOGICAL ,bFor AS CODEBLOCK, ;
               bWhile AS CODEBLOCK, nNext AS NUMERIC, nRecord AS NUMERIC, lRest AS LOGICAL ,lPlain AS LOGICAL, ;
               cHeading AS STRING ,lBEject AS LOGICAL, lSummary AS LOGICAL )
   METHOD ExecuteReport()
   METHOD ReportHeader()
   METHOD EjectPage()
   METHOD PrintIt(cString AS STRING)
   METHOD LoadReportFile(cFile AS STRING)
   METHOD GetExpr( nPointer AS NUMERIC)
   METHOD GetColumn( cFieldsBuffer AS STRING, nOffset AS NUMERIC)

ENDCLASS

METHOD NEW(cFrmName,lPrinter,cAltFile,lNoConsole,bFor,bWhile,nNext,nRecord,;
           lRest,lPlain,cHeading,lBEject,lSummary) CLASS HBReportForm

   LOCAL lPrintOn, lConsoleOn // Status of PRINTER and CONSOLE
   LOCAL cExtraFile, lExtraState // Status of EXTRA
   LOCAL nGroup
   LOCAL xBreakVal, lBroke := .F.
   LOCAL err
   LOCAL cExt
   LOCAL aReport, aTotal

   LOCAL lAnyTotals
   LOCAL lAnySubTotals

   // Resolve parameters
   IF cFRMName == NIL
      err := ErrorNew()
      err:severity := ES_ERROR
      err:genCode := EG_ARG
      err:subSystem := "FRMLBL"
      Eval(ErrorBlock(), err)
   ELSE
      /* NOTE: CA-Cl*pper does an RTrim() on the filename here, 
               but in Harbour we're using _SET_TRIMFILENAME */
      IF Set( _SET_DEFEXTENSIONS )
         hb_FNameSplit( cFRMName, NIL, NIL, @cExt )
         IF Empty( cExt )
            cFRMName += ".frm"
         ENDIF
      ENDIF
   ENDIF

#ifdef OLDCODE
   IF lPrinter == NIL
      lPrinter   := .F.
   ENDIF
#endif

   IF cHeading == NIL
      cHeading := ""
   ENDIF

   // Set output devices

   lPrintOn   := iif( lPrinter,   SET( _SET_PRINTER, lPrinter ), ;
                                       SET( _SET_PRINTER ) )

   lConsoleOn := iif( lNoConsole, SET( _SET_CONSOLE, .F.),       ;
                           SET( _SET_CONSOLE) )

   IF lPrinter                   // To the printer
      ::lFormFeeds := .T.
   ELSE
      ::lFormFeeds := .F.
   ENDIF

   IF (!Empty(cAltFile))            // To file
      lExtraState := SET( _SET_EXTRA, .T. )
      cExtraFile := SET( _SET_EXTRAFILE, cAltFile )
   ENDIF


   BEGIN SEQUENCE

      ::aReportData := ::LoadReportFile( cFRMName )  // Load the frm into an array
      ::nMaxLinesAvail := ::aReportData[RPT_LINES]

      // Modify ::aReportData based on the report parameters
      IF lSummary == .T.             // Set the summary only flag
         ::aReportData[ RPT_SUMMARY ] := lSummary
      ENDIF
      IF lBEject != NIL .AND. lBEject
         ::aReportData[ RPT_BEJECT ]  := .F.
      ENDIF
      IF lPlain                      // Set plain report flag
         ::aReportData[ RPT_PLAIN ]   := .T.
         cHeading               := ""
         ::lFormFeeds             := .F.
      ENDIF
      ::aReportData[ RPT_HEADING ]    := cHeading

      // Add to the left margin if a SET MARGIN has been defined
      // NOTE: uncommenting this line will cause REPORT FORM to respect
      // SET MARGIN to screen/to file, but double the margin TO PRINT
      // ::aReportData[ RPT_LMARGIN ] += SET( _SET_MARGIN )

      ::nPageNumber := 1                  // Set the initial page number
      ::lFirstPass  := .T.             // Set the first pass flag

      ::nLinesLeft  := ::aReportData[ RPT_LINES ]


      // Check to see if a "before report" eject, or TO FILE has been specified
      IF ::aReportData[ RPT_BEJECT ]
         ::EjectPage()

      ENDIF

      // Generate the initial report header manually (in case there are no
      // records that match the report scope)
      ::ReportHeader()

      // Initialize ::aReportTotals to track both group and report totals, then
      // set the column total elements to 0 if they are to be totaled, otherwise
      // leave them NIL
      ::aReportTotals := ARRAY( LEN(::aReportData[RPT_GROUPS]) + 1, ;
                        LEN(::aReportData[RPT_COLUMNS]) )

      // Column total elements      
      // nCol := 1
      FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
         IF aReport[RCT_TOTAL]              
            FOR EACH aTotal IN ::aReportTotals
               AFill( aTotal, 0 )
            NEXT
         ENDIF       
         // nCol++
      NEXT

      // Initialize ::aGroupTotals as an array
      ::aGroupTotals := ARRAY( LEN(::aReportData[RPT_GROUPS]) )

      // Execute the actual report based on matching records
      DBEval( { || ::ExecuteReport() }, bFor, bWhile, nNext, nRecord, lRest )

      // Generate any totals that may have been identified
      // Make a pass through all the groups
      FOR nGroup := LEN(::aReportData[RPT_GROUPS]) TO 1 STEP -1


         // make sure group has subtotals
         lAnySubTotals := .F.
         FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
            IF aReport[RCT_TOTAL]
               lAnySubTotals := .T.
               EXIT              // NOTE
            ENDIF
         NEXT

         IF !lAnySubTotals
            LOOP                 // NOTE
         ENDIF


         // Check to see if we need to eject the page
         IF ::nLinesLeft < 2
            ::EjectPage()
            IF ::aReportData[ RPT_PLAIN ]
               ::nLinesLeft := 1000
            ELSE
               ::ReportHeader()
            ENDIF
         ENDIF

         // Print the first line
         ::PrintIt( SPACE(::aReportData[RPT_LMARGIN]) + ;
               iif(nGroup==1,NationMsg(_RFRM_SUBTOTAL),;
                            NationMsg(_RFRM_SUBSUBTOTAL) ) )

         // Print the second line
         QQOUT( SPACE(::aReportData[RPT_LMARGIN]) )
//         nCol := 1
         FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
            IF HB_EnumIndex() > 1
               QQOUT( " " )
            ENDIF
            IF aReport[RCT_TOTAL]
               QQOUT( TRANSFORM( ::aReportTotals[ nGroup + 1, HB_EnumIndex() ], ;
               aReport[RCT_PICT]) )
            ELSE
               QQOUT( SPACE(aReport[RCT_WIDTH]) )
            ENDIF
//            nCol++
         NEXT

         // Send a cr/lf for the last line
         QOUT()

      NEXT

      // Any report totals?
      lAnyTotals := .F.
      FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
         IF aReport[RCT_TOTAL]
            lAnyTotals := .T.
            EXIT
         ENDIF
      NEXT


      IF lAnyTotals

         // Check to see if we need to eject the page
         IF ::nLinesLeft < 2
            ::EjectPage()
            IF ::aReportData[ RPT_PLAIN ]
               ::nLinesLeft := 1000
            ELSE
               ::ReportHeader()
            ENDIF
         ENDIF

         // Print the first line
         ::PrintIt( SPACE(::aReportData[RPT_LMARGIN]) + NationMsg(_RFRM_TOTAL ) )

         // Print the second line
         QQOUT( SPACE(::aReportData[RPT_LMARGIN]) )
//         nCol := 1
         FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
            IF HB_EnumIndex() > 1
               QQOUT( " " )
            ENDIF
            IF aReport[RCT_TOTAL]
               QQOUT( TRANSFORM( ::aReportTotals[ 1, HB_EnumIndex() ], ;
                  aReport[RCT_PICT]) )
            ELSE
               QQOUT( SPACE(aReport[RCT_WIDTH]) )
            ENDIF
//            nCol++
         NEXT

         // Send a cr/lf for the last line
         QOUT()

      ENDIF

      // Check to see if an "after report" eject, or TO FILE has been specified
      IF ::aReportData[ RPT_AEJECT ]
         ::EjectPage()
      ENDIF


   RECOVER USING xBreakVal

      lBroke := .T.

   END SEQUENCE


   // Clean up and leave
   ::aReportData   := NIL          // Recover the space
   ::aReportTotals  := NIL
   ::aGroupTotals   := NIL
   ::nPageNumber   := NIL
   ::lFirstPass    := NIL
   ::nLinesLeft    := NIL
   ::lFormFeeds    := NIL
   ::nMaxLinesAvail := NIL

   // clean up
   SET( _SET_PRINTER, lPrintOn )    // Set the printer back to prior state
   SET( _SET_CONSOLE, lConsoleOn )     // Set the console back to prior state

   IF (!Empty(cAltFile))            // Set extrafile back
      SET( _SET_EXTRAFILE, cExtraFile )
      SET( _SET_EXTRA, lExtraState )
   ENDIF

   IF lBroke
      // keep the break value going
      BREAK xBreakVal
   END

   RETURN NIL

METHOD PrintIt(cString) CLASS HBReportForm

   IF cString == NIL
      cString := ""
   ENDIF

   QQOUT(cString)
   QOUT()

   RETURN Self

METHOD EjectPage() CLASS HBReportForm

   IF ::lFormFeeds
      EJECT
   ENDIF

   RETURN Self

METHOD ReportHeader() CLASS HBReportForm

   LOCAL nLinesInHeader
   LOCAL aPageHeader    := {}
   LOCAL nHeadingLength := ::aReportData[RPT_WIDTH] - ::aReportData[RPT_LMARGIN] -30
   LOCAL nLine, nMaxColLength, cHeader
   LOCAL nHeadline
   LOCAL nRPageSize
   LOCAL aTempPgHeader
   LOCAL nHeadSize := NIL
   LOCAL cTempPgHeader, cLine, aReport

   nRPageSize := ::aReportData[RPT_WIDTH] - ::aReportData[RPT_RMARGIN]

   IF !::aReportData[RPT_PLAIN]
      IF ::aReportData[RPT_HEADING] == ""
         AADD( aPageHeader,NationMsg(_RFRM_PAGENO) + STR(::nPageNumber,6))

      ELSE
         aTempPgHeader:=ParseHeader( ::aReportData[RPT_HEADING],;
            Occurs(";",::aReportData[RPT_HEADING]) +1 )

         FOR EACH cTempPgHeader IN aTempPgHeader
            nLinesInHeader := MAX( XMLCOUNT( LTRIM( cTempPgHeader ) , ;
               nHeadingLength),1)

            FOR nHeadLine := 1 to nLinesInHeader
               AADD( aPageHeader, SPACE( 15 ) + ;
                  PADC( TRIM( XMEMOLINE( LTRIM( cTempPgHeader ),;
                  nHeadingLength,nHeadLine)), nHeadingLength))

            NEXT nHeadLine
         NEXT
         aPageHeader[ 1 ] := STUFF(aPageHeader[ 1 ], 1, 14, ;
                                   NationMsg(_RFRM_PAGENO)+STR(::nPageNumber,6))

      ENDIF
      AADD( aPageHeader, DTOC(DATE()) )

   ENDIF
   FOR EACH cLine IN ::aReportData[ RPT_HEADER ]
      nLinesInHeader := MAX( XMLCOUNT(LTRIM( cLine ), nHeadSize),1 )

      FOR nHeadLine := 1 to nLinesInHeader

         cHeader:=TRIM( XMEMOLINE( LTRIM( cLine ), nHeadSize,nHeadLine))
         AADD( aPageHeader, SPACE((nRPageSize - ::aReportData[ RPT_LMARGIN ] -;
             LEN( cHeader ) ) / 2   ) + cHeader )

      NEXT nHeadLine

   NEXT

   nLinesInHeader := LEN( aPageHeader)
   nMaxColLength :=0
   FOR EACH aReport IN ::aReportData[ RPT_COLUMNS ]
      nMaxColLength := MAX(LEN( aReport[RCT_HEADER] ), nMaxColLength)
   NEXT
   FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
      ASIZE( aReport[RCT_HEADER] ,nMaxColLength)
   NEXT
   FOR nLine:=1 TO nMaxColLength
      AADD( aPageHeader, "")
   NEXT

   FOR EACH aReport IN ::aReportData[RPT_COLUMNS]    // Cycle through the columns
      FOR nLine := 1 TO nMaxColLength
         IF HB_EnumIndex() > 1
            aPageHeader[ nLinesInHeader + nLine ] += " "
         ENDIF
         IF aReport[RCT_HEADER,nLine] == NIL
            aPageHeader[ nLinesInHeader + nLine ] += ;
                           SPACE( aReport[RCT_WIDTH] )
         ELSE
            IF aReport[RCT_TYPE] == "N"
               aPageHeader[ nLinesInHeader + nLine ] += ;
                           PADL(aReport[RCT_HEADER,nLine],;
                           aReport[RCT_WIDTH])
            ELSE
               aPageHeader[ nLinesInHeader + nLine ] += ;
                           PADR(aReport[RCT_HEADER,nLine],;
                           aReport[RCT_WIDTH])
            ENDIF
         ENDIF
      NEXT
   NEXT

   // Insert the two blank lines between the heading and the actual data
   AADD( aPageHeader, "" )
   AADD( aPageHeader, "" )
   AEVAL( aPageHeader, { | HeaderLine | ;
         ::PrintIt( SPACE(::aReportData[RPT_LMARGIN])+ HeaderLine ) } )

   // Set the page number and number of available lines
   ::nPageNumber++

   // adjust the line count to account for Summer '87 behavior
   ::nLinesLeft := ::aReportData[RPT_LINES] - LEN( aPageHeader )
   ::nMaxLinesAvail := ::aReportData[RPT_LINES] - LEN( aPageHeader )

   RETURN SELF

METHOD ExecuteReport() CLASS HBReportForm

   LOCAL aRecordHeader  := {}          // Header for the current record
   LOCAL aRecordToPrint := {}          // Current record to print
   LOCAL nCol                          // Counter for the column work
   LOCAL nGroup                        // Counter for the group work
   LOCAL lGroupChanged  := .F.         // Has any group changed?
   LOCAL lEjectGrp := .F.              // Group eject indicator
   LOCAL nMaxLines                     // Number of lines needed by record
   LOCAL nLine                         // Counter for each record line
   LOCAL cLine                         // Current line of text for parsing
   LOCAL aReport

   LOCAL lAnySubTotals

   // Add to the main column totals
// nCol := 1
   FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
      IF aReport[RCT_TOTAL]

           // 2004-02-14 Piemonte Gianluca       
           // Initialize aReportTotals[1,HB_EnumIndex()] if it's NIL
           if valtype( ::aReportTotals[ 1 ,HB_EnumIndex()] ) == "U"
              ::aReportTotals[ 1 ,HB_EnumIndex()] := 0
           endif
           
         // If this column should be totaled, do it
         ::aReportTotals[ 1 ,HB_EnumIndex()] += ;
                  EVAL( aReport[RCT_EXP] )
      ENDIF
//    nCol++
   NEXT

   // Determine if any of the groups have changed.  If so, add the appropriate
   // line to aRecordHeader for totaling out the previous records
   IF !::lFirstPass                       // Don't bother first time through

      // Make a pass through all the groups
      FOR nGroup := LEN(::aReportData[RPT_GROUPS]) TO 1 STEP -1


         // make sure group has subtotals
         lAnySubTotals := .F.
         FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
            IF aReport[RCT_TOTAL]
               lAnySubTotals := .T.
               EXIT              // NOTE
            ENDIF
         NEXT

         // retrieve group eject state from report form
         IF ( nGroup == 1 )
            lEjectGrp := ::aReportData[ RPT_GROUPS, nGroup, RGT_AEJECT ]
         ENDIF

         IF !lAnySubTotals
            LOOP                 // NOTE
         ENDIF

         //  For subgroup processing: check if group has been changed
         IF MakeAStr(EVAL(::aReportData[RPT_GROUPS, 1, RGT_EXP]),;
              ::aReportData[RPT_GROUPS, 1, RGT_TYPE]) != ::aGroupTotals[1]
            lGroupChanged  := .T.
         ENDIF

         //  If this (sub)group has changed since the last record
         IF lGroupChanged .OR. MakeAStr(EVAL(::aReportData[RPT_GROUPS,nGroup,RGT_EXP]),;
             ::aReportData[RPT_GROUPS,nGroup,RGT_TYPE]) != ::aGroupTotals[nGroup]

            AADD( aRecordHeader, iif(nGroup==1,NationMsg(_RFRM_SUBTOTAL),;
                                              NationMsg(_RFRM_SUBSUBTOTAL)) )
            AADD( aRecordHeader, "" )


            // Cycle through the columns, adding either the group
            // amount from ::aReportTotals or spaces wide enough for
            // the non-totaled columns
//            nCol := 1
            FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
               IF aReport[RCT_TOTAL]
                  aRecordHeader[ LEN(aRecordHeader) ] += ;
                     TRANSFORM(::aReportTotals[nGroup+1,HB_EnumIndex()], ;
                     aReport[RCT_PICT])
                  // Zero out the group totals column from aReportTotals
                  ::aReportTotals[nGroup+1,HB_EnumIndex()] := 0
               ELSE
                  aRecordHeader[ LEN(aRecordHeader) ] += ;
                        SPACE(aReport[RCT_WIDTH])
               ENDIF
               aRecordHeader[ LEN(aRecordHeader) ] += " "
//               nCol++
            NEXT
            // Get rid of the extra space from the last column
            aRecordHeader[LEN(aRecordHeader)] := ;
                  LEFT( aRecordHeader[LEN(aRecordHeader)], ;
                  LEN(aRecordHeader[LEN(aRecordHeader)]) - 1 )
         ENDIF
      NEXT

   ENDIF


   IF ( LEN( aRecordHeader ) > 0 ) .AND. lEjectGrp .AND. lGroupChanged
      IF LEN( aRecordHeader ) > ::nLinesLeft
         ::EjectPage()

         IF ( ::aReportData[ RPT_PLAIN ] )
            ::nLinesLeft := 1000
         ELSE
            ::ReportHeader()
         ENDIF

      ENDIF

      AEVAL( aRecordHeader, { | HeaderLine | ;
   ::PrintIt( SPACE( ::aReportData[ RPT_LMARGIN ] ) + HeaderLine ) } )

      aRecordHeader := {}

      ::EjectPage()

      IF ( ::aReportData[ RPT_PLAIN ] )
         ::nLinesLeft := 1000

      ELSE
         ::ReportHeader()

      ENDIF

   ENDIF

   // Add to aRecordHeader in the event that the group has changed and
   // new group headers need to be generated

   // Cycle through the groups
//   nGroup := 1
   FOR EACH aReport IN ::aReportData[RPT_GROUPS]
      // If the group has changed
      IF MakeAStr(EVAL(aReport[RGT_EXP]),;
            aReport[RGT_TYPE]) == ::aGroupTotals[HB_EnumIndex()]
      ELSE
         AADD( aRecordHeader, "" )   // The blank line

         // page eject after group

         //  put CRFF after group
         IF HB_EnumIndex() == 1 .AND. !::lFirstPass .AND. !lAnySubTotals
            IF aReport[ RGT_AEJECT ]
               ::nLinesLeft  := 0
            ENDIF
         ENDIF


         AADD( aRecordHeader, iif(HB_EnumIndex()==1,"** ","* ") +;
               aReport[RGT_HEADER] + " " +;
               MakeAStr(EVAL(aReport[RGT_EXP]), ;
               aReport[RGT_TYPE]) )
      ENDIF
//      nGroup++
   NEXT

   ::lFirstPass := .F.

   // Is there anything in the record header?
   IF LEN( aRecordHeader ) > 0
      // Determine if aRecordHeader will fit on the current page.  If not,
      // start a new header
      IF LEN( aRecordHeader ) > ::nLinesLeft
         ::EjectPage()
         IF ::aReportData[ RPT_PLAIN ]
            ::nLinesLeft := 1000
         ELSE
            ::ReportHeader()
         ENDIF
      ENDIF

      // Send aRecordHeader to the output device, resetting nLinesLeft
      AEVAL( aRecordHeader, { | HeaderLine | ;
              ::PrintIt( SPACE(::aReportData[RPT_LMARGIN])+ HeaderLine ) } )

      ::nLinesLeft -= LEN( aRecordHeader )

      // Make sure it didn't hit the bottom margin
      IF ::nLinesLeft == 0
         ::EjectPage()
         IF ::aReportData[ RPT_PLAIN ]
            ::nLinesLeft := 1000
         ELSE
            ::ReportHeader()
         ENDIF
      ENDIF
   ENDIF

   // Add to the group totals
//   nCol := 1
   FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
      // If this column should be totaled, do it
      IF aReport[RCT_TOTAL]
         // Cycle through the groups
         FOR nGroup := 1 TO LEN( ::aReportTotals ) - 1
         
             // 2004-02-14 Piemonte Gianluca 
             // Initialize aReportTotals[1,HB_EnumIndex()] if it's NIL
	     if valtype( ::aReportTotals[ nGroup+1,HB_EnumIndex() ] ) == "U"
	        ::aReportTotals[ nGroup+1,HB_EnumIndex() ] := 0
	     endif

         
            ::aReportTotals[nGroup+1,HB_EnumIndex()] += ;
               EVAL( aReport[RCT_EXP] )
         NEXT
      ENDIF
//      nCol++
   NEXT

   // Reset the group expressions in aGroupTotals
//   nGroup := 1
   FOR EACH aReport IN ::aReportData[RPT_GROUPS]
      ::aGroupTotals[HB_EnumIndex()] := MakeAStr(EVAL(aReport[RGT_EXP]),;
                                                      aReport[RGT_TYPE])
   NEXT

   // Only run through the record detail if this is NOT a summary report
   IF !::aReportData[ RPT_SUMMARY ]
      // Determine the max number of lines needed by each expression
      nMaxLines := 1
      FOR EACH aReport IN ::aReportdata[RPT_COLUMNS]

         IF aReport[RCT_TYPE] == "M"
            nMaxLines := MAX(XMLCOUNT(EVAL(aReport[RCT_EXP]),;
                         aReport[RCT_WIDTH]), nMaxLines)
         ELSEIF aReport[RCT_TYPE] == "C"
            nMaxLines := MAX( XMLCOUNT( STRTRAN( EVAL( aReport[RCT_EXP]),;
                         ";", CHR(13)+CHR(10)),;
                         aReport[RCT_WIDTH]), nMaxLines)
         ENDIF
      NEXT

      // Size aRecordToPrint to the maximum number of lines it will need, then
      // fill it with nulls
      ASIZE( aRecordToPrint, nMaxLines )
      AFILL( aRecordToPrint, "" )

      // Load the current record into aRecordToPrint
      nCol := 1
      FOR EACH aReport IN ::aReportData[RPT_COLUMNS]
         FOR nLine := 1 TO nMaxLines
            // Check to see if it's a memo or character
            IF aReport[RCT_TYPE] IN "CM"
               //  Load the current line of the current column into cLine
               //  with multi-lines per record ";"- method
               IF aReport[RCT_TYPE] == "C"
                  cLine := XMEMOLINE( TRIM( STRTRAN( EVAL(aReport[RCT_EXP]),;
                             ";", CHR(13)+CHR(10)) ),;
                             aReport[RCT_WIDTH], nLine )
               ELSE
                  cLine := XMEMOLINE(TRIM(EVAL(aReport[RCT_EXP])),;
                             aReport[RCT_WIDTH], nLine )
               ENDIF
               cLine := PADR( cLine, aReport[RCT_WIDTH] )
            ELSE
               IF nLine == 1
                  cLine := TRANSFORM(EVAL(aReport[RCT_EXP]),;
                           aReport[RCT_PICT])
                  cLine := PADR( cLine, aReport[RCT_WIDTH] )
               ELSE
                  cLine := SPACE( aReport[RCT_WIDTH])
               ENDIF
            ENDIF
            // Add it to the existing report line
            // 2004-02-14 Gianluca Piemonte
            // Correct the mechanism for adding a space between columns 
            IF nCol > 1 .and. nCol <= len(::aReportData[RPT_COLUMNS])
               aRecordToPrint[ nLine ] += " "
            ENDIF        
            aRecordToPrint[ nLine ] += cLine
         NEXT
         nCol++
      NEXT

      // Determine if aRecordToPrint will fit on the current page
      IF LEN( aRecordToPrint ) > ::nLinesLeft
         // The record will not fit on the current page - will it fit on
         // a full page?  If not, break it up and print it.
         IF LEN( aRecordToPrint ) > ::nMaxLinesAvail
            // This record is HUGE!  Break it up...
            nLine := 1
            DO WHILE nLine < LEN( aRecordToPrint )
               ::PrintIt( SPACE(::aReportData[RPT_LMARGIN]) + aRecordToPrint[nLine] )
               nLine++
               ::nLinesLeft--
               IF ::nLinesLeft == 0
                  ::EjectPage()
                  IF ::aReportData[ RPT_PLAIN ]
                     ::nLinesLeft := 1000
                  ELSE
                     ::ReportHeader()
                  ENDIF
               ENDIF
            ENDDO
         ELSE
            ::EjectPage()
            IF ::aReportData[ RPT_PLAIN ]
               ::nLinesLeft := 1000
            ELSE
               ::ReportHeader()
            ENDIF
            AEVAL( aRecordToPrint, ;
               { | RecordLine | ;
                 ::PrintIt( SPACE(::aReportData[RPT_LMARGIN])+ RecordLine ) ;
               } ;
            )
            ::nLinesLeft -= LEN( aRecordToPrint )
         ENDIF
      ELSE
         // Send aRecordToPrint to the output device, resetting ::nLinesLeft
         AEVAL( aRecordToPrint, ;
            { | RecordLine | ;
              ::PrintIt( SPACE(::aReportData[RPT_LMARGIN])+ RecordLine ) ;
            } ;
         )
         ::nLinesLeft -= LEN( aRecordToPrint )
      ENDIF


      // Tack on the spacing for double/triple/etc.
      IF ::aReportData[ RPT_SPACING ] > 1

         /*  Double space problem in REPORT FORM at the bottom of the page  */
         IF ::nLinesLeft >= ::aReportData[ RPT_SPACING ] - 1

            FOR nLine := 2 TO ::aReportData[ RPT_SPACING ]
               ::PrintIt()
               ::nLinesLeft--
            NEXT
         ENDIF
      ENDIF

   ENDIF    // Was this a summary report?

   RETURN NIL

METHOD LoadReportFile(cFrmFile) CLASS HBReportForm
   LOCAL cFieldsBuff
   LOCAL cParamsBuff
   LOCAL nFieldOffset   := 0
   LOCAL cFileBuff      := SPACE(SIZE_FILE_BUFF)
   LOCAL cGroupExp
   LOCAL cSubGroupExp
   LOCAL nColCount                  // Number of columns in report
   LOCAL nCount
   LOCAL nFrmHandle                 // (.frm) file handle
   LOCAL nBytesRead                 // Read/write and content record counter
   LOCAL nPointer                   // Points to an offset into EXPR_BUFF string
   LOCAL nFileError                 // Contains current file error
   LOCAL cOptionByte                // Contains option byte

   LOCAL aReport[ RPT_COUNT ]        // Create report array
   LOCAL err                        // error object

   LOCAL cDefPath          // contents of SET DEFAULT string
   LOCAL aPaths            // array of paths
   LOCAL cPath             // iteration paths

   LOCAL aHeader                           // temporary storage for report form headings
   LOCAL nHeaderIndex              // index into temporary header array

   // Initialize STATIC buffer values
   ::cLengthsBuff  := ""
   ::cOffSetsBuff  := ""
   ::cExprBuff     := ""

   // Default report values
   aReport[ RPT_HEADER ]    := {}
   aReport[ RPT_WIDTH ]     := 80
   aReport[ RPT_LMARGIN ]   := 8
   aReport[ RPT_RMARGIN ]   := 0
   aReport[ RPT_LINES ]     := 58
   aReport[ RPT_SPACING ]   := 1
   aReport[ RPT_BEJECT ]    := .T.
   aReport[ RPT_AEJECT ]    := .F.
   aReport[ RPT_PLAIN ]     := .F.
   aReport[ RPT_SUMMARY ]   := .F.
   aReport[ RPT_COLUMNS ]   := {}
   aReport[ RPT_GROUPS ]    := {}
   aReport[ RPT_HEADING ]   := ""

   // Open the report file
   nFrmHandle := FOPEN( cFrmFile )

   IF ( !EMPTY( nFileError := FERROR() ) ) .AND. !( "\" IN cFrmFile .OR. ":" IN cFrmFile )

      // Search through default path; attempt to open report file
      cDefPath := SET( _SET_DEFAULT ) + ";" + SET( _SET_PATH )
      cDefPath := STRTRAN( cDefPath, ",", ";" )
      aPaths := ListAsArray( cDefPath, ";" )

      FOR EACH cPath IN aPaths
         nFrmHandle := FOPEN( cPath + "\" + cFrmFile )
         // if no error is reported, we have our report file
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
      err:filename := cFrmFile
      Eval(ErrorBlock(), err)
   ENDIF

   // OPEN ok?
   IF nFileError == F_OK

      // Go to START of report file
      FSEEK(nFrmHandle, 0)

      // SEEK ok?
      nFileError := FERROR()
      IF nFileError == F_OK

         // Read entire file into process buffer
         nBytesRead := FREAD(nFrmHandle, @cFileBuff, SIZE_FILE_BUFF)

         // READ ok?
         IF nBytesRead == 0
            nFileError := F_EMPTY        // file is empty
         ELSE
            nFileError := FERROR()       // check for DOS errors
         ENDIF

         IF nFileError == F_OK

            // Is this a .FRM type file (2 at start and end of file)
            IF BIN2W(SUBSTR(cFileBuff, 1, 2)) == 2 .AND.;
               BIN2W(SUBSTR(cFileBuff, SIZE_FILE_BUFF - 1, 2)) == 2

               nFileError := F_OK
            ELSE
               nFileError := F_ERROR
            ENDIF

         ENDIF

      ENDIF

      // Close file
      IF !FCLOSE(nFrmHandle)
         nFileError := FERROR()
      ENDIF

   ENDIF

   // File existed, was opened and read ok and is a .FRM file
   IF nFileError == F_OK

      // Fill processing buffers
      ::cLengthsBuff := SUBSTR(cFileBuff, LENGTHS_OFFSET, SIZE_LENGTHS_BUFF)
      ::cOffSetsBuff := SUBSTR(cFileBuff, OFFSETS_OFFSET, SIZE_OFFSETS_BUFF)
      ::cExprBuff    := SUBSTR(cFileBuff, EXPR_OFFSET, SIZE_EXPR_BUFF)
      cFieldsBuff  := SUBSTR(cFileBuff, FIELDS_OFFSET, SIZE_FIELDS_BUFF)
      cParamsBuff  := SUBSTR(cFileBuff, PARAMS_OFFSET, SIZE_PARAMS_BUFF)


      // Process report attributes
      // Report width
      aReport[ RPT_WIDTH ]   := BIN2W(SUBSTR(cParamsBuff, PAGE_WIDTH_OFFSET, 2))

      // Lines per page
      aReport[ RPT_LINES ]   := BIN2W(SUBSTR(cParamsBuff, LNS_PER_PAGE_OFFSET, 2))

      // Page offset (left margin)
      aReport[ RPT_LMARGIN ] := BIN2W(SUBSTR(cParamsBuff, LEFT_MRGN_OFFSET, 2))

      // Page right margin (not used)
      aReport[ RPT_RMARGIN ] := BIN2W(SUBSTR(cParamsBuff, RIGHT_MGRN_OFFSET, 2))

      nColCount  := BIN2W(SUBSTR(cParamsBuff, COL_COUNT_OFFSET, 2))

      // Line spacing
      // Spacing is 1, 2, or 3
      aReport[ RPT_SPACING ] := iif(SUBSTR(cParamsBuff, ;
       DBL_SPACE_OFFSET, 1) IN "YyTt", 2, 1)

      // Summary report flag
      aReport[ RPT_SUMMARY ] := iif(SUBSTR(cParamsBuff, ;
       SUMMARY_RPT_OFFSET, 1) IN "YyTt", .T., .F.)

      // Process report eject and plain attributes option byte
      cOptionByte := ASC(SUBSTR(cParamsBuff, OPTION_OFFSET, 1))

      IF INT(cOptionByte / 4) == 1
         aReport[ RPT_PLAIN ] := .T.          // Plain page
         cOptionByte -= 4
      ENDIF

      IF INT(cOptionByte / 2) == 1
         aReport[ RPT_AEJECT ] := .T.         // Page eject after report
         cOptionByte -= 2
      ENDIF

      IF INT(cOptionByte / 1) == 1
         aReport[ RPT_BEJECT ] := .F.         // Page eject before report
         //cOptionByte -= 1
      ENDIF

      // Page heading, report title
      nPointer := BIN2W(SUBSTR(cParamsBuff, PAGE_HDR_OFFSET, 2))

      // Retrieve the header stored in the .FRM file
      nHeaderIndex := 4
      aHeader := ParseHeader( ::GetExpr( nPointer ), nHeaderIndex )

      // certain that we have retrieved all heading entries from the .FRM file, we
      // now retract the empty headings
      DO WHILE ( nHeaderIndex > 0 )
         IF ! EMPTY( aHeader[ nHeaderIndex ] )
            EXIT
         ENDIF
         nHeaderIndex--
      ENDDO

      aReport[ RPT_HEADER ] := iif( EMPTY( nHeaderIndex ) , {}, ;
                   ASIZE( aHeader, nHeaderIndex ) )

      // Process Groups
      // Group
      nPointer := BIN2W(SUBSTR(cParamsBuff, GRP_EXPR_OFFSET, 2))

      IF !EMPTY(cGroupExp := ::GetExpr( nPointer ))

         // Add a new group array
         AADD( aReport[ RPT_GROUPS ], ARRAY( RGT_COUNT ))

         // Group expression
         aReport[ RPT_GROUPS ][1][ RGT_TEXT ] := cGroupExp
         aReport[ RPT_GROUPS ][1][ RGT_EXP ] := &( "{ || " + cGroupExp + "}" )
         IF USED()
            aReport[ RPT_GROUPS ][1][ RGT_TYPE ] := ;
                           VALTYPE( EVAL( aReport[ RPT_GROUPS ][1][ RGT_EXP ] ) )
         ENDIF

         // Group header
         nPointer := BIN2W(SUBSTR(cParamsBuff, GRP_HDR_OFFSET, 2))
         aReport[ RPT_GROUPS ][1][ RGT_HEADER ] := ::GetExpr( nPointer )

         // Page eject after group
         aReport[ RPT_GROUPS ][1][ RGT_AEJECT ] := iif(SUBSTR(cParamsBuff, ;
         PE_OFFSET, 1) IN "YyTt", .T., .F.)

      ENDIF

      // Subgroup
      nPointer := BIN2W(SUBSTR(cParamsBuff, SUB_EXPR_OFFSET, 2))

      IF !EMPTY(cSubGroupExp := ::GetExpr( nPointer ))

         // Add new group array
         AADD( aReport[ RPT_GROUPS ], ARRAY( RGT_COUNT ))

         // Subgroup expression
         aReport[ RPT_GROUPS ][2][ RGT_TEXT ] := cSubGroupExp
         aReport[ RPT_GROUPS ][2][ RGT_EXP ] := &( "{ || " + cSubGroupExp + "}" )
         IF USED()
            aReport[ RPT_GROUPS ][2][ RGT_TYPE ] := ;
                           VALTYPE( EVAL( aReport[ RPT_GROUPS ][2][ RGT_EXP ] ) )
         ENDIF

         // Subgroup header
         nPointer := BIN2W(SUBSTR(cParamsBuff, SUB_HDR_OFFSET, 2))
         aReport[ RPT_GROUPS ][2][ RGT_HEADER ] := ::GetExpr( nPointer )

         // Page eject after subgroup
         aReport[ RPT_GROUPS ][2][ RGT_AEJECT ] := .F.

      ENDIF

      // Process columns
      nFieldOffset := 12      // dBASE skips first 12 byte fields block.
      FOR nCount := 1 to nColCount

         AADD( aReport[ RPT_COLUMNS ], ::GetColumn( cFieldsBuff, @nFieldOffset ) )

      NEXT nCount

   ENDIF

   RETURN aReport

/***
*  GetExpr( nPointer ) --> cString
*
*  Reads an expression from EXPR_BUFF via the OFFSETS_BUFF and returns
*  a pointer to offset contained in OFFSETS_BUFF that in turn points
*  to an expression located in the EXPR_BUFF string.
*
*  Notes:
*
*     1. The expression is empty if:
*         a. Passed pointer is equal to 65535
*         b. Character following character pointed to by pointer is CHR(0)
*
*/

METHOD GetExpr( nPointer ) CLASS HBReportForm
   LOCAL nExprOffset
   LOCAL nExprLength
   LOCAL nOffsetOffset := 0
   LOCAL cString := ""

   // Stuff for dBASE compatability.
   IF nPointer != 65535

      // Convert DOS FILE offset to CLIPPER string offset
      nPointer++

      // Calculate offset into OFFSETS_BUFF
      IF nPointer > 1
         nOffsetOffset := (nPointer * 2) - 1
      ENDIF

      nExprOffset := BIN2W(SUBSTR(::cOffsetsBuff, nOffsetOffset, 2))
      nExprLength := BIN2W(SUBSTR(::cLengthsBuff, nOffsetOffset, 2))

      // EXPR_OFFSET points to a NULL, so add one (+1) to get the string
      // and subtract one (-1) from EXPR_LENGTH for correct length

      nExprOffset++
      nExprLength--

      // Extract string
      cString := SUBSTR(::cExprBuff, nExprOffset, nExprLength)

      // dBASE does this so we must do it too
      // Character following character pointed to by pointer is NULL
      IF CHR(0) == SUBSTR(cString, 1, 1) .AND. LEN(SUBSTR(cString,1,1)) == 1
         cString := ""
      ENDIF
   ENDIF

   RETURN cString

STATIC FUNCTION Occurs( cSearch, cTarget )
   LOCAL nPos, nCount := 0

   DO WHILE !EMPTY( cTarget )
      IF (nPos := AT( cSearch, cTarget )) != 0
         nCount++
         cTarget := SUBSTR( cTarget, nPos + 1 )
      ELSE
         // End of string
         cTarget := ""
      ENDIF
   ENDDO

   RETURN nCount

STATIC FUNCTION XMLCOUNT( cString, nLineLength, nTabSize, lWrap )
   // Set defaults if none specified
   nLineLength := iif( nLineLength == NIL, 79, nLineLength )
   nTabSize := iif( nTabSize == NIL, 4, nTabSize )
   lWrap := iif( lWrap == NIL, .T., .F. )

   IF nTabSize >= nLineLength
      nTabSize := nLineLength - 1
   ENDIF
   RETURN MLCOUNT( TRIM(cString), nLineLength, nTabSize, lWrap )

/***
*
*  XMEMOLINE( <cString>, [<nLineLength>], [<nLineNumber>],
*         [<nTabSize>], [<lWrap>] ) --> cLine
*
*/
STATIC FUNCTION XMEMOLINE( cString, nLineLength, nLineNumber, nTabSize, lWrap )

   // Set defaults if none specified
   nLineLength := iif( nLineLength == NIL, 79, nLineLength )
   nLineNumber := iif( nLineNumber == NIL, 1, nLineNumber )
   nTabSize := iif( nTabSize == NIL, 4, nTabSize )
   lWrap := iif( lWrap == NIL, .T., lWrap )

   IF nTabSize >= nLineLength
      nTabSize := nLineLength - 1
   ENDIF

   RETURN( MEMOLINE( cString, nLineLength, nLineNumber, nTabSize, lWrap ) )

STATIC FUNCTION ParseHeader( cHeaderString, nFields )
   LOCAL cItem
   LOCAL nItemCount := 0
   LOCAL aPageHeader := {}
   LOCAL nHeaderLen := 254
   LOCAL nPos

   DO WHILE ( ++nItemCount <= nFields )

      cItem := SUBSTR( cHeaderString, 1, nHeaderLen )

      // check for explicit delimiter
      nPos := AT( ";", cItem )

      IF ! EMPTY( nPos )
         // delimiter present
         AADD( aPageHeader, SUBSTR( cItem, 1, nPos - 1 ) )
      ELSE
         IF EMPTY( cItem )
            // empty string for S87 and 5.0 compatibility
            AADD( aPageHeader, "" )
         ELSE
            // exception
            AADD( aPageHeader, cItem )

         ENDIF
         // empty or not, we jump past the field
         nPos := nHeaderLen
      ENDIF

      cHeaderString := SUBSTR( cHeaderString, nPos + 1 )

   ENDDO

   RETURN aPageHeader

/***
*  GetColumn( <cFieldBuffer>, @<nOffset> ) --> aColumn
*
*  Get a COLUMN element from FIELDS_BUFF string using nOffset to point to
*  the current FIELDS_OFFSET block.
*
*  Notes:
*     1. The Header or Contents expressions are empty if:
*        a. Passed pointer is equal to 65535
*        b. Character following character pointed to by pointer is CHR(0)
*
*/
METHOD GetColumn( cFieldsBuffer, nOffset ) CLASS HBReportForm
   LOCAL nPointer, aColumn[ RCT_COUNT ], cType,cExpr

   // Column width
   aColumn[ RCT_WIDTH ] := BIN2W(SUBSTR(cFieldsBuffer, nOffset + ;
                FIELD_WIDTH_OFFSET, 2))

   // Total column?
   aColumn[ RCT_TOTAL ] := iif(SUBSTR(cFieldsBuffer, nOffset + ;
    FIELD_TOTALS_OFFSET, 1) IN "YyTt", .T., .F.)   

   // Decimals width
   aColumn[ RCT_DECIMALS ] := BIN2W(SUBSTR(cFieldsBuffer, nOffset + ;
                FIELD_DECIMALS_OFFSET, 2))

   // Offset (relative to FIELDS_OFFSET), 'point' to
   // expression area via array OFFSETS[]

   // Content expression
   nPointer := BIN2W(SUBSTR(cFieldsBuffer, nOffset +;
               FIELD_CONTENT_EXPR_OFFSET, 2))
   aColumn[ RCT_TEXT ] := ::GetExpr( nPointer )
   cExpr := aColumn[ RCT_TEXT ]
   aColumn[ RCT_EXP ] := &( "{ || " + cExpr + "}" )   
   
   // Header expression
   nPointer := BIN2W(SUBSTR(cFieldsBuffer, nOffset +;
               FIELD_HEADER_EXPR_OFFSET, 2))

   aColumn[ RCT_HEADER ] := ListAsArray(::GetExpr( nPointer ), ";")

   // Column picture
   // Setup picture only if a database file is open
   IF USED()
      cType := VALTYPE( EVAL(aColumn[ RCT_EXP ]) )
      aColumn[ RCT_TYPE ] := cType

      SWITCH cType
      CASE "C"
      CASE "M"
         aColumn[ RCT_PICT ] := REPLICATE("X", aColumn[ RCT_WIDTH ])
         exit
      CASE "D"
         aColumn[ RCT_PICT ] := "@D"
         exit
      CASE "N"
         IF aColumn[ RCT_DECIMALS ] != 0
            aColumn[ RCT_PICT ] := REPLICATE("9", aColumn[ RCT_WIDTH ] - aColumn[ RCT_DECIMALS ] -1) + "." + ;
                                  REPLICATE("9", aColumn[ RCT_DECIMALS ])
         ELSE
            aColumn[ RCT_PICT ] := REPLICATE("9", aColumn[ RCT_WIDTH ])
         ENDIF
         exit
      CASE "L"
         aColumn[ RCT_PICT ] := "@L" + REPLICATE("X",aColumn[ RCT_WIDTH ]-1)
         exit
      END
   ENDIF

   // Update offset into ?_buffer
   nOffset += 12

   RETURN aColumn

/***
*
*  ListAsArray( <cList>, <cDelimiter> ) --> aList
*  Convert a delimited string to an array
*
*/
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

      IF SUBSTR( cList, nPos, 1 ) == cDelimiter
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

STATIC FUNCTION MakeAStr( uVar, cType )
   LOCAL cString
   
   // 2004-02-14 Piemonte Gianluca
   // Add EXIT 
   SWITCH UPPER(cType)
   CASE "D"
      cString := DTOC( uVar )
      EXIT

   CASE "L"
      cString := iif( uVar, "T", "F" )
      EXIT

   CASE "N"
      cString := STR( uVar )
      EXIT

   CASE "C"
      cString := uVar
      EXIT

   DEFAULT
      cString := "INVALID EXPRESSION"
   END
   RETURN cString

FUNCTION __ReportForm( cFRMName, lPrinter, cAltFile, lNoConsole, bFor, ;
                       bWhile, nNext, nRecord, lRest, lPlain, cHeading, ;
                       lBEject, lSummary )
   RETURN HBReportForm():New(cFrmName,lPrinter,cAltFile,lNoConsole,bFor,bWhile,nNext,nRecord,;
              lRest,lPlain,cHeading,lBEject,lSummary)
