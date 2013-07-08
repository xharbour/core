//
// $Id: overload.prg 9279 2011-02-14 18:06:32Z druzus $
//

//
// DynObj
//
// Implementation of operator overload in Harbour
//
// Date : 1999/05/15
//
// Written by Eddie Runia <eddie@runia.com>
// www - http://www.harbour-project.org
//
// Jfl 2001/11/18 command mode added
//
// Placed in the public domain
//


#include "hbclass.ch"

function Main()

   local oString := TString():New( "Hello" )

   QOut( "Testing TString with Operator Overloading" )
   QOut( oString:cValue )
   QOut( "---" )

   ? ValType( oString )

   QOut( "Equal........:", oString = "Hello" )
   QOut( "Exactly Equal:", oString == "Hello" )
   QOut( "Not Equal != :", oString != "Hello" )
   QOut( "Not Equal <> :", oString <> "Hello" )
   QOut( "Not Equal #  :", oString # "Hello" )
   QOut( "Substring $  :", oString $ "Hello" )
   QOut( "Less than    :", oString < "Hello" )
   QOut( "Less than or Equal:", oString <= "Hello" )
   QOut( "Greater than :", oString < "Hello" )
   QOut( "Greater than or Equal:", oString <= "Hello" )
   QOut( "Concatenation + :", oString + "Hello" )
   QOut( "Concatenation - :", oString - "Hello" )
   QOut( "++ :", oString++, oString:cValue )
   QOut( "-- :", oString--, oString:cValue )

return nil


create class tString

      VAR cValue

      METHOD New(cText) INLINE ::cValue := cText, self

      OPERATOR "==" ARG cArg INLINE ::cValue == cArg
      OPERATOR "!=" ARG cArg INLINE ::cValue != cArg
      OPERATOR "<"  ARG cArg INLINE ::cValue <  cArg
      OPERATOR "<=" ARG cArg INLINE ::cValue <= cArg
      OPERATOR ">"  ARG cArg INLINE ::cValue >  cArg
      OPERATOR ">=" ARG cArg INLINE ::cValue >= cArg
      OPERATOR "+"  ARG cArg INLINE ::cValue +  cArg
      OPERATOR "-"  ARG cArg INLINE ::cValue -  cArg
      OPERATOR "$"  ARG cArg INLINE ::cValue $  cArg
      OPERATOR "++" INLINE ::cValue += '.', Self
      OPERATOR "--" INLINE ::cValue := Left( ::cValue, Len( ::cValue ) - 1 ), Self

endclass



/*
function TString()

   static oClass

   if oClass == nil
      oClass = HBClass():New( "TSTRING" )  // starts a new class definition

      oClass:AddData( "cValue" )          // define this class objects datas

      oClass:AddMethod( "New", @New() )

      oClass:AddInline( "==", {| self, cTest | ::cValue == cTest } )
      oClass:AddInline( "!=", {| self, cTest | ::cValue != cTest } )
      oClass:AddInline( "<" , {| self, cTest | ::cValue <  cTest } )
      oClass:AddInline( "<=", {| self, cTest | ::cValue <= cTest } )
      oClass:AddInline( ">" , {| self, cTest | ::cValue >  cTest } )
      oClass:AddInline( ">=", {| self, cTest | ::cValue >= cTest } )
      oClass:AddInline( "+" , {| self, cTest | ::cValue +  cTest } )
      oClass:AddInline( "-" , {| self, cTest | ::cValue -  cTest } )
      oClass:AddInline( "$" , {| self, cTest | ::cValue $  cTest } )

      oClass:AddInline( "HasMsg", {| self, cMsg | __ObjHasMsg( QSelf(), cMsg ) } )

      oClass:Create()                     // builds this class
   endif

return oClass:Instance()                  // builds an object of this class

static function New( cText )

   local Self := QSelf()

   ::cValue := cText

return Self

*/
