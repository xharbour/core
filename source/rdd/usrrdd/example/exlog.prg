/*
 * $Id: exlog.prg 9279 2011-02-14 18:06:32Z druzus $
 */

#include "dbinfo.ch"

// Request for LOGRDD rdd driver
REQUEST LOGRDD

// Here put Request for RDD you want to inherit then add
// function hb_LogRddInherit() (see at bottom)
REQUEST DBFCDX

PROCEDURE Main()

   // Set LOGRDD as default RDD otherwise I have to set explicitly use
   // with DRIVER option
   RDDSetDefault( "LOGRDD" )
   // Adding Memofile Info
   rddInfo( RDDI_MEMOVERSION, DB_MEMOVER_CLIP, "LOGRDD" )

   // Define Log File Name and position
   hb_LogRddLogFileName( "logs\changes.log" )
   // Define Tag to add for each line logged
   hb_LogRddTag( NETNAME() + "\" + hb_USERNAME() )
   // Activate Logging, it can be stopped/started at any moment
   hb_LogRddActive( .T. )

   // Uncomment next command to change logged string that I have to return to standard LOGRDD file
   // hb_LogRddMsgLogBlock( {|cTag, cRDDName, cCmd, nWA, xPar1, xPar2, xPar3| MyToString( cTag, cRDDName, cCmd, nWA, xPar1, xPar2, xPar3 ) } )

   // Uncomment next command to change standard destination of my logged string
   // hb_LogRddUserLogBlock( {|cTag, cRDDName, cCmd, nWA, xPar1, xPar2, xPar3| hb_toOutDebug( MyToString( cTag, cRDDName, cCmd, nWA, xPar1, xPar2, xPar3 ) + "\n\r" ) } )

   // Start program logic

   // Open a table with logging (default RDD is LOGRDD)
   USE test
   field->name := "Francesco"
   CLOSE

   // Open a table without logging
   USE test DRIVER "DBFCDX"
   APPEND BLANK
   field->name := "Francesco"

   RETURN

STATIC FUNCTION MyToString( cCmd, nWA, xPar1, xPar2, xPar3 )
   LOCAL cString

   DO CASE
      CASE cCmd == "CREATE"
           // Parameters received: xPar1 = aOpenInfo
           cString := xPar1[ UR_OI_NAME ]
      CASE cCmd == "CREATEFIELDS"
           // Parameters received: xPar1 = aStruct
           cString := hb_ValToExp( xPar1 )
      CASE cCmd == "OPEN"
           // Parameters received: xPar1 = aOpenInfo
           // cString := 'Table : "' + xPar1[ UR_OI_NAME ] + '", Alias : "' + Alias() + '", WorkArea : ' + LTrim( Str( nWA ) )
           // In this example I don't want to log Open Command
      CASE cCmd == "CLOSE"
           // Parameters received: xPar1 = cTableName, xPar2 = cAlias
           //cString := 'Table : "' + xPar1 + '", Alias : "' + xPar2 + '", WorkArea : ' + LTrim( Str( nWA ) )
           // In this example I don't want to log Close Command
      CASE cCmd == "APPEND"
           // Parameters received: xPar1 = lUnlockAll
           cString := Alias() + "->RecNo() = " + LTrim( Str( RecNo() ) )
      CASE cCmd == "DELETE"
           // Parameters received: none
           cString := Alias() + "->RecNo() = " + LTrim( Str( RecNo() ) )
      CASE cCmd == "RECALL"
           // Parameters received: none
           cString := Alias() + "->RecNo() = " + LTrim( Str( RecNo() ) )
      CASE cCmd == "PUTVALUE"
           // Parameters received: xPar1 = nField, xPar2 = xValue, xPar3 = xOldValue
#ifndef __XHARBOUR__
           HB_UNUSED_VAR( xPar3 ) // Here don't log previous value
#endif
           cString := Alias() + "(" + LTrim( Str( RecNo() ) ) + ")->" + PadR( FieldName( xPar1 ), 10 ) + " := " + hb_LogRddValueToText( xPar2 )
      CASE cCmd == "ZAP"
           // Parameters received: none
           cString := 'Alias : "' + Alias() + ' Table : "' + dbInfo( DBI_FULLPATH ) + '"'
   ENDCASE
   RETURN cString

FUNCTION hb_LogRddInherit()
   RETURN "DBFCDX"
