#include "pebble.h"
#include "main.h"
#include "story.h"

#undef RAND_MAX
#define RAND_MAX 	3

/*	
	Round support
	https://github.com/pebble-examples/feature-menu-layer/blob/master/src/feature_menu_layer.c

	Bug:
		- Backgound image for Basalt (function: info_window_load()) -> load text from resource
		- calc hight wird immer zweimal berechnet...
		

	Story!!!
	
	Multi lang support
		- EN
		- DE
	
	better debuging 
		- debug && debug_settings 
	
	Special:		
		- Fake System Data in Settings
			* Empfang
			* Akku
			* Empfangene Nachrichten
			
		-Welcome Card:
			* add action bar. (up/down/ play)
	
	Idee: Gewählte Anwort mit in Timeline aufnehmen
			*	ältere Elemente immer löschen (First in, first out que)
			* que in Add Text möglich (clear Menu anpassen/ nur noch antw count = 0)(selced index anpassen --)
*/ //TODOs

#define debug 		0				//debug output on or off
#define TESTMODE 	0				//in testmode you ust wait a few secounds for the next message
#define StepCheck 0 			//check all steps to have a next step. -> output to console
#define	errorId 	2000		//ID to show a error

#define debug_timeline 0
#define debug_settings 0

// more game settings are in the main.h

//global Layer
static Layer 			*window_layer;

static Window 		*s_main_window;		//timeline
static Window 		*s_welc_window;		//welcome
static Window 		*s_info_window;		//info (Incomming Msg / waiting for input)
static Window 		*s_menu_window;		//settings menu
static Window			*s_mile_window;		//milestone menu
//static Window 		*s_cred_window;		//credits 

static MenuLayer 	*s_menu_layer;		//timeline_menu
static MenuLayer 	*s_set_layer;			//settings menu layer
static MenuLayer 	*s_mil_layer;			//milestone menu layer

static ScrollLayer*s_welc_layer;		//welcome layer
static TextLayer 	*s_welc_text1;
static TextLayer 	*s_welc_text2;

static TextLayer 	*s_info_text;			//info layer 
static TextLayer 	*s_info_text2;			
static Layer 			*s_info_layer;


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
	uint8_t font;
	
	//history
	int history[MAX_MILESTONE];				//ID of the milestones
	bool found_deads[MAX_DEADS];	//ID of the Achievments
	bool found_alives[MAX_ALIVES];
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
	.font = 0,
	
	//history
	.history = {0},
	.found_deads = {false},
	.found_alives = {false}
	//.history_count = 0
};

#if PBL_ROUND 
static GPathInfo path_1 = { .num_points = 8,.points = (GPoint []) {	{20,10},{25,5}, {155,5},{160,10}, {160,18+19},{155,18+24}, {25,18+24},{20,18+19}} };
static GPathInfo path_2 = { .num_points = 8,.points = (GPoint []) {	{20,10},{25,5}, {155,5},{160,10}, {160,36+19},{155,36+24}, {25,36+24},{20,36+19}} };	
static GPathInfo path_3 = { .num_points = 8,.points = (GPoint []) {	{20,10},{25,5}, {155,5},{160,10}, {160,54+19},{155,54+24}, {25,54+24},{20,54+19}} };	
static GPathInfo path_4 = { .num_points = 8,.points = (GPoint []) {	{20,10},{25,5}, {155,5},{160,10}, {160,72+19},{155,72+24}, {25,72+24},{20,72+19}} };
static GPathInfo path_5 = { .num_points = 8,.points = (GPoint []) {	{20,10},{25,5}, {155,5},{160,10}, {160,90+19},{155,90+24}, {25,90+24},{20,90+19}} };	
#else		//Aplite & Basalt
static GPathInfo path_1 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,18+19},{135,18+24}, {8,18+24},{3,18+19}} };
static GPathInfo path_2 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,36+19},{135,36+24}, {8,36+24},{3,36+19}} };	
static GPathInfo path_3 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,54+19},{135,54+24}, {8,54+24},{3,54+19}} };	
static GPathInfo path_4 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,72+19},{135,72+24}, {8,72+24},{3,72+19}} };
static GPathInfo path_5 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,90+19},{135,90+24}, {8,90+24},{3,90+19}} };
#endif
static GPath *s_path_1;
static GPath *s_path_2;
static GPath *s_path_3;
static GPath *s_path_4;
static GPath *s_path_5;


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

	if(TESTMODE){
		min = 1;
	}

	time_t wakeup_time = time(NULL) + min * 60;			//time in min
	
	//respect the sleeping time
	if(settings.sleep_time && !TESTMODE){
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
	//int index = 0;
	for(int x = 0; x < MAX_MILESTONE; x++){
		//check if id allready exist
		if(settings.history[x] == step->id){
			break;	//if exist break
		}
		//find a free slot
		if(settings.history[x] <= 0){
			//save the id
			settings.history[x] = step->id;
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVE Milestone ID: %d @ slot: %d", step->id, x);}
			break;
		}
	}
}

