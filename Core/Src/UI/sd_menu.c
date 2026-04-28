/*
 * sd_menu.c
 *
 *  Created on: 25 мар. 2026 г.
 *      Author: andrew
 */

#include "main.h"
#include "sd_menu.h"

#include "stdio.h"
#include "ui.h"
#include "st7735.h"
#include "fonts.h"
#include "sd_functions.h"
#include "encoder.h"


char sd_buf[256] = {};

static void menu_SD_select(void);
static void draw_SD(void);


void open_SD_menu(void) {
	if(!SD_CheckState())
		return;

	draw_ui = draw_SD;
	Encoder_SetClickFunc(menu_SD_select);
    Encoder_SetScrolFunc(NULL);

	draw_ui();
}

static void menu_SD_select(void) {
	open_main_menu();
	set_menu_func();
	draw_ui();
}

static void draw_SD(void) {
	char text_tmp[32];
	uint8_t pos_y = 2;

	ST7735_ClearFrameBuffer();

	sprintf(text_tmp, "Space: %d MB", SD_GetSpace());
	ST7735_print(5, pos_y, ST7735_WHITE, ST7735_BLACK, 0, &Font_6x8, 1, text_tmp);
	pos_y += 10;

	uint8_t count = SD_ListDir(sd_path, sd_buf, 256);
	sprintf(text_tmp, "Items: %hu", count);
	ST7735_print(5, pos_y, ST7735_WHITE, ST7735_BLACK, 0, &Font_6x8, 1, text_tmp);
	pos_y += 15;

	char *sd_buf_rest;
	char *item = strtok_r(sd_buf, "\n", &sd_buf_rest);
	while(item != NULL && pos_y < ST7735_Height - 13) {
		ST7735_print(2321, pos_y, ST7735_CYAN, ST7735_BLACK, 0, &Font_6x8, 1, item);
		pos_y += 12;
		item = strtok_r(NULL, "\n", &sd_buf_rest);
	}

	ST7735_Update();
}
