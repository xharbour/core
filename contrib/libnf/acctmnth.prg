/*
 * $Id: acctmnth.prg 9686 2012-09-16 08:14:35Z andijahja $
 */
/*
 * File......: ACCTMNTH.PRG
 * Author....: Jo W. French dba Practical Computing
 * CIS ID....: 74731,1751
 *
 * The functions contained herein are the original work of Jo W. French
 * and are placed in the public domain.
 *
 * Modification history:
 * ---------------------
 *
 *    Rev 1.3   28 Sep 1992 00:24:54   GLENN
 * Jo French clean up.
 *
 *    Rev 1.2   15 Aug 1991 23:02:30   GLENN
 * Forest Belt proofread/edited/cleaned up doc
 *
 *    Rev 1.1   14 Jun 1991 19:50:42   GLENN
 * Minor edit to file header
 *
 *    Rev 1.0   01 Apr 1991 01:00:24   GLENN
 * Nanforum Toolkit
 *
 */

/*  $DOC$
 *  $FUNCNAME$
 *     FT_ACCTMONTH()
 *  $CATEGORY$
 *     Date/Time
 *  $ONELINER$
 *     Return accounting month data
 *  $SYNTAX$
 *     FT_ACCTMONTH( [ <dGivenDate> ], [ <nMonthNum> ] ) -> aDateInfo
 *  $ARGUMENTS$
 *     <dGivenDate> is any valid date in any date format.  Defaults
 *     to current system date if not supplied.
 *
 *     <nMonthNum> is a number from 1 to 12 signifying a month.
 *     Defaults to current month if not supplied.
 *  $RETURNS$
 *     A three element array containing the following data:
 *
 *        aDateInfo[1] - The year and month as a character string "YYYYMM"
 *        aDateInfo[2] - The beginning date of the accounting month
 *        aDateInfo[3] - The ending date of the accounting month
 *  $DESCRIPTION$
 *     FT_ACCTMONTH() creates an array containing data about the
 *     accounting month containing the given date.
 *
 *     An accounting period has the following characteristics:
 *
 *     If the first week of the period contains 4 or more 'work'
 *     days, it is included in the period; otherwise, the first
 *     week was included in the prior period.
 *
 *     If the last week of the period contains 4 or more 'work'
 *     days it is included in the period; otherwise, the last week
 *     is included in the next period.  This results in 13 week
 *     'quarters' and 4 or 5 week 'months'.  Every 5 or 6 years, a
 *     'quarter' will contain 14 weeks and the year will contain 53
 *     weeks.
 *  $EXAMPLES$
 *     // get info about accounting month containing 9/15/90
 *     aDateInfo := FT_ACCTMONTH( Ctod("09/15/90") )
 *     ? aDateInfo[1]   //  199009       (9th month)
 *     ? aDateInfo[2]   //  09/02/90     beginning of month 9
 *     ? aDateInfo[3]   //  09/29/90     end of month 9
 *
 *     // get info about accounting month 5 in year containing 9/15/90
 *     aDateInfo := FT_ACCTMONTH( Ctod("09/15/90"), 5 )
 *     ? aDateInfo[1]   //  199005
 *     ? aDateInfo[2]   //  04/29/89   beginning of month 5
 *     ? aDateInfo[3]   //  06/02/90   end of month 5
 *  $SEEALSO$
 *     FT_DATECNFG() FT_ACCTWEEK() FT_ACCTQTR() FT_ACCTYEAR()
 *  $END$
*/
 
FUNCTION FT_ACCTMONTH( dGivenDate, nMonthNum )

   LOCAL nYTemp, nMTemp, lIsMonth, aRetVal
 
   IF ! ( ValType( dGivenDate ) $ 'ND' )
      dGivenDate := Date()
   ELSEIF ValType( dGivenDate ) == 'N'
      nMonthNum := dGivenDate
      dGivenDate := Date()
   ENDIF
 
   aRetVal := FT_MONTH( dGivenDate )
   nYTemp := Val( SubStr( aRetVal[1],1,4 ) )
   nMTemp := Val( SubStr( aRetVal[1],5,2 ) )
   aRetVal[2] := FT_ACCTADJ( aRetVal[2] )
   aRetVal[3] := FT_ACCTADJ( aRetVal[3], .T. )
 
   IF dGivenDate < aRetVal[2]
      dGivenDate := FT_MADD( dGivenDate, - 1 )
      aRetVal    := FT_MONTH( dGivenDate )
      nMTemp     -= 1
      IF nMTemp  == 0
         nYTemp -= 1
         nMTemp := 12
      ENDIF
      aRetVal[2] := FT_ACCTADJ( aRetVal[2] )
      aRetVal[3] := FT_ACCTADJ( aRetVal[3], .T. )
 
   ELSEIF dGivenDate > aRetVal[3]
 
      dGivenDate := FT_MADD( dGivenDate, 1 )
      aRetVal    := FT_MONTH( dGivenDate )
      nMTemp     += 1
      IF nMTemp == 13
         nYTemp += 1
         nMTemp := 1
      ENDIF
      aRetVal[2] := FT_ACCTADJ( aRetVal[2] )
      aRetVal[3] := FT_ACCTADJ( aRetVal[3], .T. )
 
   ENDIF
 
   lIsMonth := ( ValType( nMonthNum ) == 'N' )
   IF lIsMonth
      IF( nMonthNum < 1 .OR. nMonthNum > 12 , nMonthNum := 12, )
         aRetVal    := FT_MONTH( dGivenDate, nMonthNum )
         nYTemp     := Val( SubStr( aRetVal[1],1,4 ) )
         nMTemp     := Val( SubStr( aRetVal[1],5,2 ) )
         aRetVal[2] := FT_ACCTADJ( aRetVal[2] )
         aRetVal[3] := FT_ACCTADJ( aRetVal[3], .T. )
      ENDIF
 
      aRetVal[1] := Str( nYTemp, 4 ) + PadL( LTrim( Str(nMTemp,2 ) ), 2, '0' )
 
      RETURN aRetVal


