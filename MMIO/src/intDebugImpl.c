#include <stdio.h> // vfprintf, fprintf, stderr
#include <stdarg.h> // va_list
#include <stdbool.h> // bool

static bool debug_enabled = false;

void MMIO_enableDebug(bool value) {
	debug_enabled = value;
}

bool MMIO_intDebugEnabled(void) {
	return debug_enabled;
}

void MMIO_intDebugImpl(const char *fn, const char *fmt, ...) {
	if (!debug_enabled) return;

	va_list args;

	va_start(args, fmt);
	fprintf(stderr, "[MMIO] debug (%-30s): ", fn);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
}
