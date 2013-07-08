/*
* $Id: thread.h 9839 2012-11-25 23:43:46Z andijahja $
*/

/*
* xHarbour Project source code:
* The MT support
*
* Copyright 2002 Giancarlo Niccolai [gian@niccolai.ws]
* www - http://www.xharbour.org
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
* As a special exception, xHarbour license gives permission for
* additional uses of the text contained in its release of xHarbour.
*
* The exception is that, if you link the xHarbour libraries with other
* files to produce an executable, this does not by itself cause the
* resulting executable to be covered by the GNU General Public License.
* Your use of that executable is in no way restricted on account of
* linking the xHarbour library code into it.
*
* This exception does not however invalidate any other reasons why
* the executable file might be covered by the GNU General Public License.
*
* This exception applies only to the code released with this xHarbour
* explicit exception.  If you add/copy code from other sources,
* as the General Public License permits, the above exception does
* not apply to the code that you add in this way.  To avoid misleading
* anyone as to the status of such modified files, you must delete
* this exception notice from them.
*
* If you write modifications of your own for xHarbour, it is your choice
* whether to permit this exception to apply to your modifications.
* If you do not wish that, delete this exception notice.
*
*/


#ifndef HB_THREAD_H_
#define HB_THREAD_H_

#include "hbapi.h"

typedef struct
{
   USHORT   uiFError;
   USHORT   uiErrorLast;
   USHORT   uiOsErrorLast;
   HB_ERRCODE uiSocketError;
   int      iSocketOsError;
}
HB_IOERRORS, * PHB_IOERRORS;

typedef struct _HB_STACKRDD_TLS
{
   BOOL           fNetError;        /* current NETERR() flag */
   USHORT         uiCurrArea;       /* Current WokrArea number */
   void *         pCurrArea;        /* Current WorkArea pointer */
} HB_STACKRDD_TLS, * PHB_STACKRDD_TLS;

#ifdef HB_THREAD_SUPPORT

#if defined( HB_OS_WIN )
   /* This section should be above any #includes that may #include <windows.h> */
   #ifndef _WIN32_WINNT
      #define _WIN32_WINNT 0x0403
      /* #define _WIN32_WINNT 0x0400 */
   #endif
   #define _WINSOCKAPI_  /* Prevents inclusion of Winsock.h in Windows.h */

   /* Prevent inclusion of ole2.h and other extraneous headers in windows.h */
   #define WIN32_LEAN_AND_MEAN

   #ifndef CINTERFACE
   #define CINTERFACE
   #endif

   #include <windows.h>
#endif

#include "hbatomic.h"
#include "hbmath.h"

#if defined( __DMC__ )
   #if !defined( _MT )
      #define _MT
   #endif
   #if !defined( __NT__ )
      #define __NT__
   #endif
#endif

#define HB_ATOMIC_INC( v ) HB_ATOM_INC( &( v ) )
#define HB_ATOMIC_DEC( v ) HB_ATOM_DEC( &( v ) )

/* disable this macro if malloca/free() is not thread safe */
#define HB_SAFE_ALLOC

/* We should assert that cleanup functions must be in limited number */
typedef void (*HB_CLEANUP_FUNC)(void *);
#define HB_MAX_CLEANUPS  12
#define HB_THREAD_MAX_UNIQUE_ID  32000
#define HB_MUTEX_SIGNATURE       0xF0316913

/* Maximum number of cycles that can be completed by VM without stack unlock */
#define HB_VM_UNLOCK_PERIOD 5000

