//
// $Id: debugtst.prg 9279 2011-02-14 18:06:32Z druzus $
//

/*
 * $Doc$
 * $Description$  Debug function tests.
 *                Based on classes.prg
 * $Requirement$  source\tools\stringp.prg
 *                source\rtl\objfunc.prg
 *                source\rtl\asort.prg
 * $Date: 2011-02-14 16:06:32 -0200 (seg, 14 fev 2011) $
 * $End$
 *
 * Written by Eddie Runia <eddie@runia.com>
 * www - http://www.harbour-project.org
 *
 * Placed in the public domain
 */

function Main()

   local oForm   := TForm():New()
   local nNumber := 15

   QOut( oForm:ClassName() )
   oForm:Show()
   QOut()

   QOut( "-OBJECT additions-" )
   QOut( "What is in oForm ? " )
   Debug( oForm:Transfer() )

   QOut( "Does transfer exists ? ", __objHasMsg   ( oForm, "Transfer" ) )
   QOut( "Is   transfer DATA   ? ", __objHasData  ( oForm, "Transfer" ) )
   QOut( "Is   transfer METHOD ? ", __objHasMethod( oForm, "Transfer" ) )
   QOut( "Does nLeft    exists ? ", __objHasMsg   ( oForm, "nLeft"    ) )
   QOut( "Is   nLeft    DATA   ? ", __objHasData  ( oForm, "nLeft"    ) )
   QOut( "Is   nLeft    METHOD ? ", __objHasMethod( oForm, "nLeft"    ) )
   QOut( "Does unknown  exists ? ", __objHasMsg   ( oForm, "Unknown"  ) )
   QOut( "Is   unknown  DATA   ? ", __objHasData  ( oForm, "Unknown"  ) )
   QOut( "Is   unknown  METHOD ? ", __objHasMethod( oForm, "Unknown"  ) )

   QOut( "Set nLeft to 50 and nRight to 100" )
   oForm:Transfer( {"nLeft", 50}, {"nRight", 100} )
   Debug( oForm:Transfer() )

   Pause()


   QOut( "-DEBUG Functions-")
   QOut( "-Statics-" )
   Debug( hb_dbg_vmVarSList() )

   QOut( "-Global Stack-" )
   Debug ( hb_dbg_vmStkGList() )

   QOut( "-Local Stack-" )
   Debug ( hb_dbg_vmStkLList() )

   QOut( "-Parameters-" )
   Debug ( hb_dbg_vmParLList() )

   Pause()

   FuncSecond( 241, "Hello" )

return nil


function Pause()
return __Accept("")


function FuncSecond( nParam, cParam, uParam )

   local cWhat   := "Something"
   local nNumber := 2
   local xParam
   local xStack

   QOut()
   QOut( "-Second procedure-")
   QOut()

   QOut( "-Statics-" )
   Debug ( hb_dbg_vmVarSList() )
   QOut()

   QOut( "-Global Stack- Len=", hb_dbg_vmStkGCount() )
   Debug ( hb_dbg_vmStkGList() )
   QOut()

   QOut( "-Local Stack- Len=", hb_dbg_vmStkLCount() )
   xStack := Debug ( hb_dbg_vmStkLList() )
   QOut()

   QOut( "-Parameters-" )
   xParam := Debug( hb_dbg_vmParLList() )
   if xParam[ xStack[ 7 ] ] == "Hello"
      QOut( ":-)" )
   endif

   Pause()

return nil


/* $Doc$
 * $FuncName$     <oForm> TForm()
 * $Description$  Returns TForm object
 * $End$ */
function TForm()

   static oClass

   if oClass == nil
      oClass = HBClass():New( "TFORM" )    // starts a new class definition

      oClass:AddData( "cName" )           // define this class objects datas
      oClass:AddData( "nTop" )
      oClass:AddData( "nLeft" )
      oClass:AddData( "nBottom" )
      oClass:AddData( "nRight" )

      oClass:AddVirtual( "aExcept" )      // Export exceptions

      oClass:AddMethod( "New",  @New() )  // define this class objects methods
      oClass:AddMethod( "Show", @Show() )
      oClass:AddMethod( "Transfer", @Transfer() )

      oClass:Create()                     // builds this class
   endif

return oClass:Instance()                  // builds an object of this class


/* $Doc$
 * $FuncName$     <oForm> TForm:New()
 * $Description$  Constructor
 * $End$ */
static function New()

   local Self := QSelf()

   ::nTop    = 10
   ::nLeft   = 10
   ::nBottom = 20
   ::nRight  = 40

return Self


/* $Doc$
 * $FuncName$     TForm:Show()
 * $Description$  Show a form
 * $End$ */
static function Show()

   local Self := QSelf()

   QOut( "lets show a form from here :-)" )

return nil


//
// <xRet> TForm:Transfer( [<xArg,..>] )
//
// Generic object import and export function
//
// <xArg> is present.
//
// Maximum number of arguments passed is limited to 10 !
//
// An argument can be one of the following :
//
// { <cSymbol>, <xValue> }              Set DATA <cSymbol> to <xValue>
// { { <cSym1>, <xVal1> }, { <cSym2>, <xVal2> }, ... }
//                                      Set a whole list symbols to value
//                                      Normal way of set objects from external
//                                      sources, like memo files.
// <oObject>                            Set self according to the DATA
//                                      contained in <oObject>
//                                      Can be used to transfer info from
//                                      one class to another
//
// If <xArg> is not present, the current object will be returned as an array
// for description see __objSetValueList / __objGetValueList.
//
// The method aExcept() is called to determine the DATA which should not
// be returned. Eg. hWnd ( do not copy this DATA from external source )
//
// Say we want to copy oSource into oTarget we say :
//
// oTarget:Transfer( oSource )
//
// If we do not want 'cName' duplicated we have to use __objGetValueList :
//
// aNewExcept := aClone( oSource:aExcept() )
// aAdd( aNewExcept, "cName" )  /* Add cName to exception list               */
// oTarget:Transfer( __objGetValueList( oSource, aNewExcept ) )
//                              /* Get DATA from oSource with new exceptions */
//                              /* Transfer DATA to oTarget                  */
//
// To set two DATA of oTarget :
//
// oTarget:Transfer( { "nLeft", 10 }, { "nRight", 5 } )
//
// or :
//
// aCollect := {}
// aAdd( aCollect, { "nLeft" , 10 } )
// aAdd( aCollect, { "nRight", 5  } )
// oTarget:Transfer( aCollect )
//
// Copy oSource to a memo field :
//
// DbObject->Memo := oSource:Transfer()
//
// (Re)create oTarget from the memo field :
//
// oTarget := TTarget():New()
// oTarget:Transfer( DbObject->Memo )
//
static function Transfer( x1,x2,x3,x4,x5,x6,x7,x8,x9,x10 /* etc */ )

   local self   := QSelf()
   local aParam := hb_dbg_vmParLList()
   local nLen   := PCount()
   local xRet
   local xData
   local n

   if nLen == 0
      xRet := __objGetValueList( self, ::aExcept() )
   else
      for n := 1 to nLen

         xData := aParam[ n ]
         if ValType( xData ) == "A"

            if ValType( xData[1] ) == "A"       // 2D array passed
               xRet := __objSetValueList( self, xData )
            else                                // 1D array passed
               xRet := __objSetValueList( self, {xData} )
            endif

         elseif ValType( xData ) == "O"         // Object passed
            xRet := ::Transfer( xData:Transfer() )
         elseif ValType( xData ) != "U"
            QOut( "TRANSFER: Incorrect argument(", n, ") ", xData )
         endif

      next n
   endif

return xRet



