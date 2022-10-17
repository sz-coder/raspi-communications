/**
 * compile this file with:
 *
 * -lwiringPi -lz
 */
#include <wiringPi.h> // pinMode, digitalRead

#include <stdbool.h> // bool
#include <stdlib.h> // exit
#include <string.h> // strlen
#include <stdint.h> // uintX_t
#include <unistd.h> // usleep()
#include <stdio.h> // vfprintf()
#include <stdarg.h> // va_list

#include <time.h> // clock_gettime, struct timespec

#define RECEIVE_CLOCK_PIN   0
#define RECEIVE_DATA_PIN    2

#define ONE_SECOND_IN_MICROS 1000000L
#define CLOCK_LOW_IN_MICROS  95000L   // 95ms
#define TIMEOUT_IN_MICROS    50000L   // 50ms

#define STATE_IDLE          0
#define STATE_CLOCK_PIN_LOW 1
#define STATE_RECEIVE_DATA  2

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

	pinMode(RECEIVE_CLOCK_PIN, INPUT);
	pinMode(RECEIVE_DATA_PIN, INPUT);

	debug("pin %d = clock", RECEIVE_CLOCK_PIN);
	debug("pin %d = data", RECEIVE_DATA_PIN);

	debug("I/O initialized");
}

static int g_current_state = STATE_IDLE;

static bool g_clk_pin = false;
static bool g_prev_clk_pin = false;
static bool g_data_pin = false;

static int g_current_receive_bitno = 0;
static uint8_t g_current_byte = 0;

static struct timespec g_timer_start_point;
static bool g_timer_started = false;

void startTimer(void) {
	if (g_timer_started) {
		error("timer not stopped.");
	}

	clock_gettime(CLOCK_MONOTONIC, &g_timer_start_point);
	g_timer_started = true;
}

long getTimerValue(void) {
	if (!g_timer_started) {
		error("timer not started.");
	}

	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);

	long seconds = now.tv_sec - g_timer_start_point.tv_sec;
	long nanoseconds = now.tv_nsec - g_timer_start_point.tv_nsec;
	long microseconds = 0;

	microseconds += nanoseconds / 1E3; // 1000ns is 1us
	microseconds += (seconds * 1E6); // 1s is 1'000'000us

	return microseconds;
}

void stopTimer(void) {
	g_timer_started = false;
}

bool placeBit(bool v, int *bitpos, uint8_t *byte) {
	int cur_bitpos = *bitpos;
	int new_bitpos = (*bitpos) + 1;

	if (v) {
		*byte |= (1 << cur_bitpos);
	}

	if (new_bitpos == 8) {
		*bitpos = 0;
	} else {
		*bitpos = new_bitpos;
	}

	return cur_bitpos == 7;
}

extern void handleIncomingByte(uint8_t byte);

int main(int argc, const char **argv) {
	initIO();

	while (true) {
		// read I/O pins
		g_clk_pin = digitalRead(RECEIVE_CLOCK_PIN);
		g_data_pin = digitalRead(RECEIVE_DATA_PIN);

		switch (g_current_state) {
			case STATE_IDLE: {
				// falling clock edge
				if (!g_clk_pin && g_prev_clk_pin) {
					debug("clock pin transition high->low");

					startTimer();

					g_current_state = STATE_CLOCK_PIN_LOW;
				}
			} break;

			case STATE_CLOCK_PIN_LOW: {
				if (getTimerValue() >= CLOCK_LOW_IN_MICROS) {
					debug("clock was low for long enough. preare to receive data ...");

					// restart timer for timeout
					stopTimer();
					startTimer();

					g_current_state = STATE_RECEIVE_DATA;
					g_current_receive_bitno = 0;
					g_current_byte = 0;
				} else if (g_clk_pin) {
					warn("unexpected clock pin transition low->high");

					stopTimer();

					g_current_state = STATE_IDLE;
				}
			} break;

			case STATE_RECEIVE_DATA: {
				if (getTimerValue() >= TIMEOUT_IN_MICROS) {
					warn("receive data timeout");

					stopTimer();

					g_current_state = STATE_IDLE;
				} else {
					bool byte_received = false;

					// rising edge
					if (g_clk_pin && !g_prev_clk_pin) {
						byte_received = placeBit(
							g_data_pin,
							&g_current_receive_bitno,
							&g_current_byte
						);
					}
					// falling edge
					else if (!g_clk_pin && g_prev_clk_pin) {
						byte_received = placeBit(
							g_data_pin,
							&g_current_receive_bitno,
							&g_current_byte
						);
					}

					if (byte_received) {
						debug("received byte %2.2x (%d)", g_current_byte, g_current_byte);

						handleIncomingByte(g_current_byte);

						stopTimer();

						g_current_state = STATE_IDLE;
					}
				}
			} break;

			default: {
				error("invalid state %d", g_current_state);
			}
		}

		usleep(500);

		g_prev_clk_pin = g_clk_pin;
	}
}
