/*
 * $Id: sqlite3_test.prg 9457 2012-06-17 12:29:09Z andijahja $
 */

#include "sqlite3.ch"

#define TRACE
#define TABLE_SQL "CREATE TABLE t1( id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, age INTEGER )"

PROCEDURE main()

   ? sqlite3_libversion()
   sqlite3_sleep( 3000 )

   IF sqlite3_libversion_number() < 3005001
      RETURN
   ENDIF

   t1()
   t2()

   ? "press any key"

   inkey(0)

RETURN

/*
*/
PROCEDURE t1()
   LOCAL lCreateIfNotExist := .f.
   LOCAL db := sqlite3_open( "new.s3db", lCreateIfNotExist )

   IF ! Empty( db )
      sqlite3_exec( db, "DROP TABLE t1" )
   ENDIF

   sqlite3_close( db )
   db := NIL
RETURN

/*
*/
PROCEDURE t2()
   LOCAL lCreateIfNotExist := .t.
   LOCAL db := sqlite3_open( "new.s3db", lCreateIfNotExist )
   LOCAL stmt
   LOCAL nCCount, nCType, nI, nJ
   LOCAL aCType :=  { "SQLITE_INTEGER", "SQLITE_FLOAT", "SQLITE_TEXT", "SQLITE_BLOB", "SQLITE_NULL" }
   LOCAL aTable

   IF ! Empty( db )

#ifdef TRACE
      sqlite3_profile( db, .t. )
      sqlite3_trace( db, .t. )
