/*
   XWT - xHarbour Windowing Toolkit

   (C) 2003 Giancarlo Niccolai

   $Id: xwtcmd.ch 9279 2011-02-14 18:06:32Z druzus $

   Definitions
*/


#ifndef __XTWCMD_CH__
#define __XTWCMD_CH__
#include "xwt.ch"

#xcommand DEFINE PANE <oPane> [ <lHasBox: BOX >];
	  [ TITLE <cTitle>] [ OF <oParent> ];
          => <oPane> := XWTPane():New( <.lHasBox.>, <cTitle>, <oParent> )

#xcommand DEFINE LAYOUT <oLayout> [ MODE <nMode>];
          [ SIZE <nWidth>,<nHeigth> ] ;
          [ PADDING <nPadding>] [BORDER <nBorder> ];
          [ OF <oParent> ] => <oLayout> := XWTLayout():New( <nMode>, <nWidth>, <nHeigth>, <nPadding>, <nBorder>,  <oParent> )

#xcommand DEFINE WINDOW <oWindow> [ TITLE <cTitle>] [ MENU <aMenus> ];
          => <oWindow> := XWTFrameWindow():New( <cTitle>, <aMenus> )



// Button ----------------------------------------------------------------------
#xcommand DEFINE BUTTON <oBtn> [ TEXT <cText> ] [ MOVE <x>,<y> ][ OF <oParent> ] [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>] ;
          => <oBtn> := XWTButton():New( <cText>, <x>, <y>, <oParent>, <(cFont)>, <nSize>, <(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

#xcommand @ <x>,<y> BUTTON <oBtn> [ TEXT <cText>] [ OF <oParent> ] [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>];
          => <oBtn> := XWTButton():New( <cText>, <x>, <y>, <oParent>, <(cFont)>,<nSize>,<(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

// Label -------------------------------------------------------------------------
#xcommand DEFINE LABEL <oLabel> [ TEXT <cText> ] [ MOVE <x>,<y> ] [ OF <oParent> ] [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>];
          => <oLabel> := XWTLabel():New( <cText>, <x>, <y>, <oParent>, <(cFont)>,<nSize>,<(clr)>)

#xcommand @ <x>,<y> LABEL <oLabel> [ TEXT <cText> ] [ OF <oParent> ] [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>];
          => <oLabel> := XWTLabel():New( <cText>, <x>, <y>, <oParent>, <(cFont)>,<nSize>,<(clr)>)

// TextBox -------------------------------------------------------------------------
#xcommand DEFINE TEXTBOX [ <oTextBox> ] [ VAR <cText> ] [MOVE <x>,<y>] [ OF <oParent> ];
          => [ <oTextBox> := ] XWTTextBox():New( <cText>, <x>, <y>, <oParent> )

#xcommand @ <x>,<y> TEXTBOX [ <oTextBox> ] [ VAR <cText> ] [ OF <oParent> ] ;
          => [ <oTextBox> := ] XWTTextBox():New( <cText>, <x>, <y>, <oParent> )


// Checkbox ---------------------------------------------------------------------------
#xcommand DEFINE CHECKBOX [<oCheck>] [TEXT <cText>] [ VAR <lStatus>];
                          [ MOVE <x>,<y>] [ OF <oParent> ]  [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>] ;
                  => [<oCheck> := ] XWTCheckbox():New( <cText>,<lStatus>,<x>,<y>,<oParent> , <(cFont)>,<nSize>,<(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

#xcommand @ <x>,<y> CHECKBOX [<oCheck>] [TEXT <cText>] [ VAR <lStatus>] [ OF <oParent> ]  [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>] ;
                  => [<oCheck> := ] XWTCheckbox():New( <cText>,<lStatus>,<x>,<y>,<oParent> , <(cFont)>,<nSize>,<(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

// ToggleButton ---------------------------------------------------------------------------
#xcommand DEFINE TOGGLEBUTTON [<oToggle>] [TEXT <cText>] [ VAR <lStatus>];
                          [ MOVE <x>,<y>] [ OF <oParent> ]  [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>] ;
                  => [<oToggle> := ] XWTToggleButton():New( <cText>,<lStatus>,<x>,<y>,<oParent> , <(cFont)>,<nSize>,<(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

#xcommand @ <x>,<y> TOGGLEBUTTON [<oToggle>] [TEXT <cText>] [ VAR <lStatus>] [ OF <oParent> ]  [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>] [BGCOLOR <bgclr>]  [BASECOLOR <baclr>]  [TEXTCOLOR <Txtclr>] ;
                  => [<oToggle> := ] XWTToggleButton():New( <cText>,<lStatus>,<x>,<y>,<oParent> , <(cFont)>,<nSize>,<(clr)>, <(bgclr)>, <(baclr)> ,<(Txtclr)>)

// Splitter --------------------------------------------------------------------------------
#xcommand DEFINE SPLITTER  [<oSplitter>] [ MODE <nMode> ][ FIRSTWIDGET <oFirstWidget> ][SECONDWIDGET <oSecondWidget>];
                           [ SIZE <nSizeX>,<nSizeY> ] [ OF <oParent> ];
 => [ <oSplitter> := ] XWTSplitter():New( <nMode>, <oFirstWidget>, <oSecondWidget>, <nSizeX>, <nSizeY>, <oParent> )


// Image -------------------------------------------------------------------------------------
#xcommand DEFINE IMAGE [ <oImg> ] [FILE <cFile> ] [ OF <oParent> ];
                 => [ <oImg> := ] XWTImage():New( <cFile>,<oParent>)

// Menus ----------------------------------------------------------------------------------------
#xcommand MENU <oMenu> [ PROMPT < cPrompt> ] [ OF <oParent> ] ;
          => <oMenu> := XWTMenu():New( <cPrompt>, <oParent> )

#xcommand MENUITEM [<oMenuItem>] [ PROMPT <cPrompt> ] [ ID <nId>] [ICON <cIcon>];
                               [ ACTION <pAction> ] [ METHOD <oMethod> ][ OF <oMenu> ] [FONT <cFont>] [SIZE <nSize>] [COLOR <clr>];
          => [<oMenuItem> := ] XWTMenuItem():New( <cPrompt>, <nId>, <pAction>, <oMethod>, <cIcon>, <oMenu> ,<(cFont)>,<nSize>,<(clr)>)

//calendat
#xcommand @ <x>,<y> CALENDAR [ <oTextBox> ] [ VAR <cText> ] [ OF <oParent> ];
          => [ <oTextBox> := ] XWTCalendar():New( <cText>, <x>, <y>, <oParent> )
	  
#xcommand @ <x>,<y> COMBO [ <oTextBox> ] [ VAR <cText> ] [ OF <oParent> ] ITEMS <aItems>;
          => [ <oTextBox> := ] XWTComboBox():New( <cText>, <x>, <y>, <oParent> ,<aItems>)

#xcommand @ <x>,<y> LISTBOX [ <oTextBox> ] [ VAR <cText> ] [ OF <oParent> ] ITEMS <aItems>;
          => [ <oTextBox> := ] XWTListBox():New( <cText>, <x>, <y>, <oParent> ,<aItems>)	  
	  
// Tope level commands --------------------------------------------------------------------------
#xcommand QUIT => XWTQUIT()

#endif
