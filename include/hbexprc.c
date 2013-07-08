/*
 * $Id: hbexprc.c 9885 2012-12-16 10:13:49Z andijahja $
 */

/*
 * Harbour Project source code:
 * Compiler Expression Optimizer - utilities
 *
 * Copyright 1999 Ryszard Glab
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

/* TOFIX: Split the code, since MSC8 can't compile it, even in Huge model. */

/* TODO:
 *    - Correct post- and pre- operations to correctly handle the following code
 *    a[ i++ ]++
 *    Notice: in current implementation (an in Clipper too) 'i++' is evaluated
 *    two times! This causes that the new value (after incrementation) is
 *    stored in next element of the array.
 */

#include <math.h>

#include "hbcomp.h"
#include "hbmacro.ch"
#include "hbpcode.h"

#include "hbexemem.h"

#if defined( __HB_COMPILER__ )
   #include "hbcomp.h"
#endif

extern int hb_compLocalGetPos( char * szVarName );   /* returns the order + 1 of a local variable */

/* ************************************************************************* */

#if defined( HB_MACRO_SUPPORT )
void hb_compExprDelOperator( HB_EXPR_PTR pExpr, HB_MACRO_DECL )
#else
void hb_compExprDelOperator( HB_EXPR_PTR pExpr )
#endif
{
   if( pExpr->value.asOperator.pLeft )
      HB_EXPR_PCODE1( hb_compExprDelete, pExpr->value.asOperator.pLeft );

   if( pExpr->value.asOperator.pRight )
      HB_EXPR_PCODE1( hb_compExprDelete, pExpr->value.asOperator.pRight );
}

/* Generates pcodes for compound operators    += -= *= /= %= ^=
 *
 * pExpr is an expression created by hb_compExprNew<operator>Eq functions
 */
#if defined( HB_MACRO_SUPPORT )
   void hb_compExprPushOperEq( HB_EXPR_PTR pSelf, HB_PCODE bOpEq, HB_MACRO_DECL )
#else
   void hb_compExprPushOperEq( HB_EXPR_PTR pSelf, HB_PCODE bOpEq )
#endif
{
   /* NOTE: an object instance variable needs special handling
    */
   if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND || pSelf->value.asOperator.pLeft->ExprType == HB_ET_WITHSEND )
   {
      HB_EXPR_PTR pObj = pSelf->value.asOperator.pLeft;

      /* Push _message for the later assignment.  */
      HB_EXPR_PCODE1( hb_compGenMessageData, pObj->value.asMessage.szMessage );

      /* Push object */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

      /* Now push current value of variable */
      HB_EXPR_PCODE1( hb_compGenMessage, pObj->value.asMessage.szMessage );

      if( pObj->ExprType == HB_ET_SEND )
      {
         /* Push object */
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );

         /* Do it. */
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 0, ( BOOL ) 1 );
      }
      else
      {
         /* Push object */
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

         /* Do it. */
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 0, ( BOOL ) 1 );
      }