#if defined(HB_OS_WIN)
   typedef struct tag_HB_WINCOND_T
   {
      HANDLE semBlockLock;
      HANDLE semBlockQueue;
      CRITICAL_SECTION mtxUnblockLock;
      int nWaitersGone;
      int nWaitersBlocked;
      int nWaitersToUnblock;
   } HB_WINCOND_T, *PHB_WINCOND_T;

   #define HB_THREAD_T                 unsigned

   #define HB_CRITICAL_T               CRITICAL_SECTION
   #define HB_CRITICAL_INIT( x )       InitializeCriticalSection( &(x) )
   #define HB_CRITICAL_DESTROY( x )    DeleteCriticalSection( &(x) )
   #define HB_CRITICAL_LOCK( x )       EnterCriticalSection( &(x) )
   #define HB_CRITICAL_UNLOCK( x )     LeaveCriticalSection( &(x) )
   #define HB_CRITICAL_TRYLOCK( x )    TryEnterCriticalSection( &(x) )

   #define HB_MUTEX_T                  HANDLE
   #define HB_MUTEX_INIT( x )          ( x = CreateSemaphore( NULL, 1, 1, NULL) )
   #define HB_MUTEX_DESTROY( x )       CloseHandle( x )
   #define HB_MUTEX_LOCK( x )          WaitForSingleObject( x, INFINITE )
   #define HB_MUTEX_UNLOCK( x )        ReleaseSemaphore( x, 1, NULL )

   #define HB_COND_T                   HB_WINCOND_T
   #define HB_COND_INIT( x )           hb_threadCondInit( &(x) )
   #define HB_COND_WAIT( x, y )        hb_threadCondWait( &(x), &(y), INFINITE )
   #define HB_COND_WAITTIME( x, y, t ) hb_threadCondWait( &(x), &(y), t )
   #define HB_COND_SIGNAL( x )         hb_threadCondSignal( &(x) )
   #define HB_COND_DESTROY( x )        hb_threadCondDestroy( &(x) )

   #define HB_CURRENT_THREAD           GetCurrentThreadId
   #define HB_SAME_THREAD(x, y)        ((x) == (y))

   /* Guard for cancellation requets */
   HB_EXTERN_BEGIN
   extern HB_CRITICAL_T hb_cancelMutex;
   HB_EXTERN_END

   #define HB_ENABLE_ASYN_CANC       HB_THREAD_GUARD( hb_cancelMutex, HB_VM_STACK.bCanCancel = TRUE )
   #define HB_DISABLE_ASYN_CANC      HB_THREAD_GUARD( hb_cancelMutex, HB_VM_STACK.bCanCancel = FALSE )
   #define HB_TEST_CANCEL_ENABLE_ASYN\
   {\
      HB_CRITICAL_LOCK( hb_cancelMutex );\
      if ( HB_VM_STACK.bCanceled )\
      {\
         HB_CRITICAL_UNLOCK( hb_cancelMutex );\
         hb_threadCancelInternal();\
      }\
      HB_VM_STACK.bCanCancel = TRUE;\
      HB_CRITICAL_UNLOCK( hb_cancelMutex );\
   }

   #define HB_TEST_CANCEL\
   {\
      HB_CRITICAL_LOCK( hb_cancelMutex );\
      if ( HB_VM_STACK.bCanceled )\
      {\
         HB_CRITICAL_UNLOCK( hb_cancelMutex );\
         hb_threadCancelInternal();\
      }\
      HB_CRITICAL_UNLOCK( hb_cancelMutex );\
   }

   #define HB_CLEANUP_PUSH(X,Y) {\
      HB_VM_STACK.pCleanUp[ HB_VM_STACK.iCleanCount ] = (X);\
      HB_VM_STACK.pCleanUpParam[ HB_VM_STACK.iCleanCount ] = (void *)&(Y);\
      HB_VM_STACK.iCleanCount++;\
   }

   #define HB_CLEANUP_POP (HB_VM_STACK.iCleanCount--);

   #define HB_CLEANUP_POP_EXEC {\
      HB_VM_STACK.iCleanCount--;\
      HB_VM_STACK.pCleanUp[ HB_VM_STACK.iCleanCount ]( HB_VM_STACK.pCleanUpParam[ HB_VM_STACK.iCleanCount ]);\
   }

#if !defined( _HB_STACK_LOCAL_MACROS_ )
#ifdef __cplusplus
extern "C" {
#endif
#ifndef __IMPORT__
extern HB_EXPORT DWORD hb_dwCurrentStack;
#else
extern HB_IMPORT DWORD hb_dwCurrentStack;
#endif
#ifdef __cplusplus
}
#endif
#endif /* _HB_STACK_LOCAL_MACROS_ */

   #define hb_threadGetCurrentStack() ( (HB_STACK *) TlsGetValue( hb_dwCurrentStack ) )


