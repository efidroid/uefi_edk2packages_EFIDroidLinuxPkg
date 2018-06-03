/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_UEFI_STRING_H
#define _ASM_UEFI_STRING_H

#define __HAVE_ARCH_MEMCPY
extern void * memcpy(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMMOVE
extern void * memmove(void *, const void *, __kernel_size_t);

#define __HAVE_ARCH_MEMSET
extern void * memset(void *, int, __kernel_size_t);

#endif /* _ASM_UEFI_STRING_H */
