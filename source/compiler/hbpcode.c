/*
 * $Id: hbpcode.c 9722 2012-10-02 06:13:24Z andijahja $
 */

/*
 * Harbour Project source code:
 * Compiler PCode generation functions
 *
 * Copyright 1999 {list of individual authors and e-mail addresses}
 * www - http://www.harbour-project.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version, with one exception:
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA (or visit
 * their web site at http://www.gnu.org/).
 *
 */

#include <assert.h>
#include "hbcomp.h"
#include "hbapierr.h"

#define HB_PSIZE_FUNC( func ) HB_PCODE_FUNC( func, HB_VOID_PTR )

/*
 * functions for variable size PCODE tracing
 */
static HB_PSIZE_FUNC( hb_p_pushstrshort )
{
   HB_SYMBOL_UNUSED( cargo );
   return 2 + pFunc->pCode[ lPCodePos + 1 ];
}

static HB_PSIZE_FUNC( hb_p_pushstr )
{
   HB_SYMBOL_UNUSED( cargo );
   return 3 + HB_PCODE_MKUSHORT( &pFunc->pCode[ lPCodePos + 1 ] );
}

static HB_PSIZE_FUNC( hb_p_localnearsetstr )
{
   HB_SYMBOL_UNUSED( cargo );
   return 4 + HB_PCODE_MKUSHORT( &pFunc->pCode[ lPCodePos + 2 ] );
}

static HB_PSIZE_FUNC( hb_p_pushstrhidden )
{
   HB_SYMBOL_UNUSED( cargo );
   return 6 + HB_PCODE_MKUSHORT( &pFunc->pCode[ lPCodePos + 4 ] );
}

static HB_PSIZE_FUNC( hb_p_localnearsetstrhidden )
{
   HB_SYMBOL_UNUSED( cargo );
   return 7 + HB_PCODE_MKUSHORT( &pFunc->pCode[ lPCodePos + 5 ] );
}

static HB_PSIZE_FUNC( hb_p_pushblock )
{
   HB_SYMBOL_UNUSED( cargo );
   return HB_PCODE_MKUSHORT( &pFunc->pCode[ lPCodePos + 1 ] );
}

static HB_PSIZE_FUNC( hb_p_pushblockshort )
{
   HB_SYMBOL_UNUSED( cargo );
   return pFunc->pCode[ lPCodePos + 1 ];
}

static HB_PSIZE_FUNC( hb_p_localname )
{
   HB_SIZE ulStart = lPCodePos;

   HB_SYMBOL_UNUSED( cargo );
   lPCodePos += 3;
   while( pFunc->pCode[ lPCodePos++ ] )
   {
   }
   ;

   return lPCodePos - ulStart;
}

static HB_PSIZE_FUNC( hb_p_modulename )
{
   HB_SIZE ulStart = lPCodePos;

   HB_SYMBOL_UNUSED( cargo );
   lPCodePos += 3;
   while( pFunc->pCode[ lPCodePos++ ] )
   {
   }
   ;

   return lPCodePos - ulStart;
}

static HB_PSIZE_FUNC( hb_p_staticname )
{
   HB_SIZE ulStart = lPCodePos;

   HB_SYMBOL_UNUSED( cargo );
   lPCodePos += 4;
   while( pFunc->pCode[ lPCodePos++ ] )
   {
   }
   ;

   return lPCodePos - ulStart;
}