static void setNextStep(int id){
	//int out = -1;	
	if (id < 1){ //startet mit ID 1
		id = 1;
	}
	
	settings.next_step = findStep(id);
}

static void addText(STEP *step){
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "addText mit id: %d Textcount: %d", step->id, settings.active_text_count);}
	if(settings.active_text_count+1 < MAX_TEXT){
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText - to array");
		settings.active_text[settings.active_text_count] = step;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "addText - count++");
		settings.active_text_count++;
		//MSG counter erhöhen für game stats
		
	}else{
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "to many Text Messages in the Menu: i delete the first one");}
		//erste Nachrichten löschen (Like a fifo) + Menu_select--
		for(int x = 1; x <= settings.active_text_count; x++ ){
			//shift all msg in the que
			settings.active_text[x-1] = settings.active_text[x];
		}
		//add the new text
		settings.active_text[settings.active_text_count] = step;
		//go up with menu_index
		if(settings.menu_index.section == 0 && settings.menu_index.row > 0){
			settings.menu_index.row--;
		}
	}
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
}

static bool is_step_real(int id){
	bool out = false;
	
	for(int i = 0; i < g.step_count; i++){
		if(g.step[i].id == id){
			return true;
		}
	}
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "check Step id: %d -> %d", id, out);}
	
	return out;
}

//-------------------------------- save - load settings -------------------

void save_settings() {	
	
	//save data	
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings)); //old methode - save settings directly
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVED");}
}

void load_settings() {  
	
	//check data in the memory		
	if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == (int)sizeof(settings)){
		//load data
		persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	

		//check pointer
		if(settings.active_text_count > 0){

			if(!is_step_real(settings.active_text[0]->id)){
				//show Error

				//if(debug){
				APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Step is not real - show error");
				//}
				//clear_menu();
				settings.active_text_count = 0;
				settings.active_antw_count = 0;
				settings.wakeup_reason = 0;				//reset the wakeup reason

				setNextStep(errorId);							//set the Error as next step
			}
		}

	}else{
		//memory not okay			
		//start with default settings
	}

} 

// ------------------------------------ Game Main Function ----------------

//game action & timer callback
static void game_action(void *data){
	bool show = 0;
	if(debug || show){APP_LOG(APP_LOG_LEVEL_DEBUG, "START: GAME_ACTION ");}
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free());}
	STEP *step = settings.next_step;
	int timer 	= 0;
	bool go_on 	= true;
	bool update_menu = false;	
	
	//step = null Fehlerabfang
	if(step == NULL){
		//Error next step not found
		if(debug || show){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: Next step not found. Step is NULL");}
		return;	
	}
	
	//step not found
	if(step->id > 10000){
		//show Error
		step = findStep(errorId);	
	}
	
	//no waiting in rapid_mode or Error
	if(!settings.rapid_mode && step->id != errorId){
		if(settings.wakeup_reason == TEXT || settings.wakeup_reason == ANTW){
			//do nothing while waiting for wakeup
			if(debug || show){APP_LOG(APP_LOG_LEVEL_DEBUG, "Waiting for the next Step. WAKEUP");}
			return;	
		}
	}
	
	//if nextID = 0 (auto count up)
	if (step->next == 0){
		step->next = step->id+1;
	}
	
	if(debug || show){APP_LOG(APP_LOG_LEVEL_DEBUG, "Prozessing Step ID: %d, nextId: %d, type: %d", step->id, step->next, step->type);}
	
	switch(step->type){
		case INFO:
			//is the same like a text
		//break;
//TEXT
		case TEXT:
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "GameAction: add Text text: %s", step->text);}
			addText(step);
			timer = step->delay;
			update_menu = true;
			setNextStep(step->next);
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Next Step: %d", step->next); 
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free()); 
		break;
//ANTW
		case ANTW:
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "GameAction: add Antw id: %d", step->id);}
			findAllAntw(step->id);
			settings.next_step = NULL;
			update_menu = true;
			go_on = false;
		
			setWakeup(ANTW, 30);	//set reminder
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set wakeup  ANTW - id: %d", settings.wakeup_reason);}
		break;
