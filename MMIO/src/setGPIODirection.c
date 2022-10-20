#include "../MMIO.h"

extern raspi_gpio_registers *mmio_raspi_gpio_current_regs;

// GPFSEL0       GPIO  9   -   GPIO  0
// GPFSEL1       GPIO 19   -   GPIO 10
// GPFSEL2       GPIO 29   -   GPIO 20
// GPFSEL3       GPIO 39   -   GPIO 30
// GPFSEL4       GPIO 49   -   GPIO 40
// GPFSEL5       GPIO 53   -   GPIO 50
void MMIO_setGPIODirection(unsigned gpio, bool direction) {
	if (!mmio_raspi_gpio_current_regs) {
		MMIO_intError("not initialized.");

		return;
	} else if (gpio > 53) {
		MMIO_intError("invalid gpio=%u.", gpio);

		return;
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

	// clear all bits, making pin temporarily input (no harm done here)
	uint32_t inv_bit_clear_mask = (0x7ul << (rel_gpio_no * 3)); // 0x7ul is 0b111ul
	uint32_t bit_clear_mask     = inv_bit_clear_mask ^ 0xfffffffful;

	MMIO_intDebug(
		"gpio=%u reg_no=%u rel_gpio_no=%u reg_addr=%p, inv_clear_mask=0x%8.8x clear_mask=0x%8.8x",
		gpio, reg_no, rel_gpio_no, inv_bit_clear_mask, bit_clear_mask
	);

	*GPFSEL &= bit_clear_mask;

	if (direction) {
		uint32_t bit_set_mask = (0x1ul << (rel_gpio_no * 3)); // 0x1ul is 0b001ul

		MMIO_intDebug(
			"gpio=%u reg_no=%u rel_gpio_no=%u reg_addr=%p, bit_set_mask=0x%8.8x",
			gpio, reg_no, rel_gpio_no, bit_set_mask
		);

		*GPFSEL |= bit_set_mask;
	}
}
