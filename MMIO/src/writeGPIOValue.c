#include "../MMIO.h"
#include <stddef.h>

extern raspi_gpio_registers *mmio_raspi_gpio_current_regs;

void MMIO_writeGPIOValue(unsigned gpio, bool value) {
	if (!mmio_raspi_gpio_current_regs) {
		MMIO_intError("not initialized.");

		return;
	} else if (gpio > 53) {
		MMIO_intError("invalid gpio=%u.", gpio);

		return;
	} else if (!MMIO_intGetGPIODirection(gpio)) {
		MMIO_intError("cannot write to a pin that is in input mode.");

		return;
	}

	const raspi_gpio_registers *regs = mmio_raspi_gpio_current_regs;

	volatile uint32_t *GPREG = NULL;
	unsigned reg_no;

	// GPSET reg
	if (value) {
		reg_no = (gpio > 31) ? 1 : 0;
		GPREG  = reg_no == 1 ? regs->GPSET1 : regs->GPSET0;
	}
	// GPCLEAR reg
	else {
		reg_no = (gpio > 31) ? 1 : 0;
		GPREG  = reg_no == 1 ? regs->GPCLEAR1 : regs->GPCLEAR0;
	}

	unsigned rel_gpio_no = gpio - (reg_no * 32);
	uint32_t bit_mask = (1u << rel_gpio_no);

	MMIO_intDebug(
		"gpio=%u reg_no=%u rel_gpio_no=%u reg_addr=%p, bit_mask=0x%8.8x",
		gpio, reg_no, rel_gpio_no, bit_mask
	);

	*GPREG |= bit_mask;
}
