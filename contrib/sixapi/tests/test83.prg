/*
 * $Id: test83.prg 9576 2012-07-17 16:41:57Z andijahja $
 */
/*
   test83.prg
   sx_Query(), sx_QueryTest(), sx_QueryRecCount()
*/
#include "sixapi.ch"
#define EOL chr(10)
#command ? => outstd(EOL)
#command ? <xx,...> => outstd(<xx>, EOL)

PROCEDURE MAIN()

   USE "TEST/TEST"
   ? 'Creating index ...'
   FERASE( "TEST.NSX" )
   INDEX ON STATE TO TEST
   ? 'Making Query ...'
   ? [sx_QueryTest( "STATE LIKE '%L%'" ) =], sx_QueryTest( "STATE LIKE '%L'" )
   
   ? [sx_QueryTest( "STATE='LA'" ) =], sx_QueryTest( "STATE='LA'" )
   ? [sx_Query( "STATE='LA'" ) =], sx_Query( "STATE='LA'" )
   ? [sx_QueryRecCount() =], sx_QueryRecCount()
   ? 'Now Browse .. Press any key ...'
   PAUSE
   BROWSE
   CLS
   ?
   ? 'Now clear the Query and BROWSE ... Press any key ...'
   PAUSE
   sx_QueryClear()
   BROWSE
