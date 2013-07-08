/*
 * $Id: hbstripl.c 9722 2012-10-02 06:13:24Z andijahja $
 */

/*
 * Harbour Project source code:
 * Strip multiple HB_P_LINE PCODEs which can appear after dead code
 * and dummy jumps elimination
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

#include <assert.h>
#include "hbcomp.h"

typedef void HB_STRIP_INFO, * PHB_STRIP_INFO;

#define HB_STRIP_FUNC( func ) HB_PCODE_FUNC( func, PHB_STRIP_INFO )
typedef HB_STRIP_FUNC ( HB_STRIP_FUNC_ );
typedef HB_STRIP_FUNC_ * PHB_STRIP_FUNC;

static HB_STRIP_FUNC( hb_p_line )
{
   HB_SYMBOL_UNUSED( cargo );
   if( pFunc->pCode[ lPCodePos + 3 ] == HB_P_LINE ||
       pFunc->pCode[ lPCodePos + 3 ] == HB_P_BASELINE )
   {
      hb_compNOOPfill( pFunc, lPCodePos, 3, FALSE, FALSE );
   }

   return 3;
}

static HB_STRIP_FUNC( hb_p_baseline )
{
   HB_SYMBOL_UNUSED( cargo );
   if( pFunc->pCode[ lPCodePos + 3 ] == HB_P_BASELINE )
   {
      hb_compNOOPfill( pFunc, lPCodePos, 3, FALSE, FALSE );
   }

   return 3;
}

static HB_STRIP_FUNC( hb_p_lineoffset )
{
   HB_SYMBOL_UNUSED( cargo );
   if( pFunc->pCode[ lPCodePos + 2 ] == HB_P_BASELINE ||
       pFunc->pCode[ lPCodePos + 2 ] == HB_P_LINEOFFSET )
   {
      hb_compNOOPfill( pFunc, lPCodePos, 2, FALSE, FALSE );
   }

   return 2;
}

/* NOTE: The  order of functions have to match the order of opcodes
 *       mnemonics
 */
