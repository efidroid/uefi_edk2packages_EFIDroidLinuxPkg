/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_INIT_H
#define _LINUX_INIT_H

#include <linux/compiler.h>
#include <linux/types.h>

/* Built-in __init functions needn't be compiled with retpoline */
#if defined(__noretpoline) && !defined(MODULE)
#define __noinitretpoline __noretpoline
#else
#define __noinitretpoline
#endif

/* These macros are used to mark some functions or 
 * initialized data (doesn't apply to uninitialized data)
 * as `initialization' functions. The kernel can take this
 * as hint that the function is used only during the initialization
 * phase and free up used memory resources after
 *
 * Usage:
 * For functions:
 * 
 * You should add __init immediately before the function name, like:
 *
 * static void __init initme(int x, int y)
 * {
 *    extern int z; z = x * y;
 * }
 *
 * If the function has a prototype somewhere, you can also add
 * __init between closing brace of the prototype and semicolon:
 *
 * extern int initialize_foobar_device(int, int, int) __init;
 *
 * For initialized data:
 * You should insert __initdata or __initconst between the variable name
 * and equal sign followed by value, e.g.:
 *
 * static int init_variable __initdata = 0;
 * static const char linux_logo[] __initconst = { 0x32, 0x36, ... };
 *
 * Don't forget to initialize data not at file scope, i.e. within a function,
 * as gcc otherwise puts the data into the bss section and not into the init
 * section.
 */

/* These are for everybody (although not all archs will actually
   discard it in modules) */
#define __init		__section(.text.init) __cold  __latent_entropy __noinitretpoline
#define __initdata	__section(.data.init)
#define __initconst	__section(.rodata.init)
#define __exitdata	__section(.data.exit)
#define __exit_call	__used __section(.data.exitcall)

/*
 * modpost check for section mismatches during the kernel build.
 * A section mismatch happens when there are references from a
 * code or data section to an init section (both code or data).
 * The init sections are (for most archs) discarded by the kernel
 * when early init has completed so all such references are potential bugs.
 * For exit sections the same issue exists.
 *
 * The following markers are used for the cases where the reference to
 * the *init / *exit section (code or data) is valid and will teach
 * modpost not to issue a warning.  Intended semantics is that a code or
 * data tagged __ref* can reference code or data from init section without
 * producing a warning (of course, no warning does not mean code is
 * correct, so optimally document why the __ref is needed and why it's OK).
 *
 * The markers follow same syntax rules as __init / __initdata.
 */
#define __ref            __section(.text.ref) noinline
#define __refdata        __section(.data.ref)
#define __refconst       __section(.rodata.ref)

#ifdef MODULE
#define __exitused
#else
#define __exitused  __used
#endif

#define __exit          __section(.text.exit) __exitused __cold notrace

/* Used for MEMORY_HOTPLUG */
#define __meminit        __section(.text.meminit) __cold notrace \
						  __latent_entropy
#define __meminitdata    __section(.data.meminit)
#define __meminitconst   __section(.rodata.meminit)
#define __memexit        __section(.text.memexit) __exitused __cold notrace
#define __memexitdata    __section(.data.memexit)
#define __memexitconst   __section(.rodata.memexit)

/* For assembly routines */
#define __HEAD		.section	".text.head","ax"
#define __INIT		.section	".text.init","ax"
#define __FINIT		.previous

#define __INITDATA	.section	".data.init","aw",%progbits
#define __INITRODATA	.section	".rodata.init","a",%progbits
#define __FINITDATA	.previous

#define __MEMINIT        .section	".text.meminit", "ax"
#define __MEMINITDATA    .section	".data.meminit", "aw"
#define __MEMINITRODATA  .section	".rodata.meminit", "a"

/* silence warnings when references are OK */
#define __REF            .section       ".text.ref", "ax"
#define __REFDATA        .section       ".data.ref", "aw"
#define __REFCONST       .section       ".rodata.ref", "a"

#ifndef __ASSEMBLY__

extern bool initcall_debug;

#endif

/* Data marked not to be saved by software suspend */
#define __nosavedata __section(.data..nosave)

#ifdef MODULE
#define __exit_p(x) x
#else
#define __exit_p(x) NULL
#endif

#endif /* _LINUX_INIT_H */