/* NOTE: COMPATIBILITY ISSUE:
 *  The above HB_C52_STRICT setting determines
 * the way the chained send messages are handled.
 * For example, the following code:
 *
 * a:b( COUNT() ):c += 1
 *
 * will be handled as:
 *
 * a:b( COUNT() ):c := a:b( COUNT() ):c + 1
 *
 * in strict Clipper compatibility mode and
 *
 * temp := a:b( COUNT() ), temp:c += 1
 *
 * in non-strict mode.
 *   In practice in Clipper it will call COUNT() function two times: the
 * first time before addition and the second one after addition - in Harbour,
 * COUNT() function will be called only once, before addition.
 * The Harbour (non-strict) method is:
 * 1) faster
 * 2) it guarantees that the same instance variable of the same object will
 *   be changed
 */

      /* push increment value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* increase operation */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, ( BYTE ) bOpEq );

      if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND )
         /* Now do the assignment - call pop message with one argument */
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 1, ( BOOL ) 1 );
      else
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 1, ( BOOL ) 1 );
   }

   /* TODO: add a special code for arrays to correctly handle a[ i++ ]++
    */
   else if( bOpEq == HB_P_PLUS && pSelf->value.asOperator.pLeft->ExprType == HB_ET_ARRAYAT )
   {
      /* printf( "Optimize ArrayAt\n" ); */
      pSelf->value.asOperator.pLeft->value.asList.PopOp = bOpEq;

      /* push argument value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* apply operator and pop the new value into variable and remove it from the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );

      /* push the the new value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      return;
   }
   else
   {
#if defined( HB_MACRO_SUPPORT )
       /* This optimization is not applicable in Macro Compiler. */
#else
      if( bOpEq == HB_P_PLUS || bOpEq == HB_P_MINUS )
      {
         int iIncrement = 0, iLocal;
         BOOL bShort;

         if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_VARIABLE )
            iLocal = hb_compLocalGetPos( pSelf->value.asOperator.pLeft->value.asSymbol.szName );
         else
            iLocal = 0;

         if( pSelf->value.asOperator.pRight->ExprType == HB_ET_NUMERIC &&
             pSelf->value.asOperator.pRight->value.asNum.NumType == HB_ET_LONG &&
             pSelf->value.asOperator.pRight->value.asNum.lVal >= -32768 &&
             pSelf->value.asOperator.pRight->value.asNum.lVal <= 32767 )
         {
            bShort = TRUE;
            iIncrement = ( short ) pSelf->value.asOperator.pRight->value.asNum.lVal;

            if( bOpEq == HB_P_MINUS )
               iIncrement = -iIncrement;
         }
         else
            bShort = FALSE;

         if( iLocal && HB_LIM_INT8( iLocal ) )
         {
            if( bShort )
            {
               /* No need to generate ( X + 0 ) but *** Danger! *** of not being Error Compatible! */
               if( iIncrement == 0 )
                  hb_compGenPCode2( HB_P_PUSHLOCALNEAR, ( BYTE ) iLocal, ( BOOL ) 0 );
               else if( iIncrement == 1 )
                  hb_compGenPCode2( HB_P_PUSHLOCALNEARINC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else if( iIncrement == -1 )
                  hb_compGenPCode2( HB_P_PUSHLOCALNEARDEC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else
               {
                  hb_compGenPCode4( HB_P_LOCALNEARADDINT, ( BYTE ) iLocal, HB_LOBYTE( iIncrement ), HB_HIBYTE( iIncrement ), ( BOOL ) 0 );

                  /* HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );
                   */
                  hb_compGenPCode2( HB_P_PUSHLOCALNEAR, ( BYTE ) iLocal, ( BOOL ) 0 );
               }

               return;
            }
            else
            {
               if( bOpEq == HB_P_MINUS )
               {
                  if( pSelf->value.asOperator.pRight->ExprType == HB_ET_NUMERIC && pSelf->value.asOperator.pRight->value.asNum.NumType == HB_ET_LONG )
                  {
                     pSelf->value.asOperator.pRight->value.asNum.lVal = -pSelf->value.asOperator.pRight->value.asNum.lVal;
                     bOpEq = HB_P_PLUS;
                  }
               }

               if( bOpEq == HB_P_PLUS )
               {
                  HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

                  HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_LOCALNEARADD, ( BYTE ) iLocal, ( BOOL ) 0 );

                  /* HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );
                   */
                  hb_compGenPCode2( HB_P_PUSHLOCALNEAR, ( BYTE ) iLocal, ( BOOL ) 0 );

                  return;
               }
            }
         }
         else if( bShort )
         {
            HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

            /* No need to generate ( X + 0 ) but *** Danger! *** of not being Error Compatible! */
            if( iIncrement )
               hb_compGenPCode3( HB_P_ADDINT, HB_LOBYTE( iIncrement ), HB_HIBYTE( iIncrement ), ( BOOL ) 0 );

            HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_DUPLICATE );

            /* pop the new value into variable and leave the copy on the stack */
            HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );

            return;
         }
      }

