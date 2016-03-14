#include "pebble.h"
#include "main.h"
#include "story.h"
#include "save.h"


/* Color / BW
#ifdef PBL_COLOR

#else
	//Aplite Colors
	GColorBlack
	GColorDarkGray
	GColorLightGray
	GColorWhite
#endif

#ifdef PBL_SDK_2
PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorBlack));
*/ //colors/hardware switch

/*
	
	Idee: Gewählte Anwort mit in Timeline aufnehmen
			*	ältere Elemente immer löschen (First in, first out que)
			* que in Add Text möglich (clear Menu anpassen/ nur noch antw count = 0)(selced index anpassen --)
				
	TODO:
		- achievements
	
	Save 
		- history
		- game stats
		* achievements
	Settings
	  - Go back (History)	
	
	
	Special:		
		- Fake System Data in Settings
			* Empfang
			* Akku
			* Empfangene Nachrichten
			
	Welcome Card:
		- add action bar. (up/down/ play)
		
	overflow protection: (TEXT MSG & Responses)
		if msg_cout = max_msg_count
			delete the first and put all array(x-1)
*/ //TODOs

static bool debug = false;
static bool StepCheck = false; 		//check all steps to have a next step.

// more game settings are in the main.h

//global Layer
static Layer 			*window_layer;

static Window 		*s_main_window;		//timeline
static Window 		*s_welc_window;		//welcome
static Window 		*s_info_window;		//info (Incomming Msg / waiting for input)
static Window 		*s_menu_window;		//settings
static Window 		*s_cred_window;		//credits

static MenuLayer 	*s_menu_layer;		//timeline_menu

static ScrollLayer*s_welc_layer;		//welcome layer
static TextLayer 	*s_welc_text1;
static TextLayer 	*s_welc_text2;

static TextLayer 	*s_info_text;			//info layer 
static Layer 			*s_info_layer;

static MenuLayer 	*s_set_layer;			//settings layer

//Timer var
static AppTimer 	*s_timer;

//images
static GBitmap 		*s_background_image;
static BitmapLayer *s_background_layer;


//settings struct
typedef struct {
  //current game 
	STEP *active_text[MAX_TEXT];	//pointer of the Text Steps
	STEP *active_antw[MAX_ANTW];	//pointer of the antw Steps
	STEP *next_step;							//pointer of the next Step
	int   active_text_count;			
  int   active_antw_count;
	//int 	current_section;
	int	wakeup_reason;						//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;					//save the current position in the timeline
	
	//settings	
	bool vibe;										
	bool rapid_mode;
	bool sleep_time;
	
	//history
	int history[MAX_MILESTONE];				//ID of the milestones
	int achievement[MAX_ACHIEVEMENT];	//ID of the Achievments
	
} SETTINGS;


//set defaults
SETTINGS settings = {
	//current game var
  .active_text_count = 0,
	.active_antw_count = 0,
	.active_text = {NULL},
	.active_antw = {NULL},
	.next_step = &g.step[0],
	//.current_section = 0,
	.wakeup_reason = -1,
	.menu_index = {0,0},
	
	//settings
	.vibe = true,
	.rapid_mode = false,
	.sleep_time = true,
	
	//history
	.history = {0},
	.achievement = {0}
	//.history_count = 0
};

static GPathInfo path_1 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,18+19},{135,18+24}, {8,18+24},{3,18+19}} };
static GPathInfo path_2 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,36+19},{135,36+24}, {8,36+24},{3,36+19}} };	
static GPathInfo path_3 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,54+19},{135,54+24}, {8,54+24},{3,54+19}} };	
static GPathInfo path_4 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,72+19},{135,72+24}, {8,72+24},{3,72+19}} };	
static GPath *s_path_1;
static GPath *s_path_2;
static GPath *s_path_3;
static GPath *s_path_4;



// ------------------------------------- Game start ------------------------

static STEP * findStep(int id){	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "find Step with Id: %d", id);}
	STEP * out = NULL;
	for(int x = 0; x < g.step_count; x++){
		if(id == g.step[x].id){
			out = &g.step[x];
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Step FOUND at: %d", x);}
			break;
		}
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "NOT FOUND");
	return out;
}

