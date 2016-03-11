#include <pebble.h>
#include "save.h"

//set defaults
SAVEGAME savegame = {
	//current game var
  .active_text_count 	= 0,
	.active_antw_count 	= 0,
	.active_text 				= {0},
	.active_antw 				= {0},
	.next_step 					= 0,
	//.current_section 	= 0,
	.wakeup_reason 			= -1,
	.menu_index 				= {0,0},
	
	//settings
	.vibe 							= true,
	.rapid_mode 				= false,
	.sleep_time 				= true,
	
	//history
	.history = {-1}
	//.history_count = 0
};



void settings_to_savegame() {
	
}

void savegame_to_settings() {
	
}


