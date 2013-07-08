//
// $Id: newrdd.prg 9279 2011-02-14 18:06:32Z druzus $
//

#define TESTED_RDD "DBFDBT"
REQUEST _DBFDBT

function Main()

   local nI, aArray

   rddSetDefault( TESTED_RDD )

   SET EXCLUSIVE OFF
   CLS

   dbUseArea( .T., TESTED_RDD, "test", "TESTDBF", .T., .F. )
   ? "RecCount:", TESTDBF->( RecCount() )
   ? "Used:", TESTDBF->( Used() )
   ? "Select:", TESTDBF->( Select() )
   ? "RecSize:", TESTDBF->( RecSize() )
   ? "RecNo:", TESTDBF->( RecNo() )
   ? "NetErr:", TESTDBF->( NetErr() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   aArray := RddList( 0 )
   ? "Rdd's: "
   for nI := 1 to Len( aArray )
      ?? aArray[ nI ], ""
   next
   ? "RddName:", TESTDBF->( RddName() )
   ? "lUpdate:", TESTDBF->( lUpdate() )
   ? "Header:", TESTDBF->( Header() )
   ? "FieldPos( 'LAST' ):", TESTDBF->( FieldPos( "LAST" ) )
   ? "FieldName( 2 ):", TESTDBF->( FieldName( 2 ) )
   ? "DBF():", TESTDBF->( DBF() )
   ? "Alias( 1 ):", Alias( 1 )
   ? "dbTableExt():", TESTDBF->( dbTableExt() )
   aArray := TESTDBF->( dbStruct() )
   ? "dbStruct:"
   for nI := 1 to Len( aArray )
      ? PadR( aArray[ nI ][ 1 ], 10 ) , aArray[ nI ][ 2 ], aArray[ nI ][ 3 ], aArray[ nI ][ 4 ]
   next

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "dbGoTop():"
   dbGoTop()
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbGoTop():"
   dbGoTop()
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   InKey( 0 )
   CLS

   ? "dbGoBottom():"
   dbGoBottom()
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( 1 ):"
   dbSkip( 1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "dbSkip( -1 ):"
   dbSkip( -1 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "dbGoto( 490 ):"
   dbGoto( 490 )
   ? "Bof Eof Found Deleted RecNo:", TESTDBF->( Bof() ), TESTDBF->( Eof() ), ;
     TESTDBF->( Found() ), TESTDBF->( Deleted() ), TESTDBF->( Deleted() ), ;
     TESTDBF->( RecNo() )
   ? "FCount:", TESTDBF->( FCount() )
   for nI := 1 to TESTDBF->( FCount() )
      ? "FieldGet( " + LTrim( Str( nI ) ) + " ):", TESTDBF->( FieldGet( nI ) )
   next

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "while !TESTDBF->( Eof() )"
   ? "   ? TESTDBF->FIRST, TESTDBF->( RecNo() )"
   ? "   TESTDBF->( dbSkip() )"
   ? "end"
   ? ""
   while !TESTDBF->( Eof() )
      ? TESTDBF->FIRST, TESTDBF->( RecNo() )
      TESTDBF->( dbSkip() )
   end

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "SET FILTER TO TESTDBF->AGE == 21"
   ? "? TESTDBF->( dbFilter() )"
   ? "TESTDBF->( dbGoTop() )"
   ? "while !TESTDBF->( Eof() )"
   ? "   ? TESTDBF->FIRST, TESTDBF->AGE, TESTDBF->( RecNo() )"
   ? "   TESTDBF->( dbSkip() )"
   ? "end"
   ? "SET FILTER TO"
   ? ""
   SET FILTER TO TESTDBF->AGE == 21
   ? TESTDBF->( dbFilter() )
   TESTDBF->( dbGoTop() )
   while !TESTDBF->( Eof() )
      ? TESTDBF->FIRST, TESTDBF->AGE, TESTDBF->( RecNo() )
      TESTDBF->( dbSkip() )
   end
   SET FILTER TO

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "TESTDBF->( Found() )"
   ? "LOCATE FOR TESTDBF->AGE == 23"
   ? "while TESTDBF->( Found() )"
   ? "   ? TESTDBF->FIRST, TESTDBF->AGE, TESTDBF->( RecNo() )"
   ? "   CONTINUE"
   ? "end"
   TESTDBF->( Found() )
   LOCATE FOR TESTDBF->AGE == 23
   while TESTDBF->( Found() )
      ? TESTDBF->FIRST, TESTDBF->AGE, TESTDBF->( RecNo() )
      CONTINUE
   end

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? "TESTDBF->( dbEval( { || QOut( TESTDBF->FIRST, TESTDBF->AGE ) }, ;"
   ? "                   { || TESTDBF->AGE == 23 } ) )"
   ? ""
   ? "dbCommit()"
   TESTDBF->( dbEval( { || QOut( TESTDBF->FIRST, TESTDBF->AGE ) }, ;
                      { || TESTDBF->AGE == 23 } ) )
   TESTDBF->( dbCommit() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? 'dbCreate( "newrdd", { { "First_Name", "C", 20, 0 }, ;'
   ? '                      { "Age",        "N",  3, 0 }, ;'
   ? '                      { "Date",       "D",  8, 0 }, ;'
   ? '                      { "Rate",       "N",  6, 2 }, ;'
   ? '                      { "Memo",       "M", 10, 0 }, ;'
   ? '                      { "Student",    "L",  1, 0 } },, .T., "newrdd" )'
   ? 'SET CENTURY ON'
   dbCreate( "newrdd", { { "First_Name", "C", 20, 0 }, ;
                         { "Age",        "N",  3, 0 }, ;
                         { "Date",       "D",  8, 0 }, ;
                         { "Rate",       "N",  6, 2 }, ;
                         { "Memo",       "M", 10, 0 }, ;
                         { "Student",    "L",  1, 0 } },, .T., "newrdd" )
   SET CENTURY ON
   ? "lUpdate:", NEWRDD->( lUpdate() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? 'Select( "TESTDBF" )'
   ? "SET FILTER TO TESTDBF->SALARY > 120000"
   ? "TESTDBF->( dbGoTop() )"
   ? "while !TESTDBF->( Eof() )"
   ? "   NEWRDD->( dbAppend() )"
   ? "   NEWRDD->FIRST_NAME := TESTDBF->FIRST"
   ? "   NEWRDD->AGE := TESTDBF->AGE"
   ? "   NEWRDD->DATE := TESTDBF->HIREDATE"
   ? "   NEWRDD->RATE := Val( Right( LTrim( Str( Seconds() ) ), 5 ) )"
   ? "   NEWRDD->MEMO := TESTDBF->FIRST + Chr( 13 ) + Chr( 10 ) + ;"
   ? "                   TESTDBF->LAST + Chr( 13 ) + Chr( 10 ) + ;"
   ? "                   TESTDBF->STREET"
   ? "   NEWRDD->STUDENT := TESTDBF->MARRIED"
   ? "   TESTDBF->( dbSkip() )"
   ? "end"
   ? "SET FILTER TO"
   ? "? NEWRDD->( RecCount() )"
   ? "NEWRDD->( dbGoTop() )"
   ? "NEWRDD->( dbRLock( 1 ) )"
   ? "NEWRDD->( dbDelete() )"
   ? "? NEWRDD->( Deleted() )"
   ? "NEWRDD->( dbGoBottom() )"
   ? "NEWRDD->( dbRLock() )"
   ? "NEWRDD->( dbDelete() )"
   ? "? NEWRDD->( Deleted() )"
   ? "NEWRDD->( dbRLock( 3 ) )"
   Select( "TESTDBF" )
   SET FILTER TO TESTDBF->SALARY > 120000
   TESTDBF->( dbGoTop() )
   while !TESTDBF->( Eof() )
      NEWRDD->( dbAppend() )
      NEWRDD->FIRST_NAME := TESTDBF->FIRST
      NEWRDD->AGE := TESTDBF->AGE
      NEWRDD->DATE := TESTDBF->HIREDATE
      NEWRDD->RATE := Val( Right( LTrim( Str( Seconds() ) ), 5 ) )
      NEWRDD->MEMO := TESTDBF->FIRST + Chr( 13 ) + Chr( 10 ) + ;
                      TESTDBF->LAST + Chr( 13 ) + Chr( 10 ) + ;
                      TESTDBF->STREET
      NEWRDD->STUDENT := TESTDBF->MARRIED
      TESTDBF->( dbSkip() )
   end
   SET FILTER TO
   ? NEWRDD->( RecCount() )
   NEWRDD->( dbGoTop() )
   NEWRDD->( dbRLock( 1 ) )
   NEWRDD->( dbDelete() )
   ? NEWRDD->( Deleted() )
   NEWRDD->( dbGoBottom() )
   NEWRDD->( dbRLock() )
   NEWRDD->( dbDelete() )
   ? NEWRDD->( Deleted() )
   NEWRDD->( dbRLock( 3 ) )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   aArray := NEWRDD->( dbRLockList() )
   ? "aArray := NEWRDD->( dbRLockList() )"
   ? "for nI := 1 to Len( aArray )"
   ? "   ? aArray[ nI ]"
   ? "next"
   ? "dbRLockList(): "
   for nI := 1 to Len( aArray )
      ? aArray[ nI ]
   next

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? 'NEWRDD->( dbCloseArea() )'
   ? 'dbUseArea( .T., TESTED_RDD, "newrdd", "NEWRDD", .F., .F. )'
   ? 'nI := 1'
   ? 'NEWRDD->( __dbPack( { || QOut( nI ), nI++ } ) )'
   ? '? "RecCount:", NEWRDD->( RecCount() )'
   ? ""
   NEWRDD->( dbCloseArea() )
   dbUseArea( .T., TESTED_RDD, "newrdd", "NEWRDD", .F., .F. )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   nI := 1
   NEWRDD->( __dbPack( { || QOut( nI ), nI++ } ) )
   ? "RecCount:", NEWRDD->( RecCount() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? '? "RecCount:", NEWRDD->( RecCount() )'
   ? 'NEWRDD->( __dbZap() )'
   ? '? "RecCount:", NEWRDD->( RecCount() )'
   ? ""
   ? "RecCount:", NEWRDD->( RecCount() )
   NEWRDD->( __dbZap() )
   ? "RecCount:", NEWRDD->( RecCount() )

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   ? 'NEWRDD->( dbCloseArea() )'
   ? 'SORT ON FIRST /DC, AGE /D TO newrdd'
   ? 'dbUseArea( .T., TESTED_RDD, "newrdd", "NEWRDD", .F., .F. )'
   ? '? "RecCount:", NEWRDD->( RecCount() )'
   ? 'for nI := 1 to 8'
   ? '   ? NEWRDD->FIRST, NEWRDD->AGE'
   ? '   NEWRDD->( dbSkip() )'
   ? 'next'
   ? '? "..."'
   ? 'NEWRDD->( dbGoBottom() )'
   ? 'NEWRDD->( dbSkip( -8 ) )'
   ? 'for nI := 1 to 8'
   ? '   ? NEWRDD->FIRST, NEWRDD->AGE'
   ? '   NEWRDD->( dbSkip() )'
   ? 'next'

   ? "Press any key to continue..."
   InKey( 0 )
   CLS

   NEWRDD->( dbCloseArea() )
   Select( "TESTDBF" )
   SORT ON FIRST /DC, AGE /D TO newrdd

   dbUseArea( .T., TESTED_RDD, "newrdd", "NEWRDD", .F., .F. )
   ? "RecCount:", NEWRDD->( RecCount() )
   for nI := 1 to 8
      ? NEWRDD->FIRST, NEWRDD->AGE
      NEWRDD->( dbSkip() )
   next
   ? "..."
   NEWRDD->( dbGoBottom() )
   NEWRDD->( dbSkip( -8 ) )
   for nI := 1 to 8
      ? NEWRDD->FIRST, NEWRDD->AGE
      NEWRDD->( dbSkip() )
   next

return nil