static void setWakeup(TYPE reason, int min){
	wakeup_cancel_all();
	int addHour = 0;
	//time_t now = time(NULL);            //load the time
	time_t wakeup_time = time(NULL) + min * 60;			//time in min
	
	//respect the sleeping time
	if(settings.sleep_time){
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "check Wakeup time is in sleeping time:");}
		struct tm *t = localtime(&wakeup_time);
  	int hour = t->tm_hour;
		
		if(hour >= 22){
			addHour = (32 - hour) * 60 * 60; //h* 60min * 60sec = secounds
			
		}			
		if(hour <= 8){
			addHour = (8 - hour) * 60 * 60; 
		}
		wakeup_time = time(NULL) + (min * 60) + addHour;
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Wakeup - AddHour: %d ", addHour/60/60);}
	}
	
	//rapid mode
	if(settings.rapid_mode){
		//wakeup_time = time(NULL) + 1;
	}
	
	//first wakeup
	wakeup_schedule(wakeup_time, reason, true);	
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Wakeup - time: %d (min)", min);}
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Wakeup - reason: %d", settings.wakeup_reason);}
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Wakeup - now time: %ld", time(NULL));}
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Set Wakeup - wakeup time: %ld", wakeup_time);}
	
	//gentle reminder
	wakeup_time = time(NULL) + (min * 60) + (addHour) + (60 * 60 * 2);			//2h later
	wakeup_schedule(wakeup_time, reason, false);
	
		wakeup_time = time(NULL) + (min * 60) + (addHour) + (60 * 60 * 6);			//6h later
	wakeup_schedule(wakeup_time, reason, false);
	
	wakeup_time = time(NULL) + (min * 60) + (addHour) + (60 * 60 * 24);			//24h later
	wakeup_schedule(wakeup_time, reason, false);
	
	//set wakeup reason
	settings.wakeup_reason = reason;	//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer

	//save settings
	//save_settings();
}

static void setMilestone(STEP *step){
	//find a free slot in the history and save the step id
/*
	int index = 0;
	for(int x = 0; x < MAX_MILESTONE; x++){
		if(settings.history[x] < 0){
			index = x;
			settings.history[x] = step->id;
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVE Milestone ID: %d @ slot: %d", step->id, x);}
			break;
		}
	}
	*/
} //TODO

static void setNextStep(int id){
	//int out = -1;	
	if (id < 1){ //startet mit ID 1
		id = 1;
	}
	
	/*
	settings.next_step = NULL;
	// old find Step
	for(int x = 0; x < g.step_count; x++){
		if(id == g.step[x].id){
			//out = g.step[x].id;
			settings.next_step = &g.step[x];		
			break;
		}
	}
	*/
	
	settings.next_step = findStep(id);
}

static void addText(STEP *step){
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText mit id: %d Textcount: %d", step->id, settings.active_text_count);
	if(settings.active_text_count+1 < MAX_TEXT){
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText - to array");
		settings.active_text[settings.active_text_count] = step;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText - count++");
		settings.active_text_count++;
		//MSG counter erhöhen für game stats
		if(settings.active_text_count < 3){
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText - select++");
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu index row %d section %d", settings.menu_index.row, settings.menu_index.section);
			menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,settings.active_text_count), MenuRowAlignCenter, true);
		}
	}else{
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: to many Text Messages in the Menu");}
		//erste Nachrichten löschen (Like a fifo) + Menu_select--
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText end - update menu_layer");
	//update menu
	menu_layer_reload_data(s_menu_layer);
}

static void addAntw(STEP *step){
		if(settings.active_antw_count+1 < MAX_ANTW){
		settings.active_antw[settings.active_antw_count] = step;
		settings.active_antw_count++;
		//MSG counter erhöhen
	}else{
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: to many Options (Answers) in the Menu");}
	}
	
	//update menu
	menu_layer_reload_data(s_menu_layer);
}

static void findAllAntw(int id){
	settings.active_antw_count = 0;
	for(int x = 0; x < g.step_count; x++){
		if(id == g.step[x].id ){
			if(g.step[x].type == ANTW){
				addAntw(&g.step[x]);
			}
		}
	}
}

static void clear_menu(){
	settings.active_text_count = 0;
	settings.active_antw_count = 0;
	
	menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,0), MenuRowAlignCenter, false);
	//menu_layer_reload_data(s_menu_layer);	
}

//-------------------------------- save - load settings -------------------

void save_settings() {	
	
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "save savegame");
	
	//settings_to_savegame;
	//savegame.active_text = {0};
	for(int x = 0; x < MAX_TEXT; x++){
		if(x < settings.active_text_count){
			savegame.active_text[x] = settings.active_text[x]->id;
			APP_LOG(APP_LOG_LEVEL_DEBUG, "save text id: %d", savegame.active_text[x]);
		}else{
			savegame.active_text[x] = 0;
		}
	}
	savegame.active_text_count = settings.active_text_count;
	
	//save antw ID
	if(settings.active_antw_count > 0){
		savegame.active_antw = settings.active_antw[1]->id;
	}else{
		savegame.active_antw = 0;
	}
	
	//save milestones
	for(int x = 0; x < MAX_MILESTONE; x++){
		if(settings.history[x] > 0){
			savegame.history[x] = settings.history[x];
		}else{
			savegame.history[x] = 0;
		}
	}
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "save text_count: %d", savegame.active_text_count);
	//savegame.active_antw_count = settings.active_antw_count;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "save antw id: %d", savegame.active_antw);
	
	if(settings.next_step != NULL){
		savegame.next_step 			 = settings.next_step->id;
	}else{
		savegame.next_step 			 = 0;
	}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "save next_step id: %d", savegame.next_step);
	savegame.wakeup_reason 		 = settings.wakeup_reason;
	
	//gamesettings
	savegame.vibe 			= settings.vibe;
	savegame.rapid_mode = settings.rapid_mode;
	savegame.sleep_time = settings.sleep_time;
	savegame.menu_index = settings.menu_index;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu Index row: %d, section; %d", savegame.menu_index.row, savegame.menu_index.section);
	
	save_savegame();	
	
	//save data	
	//persist_write_data(SETTINGS_KEY, &settings, sizeof(settings)); //old methode - save settings directly

	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVED");}
} //TODO: save achievments

