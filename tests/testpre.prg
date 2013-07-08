/*
 * $Id: testpre.prg 9279 2011-02-14 18:06:32Z druzus $
 */

FUNCTION Main()

   LOCAL cString
   LOCAL i, j, aScript

   CLS

   qOut( "Testing Harbour run-time preprocessing" )
   qOut( "======================================" )
   qOut( "" )

   cString := "@ 10, 10 SAY 'Hello!'"
   qOut( cString )
   qOut( __Preprocess( cString ) )
   qOut( "" )

   cString := "? 'Hello mom'"
   qOut( cString )
   qOut( __Preprocess( cString ) )
   qOut( "" )

   cString := 'SET RELATION TO Something INTO MySelf'
   qOut( cString )
   qOut( __Preprocess( cString ) )
   qOut( "" )

   cString := 'SET RELATION ADDITIVE TO Something INTO YourSelf'
   qOut( cString )
   qOut( __Preprocess( cString ) )
   qOut( "" )

   cString := "#xcommand DEFAULT <v1> := <x1> => IF <v1> == NIL ; <v1> := <x1> ; END"
   qOut( cString )
   IF __ppAddRule( cString )
      qOut( "Rule added successfully !" )
   ELSE
      qOut( "Rule addition failed ..." )
   ENDIF

   cString := 'DEFAULT x := 100'
   qOut( cString )
   qOut( __Preprocess( cString ) )
   qOut( "" )

   qOut( "Press <Enter>..." )
   __Accept( "" )

   CLS

   aScript := { 'cMyDatabase := "DONTKNOW.DBF"', ;
                'USE (cMyDatabase)', ;
                'GO TOP', ;
                '', ;
                '? MYFIELD, YOURFIELD', ;
                '', ;
                'WAIT "Press <Enter> key..."', ;
                '', ;
                'CLOSE ALL' }

   FOR j := 1 TO 2
      qOut( if( j = 1, "Before", "After" ) + " __Preprocess()" )
      qOut( "===================" )
      qOut( "" )
      FOR i := 1 TO len( aScript )

         ? if( j = 1, aScript[i], __Preprocess( aScript[i] ) )

      NEXT
      qOut( "" )
      qOut( "Press <Enter> key..." )
      __Accept( "" )
      CLS
   NEXT

   RETURN( NIL )

Exit PROCEDURE ExitTest
   __PP_Free()
Return