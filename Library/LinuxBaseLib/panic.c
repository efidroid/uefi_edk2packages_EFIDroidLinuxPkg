/*
 *  linux/kernel/panic.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This function is used through-out the kernel (including mm and fs)
 * to indicate a major problem.
 */

#include <LinuxBase.h>
#include <linux/bug.h>

#include <Library/DebugLib.h>

int panic_on_warn __read_mostly;

/**
 *	panic - halt the system
 *	@fmt: The text string to print
 *
 *	Display a message, then perform cleanups.
 *
 *	This function never returns.
 */
void panic(const char *fmt, ...)
{
	static char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	pr_emerg("Kernel panic - not syncing: %s\n", buf);
	ASSERT(FALSE);
	for(;;);
}

EXPORT_SYMBOL(panic);

void print_oops_end_marker(void)
{
	pr_warn("---[ end oops ]---\n");
}

struct warn_args {
	const char *fmt;
	va_list args;
};

void __warn(const char *file, int line, void *caller, struct warn_args *args)
{
	if (args)
		pr_warn(CUT_HERE);

	if (file)
		pr_warn("WARNING: at %s:%d %pS\n", file, line, caller);
	else
		pr_warn("WARNING: at %pS\n", caller);

	if (args)
		vprintk(args->fmt, args->args);

	if (panic_on_warn) {
		/*
		 * This thread may hit another WARN() in the panic path.
		 * Resetting this prevents additional WARN() from panicking the
		 * system on this thread.  Other threads are blocked by the
		 * panic_mutex in panic().
		 */
		panic_on_warn = 0;
		panic("panic_on_warn set ...\n");
	}

	print_oops_end_marker();
}

#ifdef WANT_WARN_ON_SLOWPATH
void warn_slowpath_fmt(const char *file, int line, const char *fmt, ...)
{
	struct warn_args args;

	args.fmt = fmt;
	va_start(args.args, fmt);
	__warn(file, line, __builtin_return_address(0), &args);
	va_end(args.args);
}
EXPORT_SYMBOL(warn_slowpath_fmt);

void warn_slowpath_fmt_taint(const char *file, int line,
			     unsigned taint, const char *fmt, ...)
{
	struct warn_args args;

	args.fmt = fmt;
	va_start(args.args, fmt);
	__warn(file, line, __builtin_return_address(0), &args);
	va_end(args.args);
}
EXPORT_SYMBOL(warn_slowpath_fmt_taint);

void warn_slowpath_null(const char *file, int line)
{
	pr_warn(CUT_HERE);
	__warn(file, line, __builtin_return_address(0), NULL);
}
EXPORT_SYMBOL(warn_slowpath_null);
#else
void __warn_printk(const char *fmt, ...)
{
	va_list args;

	pr_warn(CUT_HERE);

	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
}
EXPORT_SYMBOL(__warn_printk);
#endif