void load_settings() {  
	//old methode - reads the settings directly
	/*
	if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == (int)sizeof(settings)){
		//load data
		persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	
		
		//savegame_to_settings;
	}else{
		//write the defaults to settings
		//savegame_to_settings; 
		//save the first time
		//save_savegame();
	}
	*/
	// new methode
	
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Load savegame");
	load_savegame();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "loaded");
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu Index row: %d, section; %d", savegame.menu_index.row, savegame.menu_index.section);
	
	//texte
	settings.active_text_count = 0;
	for(int x = 0; x < savegame.active_text_count; x++){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "load Text #%d mit ID: %d ", x, savegame.active_text[x]);
		settings.active_text[x] = findStep(savegame.active_text[x]);
		settings.active_text_count++;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Settings %d hat id: %d", x, settings.active_text[x]->id);
	}
	
	//antw
	settings.active_antw_count = 0;
	if(savegame.active_antw > 0){
		//find all Antw with id
		for(int x = 0; x < g.step_count; x++){
			if(savegame.active_antw == g.step[x].id ){
				if(g.step[x].type == ANTW){
					//addAntw(&g.step[x]);
					settings.active_antw[settings.active_antw_count] = &g.step[x];
					settings.active_antw_count++;
				}
			}
		}
	}
	
	//milestones
	for(int x = 0; x < MAX_MILESTONE; x++){
		//if(savegame.history[x] > 0){
			settings.history[x] = savegame.history[x];
		//}
	}
	
	//set next step	
	if(savegame.next_step > 0){
		setNextStep(savegame.next_step);
	}else{
		settings.next_step = NULL;
	}
	settings.wakeup_reason = savegame.wakeup_reason;
		//gamsesettings
	settings.vibe 			= savegame.vibe;
	settings.rapid_mode = savegame.rapid_mode;
	settings.sleep_time = savegame.sleep_time;
	settings.menu_index = savegame.menu_index; 
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Menu Index row: %d, section; %d", settings.menu_index.row, settings.menu_index.section);
} //TODO: load achievments

// ------------------------------------ Game Main Function ----------------

//game action & timer callback
static void game_action(void *data){
	STEP *step = settings.next_step;
	int timer 	= 0;
	bool go_on 	= true;
	bool update_menu = false;
	//if nextID = 0 (auto count up)
	if (step->next == 0){
		step->next = step->id+1;
	}	
	
	if(!settings.rapid_mode){
		if(settings.wakeup_reason == TEXT || settings.wakeup_reason == ANTW){
			//do nothing while waiting for wakeup
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Waiting for the next Step. WAKEUP");}
			return;	
		}
	}
	if(step == NULL){
		//Error next step not found
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Next step not found. Step is NULL");}
		return;	
	}
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Prozessing Step.ID: %d.", step->id);}
	
	switch(step->type){
		case INFO:
			//is the same like a text
		//break;
		case TEXT:

			addText(step);
			timer = step->delay;
			update_menu = true;
			setNextStep(step->next);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Next Step: %d", step->next); 
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free()); 
		break;
		case ANTW:

			findAllAntw(step->id);
			settings.next_step = NULL;
			update_menu = true;
			go_on = false;
		
			setWakeup(ANTW, 30);	//set reminder
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set wakeup  ANTW - id: %d", settings.wakeup_reason);}
		break;
		
		case DIE_AKKU:
			wakeup_cancel_all();
			//ToDo  save achievment 1. Todesfolge gefunden
			setNextStep(step->next);
		break;
		
		case MILESTONE:
			setNextStep(step->next);	//set next step and go on
			setMilestone(step);				//save this milestone
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set Milestone");}
		break;		
		
		case SPECIAL:
			
			clear_menu();
			//active_text_count = 0;
			settings.active_text[settings.active_text_count] = step;
			settings.active_text_count++;
			
			update_menu = true;
			//go_on = false;
			setNextStep(step->next);
			
			if(debug){ 
				APP_LOG(APP_LOG_LEVEL_DEBUG, "You Won the first Game"); 
			}
		break;
		default: 
		
		break;
	}	
	
	//update the menu layer
	if(update_menu){menu_layer_reload_data(s_menu_layer);}
	
	if(settings.rapid_mode){
		timer = 0;
	}
	
	if(go_on){
		//start the timer
		if(timer > 4){ //extra long timer with wakeup
			setWakeup(TEXT, timer);
		}else	if(timer>0){																	//time in sec
			s_timer = app_timer_register(timer*1000, game_action, NULL);
		}else{																							//now
			//short Timer
			s_timer = app_timer_register(10, game_action, NULL);
		}
	}
	
}

