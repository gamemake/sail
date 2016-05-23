#ifndef	__ZION_DEFINES__
#define	__ZION_DEFINES__

typedef unsigned char			_U8;
typedef unsigned short			_U16;
typedef unsigned int			_U32;
typedef unsigned long long		_U64;
typedef char					_S8;
typedef short					_S16;
typedef long					_S32;
typedef long long				_S64;
typedef float					_F32;
typedef double					_F64;
typedef char*					_STR;

#include <assert.h>
#include <stdlib.h>
extern void zion_set_fatal(bool msgbox);
extern void zion_fatal(const char* fmt, ...);

#define ZION_ASSERT(x)			assert(x)

#ifdef	_DEBUG
#define ZION_VERIFY(x)			assert(x)
#else
#define	ZION_VERIFY(expr)		(void)(expr)
#endif

#define ZION_FATAL	zion_fatal

#define ZION_OFFSETOF(type, member) ((size_t)(&((type*)NULL)->member))

extern void* zion_malloc(size_t size);
extern void zion_free(void* mem);

#include <stdio.h>

#ifdef _WIN32
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#include <malloc.h>
	#define ZION_ALLOC(x)			zion_malloc(x)
	#define ZION_FREE(x)			zion_free(x)
	#ifdef _DEBUG
	#define ZION_NEW				new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#else
	#define ZION_NEW				new
	#endif
	#define ZION_DELETE				delete
	#define ZION_ALLOCA(x)			_alloca(x)

	#define ZION_SLEEP(x)			Sleep(x)

	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>

	#define A_MUTEX					CRITICAL_SECTION
	#define A_MUTEX_INIT			InitializeCriticalSection
	#define A_MUTEX_DESTROY			DeleteCriticalSection
	#define A_MUTEX_TRYLOCK			TryEnterCriticalSection
	#define A_MUTEX_LOCK			EnterCriticalSection
	#define A_MUTEX_UNLOCK			LeaveCriticalSection

	#define A_ATOM_INC(v)			InterlockedIncrement((LONG*)v)
	#define A_ATOM_DEC(v)			InterlockedDecrement((LONG*)v)
	#define A_ATOM_XCHG(v, e, c)	InterlockedCompareExchange((LONG*)v, e, c)

	#define A_SLIST_HEADER			SLIST_HEADER
	#define A_SLIST_ENTRY			SLIST_ENTRY
	#define A_SLIST_INIT			InitializeSListHead
	#define A_SLIST_PUSH			InterlockedPushEntrySList
	#define A_SLIST_POP				InterlockedPopEntrySList
#else
	#include <stdlib.h>
	#include <unistd.h>

	#define ZION_ALLOC(x)			zion_malloc(x)
	#define ZION_FREE(x)			zion_free(x)
	#ifdef _DEBUG
	#define ZION_NEW				new
	#else
	#define ZION_NEW				new
	#endif
	#define ZION_DELETE				delete
	#define ZION_ALLOCA(x)			_alloca(x)

	#define ZION_SLEEP(x)			usleep(x*1000)

	static inline _U32 atom_unix_inc(_U32 volatile* mem)
	{
		return __sync_fetch_and_add(mem, 1) + 1;
	}

	static inline _U32 atom_unix_dec(_U32 volatile* mem)
	{
		return __sync_fetch_and_sub(mem, 1) - 1;
	}

	static inline _U32 atom_unix_cas(_U32 volatile* mem, _U32 with, _U32 cmp)
	{
		return __sync_val_compare_and_swap(mem, cmp, with);
	}

	typedef struct ATOM_SLIST_ENTRY
	{
		struct ATOM_SLIST_ENTRY *Next;
	} ATOM_SLIST_ENTRY;
	typedef union ATOM_SLIST_HEADER
	{
#if _LP64
		volatile __uint128_t dword;
#else
		volatile uint64_t dword;
#endif
		struct {
			ATOM_SLIST_ENTRY First;
			long Count;
		};
	} ATOM_SLIST_HEADER;

	static inline void atom_slist_init(ATOM_SLIST_HEADER* header)
	{
		header->dword = 0;
		//header->First.Next = NULL;
		//header->Count = 0;
	}

	static inline void atom_slist_push(ATOM_SLIST_HEADER* head, ATOM_SLIST_ENTRY* entry)
	{
		ATOM_SLIST_HEADER nHdr,oHdr;
		nHdr.First.Next = entry;
		do
		{
			oHdr.dword = head->dword;
			nHdr.Count = oHdr.Count + 1;
			entry->Next = oHdr.First.Next;
		} while (!__sync_bool_compare_and_swap(&head->dword,oHdr.dword,nHdr.dword));
	}

	static inline ATOM_SLIST_ENTRY* atom_slist_pop(ATOM_SLIST_HEADER* head)
	{
		ATOM_SLIST_HEADER nHdr,oHdr;
		ATOM_SLIST_ENTRY *pe;
		do
		{
			oHdr.dword = head->dword;
			if ((pe=oHdr.First.Next)==NULL) return NULL;
			nHdr.Count = oHdr.Count + 1;
			nHdr.First.Next = pe->Next;
		} while (!__sync_bool_compare_and_swap(&head->dword, oHdr.dword, nHdr.dword));
		return pe;
	}

	static inline ATOM_SLIST_ENTRY* atom_slist_flush(ATOM_SLIST_HEADER* head)
	{
		ATOM_SLIST_HEADER nHdr,oHdr;
		ATOM_SLIST_ENTRY *pe;
		do
		{
			oHdr.dword = head->dword;
			if ((pe=oHdr.First.Next)==NULL) return NULL;
			nHdr.Count = oHdr.Count + 1;
			nHdr.First.Next = NULL;
		} while (!__sync_bool_compare_and_swap(&head->dword, oHdr.dword, nHdr.dword));
		return pe;
	}

	#include <pthread.h>

	#define A_MUTEX					pthread_mutex_t
	#define A_MUTEX_INIT(x)			pthread_mutex_init(x, NULL)
	#define A_MUTEX_DESTROY			pthread_mutex_destroy
	#define A_MUTEX_TRYLOCK			pthread_mutex_trylock
	#define A_MUTEX_LOCK			pthread_mutex_lock
	#define A_MUTEX_UNLOCK			pthread_mutex_unlock

	#define A_ATOM_INC(x)			atom_unix_inc(x)
	#define A_ATOM_DEC(x)			atom_unxi_dec(x)
	#define A_ATOM_XCHG(v, e, c)	atom_unix_cas(v, e, c)

	#define A_SLIST_HEADER			ATOM_SLIST_HEADER
	#define A_SLIST_ENTRY			ATOM_SLIST_ENTRY
	#define A_SLIST_INIT			atom_slist_init
	#define A_SLIST_PUSH			atom_slist_push
	#define A_SLIST_POP				atom_slist_pop

	unsigned long GetTickCount();
	#define stricmp					strcasecmp
#endif

#endif
