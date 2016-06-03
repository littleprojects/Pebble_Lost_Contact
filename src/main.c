#include <pebble.h>
#include "main.h"
#include "lang.h"

/* TODO
*
*	Dynamische Text lentgh im struct
*
*/ //TODO

#define TESTMODE				1			//testmodus the time to next Step is set to 1 min

#define debug						1
#define debug_parser 		0
#define debug_welcome		0

#define MAX_TEXT_LEN		100
#define MAX_HEAD_LEN 		20
#define MAX_LINE_LEN		(MAX_TEXT_LEN + MAX_HEAD_LEN)

#define MAX_TEXT_COUNT 	20
#define MAX_ANTW_COUNT 	2
#define MAX_MILE_COUNT 	10

#define MAX_MILESTONE	 	10
#define MAX_ALIVES			3
#define MAX_DEADS				1

typedef enum {
  INFO 							= (uint8_t)'I',			//create a Info Textbox
	TEXT 							= (uint8_t)'T',			//create a normal Textbox
	ANTW 							= (uint8_t)'A',			//create a answer
	BUSY							= (uint8_t)'B',			//show busy text and wait

	MILESTONE 				= (uint8_t)'M', 		//save this ID as a Milestone -> restart from here
	FOUND_ALIVE 			= (uint8_t)'X',			//save this ID(time) in the FOUND history
	FOUND_DEAD 				= (uint8_t)'Y',
	BUTTON_MILESTONE 	= (uint8_t)'Z',

	END 							= (uint8_t)'E',
} TYPE;

typedef struct LINE{
	uint16_t start;
	uint16_t id;
	uint16_t next_id;
	uint8_t typ;
	uint8_t len;
	unsigned char text[MAX_TEXT_LEN];
	uint16_t special;
} LINE;

/*
LINE init_line = {
	.start 		= 0,
	.id				= 0,
	.next_id 	= 0,
	.typ 			= END,
	.len			= 0,
	.text			= {""},
	.special	= 0
};
len;ID;nextID;TYP;Payload(Text or Int)
24;1;0;I;[found signal]
*/

//global vars
LINE text[MAX_TEXT_COUNT];	//Text array
LINE antw[MAX_ANTW_COUNT];	//Antw array
LINE mile[MAX_MILE_COUNT];	//Milestone / jumppoint array
LINE work;									//work line

//memset(text[1], (uint)0, sizeof(text));
ResHandle rh;
uint filesize = 0;

//read line buffer
uint8_t line_buffer[MAX_LINE_LEN];

//file pointer
uint pointer	 	= 0;
uint text_count = 0;
uint antw_count = 0;
uint mile_count = 0;

uint old_pointer 	= 1;		//for double line read check
uint last_pointer = 0;		//the save the last found ID
uint last_id 			= 0;
uint line 				= 0;

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

//settings && savegame
typedef struct save_line{
	uint16_t start;
	uint16_t id;
} save_line;

typedef struct {
  //current game 
	save_line active_text[MAX_TEXT_COUNT];	//pointer of the Text Steps
	save_line active_antw[MAX_ANTW_COUNT];	//pointer of the antw Steps
	uint8_t   active_text_count;				
  uint8_t   active_antw_count;
	uint16_t 	next_id;								//pointer of the next Step	
	
	int8_t		wakeup_reason;								//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;								//save the current position in the timeline
	
	//settings	
	bool 	vibe;													//vibration
	bool 	rapid_mode;										//fast mode
	bool 	sleep_time;											
	uint8_t font;												//font size 0= standard, 1= +1
	
	//history
	save_line	history[MAX_MILESTONE];		//ID of the milestones
	bool 			found_deads[MAX_DEADS];		//ID of the Achievments
	bool 			found_alives[MAX_ALIVES];
} SETTINGS;

//set defaults
SETTINGS settings = {
	//current game var
  .active_text_count = 0,
	.active_antw_count = 0,
	//.active_text 	= {NULL},
	//.active_antw 	= {NULL},
	.next_id 				= 1,
	.wakeup_reason 	= -1,
	.menu_index	 		= {0,0},
	
	//settings
	.vibe = true,
	.rapid_mode = false,
	.sleep_time = true,
	.font = 0,
	
	//history
	//.history = {0},
	.found_deads = {false},
	.found_alives = {false}
};


// ------------------------------------ Parser ---------------

