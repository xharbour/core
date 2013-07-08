/*
 * $Id: test37.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test37.prg
   Copying records to a new DBF files from current work area ....
   COPY TO FILE FIELDS FOR NEXT .....
*/
#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "c:\windows\temp\sixtest.dbf"
   LOCAL aStruct := { ;
      { "MYCHAR"    , "C", 15, 0 }, ;
      { "MYDATE"    , "D", 8, 0 }, ;
      { "MYNUMBER1" , "N", 8, 0 }, ;
      { "MYNUMBER2" , "N", 8, 2 }, ;
      { "MYLOGICAL" , "L", 1, 0 }, ;
      { "MYMEMO"    , "M", 10, 0 } }
   LOCAL j, n := seconds(), nArea, cPad

   SET EPOCH 1950
   SET DATE "DD/MM/YYYY"

   IF File( "c:\windows\temp\myText.Txt" )
      FErase( "c:\windows\temp\myText.Txt" )
   ENDIF

   CREATE DBF cFile STRUCT aStruct

   USE cFile ALIAS MYALIAS VAR nArea EXCLUSIVE

   ? "--------------------------------"
   ? "Polupating DBF with 1000 Records"
   ? "--------------------------------"
   ? "Area  : ", nArea
   ? "RDD   : " + sx_rddDriver( nArea )
   ? "Start : ", n

   FOR j := 1 TO 1000
      APPEND BLANK
      cPad := PADL( j, 5, "0" )
      FieldPut( MYCHAR, "NAME_" + cPad )
      FieldPut( MYDATE,  date() + j  )
      FieldPut( MYNUMBER1,   j * 10 )
      FieldPut( MYNUMBER2, ( j * 13 ) / 3 )
      FieldPut( MYLOGICAL, ( j %  2 ) == 0 )
      FieldPut( MYMEMO,      "My Memo" )
   NEXT

   COMMIT

   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n

   ?
   ? 'Now will copy to new file using FIELD clause ... Press any key ...'
   PAUSE
   n := seconds()
   ?
   ? 'COPY TO MYCOPY FIELDS MYCHAR,MYNUMBER1,MYLOGICAL;'
   ? '   WHILE SX_GETVALUE("MYDATE")<= CTOD("31/12/2003")'

   // COPY TO MYCOPY ALL

   COPY TO MYCOPY FIELDS MYCHAR, MYNUMBER1, MYLOGICAL ;
      WHILE SX_GETVALUE( "MYDATE" ) <= CTOD( "31/12/2003" )

   ?
   ? "Start : ", n
   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? 'Now will browse the newly created DBF ... Press any key ...'
   PAUSE

   USE "MYCOPY"
   BROWSE
   CLOSE
   cls

   ?
   ? 'Now will copy to new file (NOT using FIELDS clause) ... Press any key ...'
   PAUSE
   n := seconds()
   ?
   ? 'COPY TO MYCOPY WHILE SX_GETVALUE("MYDATE")<= CTOD("31/12/2003")'

   COPY TO MYCOPY WHILE SX_GETVALUE( "MYDATE" ) <= CTOD( "31/12/2003" )

   ?
   ? "Start : ", n
   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n
   ?
   ? 'Now will browse the newly created DBF ... Press any key ...'
   PAUSE

   USE "MYCOPY"
   BROWSE

   CLOSE ALL