#endif

      /* push old value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* push argument value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* perform operation and duplicate the new value */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, ( BYTE ) bOpEq );
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_DUPLICATE );

      /* pop the new value into variable and leave the copy on the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );
   }
}

/* Generates pcodes for <operator>= syntax
 * used standalone as a statement (it cannot leave the value on the stack)
 */
#if defined( HB_MACRO_SUPPORT )
void hb_compExprUseOperEq( HB_EXPR_PTR pSelf, HB_PCODE bOpEq, HB_MACRO_DECL )
#else
void hb_compExprUseOperEq( HB_EXPR_PTR pSelf, HB_PCODE bOpEq )
#endif
{
   /* NOTE: an object instance variable needs special handling
    */
   if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND || pSelf->value.asOperator.pLeft->ExprType == HB_ET_WITHSEND )
   {
      HB_EXPR_PTR pObj = pSelf->value.asOperator.pLeft;

      /* Push _message for the later assignment.  */
      HB_EXPR_PCODE1( hb_compGenMessageData, pObj->value.asMessage.szMessage );

      /* Push object */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

      /* Now push current value of variable */
      HB_EXPR_PCODE1( hb_compGenMessage, pObj->value.asMessage.szMessage );

      if( pObj->ExprType == HB_ET_SEND )
      {
         /* Push object */
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );

         /* Do it.*/
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 0, ( BOOL ) 1 );
      }
      else
      {
         /* Push object */
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

         /* Do it.*/
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 0, ( BOOL ) 1 );
      }

      /* push argument value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* Operator */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, ( BYTE ) bOpEq );

      /* Now do the assignment - call pop message with one argument */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 1, ( BOOL ) 1 );
      else
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 1, ( BOOL ) 1 );

      /* pop the unneeded value from the stack */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_POP );
   }
   else if( bOpEq == HB_P_PLUS && pSelf->value.asOperator.pLeft->ExprType == HB_ET_ARRAYAT )
   {
      /* printf( "Optimize ArrayAt\n" ); */
      pSelf->value.asOperator.pLeft->value.asList.PopOp = bOpEq;

      /* push argument value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* apply operator and pop the new value into variable and remove it from the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );

      return;
   }
   else
   {
#if defined( HB_MACRO_SUPPORT )
       /* This optimization is not applicable in Macro Compiler. */
#else
      if( bOpEq == HB_P_PLUS || bOpEq == HB_P_MINUS )
      {
         int iIncrement = 0, iLocal;
         BOOL bShort;

         if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_VARIABLE )
            iLocal = hb_compLocalGetPos( pSelf->value.asOperator.pLeft->value.asSymbol.szName );
         else
            iLocal = 0;

         if( pSelf->value.asOperator.pRight->ExprType == HB_ET_NUMERIC &&
             pSelf->value.asOperator.pRight->value.asNum.NumType == HB_ET_LONG &&
             pSelf->value.asOperator.pRight->value.asNum.lVal >= -32768 &&
             pSelf->value.asOperator.pRight->value.asNum.lVal <= 32767 )
         {
            bShort = TRUE;
            iIncrement = ( short ) pSelf->value.asOperator.pRight->value.asNum.lVal;

            if( bOpEq == HB_P_MINUS )
               iIncrement = -iIncrement;
         }
         else
            bShort = FALSE;

         if( iLocal && HB_LIM_INT8( iLocal ) )
         {
            if( bShort )
            {
               /* No need to generate ( X + 0 ) but *** Danger! *** of not being Error Compatible!
                */
               if( iIncrement == 0 )
               {
               }
               else if( iIncrement == 1 )
                  hb_compGenPCode2( HB_P_LOCALNEARINC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else if( iIncrement == -1 )
                  hb_compGenPCode2( HB_P_LOCALNEARDEC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else
                  hb_compGenPCode4( HB_P_LOCALNEARADDINT, ( BYTE ) iLocal, HB_LOBYTE( iIncrement ), HB_HIBYTE( iIncrement ), ( BOOL ) 0 );

               return;
            }
            else
            {
               if( bOpEq == HB_P_MINUS )
               {
                  if( pSelf->value.asOperator.pRight->ExprType == HB_ET_NUMERIC && pSelf->value.asOperator.pRight->value.asNum.NumType == HB_ET_LONG )
                  {
                     pSelf->value.asOperator.pRight->value.asNum.lVal = -pSelf->value.asOperator.pRight->value.asNum.lVal;
                     bOpEq = HB_P_PLUS;
                  }
               }

               if( bOpEq == HB_P_PLUS )
               {
                  HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

                  HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_LOCALNEARADD, ( BYTE ) iLocal, ( BOOL ) 0 );

                  return;
               }
            }
         }
         else if( bShort )
         {
            HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

            /* No need to generate ( X + 0 ) but *** Danger! *** of not being Error Compatible!
             */
            if( iIncrement )
               hb_compGenPCode3( HB_P_ADDINT, HB_LOBYTE( iIncrement ), HB_HIBYTE( iIncrement ), ( BOOL ) 0 );

            /* pop the new value into variable and leave the copy on the stack
             */
            HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );

            return;
         }
      }