static void wakeup_handler() {
  
	wakeup_cancel_all();
	if (settings.vibe){
		vibes_double_pulse();	
		vibes_double_pulse();
	}	
	
	//reset the wakeup reason
	settings.wakeup_reason = -1;
	
	window_stack_pop_all(false);
  window_stack_push(s_info_window, true);
	
	//game_action(NULL);
}

//menu select
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	STEP *antw;
	switch (cell_index->section){
		case 0:	//Empfangs Items
			window_stack_push(s_menu_window, true);		//open the settings menu
		break;
		case 1: //Antwort Items
			clear_menu();
			setWakeup(TEXT, 60);	//set a reminder. 
			
			antw = settings.active_antw[cell_index->row];
		  addText(antw);
		 	setNextStep(antw->next);
			
			//zwischenstand speichern
			save_settings();
		
			APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: Go to next Step with ID: %d", antw->next);
		
			/*
			if(settings.wakeup_id >= 0){
				wakeup_cancel(settings.wakeup_id);				
				settings.wakeup_id = -1;
				wakeup_scheduled = false;
			}
			*/
			settings.wakeup_reason = -1;
		
			//clear_menu();
			//addText(buffer);
			game_action(NULL);
		
			/*
			switch (cell_index->row) {				
				case 0:
					//if(settings.active_antw_count > 0){
						addText(settings.active_antw[0]);
						setNextStep(settings.active_antw[0]->next);
						game_action(NULL);
					//}
      	break;
				case 1:
					//if(settings.active_antw_count > 1){
						addText(settings.active_antw[1]);
						setNextStep(settings.active_antw[1]->next);
						game_action(NULL);
					//}
				break;
				default: break;
  		}
			*/
		break;
		default: break;
	}
}

// ------------------------------------ Timeline Menu Layer callbacks --------------- jear 

static uint16_t get_text_hight(STEP *step){
	
	TextLayer *text_layer;
	
	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	
	if(step->type == INFO || step->type == ANTW){
		font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
	}
	
	//get the size 
	text_layer = text_layer_create(GRect(0, 0, 128, 500));
	text_layer_set_text(text_layer, step->text);	
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(text_layer, font);	
	GSize size = text_layer_get_content_size(text_layer);
	
	text_layer_destroy(text_layer);
	
	//if(size.h < 37){size.h = 37;}
	
	return size.h;	
	//return 20;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	if(debug){ APP_LOG(APP_LOG_LEVEL_DEBUG, "Anzahl section: %d", (settings.active_text_count > 0 ? 1 : 0) + (settings.active_antw_count > 0 ? 1 : 0)); }
	//return (settings.active_text_count > 0 ? 1 : 0) + (settings.active_antw_count > 0 ? 1 : 0);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "num_section");}
	if(settings.active_antw_count > 0 || settings.wakeup_reason > 0){
		return 2;
	}else{
		return 1;
	}
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "num row");}
	switch (section_index) {
    case 0:
      //if(debug){ APP_LOG(APP_LOG_LEVEL_DEBUG, "Anzahl Texte: %d", settings.active_text_count);}
			return settings.active_text_count;
    case 1:
			//if(debug){ APP_LOG(APP_LOG_LEVEL_DEBUG, "Anzahl Antworten: %d", settings.active_antw_count);}
      return settings.active_antw_count;
    default:
      return 0;
  }
}

static int16_t 	menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void 		menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw header");}
	
	// Determine which section we're working with
  switch (section_index) {
    case 0:
        // Draw title text in the section header
        menu_cell_basic_header_draw(ctx, cell_layer, "Nachrichten:");
      break;
    case 1:
        // Draw title text in the section header
				if(settings.active_antw_count > 0){
					menu_cell_basic_header_draw(ctx, cell_layer, "Senden...");	
				}else{
					if(settings.wakeup_reason != TEXT){
						menu_cell_basic_header_draw(ctx, cell_layer, "Empfange Daten...");
					}else{
						menu_cell_basic_header_draw(ctx, cell_layer, "Warte auf Daten");
					}
				}
        
      break;
  }
}

static int16_t 	menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> cell height section: %d row: %d", cell_index->section, cell_index->row);}
	//return 18+26;

	switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)
		
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> step text id %d", settings.active_text[cell_index->row]->id);}
			return get_text_hight(settings.active_text[cell_index->row])+26;
	
		break;
		case 1:		//zweite Section (Antworten)
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> step antw id %d", settings.active_antw[cell_index->row]->id);}
			return get_text_hight(settings.active_antw[cell_index->row])+26;
		break;
		default: return 0;
	}
}