/* 10/05/2004 - <maurilio.longo@libero.it>
   Built and tested with Innotek GCC for OS/2
*/
#elif defined(HB_OS_OS2)

   #define CRITICAL_SECTION            HMTX

   #define HB_COND_T                   HMUX
   #define PHB_COND_T                  PHMUX

   #define DWORD                       ULONG
   #define HB_THREAD_T                 TID

   #define HB_CRITICAL_T               HMTX
   #define HB_CRITICAL_INIT( x )       DosCreateMutexSem(NULL, &(x), 0L, FALSE)      // Creates a private, unnamed, unowned sem
   #define HB_CRITICAL_DESTROY( x )    DosCloseMutexSem( x )
   #define HB_CRITICAL_LOCK( x )       hb_threadMtxPoll( x )
   #define HB_CRITICAL_UNLOCK( x )     DosReleaseMutexSem( x )
   #define HB_CRITICAL_TRYLOCK( x )    (DosRequestMutexSem( x, SEM_IMMEDIATE_RETURN ) == NO_ERROR)

   #define HB_MUTEX_T                  HMTX
   #define HB_MUTEX_INIT( x )          HB_CRITICAL_INIT( x )
   #define HB_MUTEX_DESTROY( x )       HB_CRITICAL_DESTROY( x )
   #define HB_MUTEX_LOCK( x )          HB_CRITICAL_LOCK( x )
   #define HB_MUTEX_UNLOCK( x )        HB_CRITICAL_UNLOCK( x )

   #define HB_COND_INIT( x )           hb_threadCondInit( &(x) )
   #define HB_COND_WAIT( x, y )        hb_threadCondWait( &(x), &(y), SEM_INDEFINITE_WAIT )
   #define HB_COND_WAITTIME( x, y, t ) hb_threadCondWait( &(x), &(y), t )
   #define HB_COND_SIGNAL( x )         hb_threadCondSignal( &(x) )
   #define HB_COND_DESTROY( x )        hb_threadCondDestroy( &(x) )

   #define HB_CURRENT_THREAD           _gettid
   #define HB_SAME_THREAD(x, y)        ((x) == (y))

   /* Guard for cancellation requets */
   extern HB_CRITICAL_T hb_cancelMutex;

   #define HB_ENABLE_ASYN_CANC       HB_THREAD_GUARD( hb_cancelMutex, HB_VM_STACK.bCanCancel = TRUE )
   #define HB_DISABLE_ASYN_CANC      HB_THREAD_GUARD( hb_cancelMutex, HB_VM_STACK.bCanCancel = FALSE )
   #define HB_TEST_CANCEL_ENABLE_ASYN\
   {\
      HB_CRITICAL_LOCK( hb_cancelMutex );\
      if ( HB_VM_STACK.bCanceled )\
      {\
         HB_CRITICAL_UNLOCK( hb_cancelMutex );\
         hb_threadCancelInternal();\
      }\
      HB_VM_STACK.bCanCancel = TRUE;\
      HB_CRITICAL_UNLOCK( hb_cancelMutex );\
   }

   #define HB_TEST_CANCEL\
   {\
      HB_CRITICAL_LOCK( hb_cancelMutex );\
      if ( HB_VM_STACK.bCanceled )\
      {\
         HB_CRITICAL_UNLOCK( hb_cancelMutex );\
         hb_threadCancelInternal();\
      }\
      HB_CRITICAL_UNLOCK( hb_cancelMutex );\
   }


   #define HB_CLEANUP_PUSH(X,Y) {\
      HB_VM_STACK.pCleanUp[ HB_VM_STACK.iCleanCount ] = (X);\
      HB_VM_STACK.pCleanUpParam[ HB_VM_STACK.iCleanCount ] = (void *)&(Y);\
      HB_VM_STACK.iCleanCount++;\
   }

   #define HB_CLEANUP_POP (HB_VM_STACK.iCleanCount--);

   #define HB_CLEANUP_POP_EXEC {\
      HB_VM_STACK.iCleanCount--;\
      HB_VM_STACK.pCleanUp[ HB_VM_STACK.iCleanCount ]( HB_VM_STACK.pCleanUpParam[ HB_VM_STACK.iCleanCount ]);\
   }


#ifdef __cplusplus
extern "C" {
#endif
extern PPVOID hb_dwCurrentStack;
#ifdef __cplusplus
}
#endif

   #define  hb_threadGetCurrentStack() ( (HB_STACK *) *hb_dwCurrentStack )

#else

   #include <pthread.h>
   #include <semaphore.h>
   #include <errno.h>
   #define HB_THREAD_T                 pthread_t

#ifdef HB_OS_DARWIN_5
   #define HB_NO_RECURSIVE_MUTEXES
#endif

#ifndef HB_NO_RECURSIVE_MUTEXES

   #define HB_CRITICAL_T               pthread_mutex_t

#ifdef HB_OS_DARWIN
   /* Darwin 7.0.0 has only pthread_mutexattr_settype() */
   #define pthread_mutexattr_setkind_np pthread_mutexattr_settype
#else

   #ifdef HB_OS_HPUX
   #define pthread_mutexattr_setkind_np pthread_mutexattr_settype
   #else
   /* ODD: this definition is missing on some linux headers;
      we should remove it when this bug is fixed */
   int pthread_mutexattr_setkind_np( pthread_mutexattr_t * attr, int kind );
   #endif
#endif

   /* Some Unices (e.g., FreeBSD 4.8) don't have this define: */
   #ifdef HB_OS_BSD
      #define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
   #endif

   #ifdef HB_OS_HPUX
      #define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
   #endif

   #define HB_CRITICAL_INIT( x )       \
      {\
         pthread_mutexattr_t attr;\
         pthread_mutexattr_init( &attr );\
         pthread_mutexattr_setkind_np( &attr, PTHREAD_MUTEX_RECURSIVE_NP);\
         pthread_mutex_init( &(x), &attr );\
         pthread_mutexattr_destroy( &attr );\
      }
   #define HB_CRITICAL_DESTROY( x )    pthread_mutex_destroy( &(x) )
   #define HB_CRITICAL_LOCK( x )       pthread_mutex_lock( &(x) )
   #define HB_CRITICAL_UNLOCK( x )     pthread_mutex_unlock( &(x) )
   #define HB_CRITICAL_TRYLOCK( x )    ( pthread_mutex_trylock( &(x) ) != EBUSY )

