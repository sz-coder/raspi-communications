#include <stdio.h> // vfprintf, fprintf, stderr
#include <stdarg.h> // va_list

void MMIO_intErrorImpl(const char *fn, const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "[MMIO] \033[0;31merror\033[0;0m (%-30s): \033[0;31m", fn);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\033[0;0m\n");
	va_end(args);
}