static void 		menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
 
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw row");}
	
	STEP *step; 
	int size=0;
	
	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	GPath *s_path = s_path_1;
	
	graphics_context_set_fill_color(ctx, timeline_box_fill_color);
  graphics_context_set_stroke_color(ctx, timeline_box_border_color);
	graphics_context_set_stroke_width(ctx, 2);
	//gpath_move_to(s_path, GPoint(0,0));
	
	// Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)    	
			
			step = settings.active_text[cell_index->row];
			size = get_text_hight(step);
		
			if(step->type == INFO || step->type == ANTW){
				font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
			}
		
			if(size>18){s_path = s_path_2;}
			if(size>36){s_path = s_path_3;}
			if(size>54){s_path = s_path_4;}		
		
			//draw background
			#ifdef PBL_COLOR
			gpath_draw_filled(ctx, s_path);
			#endif
			gpath_draw_outline(ctx, s_path);			
		
			graphics_draw_text(ctx, 
				step->text, 
				font,
				GRect(8, 12, 128, 100), 
				GTextOverflowModeWordWrap, 
				GTextAlignmentCenter, 
				NULL);			
		
      break;
    case 1:			//zweite Section (Antworten)
     
			step = settings.active_antw[cell_index->row];
			size = get_text_hight(step);
		
			if(step->type == INFO || step->type == ANTW){
				font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
			}
		
			if(size>18){s_path = s_path_2;}
			if(size>36){s_path = s_path_3;}
			if(size>54){s_path = s_path_4;}	
			
			//highlight selected row
			if(menu_cell_layer_is_highlighted(cell_layer)){
				#ifdef PBL_COLOR
					graphics_context_set_stroke_color(ctx, GColorRed);
				#else
					graphics_context_set_stroke_width(ctx, 3);
				#endif				
			}else{	//not selected
				#ifdef PBL_COLOR
					graphics_context_set_stroke_color(ctx, timeline_box_border_color);	
				#else
					graphics_context_set_stroke_width(ctx, 1);
				#endif				
			}
			
			#ifdef PBL_COLOR
			gpath_draw_filled(ctx, s_path);
			#endif
			gpath_draw_outline(ctx, s_path);
			
			graphics_draw_text(ctx, 
				step->text, 
				font,
				GRect(8, 12, 128, 100), 
				GTextOverflowModeWordWrap, 
				GTextAlignmentCenter, 
				NULL);
  }
}

/*
static void     menu_draw_separator_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context){
	
}

static uint16_t	menu_get_separator_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
	return 2;
}

static void  		menu_draw_background_callback(GContext *ctx, const Layer *bg_layer, bool highlight, void *callback_context){
 
	//for a static Timeline Background
	GPath *s_current_path = gpath_create(&path_1);
	
	graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorDukeBlue, GColorBlack));
  graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 2);
	
	gpath_move_to(s_current_path, GPoint(0,3));
	gpath_draw_outline(ctx, s_current_path);
	//gpath_draw_filled(ctx, s_current_path);
	
} 
*/ //draw separator

static void main_window_load(Window *window) {
  
	//timeline_active = true;
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start timeline");}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "size settings: %d", persist_get_size(SETTINGS_KEY));
	
	s_path_1 = gpath_create(&path_1);
	s_path_2 = gpath_create(&path_2);
	s_path_3 = gpath_create(&path_3);
	s_path_4 = gpath_create(&path_4);
	
	//s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_1);
	
  // Now we prepare to initialize the menu layer
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set timeline callbacks");}
	
  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections 	= menu_get_num_sections_callback,
    .get_num_rows 			= menu_get_num_rows_callback,
    .get_header_height 	= menu_get_header_height_callback,
    .draw_header 				= menu_draw_header_callback,
    .draw_row 					= menu_draw_row_callback,
    .select_click 			= menu_select_callback,
    .get_cell_height 		= menu_get_cell_height_callback
		//.draw_background 		= menu_draw_background_callback,
		//.get_separator_height=menu_get_separator_height_callback,
		//.draw_separator			= menu_draw_separator_callback
  });
		
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "timeline callbacks ready");}
	
	//Menu config
	menu_layer_set_normal_colors(s_menu_layer, GColorClear, timeline_text_color);
	menu_layer_set_highlight_colors(s_menu_layer, GColorClear, timeline_text_color);
	
	//start the game
	//setNextStep(0);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set menu index");}
	menu_layer_set_selected_index(s_menu_layer, settings.menu_index, MenuRowAlignCenter, false);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start game action");}
	game_action(NULL);
		
  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
	
	settings.menu_index = menu_layer_get_selected_index(s_menu_layer);
	
	// Destroy the menu layer
	//layer_destroy(s_info_layer);
  menu_layer_destroy(s_menu_layer);
	//text_layer_destroy(s_info_text);
	
	gpath_destroy(s_path_1);
	gpath_destroy(s_path_2);
	gpath_destroy(s_path_3);
	gpath_destroy(s_path_4);
}

// ------------------------------------ Settings Menu Layer --------------

