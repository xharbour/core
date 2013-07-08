/*
 * $Id: scregion.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: SCREGION.PRG
 * Author....: David A. Richardson
 * CIS ID....: ?
 *
 * This is an original work by David A. Richardson and is hereby placed
 * in the public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.2   15 Aug 1991 23:05:46   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:52:56   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:02:14   GLENN
 * Nanforum Toolkit
 *
 */

STATIC aRgnStack := {}

/*  $DOC$
 *  $FUNCNAME$
 *     FT_SAVRGN()
 *  $CATEGORY$
 *     Video
 *  $ONELINER$
 *     Save a screen region for later display
 *  $SYNTAX$
 *     FT_SAVRGN( <nTop>, <nLeft>, <nBottom>, <nRight> ) -> cScreen
 *  $ARGUMENTS$
 *     <nTop>, <nLeft>, <nBottom>, and <nRight> define the portion of the
 *     screen to save.  Allowable values are 0 through 255.
 *  $RETURNS$
 *     FT_SAVRGN() returns the saved screen region and its coordinates
 *     as a character string.
 *  $DESCRIPTION$
 *     FT_SAVRGN() is similar to Clipper's SAVESCREEN(), but it saves the
 *     screen coordinates as well as the display information.  The saved
 *     area can be restored by passing the returned string to FT_RSTRGN().
 *
 *     Note that the strings returned from FT_SAVRGN() and Clipper's
 *     SAVESCREEN() are not interchangeable.  A screen region saved with
 *     with FT_SAVRGN() must be restored using FT_RSTRGN().
 *
 *     FT_SAVRGN() calls Clipper's SAVESCREEN().  Refer to the Clipper
 *     documentation for more information regarding this function.
 *  $EXAMPLES$
 *     The following example uses FT_SAVRGN() and FT_RSTRGN() to save
 *     and restore a portion of the screen.
 *
 *     @ 00, 00, 24, 79 BOX "111111111"       // fill the screen with 1's
 *     cScreen = FT_SAVRGN(10, 10, 20, 30)    // save a region
 *     @ 00, 00, 24, 79 BOX "222222222"       // fill the screen with 2's
 *     FT_RSTRGN(cScreen)                     // restore the 1's region
 *  $SEEALSO$
 *     FT_RSTRGN() FT_RGNSTACK()
 *  $END$
 */

FUNCTION FT_SAVRGN( nTop, nLeft, nBottom, nRight )

   RETURN ( Chr( nTop ) + Chr( nLeft ) + Chr( nBottom ) + Chr( nRight ) + ;
      SaveScreen( nTop, nLeft, nBottom, nRight ) )


/*  $DOC$
 *  $FUNCNAME$
 *     FT_RSTRGN()
 *  $CATEGORY$
 *     Video
 *  $ONELINER$
 *     Restore region of the screen saved with FT_SAVRGN()
 *  $SYNTAX$
 *     FT_RSTRGN( <cScreen>, [ <nTop> ], [ <nLeft> ] ) -> NIL
 *  $ARGUMENTS$
 *     <cScreen> is a screen region previously returned from FT_SAVRGN().
 *
 *     <nTop> and <nLeft> are optional parameters that define a new location
 *     for the upper left corner of the screen area contained in <cScreen>.
 *     Allowable values are 0 through 255.
 *  $RETURNS$
 *     FT_RSTRGN() returns NIL.
 *  $DESCRIPTION$
 *     FT_RSTRGN() restores a screen region previously saved with
 *     FT_SAVRGN().  Calling FT_RSTRGN() with <cScreen> as the only
 *     parameter will restore the saved region to its original location.
 *     <nTop> and <nLeft> may be used to define a new location for the
 *     upper left corner of the saved region.
 *
 *     <nTop> and <nLeft> are dependent upon each other. You may not
 *     specify one without the other.
 *
 *     FT_RSTRGN() calls Clipper's RESTSCREEN().  Refer to the Clipper
 *     documentation for more information regarding this function.
 *  $EXAMPLES$
 *     The following example uses FT_RSTRGN() to restore a saved portion
 *     of the screen to different locations.
 *
 *     @ 00, 00, 24, 79 BOX "111111111"      // fill the screen with 1's
 *     cScreen = FT_SAVRGN(10, 10, 20, 30)   // save a region
 *     @ 00, 00, 24, 79 BOX "222222222"      // fill the screen with 2's
 *     FT_RSTRGN(cScreen)                    // restore the 1's region
 *     @ 00, 00, 24, 79 BOX "222222222"      // fill the screen with 2's
 *     FT_RSTRGN(cScreen, 15, 15)            // restore to a different location
 *     @ 00, 00, 24, 79 BOX "222222222"      // fill the screen with 2's
 *     FT_RSTRGN(cScreen, 20, 60)            // restore to a different location
 *  $SEEALSO$
 *     FT_SAVRGN() FT_RGNSTACK()
 *  $END$
 */

