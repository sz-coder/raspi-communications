#include "../MMIO.h"
#include <stdio.h>
#include <arpa/inet.h>

bool MMIO_getDeviceInfo(raspi_information *info) {
	info->model_no = 0;
	info->phy_base_addr = 0;
	info->cpu = "Unknown";

	FILE *fp = fopen ("/proc/device-tree/system/linux,revision", "r");

	if (!fp) {
		return false;
	}

	uint32_t tmp = 0;
	unsigned rev = 0;

	int bytes_read = fread(&tmp, 1, sizeof(tmp), fp);

	fclose(fp);

	if (bytes_read != sizeof(tmp)) {
		return false;
	}

	rev = ntohl(tmp); // convert big endian to little endian
	rev &= 0xFFFFFF; // mask out warranty bit

	rev = (rev >> 12) & 0xF;

	switch (rev) {
		// BCM2835 (Raspberry Pi 1)
		case 0x0:
			info->model_no = 1;
			info->phy_base_addr = 0x20000000;
			info->cpu = "BCM2835";
		break;

		// BCM2836 (Raspberry Pi 2)
		case 0x1:
		// BCM2837 (Raspberry Pi 3)
		case 0x2:
			info->model_no = rev == 0x1 ? 2 : 3;
			info->phy_base_addr = 0x3F000000;

			if (info->model_no == 2) {
				info->cpu = "BCM2836";
			} else {
				info->cpu = "BCM2837";
			}
		break;

		// BCM2711 (Raspberry Pi 4)
		case 0x3:
			info->model_no = 4;
			info->phy_base_addr = 0xFE000000;
			info->cpu = "BCM2711";
		break;
	}

	if (!info->model_no) {
		MMIO_intError("Unable to detect Raspberry Pi Model no.");

		return false;
	}


	MMIO_intDebug("Detected Raspberry Pi Version %u", info->model_no);
	MMIO_intDebug("Detected CPU %s", info->cpu);
	MMIO_intDebug("phy_base_addr is %8.8x", info->phy_base_addr);

	return true;
}
