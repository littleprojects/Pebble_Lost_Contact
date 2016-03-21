#pragma once
#include "main.h"
#include "story.h"

//settings struct
typedef struct {
  //current game 
	int   active_text[MAX_TEXT];	//IDs of the current active texts - is 0 if null
	int 	active_antw;						//ID of the current antw, is 0 if no antw
	int 	next_step;							//ID of the next Step, is 0 if NULL
	int   active_text_count;			
  //int   active_antw_count;
	//int 	current_section;
	int	wakeup_reason;						//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;					//save the current position in the timeline
	//settings	
	bool vibe;
	bool rapid_mode;
	bool sleep_time;
	
	//history
	int history[MAX_MILESTONE];		//IDs of the Milesotens
	bool found_deads[MAX_DEADS]; //IDs of the Achievments
	bool found_alives[MAX_ALIVES];

} __attribute__((__packed__)) SAVEGAME;

extern SAVEGAME savegame;

extern void load_savegame();
extern void save_savegame();