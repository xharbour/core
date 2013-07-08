/*
 * $Id: hbxvm.h 9406 2011-11-10 14:56:33Z andijahja $
 */

/*
 * Harbour Project source code:
 * Header file for the Internal Terminal API
 *
 * Copyright 2006 Przemyslaw Czerpak < druzus /at/ priv.onet.pl >
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

/* NOTE: The declaration of hb_gtSetPos(), hb_gtGetPos(), hb_gtWrite(),
         hb_gtWriteAt(), hb_gtRepChar(), hb_gtBox(), hb_gtBoxS(), hb_gtBoxD()
         hb_gtInit() differs in parameter types from the original CA-Cl*pper
         versions. [vszakats] */

#ifndef HB_XVM_H_
#define HB_XVM_H_

#include "hbapi.h"
#include "hbapiitm.h"
#include "hbfast.h"
#include "hbvm.h"
#include "hbstack.h"

HB_EXTERN_BEGIN

/*
 * Functions below which return BOLL value can cause error handler
 * executing so we have to check for break request flag. The return
 * value is TRUE when break request is set.
 * other functions does not execute error handler and we do not have
 * to check HVM state so they are simply declared as void.
 */

extern HB_EXPORT void   hb_xvmExitProc( void );
extern HB_EXPORT void   hb_xvmSeqBegin( void );
extern HB_EXPORT BOOL   hb_xvmSeqEnd( void );
extern HB_EXPORT BOOL   hb_xvmSeqRecover( void );

extern HB_EXPORT void   hb_xvmSetLine( USHORT uiLine );           /* set .prg line number information */
extern HB_EXPORT void   hb_xvmBaseLine( USHORT uiLine );          /* set .prg line number information */
extern HB_EXPORT void   hb_xvmLineOffset( BYTE bLine );           /* set .prg line number information */