//FOUND DEAD		
		case FOUND_DEAD:
			wakeup_cancel_all();
			//add dead ID to the array
			settings.found_deads[step->delay] = true;		
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SET FOUND_DEAD #%d", step->delay);}
			//override the delay to go on
			step->delay = 0;
			setNextStep(step->next);
			//go_on = false;
		break;
//FOUND ALIVE		
		case FOUND_ALIVE:
			wakeup_cancel_all();
			//add dead ID to the array
			settings.found_alives[step->delay] = true;			
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SET FOUND_ALIVE #%d", step->delay);}
			//override the delay to go on
			step->delay = 0;
			setNextStep(step->next);
			//go_on = false;
		break;
		case BUTTON_MILESTONES:
			addAntw(step);
			
			wakeup_cancel_all();
			settings.next_step = NULL;
			update_menu = true;
			go_on = false;
		break;
		//case DIE_AKKU:
			//wakeup_cancel_all();
			//ToDo  save achievment 1. Todesfolge gefunden
			//setNextStep(step->next);
		//break;
//MILESTONE
		case MILESTONE:
			setNextStep(step->next);	//set next step and go on
			setMilestone(step);				//save this milestone
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "set Milestone");}
		break;		
//END
		case END:
			
			settings.next_step = NULL;
			go_on = false;
			
			//cancel all wakeup
			wakeup_cancel_all();
		
			//clear_menu();
			//active_text_count = 0;
			//settings.active_text[settings.active_text_count] = step;
			//settings.active_text_count++;
			
			//update_menu = true;
			//go_on = false;
			//setNextStep(step->next);			

			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "END OF GAME");} 

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

	//auto scroll down
	#if PBL_RECT
	if(settings.active_text_count < 3){
		//menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,settings.active_text_count), MenuRowAlignCenter, true);
		menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignCenter, PBL_IF_ROUND_ELSE(false, true));
	}	
	#endif
}

static void wakeup_handler() {
  
	wakeup_cancel_all();
	if (settings.vibe){
		vibes_double_pulse();	
		vibes_double_pulse();
	}	
	
	//reset the wakeup reason
	settings.wakeup_reason = -1;
	
	//show Info screen
	window_stack_pop_all(false);
	#if PBL_ROUND
  	window_stack_push(s_info_window, false);	
	#else
		window_stack_push(s_info_window, true);	
	#endif
}

// ------------------------------------ Timeline Menu Layer callbacks --------------- jear 

//timeline select
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	STEP *step;
	switch (cell_index->section){
		case 0:	//Empfangs Items
			//open the settings menu
			window_stack_push(s_menu_window, PBL_IF_ROUND_ELSE(false, true));	
		break;
		case 1: //Antwort Items
		
			setWakeup(TEXT, 60);	//set a reminder. 

			step = settings.active_antw[cell_index->row];		  

			if(step->type == ANTW){
				clear_menu();
				addText(step);
				setNextStep(step->next);

				//zwischenstand speichern
				save_settings();

				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: Go to next Step with ID: %d", step->next);}

				settings.wakeup_reason = -1;

				//go on
				game_action(NULL);
			}
			if(step->type == BUTTON_MILESTONES){
				settings.next_step = NULL;
				//show milestones
  			window_stack_push(s_mile_window, PBL_IF_ROUND_ELSE(false, true));	
			}

			//zwischenstand speichern
			save_settings();
				
		break;
		default: break;
	}
}

static uint16_t get_text_hight(STEP *step){
	if(step == NULL){
		if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "SORRY: Step is NULL (getTextHight)");}
		return 0;
	}	
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "calc Texthight from text: %s", step->text);}
	
	TextLayer *text_layer;
	
	GFont font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18 : FONT_KEY_GOTHIC_24));
	
	if(step->type == INFO || step->type == ANTW){
		font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
	}
	
	//get the size 
	#if PBL_ROUND
	text_layer = text_layer_create(GRect(0, 0, 130, 500));
	#else
	text_layer = text_layer_create(GRect(0, 0, 128, 500));
	#endif
	text_layer_set_text(text_layer, step->text);	
	text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
  text_layer_set_font(text_layer, font);	
	GSize size = text_layer_get_content_size(text_layer);
	
	text_layer_destroy(text_layer);
	
	//if(size.h < 37){size.h = 37;}
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "TEXT HIGHT: %d", size.h);}
	
	return size.h;	
	//return 20;
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	//if(debug){ APP_LOG(APP_LOG_LEVEL_DEBUG, "Anzahl section: %d", (settings.active_text_count > 0 ? 1 : 0) + (settings.active_antw_count > 0 ? 1 : 0)); }
	//return (settings.active_text_count > 0 ? 1 : 0) + (settings.active_antw_count > 0 ? 1 : 0);
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "num_section");}
	if(settings.active_antw_count > 0 || settings.wakeup_reason > 0){
		return 2;
	}else{
		return 2;
	}
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  //if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "num row");}
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
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw header");}
	
	// Determine which section we're working with
  switch (section_index) {
    case 0:
        // Draw title text in the section header
        menu_cell_basic_header_draw(ctx, cell_layer, "Messages:");
      break;
    case 1:
        // Draw title text in the section header
				if(settings.active_antw_count > 0){
					menu_cell_basic_header_draw(ctx, cell_layer, "send:");	
				}else{
					if(settings.wakeup_reason != TEXT){
						menu_cell_basic_header_draw(ctx, cell_layer, "receiving...");
					}else{
						menu_cell_basic_header_draw(ctx, cell_layer, "Waiting for Tom.");
					}
				}
        
      break;
  }
}