#endif
      sqlite3_exec( db, "PRAGMA auto_vacuum=0" )
      sqlite3_exec( db, "PRAGMA page_size=4096" )

      IF sqlite3_exec( db, TABLE_SQL ) == SQLITE_OK
         ? "CREATE TABLE t1 - Ok"
      END

      sqlite3_exec( db, ;
         "BEGIN TRANSACTION;" + ;
         "INSERT INTO t1( name, age ) VALUES( 'Bob', 52 );" + ;
         "INSERT INTO t1( name, age ) VALUES( 'Fred', 40 );" + ;
         "INSERT INTO t1( name, age ) VALUES( 'Sasha', 25 );" + ;
         "INSERT INTO t1( name, age ) VALUES( 'Ivet', 28 );" + ;
         "COMMIT;" )

      ? "BEGIN TRANSACTION"
      ? "INSERT INTO t1( name, age ) VALUES( 'Bob', 52 )"
      ? "INSERT INTO t1( name, age ) VALUES( 'Fred', 40 )"
      ? "INSERT INTO t1( name, age ) VALUES( 'Sasha', 25 )"
      ? "INSERT INTO t1( name, age ) VALUES( 'Ivet', 28 )"
      ? "COMMIT"

      ? "The number of database rows that were changed: " + ltrim( str( sqlite3_changes( db ) ) )
      ? "Total changes: " + ltrim( str( sqlite3_total_changes( db ) ) )

      sqlite3_sleep( 3000 )

      stmt := sqlite3_prepare( db, "INSERT INTO t1( name, age ) VALUES( :name, :age )")
      IF ! Empty( stmt )
         IF sqlite3_bind_text( stmt, 1, "Andy" ) == SQLITE_OK .AND. ;
            sqlite3_bind_int( stmt, 2, 17 ) == SQLITE_OK
            IF sqlite3_step( stmt ) == SQLITE_DONE
               ? "INSERT INTO t1( name, age ) VALUES( 'Andy', 17 ) - Done"
            ENDIF
         ENDIF
         sqlite3_reset( stmt )

         IF sqlite3_bind_text( stmt, 1, "Mary" ) == SQLITE_OK .AND. ;
            sqlite3_bind_int( stmt, 2, 19 ) == SQLITE_OK
            IF sqlite3_step( stmt ) == SQLITE_DONE
               ? "INSERT INTO t1( name, age ) VALUES( 'Mary', 19 ) - Done"
            ENDIF
         ENDIF
         sqlite3_clear_bindings( stmt )
         sqlite3_finalize( stmt )
      ENDIF

      ? "The number of database rows that were changed: " + ltrim( str( sqlite3_changes( db ) ) )
      ? "Total changes: " + ltrim( str( sqlite3_total_changes( db ) ) )
      ? "Last _ROWID_: " + str( sqlite3_last_insert_rowid( db ) )
      ? ""

      stmt := sqlite3_prepare( db, "SELECT * FROM t1 WHERE name == :name ")
      sqlite3_bind_text( stmt, 1, "Andy" )

      ?
      ? "SELECT * FROM t1 WHERE name == 'Andy'"
      nJ := 0

      DO WHILE sqlite3_step( stmt ) == SQLITE_ROW
         nCCount := sqlite3_column_count( stmt )
         ++nJ
         ? "Record # " + str( nJ )

         IF nCCount > 0
            FOR nI := 0 TO nCCount - 1
               nCType := sqlite3_column_type( stmt, nI )
               ? "Column name : " + sqlite3_column_name( stmt, nI )
               ? "Column type : " + aCType[ nCType ]
               ? "Column value: "

            SWITCH nCType
            CASE SQLITE_BLOB
               ?? "BLOB" //sqlite3_column_blob( stmt, nI )
               EXIT

            CASE SQLITE_INTEGER
               ?? str ( sqlite3_column_int( stmt, nI ) )
               EXIT

            CASE SQLITE_NULL
               ?? "NULL"
               EXIT

            CASE SQLITE_TEXT
               ?? sqlite3_column_text( stmt, nI )
               EXIT
            END SWITCH

            NEXT nI
         ENDIF
      ENDDO
      ? "Total records - " + str( nJ )

      sqlite3_clear_bindings( stmt )
      sqlite3_finalize( stmt )

      sqlite3_sleep( 3000 )

      stmt := sqlite3_prepare( db, "SELECT * FROM t1 WHERE age >= ?5")
      sqlite3_bind_int( stmt, 5, 40 )

      ?
      ? "SELECT * FROM t1 WHERE age >= 40 "
      nJ := 0
      DO WHILE sqlite3_step( stmt ) == SQLITE_ROW
         nCCount := sqlite3_column_count( stmt )
         ++nJ
         ? "Record # " + str( nJ )

         IF nCCount > 0
            FOR nI := 1 TO nCCount
               nCType := sqlite3_column_type( stmt, nI )
               ? "Column name : " + sqlite3_column_name( stmt, nI )
               ? "Column type : " + aCType[ nCType ]
               ? "Column value: "
            SWITCH nCType
            CASE SQLITE_BLOB
               ?? "BLOB" //sqlite3_column_blob( stmt, nI )
               EXIT

            CASE SQLITE_INTEGER
               ?? str( sqlite3_column_int( stmt, nI ) )
               EXIT

            CASE SQLITE_NULL
               ?? "NULL"
               EXIT

            CASE SQLITE_TEXT
               ?? sqlite3_column_text( stmt, nI )
               EXIT
            END SWITCH

            NEXT nI
         ENDIF
      ENDDO
      ? "Total records - " + str( nJ )
      sqlite3_clear_bindings( stmt )
      sqlite3_finalize( stmt )

      sqlite3_sleep( 3000 )

      ?
      ? "SELECT id, name, age + 5 FROM t1"
      stmt := sqlite3_prepare( db, "SELECT id, name, age + 5 FROM t1")

      ? sqlite3_column_name( stmt, 1 )
      ? sqlite3_column_name( stmt, 2 )
      ? sqlite3_column_name( stmt, 3 )

      ? aCType[ sqlite3_column_type( stmt, 1 ) ]
      ? aCType[ sqlite3_column_type( stmt, 2 ) ]
      ? aCType[ sqlite3_column_type( stmt, 3 ) ]

      ? sqlite3_column_decltype( stmt, 1 )
      ? sqlite3_column_decltype( stmt, 2 )
      ? sqlite3_column_decltype( stmt, 3 )

      sqlite3_finalize( stmt )

      sqlite3_sleep( 3000 )

      ?
      ? "sqlite3_get_table"
      ?
      aTable := sqlite3_get_table( db, "SELECT name, age  FROM t1 WHERE age BETWEEN 10 AND 20" )
      FOR nI := 1 TO Len( aTable )
         FOR nJ := 1 TO Len( aTable[nI] )
            ?? aTable[nI][nJ], " "
         NEXT nJ
         ?
      NEXT nI

      sqlite3_sleep( 3000 )
   ENDIF

   IF !Empty( db )
      sqlite3_close( db )
   ENDIF		

   db := nil

RETURN