FUNCTION FT_RSTRGN( cScreen, nTop, nLeft )

   IF PCount() == 3
      RestScreen( nTop, nLeft, ( nTop - Asc(cScreen ) ) + Asc( SubStr(cScreen, 3 ) ), ;
         ( nLeft - Asc( SubStr(cScreen, 2 ) ) ) + Asc( SubStr( cScreen, 4 ) ), ;
         SubStr( cScreen, 5 ) )
   ELSE
      RestScreen( Asc( cScreen ), Asc( SubStr(cScreen, 2 ) ), Asc( SubStr(cScreen, 3 ) ), ;
         Asc( SubStr( cScreen, 4 ) ), SubStr( cScreen, 5 ) )
   ENDIF

   RETURN NIL


/*  $DOC$
 *  $FUNCNAME$
 *     FT_RGNSTACK()
 *  $CATEGORY$
 *     Video
 *  $ONELINER$
 *     Push or pop a saved screen region on or off the stack
 *  $SYNTAX$
 *     FT_RGNSTACK( <cAction>, [ <nTop> ], [ <nLeft> ], [ <nBottom> ],
 *        [ <nRight> ] ) -> NIL
 *  $ARGUMENTS$
 *     <cAction> determines what action FT_RGNSTACK() will take.  The
 *     allowable values for this parameter are "push", "pop", and "pop all".
 *     If the function is called with any other string as the first parameter
 *     no action is performed.
 *
 *     <cAction> with a value of "push" will push a saved screen region onto
 *     the stack.  A value of "pop" will restore the most recently pushed
 *     screen region.  "pop all" tells the function to restore all screen
 *     images which are currently on the stack.
 *
 *     The use of <nTop>, <nLeft>, <nBottom>, and <nRight> depends on the
 *     <cAction> parameter.  If <cAction> is "push", the next four parameters
 *     define the screen region to save.  If <cAction> is "pop" or "pop all"
 *     the following four parameters are ignored.
 *  $RETURNS$
 *     FT_RGNSTACK() returns NIL.
 *  $DESCRIPTION$
 *     FT_RGNSTACK() allows multiple screens to be saved and restored from
 *     a stack.  The stack is implemented with Clipper static array that is
 *     visible only to FT_RGNSTACK().
 *
 *     The purpose of FT_RGNSTACK() is to allow multiple screen regions to be
 *     managed without the need to remember the original coordinates or to
 *     create variables for each one.
 *
 *     When called with "push", FT_RGNSTACK() places the saved screen area
 *     at the end of the static array.  The array size is incremented by one
 *     to accommodate the new screen area.
 *
 *     When called with "pop", the function restores the screen image stored
 *     in the last element of the array, and the array size is decremented by
 *     one.  If "pop all" is specified, all the saved screens are restored
 *     until the array is empty.
 *
 *     FT_RGNSTACK() calls FT_SAVRGN() and FT_RSTRGN().  Refer to the
 *     documentation for these two functions for more information.
 *  $EXAMPLES$
 *     The following example uses FT_RGNSTACK() to save and restore various
 *     sections of the screen.
 *
 *     @ 00, 00, 24, 79 BOX "111111111"         // fill the screen with 1's
 *     FT_RGNSTACK("push", 10, 05, 15, 15)      // push a region
 *     @ 00, 00, 24, 79 BOX "222222222"         // fill the screen with 2's
 *     FT_RGNSTACK("push", 10, 20, 15, 30)      // push a region
 *     @ 00, 00, 24, 79 BOX "333333333"         // fill the screen with 3's
 *     FT_RGNSTACK("push", 10, 35, 15, 45)      // push a region
 *     @ 00, 00, 24, 79 BOX "444444444"         // fill the screen with 4's
 *     FT_RGNSTACK("push", 10, 50, 15, 60)      // push a region
 *     @ 00, 00, 24, 79 BOX "555555555"         // fill the screen with 5's
 *     FT_RGNSTACK("push", 10, 65, 15, 75)      // push a region
 *     CLEAR
 *     FT_RGNSTACK("pop")        // restore the 5's region
 *     FT_RGNSTACK("pop")        // restore the 4's region
 *     FT_RGNSTACK("pop all")    // restore the 3's, 2's and 1's regions
 *  $SEEALSO$
 *     FT_SAVRGN() FT_RSTRGN()
 *  $END$
 */

FUNCTION FT_RGNSTACK( cAction, nTop, nLeft, nBottom, nRight )

   STATIC nStackPtr := 0
   LOCAL nPopTop

   IF cAction == "push"

      ASize( aRgnStack, ++nStackPtr )[nStackPtr] = ;
         FT_SAVRGN( nTop, nLeft, nBottom, nRight )

   ELSEIF cAction == "pop" .OR. cAction = "pop all"

      nPopTop = iif( "all" $ cAction, 0, nStackPtr - 1 )

      DO WHILE nStackPtr > nPopTop
         FT_RSTRGN( aRgnStack[nStackPtr--] )
      ENDDO

      ASize( aRgnStack, nStackPtr )

   ENDIF

   RETURN NIL