#else

   /* Some Unices (e.g., Darwin 5.5) don't have recursive mutexes;
    * we have to implement them manually. -- Ph.K. */
   typedef struct
   {
      pthread_mutex_t lock;
      HB_THREAD_T owner;
      ULONG count;
   } HB_RECURSIVE_MUTEX_T;

   #define HB_CRITICAL_T               HB_RECURSIVE_MUTEX_T
   #define HB_CRITICAL_INIT( x ) \
      { \
         pthread_mutex_init( &((x).lock), NULL ); \
	 (x).owner = 0; \
	 (x).count = 0; \
      }
   #define HB_CRITICAL_DESTROY( x )    pthread_mutex_destroy( &((x).lock) )
   #define HB_CRITICAL_LOCK( x ) \
      { \
         HB_THREAD_T self = pthread_self(); \
	 if ( pthread_equal( (x).owner, self ) ) \
	 { \
	    (x).count++; \
	 } \
	 else \
	 { \
	    pthread_mutex_lock( &((x).lock) ); \
            (x).owner = self; \
         } \
      }
   #define HB_CRITICAL_UNLOCK( x ) \
      { \
	 if ( pthread_equal( (x).owner, pthread_self() ) ) \
	 { \
            if ( (x).count ) \
            { \
               (x).count--; \
            } \
            else \
            { \
               (x).owner = 0; \
               pthread_mutex_unlock( &((x).lock) ); \
            } \
	 } \
      }

   #define HB_CRITICAL_TRYLOCK( x ) \
      ( pthread_equal( pthread_self(), (x).owner ) \
        ? ( (x).count++, TRUE ) \
	: ( ( pthread_mutex_trylock( &((x).lock) ) != EBUSY ) \
	    ? ( (x).owner = pthread_self(), TRUE ) \
	    : FALSE ) )

#endif

   #define HB_MUTEX_T                  HB_CRITICAL_T
   #define HB_MUTEX_INIT( x )          HB_CRITICAL_INIT( x )
   #define HB_MUTEX_DESTROY( x )       HB_CRITICAL_DESTROY( x )
   #define HB_MUTEX_LOCK( x )          HB_CRITICAL_LOCK( x )
   #define HB_MUTEX_UNLOCK( x )        HB_CRITICAL_UNLOCK( x )

   extern int hb_condTimeWait( pthread_cond_t *cond, pthread_mutex_t *mutex, int iMillisec );

   #define HB_COND_T                   pthread_cond_t
   #define HB_COND_INIT( x )           pthread_cond_init( &(x), NULL )
#ifndef HB_NO_RECURSIVE_MUTEXES
   #define HB_COND_WAIT( x, y )        pthread_cond_wait( &(x), &(y) )
   #define HB_COND_WAITTIME( x, y, t )  hb_condTimeWait( &(x) , &(y), t )
#else
   #define HB_COND_WAIT( x, y )        pthread_cond_wait( &(x), &((y).lock) )
   #define HB_COND_WAITTIME( x, y, t ) hb_condTimeWait( &(x), &((y).lock), t )
#endif
   #define HB_COND_SIGNAL( x )         pthread_cond_broadcast( &(x) )
   #define HB_COND_DESTROY( x )        pthread_cond_destroy( &(x) )

   #define HB_CURRENT_THREAD           pthread_self
   #define HB_CLEANUP_PUSH(x, y )      pthread_cleanup_push( x, (void *)&(y) )
   #define HB_CLEANUP_POP              pthread_cleanup_pop(0)
   #define HB_CLEANUP_POP_EXEC         pthread_cleanup_pop(1)
   #define HB_SAME_THREAD(x,y)         pthread_equal( x, y )

   #define HB_ENABLE_ASYN_CANC         pthread_testcancel();
   #define HB_DISABLE_ASYN_CANC        pthread_testcancel();
   #define HB_TEST_CANCEL_ENABLE_ASYN  pthread_testcancel();
   #define HB_TEST_CANCEL              pthread_testcancel();

   extern pthread_key_t hb_pkCurrentStack;
   #define hb_threadGetCurrentStack() ( (HB_STACK *) pthread_getspecific( hb_pkCurrentStack ) )

#endif

/**********************************************************/
/*
* Enanched stack for multithreading
*/

#if defined( _HB_API_INTERNAL_ )

#include "hbset.h" /* For stack definition */

/* Forward declarations for stack */
struct HB_ERROR_INFO_;
struct _AREA;
struct _HB_SEQUENCE;
struct tag_HB_THREAD_ID;
struct _HB_STACKRDD;

