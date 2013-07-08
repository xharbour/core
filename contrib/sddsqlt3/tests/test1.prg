/*
 * $Id: test1.prg 9486 2012-06-23 13:54:44Z andijahja $
 */

#include "simpleio.ch"
#include "hbrddsql.ch"

REQUEST SDDSQLITE3, SQLMIX

PROCEDURE Main()
   LOCAL tmp
   LOCAL cpath

   HB_FNAMESPLIT( EXENAME(), @cPath )
   RDDSETDEFAULT( "SQLMIX" )
   SET( _SET_DATEFORMAT, "yyyy-mm-dd" )

   AEVAL( rddList(), {| X | QOut( X ) } )

   ? "-1-"
   ? "Connect:", tmp := RDDINFO( RDDI_CONNECT, { "SQLITE3", cPath + "test.sq3" } )
   IF tmp == 0
      ? "Unable connect to the server"
   ENDIF
   ? "-2-"
   ? "Use:", DBUSEAREA( .T.,, "select * from t1", "t1" )
   ? "-3-"
   ? "Alias:", ALIAS()
   ? "-4-"
   ? "DB struct:", VALTOPRG( DBSTRUCT() )
   ? "-5-"
   FOR tmp := 1 TO FCount()
      ? FIELDNAME( tmp ), HB_FIELDTYPE( tmp )
   NEXT
   ? "-6-"
   INKEY( 0 )
   BROWSE()

   INDEX ON FIELD->AGE TO age
   DBGOTOP()
   BROWSE()
   DBCLOSEAREA()

   RETURN