static PHB_STRIP_FUNC s_stripLines_table[] =
{
   NULL,                                              /* HB_P_AND,                  */
   NULL,                                              /* HB_P_ARRAYPUSH,            */
   NULL,                                              /* HB_P_ARRAYPOP,             */
   NULL,                                              /* HB_P_ARRAYDIM,             */
   NULL,                                              /* HB_P_ARRAYGEN,             */
   NULL,                                              /* HB_P_EQUAL,                */
   NULL,                                              /* HB_P_ENDBLOCK,             */
   NULL,                                              /* HB_P_ENDPROC,              */
   NULL,                                              /* HB_P_EXACTLYEQUAL,         */
   NULL,                                              /* HB_P_FALSE,                */
   NULL,                                              /* HB_P_FORTEST,              */
   NULL,                                              /* HB_P_FUNCTION,             */
   NULL,                                              /* HB_P_FUNCTIONSHORT,        */
   NULL,                                              /* HB_P_FRAME,                */
   NULL,                                              /* HB_P_FUNCPTR,              */
   NULL,                                              /* HB_P_GREATER,              */
   NULL,                                              /* HB_P_GREATEREQUAL,         */
   NULL,                                              /* HB_P_DEC,                  */
   NULL,                                              /* HB_P_DIVIDE,               */
   NULL,                                              /* HB_P_DO,                   */
   NULL,                                              /* HB_P_DOSHORT,              */
   NULL,                                              /* HB_P_DUPLICATE,            */
   NULL,                                              /* HB_P_DUPLTWO,              */
   NULL,                                              /* HB_P_INC,                  */
   NULL,                                              /* HB_P_INSTRING,             */
   NULL,                                              /* HB_P_JUMPNEAR,             */
   NULL,                                              /* HB_P_JUMP,                 */
   NULL,                                              /* HB_P_JUMPFAR,              */
   NULL,                                              /* HB_P_JUMPFALSENEAR,        */
   NULL,                                              /* HB_P_JUMPFALSE,            */
   NULL,                                              /* HB_P_JUMPFALSEFAR,         */
   NULL,                                              /* HB_P_JUMPTRUENEAR,         */
   NULL,                                              /* HB_P_JUMPTRUE,             */
   NULL,                                              /* HB_P_JUMPTRUEFAR,          */
   NULL,                                              /* HB_P_LESSEQUAL,            */
   NULL,                                              /* HB_P_LESS,                 */
   hb_p_line,                                         /* HB_P_LINE,                 */
   NULL,                                              /* HB_P_LOCALNAME,            */
   NULL,                                              /* HB_P_MACROPOP,             */
   NULL,                                              /* HB_P_MACROPOPALIASED,      */
   NULL,                                              /* HB_P_MACROPUSH,            */
   NULL,                                              /* HB_P_MACROPUSHARG,         */
   NULL,                                              /* HB_P_MACROPUSHLIST,        */
   NULL,                                              /* HB_P_MACROPUSHINDEX,       */
   NULL,                                              /* HB_P_MACROPUSHPARE,        */
   NULL,                                              /* HB_P_MACROPUSHALIASED,     */
   NULL,                                              /* HB_P_MACROSYMBOL,          */
   NULL,                                              /* HB_P_MACROTEXT,            */
   NULL,                                              /* HB_P_MESSAGE,              */
   NULL,                                              /* HB_P_MINUS,                */
   NULL,                                              /* HB_P_MODULUS,              */
   NULL,                                              /* HB_P_MODULENAME,           */
   /* start: pcodes generated by macro compiler */
   NULL,                                              /* HB_P_MMESSAGE,             */
   NULL,                                              /* HB_P_MPOPALIASEDFIELD,     */
   NULL,                                              /* HB_P_MPOPALIASEDVAR,       */
   NULL,                                              /* HB_P_MPOPFIELD,            */
   NULL,                                              /* HB_P_MPOPMEMVAR,           */
   NULL,                                              /* HB_P_MPUSHALIASEDFIELD,    */
   NULL,                                              /* HB_P_MPUSHALIASEDVAR,      */
   NULL,                                              /* HB_P_MPUSHBLOCK,           */
   NULL,                                              /* HB_P_MPUSHFIELD,           */
   NULL,                                              /* HB_P_MPUSHMEMVAR,          */
   NULL,                                              /* HB_P_MPUSHMEMVARREF,       */
   NULL,                                              /* HB_P_MPUSHSYM,             */
   NULL,                                              /* HB_P_MPUSHVARIABLE,        */
   /* end: */
   NULL,                                              /* HB_P_MULT,                 */
   NULL,                                              /* HB_P_NEGATE,               */
   NULL,                                              /* HB_P_NOOP,                 */
   NULL,                                              /* HB_P_NOT,                  */
   NULL,                                              /* HB_P_NOTEQUAL,             */
   NULL,                                              /* HB_P_OR,                   */
   NULL,                                              /* HB_P_PARAMETER,            */
   NULL,                                              /* HB_P_PLUS,                 */
   NULL,                                              /* HB_P_POP,                  */
   NULL,                                              /* HB_P_POPALIAS,             */
   NULL,                                              /* HB_P_POPALIASEDFIELD,      */
   NULL,                                              /* HB_P_POPALIASEDFIELDNEAR,  */
   NULL,                                              /* HB_P_POPALIASEDVAR,        */
   NULL,                                              /* HB_P_POPFIELD,             */
   NULL,                                              /* HB_P_POPLOCAL,             */
   NULL,                                              /* HB_P_POPLOCALNEAR,         */
   NULL,                                              /* HB_P_POPMEMVAR,            */
   NULL,                                              /* HB_P_POPSTATIC,            */
   NULL,                                              /* HB_P_POPVARIABLE,          */
   NULL,                                              /* HB_P_POWER,                */
   NULL,                                              /* HB_P_PUSHALIAS,            */
   NULL,                                              /* HB_P_PUSHALIASEDFIELD,     */
   NULL,                                              /* HB_P_PUSHALIASEDFIELDNEAR, */
   NULL,                                              /* HB_P_PUSHALIASEDVAR,       */
   NULL,                                              /* HB_P_PUSHBLOCK,            */
   NULL,                                              /* HB_P_PUSHBLOCKSHORT,       */
   NULL,                                              /* HB_P_PUSHFIELD,            */
   NULL,                                              /* HB_P_PUSHBYTE,             */
   NULL,                                              /* HB_P_PUSHINT,              */
   NULL,                                              /* HB_P_PUSHLOCAL,            */
   NULL,                                              /* HB_P_PUSHLOCALNEAR,        */
   NULL,                                              /* HB_P_PUSHLOCALREF,         */
   NULL,                                              /* HB_P_PUSHLONG,             */
   NULL,                                              /* HB_P_PUSHMEMVAR,           */
   NULL,                                              /* HB_P_PUSHMEMVARREF,        */
   NULL,                                              /* HB_P_PUSHNIL,              */
   NULL,                                              /* HB_P_PUSHDOUBLE,           */
   NULL,                                              /* HB_P_PUSHSELF,             */
   NULL,                                              /* HB_P_PUSHSTATIC,           */
   NULL,                                              /* HB_P_PUSHSTATICREF,        */
   NULL,                                              /* HB_P_PUSHSTR,              */
   NULL,                                              /* HB_P_PUSHSTRSHORT,         */
   NULL,                                              /* HB_P_PUSHSYM,              */
   NULL,                                              /* HB_P_PUSHSYMNEAR,          */
   NULL,                                              /* HB_P_PUSHVARIABLE,         */
   NULL,                                              /* HB_P_RETVALUE,             */
   NULL,                                              /* HB_P_SEND,                 */
   NULL,                                              /* HB_P_SENDSHORT,            */
   NULL,                                              /* HB_P_SEQBEGIN,             */
   NULL,                                              /* HB_P_SEQEND,               */
   NULL,                                              /* HB_P_SEQRECOVER,           */
   NULL,                                              /* HB_P_SFRAME,               */
   NULL,                                              /* HB_P_STATICS,              */
   NULL,                                              /* HB_P_STATICNAME,           */
   NULL,                                              /* HB_P_SWAPALIAS,            */
   NULL,                                              /* HB_P_TRUE,                 */
   NULL,                                              /* HB_P_ZERO,                 */
   NULL,                                              /* HB_P_ONE,                  */
   NULL,                                              /* HB_P_MACROLIST,            */
   NULL,                                              /* HB_P_MACROLISTEND,         */
   NULL,                                              /* HB_P_LOCALNEARADDINT,      */
   NULL,                                              /* HB_P_LOCALNEARSETINT,      */
   NULL,                                              /* HB_P_LOCALNEARSETSTR,      */
   NULL,                                              /* HB_P_ADDINT,               */
   NULL,                                              /* HB_P_LEFT,                 */
   NULL,                                              /* HB_P_RIGHT,                */
   NULL,                                              /* HB_P_SUBSTR,               */
   NULL,                                              /* HB_P_MPUSHSTR,             */
   hb_p_baseline,                                     /* HB_P_BASELINE,             */
   hb_p_lineoffset,                                   /* HB_P_LINEOFFSET,           */
   NULL,                                              /* HB_P_WITHOBJECT,           */
   NULL,                                              /* HB_P_SENDWITH,             */
   NULL,                                              /* HB_P_SENDWITHSHORT,        */
   NULL,                                              /* HB_P_ENDWITHOBJECT,        */
   NULL,                                              /* HB_P_FOREACH,              */
   NULL,                                              /* HB_P_ENUMERATE,            */
   NULL,                                              /* HB_P_ENDENUMERATE,         */
   NULL,                                              /* HB_P_PUSHGLOBAL,           */
   NULL,                                              /* HB_P_POPGLOBAL,            */
   NULL,                                              /* HB_P_PUSHGLOBALREF,        */
   NULL,                                              /* HB_P_ENUMINDEX,            */
   NULL,                                              /* HB_P_SWITCHCASE,           */
   NULL,                                              /* HB_P_LIKE,                 */
   NULL,                                              /* HB_P_MATCH,                */
   NULL,                                              /* HB_P_PUSHMACROREF,         */
   NULL,                                              /* HB_P_IVARREF,              */
   NULL,                                              /* HB_P_CLASSSETMODULE,       */
   NULL,                                              /* HB_P_BITAND,               */
   NULL,                                              /* HB_P_BITOR,                */
   NULL,                                              /* HB_P_BITXOR,               */
   NULL,                                              /* HB_P_SHIFTR,               */
   NULL,                                              /* HB_P_SHIFTL,               */
   NULL,                                              /* HB_P_LARGEFRAME,           */
   NULL,                                              /* HB_P_PUSHWITH,             */
   NULL,                                              /* HB_P_PUSHLONGLONG,         */
   NULL,                                              /* HB_P_PUSHSTRHIDDEN,        */
   NULL,                                              /* HB_P_LOCALNEARSETSTRHIDDEN,*/
   NULL,                                              /* HB_P_TRYBEGIN,             */
   NULL,                                              /* HB_P_TRYEND,               */
   NULL,                                              /* HB_P_TRYRECOVER,           */
   NULL,                                              /* HB_P_FINALLY,              */
   NULL,                                              /* HB_P_ENDFINALLY,           */
   NULL,                                              /* HB_P_LOCALNEARADD          */
   NULL,                                              /* HB_P_ARRAYPUSHREF          */
   NULL,                                              /* HB_P_ARRAYPOPPLUS          */
   NULL,                                              /* HB_P_PUSHDATETIME          */
   NULL,                                              /* HB_P_PUSHDATE              */
   NULL,                                              /* HB_P_HASHGEN               */
   NULL,                                              /* HB_P_LOCALNEARINC,         */
   NULL,                                              /* HB_P_LOCALNEARDEC,         */
   NULL,                                              /* HB_P_PUSHLOCALNEARINC,     */
   NULL,                                              /* HB_P_PUSHLOCALNEARDEC,     */
   NULL,                                              /* HB_P_DIVERT                */
   NULL                                               /* HB_P_DIVERTOF              */
};

void hb_compStripFuncLines( PFUNCTION pFunc )
{
   assert( HB_P_LAST_PCODE == sizeof( s_stripLines_table ) / sizeof( PHB_STRIP_FUNC ) );

   hb_compPCodeEval( pFunc, ( HB_PCODE_FUNC_PTR * ) s_stripLines_table, NULL );
}