//convert a line_buffer part to a int
uint16_t parse_int(uint16_t start, uint16_t len){
	uint16_t out = 0;
	
	//create buffer
	//char *temp = (char*)malloc(len);				
	char temp[len];
	
	//copy elementes from line to buffer
	for(int j = 0; j < len; j++){		//len
		temp[j] = (char)line_buffer[j + start]; //start offset
	}
	
	//convert buffer to int
	out = (uint16_t)atoi((char *)temp);
	
	//destroy buffer
	//free(temp);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse Special: %d (start %d, len %d)", out, start, len);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "line %s", line_buffer);
	return out;
}

//load the payload in the work Item
void parse_string(uint16_t start ){
	int text_len = work.len-start;
	
	for(int i = 0; i < text_len; i++ ){
		work.text[i] = (char)line_buffer[i + start - 1]; //start at 0 not at 1 
	}
	//work.text[text_len] = 0;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse Text (Len %d)", text_len);
}

void game_action(){
	APP_LOG(APP_LOG_LEVEL_DEBUG, " ----- GAME ACTION -----");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "with ID: %d", work.id);
	
	//save the last file_pointer & id
	last_pointer 	= work.start;
	last_id				= work.id;
}


//durchsuche die Datei
void search_id(uint search_id){
	bool search = true;
	bool found 	= false;
	old_pointer = 1;
	
	//ToDO: überprüfe bekannte ID für eine besserer Startposition
	
	if(search_id >= last_id){
		pointer = last_pointer;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set last Pointer");
		//pointer = 0;
	}else{
		//check andere Bekante IDs (Milestone,...)
		pointer = 0;
	}
	
	//suche bis zum abbruch
	while(search){
		int len = MAX_LINE_LEN;
		found = false;
		
		//Abbruch am Fileende
		if(pointer >= filesize){
			if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Pointer >= filesize -> cancel search!");}
			break;
		}
		
		if(old_pointer == pointer){
			if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "WIEDERHOLUNG: Abbruch");}
			break;
		}else{
			old_pointer = pointer;
		}
		
		//Längennpassung wenn das Fileende erreicht wird
		if(pointer + MAX_LINE_LEN >= filesize){
			//search = false;
			len = filesize - pointer;
			if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Len anpassung: read fenster > filezize");}
		}
		
		if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "read Line from: %d to %d (%d zeichen)", pointer, pointer+len, len);}
		
		//clear line_buffer
		//line_buffer = "";
		
		//clear work Item
		work.len 			= 0;
		work.id 			= 0;
		work.next_id 	= 0;
		work.typ 			= 0;
		work.special 	= 0;
		//work.text			= {0};
		
		//read line
		resource_load_byte_range(rh, pointer, line_buffer, len);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Buffer %s", (char *)line_buffer);
		
		if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, " ----- read header -----");}
		
		int element_start = 0;
		int element		= 0;
		//read header
		for(int i = 0; i < MAX_HEAD_LEN; i++){
			
			//process
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Line Buffer '%c' %d. Zeichen", (char)line_buffer[i], i);
			
			//detect linebreak
			if(line_buffer[i] == 10 ){
				//set element_start at this point
				element_start = i+1;
			}
			
			//detect seperator
			if(line_buffer[i] == (uint8_t)';'){
				//APP_LOG(APP_LOG_LEVEL_DEBUG, "Element: %d Seperator at: %d", element, i);
				
				//uint16_t val2 = parse_int(element_start, i-element_start);
				
				//Wert separieren
				//char e_buffer = (char*)malloc(i - element_start -1);
				char e_buffer[i-element_start];// 
				//e_buffer = "";
				//memcpy(e_buffer, (char*)buffer, i-1 - start_head);
				
				//copy elementes from line to buffer
				for(int j = 0; j < i-element_start; j++){
					e_buffer[j] = (char)line_buffer[j+element_start];
					e_buffer[j+1] = (char)0;
					//APP_LOG(APP_LOG_LEVEL_DEBUG, "FILL e_buffer '%c' %d. Zeichen", e_buffer[j], j);
				}
				//APP_LOG(APP_LOG_LEVEL_DEBUG, "Buffer '%s' (%d Zeichen)", (char *)e_buffer, i-element_start);
				
				
				//Wert umwandeln
				//uint16_t val = (uint16_t)atoi(e_buffer);
				//APP_LOG(APP_LOG_LEVEL_DEBUG, "Wert: %d", val);
				
				//Wert setzten
				//set Len
				if(element == 0){	//len
					work.len = (uint16_t)atoi(e_buffer);
				}
				//set ID
				if(element == 1){	//id
					work.id = (uint16_t)atoi(e_buffer);
				//}
				
					//Check ID 
					//kleiner gesucht -> weiter suchen
					if(work.id < search_id){
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, " CHECK ID ------ ID zu klein -> Go to next");}
						break;	//diese line überspringen
					}
					//größer gesucht -> abbreuch suche
					if(work.id > search_id){
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, " CHECK ID ------ ID zu groß. Abbruch");}
						search = false;	//suche abbrechen
						break;
					}
					//gleich gesucht -> lade payload
					if(work.id == search_id){
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, " CHECK ID ------ ID gefunden. juhuuuu --- get payload");}
						found =  true;
					}
				}
				
				//set next id
				if(element == 2){
					work.next_id =  (uint16_t)atoi(e_buffer);
					//auto count up
					if (work.next_id == 0){
						work.next_id = work.id+1;
						//APP_LOG(APP_LOG_LEVEL_DEBUG, "Auto count up: nid = %d",  work.next_id);
					}
					//work.next_id =  (uint16_t)atoi(e_buffer);
				}
				//set typ
				if(element == 3){
					//APP_LOG(APP_LOG_LEVEL_DEBUG, "TYPE: %s",  e_buffer);
					work.typ = (uint8_t)e_buffer[0];
					
					element_start = element_start+3; //+sepreator + Typ + seperator
					element++;					
				}					
				
				
				//abbruch read header wenn alle 4 Elemente gelesen wurden
				if(element > 3){		
					if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Load payload:");}
					
					//read payload
					if(work.typ == BUSY || work.typ == FOUND_DEAD || work.typ == FOUND_ALIVE){
						//payload is a int
						work.special = parse_int(element_start-1, work.len-element_start);
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "SPECIAL: start: %d, len %d" , element_start, work.len-element_start );}
					}else{
						//payload is a string
						//APP_LOG(APP_LOG_LEVEL_DEBUG, "TEXT: start: %d, len %d" , element_start, work.len-element_start );
						parse_string(element_start);
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "TEXT: %s", (char*)work.text);}
					}
					
					switch(work.typ){
						case INFO:
							//APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Info");
						break;
						case BUSY:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Busy");
						break;						
						case TEXT:
							//APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Text");
						break;						
						case ANTW:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Antw");
						break;			
						case MILESTONE:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Milestone");
						break;
						
						case FOUND_ALIVE:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: alive");
						break;
						case FOUND_DEAD:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Dead");
						break;
						case BUTTON_MILESTONE:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Button_Milestone");
						break;
						case END:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: END");
						break; 	
						
						default:
							APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: +++UNKNOW+++ (%d)", work.typ);
						break;
					}
					
					break;					
				}
				
				
				//free(e_buffer);	
				element_start = i+1; //element_start + his seperator
				element++;
			}//end seperator detection
		}//end read header
		
		if(element == 0){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "kein Header Element gefunden");
			break;
		}
		if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "end Read header");}
		
		work.start = pointer;
		if(debug_parser){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK start: %d", work.start);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK len: %d", work.len);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK id: %d", work.id);
		}
		if(found){
			if(debug_parser){
				APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK nid: %d", work.next_id);
				APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK typ: %d", work.typ);
				APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK text: %s", (char*)work.text);
				APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK spec: %d", work.special);
			}
			// work ITEM verarbeiten
			//game_action();
			
			//set last Pointer
			last_pointer 	= pointer;	
			last_id 			= work.id;
			
			//suche abbrechen
			search = false;
			break;
		}		
		
		//set new pointer
		pointer = pointer + work.len + 1; //+linebraker
		
		if(debug_parser){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "set pointer (nex line starts at): %d", pointer);	
			APP_LOG(APP_LOG_LEVEL_DEBUG, "--------------- end line -----------------\n\n");
		}
	}//end while search
	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "end search");}
}