#endif

      /* push old value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* push argument value */
      HB_EXPR_USE( pSelf->value.asOperator.pRight, HB_EA_PUSH_PCODE );

      /* Operator */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, ( BYTE ) bOpEq );

      /* pop the new value into variable and remove it from the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );
   }
}

/* Generates the pcodes for pre- increment/decrement expressions
 */
#if defined( HB_MACRO_SUPPORT )
void hb_compExprPushPreOp( HB_EXPR_PTR pSelf, BYTE bOper, HB_MACRO_DECL )
#else
void hb_compExprPushPreOp( HB_EXPR_PTR pSelf, BYTE bOper )
#endif
{
   /* NOTE: an object instance variable needs special handling
    */
   if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND || pSelf->value.asOperator.pLeft->ExprType == HB_ET_WITHSEND )
   {
      HB_EXPR_PTR pObj = pSelf->value.asOperator.pLeft;

      /* Push _message for later use */
      HB_EXPR_PCODE1( hb_compGenMessageData, pObj->value.asMessage.szMessage );

      /* Push object */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

      /* Now push current value of variable */
      HB_EXPR_PCODE1( hb_compGenMessage, pObj->value.asMessage.szMessage );

      /* Push object */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_USE( pObj->value.asMessage.pObject, HB_EA_PUSH_PCODE );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_PUSHWITH );

      /* Do it. */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 0, ( BOOL ) 1 );
      else
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 0, ( BOOL ) 1 );

      /* increase/decrease operation */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, bOper );

      /* Now, do the assignment - call pop message with one argument - it leaves the value on the stack */
      if( pObj->ExprType == HB_ET_SEND )
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDSHORT, 1, ( BOOL ) 1 );
      else
         HB_EXPR_GENPCODE2( hb_compGenPCode2, HB_P_SENDWITHSHORT, 1, ( BOOL ) 1 );
   }
   else
   {
#if defined( HB_MACRO_SUPPORT )
       /* This optimization is not applicable in Macro Compiler. */
#else
      if( bOper == HB_P_INC || bOper == HB_P_DEC )
      {
         int iLocal;

         if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_VARIABLE )
         {
            iLocal = hb_compLocalGetPos( pSelf->value.asOperator.pLeft->value.asSymbol.szName );

            if( iLocal && HB_LIM_INT8( iLocal ) )
            {
               if( bOper == HB_P_INC )
                  hb_compGenPCode2( HB_P_PUSHLOCALNEARINC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else
                  hb_compGenPCode2( HB_P_PUSHLOCALNEARDEC, ( BYTE ) iLocal, ( BOOL ) 0 );

               return;
            }
         }
      }
#endif
      /* Push current value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* Increment */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, bOper );

      /* duplicate a value */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_DUPLICATE );

      /* pop new value and leave the duplicated copy of it on the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );
   }
}

/* Generates the pcodes for post- increment/decrement expressions
 */
