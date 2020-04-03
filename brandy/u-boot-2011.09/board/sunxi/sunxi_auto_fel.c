/* Copyright 2018-2020
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <sys_config.h>
#include <private_uboot.h>
#include <sunxi_board.h>

DECLARE_GLOBAL_DATA_PTR;

extern int do_fel_from_boot(cmd_tbl_t *cmdtp, int flag, int arg, char *const argv[]);

int sunxi_auto_fel_by_usb(void)
{
	int auto_fel = 0;

	if(uboot_spare_head.boot_data.work_mode != WORK_MODE_BOOT)
		return 0;

	script_parser_fetch("target", "auto_fel", &auto_fel, 1);

	if (auto_fel != 1) {
		debug("auto fel not enabled\n");
		return 0;
	}

	if(gd->vbus_status == SUNXI_VBUS_NOT_EXIST)
	{
		puts("out of usb burn from boot: without usb\n");

		return 0;
	}

	return do_fel_from_boot(NULL, 0, 0, NULL);
}