typedef struct tag_HB_STACK
{
   PHB_ITEM * pItems;           /* pointer to the stack items */
   PHB_ITEM * pPos;             /* pointer to the latest used item */
   PHB_ITEM * pEnd;             /* pointer to the end of stack items */
   long       wItems;           /* total items that may be holded on the stack */
   HB_ITEM    Return;           /* latest returned value */
   PHB_ITEM * pBase;            /* stack frame position for the current function call */
   PHB_ITEM * pEvalBase;        /* stack frame position for the evaluated codeblock */
   long       lStatics;         /* statics base for the current function call */
   long       lWithObject;      /* stack offset to base current WITH OBJECT item */
   HB_SIZE    lRecoverBase;     /* current SEQUENCE envelope offset or 0 if no SEQUENCE is active */
   //USHORT   uiActionRequest;  /* Request for some action - stop processing of opcodes */
   char       szDate[ 26 ];     /* last returned date from _pards() yyyymmdd format */
   struct _HB_STACKRDD * rdd;   /* RDD related data */
   HB_STACKRDD_TLS rddTls;      /* RDD related data which is always thread-local */
   HB_IOERRORS IOErrors;      /* MT safe buffer for IO errors */

   HB_SET_STRUCT set;
   /* JC1: thread safe classes messaging */
   struct hb_class_method * pMethod;        /* Selcted method to send message to */
   struct hb_class_sync * pSyncId;

   UINT th_vm_id;

   /* Pcode releasing accounting */
   int iPcodeCount;

   HB_THREAD_T th_id;
   /* Is this thread going to run a method? */
   BOOL bIsMethod;
   /* data to initialize the stack */
   UINT uiParams;
   /* Flag to signal that the context is in use */
   BOOL bInUse; /* this must be used with the guard of a global resource */
   BOOL bActive;
   /* Mark current thread as idle inspector  */
   USHORT uiIdleInspect;

   /* MT error handler, one for thread! */
   struct HB_ERROR_INFO_ *errorHandler;
   /* Codeblock for error handling */
   PHB_ITEM errorBlock;
   int     iLaunchCount;
   USHORT  uiErrorDOS; /* The value of DOSERROR() */

   /* Math error handler, again one for thread */
   HB_MATH_EXCEPTION    math_exc;

   /* List of error handlers for TRY/CATCH blocks */
   PHB_ITEM aTryCatchHandlerStack;

   /* Mt With Object index */
   HB_ITEM aWithObject[ HB_MAX_WITH_OBJECTS ];
   UINT    wWithObjectCounter;

   /* Mt for each enumeration index */
   HB_ITEM  aEnumCollection[ HB_MAX_ENUMERATIONS ];
   PHB_ITEM apEnumVar[ HB_MAX_ENUMERATIONS ];
   HB_SIZE  awEnumIndex[ HB_MAX_ENUMERATIONS ];
   UINT     wEnumCollectionCounter;

   /* management of codeblock and macro params */
   int aiExtraParams[HB_MAX_MACRO_ARGS];
   int iExtraParamsIndex;
   PHB_SYMB apExtraParamsSymbol[HB_MAX_MACRO_ARGS];
   int aiExtraElements[HB_MAX_MACRO_ARGS];
   int iExtraElementsIndex;
   int iExtraElements;
   int iExtraIndex;

   /* BEGIN SEQUENCE [RECOVER] END*/
   struct _HB_SEQUENCE *pSequence;

   struct _HB_FINALLY *pFinally;

   unsigned int uiVMFlags;

   /* FS api error system */
   USHORT uiErrorLast;
   USHORT uiOsErrorLast;

   /* Dynsym thread-specific table */
   UINT uiClosestDynSym;
   PDYNHB_ITEM pDynItems;
   USHORT uiDynSymbols;

   /* Management of PRIVATE variables (and macro memvars) */
   PHB_DYNS * privateStack;
   HB_SIZE privateStackSize;
   HB_SIZE privateStackCnt;
   HB_SIZE privateStackBase;

   /* Management of globals memvars */
   HB_SIZE globalTableSize;
   HB_SIZE globalFirstFree;
   HB_SIZE globalLastFree;
   PHB_VALUE globalTable;

   /* Pointers to hMemvar for thread aware dynsyms */
   HB_HANDLE *hMemvars;
   HB_SIZE hMemvarsAllocated;
   HB_SIZE hMemvarsLastFree;

   /* Background per-thread jobs */
   struct HB_BACKGROUNDTASK_ **pBackgroundTasks;
   BOOL bIamBackground;
   USHORT uiBackgroundTask;
   USHORT uiBackgroundMaxTask;
   ULONG  ulBackgroundID;

#if defined(HB_OS_WIN)
   HANDLE th_h;
#endif
#if defined(HB_OS_WIN) || defined(HB_OS_OS2)
   volatile BOOL bCanceled; /* set when there is a cancel request and bInUse is true */
   volatile BOOL bCanCancel;
   /* Windows cleanup functions */
   HB_CLEANUP_FUNC *pCleanUp;
   void **pCleanUpParam;
   int iCleanCount;
#endif
   struct tag_HB_THREAD_READY *pThreadReady;

   struct tag_HB_THREAD_ID *pThreadID;
   struct tag_HB_STACK *next;

} HB_STACK, * PHB_STACK;

/*********************************************************************/
/* Complex PRG LEVEL Mutex Structure                                 */

typedef struct tag_HB_MUTEX_STRUCT
{
   ULONG sign;
   HB_CRITICAL_T mutex;
   HB_COND_T cond;
   HB_THREAD_T locker;
   USHORT lock_count;
   int waiting;
   PHB_ITEM aEventObjects;
   struct tag_HB_MUTEX_STRUCT *next;
   struct tag_HB_MUTEX_STRUCT *prev;
}
HB_MUTEX_STRUCT, * PHB_MUTEX_STRUCT;

/*********************************************************************/
/* Thread object structure                                           */
#define HB_THREAD_ID_SIGN     0xF01B1A23

