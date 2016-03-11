#pragma once
#include <pebble.h>
#include "main.h"

//----------------------------- ENUMs & STRUCTs ----------------------------

typedef enum {
  INFO,			//create a Info Textbox
	TEXT,			//create a normal Textbox
	ANTW,			//create a answer
	CLEAR,		//clear all messages
	//SHOW_OVERLAY,
	//HIDE_OVERLAY,
	MILESTONE, //save this ID as a Milestone -> restart from here
	DIE_AKKU,
	SHOW_WON,
	SHOW_DIE,
	ACHIEVEMENT_X,
	//CHANGE_SETTING_X,
	SPECIAL,
} TYPE;

typedef struct {
	int 			id;
	int 			next;
	uint16_t 	delay;
	TYPE 			type;
	char 			*text;
} STEP;

typedef struct {
	char 			*name;
	char 			*desc;
	uint16_t 	step_count;
	char			*step_count_text;
	char			*version;
	char			*credits;
	STEP 			step[];
} GAME_FLOW;

//----------------------------- extern vars -------------------------------

//extern int TYPE;

extern GAME_FLOW g;