static int16_t 	menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> cell height section: %d row: %d", cell_index->section, cell_index->row);}
	//return 18+26;

	switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)
		
			if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> step text id %d", settings.active_text[cell_index->row]->id);}
			return get_text_hight(settings.active_text[cell_index->row])+26;
	
		break;
		case 1:		//zweite Section (Antworten)
			if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> step antw id %d", settings.active_antw[cell_index->row]->id);}
			return get_text_hight(settings.active_antw[cell_index->row])+26;
		break;
		default: return 0;
	}
}

static void 		menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
 
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw row");}
	
	STEP *step; 
	int size=0;
	
	GFont font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18 : FONT_KEY_GOTHIC_24));
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
				font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
			}
		
			if(size>18){s_path = s_path_2;}
			if(size>36){s_path = s_path_3;}
			if(size>54){s_path = s_path_4;}
			if(size>72){s_path = s_path_5;}	
		
			//draw background
			#ifdef PBL_COLOR
			gpath_draw_filled(ctx, s_path);
			#endif
			gpath_draw_outline(ctx, s_path);			
		
			graphics_draw_text(ctx, 
				step->text, 
				font,
				#if PBL_ROUND
					GRect(25, 12, 130, 100), 
				#else
					GRect(8, 12, 128, 100), 
				#endif
				GTextOverflowModeWordWrap, 
				GTextAlignmentCenter, 
				NULL);			
		
      break;
    case 1:			//zweite Section (Antworten)
     
			step = settings.active_antw[cell_index->row];
			size = get_text_hight(step);
		
			if(step->type == INFO || step->type == ANTW){
				font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
			}
		
			if(size>18){s_path = s_path_2;}
			if(size>36){s_path = s_path_3;}
			if(size>54){s_path = s_path_4;}
			if(size>72){s_path = s_path_5;}	
			
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
			
			#if PBL_ROUND
				graphics_context_set_fill_color(ctx, timeline_ant_fill_color);
			#endif
		
			#ifdef PBL_COLOR
			gpath_draw_filled(ctx, s_path);
			#endif
			gpath_draw_outline(ctx, s_path);
			
			graphics_draw_text(ctx, 
				step->text, 
				font,
				#if PBL_ROUND
					GRect(25, 12, 130, 100), 
				#else
					GRect(8, 12, 128, 100), 
				#endif 
				GTextOverflowModeWordWrap, 
				GTextAlignmentCenter, 
				NULL);
  }
}

static void 		main_window_load(Window *window) {
  
	//timeline_active = true;
	
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "show timeline window");}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "size settings: %d", persist_get_size(SETTINGS_KEY));
	
	//create the msg borders
	s_path_1 = gpath_create(&path_1);		//1 line border
	s_path_2 = gpath_create(&path_2);
	s_path_3 = gpath_create(&path_3);
	s_path_4 = gpath_create(&path_4);		//4 line border
	s_path_5 = gpath_create(&path_5);
	
	//s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG_1);
	
  // Now we prepare to initialize the menu layer
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "set timeline callbacks");}
	
  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections 	= menu_get_num_sections_callback,
    .get_num_rows 			= menu_get_num_rows_callback,
    .get_header_height 	= PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header 				= PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL),
    .draw_row 					= menu_draw_row_callback,
    .select_click 			= menu_select_callback,
    .get_cell_height 		= menu_get_cell_height_callback
		//.draw_background 		= menu_draw_background_callback,
		//.get_separator_height=menu_get_separator_height_callback,
		//.draw_separator			= menu_draw_separator_callback
  });
		
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "timeline callbacks ready");}
	
	//Menu config
	menu_layer_set_normal_colors(s_menu_layer, GColorClear, timeline_text_color);
	menu_layer_set_highlight_colors(s_menu_layer, GColorClear, timeline_text_color);
	#if PBL_ROUND
		menu_layer_pad_bottom_enable(s_menu_layer, true);
	#else
		menu_layer_set_center_focused(s_menu_layer, false);
	#endif
	
	
	//set menu index to the old postionen
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "set menu index");}
	menu_layer_set_selected_index(s_menu_layer, settings.menu_index, MenuRowAlignCenter, false);
	
	//start the game
	if(debug && debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "start game action");}
	game_action(NULL);
		
  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void 		main_window_unload(Window *window) {
	
	settings.menu_index = menu_layer_get_selected_index(s_menu_layer);
	
	// Destroy the menu layer
	//layer_destroy(s_info_layer);
  menu_layer_destroy(s_menu_layer);
	//text_layer_destroy(s_info_text);
	
	gpath_destroy(s_path_1);
	gpath_destroy(s_path_2);
	gpath_destroy(s_path_3);
	gpath_destroy(s_path_4);
	gpath_destroy(s_path_5);
}

