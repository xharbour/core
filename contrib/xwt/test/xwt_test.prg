************************************************************
* XWT_TEST. 
*
* Giancarlo Niccolai et al.:
* (PLZ. add your copyright...)
*
* $Id: xwt_test.prg 9279 2011-02-14 18:06:32Z druzus $
*


#include "xwt.ch"
#include "xwtcmd.ch"

GLOBAL oFileSel
GLOBAL oOtherBox

#xcommand ? <data,...> => OutStd( <data>, HB_OSNewLine() )

PROCEDURE MAIN()
   LOCAL oWindow, oButton
   LOCAL oTextbox, oLabel, oViewPort, oPane
   LOCAL oImg, oHlay, oVLay, oVlay2, oFrame, oSplit
   LOCAL oGrid
   Local oCalen
   LOCAL oList, oCheck, aInputs, oInput, oRadioPanel,oCombo
   Local cData,cCombo
   Local aItems := {"Luiz","Ron","Gian","Marcelo","Patrick"}
   Local oNoteBook
   Local oEditBox

   XwtInit()

   /*** Window creation ****/
   DEFINE WINDOW oWindow TITLE "New FrameWindow from XWT" MENU BuildMenu()
   oWindow:AddEventListener( XWT_E_DESTROY_REQ, @XwtQuit() )


   /*** Splitter panes layout( our new main widget) ***/
   DEFINE LAYOUT oVLay  MODE XWT_LM_VERT PADDING 5 BORDER 2
   DEFINE LAYOUT oVLay2 MODE XWT_LM_VERT PADDING 5 BORDER 2

   DEFINE SPLITTER oSplit MODE XWT_LM_HORIZ FIRSTWIDGET oVLay SECONDWIDGET oVLay2 OF oWindow

   oSplit:SetShrinkFirst( .F. )


   // inside an horiz. layout
   DEFINE LAYOUT oHLay  MODE XWT_LM_HORIZ

   /*** Text And Button ***/
   DEFINE LABEL oLabel TEXT "Text: " FONT "Courier" COLOR "#44FD56" OF oHlay

   oHLay:SetFill( .T. )
   oHLay:SetExpand( .T. )

   DEFINE BUTTON oButton TEXT "Hello" OF oHLay FONT "Courier"  size 32 COLOR "#90CF02"  BGCOLOR "#A38103"  TEXTCOLOR "#DD0000"
   oButton:AddListener( @DumpEvent() )

   oHLay:SetBox( .T.,"Horiz Box" )

   oVlay:Add( oHLay )

   /* A couple of Textboxes in a pane, inside a scrollable */
   DEFINE PANE oPane BOX TITLE "A Fixed Pane" OF oVLay

   @ 10,10 TEXTBOX oTextBox VAR "A Text" OF oPane 
   oTextbox:AddEventListener(XWT_E_UPDATED, @BoxModified())

   @ 10,40  TEXTBOX oOtherBox VAR "Another box" OF oPane
   @ 10,75  CHECKBOX        TEXT "CkBox 1" VAR .T. OF oPane FONT "Sans" COLOR "#09FC20"  BGCOLOR "#3A1803"  TEXTCOLOR "#112233" SIZE 5
   @ 110,75 CHECKBOX oCheck TEXT "CkBox 2" VAR .F. OF oPane FONT "Courier" COLOR "#222222"  BGCOLOR "#A38103"  TEXTCOLOR "#FF3400" SIZE 20
   @ 010,105 TOGGLEBUTTON   TEXT "Button Toggle 1" VAR .T. OF oPane FONT "Clean" COLOR "#334455"  BGCOLOR "#66CFAA"  TEXTCOLOR "#FF0000" SIZE 15
   @ 120,105 TOGGLEBUTTON   TEXT "Button Toggle 2" VAR .F. OF oPane FONT "Helvetica" COLOR "#556677"  BGCOLOR "#44FFCC"  TEXTCOLOR "#DD0000" SIZE 12

   // add a button to query the status.
   @10,135 BUTTON oButton TEXT "Click to Query pane status"  OF oPane FONT "Helvetica" COLOR "#556677"  BGCOLOR "#44FFCC"  TEXTCOLOR "#DD0000"
   oButton:AddEventListener( XWT_E_CLICKED, @PaneStatus() )


   /* A beautiful GRID */

   oGrid := XwtGrid():New(2,4)

   DEFINE LABEL oLabel TEXT "Field label"  FONT "Clean" SIZE 20

   oGrid:setPadding( 2, 10 )
   oGrid:SetFill( .T. )

   oGrid:Attach( oLabel, 1, 2 )
   oGrid:Attach( XwtLabel():New("Field 2",,,,"Helvetica",5,"#AC4F3C"), 2, 2 )

   oGrid:SetExpand( .T. )
   oGrid:Attach( XwtTextBox():New("Data 1"), 1, 3 )
   oGrid:Attach( XwtTextBox():New("Data 2"), 2, 3 )

   oGrid:SetBox( .T.,"A Grid " )

   oVLay2:Add( oGrid )

   /*** IMAGE ***/
   DEFINE IMAGE oImg FILE "test\icon.png" // OF oVLay2
   //oImg:SetSensible()  // TODO: ANTES que oParent:Add( Self ), sino warning de GTK+

   oNotebook := XWTNoteBook():New(,, oVLay2)

   DEFINE LABEL oLabel TEXT "Page 1: " FONT "Courier" COLOR "#44FD56" 

   oNotebook:AddPage( oLabel )

   oNotebook:AddPage( oImg )

   oEditbox := XWTEditBox():New(,, oVLay2)

   /*** An input mask ***/
   aInputs := {;
      { "Variable 1", "Default value"}, ;
      { "Variable 2", "Default value 2"}, ;
      { "Variable 3", "Default value 3"} ;
   }
   oInput := XWTInputMask():New( aInputs )
   oInput:AddEventListener( XWT_E_CHANGED, @InputChanged() )

   oFrame := XwtLayout():New( XWT_LM_VERT )
   oViewPort := XwtViewPort():new(75, 50)
   oFrame:add( oViewPort )
   oViewPort:SetContent( oInput )
   oFrame:SetBox( .T., "Inside a scrollable..." )
   oVLay2:Add( oFrame )


   /***  Radio buttons ***/
   oRadioPanel := XWTLayout():New( XWT_LM_VERT )
   oRadioPanel:add( XWTRadioButton():New( "Option 1" ,"Sans",5,"#111110","#A38103",,"#A38103") )
   oRadioPanel:add( XWTRadioButton():New( "Option 2" ,"Sans",6,"#A38103","#456789",,"#A38103") )
   oRadioPanel:add( XWTRadioButton():New( "Option 3" ,"Sans",7,"#111210","#222222","#A38103","#A38103") )

   oVLay2:Add( oRadioPanel )
   @ 50,2 COMBO oCombo VAR cCombo of oVlay2 items aItems 
      oCombo:AddEventListener(XWT_E_UPDATED, @ComboModified())
      
   @ 60,2 LISTBOX oCombo VAR cCombo of oVlay2 items aItems 
      oCombo:AddEventListener(XWT_E_UPDATED, @ComboModified())
      
      
      