#if defined( HB_MACRO_SUPPORT )
void hb_compExprPushPostOp( HB_EXPR_PTR pSelf, BYTE bOper, HB_MACRO_DECL )
#else
void hb_compExprPushPostOp( HB_EXPR_PTR pSelf, BYTE bOper )
#endif
{
   /* NOTE: an object instance variable needs special handling
    */
   if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND || pSelf->value.asOperator.pLeft->ExprType == HB_ET_WITHSEND )
   {
      /* push current value - it will be a result of whole expression */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* now increment the value */
      HB_EXPR_PCODE2( hb_compExprPushPreOp, pSelf, bOper );

      /* pop the value from the stack */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_POP );
   }
   else
   {
#if defined( HB_MACRO_SUPPORT )
       /* This optimization is not applicable in Macro Compiler. */
#else
      if( bOper == HB_P_INC || bOper == HB_P_DEC )
      {
         int iLocal;

         if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_VARIABLE )
         {
            iLocal = hb_compLocalGetPos( pSelf->value.asOperator.pLeft->value.asSymbol.szName );

            if( iLocal && HB_LIM_INT8( iLocal ) )
            {
               /* First push the original value.
                * HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );
                */
               hb_compGenPCode2( HB_P_PUSHLOCALNEAR, ( BYTE ) iLocal, ( BOOL ) 0 );

               /* Inc/Dec without leaving value on stack. */
               if( bOper == HB_P_INC )
                  hb_compGenPCode2( HB_P_LOCALNEARINC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else
                  hb_compGenPCode2( HB_P_LOCALNEARDEC, ( BYTE ) iLocal, ( BOOL ) 0 );

               return;
            }
         }
      }
#endif
      /* Push current value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* Duplicate value */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_DUPLICATE );

      /* Increment */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, bOper );

      /* pop new value from the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );
   }
}

/* Generates the pcodes for increment/decrement operations
 * used standalone as a statement
 */
#if defined( HB_MACRO_SUPPORT )
void hb_compExprUsePreOp( HB_EXPR_PTR pSelf, BYTE bOper, HB_MACRO_DECL )
#else
void hb_compExprUsePreOp( HB_EXPR_PTR pSelf, BYTE bOper )
#endif
{
   /* NOTE: an object instance variable needs special handling
    */
   if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_SEND || pSelf->value.asOperator.pLeft->ExprType == HB_ET_WITHSEND )
   {
      HB_EXPR_PCODE2( hb_compExprPushPreOp, pSelf, bOper );

      /* pop the value from the stack */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_POP );
   }
   else
   {
#if defined( HB_MACRO_SUPPORT )
       /* This optimization is not applicable in Macro Compiler. */
#else
      if( bOper == HB_P_INC || bOper == HB_P_DEC )
      {
         int iLocal;

         if( pSelf->value.asOperator.pLeft->ExprType == HB_ET_VARIABLE )
         {
            iLocal = hb_compLocalGetPos( pSelf->value.asOperator.pLeft->value.asSymbol.szName );

            if( iLocal && HB_LIM_INT8( iLocal ) )
            {
               if( bOper == HB_P_INC )
                  hb_compGenPCode2( HB_P_LOCALNEARINC, ( BYTE ) iLocal, ( BOOL ) 0 );
               else
                  hb_compGenPCode2( HB_P_LOCALNEARDEC, ( BYTE ) iLocal, ( BOOL ) 0 );

               return;
            }
         }
      }
#endif

      /* Push current value */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_PUSH_PCODE );

      /* Increment */
      HB_EXPR_GENPCODE1( hb_compGenPCode1, bOper );

      /* pop new value from the stack */
      HB_EXPR_USE( pSelf->value.asOperator.pLeft, HB_EA_POP_PCODE );
   }
}