const BYTE hb_comp_pcode_len[] = {
   1,                                                          /* HB_P_AND,                  */
   1,                                                          /* HB_P_ARRAYPUSH,            */
   1,                                                          /* HB_P_ARRAYPOP,             */
   3,                                                          /* HB_P_ARRAYDIM,             */
   3,                                                          /* HB_P_ARRAYGEN,             */
   1,                                                          /* HB_P_EQUAL,                */
   1,                                                          /* HB_P_ENDBLOCK,             */
   1,                                                          /* HB_P_ENDPROC,              */
   1,                                                          /* HB_P_EXACTLYEQUAL,         */
   1,                                                          /* HB_P_FALSE,                */
   1,                                                          /* HB_P_FORTEST,              */
   3,                                                          /* HB_P_FUNCTION,             */
   2,                                                          /* HB_P_FUNCTIONSHORT,        */
   3,                                                          /* HB_P_FRAME,                */
   1,                                                          /* HB_P_FUNCPTR,              */
   1,                                                          /* HB_P_GREATER,              */
   1,                                                          /* HB_P_GREATEREQUAL,         */
   1,                                                          /* HB_P_DEC,                  */
   1,                                                          /* HB_P_DIVIDE,               */
   3,                                                          /* HB_P_DO,                   */
   2,                                                          /* HB_P_DOSHORT,              */
   1,                                                          /* HB_P_DUPLICATE,            */
   1,                                                          /* HB_P_DUPLTWO,              */
   1,                                                          /* HB_P_INC,                  */
   1,                                                          /* HB_P_INSTRING,             */
   2,                                                          /* HB_P_JUMPNEAR,             */
   3,                                                          /* HB_P_JUMP,                 */
   4,                                                          /* HB_P_JUMPFAR,              */
   2,                                                          /* HB_P_JUMPFALSENEAR,        */
   3,                                                          /* HB_P_JUMPFALSE,            */
   4,                                                          /* HB_P_JUMPFALSEFAR,         */
   2,                                                          /* HB_P_JUMPTRUENEAR,         */
   3,                                                          /* HB_P_JUMPTRUE,             */
   4,                                                          /* HB_P_JUMPTRUEFAR,          */
   1,                                                          /* HB_P_LESSEQUAL,            */
   1,                                                          /* HB_P_LESS,                 */
   3,                                                          /* HB_P_LINE,                 */
   0,                                                          /* HB_P_LOCALNAME,            */
   2,                                                          /* HB_P_MACROPOP,             */
   2,                                                          /* HB_P_MACROPOPALIASED,      */
   2,                                                          /* HB_P_MACROPUSH,            */
   2,                                                          /* HB_P_MACROPUSHARG,         */
   2,                                                          /* HB_P_MACROPUSHLIST,        */
   2,                                                          /* HB_P_MACROPUSHINDEX,       */
   2,                                                          /* HB_P_MACROPUSHPARE,        */
   2,                                                          /* HB_P_MACROPUSHALIASED,     */
   1,                                                          /* HB_P_MACROSYMBOL,          */
   1,                                                          /* HB_P_MACROTEXT,            */
   3,                                                          /* HB_P_MESSAGE,              */
   1,                                                          /* HB_P_MINUS,                */
   1,                                                          /* HB_P_MODULUS,              */
   0,                                                          /* HB_P_MODULENAME,           */
   /* start: pcodes generated by macro compiler */
   3,                                                          /* HB_P_MMESSAGE,             */
   3,                                                          /* HB_P_MPOPALIASEDFIELD,     */
   3,                                                          /* HB_P_MPOPALIASEDVAR,       */
   3,                                                          /* HB_P_MPOPFIELD,            */
   3,                                                          /* HB_P_MPOPMEMVAR,           */
   3,                                                          /* HB_P_MPUSHALIASEDFIELD,    */
   3,                                                          /* HB_P_MPUSHALIASEDVAR,      */
   0,                                                          /* HB_P_MPUSHBLOCK,           */
   3,                                                          /* HB_P_MPUSHFIELD,           */
   3,                                                          /* HB_P_MPUSHMEMVAR,          */
   3,                                                          /* HB_P_MPUSHMEMVARREF,       */
   3,                                                          /* HB_P_MPUSHSYM,             */
   3,                                                          /* HB_P_MPUSHVARIABLE,        */
   /* end: */
   1,                                                          /* HB_P_MULT,                 */
   1,                                                          /* HB_P_NEGATE,               */
   1,                                                          /* HB_P_NOOP,                 */
   1,                                                          /* HB_P_NOT,                  */
   1,                                                          /* HB_P_NOTEQUAL,             */
   1,                                                          /* HB_P_OR,                   */
   4,                                                          /* HB_P_PARAMETER,            */
   1,                                                          /* HB_P_PLUS,                 */
   1,                                                          /* HB_P_POP,                  */
   1,                                                          /* HB_P_POPALIAS,             */
   3,                                                          /* HB_P_POPALIASEDFIELD,      */
   2,                                                          /* HB_P_POPALIASEDFIELDNEAR,  */
   3,                                                          /* HB_P_POPALIASEDVAR,        */
   3,                                                          /* HB_P_POPFIELD,             */
   3,                                                          /* HB_P_POPLOCAL,             */
   2,                                                          /* HB_P_POPLOCALNEAR,         */
   3,                                                          /* HB_P_POPMEMVAR,            */
   3,                                                          /* HB_P_POPSTATIC,            */
   3,                                                          /* HB_P_POPVARIABLE,          */
   1,                                                          /* HB_P_POWER,                */
   1,                                                          /* HB_P_PUSHALIAS,            */
   3,                                                          /* HB_P_PUSHALIASEDFIELD,     */
   2,                                                          /* HB_P_PUSHALIASEDFIELDNEAR, */
   3,                                                          /* HB_P_PUSHALIASEDVAR,       */
   0,                                                          /* HB_P_PUSHBLOCK,            */
   0,                                                          /* HB_P_PUSHBLOCKSHORT,       */
   3,                                                          /* HB_P_PUSHFIELD,            */
   2,                                                          /* HB_P_PUSHBYTE,             */
   3,                                                          /* HB_P_PUSHINT,              */
   3,                                                          /* HB_P_PUSHLOCAL,            */
   2,                                                          /* HB_P_PUSHLOCALNEAR,        */
   3,                                                          /* HB_P_PUSHLOCALREF,         */
   5,                                                          /* HB_P_PUSHLONG,             */
   3,                                                          /* HB_P_PUSHMEMVAR,           */
   3,                                                          /* HB_P_PUSHMEMVARREF,        */
   1,                                                          /* HB_P_PUSHNIL,              */
   1 + sizeof( double ) + sizeof( BYTE ) + sizeof( BYTE ),     /* HB_P_PUSHDOUBLE,           */
   1,                                                          /* HB_P_PUSHSELF,             */
   3,                                                          /* HB_P_PUSHSTATIC,           */
   3,                                                          /* HB_P_PUSHSTATICREF,        */
   0,                                                          /* HB_P_PUSHSTR,              */
   0,                                                          /* HB_P_PUSHSTRSHORT,         */
   3,                                                          /* HB_P_PUSHSYM,              */
   2,                                                          /* HB_P_PUSHSYMNEAR,          */
   3,                                                          /* HB_P_PUSHVARIABLE,         */
   1,                                                          /* HB_P_RETVALUE,             */
   3,                                                          /* HB_P_SEND,                 */
   2,                                                          /* HB_P_SENDSHORT,            */
   4,                                                          /* HB_P_SEQBEGIN,             */
   4,                                                          /* HB_P_SEQEND,               */
   1,                                                          /* HB_P_SEQRECOVER,           */
   3,                                                          /* HB_P_SFRAME,               */
   5,                                                          /* HB_P_STATICS,              */
   0,                                                          /* HB_P_STATICNAME,           */
   1,                                                          /* HB_P_SWAPALIAS,            */
   1,                                                          /* HB_P_TRUE,                 */
   1,                                                          /* HB_P_ZERO,                 */
   1,                                                          /* HB_P_ONE,                  */
   1,                                                          /* HB_P_MACROLIST,            */
   1,                                                          /* HB_P_MACROLISTEND,         */
   4,                                                          /* HB_P_LOCALNEARADDINT,      */
   4,                                                          /* HB_P_LOCALNEARSETINT,      */
   0,                                                          /* HB_P_LOCALNEARSETSTR,      */
   3,                                                          /* HB_P_ADDINT,               */
   3,                                                          /* HB_P_LEFT,                 */
   3,                                                          /* HB_P_RIGHT,                */
   5,                                                          /* HB_P_SUBSTR,               */
   0,                                                          /* HB_P_MPUSHSTR,             */
   3,                                                          /* HB_P_BASELINE,             */
   2,                                                          /* HB_P_LINEOFFSET,           */
   1,                                                          /* HB_P_WITHOBJECT            */
   3,                                                          /* HB_P_SENDWITH,             */
   2,                                                          /* HB_P_SENDWITHSHORT,        */
   1,                                                          /* HB_P_ENDWITHOBJECT,        */
   1,                                                          /* HB_P_FOREACH,              */
   1,                                                          /* HB_P_ENUMERATE,            */
   1,                                                          /* HB_P_ENDENUMERATE,         */
   2,                                                          /* HB_P_PUSHGLOBAL,           */
   2,                                                          /* HB_P_POPGLOBAL,            */
   2,                                                          /* HB_P_PUSHGLOBALREF,        */
   1,                                                          /* HB_P_ENUMINDEX,            */
   5,                                                          /* HB_P_SWITCHCASE,           */
   1,                                                          /* HB_P_LIKE,                 */
   1,                                                          /* HB_P_MATCH,                */
   1,                                                          /* HB_P_PUSHMACROREF,         */
   1,                                                          /* HB_P_IVARREF,              */
   1,                                                          /* HB_P_CLASSSETMODULE,       */
   1,                                                          /* HB_P_BITAND,               */
   1,                                                          /* HB_P_BITOR,                */
   1,                                                          /* HB_P_BITXOR,               */
   1,                                                          /* HB_P_BITSHIFTR,            */
   1,                                                          /* HB_P_BITSHIFTL,            */
   4,                                                          /* HB_P_LARGEFRAME,           */
   1,                                                          /* HB_P_PUSHWITH,             */
   9,                                                          /* HB_P_PUSHLONGLONG          */
   0,                                                          /* HB_P_PUSHSTRHIDDEN,        */
   0,                                                          /* HB_P_LOCALNEARSETSTRHIDDEN */
   4,                                                          /* HB_P_TRYBEGIN,             */
   4,                                                          /* HB_P_TRYEND,               */
   4,                                                          /* HB_P_TRYRECOVER,           */
   1,                                                          /* HB_P_FINALLY,              */
   1,                                                          /* HB_P_ENDFINALLY,           */
   2,                                                          /* HB_P_LOCALNEARADD ,        */
   1,                                                          /* HB_P_ARRAYPUSHREF,         */
   1,                                                          /* HB_P_ARRAYPOPPLUS          */
   9,                                                          /* HB_P_PUSHDATETIME,         */
   5,                                                          /* HB_P_PUSHDATE,             */
   3,                                                          /* HB_P_HASHGEN               */
   2,                                                          /* HB_P_LOCALNEARINC,         */
   2,                                                          /* HB_P_LOCALNEARDEC,         */
   2,                                                          /* HB_P_PUSHLOCALNEARINC,     */
   2,                                                          /* HB_P_PUSHLOCALNEARDEC,     */
   1,                                                          /* HB_P_DIVERT                */
   1                                                           /* HB_P_DIVERTOF              */
};

