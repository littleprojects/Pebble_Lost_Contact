#pragma once
#include <pebble.h>
//#include "story.h"
//#include "save.h"

//---------------------------------- main settings -----------------------

#define MAX_TEXT 15
#define MAX_ANTW 4
#define MAX_MILESTONE 10
#define MAX_DEADS 4
#define MAX_ALIVES 1

#define SETTINGS_KEY 1						//saveloaction

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

#define milestone_text_color			PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack)
#define milestone_bg_color				PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)
#define milestone_text_color_hi		PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define milestone_bg_color_hi			PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)

#define credits_text_color				GColorBlack
#define credits_bg_color					PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)
