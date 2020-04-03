/* Copyright 2018 allwinnertech  <wangwei@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * wangwei@allwinnertech.com
 */

#include <common.h>
//#include <console.h>
#include <command.h>
#include <sunxi_board.h>

extern int sunxi_usb_dev_register(uint dev_name);
extern  void sunxi_usb_main_loop(int delaytime);
extern  int sunxi_usb_extern_loop(void);
extern  int sunxi_usb_init(int delaytime);
extern  int sunxi_usb_exit(void);

DECLARE_GLOBAL_DATA_PTR;

extern volatile int sunxi_usb_burn_from_boot_handshake, sunxi_usb_burn_from_boot_init, sunxi_usb_burn_from_boot_setup;
extern volatile int sunxi_usb_probe_update_action;

int do_fel_from_boot(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int    ret;
	ulong begin_time= 0, over_time = 0;

	tick_printf("probe MP tools from boot\n");
	if (sunxi_usb_dev_register(4) < 0) {
		printf("usb burn fail: not support burn private data\n");
		return -1;
	}

	if (sunxi_usb_init(0)) {
		printf("%s usb init fail\n", __func__);
		sunxi_usb_exit();
		return 0;
	}

	tick_printf("usb init ok\n");
	begin_time = get_mtimer(0);
	over_time = 500; /* 800ms */
	while(1) {
		if (sunxi_usb_burn_from_boot_init) {
			printf("usb sof ok\n");
			break;
		}
		if (get_mtimer(begin_time) > over_time) {
			tick_printf("usb overtime\n");
			sunxi_usb_exit();
			return 0;
		}
	}
	tick_printf("usb probe ok\n");

	begin_time = get_mtimer(0);
	over_time = 2000; /* 3000ms */
	while(1) {
		ret = sunxi_usb_extern_loop();
		if (ret)
			break;

		if (!sunxi_usb_burn_from_boot_handshake) {
			if(get_mtimer(begin_time) > over_time) {
				sunxi_usb_exit();
				printf("usb: have no handshake\n");
				return 0;
			}
		}

		if (sunxi_usb_probe_update_action == 1) {
			printf("usb: enter fel mode\n");
			sunxi_board_run_fel();
		}
		else if (sunxi_usb_probe_update_action == 2) {
			printf("ready to boot normal \n");
			break ;
		}
	}
	tick_printf("exit usb fel from boot\n");
	sunxi_usb_exit();

	return 0;
}

U_BOOT_CMD(sunxi_fel, CONFIG_SYS_MAXARGS, 1, do_fel_from_boot,
		"do fel from boot",
		"sunxi_fel"
		"NULL"
	  );