extern HB_EXPORT void   hb_xvmFrame( int iLocals, int iParams );  /* increases the stack pointer for the amount of locals and params suplied */
extern HB_EXPORT void   hb_xvmSFrame( PHB_SYMB pSymbol );
extern HB_EXPORT void   hb_xvmStatics( PHB_SYMB pSymbol, USHORT uiStatics );
extern HB_EXPORT void   hb_xvmParameter( PHB_SYMB pSymbol, int iParams );
extern HB_EXPORT void   hb_xvmRetValue( void );                   /* pops the latest stack value into stack.Return */
extern HB_EXPORT BOOL   hb_xvmDo( USHORT uiParams );
extern HB_EXPORT BOOL   hb_xvmFunction( USHORT uiParams );
extern HB_EXPORT BOOL   hb_xvmSend( USHORT uiParams );
extern HB_EXPORT void   hb_xvmPushStatic( USHORT uiStatic );
extern HB_EXPORT void   hb_xvmPushStaticByRef( USHORT uiStatic );
extern HB_EXPORT void   hb_xvmPopStatic( USHORT uiStatic );
extern HB_EXPORT void   hb_xvmPushGlobal( USHORT uiGlobal, PHB_ITEM** pGlobals );
extern HB_EXPORT void   hb_xvmPushGlobalByRef( USHORT uiGlobal, PHB_ITEM** pGlobals );
extern HB_EXPORT void   hb_xvmPopGlobal( USHORT uiGlobal, PHB_ITEM** pGlobals );
extern HB_EXPORT BOOL   hb_xvmPushVariable( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPopVariable( PHB_SYMB pSymbol );
extern HB_EXPORT void   hb_xvmPushBlock( const BYTE * pCode, USHORT usSize ); /* creates a codeblock */
extern HB_EXPORT void   hb_xvmPushBlockShort( const BYTE * pCode, USHORT usSize ); /* creates a codeblock */
extern HB_EXPORT void   hb_xvmPushSelf( void );
extern HB_EXPORT void   hb_xvmPushLocal( SHORT iLocal );          /* pushes the containts of a local onto the stack */
extern HB_EXPORT void   hb_xvmPushLocalByRef( SHORT iLocal );     /* pushes a local by refrence onto the stack */
extern HB_EXPORT void   hb_xvmPopLocal( SHORT iLocal );           /* pops the stack latest value onto a local */
extern HB_EXPORT BOOL   hb_xvmPushField( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPopField( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPushMemvar( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPushMemvarByRef( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPopMemvar( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPushAliasedField( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPopAliasedField( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPushAliasedVar( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPopAliasedVar( PHB_SYMB pSymbol );
extern HB_EXPORT BOOL   hb_xvmPushAlias( void );
extern HB_EXPORT BOOL   hb_xvmPopAlias( void );                   /* select the workarea using a given item or a substituted value */
extern HB_EXPORT BOOL   hb_xvmPopLogical( BOOL * );               /* pops the stack latest value and returns its logical value */
extern HB_EXPORT BOOL   hb_xvmSwapAlias( void );                  /* swaps items on the eval stack and pops the workarea number */
extern HB_EXPORT BOOL   hb_xvmAddInt( LONG lAdd );                 /* add integer to top stack item */
extern HB_EXPORT BOOL   hb_xvmLocalAddInt( int iLocal, LONG lAdd ); /* add integer to given local variable */
extern HB_EXPORT BOOL   hb_xvmLocalAdd( int iLocal );

extern HB_EXPORT BOOL   hb_xvmAnd( void );
extern HB_EXPORT BOOL   hb_xvmOr( void );
extern HB_EXPORT BOOL   hb_xvmNot( void );
extern HB_EXPORT BOOL   hb_xvmNegate( void );
extern HB_EXPORT BOOL   hb_xvmPower( void );
extern HB_EXPORT void   hb_xvmDuplicate( void );
extern HB_EXPORT void   hb_xvmDuplTwo( void );
extern HB_EXPORT BOOL   hb_xvmForTest( void );
extern HB_EXPORT void   hb_xvmFuncPtr( void );
extern HB_EXPORT BOOL   hb_xvmEqual( BOOL fExact );               /* checks if the two latest values on the stack are equal, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmNotEqual( void );                   /* checks if the two latest values on the stack are not equal, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmLess( void );                       /* checks if the latest - 1 value is less than the latest, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmLessEqual( void );                  /* checks if the latest - 1 value is less than or equal the latest, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmGreater( void );                    /* checks if the latest - 1 value is greater than the latest, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmGreaterEqual( void );               /* checks if the latest - 1 value is greater than or equal the latest, removes both and leaves result */
extern HB_EXPORT BOOL   hb_xvmInstring( void );                   /* check whether string 1 is contained in string 2 */
extern HB_EXPORT BOOL   hb_xvmPlus( void );                       /* sums the latest two values on the stack, removes them and leaves the result */
extern HB_EXPORT BOOL   hb_xvmMinus( void );                      /* substracts the latest two values on the stack, removes them and leaves the result */
extern HB_EXPORT BOOL   hb_xvmMult( void );                       /* multiplies the latest two values on the stack, removes them and leaves the result */
extern HB_EXPORT BOOL   hb_xvmDivide( void );                     /* divides the latest two values on the stack, removes them and leaves the result */
extern HB_EXPORT BOOL   hb_xvmModulus( void );                    /* calculates the modulus of latest two values on the stack, removes them and leaves the result */
extern HB_EXPORT BOOL   hb_xvmInc( void );
extern HB_EXPORT BOOL   hb_xvmDec( void );

extern HB_EXPORT void   hb_xvmArrayDim( USHORT uiDimensions );    /* generates an uiDimensions Array and initialize those dimensions from the stack values */
extern HB_EXPORT void   hb_xvmArrayGen( ULONG ulElements );       /* generates an ulElements Array and fills it from the stack values */
extern HB_EXPORT BOOL   hb_xvmArrayPush( void );                  /* pushes an array element to the stack, removing the array and the index from the stack */
extern HB_EXPORT BOOL   hb_xvmArrayPushRef( void );
extern HB_EXPORT BOOL   hb_xvmArrayPop( void );                   /* pops a value from the stack */

extern HB_EXPORT void   hb_xvmHashGen( ULONG ulPairs );       /* generates an ulElements Array and fills it from the stack values */

extern HB_EXPORT void   hb_xvmLocalName( USHORT uiLocal, char * szLocalName );
extern HB_EXPORT void   hb_xvmStaticName( BYTE bIsGlobal, USHORT uiStatic, char * szStaticName );
extern HB_EXPORT void   hb_xvmModuleName( char * szModuleName );

extern HB_EXPORT void   hb_xvmMacroList( void );
extern HB_EXPORT void   hb_xvmMacroListEnd( void );
extern HB_EXPORT BOOL   hb_xvmMacroPush( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPushIndex( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPushArg( PHB_SYMB pSymbol, BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPushList( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPushAliased( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPushPare( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPop( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroPopAliased( BYTE bFlags );
extern HB_EXPORT BOOL   hb_xvmMacroSymbol( void );
extern HB_EXPORT BOOL   hb_xvmMacroText( void );
extern HB_EXPORT BOOL   hb_xvmPushMacroRef( void );

extern HB_EXPORT void   hb_xvmPushDouble( double dNumber, int iWidth, int iDec );
#ifdef HB_LONG_LONG_OFF
extern HB_EXPORT void   hb_xvmPushLongLong( double dNumber );
#else
extern HB_EXPORT void   hb_xvmPushLongLong( LONGLONG llNumber );
#endif

#define hb_xvmPushLogical( f )            hb_vmPushLogical( f )
#define hb_xvmPushInteger( i )            hb_vmPushInteger( i )
#define hb_xvmPushLong( l )               hb_vmPushLong( l )
#define hb_xvmPushNil()                   hb_vmPushNil()
#define hb_xvmPushStringConst( psz, ul )  hb_itemPushStaticString( psz, ul )
#define hb_xvmPushSymbol( p )             hb_vmPushSymbol( p )
#define hb_xvmPushDate( l )               hb_vmPushDate( l )
/* #define hb_xvmPushDateTime( l1, l2 )      hb_vmPushDateTime( l1, (double)(l2) / HB_DATETIMEINSEC ) */
#define hb_xvmPushDateTime( l1, l2 )      hb_vmPushDateTime( l1, l2 )

extern HB_EXPORT void   hb_xvmLocalSetInt( int iLocal, int iVal );
extern HB_EXPORT void   hb_xvmLocalSetStr( int iLocal, const char * pVal, ULONG ulLen );

extern HB_EXPORT void   hb_xvmLocalSetStringHidden( int iLocal, BYTE bType, ULONG ulSize, const char * pVal, ULONG ulBufferSize );
extern HB_EXPORT void   hb_xvmPushStringHidden( BYTE bType, ULONG ulSize, const char * pVal, ULONG ulBufferSize );

extern HB_EXPORT BOOL   hb_xvmBitAnd( void );
extern HB_EXPORT BOOL   hb_xvmBitOr( void );
extern HB_EXPORT BOOL   hb_xvmBitXor( void );
extern HB_EXPORT BOOL   hb_xvmBitShiftL( void );
extern HB_EXPORT BOOL   hb_xvmBitShiftR( void );

extern HB_EXPORT BOOL   hb_xvmLike( void );
extern HB_EXPORT BOOL   hb_xvmMatch( void );
extern HB_EXPORT BOOL   hb_xvmLeft( USHORT usLeft );
extern HB_EXPORT BOOL   hb_xvmRight( USHORT usRight );
extern HB_EXPORT BOOL   hb_xvmSubstr( USHORT usAt, USHORT usChars );

extern HB_EXPORT BOOL   hb_xvmSwitchCase( LONG lCase );

extern HB_EXPORT BOOL   hb_xvmIVarRef( void );
extern HB_EXPORT BOOL   hb_xvmClassSetModule( void );

extern HB_EXPORT BOOL   hb_xvmWithObject( void );
extern HB_EXPORT BOOL   hb_xvmEndWithObject( void );
extern HB_EXPORT void   hb_xvmPushWith( void );
extern HB_EXPORT BOOL   hb_xvmSendWith( USHORT uiParams );

extern HB_EXPORT BOOL   hb_xvmArrayPopPlus( void );

extern HB_EXPORT BOOL   hb_xvmForEach( void );
extern HB_EXPORT BOOL   hb_xvmEnumerate( void );
extern HB_EXPORT BOOL   hb_xvmEndEnumerate( void );
extern HB_EXPORT void   hb_xvmEnumIndex( void );

extern HB_EXPORT void   hb_xvmTryBegin( void );
extern HB_EXPORT BOOL   hb_xvmTryEnd( void );
extern HB_EXPORT void   hb_xvmTryEndFin( void );
extern HB_EXPORT BOOL   hb_xvmTryRecover( void );
extern HB_EXPORT USHORT hb_xvmBeginFinally( void );
extern HB_EXPORT BOOL   hb_xvmEndFinally( USHORT uiAction );

/*
 * additional multi PCODE operations
 */
extern HB_EXPORT BOOL   hb_xvmArrayItemPush( ULONG ulIndex );
extern HB_EXPORT BOOL   hb_xvmArrayItemPop( ULONG ulIndex );
extern HB_EXPORT BOOL   hb_xvmMultByInt( LONG lValue );
extern HB_EXPORT BOOL   hb_xvmDivideByInt( LONG lValue );
extern HB_EXPORT BOOL   hb_xvmAddInt( LONG lValue );

extern HB_EXPORT void   hb_xvmDivert( BOOL bDivertOf );

HB_EXTERN_END

#endif /* HB_XVM_H_ */
