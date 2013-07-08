/*
 * $Id: test76.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test76.prg
   sx_FLock()
*/

#include "sixapi.ch"

#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)
 
PROCEDURE MAIN

   LOCAL aStruct1 := { ;
      { "PART_NO"   , "C" , 10, 0 }, ;
      { "PART_NAME" , "C", 10, 0 }, ;
      { "PRICE"     , "N", 10, 2 }, ;
      { "DATE_SOLD" , "D", 8, 0 }, ;
      { "COST"      , "N", 10, 2 }, ;
      { "NOTES"     , "M", 10, 0 } }
   LOCAL nCount, i, cFieldName

   SET RDD SDENSX
   SET DATE "dd/mm/yyyy"
   CREATE TABLE "TEST1" STRUCT aStruct1
   USE "test1"
   APPEND BLANK

   ? sx_Flock()