// ------------------------------------ Settings Menu Layer --------------

static void 		set_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	if(debug && debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: select");}
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
					if(settings.wakeup_reason == 1){	//faster at incoming msg
						game_action(NULL);	
					}
				break;				
				case 2:	//Ruhezeit
					settings.sleep_time = !settings.sleep_time;
					menu_layer_reload_data(menu_layer);
				break;
				/*
				case 3:	//fontsize
					settings.font = (settings.font == 0 ? 1 : 0);
					menu_layer_reload_data(menu_layer);
				break;
				*/
				/*
				case 3:	//neustart
					setNextStep(0);
					wakeup_cancel_all();
					settings.wakeup_reason = -1;
				
					clear_menu();
					window_stack_pop_all(true);
					window_stack_push(s_welc_window, true);
				break;
				*/
				case 3:	//Milestone
					//show milestones
					#if PBL_ROUND
						window_stack_push(s_mile_window, false);	
					#else
						window_stack_push(s_mile_window, true);	
					#endif
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
		//reset
		case 2:
			switch(cell_index->row){
				case 0:
					//show intro
					#if PBL_ROUND
						window_stack_push(s_welc_window, false);	
					#else
						window_stack_push(s_welc_window, true);	
					#endif
				break;
				case 1:	//reset data
					//delete the savegame
					persist_delete(SETTINGS_KEY);
				
					// reset settings
					settings.vibe = true;
					settings.rapid_mode = false;
					settings.sleep_time = true;
	
					//clear history
					for(int x = 0; x < MAX_MILESTONE; x++){
						settings.history[x] = 0;
					}
					for(int x = 0; x < MAX_DEADS; x++){
						settings.found_deads[x] = false;
					}
					for(int x = 0; x < MAX_ALIVES; x++){
						settings.found_alives[x] = false;
					}
					
					//restart game
					setNextStep(0);
					wakeup_cancel_all();
					settings.wakeup_reason = -1;
				
					clear_menu();
					
					window_stack_pop_all(false);
					window_stack_push(s_welc_window, PBL_IF_ROUND_ELSE(false, true));	
				break;
				case 2:
					//show credits
					//window_stack_push(s_cred_window, true);
				break;
			}
			
		break;
		default: break;
	}
}

static uint16_t set_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	return 3;
}

