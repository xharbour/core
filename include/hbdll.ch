/*
 * $Id: hbdll.ch 9791 2012-10-31 11:34:30Z andijahja $
 */

#ifndef __DLL_CH
   #define __DLL_CH

   #include "hbdll.h"

   // Avoid xHarbour code when included by dllcall.c
   #ifdef __PLATFORM__Windows
      #include "cstruct.ch"
      #include "wintypes.ch"

      // Native Syntax
      #xcommand IMPORT [<Qualifier: STATIC>] [<CallConvention: 0x0000, 0x0001, 0x0010, 0x0020, 0x0100, 0x0200> ] ;
                       <FuncName>( [<ArgName> [, <ArgNameN>] ] ) FROM [<!DllFuncName!> OF ] <(DllName)> => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.CallConvention.>, <CallConvention>, DC_CALL_STD ), IIF( <.DllFuncName.>, #<DllFuncName>, #<FuncName> ) );;
             ENDIF;;
             ;;
             IF <.ArgName.>;;
                RETURN DllExecuteCall( cTemplate, <ArgName> [, <ArgNameN>] );;
             ENDIF;;
          RETURN DllExecuteCall( cTemplate )

      #xcommand IMPORT [<Qualifier: STATIC>] [<CallConvention: 0x0000, 0x0001, 0x0010, 0x0020, 0x0100, 0x0200> ] ;
                       <FuncName>( [<ArgName> [, <ArgNameN>] ] ) FROM <(DllName)> [EXPORTED AS <!DllFuncName!>] => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.CallConvention.>, <CallConvention>, DC_CALL_STD ), IIF( <.DllFuncName.>, #<DllFuncName>, #<FuncName> ) );;
             ENDIF;;
             ;;
             IF <.ArgName.>;;
                RETURN DllExecuteCall( cTemplate, <ArgName> [, <ArgNameN>] );;
             ENDIF;;
          RETURN DllExecuteCall( cTemplate )


      #xcommand IMPORT [<Qualifier: STATIC>] [<CallConvention: 0x0000, 0x0001, 0x0010, 0x0020, 0x0100, 0x0200> ] ;
                       <Type> <FuncName>( [0] [<ArgType> <ArgName> [, <ArgTypeN> <ArgNameN>] ] ) FROM <(DllName)> [EXPORTED AS <!DllFuncName!>] => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.CallConvention.>, <CallConvention>, DC_CALL_STD ), IIF( <.DllFuncName.>, #<DllFuncName>, #<FuncName> ) );;
             ENDIF;;
             ;;
             IF <.ArgName.>;;
                RETURN DllExecuteCall( cTemplate, <ArgName> [, <ArgNameN>] );;
             ENDIF;;
          RETURN DllExecuteCall( cTemplate )

      #xcommand IMPORT [<Qualifier: STATIC>] [<CallConvention: 0x0000, 0x0001, 0x0010, 0x0020, 0x0100, 0x0200> ] ;
                       <Type> <FuncName>( [0] [<ArgType> <ArgName> [, <ArgTypeN> <ArgNameN>] ] ) FROM [<!DllFuncName!> OF ] <(DllName)> => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.CallConvention.>, <CallConvention>, DC_CALL_STD ), IIF( <.DllFuncName.>, #<DllFuncName>, #<FuncName> ) );;
             ENDIF;;
             ;;
             IF <.ArgName.>;;
                RETURN DllExecuteCall( cTemplate, <ArgName> [, <ArgNameN>] );;
             ENDIF;;
          RETURN DllExecuteCall( cTemplate )

      // FoxPro Syntax
      #xcommand DECLARE <!Type!> <!FuncName!> IN <(DllName)> [AS <(DllFuncName)>] [<ArgType> [@] [<ArgName>] [, <ArgTypeN> [@] [<ArgNameN>]]] => ;
          UTILITY FUNCTION <FuncName>( ... );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, DC_CALL_STD, IIF( <.DllFuncName.>, <(DllFuncName)>, #<FuncName> ) );;
             ENDIF;;
             ;;
             SWITCH PCount();;
                CASE 0;;
                   RETURN DllExecuteCall( cTemplate );;
                CASE 1;;
                   RETURN DllExecuteCall( cTemplate, PValue(1) );;
                CASE 2;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2) );;
                CASE 3;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3) );;
                CASE 4;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4) );;
                CASE 5;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4), PValue(5) );;
                CASE 6;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4), PValue(5), PValue(6) );;
                CASE 7;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4), PValue(5), PValue(6), PValue(7) );;
                CASE 8;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4), PValue(5), PValue(6), PValue(7), PValue(8) );;
                CASE 9;;
                   RETURN DllExecuteCall( cTemplate, PValue(1), PValue(2), PValue(3), PValue(4), PValue(5), PValue(6), PValue(7), PValue(8), PValue(9) );;
             END;;
             ;;
          RETURN NIL

      // FWH
      #xcommand DLL [<Qualifier: STATIC>] FUNCTION <FuncName>( [ <ArgName> AS <ArgType> ] [, <ArgNameN> AS <ArgTypeN> ] ) AS <Type> [<Pascal: PASCAL>] [ FROM <DllFuncName> ] LIB <(DllName)> => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.Pascal.>, DC_CALL_STD, DC_CALL_CDECL ), IIF( <.DllFuncName.>, <(DllFuncName)>, #<FuncName> ) );;
             ENDIF;;
             ;;
          RETURN DllExecuteCall( cTemplate [, <ArgName>] [, <ArgNameN>] )

      #xcommand DLL32 [<Qualifier: STATIC>] FUNCTION <FuncName>( [ <ArgName> AS <ArgType> ] [, <ArgNameN> AS <ArgTypeN> ] ) AS <Type> [<Pascal: PASCAL>] [ FROM <DllFuncName> ] LIB <(DllName)> => ;
          UTILITY [<Qualifier>] FUNCTION <FuncName>( <ArgName> [, <ArgNameN>] );;
             STATIC cTemplate;;
             ;;
             IF cTemplate == NIL;;
                cTemplate := DllPrepareCall( <(DllName)>, IIF( <.Pascal.>, DC_CALL_STD, DC_CALL_CDECL ), IIF( <.DllFuncName.>, <(DllFuncName)>, #<FuncName> ) );;
             ENDIF;;
             ;;
          RETURN DllExecuteCall( cTemplate [, <ArgName>] [, <ArgNameN>] )

   #endif
#endif