/*
 * this table has pointers to functions which count
 * real size of variable size PCODEs
 */
static HB_PCODE_FUNC_PTR s_psize_table[] =
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
   NULL,                                              /* HB_P_LINE,                 */
   hb_p_localname,                                    /* HB_P_LOCALNAME,            */
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
   hb_p_modulename,                                   /* HB_P_MODULENAME,           */
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
   hb_p_pushblock,                                    /* HB_P_PUSHBLOCK,            */
   hb_p_pushblockshort,                               /* HB_P_PUSHBLOCKSHORT,       */
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
   hb_p_pushstr,                                      /* HB_P_PUSHSTR,              */
   hb_p_pushstrshort,                                 /* HB_P_PUSHSTRSHORT,         */
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
   hb_p_staticname,                                   /* HB_P_STATICNAME,           */
   NULL,                                              /* HB_P_SWAPALIAS,            */
   NULL,                                              /* HB_P_TRUE,                 */
   NULL,                                              /* HB_P_ZERO,                 */
   NULL,                                              /* HB_P_ONE,                  */
   NULL,                                              /* HB_P_MACROLIST,            */
   NULL,                                              /* HB_P_MACROLISTEND,         */
   NULL,                                              /* HB_P_LOCALNEARADDINT,      */
   NULL,                                              /* HB_P_LOCALNEARSETINT,      */
   hb_p_localnearsetstr,                              /* HB_P_LOCALNEARSETSTR,      */
   NULL,                                              /* HB_P_ADDINT,               */
   NULL,                                              /* HB_P_LEFT,                 */
   NULL,                                              /* HB_P_RIGHT,                */
   NULL,                                              /* HB_P_SUBSTR,               */
   NULL,                                              /* HB_P_MPUSHSTR,             */
   NULL,                                              /* HB_P_BASELINE,             */
   NULL,                                              /* HB_P_LINEOFFSET,           */
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
   NULL,                                              /* HB_IVARREF,                */
   NULL,                                              /* HB_CLASSSETMODULE,         */
   NULL,                                              /* HB_P_BITAND,               */
   NULL,                                              /* HB_P_BITOR,                */
   NULL,                                              /* HB_P_BITXOR,               */
   NULL,                                              /* HB_P_SHIFTR,               */
   NULL,                                              /* HB_P_SHIFTL,               */
   NULL,                                              /* HB_P_LARGEFRAME,           */
   NULL,                                              /* HB_P_PUSHWITH,             */
   NULL,                                              /* HB_P_PUSHLONGLONG,         */
   hb_p_pushstrhidden,                                /* HB_P_PUSHSTRHIDDEN,        */
   hb_p_localnearsetstrhidden,                        /* HB_P_LOCALNEARSETSTRHIDDEN,*/
   NULL,                                              /* HB_P_TRYBEGIN,             */
   NULL,                                              /* HB_P_TRYEND,               */
   NULL,                                              /* HB_P_TRYRECOVER,           */
   NULL,                                              /* HB_P_FINALLY,              */
   NULL,                                              /* HB_P_ENDFINALLY,           */
   NULL,                                              /* HB_P_LOCALNEARADD          */
   NULL,                                              /* HB_P_ARRAYPUSHREF          */
   NULL,                                              /* HB_P_ARRAYPOPPLUS          */
   NULL,                                              /* HB_P_PUSHDATETIME,         */
   NULL,                                              /* HB_P_PUSHDATE,             */
   NULL,                                              /* HB_P_HASHGEN               */
   NULL,                                              /* HB_P_LOCALNEARINC,         */
   NULL,                                              /* HB_P_LOCALNEARDEC,         */
   NULL,                                              /* HB_P_PUSHLOCALNEARINC,     */
   NULL,                                              /* HB_P_PUSHLOCALNEARDEC,     */
   NULL,                                              /* HB_P_DIVERT                */
   NULL                                               /* HB_P_DIVERTOF              */
};