static uint16_t set_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	if(debug && debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: num row");}
  switch (section_index) {
    case 0:			
			//settings
			return 4;
    case 1:
			//achievments
      return 2;
		case 2:
			//credits + welc
			return 2;
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
	if(debug && debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: draw header");}
  switch (section_index) {
    case 0:
        // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Settings:");
      break;
    case 1:
        // Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, "Achievements:");        
    	break;
		case 2:
			menu_cell_basic_header_draw(ctx, cell_layer, "Extras:");        
		break;
  }
}

static void 		set_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	int i = 0;
	char buf[] = "0 / 0";
	// Determine which section we're going to draw in
	if(debug && debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: draw row");}
  switch (cell_index->section) {
    case 0:			//erste Section (Einstellungen)    	
			switch (cell_index->row) {
				case 0: 
					menu_cell_basic_draw(ctx, cell_layer, "Vibration", (settings.vibe ? "on" : "off"), NULL);
				break;
				case 1:
					menu_cell_basic_draw(ctx, cell_layer, "Rapid mode", (settings.rapid_mode ? "on" : "off"), NULL);
				break;
				case 2:
					menu_cell_basic_draw(ctx, cell_layer, "Rest period", (settings.sleep_time ? "22:00-8:00" : "off"), NULL);
				break;
				/*
				case 3:
					menu_cell_basic_draw(ctx, cell_layer, "Font size", (settings.font == 0 ? "default" : "+1"), NULL);
				break;
				*/
				/*
				case 3:
					menu_cell_basic_draw(ctx, cell_layer, "Restart", NULL, NULL);	//"start from 0"
				break;
				*/
				case 3:
					menu_cell_basic_draw(ctx, cell_layer, "Milestones", NULL, NULL);	//"go back in time"
				break;
			}
      break;
    case 1:			//zweite Section (Erfolge)
    	switch (cell_index->row){
				case 0:
					for(int x = 0; x < MAX_ALIVES; x++){
						if(settings.found_alives[x]){i++;}
					}
					snprintf(buf, sizeof(buf), "%d / %d", i, MAX_ALIVES);				
					menu_cell_basic_draw(ctx, cell_layer, buf, "found ways to survive", NULL);		
				break;
				case 1: 					
					for(int x = 0; x < MAX_DEADS; x++){
						if(settings.found_deads[x]){i++;}
					}
					snprintf(buf, sizeof(buf), "%d / %d", i, MAX_DEADS);
					menu_cell_basic_draw(ctx, cell_layer, buf, "found ways to die", NULL);
				break;
			}
		break;
		case 2:
			switch (cell_index->row){
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, "Intro", NULL, NULL);
				break;
				case 1:
					menu_cell_basic_draw(ctx, cell_layer, "Reset", NULL, NULL); //"delete all"
				break;
				case 2:
					menu_cell_basic_draw(ctx, cell_layer, "Credits", NULL, NULL);
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
    .get_header_height 	= PBL_IF_RECT_ELSE(set_get_header_height_callback,NULL),
    .draw_header 				= PBL_IF_RECT_ELSE(set_draw_header_callback,NULL),
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


// ------------------------------------ Milestone Menu Layer --------------

static void 		mil_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {

	if(cell_index->row == 0){
		//restart
		setNextStep(0);
		wakeup_cancel_all();
		settings.wakeup_reason = -1;

		clear_menu();
		window_stack_pop_all(false);
		#if PBL_ROUND
			window_stack_push(s_welc_window, false);	
		#else
			window_stack_push(s_welc_window, true);	
		#endif
	}else{
		//Milestone
		//cell_index->row-1
		setNextStep(settings.history[cell_index->row-1]);
		wakeup_cancel_all();
		settings.wakeup_reason = -1;

		clear_menu();
		window_stack_pop_all(false);
		#if PBL_ROUND
			window_stack_push(s_main_window, false);	
		#else
			window_stack_push(s_main_window, true);	
		#endif
	}		
}

static uint16_t mil_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	return 1;
}

static uint16_t mil_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: num row");}
	int rows = 1;
  switch (section_index) {
    case 0:			
			//milestones
			for(int x = 0; x < MAX_MILESTONE; x++){
				if(settings.history[x] > 0){
					rows++;
				}
			}
			return rows;
    break;
    default:
      return 0;
  }
}

static int16_t 	mil_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void 		mil_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
        // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Milestones:");
    break;
  }
}

static void 		mil_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	 	
	//Menu Text
	if(cell_index->row == 0){
		//restart
		menu_cell_basic_draw(ctx, cell_layer, "Beginning", NULL, NULL);
	}else{
		//Milestones
		menu_cell_basic_draw(ctx, cell_layer, findStep(settings.history[cell_index->row-1])->text, NULL, NULL);
	}
}

static int16_t  mil_get_separator_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_contex){
	return 0;	
}

static void 		mil_window_load(Window *window) {
  
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "start milestone menu");}
	
  // Now we prepare to initialize the menu layer
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
	
	s_mil_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_mil_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections 	= mil_get_num_sections_callback,
    .get_num_rows 			= mil_get_num_rows_callback,
    .get_header_height 	= PBL_IF_RECT_ELSE(mil_get_header_height_callback,NULL),
    .draw_header 				= PBL_IF_RECT_ELSE(mil_draw_header_callback,NULL),
    .draw_row 					= mil_draw_row_callback,
    .select_click 			= mil_select_callback,
    //.get_cell_height 		= menu_get_cell_height_callback,
		//.draw_background 		= menu_draw_background_callback,
		.get_separator_height=mil_get_separator_height_callback
		//.draw_separator			= menu_draw_separator_callback
  });
	
	menu_layer_set_normal_colors(s_mil_layer, milestone_bg_color, milestone_text_color);
	menu_layer_set_highlight_colors(s_mil_layer, milestone_bg_color_hi, milestone_text_color_hi);
	
	menu_layer_set_click_config_onto_window(s_mil_layer, window);
	
  layer_add_child(window_layer, menu_layer_get_layer(s_mil_layer));
}

