/*
 * $Id: hbtypes.h 9830 2012-11-20 03:13:35Z andijahja $
 */

/*
 * Harbour Project source code:
 * Header file for typedef for maindllp.c
 *
 * Copyright 2001-2002 Luiz Rafael Culik <culik@sl.conex.net>
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

#ifndef HB_TYPES_H_
#define HB_TYPES_H_

#ifdef HB_API_MACROS
   #undef HB_API_MACROS
#endif

#include "hbvm.h"
#include "hbapiitm.h"

//typedef PHB_SYMB ( * VM_PROCESS_DLL_SYMBOLS ) ( PHB_SYMB pModuleSymbols, ... );
typedef PSYMBOLS ( * VM_PROCESS_DLL_SYMBOLS ) ( PHB_SYMB pSymbols, USHORT uiModuleSymbols, const char *szModule, int iPCodeVer, PHB_ITEM *pGlobals );
typedef void ( * VM_DLL_EXECUTE ) ( const BYTE * pCode, PHB_SYMB pSymbols );

typedef BOOL     ( * EXT_IS_ARRAY ) ( int iParam );
typedef char *   ( * EXT_PARC1 )  ( int iParam );
typedef char *   ( * EXT_PARC2 )  ( int iParam, ULONG ulArrayIndex );
typedef PHB_ITEM ( * HB_PARAM)( int iParam, long lMask );
typedef PHB_ITEM ( * HB_PARAMERROR)( int iParam );
typedef int      ( * HB_PCOUNTS )( void );
typedef void     ( * HB_RET)( void );
typedef void     ( * HB_RETC)( const char * szText );
typedef void     ( * HB_RETCLEN)( const char * szText, HB_SIZE ulLen );
typedef void     ( * HB_RETDS)( const char * szDate );
typedef void     ( * HB_RETD)( int iYear, int iMonth, int iDay );
typedef void     ( * HB_RETDL)( long lJulian );
typedef void     ( * HB_RETL)( int iTrueFalse );
typedef void     ( * HB_RETND)( double dNumber );
typedef void     ( * HB_RETNI)( int iNumber );
typedef void     ( * HB_RETNL)( long lNumber );
typedef void     ( * HB_RETNS)( HB_ISIZ lNumber );
typedef void     ( * HB_RETNLEN)( double dNumber, int iWidth, int iDec );
typedef void     ( * HB_RETNDLEN)( double dNumber, int iWidth, int iDec );
typedef void     ( * HB_RETNILEN)( int iNumber, int iWidth );
typedef void     ( * HB_RETNLLEN)( long lNumber, int iWidth );
typedef void     ( * HB_RETA)( HB_SIZE ulLen );
typedef HB_SIZE  ( * HB_PARINFA)( int iParamNum, HB_SIZE uiArrayIndex );
typedef HB_SIZE  ( * HB_PARINFO)( int iParam );
typedef HB_SIZE  ( * HB_PARCLEN)( int iParam );
typedef HB_SIZE  ( * HB_PARCSIZ)( int iParam );
typedef char *   ( * HB_PARDS)( int iParam );
typedef char *   ( * HB_PARDSBUFF)( char * szDate,int iParam);
typedef int      ( * HB_PARL)( int iParam );
typedef double   ( * HB_PARND)( int iParam );
typedef int      ( * HB_PARNI)( int iParam );
typedef long     ( * HB_PARNL)( int iParam );
typedef HB_ISIZ  ( * HB_PARNS)( int iParam );
typedef HB_SIZE  ( * HB_PARCLEN2)( int iParam, ULONG ulArrayIndex );
typedef HB_SIZE  ( * HB_PARCSIZ2)( int iParam, ULONG ulArrayIndex );
typedef char *   ( * HB_PARDS2)( int iParam, ULONG ulArrayIndex );
typedef char *   ( * HB_PARDSBUFF2)( char * szDate,int iParam, ULONG ulArrayIndex );
typedef int      ( * HB_PARL2)( int iParam, ULONG ulArrayIndex );
typedef double   ( * HB_PARND2)( int iParam, ULONG ulArrayIndex );
typedef int      ( * HB_PARNI2)( int iParam, ULONG ulArrayIndex );
typedef long     ( * HB_PARNL2)( int iParam, ULONG ulArrayIndex );
typedef HB_SIZE  ( * HB_PARNS2)( int iParam, ULONG ulArrayIndex );
typedef int      ( * HB_STORC)( const char * szText, int iParam );
typedef int      ( * HB_STORCLEN)( const char * szText, HB_SIZE ulLength, int iParam);
typedef int      ( * HB_STORDS)( const char * szDate, int iParam) ;
typedef int      ( * HB_STORL)( int iLogical, int iParam );
typedef int      ( * HB_STORNI)( int iValue, int iParam );
typedef int      ( * HB_STORNL)( long lValue, int iParam );
typedef int      ( * HB_STORND)( double dValue, int iParam );
typedef int      ( * HB_STORNS)( HB_ISIZ dValue, int iParam );
typedef int      ( * HB_STORC2)( const char * szText, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORCLEN2)( const char * szText, HB_SIZE ulLength, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORDS2)( const char * szDate, int iParam , ULONG ulArrayIndex) ;
typedef int      ( * HB_STORL2)( int iLogical, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORNI2)( int iValue, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORNL2)( long lValue, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORND2)( double dValue, int iParam , ULONG ulArrayIndex);
typedef int      ( * HB_STORNS2)( HB_SIZE dValue, int iParam , ULONG ulArrayIndex);
typedef BOOL     ( * HB_ARRAYNEW)( PHB_ITEM pItem, HB_SIZE ulLen );
typedef HB_SIZE  ( * HB_ARRAYLEN)( PHB_ITEM pArray );
typedef BOOL     ( * HB_ARRAYISOBJECT)( PHB_ITEM pArray );
typedef BOOL     ( * HB_ARRAYADD)( PHB_ITEM pArray, PHB_ITEM pItemValue );
typedef BOOL     ( * HB_ARRAYINS)( PHB_ITEM pArray, HB_SIZE ulIndex );
typedef BOOL     ( * HB_ARRAYDEL)( PHB_ITEM pArray, HB_SIZE ulIndex );
typedef BOOL     ( * HB_ARRAYSIZE)( PHB_ITEM pArray, HB_SIZE ulLen );
typedef BOOL     ( * HB_ARRAYLAST)( PHB_ITEM pArray, PHB_ITEM pResult );
typedef BOOL     ( * HB_ARRAYRELEASE)( PHB_ITEM pArray );
typedef BOOL     ( * HB_ARRAYSET)( PHB_ITEM pArray, HB_SIZE ulIndex, PHB_ITEM pItem );
typedef BOOL     ( * HB_ARRAYGET)( PHB_ITEM pArray, HB_SIZE ulIndex, PHB_ITEM pItem );
typedef void     ( * HB_XINIT)( void );                         /* Initialize fixed memory subsystem */
typedef void     ( * HB_XEXIT)( void );                         /* Deinitialize fixed memory subsystem */
typedef void *   ( * HB_XALLOC)( HB_SIZE ulSize );                /* allocates memory, returns NULL on failure */
typedef void *   ( * HB_XGRAB)( HB_SIZE ulSize );                 /* allocates memory, exits on failure */
typedef void     ( * HB_XFREE)( void * pMem );                  /* frees memory */
typedef void *   ( * HB_XREALLOC)( void * pMem, HB_SIZE ulSize ); /* reallocates memory */
typedef HB_SIZE  ( * HB_XSIZE)( void * pMem );                  /* returns the size of an allocated memory block */
typedef void     ( * HB_FSCLOSE  ) ( HB_FHANDLE hFileHandle );
typedef BOOL     ( * HB_FSDELETE ) ( const char * pszFileName );
typedef HB_FHANDLE  ( * HB_FSOPEN   ) (  const char * pszFileName, USHORT uiFlags  );
typedef USHORT   ( * HB_FSREAD   ) ( HB_FHANDLE hFileHandle, void * pBuff, USHORT ulCount );
typedef ULONG    ( * HB_FSSEEK   ) ( HB_FHANDLE hFileHandle, LONG lOffset, USHORT uiMode );
typedef USHORT   ( * HB_FSWRITE  ) ( HB_FHANDLE hFileHandle, const void * pBuff, USHORT ulCount );
typedef HB_FHANDLE  ( * HB_FSCREATE ) (  const char * pszFileName, ULONG uiAttr  );

#endif /* HB_TYPES_H_ */
