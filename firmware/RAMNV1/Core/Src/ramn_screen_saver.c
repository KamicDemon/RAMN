/*
 * ramn_screen_saver.h
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2024 TOYOTA MOTOR CORPORATION.
 * ALL RIGHTS RESERVED.</center></h2>
 *
 * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include "ramn_screen_saver.h"


char ascii_string[16] = {0};
char random_char_line[19] = {0};
uint8_t menu_drawn = 0U;

static void ScreenSaver_Init() {
	RAMN_SCREENUTILS_DrawBase();
}

static void ScreenSaver_Update(uint32_t tick) {
	if (menu_drawn == 0U)
	{
		menu_drawn = 1U;
		RAMN_SPI_DrawString(5+22,5+64, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, " Use ECU C Shift \rto control screen");

	}
	if (tick > 5000) // don't erase display for the first 5 seconds
	{
	//random value for the "digital rain" effect on screen
	uint16_t random_colors[] = {RAMN_SCREENUTILS_COLORTHEME.DARK, RAMN_SCREENUTILS_COLORTHEME.DARK, RAMN_SCREENUTILS_COLORTHEME.MEDIUM, RAMN_SCREENUTILS_COLORTHEME.MEDIUM, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.LIGHT, RAMN_SCREENUTILS_COLORTHEME.WHITE};
	uint8_t random_X_line = RAMN_RNG_Pop8() % sizeof(random_char_line);
	uint8_t random_Y_line = RAMN_RNG_Pop8() % 12;
	uint8_t random_val = RAMN_RNG_Pop8();
	uint16_t color = random_colors[random_val % ((sizeof(random_colors)/sizeof(uint16_t)))];
	uint8_t random_char = (random_val % 75) + '0';

	RAMN_SPI_DrawChar(5+(random_X_line*12), 5+(random_Y_line*16), color, RAMN_SCREENUTILS_COLORTHEME.BACKGROUND, random_char);
	}
	//Code to display a message if problems happened happened
	if (RAMN_SCREENUTILS_LoopCounter % 5 == 0)
	{
		RAMN_SCREENUTILS_DrawSubconsoleUpdate();
	}


}

static void ScreenSaver_Deinit() {

}



static void ScreenSaver_UpdateInput(JoystickEventType event) {

	// As an example, change color theme if center button is pressed
	if (event == JOYSTICK_EVENT_CENTER_PRESSED)
	{
		RAMN_SCREENUTILS_NextTheme();
	};

}

RAMNScreen ScreenSaver = {
    .Init = ScreenSaver_Init,
    .Update = ScreenSaver_Update,
    .Deinit = ScreenSaver_Deinit,
	.UpdateInput = ScreenSaver_UpdateInput
};