static void 		set_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	switch (cell_index->section){
		case 0:	//Settings
			switch (cell_index->row){
				case 0: //vibe
					settings.vibe = !settings.vibe;
					if(settings.vibe){
						vibes_short_pulse();
					}
					menu_layer_reload_data(menu_layer);
				break;
				case 1:	//rapid mode
					settings.rapid_mode = !settings.rapid_mode;					
					menu_layer_reload_data(menu_layer);
					game_action(NULL);
				break;				
				case 2:	//Ruhezeit
					settings.sleep_time = !settings.sleep_time;
					menu_layer_reload_data(menu_layer);
				break;
				case 3:	//neustart
					setNextStep(0);
					wakeup_cancel_all();
					settings.wakeup_reason = -1;
				
					clear_menu();
					window_stack_pop_all(true);
					window_stack_push(s_welc_window, true);
				break;
				case 4:	//reset data
					
				break;
			}			
		
		break;
		case 1: //Antwort Items		
			/*
			switch (cell_index->row) {				
				case 0:
					
      	break;
				case 1:
					
				break;
				default: break;
  		}
			*/
		break;
		case 2: 
			window_stack_push(s_cred_window, true);
		break;
		default: break;
	}
}

static uint16_t set_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	return 3;
}

static uint16_t set_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:			
			//settings
			return 4;
    case 1:
			//achievments
      return 2;
		case 2:
			//credits
			return 1;
		break;
    default:
      return 0;
  }
}

static int16_t 	set_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void 		set_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
        // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Einstellungen:");
      break;
    case 1:
        // Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "Erfolge:");        
    	break;
		case 2:
			menu_cell_basic_header_draw(ctx, cell_layer, "Extras:");        
		break;
  }
}

static void 		set_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {

	// Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)    	
			switch (cell_index->row) {
				case 0: 
					menu_cell_basic_draw(ctx, cell_layer, "Vibration", (settings.vibe ? "on" : "off"), NULL);
				break;
				case 1:
					menu_cell_basic_draw(ctx, cell_layer, "Rapid Mode", (settings.rapid_mode ? "on" : "off"), NULL);
				break;
				case 2:
					menu_cell_basic_draw(ctx, cell_layer, "Ruhezeit", (settings.sleep_time ? "22:00-8:00" : "off"), NULL);
				break;
				case 3:
					menu_cell_basic_draw(ctx, cell_layer, "Neustart", NULL, NULL);
				break;
				case 4:
					menu_cell_basic_draw(ctx, cell_layer, "Reset", NULL, NULL);
				break;
			}
      break;
    case 1:			//zweite Section (Antworten)
    	switch (cell_index->row){
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "1 / 2", "Erfolge gefunden", NULL);		
				break;
				case 1: 
					menu_cell_basic_draw(ctx, cell_layer, "2 / 7", "Tode gefunden", NULL);
			}
		break;
		case 2:
			switch (cell_index->row){
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "Credits", NULL, NULL);
				break;
				case 1:
					//menu_cell_basic_draw(ctx, cell_layer, g.version, "Version", NULL);
				break;
			}
		break;
  }
}

static int16_t  set_get_separator_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_contex){
	return 0;	
}

static void 		set_window_load(Window *window) {
  
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start setting menu");}
	
  // Now we prepare to initialize the menu layer
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
	s_set_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_set_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections 	= set_get_num_sections_callback,
    .get_num_rows 			= set_get_num_rows_callback,
    .get_header_height 	= set_get_header_height_callback,
    .draw_header 				= set_draw_header_callback,
    .draw_row 					= set_draw_row_callback,
    .select_click 			= set_select_callback,
    //.get_cell_height 		= menu_get_cell_height_callback,
		//.draw_background 		= menu_draw_background_callback,
		.get_separator_height=set_get_separator_height_callback
		//.draw_separator			= menu_draw_separator_callback
  });
	
	menu_layer_set_normal_colors(s_set_layer, settings_bg_color, settings_text_color);
	menu_layer_set_highlight_colors(s_set_layer, settings_bg_color_hi, settings_text_color_hi);
	
	menu_layer_set_click_config_onto_window(s_set_layer, window);
	
  layer_add_child(window_layer, menu_layer_get_layer(s_set_layer));
}

static void 		set_window_unload(Window *window) {
	
	// Destroy the menu layer
  menu_layer_destroy(s_set_layer);
}


// ------------------------------------ Welcome Layer callbacks ---------------

void welc_select_click_handler(ClickRecognizerRef recognizer, void *context){
	window_stack_remove(s_welc_window,false);
	window_stack_push(s_main_window, true);	
}

