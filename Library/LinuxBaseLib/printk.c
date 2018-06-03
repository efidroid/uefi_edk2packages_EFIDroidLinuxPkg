#include <LinuxBase.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/printk.h>
#include <linux/kernel.h>

#include <Library/DebugLib.h>

#define PREFIX_MAX		32
#define LOG_LINE_MAX		(1024 - PREFIX_MAX)

enum log_flags {
	LOG_NOCONS	= 1,	/* already flushed, do not print to console */
	LOG_NEWLINE	= 2,	/* text ended with a newline */
	LOG_PREFIX	= 4,	/* text started with a prefix */
	LOG_CONT	= 8,	/* text is a fragment of a continuation line */
};

int console_printk[4] = {
	CONSOLE_LOGLEVEL_DEFAULT,	/* console_loglevel */
	MESSAGE_LOGLEVEL_DEFAULT,	/* default_message_loglevel */
	CONSOLE_LOGLEVEL_MIN,		/* minimum_console_loglevel */
	CONSOLE_LOGLEVEL_DEFAULT,	/* default_console_loglevel */
};

#ifdef CONFIG_PRINTK

static size_t log_output(int facility, int level, enum log_flags lflags, const char *dict, size_t dictlen, char *text, size_t text_len)
{
	/* Skip empty continuation lines that couldn't be added - they just flush */
	if (!text_len && (lflags & LOG_CONT))
		return 0;

	bool need_newline = !!(lflags&LOG_NEWLINE);

	/* Print */
	DEBUG ((DEBUG_ERROR, "<%u>%a%a", level, text, (need_newline?"\n":"")));
	return text_len;
}

asmlinkage int vprintk_emit(int facility, int level,
			    const char *dict, size_t dictlen,
			    const char *fmt, va_list args)
{
	static char textbuf[LOG_LINE_MAX];
	char *text = textbuf;
	size_t text_len;
	enum log_flags lflags = 0;
	int printed_len;

	/*
	 * The printf needs to come first; we need the syslog
	 * prefix which might be passed-in as a parameter.
	 */
	text_len = vscnprintf(text, sizeof(textbuf), fmt, args);

	/* mark and strip a trailing newline */
	if (text_len && text[text_len-1] == '\n') {
		text[text_len-1] = '\0';
		text_len--;
		lflags |= LOG_NEWLINE;
	}

	/* strip kernel syslog prefix and extract log level or control flags */
	if (facility == 0) {
		int kern_level;

		while ((kern_level = printk_get_level(text)) != 0) {
			switch (kern_level) {
			case '0' ... '7':
				if (level == LOGLEVEL_DEFAULT)
					level = kern_level - '0';
				/* fallthrough */
			case 'd':	/* KERN_DEFAULT */
				lflags |= LOG_PREFIX;
				break;
			case 'c':	/* KERN_CONT */
				lflags |= LOG_CONT;
			}

			text_len -= 2;
			text += 2;
		}
	}

	if (level == LOGLEVEL_DEFAULT)
		level = default_message_loglevel;

	if (dict)
		lflags |= LOG_PREFIX|LOG_NEWLINE;

	printed_len = log_output(facility, level, lflags, dict, dictlen, text, text_len);

	return printed_len;
}
EXPORT_SYMBOL(vprintk_emit);

asmlinkage int printk_emit(int facility, int level,
			   const char *dict, size_t dictlen,
			   const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk_emit(facility, level, dict, dictlen, fmt, args);
	va_end(args);

	return r;
}
EXPORT_SYMBOL(printk_emit);

static int vprintk_default(const char *fmt, va_list args)
{
	return vprintk_emit(0, LOGLEVEL_DEFAULT, NULL, 0, fmt, args);
}

asmlinkage int vprintk(const char *fmt, va_list args)
{
	return vprintk_default(fmt, args);
}
EXPORT_SYMBOL(vprintk);

asmlinkage __visible int printk(const char *fmt, ...)
{
	va_list args;
	int r;

	va_start(args, fmt);
	r = vprintk_default(fmt, args);
	va_end(args);

	return r;
}
EXPORT_SYMBOL(printk);

#endif  /* CONFIG_PRINTK */
