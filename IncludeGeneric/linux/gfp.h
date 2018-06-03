/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_GFP_H
#define __LINUX_GFP_H

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/linkage.h>

/*
 * In case of changes, please don't forget to update
 * include/trace/events/mmflags.h and tools/perf/builtin-kmem.c
 */

/* Plain integer GFP bitmasks. Do not use this directly. */
#define ___GFP_DMA		0x01u
#define ___GFP_DMA32		0x04u
#define ___GFP_ZERO		0x8000u
#define ___GFP_ATOMIC		0x80000u
/* If the above are modified, __GFP_BITS_SHIFT may need updating */

/*
 * Physical address zone modifiers (see linux/mmzone.h - low four bits)
 *
 * Do not put any conditional on these. If necessary modify the definitions
 * without the underscores and use them consistently. The definitions here may
 * be used in bit comparisons.
 */
#define __GFP_DMA	((__force gfp_t)___GFP_DMA)
#define __GFP_DMA32	((__force gfp_t)___GFP_DMA32)

/*
 * Watermark modifiers -- controls access to emergency reserves
 *
 * __GFP_ATOMIC indicates that the caller cannot reclaim or sleep and is
 *   high priority. Users are typically interrupt handlers. This may be
 *   used in conjunction with __GFP_HIGH
 */
#define __GFP_ATOMIC	((__force gfp_t)___GFP_ATOMIC)

/*
 * Action modifiers
 *
 * __GFP_ZERO returns a zeroed page on success.
 */
#define __GFP_ZERO	((__force gfp_t)___GFP_ZERO)

/* Room for N __GFP_FOO bits */
#define __GFP_BITS_SHIFT (25)
#define __GFP_BITS_MASK ((__force gfp_t)((1 << __GFP_BITS_SHIFT) - 1))

/*
 * Useful GFP flag combinations that are commonly used. It is recommended
 * that subsystems start with one of these combinations and then set/clear
 * __GFP_FOO flags as necessary.
 *
 * GFP_ATOMIC users can not sleep and need the allocation to succeed. A lower
 *   watermark is applied to allow access to "atomic reserves"
 *
 * GFP_KERNEL is typical for kernel-internal allocations. The caller requires
 *   ZONE_NORMAL or a lower zone for direct access but can direct reclaim.
 *
 * GFP_DMA exists for historical reasons and should be avoided where possible.
 *   The flags indicates that the caller requires that the lowest zone be
 *   used (ZONE_DMA or 16M on x86-64). Ideally, this would be removed but
 *   it would require careful auditing as some users really require it and
 *   others use the flag to avoid lowmem reserves in ZONE_DMA and treat the
 *   lowest zone as a type of emergency reserve.
 *
 * GFP_DMA32 is similar to GFP_DMA except that the caller requires a 32-bit
 *   address.
 */
#define GFP_ATOMIC	(0)
#define GFP_KERNEL	(0)
#define GFP_DMA		__GFP_DMA
#define GFP_DMA32	__GFP_DMA32

#endif /* __LINUX_GFP_H */