static void 		mil_window_unload(Window *window) {
	
	// Destroy the menu layer
  menu_layer_destroy(s_mil_layer);
}

// ------------------------------------ Welcome Layer callbacks ---------------

void welc_select_click_handler(ClickRecognizerRef recognizer, void *context){
	window_stack_remove(s_welc_window,false);
	#if PBL_ROUND
		window_stack_push(s_main_window, false);	
	#else
		window_stack_push(s_main_window, true);	
	#endif
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
	#if PBL_ROUND
  	GRect max_bounds = GRect(0, 00, 160, 2000);
	#else
		GRect max_bounds = GRect(0, 0, 144, 2000);
	#endif
	
	s_welc_layer = scroll_layer_create(bounds);
	
	scroll_layer_set_click_config_onto_window(s_welc_layer, window);
	scroll_layer_set_callbacks(s_welc_layer, (ScrollLayerCallbacks){
		.click_config_provider = welc_click_config_provider
	});
	
	window_set_background_color(s_welc_window, welcome_bg_color);
	
	//draw Layer inputs	
	#if PBL_ROUND
		s_welc_text1 = text_layer_create(GRect(20, 20, max_bounds.size.w, 100));
	#else
		s_welc_text1 = text_layer_create(GRect(0, 0, max_bounds.size.w, 100));
	#endif
	text_layer_set_text(s_welc_text1, g.name);	//Headline
  text_layer_set_font(s_welc_text1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_background_color(s_welc_text1, GColorClear);
	text_layer_set_text_color(s_welc_text1, welcome_text_color);
	text_layer_set_text_alignment(s_welc_text1, GTextAlignmentCenter);	

	#if PBL_ROUND
		s_welc_text2 = text_layer_create(GRect(25,80, max_bounds.size.w-10, max_bounds.size.h));
	#else
		s_welc_text2 = text_layer_create(GRect(5,60, max_bounds.size.w-10, max_bounds.size.h));
	#endif
	text_layer_set_text(s_welc_text2, g.desc);			//description text
  text_layer_set_font(s_welc_text2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_welc_text2, GTextAlignmentLeft);	
	text_layer_set_background_color(s_welc_text2, GColorClear);
	text_layer_set_text_color(s_welc_text1, welcome_text_color);
	GSize size = text_layer_get_content_size(s_welc_text2);
	text_layer_set_size(s_welc_text1, GSize(144,size.h+10));
	
	//resize the scroll layer
	#if PBL_ROUND
		scroll_layer_set_content_size(s_welc_layer, GSize(160, size.h + 110 + 10));
	#else
		scroll_layer_set_content_size(s_welc_layer, GSize(144, size.h + 60 + 10));
	#endif
	
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
	#if PBL_ROUND
		window_stack_push(s_main_window, false);		
	#else
		window_stack_push(s_main_window, true);		
	#endif
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

	s_info_layer = layer_create(bounds);
	
	//set a background image	

	//get random number
	int random = rand()%3;

	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "rand %d", random );}
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free());}

	uint32_t resource_id = RESOURCE_ID_MSG_BG1;

	switch(random){
		case 0: 	resource_id = RESOURCE_ID_MSG_BG1; 	break;
		case 1: 	resource_id = RESOURCE_ID_MSG_BG2; 	break;
		case 2: 	resource_id = RESOURCE_ID_MSG_BG3; 	break;
		default: 	resource_id = RESOURCE_ID_MSG_BG1; 	break;
	}
		
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "resource_id: %d", (int)resource_id );
	
	s_background_image = gbitmap_create_with_resource(resource_id);

	s_background_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_compositing_mode(s_background_layer, GCompOpSet);
	bitmap_layer_set_bitmap(s_background_layer, s_background_image);

	//Add image to the layer
	layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

		
	//draw Layer inputs
	#if PBL_ROUND
		s_info_text = text_layer_create(GRect(25, 61, 137, 100));	//round position
	#else
		s_info_text = text_layer_create(GRect(7, 50, 137, 100));	//rect position
	#endif
	text_layer_set_text(s_info_text, g.text_msg);	//Headline incoming Msg
	if(settings.next_step == NULL){
		text_layer_set_text(s_info_text, g.text_wait);	//Headline waiting for answer
	}
	text_layer_set_background_color(s_info_text, GColorClear);
	text_layer_set_text_color(s_info_text, GColorBlack);
  text_layer_set_font(s_info_text, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(s_info_text, GTextAlignmentCenter);		
	
	//add text to the layer
	layer_add_child(s_info_layer, text_layer_get_layer(s_info_text));	
	
	//add layer to the window
	layer_add_child(window_layer, s_info_layer);
	
	//set click_provider
	window_set_click_config_provider(window, (ClickConfigProvider) info_click_config_provider);
	//if(debug){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap END LOAD INFO Window: %d", (int)heap_bytes_free());
	//}
}

