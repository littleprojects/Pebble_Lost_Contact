#include <pebble.h>
#include "main.h"
#include "save.h"
#include "story.h"

//set defaults
SAVEGAME savegame = {
	//current game var
  .active_text_count 	= 0,
	//.active_antw_count 	= 0,
	.active_text 				= {0},
	.active_antw 				= 0,
	.next_step 					= 0,
	//.current_section 	= 0,
	.wakeup_reason 			= -1,
	.menu_index 				= {0,0},
	
	//settings
	.vibe 							= true,
	.rapid_mode 				= false,
	.sleep_time 				= true,
	
	//history
	.history = {0},
	.achievement = {0}
	//.history_count = 0
};



void save_savegame() {
	//wirte savegame in the memory
	persist_write_data(SETTINGS_KEY, &savegame, sizeof(savegame)); 
}

void load_savegame() {
	//check the savegame (exist & size)
	if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == (int)sizeof(savegame)){
		//read data from momory
		persist_read_data(SETTINGS_KEY, &savegame, sizeof(savegame));	
		
	}else{
		//write the defaults to settings
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "load default Data");

		//save the first time
		//save_savegame();
	}	
}


