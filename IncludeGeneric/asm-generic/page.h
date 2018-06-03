/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_GENERIC_PAGE_H
#define __ASM_GENERIC_PAGE_H
/*
 * Generic page.h implementation, for NOMMU architectures.
 * This provides the dummy definitions for the memory management.
 */

#ifdef CONFIG_MMU
#error need to prove a real asm/page.h
#endif


/* PAGE_SHIFT determines the page size */

#define PAGE_SHIFT	12
#ifdef __ASSEMBLY__
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#else
#define PAGE_SIZE	(1UL << PAGE_SHIFT)
#endif
#define PAGE_MASK	(~(PAGE_SIZE-1))

#ifndef __ASSEMBLY__

#define clear_page(page)	memset((page), 0, PAGE_SIZE)
#define copy_page(to,from)	memcpy((to), (from), PAGE_SIZE)

#endif /* !__ASSEMBLY__ */

#ifndef __ASSEMBLY__

#define __va(x) ((void *)((unsigned long) (x)))
#define __pa(x) ((unsigned long) (x))

#define virt_to_pfn(kaddr)	(__pa(kaddr) >> PAGE_SHIFT)
#define pfn_to_virt(pfn)	__va((pfn) << PAGE_SHIFT)

#define virt_to_page(addr)	pfn_to_page(virt_to_pfn(addr))
#define page_to_virt(page)	pfn_to_virt(page_to_pfn(page))

#ifndef page_to_phys
#define page_to_phys(page)      ((dma_addr_t)page_to_pfn(page) << PAGE_SHIFT)
#endif

#endif /* __ASSEMBLY__ */

#include <asm-generic/getorder.h>

#endif /* __ASM_GENERIC_PAGE_H */