HB_SIZE hb_compPCodeSize( PFUNCTION pFunc, HB_SIZE ulOffset )
{
   HB_SIZE  lSize    = 0;
   BYTE     opcode   = pFunc->pCode[ ulOffset ];

   if( opcode < HB_P_LAST_PCODE )
   {
      lSize = hb_comp_pcode_len[ opcode ];

      if( lSize == 0 )
      {
         HB_PCODE_FUNC_PTR pCall = s_psize_table[ opcode ];

         if( pCall != NULL )
            lSize = pCall( pFunc, ulOffset, NULL );
      }
   }
   return lSize;
}

void hb_compPCodeEval( PFUNCTION pFunc, const HB_PCODE_FUNC_PTR * pFunctions, void * cargo )
{
   HB_SIZE  ulPos = 0;
   HB_SIZE  ulSkip;
   BYTE     opcode;

   /* Make sure that tables are correct */
   assert( sizeof( hb_comp_pcode_len ) == HB_P_LAST_PCODE );
   assert( sizeof( s_psize_table ) / sizeof( HB_PCODE_FUNC_PTR ) == HB_P_LAST_PCODE );

   while( ulPos < pFunc->lPCodePos )
   {
      opcode = pFunc->pCode[ ulPos ];
      if( opcode < HB_P_LAST_PCODE )
      {
         HB_PCODE_FUNC_PTR pCall = pFunctions[ opcode ];
         ulSkip = pCall ? pCall( pFunc, ulPos, cargo ) : 0;
         if( ulSkip == 0 )
         {
            ulSkip = hb_comp_pcode_len[ opcode ];
            if( ulSkip == 0 )
            {
               pCall = s_psize_table[ opcode ];
               if( pCall != NULL )
                  ulSkip = pCall( pFunc, ulPos, NULL );
            }
         }

         if( ulSkip == 0 )
         {
            char szOpcode[ 16 ];
            hb_snprintf( szOpcode, sizeof( szOpcode ), "%i", opcode );
            hb_errInternal( HB_EI_COMPBADOPSIZE, "Invalid (zero) opcode %s size in hb_compPCodeEval()", szOpcode, NULL );
            ++ulPos;
         }
#if 0
         /*
          * Test code to validate return values by PCODE eval functions,
          * in some cases the eval functions can return intentionally differ
          * values so it's not enabled by default. [druzus]
          */
         if( hb_comp_pcode_len[ opcode ] != 0 && hb_comp_pcode_len[ opcode ] != ulSkip )
         {
            char szMsg[ 100 ];
            hb_snprintf( szMsg, sizeof( szMsg ), "Wrong PCODE (%d) size (%ld!=%d)", opcode, ulSkip, hb_comp_pcode_len[ opcode ] );
            hb_errInternal( HB_EI_COMPBADOPSIZE, szMsg, NULL, NULL );
         }
#endif
         ulPos += ulSkip;
      }
      else
      {
         char szOpcode[ 16 ];
         hb_snprintf( szOpcode, sizeof( szOpcode ), "%i", opcode );
         hb_errInternal( HB_EI_COMPBADOPCODE, "Invalid opcode: %s in hb_compPCodeEval()", szOpcode, NULL );
         ++ulPos;
      }
   }
}