typedef struct tag_HB_THREAD_ID
{
   ULONG sign;
   /* System level thread id */
   HB_THREAD_T threadId;
   /* 0, NULL, -1 etc are not valid flags for unused ids. */
   BOOL bReady;
   /* Pointer to the thread stack. I can see a lot of uses for it */
   volatile HB_STACK *pStack;

   struct tag_HB_THREAD_READY *pThreadReady;

   /* Next thread id */
   struct tag_HB_THREAD_ID *next;
}
HB_THREAD_ID, *PHB_THREAD_ID;

typedef struct tag_HB_THREAD_READY
{
   volatile BOOL        bActive;
   HB_COUNTER           ulCounter;
} HB_THREAD_READY, *PHB_THREAD_READY;

#else

typedef void * PHB_STACK;
typedef void * PHB_MUTEX_STRUCT;

#endif /* _HB_API_INTERNAL_ */

/*********************************************************************/
/* Shared resource is a set of a resource, a mutex and a condition. */

typedef struct tag_HB_SHARED_RESOURCE
{
   /* mutex is used to read or write safely */
   HB_CRITICAL_T Mutex;

   /* data that can be read or written */
   union {
      volatile long asLong;
      volatile void *asPointer;
   } content;

   /* Auxiliary ancillary data to signal metastatus of content */
   volatile unsigned int aux;

   /* Event that is risen when the condition changes */
   HB_COND_T Cond;

} HB_SHARED_RESOURCE;

#define HB_SHARED_INIT( pshr, data ) \
{ \
   HB_CRITICAL_INIT( pshr.Mutex );\
   HB_COND_INIT( pshr.Cond ); \
   pshr.aux = 0;\
   pshr.content.asLong = data;\
}

#define HB_SHARED_DESTROY( pshr ) \
{ \
   HB_CRITICAL_DESTROY( pshr.Mutex );\
   HB_COND_DESTROY( pshr.Cond ); \
}

/* It is said that pthread_cond_wait() should not be used with recursive
 * mutexes. But it seems to work on systems where recursive mutexes are
 * implemented. However, on Darwin < 6.x our recursive mutex implementation
 * blocks everything in this situation. So I try to use non-recursive
 * mutex here. -- Ph.K. */
#ifndef HB_NO_RECURSIVE_MUTEXES
#define HB_SHARED_LOCK( pshr )   HB_CRITICAL_LOCK( (pshr).Mutex )
#define HB_SHARED_UNLOCK( pshr ) HB_CRITICAL_UNLOCK( (pshr).Mutex )
#else
#define HB_SHARED_LOCK( pshr )   pthread_mutex_lock( &((pshr).Mutex.lock) )
#define HB_SHARED_UNLOCK( pshr ) pthread_mutex_unlock( &((pshr).Mutex.lock) )
#endif

#define HB_SHARED_SIGNAL( pshr )    HB_COND_SIGNAL( (pshr).Cond )
#define HB_SHARED_WAIT( pshr )      HB_COND_WAIT( (pshr).Cond, (pshr).Mutex )

/**********************************************************/
/* Context using management
   This macros are used for directing threads and blocking
   them on clean points.
*/

#define HB_STACK_LOCK \
{\
   if( ! HB_VM_STACK.bInUse && HB_VM_STACK.uiIdleInspect == 0) \
   {\
      HB_SHARED_LOCK( hb_runningStacks );\
      while ( hb_runningStacks.aux ) \
      {\
         HB_SHARED_WAIT( hb_runningStacks );\
      }\
      hb_runningStacks.content.asLong++;\
      HB_VM_STACK.bInUse = TRUE;\
      HB_SHARED_UNLOCK( hb_runningStacks );\
   }\
}

#define HB_STACK_UNLOCK \
{\
   if( HB_VM_STACK.bInUse && HB_VM_STACK.uiIdleInspect == 0 ) \
   {\
      HB_SHARED_LOCK( hb_runningStacks );\
      hb_runningStacks.content.asLong--;\
      HB_VM_STACK.bInUse = FALSE;\
      HB_SHARED_SIGNAL( hb_runningStacks );\
      HB_SHARED_UNLOCK( hb_runningStacks );\
   }\
}

/*********************************************************************/
/** In MT libs, every function accessing stack will record the HB_STACK
   (provided by hb_threadGetCurrentContext()) into a local Stack variable, and
   this variable will be accessed instead of HB_VM_STACK.
*/

#if ! defined( HB_THREAD_TLS_KEYWORD )
   /* Use standard hb_threadGetCurrentStack(), that can be macroed into a
      TLS function, like TlsGetValue() or pthread_getspecific, i.e. */

   #if defined( HB_THREAD_OPTIMIZE_STACK ) && defined( HB_STACK_MACROS )
      #define HB_VM_STACK (*_pStack_)
      #define HB_THREAD_STUB\
         HB_STACK *_pStack_ = hb_threadGetCurrentStack();
   #else
      #define HB_VM_STACK (* hb_threadGetCurrentStack() )
      #define HB_THREAD_STUB
   #endif