void welc_click_config_provider() {
  
	window_single_click_subscribe(BUTTON_ID_SELECT, welc_select_click_handler);
	//window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void welc_window_load(Window *window) {
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start welcome window");}
	
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_bounds = GRect(0, 0, 144, 2000);
	
	s_welc_layer = scroll_layer_create(bounds);
	
	scroll_layer_set_click_config_onto_window(s_welc_layer, window);
	scroll_layer_set_callbacks(s_welc_layer, (ScrollLayerCallbacks){
		.click_config_provider = welc_click_config_provider
	});
	
	window_set_background_color(s_welc_window, welcome_bg_color);
	
	//draw Layer inputs	
	s_welc_text1 = text_layer_create(GRect(0, 0, max_bounds.size.w, 100));
	text_layer_set_text(s_welc_text1, g.name);	//Headline
  text_layer_set_font(s_welc_text1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_background_color(s_welc_text1, GColorClear);
	text_layer_set_text_color(s_welc_text1, welcome_text_color);
	text_layer_set_text_alignment(s_welc_text1, GTextAlignmentCenter);	

	s_welc_text2 = text_layer_create(GRect(5,60, max_bounds.size.w-10, max_bounds.size.h));
	text_layer_set_text(s_welc_text2, g.desc);			//description text
  text_layer_set_font(s_welc_text2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_welc_text2, GTextAlignmentLeft);	
	text_layer_set_background_color(s_welc_text2, GColorClear);
	text_layer_set_text_color(s_welc_text1, welcome_text_color);
	GSize size = text_layer_get_content_size(s_welc_text2);
	text_layer_set_size(s_welc_text1, GSize(144,size.h+10));
	
	//resize the scroll layer
	scroll_layer_set_content_size(s_welc_layer, GSize(144, size.h + 60 + 10));
	
	//add text to the scroll layer
	scroll_layer_add_child(s_welc_layer, text_layer_get_layer(s_welc_text1));	
	scroll_layer_add_child(s_welc_layer, text_layer_get_layer(s_welc_text2));	
	
	//add scroll layer to the window
	layer_add_child(window_layer, scroll_layer_get_layer(s_welc_layer));
}

static void welc_window_unload(Window *window) {
	scroll_layer_destroy(s_welc_layer);
	
	text_layer_destroy(s_welc_text1);
	text_layer_destroy(s_welc_text2);
}

// ------------------------------------ Info Layer callbacks ---------------

void info_click_handler(){
	window_stack_remove(s_info_window,false);
	window_stack_push(s_main_window, true);		
}

void info_click_config_provider(){
	window_single_click_subscribe(BUTTON_ID_SELECT, info_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, info_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, info_click_handler);
  //window_single_click_subscribe(BUTTON_ID_BACK, info_click_handler);
}

static void info_window_load(Window *window) {
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start info window");}
	
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  //GRect max_bounds = GRect(0, 0, 130, 2000);
	
	//graphics_draw_bitmap_in_rect(ctx, s_msg_background, bounds);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "rand %d", rand() %3 );}
	switch(rand() %3){
		case 0: s_background_image = gbitmap_create_with_resource(RESOURCE_ID_MSG_BG1); 	break;
		case 1: s_background_image = gbitmap_create_with_resource(RESOURCE_ID_MSG_BG2); 	break;
		case 2: s_background_image = gbitmap_create_with_resource(RESOURCE_ID_MSG_BG3); 	break;
		default: s_background_image = gbitmap_create_with_resource(RESOURCE_ID_MSG_BG1); 	break;
	}
	
  s_background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(s_background_layer, s_background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
	
	
	s_info_layer = layer_create(bounds);
	
	window_set_click_config_provider(window, (ClickConfigProvider) info_click_config_provider);	
	
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "show: %d", (int)settings.next_step->id);}
	
	//draw Layer inputs	
	s_info_text = text_layer_create(GRect(7, 50, 137, 100));
	text_layer_set_text(s_info_text, g.text_msg);	//Headline incoming Msg
	if(settings.next_step == NULL){
		text_layer_set_text(s_info_text, g.text_wait);	//Headline waiting for answer
	}
	text_layer_set_background_color(s_info_text, GColorClear);
	text_layer_set_text_color(s_info_text, GColorBlack);
  text_layer_set_font(s_info_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(s_info_text, GTextAlignmentCenter);		
	
	//add text to the scroll layer
	layer_add_child(s_info_layer, text_layer_get_layer(s_info_text));		
	
	//add layer to the window
	layer_add_child(window_layer, s_info_layer);
}

static void info_window_unload(Window *window) {
	save_settings();
	
	bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_image);
	
	layer_destroy(s_info_layer);
	
	text_layer_destroy(s_info_text);
}

// ------------------------------------ Credits Layer callbacks ---------------


void cred_select_click_handler(ClickRecognizerRef recognizer, void *context){
	window_stack_remove(s_welc_window,false);
	window_stack_push(s_main_window, true);	
}

