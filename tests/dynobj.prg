//
// $Id: dynobj.prg 9279 2011-02-14 18:06:32Z druzus $
//

//
// DynObj
//
// Implementation of dynamic objects in Harbour
//
// Date : 1999/05/12
//
// Written by Eddie Runia <eddie@runia.com>
// www - http://www.harbour-project.org
//
// Placed in the public domain
//

function Main()

   local oForm := TForm():New()
   local nSeq

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

/* Let's add an inline at run-time. Should already be possible */

   QOut( "Let's add inline 'CalcArea' at run-time to an already instanced class" )

   __objAddInline( oForm, "CalcArea", ;
      {|self| ( ::nRight  - ::nLeft ) * ( ::nBottom - ::nTop ) } )

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

   QOut( "What is the Form area ?" )
   QOut( oForm:CalcArea() )

   QOut( "Let's add method 'Smile' at run-time to an already instanced class" )

   __objAddMethod( oForm, "Smile", @Smile() )

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

   QOut( "Smile please " )
   oForm:Smile()

   Pause()

   QOut( "Data items before" )
   Debug( oForm )

   QOut( "Let's add an additional data item" )

   __objAddData( oForm, "cHelp" )

   oForm:cHelp := "This is a real tricky test"

   QOut( "Data items after" )
   Debug( oForm )

   Pause()

   QOut( "Let's attach a bigger smile" )

   __objModMethod( oForm, "Smile", @BigSmile() )

   QOut( "Let's smile" )
   oForm:Smile()

   QOut( "And CalcArea() will now give a result in square inches" )

   __objModInline( oForm, "CalcArea", ;
      {|self| ( ::nRight  - ::nLeft ) * ( ::nBottom - ::nTop ) / (2.54*2.54) } )

   QOut( "What is the Form area ?" )
   QOut( oForm:CalcArea() )

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

   QOut( "Delete CalcArea" )
   __objDelInline( oForm, "CalcArea" )

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

   QOut( "Delete Smile" )
   __objDelMethod( oForm, "Smile" )

   QOut( "What methods are in the class :" )
   Debug( __objGetMethodList( oForm ) )

   Pause()

   QOut( "Data items before" )
   Debug( oForm )

   QOut( "Let's delete cHelp" )

   __objDelData( oForm, "cHelp" )

   QOut( "Data items after" )
   Debug( oForm )

/*   oForm:cHelp := "Please crash" */

return nil


function TForm()

   static oClass

   if oClass == nil
      oClass = HBClass():New( "TFORM" )    // starts a new class definition

      oClass:AddData( "cText" )           // define this class objects datas
      oClass:AddData( "nTop" )
      oClass:AddData( "nLeft" )
      oClass:AddData( "nBottom" )
      oClass:AddData( "nRight" )

      oClass:AddMethod( "New",  @New() )  // define this class objects methods
      oClass:AddInline( "Show", {|self| ::cText } )

      oClass:Create()                     // builds this class
   endif

return oClass:Instance()                  // builds an object of this class


static function New()

   local Self := QSelf()

   ::nTop    := 10
   ::nLeft   := 10
   ::nBottom := 20
   ::nRight  := 40

return Self


static function Smile()

   local self := QSelf()

   if ::CalcArea() == 300
      QOut( ":-)" )
   else
      QOut( ":-(" )
   endif
return self


static function BigSmile()

   local self := QSelf()

   QOut( ":-)))" )
return self


function Pause()

   __Accept( "Pause :" )
return nil