//   oVlay2:add(oCombo)
   
   /***** A list **********/
   oList := XWTTreeList():New(;
         { {"uno", 2, 3.12, "fir"}, ;
           {"dos",3,4,5} },;  //the table
         { "C1", "C2", "C3", "C4" } ; // the titles
         )
   oList:SetColumnEditable( 0 )
   oVLay:Add( oList )
   //oCalen := XWTCalendar():New()
   @  200,50 CALENDAR oCalen VAR cData 
      oCalen:AddEventListener(XWT_E_UPDATED, @BoxModified1())
   ovlay:add(oCalen)
   /*** Showing window ***/
   oWindow:Resize( 200, 200 )
   oWindow:Show()
   oWindow:AddListener( @WindowReceive() )

   /*** Main LOOP ***/
   XwtMainLoop()

   /*** Going to terminate */
   //oButton:Destroy()  // the button might or might not be deteached from window
   oWindow:Destroy()

RETURN



FUNCTION WindowReceive( oEvent )
?  "Received event at top level ", oEvent:nType, " from ", oEvent:oSender:GetText()
RETURN .F.

FUNCTION DumpEvent( oEvent )
?  "Event type: ", oEvent:nType, " from ", oEvent:oSender:GetText()
?   Len ( oEvent:aParams )
   IF Len ( oEvent:aParams ) == 1
?  "Parameter ", oEvent:aParams[1]
   ELSE
      IF Len( oEvent:aParams ) == 2
?  "Parameters ", oEvent:aParams[1], " ", oEvent:aParams[2]
      ENDIF
   ENDIF
   IF oEvent:nType == XWT_E_CLICKED
      IF XWT_MsgBox( "Are you really, really, sure?", ;
         XWT_MSGBOX_YES + XWT_MSGBOX_NO, XWT_MSGBOX_QUESTION) == XWT_MSGBOX_YES
            oEvent:oSender:Hide()
      ENDIF
   ENDIF
RETURN .F.

FUNCTION FileEvent( oEvent )
   //Filesel can't be local!
   //Local oFileSel
   Local cFileName
   local cDate := ""
   
?  "Menu activated: ", oEvent:oSender:nId
   IF oEvent:oSender:nId == 1
      oFileSel := XWTFileSel():New( "Open file" )
      oFileSel:SetFile( "test.file" )

      // Notice: after a "do_modal", the object will not exist anymore
      cFileName := oFileSel:DoModal()
      IF cFileName == ""
?  "Canceled!"
      ELSE