void hb_compPCodeTrace( PFUNCTION pFunc, const HB_PCODE_FUNC_PTR * pFunctions, void * cargo )
{
   HB_SIZE ulPos = 0;

   /* Make sure that table is correct */
   assert( sizeof( hb_comp_pcode_len ) == HB_P_LAST_PCODE );

   while( ulPos < pFunc->lPCodePos )
   {
      BYTE opcode = pFunc->pCode[ ulPos ];
      if( opcode < HB_P_LAST_PCODE )
      {
         HB_PCODE_FUNC_PTR pCall = pFunctions[ opcode ];
         if( pCall )
            ulPos = pCall( pFunc, ulPos, cargo );
         else
            ulPos += hb_comp_pcode_len[ opcode ];
      }
      else
      {
         char szOpcode[ 16 ];
         hb_snprintf( szOpcode, sizeof( szOpcode ), "%i", opcode );
         hb_errInternal( HB_EI_COMPBADOPCODE, "Invalid opcode: %s in hb_compPCodeTrace()", szOpcode, NULL );
         ++ulPos;
      }
   }
}

void hb_compGenPCode1( BYTE byte )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->pCode      = ( BYTE * ) hb_xgrab( HB_PCODE_CHUNK );
      pFunc->lPCodeSize = HB_PCODE_CHUNK;
      pFunc->lPCodePos  = 0;
   }
   else if( ( pFunc->lPCodeSize - pFunc->lPCodePos ) < 1 )
   {
      pFunc->pCode = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize += HB_PCODE_CHUNK );
   }

   pFunc->pCode[ pFunc->lPCodePos++ ] = byte;

