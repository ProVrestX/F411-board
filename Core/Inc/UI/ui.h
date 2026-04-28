/*
 * ui.h
 *
 *  Created on: 27 февр. 2026 г.
 *      Author: andrew
 */

#ifndef SRC_UI_H_
#define SRC_UI_H_

#include "main.h"


typedef struct {
	char *text;
	void (*clk_handler)(void);
} Item_t;

typedef struct {
	char *title;
	Item_t *menu;
	uint8_t menu_size;
} Menu_t;


extern void (*draw_ui)(void);

void ui_init(void);
void set_menu_func(void);
void open_main_menu(void);
void open_sec_menu(void);



#endif /* SRC_UI_H_ */