// ------------------------------------ Game functions ---------------

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

// ------------------------------------ Wakeup handler ---------------

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
	
	if(debug_welcome){APP_LOG(APP_LOG_LEVEL_DEBUG, "start welcome window");}
	
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
	text_layer_set_text(s_welc_text1, lang.name);	//Headline
  text_layer_set_font(s_welc_text1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_background_color(s_welc_text1, GColorClear);
	text_layer_set_text_color(s_welc_text1, welcome_text_color);
	text_layer_set_text_alignment(s_welc_text1, GTextAlignmentCenter);	

	#if PBL_ROUND
		s_welc_text2 = text_layer_create(GRect(25,80, max_bounds.size.w-10, max_bounds.size.h));
	#else
		s_welc_text2 = text_layer_create(GRect(5,60, max_bounds.size.w-10, max_bounds.size.h));
	#endif
	text_layer_set_text(s_welc_text2, lang.desc);			//description text
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
	text_layer_set_text(s_info_text, lang.text_msg);	//Headline incoming Msg
	if(settings.next_id == 0){
		text_layer_set_text(s_info_text, lang.text_wait);	//Headline waiting for answer
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
	//save_settings();
	
	bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_image);
	
	layer_destroy(s_info_layer);
	
	text_layer_destroy(s_info_text);
	text_layer_destroy(s_info_text2);
}

