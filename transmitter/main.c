/**
 * compile this file with:
 *
 * -lwiringPi -lz
 */
#include <wiringPi.h> // pinMode, digitalRead
#include <zlib.h> // crc32

#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <string.h> // strlen
#include <stdint.h> // uintX_t
#include <unistd.h> // usleep()
#include <stdio.h> // vfprintf()
#include <stdarg.h> // va_list

#define TRANSMIT_CLOCK_PIN   0
#define TRANSMIT_DATA_PIN    2

void debug(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "[debug] ");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void warn(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "[warn ] \e[0;33m");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\e[0;0m\n");
	va_end(args);
}

void error(const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "[error] \e[0;31m");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\e[0;0m\n");
	va_end(args);

	exit(1);
}

void initIO(void) {
	debug("initializing I/O");

	if (wiringPiSetup() == -1) {
		error("failed to initialize I/O");
	}

	pinMode(TRANSMIT_CLOCK_PIN, OUTPUT);
	pinMode(TRANSMIT_DATA_PIN, OUTPUT);

	debug("pin %d = clock", TRANSMIT_CLOCK_PIN);
	debug("pin %d = data", TRANSMIT_DATA_PIN);

	debug("I/O initialized");
}

static void putDataBit(bool bit, bool clk_val) {
	usleep(4E3);

	digitalWrite(TRANSMIT_DATA_PIN, bit);
	usleep(1E3);
	digitalWrite(TRANSMIT_CLOCK_PIN, clk_val);
}

/**
 * Transmit two bits `v1` and `v2`.
 */
static void sendBits(bool v1, bool v2) {
	putDataBit(v1, true);
	putDataBit(v2, false);
}

static void sendByte(uint8_t byte) {
	digitalWrite(TRANSMIT_CLOCK_PIN, 1);
	usleep(1E3);
	digitalWrite(TRANSMIT_CLOCK_PIN, 0);
	usleep(100E3);

	for (int i = 0; i < 8; ++i) {
		// j is first bit position
		int j = i++;

		bool v1 = (byte >> j) & 1;
		bool v2 = (byte >> i) & 1;

		sendBits(v1, v2);
	}

	usleep(10E3);
}

static void sendPacket(const char *str) {
	// calculate CRC32 of packet..
	uint32_t packet_crc32 = crc32(0L, Z_NULL, 0);

	packet_crc32 = crc32(packet_crc32, str, strlen(str));

	// packet start byte
	sendByte(0xae);
	// packet size
	sendByte(strlen(str));
	// crc32 value

	for (int i = 0; i < 4; ++i) {
		uint8_t tmp = (packet_crc32 >> (i * 8));

		sendByte(tmp);
	}

	for (int i = 0; i < strlen(str); ++i) {
		sendByte(str[i]);
	}
}

int main(int argc, const char **argv) {
	initIO();

	sendPacket("hello, world!");

	for (int i = 1; i < argc; ++i) {

	}
}
