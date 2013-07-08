//
// $Id: testhtml.prg 9279 2011-02-14 18:06:32Z druzus $
//

/*
*
*  TestHTML.PRG
*  Harbour Test of a HTML-Generator class.
*
*  1999/05/30  First implementation.
*
*              Tips: - Use ShowResults to make dynamic html (to test dynamic
*                      results, put the exe file on CGI-BIN dir or equivalent);
*                    - Use SaveToFile to make static html page
*
**/

STATIC s_cNewLine

FUNCTION Main()

   LOCAL oHTML := THTML():New()

   s_cNewLine := HB_OSNewLine()

   oHTML:SetTitle( "Harbour Power Demonstration" )
   oHTML:AddHead( "Harbour Project" )
   oHTML:AddPara( "<B>Harbour</B> is xBase at its best. Have a taste today!", "LEFT" )
   oHTML:AddPara( "<B>L i n k s</B>", "CENTER" )
   oHTML:AddLink( "http://www.Harbour-Project.org", "Meet the harbour power!" )
   oHTML:Generate()

   // Uncomment the following if you don't have a Web Server to test
   // this sample

   // oHTML:SaveToFile( "test.htm" )

   // If the above is uncommented, you may comment this line:

   oHTML:ShowResult()

   RETURN( NIL )

/*---------------------------------------------------------------------------*/

FUNCTION THTML

   STATIC oClass

   IF oClass == NIL
      oClass = HBClass():New( "THTML" )

      oClass:AddData( "cTitle" )                       // Page Title
      oClass:AddData( "cBody" )                        // HTML Body Handler
      oClass:AddData( "cBGColor" )                     // Background Color
      oClass:AddData( "cLinkColor" )                   // Link Color
      oClass:AddData( "cvLinkColor" )                  // Visited Link Color
      oClass:AddData( "cContent" )                     // Page Content Handler

      oClass:AddMethod( "New",        @New() )         // New Method
      oClass:AddMethod( "SetTitle",   @SetTitle() )    // Set Page Title
      oClass:AddMethod( "AddHead",    @AddHead() )     // Add <H1> Header
      oClass:AddMethod( "AddLink",    @AddLink() )     // Add Hyperlink
      oClass:AddMethod( "AddPara",    @AddPara() )     // Add Paragraph
      oClass:AddMethod( "Generate",   @Generate() )    // Generate HTML
      oClass:AddMethod( "SaveToFile", @SaveToFile() )  // Saves Content to File
      oClass:AddMethod( "ShowResult", @ShowResult() )  // Show Result - SEE Fcn

      oClass:Create()
   ENDIF

   RETURN( oClass:Instance() )

STATIC FUNCTION New()

   LOCAL Self := QSelf()

   ::cTitle      := "Untitled"
   ::cBGColor    := "#FFFFFF"
   ::cLinkColor  := "#0000FF"
   ::cvLinkColor := "#FF0000"
   ::cContent    := ""
   ::cBody       := ""

   RETURN( Self )

STATIC FUNCTION SetTitle( cTitle )

   LOCAL Self := QSelf()

   ::cTitle := cTitle

   RETURN( Self )

STATIC FUNCTION AddLink( cLinkTo, cLinkName )

   LOCAL Self := QSelf()

   ::cBody := ::cBody + ;
      "<A HREF='" + cLinkTo + "'>" + cLinkName + "</A>"

   RETURN( Self )

STATIC FUNCTION AddHead( cDescr )

   LOCAL Self := QSelf()

   // Why this doesn't work?
   // ::cBody += ...
   // ???

   ::cBody := ::cBody + ;
      "<H1>" + cDescr + "</H1>"

   RETURN( NIL )

STATIC FUNCTION AddPara( cPara, cAlign )

   LOCAL Self := QSelf()

   //Default( cAlign, "Left" ) // removed Patrick Mast 2000-06-07
   cAlign:=If(cAlign==NIL,"Left",cAlign) //Added Patrick Mast 2000-06-17

   ::cBody := ::cBody + ;
      "<P ALIGN='" + cAlign + "'>" + s_cNewLine + ;
      cPara + s_cNewLine + ;
      "</P>"

   RETURN( Self )

STATIC FUNCTION Generate()

   LOCAL Self := QSelf()

   ::cContent :=                                                           ;
      "<HTML><HEAD>"                                           + s_cNewLine + ;
      "<TITLE>" + ::cTitle + "</TITLE>"                        + s_cNewLine + ;
      "<BODY link='" + ::cLinkColor + "' " +                               ;
      "vlink='" + ::cvLinkColor + "'>" +                       + s_cNewLine + ;
      ::cBody                                                  + s_cNewLine + ;
      "</BODY></HTML>"

   RETURN( Self )

STATIC FUNCTION ShowResult()

   LOCAL Self := QSelf()

   qqOut(                                                                  ;
      "HTTP/1.0 200 OK"                                        + s_cNewLine + ;
      "CONTENT-TYPE: TEXT/HTML"                      + s_cNewLine + s_cNewLine + ;
      ::cContent )

   RETURN( Self )

STATIC FUNCTION SaveToFile( cFile )

   LOCAL Self  := QSelf()
   LOCAL hFile := fCreate( cFile )

   fWrite( hFile, ::cContent )
   fClose( hFile )

   RETURN( Self )
