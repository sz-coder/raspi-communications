#include <zlib.h> // crc32
#include <stdint.h> // uintX_t
#include <string.h> // memset
#include <stdio.h>

extern void debug(const char *msg, ...);
extern void warn(const char *msg, ...);
extern void error(const char *msg, ...);

#define STATE_INITIAL 0
#define STATE_RECEIVE_PACKET_LENGTH 1
#define STATE_RECEIVE_PACKET_CRC32 2
#define STATE_RECEIVE_PACKET 3

static int g_current_state = STATE_INITIAL;
static uint8_t g_packet_size = 0;
static int g_crc32_pos = 0;
static uint32_t g_crc32_value = 0;
static int g_received_bytes = 0;
static char g_packet_buffer[256];

void handleIncomingPacket(const char *buffer, uint8_t buffer_size) {
	fprintf(stderr, "I GOT %s\n", buffer);
}

void handleIncomingByte(uint8_t byte) {
	debug("got byte %d", byte);

	switch (g_current_state) {
		case STATE_INITIAL: {
			if (byte == 0xAE) {
				debug("--- received PACKET START byte ---");

				g_current_state = STATE_RECEIVE_PACKET_LENGTH;
			} else {
				warn("--- invalid byte 0x%2.2x expected 0xae", byte);
			}
		} break;

		case STATE_RECEIVE_PACKET_LENGTH: {
			debug("packet size = %d", byte);

			g_packet_size = byte;
			g_crc32_value = 0;
			g_crc32_pos = 0;
			g_current_state = STATE_RECEIVE_PACKET_CRC32;
		} break;

		case STATE_RECEIVE_PACKET_CRC32: {
			debug("receive crc32 value %d (pos=%d)", byte, g_crc32_pos);

			unsigned char shift = (g_crc32_pos * 8);

			g_crc32_value |= (byte << shift);

			++g_crc32_pos;

			if (g_crc32_pos == 4) {
				debug("received crc32 value %8.8x", g_crc32_value);

				memset(g_packet_buffer, 0, sizeof(g_packet_buffer));

				g_received_bytes = 0;
				g_current_state = STATE_RECEIVE_PACKET;
			}
		} break;

		case STATE_RECEIVE_PACKET: {
			debug("received byte");

			g_packet_buffer[g_received_bytes] = byte;

			++g_received_bytes;

			if (g_received_bytes == g_packet_size) {
				debug("+++ packet end +++");

				g_packet_buffer[255] = 0;

				// calculate CRC32 of packet..
				uint32_t packet_crc32 = crc32(0L, Z_NULL, 0);

				packet_crc32 = crc32(packet_crc32, g_packet_buffer, g_packet_size);

				debug("calculated crc32 is %8.8x", packet_crc32);
				debug("expected crc32 is %8.8x", g_crc32_value);

				if (packet_crc32 != g_crc32_value) {
					warn("crc32 mismatch! <%8.8x != %8.8x>", packet_crc32, g_crc32_value);
				} else {
					handleIncomingPacket(g_packet_buffer, g_packet_size);
				}

				g_current_state = STATE_INITIAL;
			}
		} break;
	}
}
