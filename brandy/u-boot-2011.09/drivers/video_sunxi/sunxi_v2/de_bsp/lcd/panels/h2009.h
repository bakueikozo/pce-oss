#ifndef  __DEFAULT_PANEL_H__
#define  __DEFAULT_PANEL_H__
#include "panels.h"
#include "../../de/disp_display.h"
#include "../lcd_source_interface.h"
#define panel_rst(v)    (sunxi_lcd_gpio_set_value(0, 0, v))
extern __lcd_panel_t h2009_panel;
#endif
