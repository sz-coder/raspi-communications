#if !defined(RASPI_MMIO_H)
	#define RASPI_MMIO_H

	#include <stdbool.h> // bool
	#include <stdint.h> // uintX_t

	/**
	 * Represenation of various GPIO registers in memory.
	 *
	 * *BASE is the base pointer address returned by mmap().
	 */
	typedef struct {
		volatile uint32_t *BASE;

		volatile uint32_t *GPFSEL0;
		volatile uint32_t *GPFSEL1;
		volatile uint32_t *GPFSEL2;
		volatile uint32_t *GPFSEL3;
		volatile uint32_t *GPFSEL4;
		volatile uint32_t *GPFSEL5;

		volatile uint32_t *GPSET0;
		volatile uint32_t *GPSET1;

		volatile uint32_t *GPCLEAR0;
		volatile uint32_t *GPCLEAR1;

		volatile uint32_t *GPLEV0;
		volatile uint32_t *GPLEV1;
	} raspi_gpio_registers;

	typedef struct {
		unsigned model_no;
		uint32_t phy_base_addr;
		const char *cpu;
	} raspi_information;

	bool MMIO_getDeviceInfo(raspi_information *info);

	const raspi_gpio_registers *MMIO_open(
		uint32_t phy_base_addr
	);

	void MMIO_dump(void);

	void MMIO_setGPIODirection(unsigned gpio, bool direction);
	void MMIO_writeGPIOValue(unsigned gpio, bool value);
	bool MMIO_readGPIOValue(unsigned gpio);

	bool MMIO_intGetGPIODirection(unsigned gpio);

	void MMIO_intDebugImpl(
		const char *fn,
		const char *fmt, ...
	);

	void MMIO_enableDebug(bool value);
	bool MMIO_intDebugEnabled(void);

	#define MMIO_intDebug(fmt, ...) \
		MMIO_intDebugImpl(__func__, fmt, ##__VA_ARGS__)

	void MMIO_intErrorImpl(
		const char *fn,
		const char *fmt, ...
	);

	#define MMIO_intError(fmt, ...) \
		MMIO_intErrorImpl(__func__, fmt, ##__VA_ARGS__)
#endif
