//
// $Id: begin.prg 9279 2011-02-14 18:06:32Z druzus $
//
// This files demonstrates the use of BEGIN/RECOVER/END SEQUENCE
// and BREAK statement
//
MEMVAR oMemvar
MEMVAR mPrivate

PROCEDURE MAIN
LOCAL oLocal
PRIVATE mPrivate:='private value in MAIN'


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 1"
    ? "  No break issued...."
  RECOVER
    ? "  OOPS!!! Recovering in 1 ..."
  END SEQUENCE
  ? "After SEQUENCE 1"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 2"
    Break( "VALUE 2" )
  RECOVER USING oLocal
    ? "  Recovering in 2 using....", oLocal
  END SEQUENCE
  ? "After SEQUENCE 2"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 3"
    Break
  RECOVER USING oLocal
    ? "  Recovering in 3 using....", oLocal
  END SEQUENCE
  ? "After SEQUENCE 3"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 4"
    Break
    ? "  Recovering in 4 using....", oLocal
  END SEQUENCE
  ? "After SEQUENCE 4"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 5"
    Break1( )
    ? "  Recovering in 5 using....", oLocal
  END SEQUENCE
  ? "After SEQUENCE 5"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 6"
    Break1( )
  RECOVER USING oMemvar
    ? "  Recovering in 6 using... ", oMemvar
  END SEQUENCE
  ? "After SEQUENCE 6"


  BEGIN SEQUENCE
    ? " Inside SEQUENCE 7"
    Break2( )
  RECOVER USING oMemvar
    ? "  Recovering in 7 using... ", oMemvar
  END SEQUENCE
  ? "After SEQUENCE 7"

  ? M->mPrivate  
  BREAK( "exit from MAIN" )
  ? "This text will be not printed"
  
RETURN

PROCEDURE Break1()
  PRIVATE mPrivate:='VALUE from Break1'

  BREAK M->mPrivate

RETURN

PROCEDURE Break2()

    BEGIN SEQUENCE
        ? " Inside SEQUENCE 8"
        Break3( )
    RECOVER USING oMemvar
        ? "  Recovering in 8 using...", EVAL( oMemvar, ' eval in 8' )
        BREAK( "BREAK from recovery code" )
    END SEQUENCE
    ? "After SEQUENCE 8"

RETURN


PROCEDURE Break3()
STATIC oStatic

    BEGIN SEQUENCE
        ? " Inside SEQUENCE 9"

        BEGIN SEQUENCE
            ? " Inside SEQUENCE 10"
            Break( "value from nested SEQUENCE 10" )
        RECOVER USING oStatic
            ? "  Recovering in 10 using...", oStatic
        END SEQUENCE
        ? "After SEQUENCE 10"

        Break4( " and parameter" )

    RECOVER USING oMemvar
        ? "  Recovering in 9 using...", EVAL( oMemvar, ' eval in 9' )
        BREAK( oMemvar )
    END SEQUENCE
    ? "After SEQUENCE 9"

RETURN


PROCEDURE Break4( cValue )
LOCAL oLocal:=' detached Break4 '

  BREAK( {|x| oLocal + x + cValue} )

RETURN