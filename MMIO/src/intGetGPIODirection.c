#include "../MMIO.h"
#include <stdio.h>

extern raspi_gpio_registers *mmio_raspi_gpio_current_regs;

bool MMIO_intGetGPIODirection(unsigned gpio) {
	if (!mmio_raspi_gpio_current_regs) {
		MMIO_intError("not initialized.");

		return false;
	} else if (gpio > 53) {
		MMIO_intError("invalid gpio=%u.", gpio);

		return false;
	}

	const raspi_gpio_registers *regs = mmio_raspi_gpio_current_regs;

	// one GPFSEL0 is 4 bytes wide

	// reg_no 0 = GPFSEL0
	// reg_no 1 = GPFSEL1
	// etc..
	unsigned reg_no = gpio / 10;

	// do pointer arithmetic on GPFSEL0
	volatile unsigned char *tmp = (volatile unsigned char *)regs->GPFSEL0;
	tmp += 4 * reg_no;

	// GPFSEL is now a pointer to GPFSELn where n is `reg_no`
	// examples:
	// reg_no=1, GPFSEL=GFPSEL1
	// reg_no=3, GPFSEL=GFPSEL3
	volatile uint32_t *GPFSEL = ((volatile uint32_t *)tmp);

	// 10 gpios per register
	// 30 bits (-2 reserved)

	// relative GPIO number
	// 0 means first GPIO pin of that register (of GPFSEL)
	// ...
	// 9 means last GPIO pin of that register (of GPFSEL)
	unsigned rel_gpio_no = (gpio - (reg_no * 10));

	// clear all irrelevant bits
	uint32_t bit_clear_mask = (0x001ul << (rel_gpio_no * 3)); // 0x001ul is 0b001ul

	uint32_t read_value = *GPFSEL;

	read_value &= bit_clear_mask;

	MMIO_intDebug(
		"gpio=%u reg_no=%u rel_gpio_no=%u reg_addr=%p, clear_mask=0x%8.8x read_value=0x%8.8x",
		gpio, reg_no, rel_gpio_no, bit_clear_mask, read_value
	);

	return read_value > 0;
}
