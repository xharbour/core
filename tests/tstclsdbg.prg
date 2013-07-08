**********************************************
* Class debug demo.
*
* This program demonstrates how to debug a class using
* __SetClassScope(), __objGetMsgFullList() and __objGetValueFullList()
* functions.
*
* On Windows best viewed with DebugView (from www.sysinternals.com)
*
* (C) 2003 Francesco Saverio Giudice
*
* $Id: tstclsdbg.prg 9279 2011-02-14 18:06:32Z druzus $
*

#include "common.ch"
#include "hbclass.ch"

#xcommand IF <x> THEN <*y*> => IF <x> ;; <y> ;; END

PROCEDURE Main( cNoWait )

  LOCAL a := Hash()
  LOCAL b := Hash()
  LOCAL h := Hash()
  LOCAL t
  LOCAL lWait := TRUE

  CLEAR SCREEN

  IF cNoWait <> NIL .AND. Upper( cNoWait ) == "/N"
     lWait := FALSE
  ENDIF

  t := Test( 100 )  // Declaring t as Test Object autoinizializing with a value.
                    // Look at CONSTRUCTOR in class definition

  //---------------------------

  a['Test1'] := Date()
  a['TEST2'] := "Francesco"
  a['Test3'] := 10

  //---------------------------

  b:Var1 := 10             // This will be stored in UPPER case
  b:Var2 := .t.            // This will be stored in UPPER case
  b:SendKey( "Var3", 20 )  // This will be stored as sent
  b['VaR4'] := { "This", "is", "an", "array", ;  // As above
                 "of", 11, "items", Date(), .T., { "Array", "of", 4, "items" }, {"AssocArray" => .t. } }
  b['Var5'] := ErrorNew()  // An object
  b['var6'] := {}          // An empty array
  b['var7'] := { "Test 1" => "an associative array nested" } // Another associative array
  b['var8'] := { "Test 2" => "an hash nested" }

  //---------------------------

  h['Hash1'] := "Giudice"
  h[1]       := "Frank"
  h[Date()]  := { "10" => "a hash of hash" }
  h['']      := "Hash with null key (empty string)"
  h[2]       := { => }   // another hash of an empty hash

  //---------------------------

  t:Add( "x"  , 20 )
  t:Add( "one", { "This", "is", "an", "array", "of", 11, "items", Date(), .T., { "Array", "of", 4, "items" }, {"AssocArray" => .t. } } )
  t:Add( "two", 40 )

  //---------------------------

  //? a['Test2']       // Uncomment this to get error because TEST2 is declared in UPPER case

  //---------------------------

  Write( "Display value of: a" )
  Write( HB_DumpVar( a ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display keys of var a" )
  Write( HB_DumpVar( a:Keys ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of var b" )
  Write( HB_DumpVar( b ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of var b recursively" )
  Write( HB_DumpVar( b,, TRUE ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of var h" )
  Write( HB_DumpVar( h ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of var h recursively" )
  Write( HB_DumpVar( h,, TRUE ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of var t that is an OBJECT" )
  Write( HB_DumpVar( t ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display value of: t:var3" )
  t:Dump()
  Write( "" )
  IF lWait THEN Write( "Press any key to continue" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "Display the object HASH itself" )
  Write( HB_DumpVar( a, TRUE ) )
  Write( "" )
  IF lWait THEN Write( "Press any key to end" )
  IF lWait THEN Inkey( 0 )
  Write( "" )
  Write( "End of test" )

RETURN

//-----------------------------------------------------------------------------------//

CLASS TestParent
   METHOD MyConstructorParent CONSTRUCTOR
ENDCLASS

METHOD MyConstructorParent() CLASS TestParent
  // Notinhg to do
RETURN Self

CLASS Test FROM TestParent

   DATA nVar1 INIT 1
   DATA cVar2 INIT "Test"

   METHOD Test()
   CONSTRUCTOR Test() // Here we can have any method name.
                      // CONSTRUCTOR Test() syntax is equal to METHOD Test() CONSTRUCTOR
   METHOD Dump()
   METHOD Add( cKey, xVal )  INLINE ::aVar3:SendKey( cKey, xVal )

 PUBLISHED:
   DATA bVar4 INIT {|| TRUE }

   METHOD MyPublishedMethod()   VIRTUAL

 PROTECTED:
   DATA aVar5 INIT { 1, 2, "3" }

   METHOD MyProtectedMethod()

 HIDDEN:
   DATA lVar6 INIT FALSE
   DATA nVar7
   DATA aVar3 INIT Hash()

   METHOD MyHiddenMethod()      INLINE Self  // On dump look at inline method type

ENDCLASS

METHOD Test( nInt ) CLASS Test
   ::aVar3 := Hash()
   ::nVar7 := nInt
RETURN Self

METHOD Dump() CLASS Test
   Write( HB_DumpVar( ::aVar3 ) )
RETURN Self

METHOD MyProtectedMethod() CLASS Test
RETURN Self



#define CRLF() (CHR(13)+CHR(10))

// ********************************************* ------------------------------- *************************

STATIC PROCEDURE Write( x )
  x += CRLF()
  OutStd( x )
  HB_OutDebug( x )
RETURN
