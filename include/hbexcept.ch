/*
 * $Id: hbexcept.ch 9482 2012-06-21 13:32:16Z andijahja $
 */

#ifndef EXCEPT_H
   #define EXCEPT_H

   #ifdef __PLATFORM__Windows
      #define EXCEPTION_EXECUTE_HANDLER    1
      #define SEM_NOGPFAULTERRORBOX        2

      #include "cstruct.ch"
      #include "wintypes.ch"

      #define STATUS_ABANDONED_WAIT_0          0x00000080
      #define STATUS_USER_APC                  0x000000C0
      #define STATUS_TIMEOUT                   0x00000102
      #define STATUS_PENDING                   0x00000103
      #define STATUS_SEGMENT_NOTIFICATION      0x40000005
      #define STATUS_GUARD_PAGE_VIOLATION      0x80000001
      #define STATUS_DATATYPE_MISALIGNMENT     0x80000002
      #define STATUS_BREAKPOINT                0x80000003
      #define STATUS_SINGLE_STEP               0x80000004
      #define STATUS_ACCESS_VIOLATION          0xC0000005
      #define STATUS_IN_PAGE_ERROR             0xC0000006
      #define STATUS_INVALID_HANDLE            0xC0000008
      #define STATUS_NO_MEMORY                 0xC0000017
      #define STATUS_ILLEGAL_INSTRUCTION       0xC000001D
      #define STATUS_NONCONTINUABLE_EXCEPTION  0xC0000025
      #define STATUS_INVALID_DISPOSITION       0xC0000026
      #define STATUS_ARRAY_BOUNDS_EXCEEDED     0xC000008C
      #define STATUS_FLOAT_DENORMAL_OPERAND    0xC000008D
      #define STATUS_FLOAT_DIVIDE_BY_ZERO      0xC000008E
      #define STATUS_FLOAT_INEXACT_RESULT      0xC000008F
      #define STATUS_FLOAT_INVALID_OPERATION   0xC0000090
      #define STATUS_FLOAT_OVERFLOW            0xC0000091
      #define STATUS_FLOAT_STACK_CHECK         0xC0000092
      #define STATUS_FLOAT_UNDERFLOW           0xC0000093
      #define STATUS_INTEGER_DIVIDE_BY_ZERO    0xC0000094
      #define STATUS_INTEGER_OVERFLOW          0xC0000095
      #define STATUS_PRIVILEGED_INSTRUCTION    0xC0000096
      #define STATUS_STACK_OVERFLOW            0xC00000FD
      #define STATUS_CONTROL_C_EXIT            0xC000013A
      #define STATUS_FLOAT_MULTIPLE_FAULTS     0xC00002B4
      #define STATUS_FLOAT_MULTIPLE_TRAPS      0xC00002B5
      #define STATUS_REG_NAT_CONSUMPTION       0xC00002C9

      #define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
      #define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
      #define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
      #define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
      #define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
      #define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
      #define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
      #define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
      #define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
      #define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
      #define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
      #define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
      #define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
      #define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
      #define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
      #define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
      #define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
      #define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
      #define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
      #define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
      #define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
      #define EXCEPTION_INVALID_HANDLE            STATUS_INVALID_HANDLE

      #define EXCEPTION_MAXIMUM_PARAMETERS 15

      typedef struct _EXCEPTION_RECORD {  ;
        DWORD ExceptionCode;
        DWORD ExceptionFlags;
        struct _EXCEPTION_RECORD* ExceptionRecord;
        PVOID ExceptionAddress;
        DWORD NumberParameters;
        ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
      } EXCEPTION_RECORD, *PEXCEPTION_RECORD

      typedef struct _CONTEXT {;
          ULONGLONG FltF0;
          ULONGLONG FltF1;
          ULONGLONG FltF2;
          ULONGLONG FltF3;
          ULONGLONG FltF4;
          ULONGLONG FltF5;
          ULONGLONG FltF6;
          ULONGLONG FltF7;
          ULONGLONG FltF8;
          ULONGLONG FltF9;
          ULONGLONG FltF10;
          ULONGLONG FltF11;
          ULONGLONG FltF12;
          ULONGLONG FltF13;
          ULONGLONG FltF14;
          ULONGLONG FltF15;
          ULONGLONG FltF16;
          ULONGLONG FltF17;
          ULONGLONG FltF18;
          ULONGLONG FltF19;
          ULONGLONG FltF20;
          ULONGLONG FltF21;
          ULONGLONG FltF22;
          ULONGLONG FltF23;
          ULONGLONG FltF24;
          ULONGLONG FltF25;
          ULONGLONG FltF26;
          ULONGLONG FltF27;
          ULONGLONG FltF28;
          ULONGLONG FltF29;
          ULONGLONG FltF30;
          ULONGLONG FltF31;
          ULONGLONG IntV0;
          ULONGLONG IntT0;
          ULONGLONG IntT1;
          ULONGLONG IntT2;
          ULONGLONG IntT3;
          ULONGLONG IntT4;
          ULONGLONG IntT5;
          ULONGLONG IntT6;
          ULONGLONG IntT7;
          ULONGLONG IntS0;
          ULONGLONG IntS1;
          ULONGLONG IntS2;
          ULONGLONG IntS3;
          ULONGLONG IntS4;
          ULONGLONG IntS5;
          ULONGLONG IntFp;
          ULONGLONG IntA0;
          ULONGLONG IntA1;
          ULONGLONG IntA2;
          ULONGLONG IntA3;
          ULONGLONG IntA4;
          ULONGLONG IntA5;
          ULONGLONG IntT8;
          ULONGLONG IntT9;
          ULONGLONG IntT10;
          ULONGLONG IntT11;
          ULONGLONG IntRa;
          ULONGLONG IntT12;
          ULONGLONG IntAt;
          ULONGLONG IntGp;
          ULONGLONG IntSp;
          ULONGLONG IntZero;
          ULONGLONG Fpcr;
          ULONGLONG SoftFpcr;
          ULONGLONG Fir;
          DWORD Psr;
          DWORD ContextFlags;
          DWORD Fill[4];
      } CONTEXT, *PCONTEXT

      typedef struct _EXCEPTION_POINTERS { ;
        PEXCEPTION_RECORD ExceptionRecord;
        PCONTEXT ContextRecord;
      } EXCEPTION_POINTERS, *PEXCEPTION_POINTERS

   #endif

#endif
