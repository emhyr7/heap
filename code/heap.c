#define __MODE_TESTING__ 1

#if defined(__cplusplus)
	#define _DEMANGLED_ extern "C"
#else
	#define _DEMANGLED_
#endif

#define COUNT_OF_(x) (sizeof(x) / sizeof((x)[0]))

#if defined(_MSC_VER)
	#define _CDECL_ __cdecl
	#define _STDCALL_ __stdcall

	#if defined(_DLL)
		#define _IMPORTED_ __declspec(dllimport) _DEMANGLED_
	#else
		#define _IMPORTED_ _DEMANGLED_
	#endif

	#define _WCHAR_ __wchar_t
	#define _LONG_ long long

	#define _INLINE_ static __forceinline

#endif

#define BYTE_WIDTH 8
#define BITS_COUNT_OF(x) (sizeof(x) * BYTE_WIDTH)

typedef unsigned _LONG_ BIT64;

typedef char BYTE;

typedef unsigned int    UWORD;
typedef unsigned _LONG_ ULONG;

typedef UWORD WORD;

typedef ULONG UINTPTR;
typedef ULONG SIZE;

typedef char CHAR;
typedef int INT;

typedef unsigned int UINT;

typedef unsigned char UBYTE;
typedef unsigned int  UWORD;

typedef int BOOLEAN;

typedef void VOID;

typedef signed _LONG_ RLONG; // negative values implies failure

typedef struct FILE FILE;

_IMPORTED_ INT _CDECL_ fprintf(FILE *, const char *, ...);

_IMPORTED_ VOID *_CDECL_ memset(VOID *, INT, SIZE);

#if defined(_MSC_VER)
	#include <intrin.h>

	_IMPORTED_ FILE *_CDECL_ __acrt_iob_func(unsigned);

	#define stdin  __acrt_iob_func(0)
	#define stdout __acrt_iob_func(1)
	#define stderr __acrt_iob_func(2)

	_IMPORTED_ void _CDECL_ _wassert(const _WCHAR_ *, const _WCHAR_ *, unsigned);

	#define _WIDEN__(s) L ## s
	#define _WIDEN_(s) _WIDEN__(s)

	#define _STRINGEN__(s) #s
	#define _STRINGEN_(s) _STRINGEN__(s)

	#define ASSERT_(e) (!!(e) || (_wassert((_WCHAR_ *)_WIDEN_(_STRINGEN_(e)), (_WCHAR_ *)_WIDEN_(__FILE__), (unsigned)(__LINE__)), 0))

	_IMPORTED_ VOID *_STDCALL_ VirtualAlloc(VOID *, SIZE, UWORD, UWORD);
	_IMPORTED_ BOOLEAN _STDCALL_ VirtualFree(VOID *, SIZE, UWORD);

	_INLINE_ VOID *reserve_memory(SIZE size)
	{
		VOID *memory = VirtualAlloc(0, size, 0x00002000 /* MEM_RESERVE */, 0x04 /* PAGE_READWRITE */);
		ASSERT_(memory);
		return memory;
	}

	_INLINE_ VOID commit_memory(VOID *memory, SIZE size)
	{
		ASSERT_(VirtualAlloc(memory, size, 0x00001000 /* MEM_COMMIT */, 0x04 /* PAGE_READWRITE */));
	}

	_INLINE_ VOID release_memory(VOID *memory)
	{
		ASSERT_(VirtualFree(memory, 0, 0x00008000 /* MEM_RELEASE */));
	}

	_INLINE_ RLONG ffs(BIT64 m)
	{
		unsigned long i;
		unsigned char r = _BitScanForward64(&i, m);
		return r ? i : -1;
	}
#endif

enum { SYSTEM_PAGE_SIZE = 4096 };

typedef struct
{
	ULONG reservation_size;
	BYTE *base_address;
	ULONG commission_size;
	BIT64 page_flags[];
} HEAP;

#define FIELD_OF_(type, member) (((type *)0)->member)

_INLINE_ SIZE base2_remainder(SIZE a, SIZE b) { return b ? a & (b - 1) : 0; }
_INLINE_ BOOLEAN is_aligned_by(SIZE a, SIZE b) { return base2_remainder(a, b) == 0; }
_INLINE_ SIZE align_forwards(SIZE a, SIZE b) { UINTPTR r = base2_remainder(a, b); return a + (r ? b - r : 0); }

HEAP *create_heap(ULONG pages_count)
{
	const ULONG sizeof_page_flag = sizeof(*FIELD_OF_(HEAP, page_flags));

	ULONG page_flags_count = pages_count / (sizeof_page_flag * 8);

	ULONG header_size = align_forwards(sizeof(HEAP) + page_flags_count * sizeof_page_flag, SYSTEM_PAGE_SIZE);
	ULONG reservation_size = pages_count * SYSTEM_PAGE_SIZE;
	ULONG total_reservation_size = header_size + reservation_size;
	BYTE *memory = reserve_memory(total_reservation_size);

	HEAP *heap = (HEAP *)memory;
	heap->reservation_size = reservation_size;
	heap->base_address = memory + header_size;
	heap->commission_size = 0;
	memset(heap->page_flags, ~0, page_flags_count * sizeof_page_flag);
	return heap;
}

static BIT64 *find_bit_range(SIZE n, BIT64 *p, BIT64 *q)
{
	ASSERT_(p < q);
	const INT d = sizeof(*p) * BYTE_WIDTH;
	for(BIT64 b = *p;; b = *++p)
	{
		if(p == q) break;
		RLONG i = ffs(b);
		if(i < 0) continue;
		// NOTE(Emhyr): we can simplify this by having `ffs` return `BITS_COUNT_OF(*p)` instead of -1
		RLONG r = ffs(~(b | (i ? ((BIT64)1 << i) - 1 : 0)));
		SIZE j = i + (r < 0 ? d - i : r);
		if(j - i < n)
		{
			if(r >= 0) continue;
			do
			{
				if(++p == q) goto done;
				r = ffs(~(b = *p));
				j += r < 0 ? d : r;
				if(j - i >= n) goto done;
			} while(r < 0);
			continue;
		}
		else goto done;
	}
done:
	return p;
}

#if defined(__MODE_TESTING__)
	static VOID test__find_bit_range(VOID)
	{
		BIT64 bits[4] = {0x1, 0x3, 0x7, 0xf};
		BIT64 *q = bits + COUNT_OF_(bits);
		ASSERT_(bits + 0 == find_bit_range(1, bits, q));
		ASSERT_(bits + 1 == find_bit_range(2, bits, q));
		ASSERT_(bits + 2 == find_bit_range(3, bits, q));
		ASSERT_(bits + 3 == find_bit_range(4, bits, q));
		ASSERT_(bits + 4 == find_bit_range(5, bits, q));
	}
#endif

typedef struct
{
	ULONG size;
	BYTE *memory;
} ALLOCATION;

INT main(INT argc, CHAR *argv[])
{
	(void)argc, (void)argv;

	HEAP *heap = create_heap(128);
	return 0;
}
