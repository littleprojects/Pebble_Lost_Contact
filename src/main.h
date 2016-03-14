#pragma once
#include <pebble.h>
//#include "story.h"
//#include "save.h"

//---------------------------------- main settings -----------------------

#define MAX_TEXT 20
#define MAX_ANTW 5
#define MAX_MILESTONE 10
#define MAX_ACHIEVEMENT 10

#define SETTINGS_KEY 1

#define welcome_text_color				GColorBlack
#define welcome_bg_color					PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)

#define timeline_text_color 			GColorBlack
#define timeline_box_border_color GColorBlack
#define timeline_box_fill_color   PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite)

// Blue-Black OxfordBlue-White
#define settings_text_color				PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack)
#define settings_bg_color					PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)
#define settings_text_color_hi		PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define settings_bg_color_hi			PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)

#define credits_text_color				GColorBlack
#define credits_bg_color					PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)
