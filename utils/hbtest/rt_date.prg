/*
 * $Id: rt_date.prg 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Regression tests for the runtime library (date)
 *
 * Copyright 1999-2001 Viktor Szakats <viktor.szakats@syenar.hu>
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA (or visit the web site http://www.gnu.org/).
 *
 * As a special exception, the Harbour Project gives permission for
 * additional uses of the text contained in its release of Harbour.
 *
 * The exception is that, if you link the Harbour libraries with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the Harbour library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by the Harbour
 * Project under the name Harbour.  If you copy code from other
 * Harbour Project or Free Software Foundation releases into a copy of
 * Harbour, as the General Public License permits, the exception does
 * not apply to the code that you add in this way.  To avoid misleading
 * anyone as to the status of such modified files, you must delete
 * this exception notice from them.
 *
 * If you write modifications of your own for Harbour, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 *
 */

#include "rt_main.ch"

/* Don't change the position of this #include. */
#include "rt_vars.ch"

PROCEDURE Main_DATE()
   LOCAL cDate := "1999/11/25"

   /* YEAR() */

   TEST_LINE( Year(NIL)                       , "E BASE 1112 Argument error YEAR A:1:U:NIL F:S" )
   TEST_LINE( Year(100)                       , "E BASE 1112 Argument error YEAR A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( Year(@sdDate)                   , 1984                                   ) /* Bug in CA-Cl*pper, it returns: "E BASE 1112 Argument error YEAR F:S" */
#endif
   TEST_LINE( Year(sdDate)                    , 1984                                   )
   TEST_LINE( Year(sdDateE)                   , 0                                      )
   TEST_LINE( Str(Year(HB_SToD("19990905")))  , " 1999"                                )

   /* MONTH() */

   TEST_LINE( Month(NIL)                      , "E BASE 1113 Argument error MONTH A:1:U:NIL F:S" )
   TEST_LINE( Month(100)                      , "E BASE 1113 Argument error MONTH A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( Month(@sdDate)                  , 3                                      ) /* Bug in CA-Cl*pper, it returns: "E BASE 1113 Argument error MONTH F:S" */
#endif
   TEST_LINE( Month(sdDate)                   , 3                                      )
   TEST_LINE( Month(sdDateE)                  , 0                                      )
   TEST_LINE( Str(Month(HB_SToD("19990905"))) , "  9"                                  )

   /* DAY() */

   TEST_LINE( Day(NIL)                        , "E BASE 1114 Argument error DAY A:1:U:NIL F:S" )
   TEST_LINE( Day(100)                        , "E BASE 1114 Argument error DAY A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( Day(@sdDate)                    , 25                                     ) /* Bug in CA-Cl*pper, it returns: "E BASE 1114 Argument error DAY F:S" */
#endif
   TEST_LINE( Day(sdDate)                     , 25                                     )
   TEST_LINE( Day(sdDateE)                    , 0                                      )
   TEST_LINE( Str(Day(HB_SToD("19990905")))   , "  5"                                  )

   /* TIME() */

   TEST_LINE( Len(Time())                     , 8                                      )

   /* DOW() */

   TEST_LINE( Dow(NIL)                        , "E BASE 1115 Argument error DOW A:1:U:NIL F:S" )
   TEST_LINE( Dow(100)                        , "E BASE 1115 Argument error DOW A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( Dow(@sdDate)                    , 1                                      ) /* Bug in CA-Cl*pper, it returns: "E BASE 1115 Argument error DOW F:S" */
#endif
   TEST_LINE( Dow(sdDate)                     , 1                                      )
   TEST_LINE( Dow(sdDateE)                    , 0                                      )
   TEST_LINE( Dow(HB_SToD("20000222"))        , 3                                      )
   TEST_LINE( Dow(HB_SToD("20000223"))        , 4                                      )
   TEST_LINE( Dow(HB_SToD("20000224"))        , 5                                      )
   TEST_LINE( Dow(HB_SToD("20000225"))        , 6                                      )
   TEST_LINE( Dow(HB_SToD("20000226"))        , 7                                      )
   TEST_LINE( Dow(HB_SToD("20000227"))        , 1                                      )
   TEST_LINE( Dow(HB_SToD("20000228"))        , 2                                      )
   TEST_LINE( Dow(HB_SToD("20000229"))        , 3                                      )
   TEST_LINE( Dow(HB_SToD("20000230"))        , 0                                      )
   TEST_LINE( Dow(HB_SToD("20000231"))        , 0                                      )
   TEST_LINE( Dow(HB_SToD("20000301"))        , 4                                      )

   /* CMONTH() */

   TEST_LINE( CMonth(NIL)                     , "E BASE 1116 Argument error CMONTH A:1:U:NIL F:S" )
   TEST_LINE( CMonth(100)                     , "E BASE 1116 Argument error CMONTH A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( CMonth(@sdDate)                 , "March"                                 ) /* Bug in CA-Cl*pper, it returns: "E BASE 1116 Argument error CMONTH F:S" */
#endif
   TEST_LINE( CMonth(sdDate)                  , "March"                                 )
   TEST_LINE( CMonth(sdDateE)                 , ""                                      )
   TEST_LINE( CMonth(HB_SToD("19990101"))     , "January"                               )
   TEST_LINE( CMonth(HB_SToD("19990201"))     , "February"                              )
   TEST_LINE( CMonth(HB_SToD("19990301"))     , "March"                                 )
   TEST_LINE( CMonth(HB_SToD("19990401"))     , "April"                                 )
   TEST_LINE( CMonth(HB_SToD("19990501"))     , "May"                                   )
   TEST_LINE( CMonth(HB_SToD("19990601"))     , "June"                                  )
   TEST_LINE( CMonth(HB_SToD("19990701"))     , "July"                                  )
   TEST_LINE( CMonth(HB_SToD("19990801"))     , "August"                                )
   TEST_LINE( CMonth(HB_SToD("19990901"))     , "September"                             )
   TEST_LINE( CMonth(HB_SToD("19991001"))     , "October"                               )
   TEST_LINE( CMonth(HB_SToD("19991101"))     , "November"                              )
   TEST_LINE( CMonth(HB_SToD("19991201"))     , "December"                              )

   /* CDOW() */

   TEST_LINE( CDow(NIL)                       , "E BASE 1117 Argument error CDOW A:1:U:NIL F:S" )
   TEST_LINE( CDow(100)                       , "E BASE 1117 Argument error CDOW A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( CDow(@sdDate)                   , "Sunday"                               ) /* Bug in CA-Cl*pper, it returns: "E BASE 1117 Argument error CDOW F:S" */
#endif
   TEST_LINE( CDow(sdDate)                    , "Sunday"                               )
   TEST_LINE( CDow(sdDateE)                   , ""                                     )
   TEST_LINE( CDow(HB_SToD("20000222"))       , "Tuesday"                              )
   TEST_LINE( CDow(HB_SToD("20000223"))       , "Wednesday"                            )
   TEST_LINE( CDow(HB_SToD("20000224"))       , "Thursday"                             )
   TEST_LINE( CDow(HB_SToD("20000225"))       , "Friday"                               )
   TEST_LINE( CDow(HB_SToD("20000226"))       , "Saturday"                             )
   TEST_LINE( CDow(HB_SToD("20000227"))       , "Sunday"                               )
   TEST_LINE( CDow(HB_SToD("20000228"))       , "Monday"                               )
   TEST_LINE( CDow(HB_SToD("20000229"))       , "Tuesday"                              )
   TEST_LINE( CDow(HB_SToD("20000230"))       , ""                                     )
   TEST_LINE( CDow(HB_SToD("20000231"))       , ""                                     )
   TEST_LINE( CDow(HB_SToD("20000301"))       , "Wednesday"                            )

   /* DTOC() */

   TEST_LINE( DToC(NIL)                       , "E BASE 1118 Argument error DTOC A:1:U:NIL F:S" )
   TEST_LINE( DToC(100)                       , "E BASE 1118 Argument error DTOC A:1:N:100 F:S" )
   TEST_LINE( DToC("")                        , "E BASE 1118 Argument error DTOC A:1:C: F:S"    )
#ifdef __HARBOUR__
   TEST_LINE( DToC(@sdDate)                   , "1984.03.25"                           ) /* Bug in CA-Cl*pper, it returns: "E BASE 1118 Argument error DTOC F:S" */
#endif
   TEST_LINE( DToC(sdDate)                    , "1984.03.25"                           )
   TEST_LINE( DToC(sdDateE)                   , "    .  .  "                           )

   /* CTOD() */

   TEST_LINE( CToD(NIL)                       , "E BASE 1119 Argument error CTOD A:1:U:NIL F:S" )
   TEST_LINE( CToD(100)                       , "E BASE 1119 Argument error CTOD A:1:N:100 F:S" )
   TEST_LINE( CToD("")                        , HB_SToD("        ")                    )
#ifdef __HARBOUR__
   TEST_LINE( CToD(@cDate)                    , HB_SToD("19991125")                    ) /* Bug in CA-Cl*pper, it returns: "E BASE 1119 Argument error CTOD F:S" */
#endif
   TEST_LINE( CToD(cDate)                     , HB_SToD("19991125")                    )
   TEST_LINE( CToD("1999/11/25/10")           , HB_SToD("19991125")                    )

   /* DTOS() */

   TEST_LINE( DToS(NIL)                       , "E BASE 1120 Argument error DTOS A:1:U:NIL F:S" )
   TEST_LINE( DToS(100)                       , "E BASE 1120 Argument error DTOS A:1:N:100 F:S" )
#ifdef __HARBOUR__
   TEST_LINE( DToS(@sdDate)                   , "19840325"                             ) /* Bug in CA-Cl*pper, it returns: "E BASE 1120 Argument error DTOS F:S" */
#endif
   TEST_LINE( DToS(sdDate)                    , "19840325"                             )
   TEST_LINE( DToS(sdDateE)                   , "        "                             )

   RETURN

/* Don't change the position of this #include. */
#include "rt_init.ch"
