/*
 * ui.c
 *
 *  Created on: 27 февр. 2026 г.
 *      Author: andrew
 */

#include "main.h"

#include "ui.h"
#include "stdio.h"

#include "st7735.h"
#include "fonts.h"
#include "sd_functions.h"
#include "encoder.h"

#include "sd_menu.h"


Item_t main_menu_item[] = {
		{"Sec menu", &open_sec_menu},
		{"SD menu", &open_SD_menu},
		{"3rd_line", NULL}};
Menu_t main_menu = {"Main", main_menu_item, 3};

Item_t sec_menu_item[] = {
		{"1---", NULL},
		{"Main menu", &open_main_menu},
		{"...", NULL},
		{"test<>", NULL}};
Menu_t sec_menu = {"Second", sec_menu_item, 4};

Menu_t *cur_menu = &main_menu;
uint8_t cursor_pos = 0;

static void draw_menu(void);
static void cursor_scrol(int8_t dur);
static void cursor_select(void);


void (*draw_ui)(void) = draw_menu;

void ui_init(void) {
    cursor_pos = 0;
    cur_menu = &main_menu;
    set_menu_func();
}

void set_menu_func(void) {
	draw_ui = draw_menu;
	Encoder_SetClickFunc(cursor_select);
    Encoder_SetScrolFunc(cursor_scrol);
}

static void draw_menu(void) {
	uint8_t pos_y = 2;
	char text_tmp[32];

	ST7735_ClearFrameBuffer();

	// Draw head
	sprintf(text_tmp, "Hello!");
	ST7735_print(5, pos_y, ST7735_WHITE, ST7735_BLACK, 0, &Font_6x8, 1, text_tmp);
	pos_y += 10;

	// Draw menu
	ST7735_print(5, pos_y, ST7735_CYAN, ST7735_BLACK, 0, &Font_7x9, 1, cur_menu->title);
	pos_y += 15;

	for(uint8_t line = 0; line < cur_menu->menu_size; line++) {
		ST7735_print(10, pos_y, ST7735_CYAN, ST7735_BLACK, 0, &Font_7x9, 1, cur_menu->menu[line].text);
		if(line == cursor_pos) {
			ST7735_print(2, pos_y, ST7735_CYAN, ST7735_BLACK, 0, &Font_7x9, 1, ">");
		}
		pos_y += 12;
	}

	// Draw bottom
	sprintf(text_tmp, "SD: %s (%d MB)", (SD_GetState())? "ok": "err", SD_GetSpace());
	ST7735_print(5, ST7735_Height - 10, ST7735_WHITE, ST7735_BLACK, 0, &Font_6x8, 1, text_tmp);

	ST7735_Update();
}

static void cursor_scrol(int8_t dur) {
	if(dur < 0) {
		if(cursor_pos > 0)
			cursor_pos--;
	}
	else if(dur > 0) {
		if(cursor_pos + 1 < cur_menu->menu_size)
			cursor_pos++;
	}
}

static void cursor_select(void) {
	if(cur_menu->menu[cursor_pos].clk_handler != NULL) {
		cur_menu->menu[cursor_pos].clk_handler();
	}
}

void open_main_menu(void) {
	cur_menu = &main_menu;
	cursor_pos = 0;
}

void open_sec_menu(void) {
	cur_menu = &sec_menu;
	cursor_pos = 0;
}




