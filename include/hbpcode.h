/*
 * $Id: hbpcode.h 9279 2011-02-14 18:06:32Z druzus $
 */

/*
 * Harbour Project source code:
 * Header file for the PCODE declarations
 *
 * Copyright 1999 Antonio Linares <alinares@fivetech.com>
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

#ifndef HB_PCODE_H_
#define HB_PCODE_H_

/* NOTE:
 * Please update any opcode lookup tables present in
 *  genc.c
 *  harbour.c
 * when new opcode is added
 */
typedef enum
{
   HB_P_AND,                   /*   0 peforms the logical AND of two latest stack values, removes them and places result */
   HB_P_ARRAYPUSH,             /*   1 places on the virtual machine stack an array element */
   HB_P_ARRAYPOP,              /*   2 pops a value from the eval stack into an array element */
   HB_P_ARRAYDIM,              /*   3 instructs the virtual machine to build an array with some specific dimensions */
   HB_P_ARRAYGEN,              /*   4 instructs the virtual machine to build an array and load elemnst from the stack */
   HB_P_EQUAL,                 /*   5 check if the latest two values on the stack are equal, removing them and leaving there the result */
   HB_P_ENDBLOCK,              /*   6 end of a codeblock definition */
   HB_P_ENDPROC,               /*   7 instructs the virtual machine to end execution */
   HB_P_EXACTLYEQUAL,          /*   8 check if the latest two values on the stack are exactly equal, removing them and leaving there the result */
   HB_P_FALSE,                 /*   9 pushes false on the virtual machine stack */
   HB_P_FORTEST,               /*  10 For STEP. If step > 1 less. If step < 1 greater. */
   HB_P_FUNCTION,              /*  11 instructs the virtual machine to execute a function saving its result */
   HB_P_FUNCTIONSHORT,         /*  12 instructs the virtual machine to execute a function saving its result */
   HB_P_FRAME,                 /*  13 instructs the virtual machine about how many parameters and locals a function uses */
   HB_P_FUNCPTR,               /*  14 returns a function address pointer */
   HB_P_GREATER,               /*  15 checks if the second latest value on the stack is greater that the lastest one */
   HB_P_GREATEREQUAL,          /*  16 checks if the second latest value on the stack is greater equal that the latest one, leaves the result only */
   HB_P_DEC,                   /*  17 decrements the latest value on the virtual machine stack */
   HB_P_DIVIDE,                /*  18 divides the latest two values on the stack, removing them and leaving there the result */
   HB_P_DO,                    /*  19 instructs the virtual machine to execute a function discarding its result */
   HB_P_DOSHORT,               /*  20 instructs the virtual machine to execute a function discarding its result */
   HB_P_DUPLICATE,             /*  21 places a copy of the latest virtual machine stack value on to the stack */
   HB_P_DUPLTWO,               /*  22 places a copy of the latest two virtual machine stack value on to the stack */
   HB_P_INC,                   /*  23 increments the latest value on the virtual machine stack */
   HB_P_INSTRING,              /*  24 checks if the second latest value on the stack is a substring of the latest one */
   HB_P_JUMPNEAR,              /*  25 jumps to a relative offset 1 Byte */
   HB_P_JUMP,                  /*  26 jumps to a relative offset 2 Bytes */
   HB_P_JUMPFAR,               /*  27 jumps to a relative offset 3 Bytes */
   HB_P_JUMPFALSENEAR,         /*  28 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_JUMPFALSE,             /*  29 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_JUMPFALSEFAR,          /*  30 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_JUMPTRUENEAR,          /*  31 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_JUMPTRUE,              /*  32 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_JUMPTRUEFAR,           /*  33 checks a logic expression of the stack and jumps to a relative offset */
   HB_P_LESSEQUAL,             /*  34 checks if the second latest value on the stack is less equal that the latest one, leaves the result only */
   HB_P_LESS,                  /*  35 checks if the second latest value on the stack is less that the lastest one */
   HB_P_LINE,                  /*  36 currently compiled source code line number */
   HB_P_LOCALNAME,             /*  37 sets the name of local variable */
   HB_P_MACROPOP,              /*  38 compile and run - pop a value from the stack */
   HB_P_MACROPOPALIASED,       /*  39 compile and run - pop a field value from the stack */
   HB_P_MACROPUSH,             /*  40 compile and run - leave the result on the stack */
   HB_P_MACROPUSHARG,          /*  41 compile and run - leave the result on the stack */
   HB_P_MACROPUSHLIST,         /*  42 compile and run - leave the result on the stack */
   HB_P_MACROPUSHINDEX,        /*  43 compile and run - leave the result on the stack */
   HB_P_MACROPUSHPARE,         /*  44 compile and run - leave the result on the stack */
   HB_P_MACROPUSHALIASED,      /*  45 compile and run - leave the field value on the stack */
   HB_P_MACROSYMBOL,           /*  46 compile into a symbol name (used in function calls) */
   HB_P_MACROTEXT,             /*  47 macro text substitution */
   HB_P_MESSAGE,               /*  48 sends a message to an object */
   HB_P_MINUS,                 /*  49 subs the latest two values on the stack, removing them and leaving there the result */
   HB_P_MODULUS,               /*  50 calculates the modulus of the two values on the stack, removing them and leaving there the result */
   HB_P_MODULENAME,            /*  51 sets the name of debugged module */
/* start: pcodes generated by the macro compiler - the symbol address is used */
   HB_P_MMESSAGE,              /*  52 */
   HB_P_MPOPALIASEDFIELD,      /*  53 */
   HB_P_MPOPALIASEDVAR,        /*  54 */
   HB_P_MPOPFIELD,             /*  55 */
   HB_P_MPOPMEMVAR,            /*  56 */
   HB_P_MPUSHALIASEDFIELD,     /*  57 */
   HB_P_MPUSHALIASEDVAR,       /*  58 */
   HB_P_MPUSHBLOCK,            /*  59 */
   HB_P_MPUSHFIELD,            /*  60 */
   HB_P_MPUSHMEMVAR,           /*  61 */
   HB_P_MPUSHMEMVARREF,        /*  62 */
   HB_P_MPUSHSYM,              /*  63 */
   HB_P_MPUSHVARIABLE,         /*  64 Note also 132 HB_P_MPUSHSTR */
/* end: */
   HB_P_MULT,                  /*  65 multiplies the latest two values on the stack, removing them and leaving there the result */
   HB_P_NEGATE,                /*  66 numerically negates the latest value on the stack */
   HB_P_NOOP,                  /*  67 no operation */
   HB_P_NOT,                   /*  68 logically negates the latest value on the stack */
   HB_P_NOTEQUAL,              /*  69 checks if the latest two stack values are equal, leaves just the result */
   HB_P_OR,                    /*  70 peforms the logical OR of two latest stack values, removes them and places result */
   HB_P_PARAMETER,             /*  71 creates PRIVATE variables and assigns values to functions paramaters */
   HB_P_PLUS,                  /*  72 adds the latest two values on the stack, removing them and leaving there the result */
   HB_P_POP,                   /*  73 removes the latest value from the stack */
   HB_P_POPALIAS,              /*  74 pops the item from the eval stack and selects the current workarea */
   HB_P_POPALIASEDFIELD,       /*  75 pops aliased field */
   HB_P_POPALIASEDFIELDNEAR,   /*  76 pops aliased field */
   HB_P_POPALIASEDVAR,         /*  77 pops aliased variable (either a field or a memvar) */
   HB_P_POPFIELD,              /*  78 pops unaliased field */
   HB_P_POPLOCAL,              /*  79 pops the contains of the virtual machine stack onto a local variable */
   HB_P_POPLOCALNEAR,          /*  80 pops the contains of the virtual machine stack onto a local variable */
   HB_P_POPMEMVAR,             /*  81 pops the contains of a memvar variable to the virtual machine stack */
   HB_P_POPSTATIC,             /*  82 pops the contains of the virtual machine stack onto a static variable */
   HB_P_POPVARIABLE,           /*  83 pops the contains of an undeclared variable from the virtual machine stack */
   HB_P_POWER,                 /*  84 calculates the power of the two values on the stack, removing them and leaving there the result */
   HB_P_PUSHALIAS,             /*  85 saves the current workarea number on the eval stack */
   HB_P_PUSHALIASEDFIELD,      /*  86 pushes aliased field */
   HB_P_PUSHALIASEDFIELDNEAR,  /*  87 pushes aliased field */
   HB_P_PUSHALIASEDVAR,        /*  88 pushes aliased variable (either a field or a memvar) */
   HB_P_PUSHBLOCK,             /*  89 start of a codeblock definition */
   HB_P_PUSHBLOCKSHORT,        /*  90 start of a codeblock definition */
   HB_P_PUSHFIELD,             /*  91 pushes unaliased field */
   HB_P_PUSHBYTE,              /*  92 places a 1 byte integer number on the virtual machine stack */
   HB_P_PUSHINT,               /*  93 places an integer number on the virtual machine stack */
   HB_P_PUSHLOCAL,             /*  94 pushes the contains of a local variable to the virtual machine stack */
   HB_P_PUSHLOCALNEAR,         /*  95 pushes the contains of a local variable to the virtual machine stack */
   HB_P_PUSHLOCALREF,          /*  96 pushes a local variable by reference to the virtual machine stack */
   HB_P_PUSHLONG,              /*  97 places an integer number on the virtual machine stack */
   HB_P_PUSHMEMVAR,            /*  98 pushes the contains of a memvar variable to the virtual machine stack */
   HB_P_PUSHMEMVARREF,         /*  99 pushes the a memvar variable by reference to the virtual machine stack */
   HB_P_PUSHNIL,               /* 100 places a nil on the virtual machine stack */
   HB_P_PUSHDOUBLE,            /* 101 places a double number on the virtual machine stack */
   HB_P_PUSHSELF,              /* 102 pushes Self for the current processed method */
   HB_P_PUSHSTATIC,            /* 103 pushes the contains of a static variable to the virtual machine stack */
   HB_P_PUSHSTATICREF,         /* 104 pushes the a static variable by reference to the virtual machine stack */
   HB_P_PUSHSTR,               /* 105 places a string on the virtual machine stack */
   HB_P_PUSHSTRSHORT,          /* 106 places a string on the virtual machine stack */
   HB_P_PUSHSYM,               /* 107 places a symbol on the virtual machine stack */
   HB_P_PUSHSYMNEAR,           /* 108 places a symbol on the virtual machine stack */
   HB_P_PUSHVARIABLE,          /* 109 pushes the contains of an undeclared variable to the virtual machine stack */
   HB_P_RETVALUE,              /* 110 instructs the virtual machine to return the latest stack value */
   HB_P_SEND,                  /* 111 send operator */
   HB_P_SENDSHORT,             /* 112 send operator */
   HB_P_SEQBEGIN,              /* 113 BEGIN SEQUENCE */
   HB_P_SEQEND,                /* 114 END SEQUENCE */
   HB_P_SEQRECOVER,            /* 115 RECOVER statement */
   HB_P_SFRAME,                /* 116 sets the statics frame for a function */
   HB_P_STATICS,               /* 117 defines the number of statics variables for a PRG */
   HB_P_STATICNAME,            /* 118 sets the name of static variable */
   HB_P_SWAPALIAS,             /* 119 restores the current workarea number from the eval stack */
   HB_P_TRUE,                  /* 120 pushes true on the virtual machine stack */
   HB_P_ZERO,                  /* 121 places a ZERO on the virtual machine stack */
   HB_P_ONE,                   /* 122 places a ONE on the virtual machine stack */
   HB_P_MACROLIST,             /* 123 HB_P_MACROPUSHLIST envelope start. */
   HB_P_MACROLISTEND,          /* 124 HB_P_MACROPUSHLIST envelope end. */
   HB_P_LOCALNEARADDINT,       /* 125 Add/Subtract specified int into specified local without using the stack. */
   HB_P_LOCALNEARSETINT,       /* 126 Set specified int into specified local without using the stack.*/
   HB_P_LOCALNEARSETSTR,       /* 127 Set specified string into specified local without using the stack.*/
   HB_P_ADDINT,                /* 128 Add/Subtract specified int onto Stack Top value without extra push/pop. */
   HB_P_LEFT,                  /* 129 Optimized Left(). */
   HB_P_RIGHT,                 /* 130 Optimized Right(). */
   HB_P_SUBSTR,                /* 131 Optimized SubStr() */
   HB_P_MPUSHSTR,              /* 132 Macro compiled Pushed String. */
   HB_P_BASELINE,              /* 133 Line Number Offset. */
   HB_P_LINEOFFSET,            /* 134 Line Number Offset. */
   HB_P_WITHOBJECT,            /* 135 */
   HB_P_SENDWITH,              /* 136 */
   HB_P_SENDWITHSHORT,         /* 137 */
   HB_P_ENDWITHOBJECT,         /* 138 */
   HB_P_FOREACH,               /* 139 */
   HB_P_ENUMERATE,             /* 140 */
   HB_P_ENDENUMERATE,          /* 141 */
   HB_P_PUSHGLOBAL,            /* 142 */
   HB_P_POPGLOBAL,             /* 143 */
   HB_P_PUSHGLOBALREF,         /* 144 */
   HB_P_ENUMINDEX,             /* 145 */
   HB_P_SWITCHCASE,            /* 146 */
   HB_P_LIKE,                  /* 147 */
   HB_P_MATCH,                 /* 148 */
   HB_P_PUSHMACROREF,          /* 149 */
   HB_P_IVARREF,               /* 150 */
   HB_P_CLASSSETMODULE,        /* 151 */
   HB_P_BITAND,                /* 152 */
   HB_P_BITOR,                 /* 153 */
   HB_P_BITXOR,                /* 154 */
   HB_P_BITSHIFTR,             /* 155 */
   HB_P_BITSHIFTL,             /* 156 */
   HB_P_LARGEFRAME,            /* 157 */
   HB_P_PUSHWITH,              /* 158 */
   HB_P_PUSHLONGLONG,          /* 159 places 64bit integer number on the virtual machine stack */
   HB_P_PUSHSTRHIDDEN,         /* 160 places a "hidden" string on the virtual machine stack */
   HB_P_LOCALNEARSETSTRHIDDEN, /* 161 Set specified "hidden" string into specified local without using the stack.*/
   HB_P_TRYBEGIN,              /* 162 TRY */
   HB_P_TRYEND,                /* 163 END <TRY>*/
   HB_P_TRYRECOVER,            /* 164 CATCH */
   HB_P_FINALLY,               /* 165 CATCH */
   HB_P_ENDFINALLY,            /* 166 CATCH */
   HB_P_LOCALNEARADD,          /* 167 */
   HB_P_ARRAYPUSHREF,          /* 168 */
   HB_P_ARRAYPOPPLUS,          /* 169 */
   HB_P_PUSHDATETIME,          /* 170 places an date on the virtual machine stack */
   HB_P_PUSHDATE,              /* 171 */
   HB_P_HASHGEN,               /* 172 */
   HB_P_LOCALNEARINC,          /* 173 */
   HB_P_LOCALNEARDEC,          /* 174 */
   HB_P_PUSHLOCALNEARINC,      /* 175 */
   HB_P_PUSHLOCALNEARDEC,      /* 176 */
   HB_P_DIVERT,                /* 177 */
   HB_P_DIVERTOF,              /* 178*/

/* NOTE: This have to be the last definition */
   HB_P_LAST_PCODE             /* 179 this defines the number of defined pcodes */
} HB_PCODE;

#endif /* HB_PCODE_H_ */