/* Generate pcode for aliased expression which contains macro operator on
 * the left or right side of the alias operator
 * expression->&macro or &macro->expression or &macro->&macro
 */
#if defined( HB_MACRO_SUPPORT )
void hb_compExprUseAliasMacro( HB_EXPR_PTR pAliasedVar, BYTE bAction, HB_MACRO_DECL )
#else
void hb_compExprUseAliasMacro( HB_EXPR_PTR pAliasedVar, BYTE bAction )
#endif
{
   HB_EXPR_PTR pAlias, pVar;

   /* Alias->Var
    */
   pAlias = pAliasedVar->value.asAlias.pAlias;
   pVar   = pAliasedVar->value.asAlias.pVar;
   if( pAlias->ExprType == HB_ET_ALIAS )
   {
      /* database alias */
      /* Push alias identifier as string so it can be joined with
       * variable at runtime
       * NOTE:
       *    ALIAS->&var is the same as &( "ALIAS->" + var )
       *
       */
      HB_EXPR_PCODE2( hb_compGenPushString, pAlias->value.asSymbol.szName, strlen( pAlias->value.asSymbol.szName ) + 1 );
      HB_EXPR_USE( pVar, HB_EA_PUSH_PCODE );

      if( bAction == HB_EA_PUSH_PCODE )
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPUSHALIASED );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPOPALIASED );
   }
   else if( pVar->ExprType == HB_ET_VARIABLE )
   {
      /* NOTE:
       *    &macro->var is the  same as: &( macro + "->var" )
       */
      HB_EXPR_USE( pAlias, HB_EA_PUSH_PCODE );
      HB_EXPR_PCODE2( hb_compGenPushString, pVar->value.asSymbol.szName, strlen( pVar->value.asSymbol.szName ) + 1 );

      if( bAction == HB_EA_PUSH_PCODE )
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPUSHALIASED );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPOPALIASED );
   }
   else
   {
      HB_EXPR_USE( pAlias, HB_EA_PUSH_PCODE );
      HB_EXPR_USE( pVar, HB_EA_PUSH_PCODE );
      if( bAction == HB_EA_PUSH_PCODE )
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPUSHALIASED );
      else
         HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_P_MACROPOPALIASED );
   }

   /* Always add add byte to pcode indicating requested macro compiler flag. */
#if defined( HB_MACRO_SUPPORT )
   HB_EXPR_GENPCODE1( hb_compGenPCode1, HB_COMPFLAG_RT_MACRO );
#else
   HB_EXPR_GENPCODE1( hb_compGenPData1,
        (
          ( hb_comp_Supported & HB_COMPFLAG_HARBOUR  ? HB_SM_HARBOUR   : 0 ) |
          ( hb_comp_Supported & HB_COMPFLAG_XBASE    ? HB_SM_XBASE     : 0 ) |
          ( hb_comp_bShortCuts                       ? HB_SM_SHORTCUTS : 0 ) |
          ( hb_comp_Supported & HB_COMPFLAG_RT_MACRO ? HB_SM_RT_MACRO  : 0 )
        )
      );
#endif
}

/* Reduces the list of expressions
 *
 * pExpr is the first expression on the list
 */
#if defined( HB_MACRO_SUPPORT )
ULONG hb_compExprReduceList( HB_EXPR_PTR pExpr, HB_MACRO_DECL )
#else
ULONG hb_compExprReduceList( HB_EXPR_PTR pExpr )
#endif
{
   HB_EXPR_PTR pNext;
   HB_EXPR_PTR * pPrev;
   ULONG ulCnt = 0;

   /* NOTE: During optimalization an expression on the list can be
    * replaced by the new one
    */

   pPrev = &pExpr->value.asList.pExprList;
   pExpr = pExpr->value.asList.pExprList;
   while( pExpr )
   {
      pNext  = pExpr->pNext; /* store next expression in case the current  will be reduced */
      pExpr  = HB_EXPR_USE( pExpr, HB_EA_REDUCE );
      *pPrev = pExpr;   /* store a new expression into the previous one */
      pExpr->pNext = pNext;  /* restore the link to next expression */
      pPrev  = &pExpr->pNext;
      pExpr  = pNext;
      ++ulCnt;
   }
   return ulCnt;
}