#if defined( HB_COMP_STRONG_TYPES )
   if( hb_comp_iWarnings >= 3 )
   {
      hb_compStrongType( 1 );
   }
#endif
}

void hb_compGenPData1( BYTE byte )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->pCode      = ( BYTE * ) hb_xgrab( HB_PCODE_CHUNK );
      pFunc->lPCodeSize = HB_PCODE_CHUNK;
      pFunc->lPCodePos  = 0;
   }
   else if( ( pFunc->lPCodeSize - pFunc->lPCodePos ) < 1 )
   {
      pFunc->pCode = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize += HB_PCODE_CHUNK );
   }

   pFunc->pCode[ pFunc->lPCodePos++ ] = byte;
}

void hb_compGenPCode2( BYTE byte1, BYTE byte2, BOOL bStackAffected )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->pCode      = ( BYTE * ) hb_xgrab( HB_PCODE_CHUNK );
      pFunc->lPCodeSize = HB_PCODE_CHUNK;
      pFunc->lPCodePos  = 0;
   }
   else if( ( pFunc->lPCodeSize - pFunc->lPCodePos ) < 2 )
   {
      pFunc->pCode = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize += HB_PCODE_CHUNK );
   }

   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte1;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte2;

#if defined( HB_COMP_STRONG_TYPES )
   if( hb_comp_iWarnings >= 3 && bStackAffected )
   {
      hb_compStrongType( 2 );
   }