// --------------------------------------------- Check file ---------------------------

void check_file(){
	LINE line;
	bool search = true;
	//bool found 	= false;
	uint line_pointer 		= 0;
	uint old_line_pointer = 1;
	uint line_counter 		= 1;
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "START CHECK");
	
	//suche bis zum abbruch
	while(search){
		int len = MAX_LINE_LEN;
		//found = false;
		
		//Abbruch am Fileende
		if(line_pointer >= filesize){
			if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Pointer >= filesize -> cancel search!");}
			search = false;
			break;
		}
		
		if(old_line_pointer == line_pointer){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "WIEDERHOLUNG: Abbruch at: %d", line_pointer);
			search = false;
			break;
		}
		old_line_pointer = line_pointer;
		
		//Längennpassung wenn das Fileende erreicht wird
		if(line_pointer + MAX_LINE_LEN >= filesize){
			//search = false;
			len = filesize - line_pointer;
			if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Len anpassung: read fenster > filezize");}
		}
		
		//clear work Item
		line.len 			= 0;
		line.id 			= 0;
		line.next_id 	= 0;
		line.typ 			= 0;
		line.special 	= 0;
		//work.text			= {0};
		
		//read line
		resource_load_byte_range(rh, line_pointer, line_buffer, len);
		
		APP_LOG(APP_LOG_LEVEL_DEBUG, "CHECK LINE: %d", line_counter);
		
		int element_start = 0;
		int element				= 0;
		
		//read header
		for(int i = 0; i < MAX_HEAD_LEN; i++){
			
			//detect linebreak
			if(line_buffer[i] == 10 ){
				APP_LOG(APP_LOG_LEVEL_DEBUG, "FOUND A LINEBREAK in header!");	
				search = false;
				//set element_start at this point
				element_start = i+1;
			}
			
			//detect seperator
			if(line_buffer[i] == (uint8_t)';'){
				
				char e_buffer[i-element_start];// 
				
				//copy elementes from line to buffer
				for(int j = 0; j < i-element_start; j++){
					e_buffer[j] = (char)line_buffer[j+element_start];
					e_buffer[j+1] = (char)0;
				}

				//set Len
				if(element == 0){	//len
					line.len = (uint16_t)atoi(e_buffer);
				}
				//set ID
				if(element == 1){	//id
					line.id = (uint16_t)atoi(e_buffer);
					//found =  true;
				}
				
				//set next id
				if(element == 2){
					line.next_id =  (uint16_t)atoi(e_buffer);
					//auto count up
					if (line.next_id == 0){
						line.next_id = line.id+1;
					}
					break;
				}					
									
				element_start = i+1; //element_start + his seperator
				element++;
			}//end seperator detection
		}//end read header
		
		//search next id
		search_id(line.next_id);
		
		//check found line
		if(work.id != line.next_id){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "No ID Found under ID %d (%d gefunden)", line.next_id, work.id);	
		}
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "LINE ID: %d", line.id);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "LINE NID: %d", line.next_id);
		
		//set new pointer
		line_pointer = line_pointer + line.len + 1; //+linebraker
		line_counter++;
		
		//abbruch
		if(line_counter > 4){
			//search = false;
		}
		
	}//end while search	
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "END CHECK");	
}

// ------------------------------------ Init ---------------

static void init(void) {
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START APP");}
	
	//set resource handle
	rh = resource_get_handle(RESOURCE_ID_STORY_EN);
	//set filesize
	filesize = resource_size(rh);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d Byte", filesize);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d Zeichen", filesize);
	
	//check_file();
	
	//seed random generator
	srand(time(NULL));
		
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
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "launche_reason: %d", launch_reason());}
	
	//App start from wakeup
  if(launch_reason() == APP_LAUNCH_WAKEUP) { 

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START wakeup handler");}    
		wakeup_handler();

	//normal app system start
  } else {																		

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START normal start - no wakeup");}
		
		if(settings.next_id > 0 && settings.active_text_count == 0){
			
			if(settings.next_id <= 1){ 		//first run
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
	
}

static void deinit(void) {
	// Destroy the text layer
	//text_layer_destroy(s_text_layer);
	
	// Destroy the window
	//window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