?  "FILENAME: ", cFileName
      ENDIF
   elseIF oEvent:oSender:nId == 2
      oFileSel := XWTFontSel():New( "Open file" )
      oFileSel:SetFile( "Select an Font" )

      // Notice: after a "do_modal", the object will not exist anymore
      cFileName := oFileSel:DoModal()
      IF cFileName == ""
?  "Canceled!"
      ELSE
?  "Font NAME: ", cFileName
      ENDIF      
   elseIF oEvent:oSender:nId == 3
      oFileSel := XWTCalendar():New( "Open file" ,,,,.T.,cFileName)
//      oFileSel:SetFile( "Select an Font" )

      // Notice: after a "do_modal", the object will not exist anymore
      cFileName := oFileSel:DoModal()
//      IF cFileName == ""
//?  "Canceled!"
//      ELSE
?  "Date: ", cFileName
//      ENDIF      
      
      
   ELSEIF oEvent:oSender:nId == 99
      Quit
   ENDIF
   
RETURN .F.

FUNCTION BoxModified( oEvent )
tracelog('Em BoxModified( oEvent )')
tracelog( oEvent:oSender:getText())

?  "Text entered in box: ", oEvent:oSender:getText()
   oEvent:oSender:SetText( "Reset" )
   oOtherBox:SetFocus()
RETURN .F.

FUNCTION ComboModified( oEvent )
tracelog('Em ComboModified( oEvent )')
tracelog( oEvent:oSender:getItem())
?  "Text entered in box: ", oEvent:oSender:getItem()
//   oEvent:oSender:SetText( "Reset" )
   oOtherBox:SetFocus()
RETURN .F.

FUNCTION BoxModified1( oEvent )
?  "Text entered in box: ", oEvent:oSender:getdate()
//   oEvent:oSender:SetText( "Reset" )
//   oOtherBox:SetFocus()
RETURN .F.


FUNCTION InputChanged( oEvent )
   Local aField
?  "Input has been set:"

   FOR EACH aField IN oEvent:oSender:aInputFields
?  aField[1], ": ", aField[2]
   NEXT

?  ""
RETURN .F.


FUNCTION PaneStatus( oEvent )
   LOCAL oObj

   ? "Clicked QUERY PANE STATUS Button"
   FOR EACH oObj in oEvent:oSender:oOwner:aChildren
      IF oObj:GetType() == XWT_TYPE_CHECKBOX .or. oObj:GetType() == XWT_TYPE_TOGGLEBUTTON
         ? oObj:GetText(), IIF( oObj:GetStatus(), "(Active)", "(NOT Active)" )
      ELSE
         ? oObj:GetText()
      ENDIF
   NEXT
   ? ""
RETURN .T.

/***************************************************************/
// Example Contructor the design menus
/***************************************************************/
FUNCTION BuildMenu()
   Local oMenu,oMenuItem,oMenuSec,oMenuHelp

   MENU oMenu PROMPT "_File"
        MENUITEM oMenuItem PROMPT "Op_en" ICON "valley.png" ACTION @FileEvent() OF oMenu FONT "Clean" Size 20 Color "#44DE5F"
        MENUITEM oMenuItem PROMPT "Font s_el"  ACTION @FileEvent() OF oMenu FONT "Dingbats" Size 30 Color "#A300FF"	
        MENUITEM oMenuItem PROMPT "Calendar"  ACTION @FileEvent() OF oMenu FONT "Dingbats" Size 30 Color "#A300FF"	
        MENUITEM PROMPT "Close" ACTION @FileEvent() OF oMenu SIZE 5 COLOR "#A38103"

        MENU oMenuSec PROMPT "SubMenu" OF oMenu
             MENUITEM PROMPT "Opt1" ID 10 ACTION @FileEvent() OF oMenuSec FONT "Sans" SIZE 60
             MENUITEM PROMPT "Opt2" ID 11 ACTION @FileEvent() OF oMenuSec FONT "Clean" SIZE 14

        MENUITEM PROMPT "QUIT"  ID 99 ACTION @FileEvent() OF oMenu SIZE 40

   MENU oMenuHelp PROMPT "Help"
        MENUITEM PROMPT "About" ACTION {||About() } OF oMenuHelp  FONT "Helvetica" SIZE 18
        MENUITEM PROMPT "Help"  ACTION {|o|FileEvent(o) } OF oMenuHelp  FONT "Courier" SIZE 10

RETURN( { oMenu, oMenuHelp }  )


/***************************************************************/
// Example Multiples lines the test in MsgBox
/***************************************************************/
FUNCTION About(  )

   XWT_MsgBox( "XWT. eXtended Windowing Toolkit"+HB_OSNewLine()+;
               "(c) Giancarlo Nicolai"+HB_OSNewLine()+;
	       "Help desenvoluped by Luiz Culik and Rafa Carmona" ,  XWT_MSGBOX_OK , XWT_MSGBOX_INFO)

RETURN .T.