#else

   /* Use __thread keyword where available */
   #if __GNUC__ >= 3 && defined( HB_THREAD_TLS_BUG )
      extern HB_STACK hb_stackMT;
      #if defined( HB_THREAD_OPTIMIZE_STACK ) && defined( HB_STACK_MACROS )
         #define HB_THREAD_STUB\
            HB_STACK *_pStack_ = ( ((unsigned long)&hb_thread_stack) & 0xf0000000) == 0xf0000000 ? &hb_stackMT : hb_thread_stack;
         #define HB_VM_STACK (*_pStack_)
      #else
         #define HB_VM_STACK (*( ( ((unsigned long)&hb_thread_stack) & 0xf0000000)==0xf0000000 ? &hb_stackMT : hb_thread_stack ))
         #define HB_THREAD_STUB
      #endif
   #else
      #define HB_VM_STACK ( *hb_thread_stack )
      #define HB_THREAD_STUB
   #endif
   #undef  hb_threadGetCurrentStack
   #define hb_threadGetCurrentStack() ( hb_thread_stack )

   #if __GNUC__ >= 3 || defined( __BORLANDC__ )
      extern __thread HB_STACK *hb_thread_stack;
   #elif defined( _MSC_VER )
      extern HB_STACK __declspec(thread) *hb_thread_stack;
   #else
      #error "This platform does not support __thread keyword; undefine HB_THREAD_TLS_KEYWORD & recompile"
   #endif
#endif


/*********************************************************************/
/* More elegant guard of a small section of code                     */
#define HB_THREAD_GUARD( mutex, code )\
   {\
      HB_CRITICAL_LOCK( mutex );\
      { code; }\
      HB_CRITICAL_UNLOCK( mutex );\
   }

HB_EXTERN_BEGIN

extern HB_COND_T hb_threadStackCond;

/************************************************************
* List of mutexes that can be used to regulate concurency
*************************************************************/
/* Monitor for sync access to the global stack */
extern HB_CRITICAL_T hb_globalsMutex;
/* Monitor for sync access to the global stack */
extern HB_CRITICAL_T hb_staticsMutex;
/* Monitor for sync access to the global stack */
extern HB_CRITICAL_T hb_memvarsMutex;
/* Guard for threadunsafe malloc and free */
extern HB_CRITICAL_T hb_allocMutex;
/* Guard for console and output and free */
extern HB_CRITICAL_T hb_outputMutex;
/* Guard for memory allocated by the garbage collector */
extern HB_CRITICAL_T hb_garbageAllocMutex;
/* Guard for thread unsafe macro compilation */
extern HB_CRITICAL_T hb_macroMutex;
/* Mutex to control dynsym scanning and add */
extern HB_CRITICAL_T hb_dynsymMutex;

/* As dynsym require recursive locks, here I provide an hook that can
   be used in system that do not provide a native spinlock count to
   implement a user-defined spinlock scheme */
#define hb_dynsymLock()      HB_CRITICAL_LOCK( hb_dynsymMutex )
#define hb_dynsymUnlock()    HB_CRITICAL_UNLOCK( hb_dynsymMutex )

#ifdef HB_OS_OS2
extern HEV  hb_hevWakeUpAll; /* Semaphore posted to wake up all threads waiting somewhere on an INDEFINITE wait */
#endif

/* count of running stacks */
extern HB_SHARED_RESOURCE hb_runningStacks;

/* regulates idle aware threads to be fenced or free */
extern BOOL hb_bIdleFence;

/***********************************************************************/
/* Function and globals definitions */
extern HB_EXPORT BOOL hb_vm_bQuitRequest;

extern PHB_STACK last_stack;
extern PHB_STACK hb_ht_stack;
extern PHB_MUTEX_STRUCT hb_ht_mutex;
extern HB_THREAD_T hb_main_thread_id;
extern BOOL hb_threadIsInspector;

extern PHB_STACK hb_threadCreateStack( HB_THREAD_T th_id );
extern void hb_threadSetupStack( PHB_STACK tc, HB_THREAD_T th );
extern PHB_STACK hb_threadLinkStack( PHB_STACK tc );
extern PHB_STACK hb_threadUnlinkStack( PHB_STACK pStack );
extern void hb_threadDestroyStack( PHB_STACK pStack );
extern PHB_STACK hb_threadGetStack( HB_THREAD_T th_id );
extern void hb_threadInit( void );
extern void hb_threadExit( void );
extern int hb_threadCountStacks( void );
extern void hb_threadFillStack( PHB_STACK pStack, PHB_ITEM pArgs );
extern HB_EXPORT void hb_threadWaitAll( void );
extern HB_EXPORT void hb_threadKillAll( void );
extern void hb_threadSleep( int millisec, BOOL bIdleWaitNoCpu );
extern void hb_mutexForceUnlock( void *);
extern void hb_rawMutexForceUnlock( void *);
extern PHB_MUTEX_STRUCT hb_threadLinkMutex( PHB_MUTEX_STRUCT mx );
extern PHB_MUTEX_STRUCT hb_threadUnlinkMutex( PHB_MUTEX_STRUCT mx );
extern void hb_threadTerminator( void *pData );
extern void hb_threadWaitForIdle( void );
extern void hb_threadIdleEnd( void );
extern PHB_ITEM hb_threadMutexCreate( PHB_ITEM pItem );
extern void hb_threadMutexUnlock( PHB_ITEM pItem, BOOL bError );
extern BOOL hb_threadMutexLock( PHB_ITEM pItem, BOOL bError );
extern BOOL hb_threadMutexTimeOutLock( PHB_ITEM pItem, int iTimeOut, BOOL bError );
extern BOOL hb_threadMutexTryLock( PHB_ITEM pItem, BOOL bError );

