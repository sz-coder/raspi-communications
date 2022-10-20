#include "../MMIO.h"
#include <sys/mman.h> // mmap
#include <fcntl.h> // O_RDWR, O_SYNC
#include <unistd.h> // open, close

static raspi_gpio_registers current;
raspi_gpio_registers *mmio_raspi_gpio_current_regs = NULL;

static volatile uint32_t *mapMemory(uint32_t base_addr, uint32_t len) {
	int fd = open("/dev/mem", O_RDWR | O_SYNC);

	if (0 > fd) {
		MMIO_intError("failed to open '/dev/mem'. Are you root?");

		return MAP_FAILED;
	}

	uint32_t *ret = (uint32_t *)mmap(
		0,
		// length
		len,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_LOCKED,
		// file descriptor to /dev/mem
		fd,
		// address
		base_addr
	);

	close(fd);

	if (ret == MAP_FAILED) {
		MMIO_intError("failed mmap.");

		return MAP_FAILED;
	}

	return ret;
}

static raspi_gpio_registers initGPIORegistersStruct(
	volatile uint32_t *gpio_base_addr
) {
	raspi_gpio_registers ret;

	ret.BASE     = gpio_base_addr;

	ret.GPFSEL0  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x00);
	ret.GPFSEL1  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x04);
	ret.GPFSEL2  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x08);
	ret.GPFSEL3  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x0C);
	ret.GPFSEL4  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x10);
	ret.GPFSEL5  = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x14);

	ret.GPSET0   = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x1C);
	ret.GPSET1   = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x20);

	ret.GPCLEAR0 = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x28);
	ret.GPCLEAR1 = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x2C);

	ret.GPLEV0   = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x34);
	ret.GPLEV1   = (volatile uint32_t *)(((unsigned char *)gpio_base_addr) + 0x38);

	return ret;
}

const raspi_gpio_registers *MMIO_open(
	uint32_t phy_base_addr
) {
	if (mmio_raspi_gpio_current_regs) {
		MMIO_intDebug(
			"returning already initialized raspi_gpio_registers %p",
			(void *)mmio_raspi_gpio_current_regs
		);

		return mmio_raspi_gpio_current_regs;
	}

	volatile uint32_t *gpio_virt_base = mapMemory(
		phy_base_addr + 0x00200000, 0xF4
	);

	current = initGPIORegistersStruct(gpio_virt_base);
	mmio_raspi_gpio_current_regs = &current;

	MMIO_intDebug(
		"initialized mmio_raspi_gpio_current_regs to %p",
		(void *)mmio_raspi_gpio_current_regs
	);

	return mmio_raspi_gpio_current_regs;
}
