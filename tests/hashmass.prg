*****************************************************
* Hash massive ( stress ) test
*
* Giancarlo Niccolai (C) 2003
*
* This test demonstrates limits and advantages of
* hashes.
*
* $Id: hashmass.prg 9279 2011-02-14 18:06:32Z druzus $
*

PROCEDURE Main()
   LOCAL i, hFile
   LOCAL nSecs
   LOCAL nRow := 1
   LOCAL hHash

   SET COLOR TO w+/b
   Clear Screen
   @nRow++,0 SAY PadC(" --- HASH MASSIVE tests ---", MaxCol() )
   @nRow++,0 SAY PadC(" Giancarlo Niccolai ", MaxCol() )

   @nRow++,5 SAY "Insertion in a non-optimized hash"
   hHash := {=>}
   nSecs := Seconds()
   FOR i := 1 TO 10000
      IF I % 500 == 0
         @nRow,5 say Str(i)
         // Test for GC usage.
         HB_GcAll(.T.)
      ENDIF
      HSet( hHash, RandStr(5), i )
   NEXT
   @nRow++,5 SAY "HASH Inserting " + Str( Seconds() - nSecs )

   * Doing the same but with preallocation

   hHash['n'] := "an element"
   @nRow++,5 SAY "Data Linear retrival"
   nSecs := Seconds()
   FOR i := 1 TO 1000000
      IF I % 5000 == 0
         @nRow,5 say Str(i)
         // Test for GC usage.
         HB_GcAll(.T.)
      ENDIF
      IF HGet( hHash, 'n' ) != "an element"
         @nRow++,5 SAY "    ERROR!   "
         @nRow++,5 SAY ValToPrg( HGet( hHash, 'n' ) )
         EXIT
      ENDIF
   NEXT
   @nRow++,5 SAY "HASH linear Retreiving " + Str( Seconds() - nSecs )


   @nRow++,5 SAY "Insertion With Partition level 1, 5000 pages per partition"
   hHash := Hash()  // Alternative grammar
   HSetPartition( hHash, 5000 )

   nSecs := Seconds()
   FOR i := 1 TO 100000
      IF I % 500 == 0
         @nRow,5 say Str(i)
         HB_GcAll(.T.)
      ENDIF
      HSet( hHash, RandStr(5), i )
   NEXT
   @nRow++,5 SAY "HASH Partitioned Inserting " + Str( Seconds() - nSecs )

   @nRow++,5 SAY "Data Partitioned retrival"
   hHash['n'] := "an element"
   nSecs := Seconds()
   FOR i := 1 TO 1000000
      IF I % 5000 == 0
         @nRow,5 say Str(i)
         // Test for GC usage.
         HB_GcAll(.T.)
      ENDIF
      IF HGet( hHash, 'n' ) != "an element"
         @nRow++,5 SAY "    ERROR!   "
         @nRow++,5 SAY ValToPrg( HGet( hHash, 'n' ) )
         EXIT
      ENDIF
   NEXT
   @nRow++,5 SAY "HASH Partitioned Retreiving " + Str( Seconds() - nSecs )


   @nRow++,5 SAY "Insertion With Partition level 1, 500 pages per partition"
   hHash := Hash()  // Alternative grammar
   HSetPartition( hHash, 500 )

   nSecs := Seconds()
   FOR i := 1 TO 100000
      IF I % 1000 == 0
         @nRow,5 say Str(i)
         HB_GcAll(.T.)
      ENDIF
      HSet( hHash, RandStr(5), i )
   NEXT
   @nRow++,5 SAY "HASH Partitioned Inserting " + Str( Seconds() - nSecs )

   @nRow++,5 SAY "Data Partitioned retrival"
   hHash['n'] := "an element"
   nSecs := Seconds()
   FOR i := 1 TO 1000000
      IF I % 5000 == 0
         @nRow,5 say Str(i)
         // Test for GC usage.
         HB_GcAll(.T.)
      ENDIF
      IF HGet( hHash, 'n' ) != "an element"
         @nRow++,5 SAY "    ERROR!   "
         @nRow++,5 SAY ValToPrg( HGet( hHash, 'n' ) )
         EXIT
      ENDIF
   NEXT
   @nRow++,5 SAY "HASH Partitioned Retreiving " + Str( Seconds() - nSecs )


   @nRow++,5 SAY "Insertion With Partition level 2, 200 pages per partition"
   hHash := Hash()  // Alternative grammar
   HSetPartition( hHash, 200, 2 )

   nSecs := Seconds()
   FOR i := 1 TO 100000
      IF I % 5000 == 0
         @nRow,5 say Str(i)
         HB_GcAll(.T.)
      ENDIF
      HSet( hHash, RandStr(5), i )
   NEXT
   @nRow++,5 SAY "HASH Partitioned Inserting " + Str( Seconds() - nSecs )

   @nRow++,5 SAY "Data Partitioned retrival"
   hHash['n'] := "an element"
   nSecs := Seconds()
   FOR i := 1 TO 1000000
      IF I % 5000 == 0
         @nRow,5 say Str(i)
         // Test for GC usage.
         HB_GcAll(.T.)
      ENDIF
      IF HGet( hHash, 'n' ) != "an element"
         @nRow++,5 SAY "    ERROR!   "
         @nRow++,5 SAY ValToPrg( HGet( hHash, 'n' ) )
         EXIT
      ENDIF
   NEXT
   @nRow++,5 SAY "HASH Partitioned Retreiving " + Str( Seconds() - nSecs )

   @nRow++,5 SAY "    Done   "
   @nRow+2,0

RETURN

FUNCTION RandStr( nCount )
   LOCAL cVal := ""

   WHILE Len( cVal ) < nCount
      cVal += Chr( Asc('a') + HB_RandomInt( 0, 25 ) )
   ENDDO

RETURN cVal