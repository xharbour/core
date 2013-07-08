//
// $Id: arrays.prg 9279 2011-02-14 18:06:32Z druzus $
//

// Harbour multidimensional arrays support

function Main()

   local a := { 100, 200, "Third" }
   local b := Array( 10000 )  // 10.000 elements !!!

   QOut( ValType( a ) )
   QOut( ValType( { "A" } ) )

   AAdd( a, "new element" )
   QOut( Len( a ) )

   QOut( a[ 1 ] )
   QOut( a[ 2 ] )
   QOut( a[ 3 ] )
   QOut( a[ 4 ] )

   QOut( ATail( a ) )

   a[ 3 ] = { "this", { "seems", "to", { "work", "so", "well" } } }
   QOut( a[ 3 ][ 2 ][ 3 ][ 1 ] ) // "work"

   a[ 3, 2 ][ 3, 1 ] := "Harbour power!"  // different ways to specify the indexes
   QOut( a[ 3, 2, 3, 1 ] )

   QOut( ValType( b ) )
   QOut( Len( b ) )

   b[ 8000 ] = "Harbour"

   QOut( b[ 8000 ] )

   ASize( b, 2000 )
   QOut( Len( b ) )

   b[ 1000 ] = 10
   Test( b[ 1000 ]++ )
   QOut( b[ 1000 ] )

   b[ 1000 ] = 10
   Test( ++b[ 1000 ] )
   QOut( b[ 1000 ] )

   b = { 1, { 2, { 4, 5 } } }
   Test( b[ 2 ][ 2 ][ 1 ]++ )
   QOut( b[ 2 ][ 2 ][ 1 ] )

   b[ 2 ][ 2 ][ 1 ] = 2
   Test( ++b[ 2 ][ 2 ][ 1 ] )
   QOut( b[ 2 ][ 2 ][ 1 ] )

   ReleaseTest()

return nil

function Test( n )

   QOut( n )

return nil

function ReleaseTest()

   local a := { 1, 2, 3 }

return nil
