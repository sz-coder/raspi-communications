#include "MMIO.h"
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

raspi_information info;

int main(void) {
	if (!MMIO_getDeviceInfo(&info)) {
		fprintf(stderr, "Unable to detect Raspberry Pi Model!\n");
		exit(1);
	}

	MMIO_open(info.phy_base_addr);

	MMIO_dump();

	return 0;
}
