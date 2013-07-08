/*
 * $Id: test34.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test34.prg
   creating new index files, seek and found tests
   closing files and reusing existing index file
   COPYTEXT .....
*/
#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN()

   LOCAL cFile   := "sixtest.dbf"
   LOCAL aStruct := { ;
      { "MYCHAR"    , "C", 15, 0 }, ;
      { "MYDATE"    , "D", 8, 0 }, ;
      { "MYNUMBER1" , "N", 8, 0 }, ;
      { "MYNUMBER2" , "N", 8, 0 } }
   LOCAL j, n := seconds(), nArea, cPad
   LOCAL cApplication

   SET EPOCH 1950
   SET DATE "DD/MM/YYYY"

   IF File( "C:\WINDOWS\TEMP\myText.Txt" )
      FErase( "C:\WINDOWS\TEMP\myText.Txt" )
   ENDIF

   CREATE DBF cFile STRUCT aStruct RDD SDENSX

   USE cFile ALIAS MYALIAS RDD SDENSX VAR nArea EXCLUSIVE

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
      FieldPut( MYNUMBER2,   j * 20 )
   NEXT

   COMMIT

   ? "End   : ", seconds()
   ? "Time  : ", seconds() - n

/*
Delimiter should be one of the followings:
   COMMA_DELIM
   SDF_FILE
   TAB_DELIM
   SPACE_DELIM
*/

   COPYTEXT TO C:\WINDOWS\TEMP\mytext.txt DELIMITED WITH TAB_DELIM
   // COPYTEXT TO mytext.txt DELIMITED WITH COMMA_DELIM
   // COPYTEXT TO mytext.txt DELIMITED WITH SDF_DELIM
   // COPYTEXT TO mytext.txt DELIMITED WITH SPACE_DELIM

   CLOSE ALL

   IF !empty( cApplication := appReg( "txt" ) )
      ? 'Now will browse text file ... Press any key ...'
      PAUSE
      IF File( "c:\windows\temp\myText.Txt" )
         __run( cApplication + " " + "c:\windows\temp\mytext.txt" )
      ENDIF
   ENDIF
