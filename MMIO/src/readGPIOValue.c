#include "../MMIO.h"
#include <stddef.h>

extern raspi_gpio_registers *mmio_raspi_gpio_current_regs;

bool MMIO_readGPIOValue(unsigned gpio) {
	if (!mmio_raspi_gpio_current_regs) {
		MMIO_intError("not initialized.");

		return false;
	} else if (gpio > 53) {
		MMIO_intError("invalid gpio=%u.", gpio);

		return false;
	} else if (MMIO_intGetGPIODirection(gpio)) {
		MMIO_intError("cannot read a pin that is in output mode.");

		return false;
	}

	const raspi_gpio_registers *regs = mmio_raspi_gpio_current_regs;

	volatile uint32_t *GPLEV = NULL;
	unsigned reg_no;

	// GPLEV0 reg
	reg_no = (gpio > 31) ? 1 : 0;
	GPLEV  = reg_no == 1 ? regs->GPLEV1 : regs->GPLEV0;

	unsigned rel_gpio_no = gpio - (reg_no * 32);
	uint32_t bit_mask = (1u << rel_gpio_no);

	uint32_t read_value = *GPLEV;

	read_value &= bit_mask;

	MMIO_intDebug(
		"gpio=%u reg_no=%u rel_gpio_no=%u reg_addr=%p, bit_mask=0x%8.8x read_value=%8.8x",
		gpio, reg_no, rel_gpio_no, bit_mask, read_value
	);

	return !!read_value;
}