/* Functions used to support SYNC methods */
extern void hb_clsPutSyncID( USHORT uiClass );
extern ULONG hb_clsDelSyncID( USHORT uiClass );
extern void hb_clsUnmutexSync( void );
extern void hb_clsRemutexSync( void );

/* External functions used by thread as helper */
extern void hb_memvarsInit( PHB_STACK );
extern void hb_memvarsRelease( PHB_STACK );
extern void hb_memvarValueDecRefMT( HB_HANDLE hValue, PHB_STACK pStack );
extern HB_EXPORT void hb_itemClearMT( PHB_ITEM pItem, PHB_STACK pStack );

/* Used by dynsym thread specific system */
void hb_threadSetHMemvar( PHB_DYNS pDyn, HB_HANDLE hv );

/* A thread willingly terminating itself */
void hb_threadCancelInternal( void );

/* Win 32 specific functions */
#if defined(HB_OS_WIN)
   BOOL hb_threadCondInit( HB_WINCOND_T *cond );
   void hb_threadCondDestroy( HB_WINCOND_T *cond );
   void hb_threadCondSignal( HB_WINCOND_T *cond );
   BOOL hb_threadCondWait( HB_WINCOND_T *cond, CRITICAL_SECTION *mutex , DWORD dwTimeout );

#elif defined(HB_OS_OS2)
   BOOL hb_threadCondInit( HB_COND_T *cond );
   void hb_threadCondDestroy( HB_COND_T *cond );
   void hb_threadCondSignal( HB_COND_T *cond );
   BOOL hb_threadCondWait( HB_COND_T *cond, CRITICAL_SECTION *mutex , DWORD dwTimeout );
   void hb_threadMtxPoll( HB_CRITICAL_T mtx );
#endif

HB_EXTERN_END

/******************************************************/
/* Definitions when threading is turned off */

#else

   #define HB_THREAD_T HB_PTRDIFF
   #define HB_CRITICAL_T               ULONG
   #define HB_ATOMIC_INC( x )    ( ++(x) )
   #define HB_ATOMIC_DEC( x )    ( --(x) )

   #define HB_CRITICAL_LOCK( x )
   #define HB_CRITICAL_TRYLOCK( x )
   #define HB_CRITICAL_UNLOCK( x )
   #define HB_THREAD_GUARD( mutex, code ) { code; }

   #define HB_TEST_CANCEL
   #define HB_STACK_LOCK
   #define HB_STACK_UNLOCK
   #define HB_CLEANUP_PUSH( x, y )
   #define HB_CLEANUP_POP
   #define HB_CLEANUP_POP_EXEC

   #define HB_THREAD_STUB

   #ifndef HB_VM_STACK
      #define HB_VM_STACK hb_stackST
      /* This version eliminates the problem with DLL linking */
      /* #define HB_VM_STACK (*hb_stack_ptr) */
   #endif


   #define HB_ENABLE_ASYN_CANC
   #define HB_DISABLE_ASYN_CANC
   #define HB_TEST_CANCEL_ENABLE_ASYN

   #define HB_SHARED_LOCK
   #define HB_SHARED_UNLOCK
   #define HB_SHARED_SIGNAL
   #define HB_SHARED_WAIT

   #define hb_dynsymLock()
   #define hb_dynsymUnlock()

#endif

HB_EXTERN_BEGIN
typedef enum
{
   UNKNOWN,
   S_ARC4MTX,
   S_SOCKMTX,
   S_HSXMTX,
   S_FILEMTX,
   S_FILENETMTX,
   S_TRACEMTX,
   S_SERVICEMUTEX,
   HB_MACROMUTEX,
   HB_OUTPUTMUTEX
} __MUTEX;
extern void hb_threadLockInit( int iMtx );
extern void hb_threadLock( int iMtx );
extern void hb_threadUnLock( int iMtx );
extern void hb_threadLockDestroy( int iMtx );
HB_EXTERN_END

#ifdef HB_API_MACROS
#  define HB_THREAD_STUB_API        HB_THREAD_STUB
#else
#  define HB_THREAD_STUB_API
#endif

#ifdef HB_STACK_MACROS
#  define HB_THREAD_STUB_STACK      HB_THREAD_STUB
#else
#  define HB_THREAD_STUB_STACK
#endif

#if defined( HB_STACK_MACROS ) || defined( HB_API_MACROS )
#  define HB_THREAD_STUB_ANY        HB_THREAD_STUB
#else
#  define HB_THREAD_STUB_ANY
#endif

#endif