#else
   HB_SYMBOL_UNUSED( bStackAffected );
#endif
}

void hb_compGenPCode3( BYTE byte1, BYTE byte2, BYTE byte3, BOOL bStackAffected )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->pCode      = ( BYTE * ) hb_xgrab( HB_PCODE_CHUNK );
      pFunc->lPCodeSize = HB_PCODE_CHUNK;
      pFunc->lPCodePos  = 0;
   }
   else if( ( pFunc->lPCodeSize - pFunc->lPCodePos ) < 3 )
   {
      pFunc->pCode = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize += HB_PCODE_CHUNK );
   }

   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte1;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte2;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte3;

#if defined( HB_COMP_STRONG_TYPES )
   if( hb_comp_iWarnings >= 3 && bStackAffected )
   {
      hb_compStrongType( 3 );
   }
#else
   HB_SYMBOL_UNUSED( bStackAffected );
#endif
}

void hb_compGenPCode4( BYTE byte1, BYTE byte2, BYTE byte3, BYTE byte4, BOOL bStackAffected )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->pCode      = ( BYTE * ) hb_xgrab( HB_PCODE_CHUNK );
      pFunc->lPCodeSize = HB_PCODE_CHUNK;
      pFunc->lPCodePos  = 0;
   }
   else if( ( pFunc->lPCodeSize - pFunc->lPCodePos ) < 4 )
   {
      pFunc->pCode = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize += HB_PCODE_CHUNK );
   }

   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte1;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte2;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte3;
   pFunc->pCode[ pFunc->lPCodePos++ ]  = byte4;

#if defined( HB_COMP_STRONG_TYPES )
   if( hb_comp_iWarnings >= 3 && bStackAffected )
   {
      hb_compStrongType( 4 );
   }
#else
   HB_SYMBOL_UNUSED( bStackAffected );
#endif
}

void hb_compGenPCodeN( BYTE * pBuffer, HB_SIZE ulSize, BOOL bStackAffected )
{
   PFUNCTION pFunc = hb_comp_functions.pLast;   /* get the currently defined Clipper function */

   if( ! pFunc->pCode )                         /* has been created the memory block to hold the pcode ? */
   {
      pFunc->lPCodeSize = ( ( ulSize / HB_PCODE_CHUNK ) + 1 ) * HB_PCODE_CHUNK;
      pFunc->pCode      = ( BYTE * ) hb_xgrab( pFunc->lPCodeSize );
      pFunc->lPCodePos  = 0;
   }
   else if( pFunc->lPCodePos + ulSize > pFunc->lPCodeSize )
   {
      /* not enough free space in pcode buffer - increase it */
      pFunc->lPCodeSize += ( ( ( ulSize / HB_PCODE_CHUNK ) + 1 ) * HB_PCODE_CHUNK );
      pFunc->pCode      = ( BYTE * ) hb_xrealloc( pFunc->pCode, pFunc->lPCodeSize );
   }

   HB_MEMCPY( pFunc->pCode + pFunc->lPCodePos, pBuffer, ( size_t ) ulSize );
   pFunc->lPCodePos += ulSize;

#if defined( HB_COMP_STRONG_TYPES )
   if( hb_comp_iWarnings >= 3 && bStackAffected )
   {
      hb_compStrongType( ulSize );
   }
#else
   HB_SYMBOL_UNUSED( bStackAffected );
#endif
}