BOOL hb_compExprCheckMacroVar( char * szText )
{
   char * pTmp = szText;
   BOOL bTextSubst;
   BOOL bMacroText = FALSE;

   while( ( pTmp = strchr( pTmp, '&' ) ) != NULL )
   {
      /* Check if macro operator is used inside a string
       * Macro operator is ignored if it is the last char or
       * next char is '(' e.g. "this is &(ignored)"
       *
       * NOTE: This uses _a-zA-Z pattern to check for
       * variable name beginning
       */

      ++pTmp;
      bTextSubst = ( *pTmp == '_' || (*pTmp >= 'A' && *pTmp <= 'Z') || (*pTmp >= 'a' && *pTmp <= 'z') );
      /* NOTE: All variables are assumed memvars in macro compiler -
       * there is no need to check for a valid name
       */
      if( bTextSubst )
      {
#if defined( HB_MACRO_SUPPORT )
         HB_SYMBOL_UNUSED( bMacroText );
         return TRUE;    /*there is no need to check all '&' occurences */
#else
         /* There is a valid character after '&' that can be used in
          * variable name - check if the whole variable name is valid
          * (local, static and field  variable names are invalid because
          * they are not visible at runtime)
          */
         char * pStart = pTmp;
         char cSave;

         /* NOTE: This uses _a-zA-Z0-9 pattern to check for
          * variable name
          */
         while( *pTmp && (*pTmp == '_' || (*pTmp >= 'A' && *pTmp <= 'Z') || (*pTmp >= 'a' && *pTmp <= 'z') || (*pTmp >= '0' && *pTmp <= '9')) )
            ++pTmp;

         cSave = *pTmp;
         *pTmp = '\0';

         hb_compVariableMacroCheck( pStart );

         *pTmp      = cSave;
         bMacroText = TRUE;
#endif
      }
   }
   return bMacroText;
}

/* Reduces the list of expressions
 *
 * pExpr is the first expression on the list
 */
HB_EXPR_PTR hb_compExprReducePlusStrings( HB_EXPR_PTR pLeft, HB_EXPR_PTR pRight, HB_MACRO_DECL )
{
#if defined( HB_MACRO_SUPPORT )

   pLeft->value.asString.string = (char *) hb_xrealloc( pLeft->value.asString.string, pLeft->ulLength + pRight->ulLength + 1 );
   pLeft->value.asString.dealloc = TRUE;
   HB_MEMCPY( pLeft->value.asString.string + pLeft->ulLength, pRight->value.asString.string, (size_t) pRight->ulLength );
   pLeft->ulLength += pRight->ulLength;
   pLeft->value.asString.string[ pLeft->ulLength ] = '\0';
   hb_compExprFree( pRight, HB_MACRO_PARAM );

#else
   /* NOTE: compiler uses the hash table for storing identifiers and literals
    * Strings passed for reduction can be referenced by other expressions
    * then we cannot resize them or deallocate
   */
   char *szString;

   szString = (char *) hb_xgrab( pLeft->ulLength + pRight->ulLength + 1 );
   HB_MEMCPY( szString, pLeft->value.asString.string, (size_t) pLeft->ulLength );
   HB_MEMCPY( szString + pLeft->ulLength, pRight->value.asString.string, (size_t) pRight->ulLength );
   pLeft->ulLength += pRight->ulLength;
   szString[ pLeft->ulLength ] = '\0';

   if( pLeft->value.asString.dealloc )
      hb_xfree( pLeft->value.asString.string );

   pLeft->value.asString.string = szString;
   pLeft->value.asString.dealloc = TRUE;

   hb_compExprFree( pRight, HB_MACRO_PARAM );

#endif

   HB_SYMBOL_UNUSED( pMacro );    /* to suppress BCC warning */

   return pLeft;
}
