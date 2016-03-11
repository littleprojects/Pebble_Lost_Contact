#pragma once
#include "main.h"
#include "story.h"

//settings struct
typedef struct {
  //current game 
	int 	active_text[MAX_TEXT];
	int 	active_antw[MAX_ANTW];
	int 	next_step;
	int   active_text_count;
  int   active_antw_count;
	//int 	current_section;
	int	wakeup_reason;					//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;			//save the current position in the timeline
	//settings	
	bool vibe;
	bool rapid_mode;
	bool sleep_time;
	
	//history
	int history[MAX_MILESTONE];
	//int 	history_count;
	/*
	ToDo:
		- game stats
			* Stats
			* achievements
	*/	
} __attribute__((__packed__)) SAVEGAME;


//extern void load_savegame();
//extern void save_savegame();