void cred_click_config_provider() {
  
	//window_single_click_subscribe(BUTTON_ID_SELECT, credits_select_click_handler);
	//window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void cred_window_load(Window *window) {
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start welcome window");}
	
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_bounds = GRect(0, 0, 139, 2000);
	
	s_welc_layer = scroll_layer_create(bounds);
	
	scroll_layer_set_click_config_onto_window(s_welc_layer, window);
	scroll_layer_set_callbacks(s_welc_layer, (ScrollLayerCallbacks){
		.click_config_provider = cred_click_config_provider
	});
	
	window_set_background_color(s_cred_window, credits_bg_color);
	
	//draw Layer inputs	
	s_welc_text1 = text_layer_create(GRect(0, 0, max_bounds.size.w, 100));
	text_layer_set_text(s_welc_text1, g.name);	//Headline
  text_layer_set_font(s_welc_text1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(s_welc_text1, GTextAlignmentCenter);	
	text_layer_set_background_color(s_welc_text1, GColorClear);
	text_layer_set_text_color(s_welc_text1, credits_text_color);

	s_welc_text2 = text_layer_create(GRect(5,60, max_bounds.size.w-20, max_bounds.size.h));
	text_layer_set_text(s_welc_text2, g.credits);			//description text
  text_layer_set_font(s_welc_text2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_welc_text2, GTextAlignmentLeft);	
	text_layer_set_background_color(s_welc_text2, GColorClear);
	text_layer_set_text_color(s_welc_text2, credits_text_color);
	GSize size = text_layer_get_content_size(s_welc_text2);
	text_layer_set_size(s_welc_text1, GSize(144,size.h+10));
	
	//resize the scroll layer
	scroll_layer_set_content_size(s_welc_layer, GSize(144, size.h + 60 + 10));
	
	//add text to the scroll layer
	scroll_layer_add_child(s_welc_layer, text_layer_get_layer(s_welc_text1));	
	scroll_layer_add_child(s_welc_layer, text_layer_get_layer(s_welc_text2));	
	
	//add scroll layer to the window
	layer_add_child(window_layer, scroll_layer_get_layer(s_welc_layer));
}

static void cred_window_unload(Window *window) {
	scroll_layer_destroy(s_welc_layer);
	
	text_layer_destroy(s_welc_text1);
	text_layer_destroy(s_welc_text2);
}

// -------------------------------------- Start and Close the App ----------

static bool is_step_real(int id){
	bool out = false;
	
	for(int i = 0; i < g.step_count; i++){
		if(g.step[i].id == id){
			out = true;
		}
	}
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "check Step id: %d -> %d", id, out);}
	
	return out;
}

void checkSteps(){
	//const int i = g.step_count;
	//int stepID[200] = {-1};
	//bool nextID[200] = {false};
	//bool call[200]	 = {false};
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "check Steps start");
	
	for(int i = 0; i < g.step_count; i++){
		//stepID[i] = g.step[i].id;
		
		//nextID[i] = 
		if(!is_step_real(g.step[i].next)){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "check Step id: %d -> %d", g.step[i].id, g.step[i].next);
		}		
	}
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "check Steps End");	
}

static void init() {
	//persist_delete(SETTINGS_KEY);
	
	srand(time(NULL));
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "rand %d", rand() %3 );}
	
	//load data
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD SAVEGAME");}
	load_settings();	
	
	//check the steps
	if(StepCheck){checkSteps();}	
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup reason in Persistent: %d", settings.wakeup_reason);}
	
	//init timeline
  s_main_window = window_create();	//timeline
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load 	= main_window_load,
    .unload = main_window_unload,
  });
  //window_stack_push(s_main_window, true);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: timeline");}
	
	//init welcome
	s_welc_window = window_create();	//welcome
  window_set_window_handlers(s_welc_window, (WindowHandlers){
    .load 	= welc_window_load,
    .unload = welc_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: welcome");}
	
	//init info/msg layer
	s_info_window = window_create();	//info (incoming Msg / waiting for answer)
  window_set_window_handlers(s_info_window, (WindowHandlers){
    .load 	= info_window_load,
    .unload = info_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: infocard");}
	
	//init settings menu
	s_menu_window = window_create();	//settings_menu
  window_set_window_handlers(s_menu_window, (WindowHandlers){
    .load 	= set_window_load,
    .unload = set_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: settings");}
	
	//init credits layer
	s_cred_window = window_create();
  window_set_window_handlers(s_cred_window, (WindowHandlers){
    .load 	= cred_window_load,
    .unload = cred_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: credits");}

	// subscribe to wakeup service to get wakeup events while app is running
	wakeup_service_subscribe(wakeup_handler);
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "launche_reason: %d", launch_reason());}
	
  if(launch_reason() == APP_LAUNCH_WAKEUP) { //App start from wakeup

		//if (wakeup_get_launch_event(&id, &reason)) {
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START wakeup handler");}
    
		wakeup_handler();
		//window_stack_push(s_info_window, true);
    //}
  //} else if (wakeup_scheduled) {
    //window_stack_push(s_info_window, true);
  } else {																		//normal app system start

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START normal start");}
		
		if(settings.next_step != NULL && settings.active_text_count == 0){
			
			if(settings.next_step->id <= 1){ 		//first run
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Welcome window...");}
			
				wakeup_cancel_all();
				settings.wakeup_reason = -1;				
			
				window_stack_push(s_welc_window, true);	
			}else{													//show currend game
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window");}
				window_stack_push(s_main_window, true);	
			}
	
		}else{
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window (next_step == NULL)");}
			window_stack_push(s_main_window, true);	
		}
				
  }  
}

static void deinit() {
	save_settings();
	
  window_destroy(s_main_window);
	window_destroy(s_welc_window);
	window_destroy(s_info_window);
	window_destroy(s_menu_window);
	window_destroy(s_cred_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
