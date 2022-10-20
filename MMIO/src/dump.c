#include "../MMIO.h"
#include <stdio.h>

extern raspi_gpio_registers *mmio_raspi_gpio_current_regs;

void MMIO_dump(void) {
	if (!mmio_raspi_gpio_current_regs) {
		MMIO_intError("not initialized.");

		return;
	}

	raspi_information info;
	MMIO_getDeviceInfo(&info);

	bool debug_enabled = MMIO_intDebugEnabled();
	MMIO_enableDebug(false);

	fprintf(stderr, "\n");
	fprintf(stderr, "Raspberry Pi Model Version : %u\n", info.model_no);
	fprintf(stderr, "Physical RAM Address       : %8.8x\n", info.phy_base_addr);
	fprintf(stderr, "CPU                        : %s\n", info.cpu);
	fprintf(stderr, "\n");

	for (int row = 0; row < 10; ++row) {
		for (int column = 0; column < 6; ++column) {
			int gpio_id = (column * 10) + row;

			if (gpio_id > 53) break;

			bool direction = MMIO_intGetGPIODirection(gpio_id);

			fprintf(stderr, "GPIO %-2d %s", gpio_id, direction ? "OUT" : "IN");

			if (!direction) {
				bool value = MMIO_readGPIOValue(gpio_id);

				if (value) {
					fprintf(stderr, "\033[0;33m<HI>\033[0;0m ");
				} else {
					fprintf(stderr, "\033[0;34m<LO>\033[0;0m ");
				}
			} else {
				fprintf(stderr, "\033[0;30m<?>\033[0;0m ");
			}

			fprintf(stderr, "   ");
		}

		fprintf(stderr, "\n");
	}

	MMIO_enableDebug(debug_enabled);
}
