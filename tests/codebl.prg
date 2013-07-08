//
// $Id: codebl.prg 9279 2011-02-14 18:06:32Z druzus $
//

STATIC cbStatic

Function Main()
Local a := TestBlocks()
LOCAL cb

   qout( eval( a[ 1 ] ) )      // 23
   qout( eval( a[ 2 ], 42 ) )  // 42
   qout( eval( a[ 1 ] ) )      // 42
   qout( eval( a[ 2 ], 15 ) )  // 15

   mqout( 15, eval( a[ 1 ] ) )      // 15 15
   mqout( 14, eval( a[ 1 ] ) )      // 14 15
   mqout( 42, eval( a[ 2 ], 42 ) )  // 42 42
   mqout( 14, eval( a[ 2 ], 42 ) )  // 14 42
   mqout( 42, eval( a[ 1 ] ) )      // 42 42
   mqout( 14, eval( a[ 1 ] ) )      // 14 42

   GetArray( @a )
   PrintArray( @a )

   qout( "Test for variables passed by reference in a codeblock" )
   DetachWithRefer()

   qout( "Test for indirect detaching of local variables" )
   DetachToStatic( 1 )
   mqout( 2, EVAL( cbStatic, 1 ) )
   mqout( 3, EVAL( cbStatic, 2 ) )
   cb :=cbStatic
   DetachToStatic( 100 )
   mqout( 200, EVAL( cbStatic, 100 ) )
   mqout( 300, EVAL( cbStatic, 200 ) )
   mqout( 4, EVAL( cb, 3 ) )

Return( NIL )

Static Function TestBlocks()
LOCAL nFoo := 23
Return( { {|| nFoo }, {|n| nFoo := n } } )

Static Function mqout( nExpected, nGot )

   qout( nExpected, nGot )

Return( NIL )

/////////////////////////////////////////////////////////////////
PROCEDURE GetArray( a )
LOCAL i

   a :=ARRAY( 100 )
   FOR i:=1 TO 100
     IF( (i % 6) == 0 )
         a[ i-2 ] =NIL
         a[ i-4 ] =NIL
     ENDIF
     a[ i ] := TestBlocks()
   NEXT

RETURN

PROCEDURE PrintArray( a )
LOCAL i

   FOR i:=1 TO 100
     IF( a[i] != NIL )
       EVAL( a[ i ][ 2 ], i )
       mqout( i, EVAL( a[ i ][ 1 ] ) )
     ENDIF
   NEXT

RETURN

//////////////////////////////////////////////////////////////////
Function DetachWithRefer()
Local nTest
Local bBlock1 := MakeBlock()
Local bBlock2 := {|| DoThing( @nTest ), qout( nTest ) }

   eval( bBlock1 )
   eval( bBlock2 )

Return( NIL )

Function MakeBlock()
Local nTest
RETURN( {|| DoThing( @nTest ), qout( nTest ) } )

Function DoThing( n )

   n := 42

Return( NIL )

//////////////////////////////////////////////////////////////////////
FUNCTION DetachToStatic( n )

  cbStatic ={|x| n+x}

RETURN NIL