static void info_window_unload(Window *window) {
	save_settings();
	
	bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_image);
	
	layer_destroy(s_info_layer);
	
	text_layer_destroy(s_info_text);
	text_layer_destroy(s_info_text2);
}

/*
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
*/ //credit layer
// -------------------------------------- Start and Close the App ----------

void checkSteps(){
	//const int i = g.step_count;
	//int stepID[200] = {-1};
	//bool nextID[200] = {false};
	//bool call[200]	 = {false};
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "check Steps start");
	
	for(int i = 0; i < g.step_count-1; i++){
		//stepID[i] = g.step[i].id;
		
		int nextId = g.step[i].next;
		if(nextId == 0){
			nextId = g.step[i].id+1;
		}
		if(!is_step_real(nextId)){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "check Step id: %d -> %d", g.step[i].id, g.step[i].next);
		}		
	}
	
	//check the length
	if(g.step[g.step_count-1].type == END){
		APP_LOG(APP_LOG_LEVEL_DEBUG, "step_count is right");	
	}else{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "step_count is wrong");	
		bool search = true;
		int x = 0;
		while(search){
			if(g.step[x].type == END){
				APP_LOG(APP_LOG_LEVEL_DEBUG, "step_count is %d", x+1 );	
				search = false;
			}
			x++;
		}
		
	}
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "check Steps End");	
}

static void init() {
	//persist_delete(SETTINGS_KEY);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "size of settings; %d", (int)sizeof(settings));
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "size of persist; %d", persist_get_size(SETTINGS_KEY));
	
	//if(debug){APP_LOG(
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free());
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Used heap: %d", (int)heap_bytes_used());
	//}
		
	srand(time(NULL));
	
	//load data
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD SAVEGAME");}
	load_settings();	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOADED");}
	
	//settings.history[0] = 3;
	
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
	
	
	//init milestone menu
	s_mile_window = window_create();	//settings_menu
  window_set_window_handlers(s_mile_window, (WindowHandlers){
    .load 	= mil_window_load,
    .unload = mil_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: milestone menu");}
	
	/*
	//init credits layer
	s_cred_window = window_create();
  window_set_window_handlers(s_cred_window, (WindowHandlers){
    .load 	= cred_window_load,
    .unload = cred_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: credits");}
*/
	
	// subscribe to wakeup service to get wakeup events while app is running
	wakeup_service_subscribe(wakeup_handler);
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "launche_reason: %d", launch_reason());}
	
	//App start from wakeup
  if(launch_reason() == APP_LAUNCH_WAKEUP) { 

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START wakeup handler");}    
		wakeup_handler();

	//normal app system start
  } else {																		

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START normal start - no wakeup");}
		
		if(settings.next_step != NULL && settings.active_text_count == 0){
			
			if(settings.next_step->id <= 1){ 		//first run
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Welcome window... (FIRST RUN)");}
			
				wakeup_cancel_all();
				settings.wakeup_reason = -1;
				
				//setNextStep(1);
	
				#if PBL_ROUND
					window_stack_push(s_welc_window, false);	
				#else
					window_stack_push(s_welc_window, true);	
				#endif
			}else{													//show currend game
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window (SHOW CURREND GAME, GO ON WITH NEXT STEP)");}
				#if PBL_ROUND
					window_stack_push(s_main_window, false);	
				#else
					window_stack_push(s_main_window, true);	
				#endif
			}
	
		}else{
			
			
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window (next_step == NULL)");}
			#if PBL_ROUND
				window_stack_push(s_main_window, false);	
			#else
				window_stack_push(s_main_window, true);	
			#endif
		}
				
  }  
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free()); 
	//window_stack_push(s_info_window, false);  //just to test this page
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap after: %d", (int)heap_bytes_free());
}

static void deinit() {
	save_settings();
	
  window_destroy(s_main_window);
	window_destroy(s_welc_window);
	window_destroy(s_info_window);
	window_destroy(s_menu_window);
	window_destroy(s_mile_window);
	//window_destroy(s_cred_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
