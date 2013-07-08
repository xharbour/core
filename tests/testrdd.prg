#include "ORD.CH"

#define  CRLF            Chr(13)+Chr(10)
#define  MAX_TEST_RECS   100
#define  INDEX_KEY_CHAR  CHAR + Str( NUM ) + DTOS( DATE )
#define  INDEX_KEY_NUM   NUM
#define  INDEX_KEY_DATE  DATE
#define  INDEX_KEY_LOG   LOG

#ifdef ADS
  EXTERNAL _ADS
#endif

EXTERNAL DBFNTX
EXTERNAL DBFCDX

procedure Main( cRDDType, cAdsMode )

local cRDD, aStruct, xTemp, bMemoText

field CHAR, NUM, DATE, LOG

bMemoText := { || "This is memo #" + LTrim( Str( RecNo() ) ) + "." + CRLF + ;
                  CRLF + ;
                  "This is a very long string. " + ;
                  "This may seem silly however strings like this are still " + ;
                  "used. Not by good programmers though, but I've seen " + ;
                  "stuff like this used for Copyright messages and other " + ;
                  "long text. What is the point to all of this you'd say. " + ;
                  "Well I am coming to the point right now, the constant " + ;
                  "string is limited to 256 characters and this string is " + ;
                  "a lot bigger. Do you get my drift ? If there is somebody " + ;
                  "who has read this line upto the very end: Esto es un " + ;
                  "sombrero grande rid�culo." + CRLF + "/" + CRLF + "[;-)" + CRLF + "\" }

do case

case Empty( cRDDType )

    NotifyUser( "Usage: TESTRDD RDDTYPE [ADSMODE]" + CRLF + ;
                CRLF + ;
                "RDDTYPE = DBFNTX, DBFCDX, ADSCDX, ADSNTX or ADSADT" + CRLF + ;
                CRLF + ;
                "ADSMODE = LOCAL or SERVER (only applies to ADSCDX, ADSNTX and ADSADT)" + CRLF + ;
                "(If specify SERVER, must be run from a drive suported by ADS server)", .t. )

#ifdef ADS
case Left( cRDDType := Upper( AllTrim( cRDDType ) ), 3 ) == "ADS"

    // Do not include ADS.CH as don't want unintended affects when not using
    // ADS - If need behavior from ADS.CH, include defines and undefs in
    // these areas.

    #define ADS_LOCAL_SERVER  1
    #define ADS_REMOTE_SERVER 2
    #define ADS_NTX           1
    #define ADS_CDX           2
    #define ADS_ADT           3

    RDDSetDefault( "ADSCDX" )

    if Empty( cADSMode )
        NotifyUser( "Missing ADS mode" )
    endif

    cADSMode := Upper( AllTrim( cADSMode ) )

    do case
    case cADSMode == "LOCAL"  ; AdsSetServerType( ADS_LOCAL_SERVER )
    case cADSMode == "SERVER" ; AdsSetServerType( ADS_REMOTE_SERVER )
    otherwise                 ; NotifyUser( "Bad ADS mode" )
    endcase

    do case
    case cRDDType == "ADSNTX" ; AdsSetFileType( ADS_NTX ) ; cRDD := "ADSNTX"
    case cRDDType == "ADSADT" ; AdsSetFileType( ADS_ADT ) ; cRDD := "ADSADT"
    case cRDDType == "ADSCDX" ; AdsSetFileType( ADS_CDX ) ; cRDD := "ADSCDX"
    otherwise                 ; NotifyUser( "Bad ADS flavor" )
    endcase

    AdsLocking( .t. )
    AdsRightsCheck( .t. )

    AdsSetDefault( "" )
    AdsSetSearchPath( "" )

    #undef ADS_LOCAL_SERVER
    #undef ADS_REMOTE_SERVER
    #undef ADS_NTX
    #undef ADS_CDX
    #undef ADS_ADT
#endif

case cRDDType == "DBFCDX" .or. ;
     cRDDType == "DBFNTX"

    RDDSetDefault( cRDD := cRDDType )

otherwise

    NotifyUser( "Bad DBF flavor" )

endcase

// Delete TEST.* since may be changing RDD flavors (avoid conflicts)
AEval( Directory( "TEST.*"  ), { | a | FErase( a[1] ) } )
AEval( Directory( "TEST?.*" ), { | a | FErase( a[1] ) } )

if File( "TEST.DBF" )
    NotifyUser( "Cannot delete TEST.DBF" )
endif

// TEST: DBCreate()

DBCreate( "TEST.DBF", ;
          aStruct := { { "CHAR", "C", 30, 0 }, ;
                       { "NUM",  "N", 15, 3 }, ;
                       { "DATE", "D",  8, 0 }, ;
                       { "LOG",  "L",  1, 0 }, ;
                       { "MEMO", "M", 10, 0 } } )

if .not. File( "TEST.DBF" )
    NotifyUser( "Failed to create TEST.DBF" )
endif

// TEST: DBUseArea()/USE

use TEST.DBF new shared alias MYTEST

if .not. Alias() == "MYTEST"
    NotifyUser( "Failed to open TEST.DBF" )
endif

// TEST: RDDName()

if .not. RDDName() == cRDD
    NotifyUser( "Failed to set RDD to " + cRDD )
endif

// TEST: DBStruct()

if .not. CompareArray( aStruct, DBStruct() )
    NotifyUser( "Resulting table structure is not what we asked for" )
endif

// TEST: Header()

if .not. Header() == 194
    NotifyUser( "Header() returned wrong size (" + LTrim( Str( Header() ) ) + " bytes)" )
endif

// Add a mix of data to table

do while LastRec() < MAX_TEST_RECS

    // TEST: DBAppend()/APPEND BLANK

    append blank

    // TEST: REPLACE

    replace CHAR with Chr( 65 + Val( SubStr( LTrim( Str( RecNo() ) ), 2, 1 ) ) ) + ;
                      " RECORD " + LTrim( Str( RecNo() ) )

    // TEST: Direct field assigment

    MYTEST->NUM  := ( if( RecNo() % 2 > 0, -1, 1 ) * RecNo() ) + ( RecNo() / 1000 )
    MYTEST->DATE := Date() + Int( FIELD->NUM )
    MYTEST->LOG  := ( FIELD->NUM < 0 )
    MYTEST->MEMO := Eval( bMemoText )

enddo

// TEST: LastRec()

if .not. LastRec() == MAX_TEST_RECS
    NotifyUser( "DbAppend and/or LastRec failed" )
endif

// TEST: DbGoBotom()/GO BOTTOM

go bottom

if .not. RecNo() == MAX_TEST_RECS
    NotifyUser( "DbGoBottom failed" )
endif

// TEST: DbGoTop()/GO TOP

go top

if .not. RecNo() == 1
    NotifyUser( "DbGoTop failed" )
endif

// Now check each and every record for accuracy

do while .not. EOF()

    // TEST: Field access

    if .not. Trim( FIELD->CHAR ) == Chr( 65 + Val( SubStr( LTrim( Str( RecNo() ) ), 2, 1 ) ) ) + ;
                                    " RECORD " + LTrim( Str( RecNo() ) ) .or. ;
       .not. FIELD->NUM == ( if( RecNo() % 2 > 0, -1, 1 ) * RecNo() ) + ( RecNo() / 1000 ) .or. ;
       .not. FIELD->DATE == Date() + Int( FIELD->NUM ) .or. ;
       .not. FIELD->LOG == ( FIELD->NUM < 0 ) .or. ;
       .not. FIELD->MEMO == Eval( bMemoText )

        NotifyUser( "Data in table is incorrect" )

    endif

    skip

enddo

// TEST: Index creation

index on INDEX_KEY_CHAR to TESTC
index on INDEX_KEY_NUM  to TESTN additive
index on INDEX_KEY_DATE to TESTD additive
index on INDEX_KEY_LOG  to TESTL additive

// TEST: IndexOrd()

if .not. IndexOrd() == 4
    NotifyUser( "Bad IndexOrd()" )
endif

// TEST: DBOI_KEYCOUNT

set order to 1
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == MAX_TEST_RECS
    NotifyUser( "Bad DBOI_KEYCOUNT/1" )
endif

set order to 2
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == MAX_TEST_RECS
    NotifyUser( "Bad DBOI_KEYCOUNT/2" )
endif

set order to 3
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == MAX_TEST_RECS
    NotifyUser( "Bad DBOI_KEYCOUNT/3" )
endif

set order to 4
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == MAX_TEST_RECS
    NotifyUser( "Bad DBOI_KEYCOUNT/4" )
endif

// TEST: Character index
set order to 1
go top
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_CHAR
    NotifyUser( "Bad DBOI_KEYVAL (CHAR)" )
endif

// TEST: Positive index key
set order to 2
locate for FIELD->NUM > 0
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_NUM
    NotifyUser( "Bad DBOI_KEYVAL (NUM)" )
endif

// TEST: Negative index key
set order to 2
locate for FIELD->NUM < 0
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_NUM
    NotifyUser( "Bad DBOI_KEYVAL (NUM)" )
endif

// TEST: Date index
set order to 3
go bottom
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_DATE
    NotifyUser( "Bad DBOI_KEYVAL (DATE)" )
endif

// TEST: Logical index
set order to 4
go top
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_LOG
    NotifyUser( "Bad DBOI_KEYVAL (LOG/1)" )
endif
go bottom
if .not. DbOrderInfo( DBOI_KEYVAL ) == INDEX_KEY_LOG
    NotifyUser( "Bad DBOI_KEYVAL (LOG/2)" )
endif

// TEST: EXACT with a locate

set order to 0

set exact on
locate for FIELD->CHAR = "J RECORD"
if .not. EOF()
    NotifyUser( "LOCATE with EXACT ON failed" )
endif

set exact off
locate for FIELD->CHAR = "J RECORD"
if EOF()
    NotifyUser( "LOCATE with EXACT OFF failed" )
endif

// TEST: EXACT with an index (also tests COUNT)

set exact on
set order to 0
count for Trim( FIELD->CHAR ) = "A RECORD 1" to xTemp  // Get proper count
index on CHAR to TESTE for Trim( FIELD->CHAR ) = "A RECORD 1" additive
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == xTemp
    NotifyUser( "Bad conditional index count with EXACT ON" )
endif

set exact off
set order to 0
count for Trim( FIELD->CHAR ) = "A RECORD 1" to xTemp  // Get proper count
index on CHAR to TESTE for Trim( FIELD->CHAR ) = "A RECORD 1" additive
if .not. DbOrderInfo( DBOI_KEYCOUNT ) == xTemp
    NotifyUser( "Bad conditional index count with EXACT OFF" )
endif

//
//
// *********************************************
// P U T   M O R E   R D D   T E S T S   H E R E
// *********************************************
//
//
//

// TEST: DBCloseArea()

MYTEST->( DBCloseArea() )

if Select( "MYTEST" ) > 0
    NotifyUser( "Failed to close table" )
endif

NotifyUser( "Test PASSED! :-)", .t. )

return


procedure ErrorSys()
ErrorBlock( { |e| MyError( e ) } )
return


static procedure MyError( e )

local cTrace := "", i := 1 /*Start are "real" error*/, cErr

cErr := "Runtime error" + CRLF + ;
        CRLF + ;
        "Gencode: " + LTrim( Str( e:GenCode ) ) + CRLF + ;
        "Desc: " + e:Description +  + CRLF + ;
        "Sub-system: " + LTrim( Str( e:SubCode ) ) + CRLF + ;
        CRLF + ;
        "Call trace:" + CRLF + ;
        CRLF

do while .not. Empty( ProcName( ++i ) )
    cErr += Trim( ProcName( i ) ) + "(" + Ltrim( Str( ProcLine( i ) ) ) + ")" + CRLF
enddo

NotifyUser( cErr )  // Calls quit

return


static function CompareArray( a1, a2 )

local i, j

if .not. Len( a1 ) == Len( a2 )
    return .f.
endif

for i := 1 to Len( a1 )

    for j := 1 to Len( a1[i] )

        if .not. a1[i,j] == a2[i,j]
            return .f.
        endif

    next

next

return .t.


static procedure NotifyUser( cErr, lNotError )

? cErr

Quit  // If remove this, will display all error without stopping

return
