#ifndef __INTERNAL_POOL_H
#define __INTERNAL_POOL_H

PVOID STDCALL ExAllocateNonPagedPoolWithTag (POOL_TYPE	type, 
					     ULONG		size, 
					     ULONG		Tag,
					     PVOID		Caller);

PVOID STDCALL ExAllocatePagedPoolWithTag (POOL_TYPE	Type,
					  ULONG		size,
					  ULONG		Tag);
VOID STDCALL ExFreeNonPagedPool (PVOID block);

VOID STDCALL
ExFreePagedPool(IN PVOID Block);
VOID MmInitializePagedPool(VOID);

extern PVOID MmPagedPoolBase;
extern ULONG MmPagedPoolSize;

#define MM_PAGED_POOL_SIZE	(100*1024*1024)
#define MM_NONPAGED_POOL_SIZE	(100*1024*1024)

/*
 * Paged and non-paged pools are 8-byte aligned
 */
#define MM_POOL_ALIGNMENT	8

/*
 * Maximum size of the kmalloc area (this is totally arbitary)
 */
#define MM_KERNEL_MAP_SIZE	(16*1024*1024)

/*
 * FIXME - different architectures have different cache line sizes...
 */
#define MM_CACHE_LINE_SIZE  32

#define MM_ROUND_UP(x,s)    ((PVOID)(((ULONG)(x)+(s)-1) & ~((s)-1)))
#define MM_ROUND_DOWN(x,s)  ((PVOID)(((ULONG)(x)) & ~((s)-1)))

#endif /* __INTERNAL_POOL_H */
