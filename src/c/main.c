#include <pebble.h>
#include "main.h"
#include "lang.h"

/* TODO
*
* TESTEN - SAVE & LOAD data
 *
* STORY!!!!
*
*	Dynamische Text lentgh im LINE struct (wenn das möglich ist) ist ja ein Array um speicher zu sparen
*
* wakeup reason (ist jetzt unsigned 0-2) vielleicht als enum (NO,new_message,waiting)
*
* Specials: 
	- search id: verbesserung der Startposition (check milestones for a better/faster start point)
	- error ID hinterlegen
	- longer time for the next message (like real writing) letters * time = writetime
	- check nextId -> write time (+ is Id loaded? (check work))
* - (...) note if Tim is writing
* - a bigger Font size,
* - Debug output (IDs) in Game
*
*/ //TODO

#define TESTMODE				0			//testmodus the time to next Step is set to 1 min (0 default)
#define JUMPPOINT				500		//jump to this ID after connect (for quick bugfixing in the storyline) 0(default) = no jump

#define debug						0			//all debug are default on 0
#define debug_game			0			//debug for the gameaction function
#define debug_save			0
#define debug_parser 		0
#define debug_welcome		0
#define debug_settings  0
#define debug_timeline	0

#define FCT_SAVE				1			//enable(1 default) oder disable (0) the save (to persitend) function

#define MAX_TEXT_COUNT 	20
#define MAX_ANTW_COUNT 	2
#define MAX_MILE_COUNT 	10

#define MAX_ALIVES			1
#define MAX_DEADS				3

#define DEFAULT_DELAY		3			//secound between the messages

//Parser Settings
#define MAX_TEXT_LEN		100
#define MAX_HEAD_LEN 		20
#define MAX_LINE_LEN		(MAX_TEXT_LEN + MAX_HEAD_LEN)

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
	uint16_t start;						//start point
	uint16_t id;
	uint16_t next_id;
	uint8_t typ;
	uint8_t len;							//length of the message
	uint8_t hight;						//text hight
	unsigned char text[MAX_TEXT_LEN];
	uint16_t special;
} LINE;

/*
len;ID;nextID;TYP;Payload(Text or Int)
24;1;0;I;[found signal]
*/ //example line

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
uint16_t pointer	 	= 0;

uint16_t old_pointer 	= 1;		//for double line read check
uint16_t last_pointer = 0;		//the save the last found ID
uint16_t last_id 			= 0;
uint16_t line 				= 0;

//global Layer
static Layer 			*window_layer;

static Window 		*s_main_window;		//timeline
static Window 		*s_welc_window;		//welcome
static Window 		*s_info_window;		//info (Incomming Msg / waiting for input)
static Window 		*s_menu_window;		//settings menu
static Window			*s_mile_window;		//milestone menu
static Window 		*s_cred_window;		//credits 

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
static GBitmap 			*s_background_image;
static BitmapLayer 	*s_background_layer;

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
	save_line text[MAX_TEXT_COUNT];	//pointer of the Text Steps
	save_line antw[MAX_ANTW_COUNT];	//pointer of the antw Steps
	save_line	mile[MAX_MILE_COUNT];		//ID of the milestones
	
	uint8_t   active_text_count;				
  uint8_t   active_antw_count;
	uint16_t 	next_id;								//ID of the next Step	(0 = STOP)
	
	uint8_t		wakeup_reason;								//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;								//save the current position in the timeline
	
	//settings	
	bool 	vibe;													//vibration
	bool 	rapid_mode;										//fast mode
	bool 	sleep_time;											
	uint8_t font;												//font size 0= standard, 1= +1
	
	//history
	
	bool 			found_deads[MAX_DEADS];		//ID of the Achievments
	bool 			found_alives[MAX_ALIVES];
}__attribute__((packed, aligned(1))) SETTINGS;

//set defaults
SETTINGS settings = {
	//current game var
  .active_text_count = 0,
	.active_antw_count = 0,
	//.text 	= {NULL},
	//.antw 	= {NULL},
	.next_id 				= 1,
	.wakeup_reason 	= 0,
	.menu_index	 		= {0,0},
	
	//settings
	.vibe = true,
	.rapid_mode = false,
	.sleep_time = true,
	.font = 0,
	
	//history
	//.mile = {0},
	.found_deads = {false},
	.found_alives = {false}
};

// ------------------------------------ Parser ---------------

//convert a line_buffer part to a int
uint16_t parse_int(uint16_t start, uint16_t len){
	//if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse INT");}	
	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse INT (start %d, len %d)", start, len);}
	
	uint16_t out = 0;
	
	//create buffer
	//char *temp = (char*)malloc(len);				
	char temp[len];
	
	//copy elementes from line to buffer
	for(int j = 0; j < len; j++){		//len
		//if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "add char: %d", (uint)temp[j]);}	
		
		temp[j] = (char)line_buffer[j + start]; //start offset
	}
	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "INT-String: -->%s<--", (char*)temp);}	
	
	//convert buffer to int
	out = (uint16_t)atoi((char *)temp);
	
	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "INT = %d", out);}

	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse Special: %d (start %d, len %d)", out, start, len);}
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "line %s", line_buffer);
	return out;
}

//load the payload in the work Item
void parse_string(uint16_t start ){
	
	for(int i = 0; i < MAX_TEXT_LEN; i++){
		work.text[i] = (char)line_buffer[i + start - 1];
		
		//check for linebrake
		if(work.text[i] == 10){
			work.text[i-1] = 3;	//make linebrake to end of line
			work.text[i] = 0; 
			break;
		}
	}
	
	/*uint8_t text_len = work.len-start;

	for(uint8_t i = 0; i < text_len; i++ ){
		work.text[i] = (char)line_buffer[i + start - 1]; //start at 0 not at 1 
	}
	
	//work.text[text_len] = 0;
	work.text[text_len+1] = 0;	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Parse Text (Len %d)", text_len);
	*/
}

//durchsuche die Datei  													TODO: verbesserung der Startposition
bool search_id(uint16_t search_id){
	bool search = true;
	bool found 	= false;
	old_pointer = 1;
	
	//cancel if id 0
	if(search_id == 0){
		return false;
	}
	
	if(search_id >= last_id){
		//start from last known position
		pointer = last_pointer;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set last Pointer");
		//pointer = 0;
	}else{
		
		//check andere Bekante IDs (Milestone,...)
		//TODO: überprüfe bekannte ID für eine besserer Startposition
		
		//start from 0
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
		if(pointer + (uint16_t)MAX_LINE_LEN >= (uint16_t)filesize){
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
		work.text[0]	= 0;
		
		//read line
		resource_load_byte_range(rh, pointer, line_buffer, len);
		
		if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "LINE -->%s<--", (char *)line_buffer);}
		
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
				
				//corret pointer
				pointer = pointer + element_start;
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
						search = false;
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
					
					if(debug_parser){
						switch(work.typ){
							case INFO:
								APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Info");
							break;
							case BUSY:
								APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Busy");
							break;						
							case TEXT:
								APP_LOG(APP_LOG_LEVEL_DEBUG, "TYP: Text");
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
					}
					
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
						if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "TEXT: -->%s<--", (char*)work.text);}
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
			
			//set last Pointer
			last_pointer 	= pointer;	
			last_id 			= work.id;
			
			//suche abbrechen
			search = false;
			//break;
		}		
		
		//set new pointer
		pointer = pointer + work.len + 1; //+linebraker
		
		if(debug_parser){
			APP_LOG(APP_LOG_LEVEL_DEBUG, "set pointer (nex line starts at): %d", pointer);	
			APP_LOG(APP_LOG_LEVEL_DEBUG, "--------------- end line -----------------\n\n");
		}
	}//end while search
	
	if(!found){
		if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "ID NOT FOUND! clear work");}
		work.id 		= 0;
		work.start 	= 0;
	}
	
	if(debug_parser){APP_LOG(APP_LOG_LEVEL_DEBUG, "end search");}
	
	return found;
}

bool search_id_at(uint id, uint start_point){
	last_id 			= id;
	last_pointer 	= start_point;
	
	return search_id(id);
}


// ------------------------------------- Game function ------------------------

static void clear_menu(){
	settings.active_text_count = 0;
	settings.active_antw_count = 0;
	
	menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,0), MenuRowAlignCenter, false);
}

static void setWakeup(uint8_t reason, int min){
	//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	
	wakeup_cancel_all();
	int addHour = 0;

	//going faster in TEST- oder Rapid-Mode
	if(TESTMODE){
		min = 1;
	}	
	if(settings.rapid_mode && reason == 1){	//speed up the wait time in rapid mode
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
	settings.wakeup_reason = reason;	//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)

	//save settings
	//save_settings();
}

static void wakeup_handler() {
  
	//wakeup_cancel_all();
	if (settings.vibe){
		vibes_double_pulse();	
		vibes_double_pulse();
	}	
	
	//reset the wakeup reason
	settings.wakeup_reason = 0;			//wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	
	//set new reminder
	//setWakeup(0,24*60);	//1day
	
	//show Info screen
	window_stack_pop_all(false);
	window_stack_push(s_info_window, PBL_IF_ROUND_ELSE(false, true));	
}

static void set_text_hight(LINE *line){  //-> call set_text_hight(&text[1]);
	//Fehlerabfang
	if(line->id == 0){
		if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "SORRY: LINE ID is 0 (setTextHight)");}
		//return 0;
	}else{

		if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "calc Texthight for text: %s", line->text);}

		TextLayer *text_layer;

		GFont font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18 : FONT_KEY_GOTHIC_24));

		//INFO & ANTW are bold
		if(line->typ == INFO || line->typ == ANTW){ 
			font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
		}	

		//get the size 
		#if PBL_ROUND
		text_layer = text_layer_create(GRect(0, 0, 130, 500));
		#else
		text_layer = text_layer_create(GRect(0, 0, 128, 500));
		#endif
//TODO
		text_layer_set_text(text_layer, (char*)line->text);
		text_layer_set_overflow_mode(text_layer, GTextOverflowModeWordWrap);
		text_layer_set_font(text_layer, font);	
		GSize size = text_layer_get_content_size(text_layer);

		text_layer_destroy(text_layer);

		//if(size.h < 37){size.h = 37;}
		if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "TEXT HIGHT: %d", size.h);}

		//set line hight
		line->hight = size.h;	
	}
}

static void addText(){
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> addText mit id: %d Text_array_id: %d", work.id, settings.active_text_count);}

	//overflow protection
	if(settings.active_text_count+1 == MAX_TEXT_COUNT){
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: to many Text Messages in the Menu: i delete the first one");}
		
		//erste Nachrichten löschen (Like a fifo) + Menu_select--
		for(int x = 1; x <= settings.active_text_count; x++ ){
			//shift all msg in the que
			text[x-1] = text[x];
		}
		
		//go down with menu_index
		settings.menu_index.row--;
		
		settings.active_text_count--;
	}
	
	//add BUSY textline
	if(work.typ == BUSY){
		for(uint i = 0; i < strlen(lang.time_busy); i++){
			work.text[i] = (char)lang.time_busy[i];
		}
		work.text[strlen(lang.time_busy)] = 0;
		
		work.typ  = INFO;
	}
	
	//add Line from work to text array
	text[settings.active_text_count] = work;
	
	//calc text hight
	set_text_hight(&text[settings.active_text_count]);
	
	settings.active_text_count++;	
}

static void addAntw(){
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> addAntw mit id: %d Antwcount: %d", work.id, settings.active_antw_count);}

	//overflow protection
	if(settings.active_antw_count == MAX_ANTW_COUNT){
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: to many Text Response options in the Menu");}		
		
	}else{
		
		//Add Milestone Text
		if(work.typ == BUTTON_MILESTONE){
			for(uint i = 0; i < strlen(lang.set_milestone); i++){
				work.text[i] = (char)lang.set_milestone[i];
			}
			work.text[strlen(lang.time_busy)] = 0;
		}
		
		//add Line from work to text array
		antw[settings.active_antw_count] = work;
		
		//calc text hight
		set_text_hight(&antw[settings.active_antw_count]);	
		
		settings.active_antw_count++;
	}
}

static void addMilestone(){
	
	for(uint8_t x = 0; x < MAX_MILE_COUNT; x++){
		//check if milestone already exist
		if(mile[x].id == work.id){
			break; //exit
		}
		
		//find a free slot
		if(mile[x].id == 0){
			//save the milestone
			mile[x] = work;
			
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> ADD Milestone ID: %d @ slot: %d", work.id, x);}
			
			//exit
			break;
		}
	}
}

static void addAllAntw(uint16_t id){
	
	search_id(id);	
	addAntw();
	
	while(search_id_at(id, work.start+work.len)){
		addAntw();		
	}
}

static void setNextId(uint16_t id){
	
	//if(id == 0){
	//	id = 1;
	//}
	
	settings.next_id = id;
}

//-------------------------------- save - load settings -------------------

//TODO - CHECK this function
void save_settings() {		
	//write the current Lines (text, antw & mile) the the save_line in the setting (start, id)
	//Text Lines
	if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVE: prepare the settings to save");}
	
	uint8_t x = 0;
	if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> SAVE: prepare text");}
	for (x = 0; x < MAX_TEXT_COUNT; x++){
		if(x < settings.active_text_count){
			settings.text[x].start = text[x].start;
			settings.text[x].id		= text[x].id;
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "---> TEXT slot %d; ID: %d; start: %d ", x,text[x].id, text[x].start);}
		}else{
			settings.text[x].start = 0;
			settings.text[x].id		= 0;
		}
	}
	//ANTW Lines
	if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> SAVE: prepare antw");}
	for (x = 0; x < MAX_ANTW_COUNT; x++){
		if(x < settings.active_antw_count){
			settings.antw[x].start = antw[x].start;
			settings.antw[x].id		= antw[x].id;
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "---> ANTW slot %d; ID: %d; start: %d ", x,antw[x].id, antw[x].start);}
		}else{
			settings.antw[x].start = 0;
			settings.antw[x].id		= 0;
		}
	}
	//Milestine Line
	if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> SAVE: prepare milestone");}
	for (x = 0; x < MAX_MILE_COUNT; x++){
		if(mile[x].id > 0){
			settings.mile[x].start = mile[x].start;
			settings.mile[x].id		= mile[x].id;
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "---> MILE slot %d; ID: %d; start: %d ", x,mile[x].id, mile[x].start);}
		}else{
			settings.mile[x].start = 0;
			settings.mile[x].id		= 0;
		}
	}
	
	//save data	to persitent	
	if(FCT_SAVE){persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));}
	
	if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "SAVED %d bytes", (int)sizeof(settings));}
}

//TODO - CHECK - restore the WORK LINES (text, antw & mile) from the settings data (start, id)
void load_settings() {  
	
	if(FCT_SAVE)
	{
		if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOAD settings");}
		
		//check data in the memory		
		if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == (int)sizeof(settings)){

			//load data
			persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	
			
			settings.active_text_count = 0;
			settings.active_antw_count = 0;
			
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOADED %d bytes", (int)sizeof(settings));}

			//restore the Line_text, line_antw & Line_mile array from save_line (start,id) data
			for(int x = 0; x < MAX_TEXT_COUNT; x++){
				if(settings.text[x].id > 0){
					if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--->add TEXT %d; ID: %d; start: %d ", x,settings.text[x].id, settings.text[x].start);}
					//find id settings.text[x].id at start_point settings.text[x].start
					if(search_id_at(settings.text[x].id, settings.text[x].start)){
						//write line in text[x]
						addText();
						if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "added");}
					}
				}
			}
			
			//antw
			for(int x = 0; x < MAX_ANTW_COUNT; x++){
				if(settings.antw[x].id > 0){
					if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--->add ANTW %d; ID: %d; start: %d ", x,settings.antw[x].id, settings.antw[x].start);}

					//find id settings.text[x].id at start_point settings.text[x].start
					if(search_id_at(settings.antw[x].id, settings.antw[x].start)){					

						//write line in text[x]
						addAntw();
						if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "added");}
					}
				}
			}
			
			//milestone
			for(int x = 0; x < MAX_MILE_COUNT; x++){
				
				if(settings.mile[x].id > 0){
						
					if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "--->add MILE %d; ID: %d; start: %d ", x,settings.mile[x].id, settings.mile[x].start);}

					//find id settings.text[x].id at start_point settings.text[x].start
					if(search_id_at(settings.mile[x].id, settings.mile[x].start)){

						//write line in text[x]
						addMilestone();
					}
				}
			}
			
			
			/*
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
			*/
			
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "LOADED Data restored");}

		}else{
			//memory not okay			
			//start with default settings
			if(debug_save){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: no savegame found or wrong size: %d bytes found, %d bytes expected", (int)sizeof(settings), (int)persist_get_size(SETTINGS_KEY) );}
		}
	}

} 

// ------------------------------------ Game Main Function ----------------

//TODO
void game_action(void *data){
	//Jumppoint, just for debuging
	if(JUMPPOINT > 0){
		if(settings.next_id == 5){settings.next_id = JUMPPOINT;}	//Jumppoit for tests
	}
	
	if(debug_game){
		APP_LOG(APP_LOG_LEVEL_DEBUG, " ----- GAME ACTION start -----");
		APP_LOG(APP_LOG_LEVEL_DEBUG, "nextID: %d", settings.next_id);
	}
	
	//find next ID
	if(!search_id(settings.next_id)){
		//Id not found
		if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: ID %d not found", settings.next_id);}	
															 
		//errorhandling
		return;
	}else{
								
		if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "FOUND this ID");}	
		
		//found the ID
		
		uint timer 	= 0;
		bool go_on 	= true;
		bool update_menu = false;	
	
		//save the last file_pointer & id
		//last_pointer 	= work.start;
		//last_id				= work.id;		
		
		//no waiting in rapid_mode or Error
		//if(!settings.rapid_mode){																// TODO ERROR handling   && step->id != errorId
			if(settings.wakeup_reason != 0){
				//do nothing while waiting for wakeup
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "Waiting for WAKEUP, wakeup_reason = %d", settings.wakeup_reason);}
				return;	
			}
		//}
		
		if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "Prozessing Step ID: %d, nextId: %d, type: %d", work.id, work.next_id, work.typ);}
		
		switch(work.typ){
			case INFO:
				//is the same like Text
//INFO & TEXT			
			case TEXT:
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "GameAction: add Text text: %s", (char*)work.text);}
				addText();
				timer = DEFAULT_DELAY;
				update_menu = true;
				setNextId(work.next_id);
			break;
//ANTW
			case ANTW:
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "GameAction: add Antw id: %d", work.id);}
				addAllAntw(work.id);
				
				settings.next_id = 0;
				update_menu = true;
				go_on 			= false;

				setWakeup(2, 30);	//set reminder
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "set wakeup  ANTW - id: %d", settings.wakeup_reason);}
			break;
//BUSY
			case BUSY:
				/*
				for(uint i = 0; i < strlen(lang.time_busy); i++){
					work.text[i] = (char)lang.time_busy[i];
				}
				work.text[strlen(lang.time_busy)] = 0;
				//work.text = (char)lang.time_busy;
				//strcpy(work.text, lang.time_busy);
				work.typ  = INFO;
				*/
				addText();
				timer = work.special;
				update_menu = true;
				setNextId(work.next_id);
				//go_on = false;
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "BUSY for %d min", work.special);} 
			break;			
//MILESTONE	add a Milestone the Milestone menu
			case MILESTONE:
				setNextId(work.next_id);	//set next step and go on
				addMilestone();				//save this milestone
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "set Milestone");}
			break;		
//FOUND ALIVE		
			case FOUND_ALIVE:
				//add dead ID to the array
				settings.found_alives[work.special] = true;			
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "SET FOUND_ALIVE #%d", work.special);}
				//override the delay to go on
				timer = 0;
				setNextId(work.next_id);
			break;
//FOUND DEAD
			case FOUND_DEAD:
				//add dead ID to the array
				settings.found_deads[work.special] = true;			
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "SET FOUND_DEAD #%d", work.special);}
				//override the delay to go on
				timer = 0;
				setNextId(work.next_id);
			break;		
//Button Milestone
			case BUTTON_MILESTONE:
				/*
				for(uint i = 0; i < strlen(lang.set_milestone); i++){
					work.text[i] = (char)lang.set_milestone[i];
				}
				work.text[strlen(lang.time_busy)] = 0;
				//work.text[strlen(lang.time_busy)] = 10;
				*/
				addAntw();
				setNextId(0);
			
				wakeup_cancel_all();
				update_menu = true;
				go_on = false;
			break;
//END
			case END:

				//settings.next_step = NULL;
				setNextId(0);
				go_on = false;

				//cancel all wakeup
				wakeup_cancel_all();

				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "END OF GAME");} 

			break;
//default
			default:
				return;
			break;
		} //end switch
		
		
		//update the menu layer
		if(update_menu){
			menu_layer_reload_data(s_menu_layer);
			if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "update the menu");} 
		}
		
		//rapidmode
		//if(settings.rapid_mode){
			//timer = 0;
		//}		
		if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "set timer: %d sec", timer);}
		
		if(go_on){
			if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "GO ON :)");} 
			//start the timer
			if(timer > 4){ //extra long timer with wakeup reason: wait for new message 
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMER: set long timer");} 
				setWakeup(1, timer);
			}else	if(timer>0){																	//time in sec
				s_timer = app_timer_register(timer*1000, game_action, NULL);
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMER: %d sec", timer);} 
			}else{																							//now
				//short Timer
				s_timer = app_timer_register(500, game_action, NULL);
				if(debug_game){APP_LOG(APP_LOG_LEVEL_DEBUG, "TIMER: short");} 
			}
		}
		
		#if PBL_RECT
		//auto scroll down
		if(settings.active_text_count < 3){
			//menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,settings.active_text_count), MenuRowAlignCenter, true);
			menu_layer_set_selected_next(s_menu_layer, false, MenuRowAlignCenter, PBL_IF_ROUND_ELSE(false, true));
		}	
		#endif
		
	}//end ID found
	
	if(debug_game){
		APP_LOG(APP_LOG_LEVEL_DEBUG, " ----- GAME ACTION ende -----");
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d \n", (int)heap_bytes_free());
	}	
}

// ------------------------------------ Timeline Menu Layer callbacks --------------- jear 

//timeline select
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
	LINE *line;
	
	switch (cell_index->section){
		case 0:	//Empfangs Items
			//open the settings menu
			window_stack_push(s_menu_window, PBL_IF_ROUND_ELSE(false, true));	
		break;
		case 1: //Antwort Items
		
			setWakeup(TEXT, 60);	//set a reminder.
			
			//step = settings.active_antw[cell_index->row];		  
			line = &antw[cell_index->row];

			if(line->typ == ANTW){
				clear_menu();
				
				//add antw to Messages
				work = antw[cell_index->row];
				addText();
				
				setNextId(line->next_id);

				//zwischenstand speichern
				//save_settings();

				if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "INFO: Go to next Step with ID: %d", line->next_id);}

				settings.wakeup_reason = 0;

				//go on
				game_action(NULL);
			}
			if(line->typ == BUTTON_MILESTONE){
				//show milestones
  			window_stack_push(s_mile_window, PBL_IF_ROUND_ELSE(false, true));	
			}
				
		break;
		default: break;
	}
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
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw header");}
	
	// Determine which section we're working with
  switch (section_index) {
    case 0:
        // Draw title text in the section header
        menu_cell_basic_header_draw(ctx, cell_layer, lang.time_msg);
      break;
    case 1:
        // Draw title text in the section header
				if(settings.active_antw_count > 0){
					menu_cell_basic_header_draw(ctx, cell_layer, lang.time_send);	
				}else{
					if(settings.wakeup_reason != 1){		//0 = nothing, 1 = text, 2= antw
						if(settings.next_id > 3){	//TODO dont write anything when its a info message
							menu_cell_basic_header_draw(ctx, cell_layer, lang.time_reseiv);
						}else{
							menu_cell_basic_header_draw(ctx, cell_layer, "");
						}
					}else{
						menu_cell_basic_header_draw(ctx, cell_layer, lang.time_wait);
					}
				}
        
      break;
  }
}

static int16_t 	menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> cell height section: %d row: %d", cell_index->section, cell_index->row);}
	//return 18+26;

	switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)		
			if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> text id %d", text[cell_index->row].id);}
			return text[cell_index->row].hight +26;	
		break;
		case 1:		//zweite Section (Antworten)
			if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "--> antw id %d", antw[cell_index->row].id);}
			return antw[cell_index->row].hight +26;
		break;
		default: return 0;
	}
	//return 50; //test ouput
}

static void 		menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
 
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "draw row");}

	LINE *line;
	
	GFont font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18 : FONT_KEY_GOTHIC_24));
	GPath *s_path = s_path_1;
	
	graphics_context_set_fill_color(ctx, timeline_box_fill_color);
  graphics_context_set_stroke_color(ctx, timeline_box_border_color);
	graphics_context_set_stroke_width(ctx, 2);
	//gpath_move_to(s_path, GPoint(0,0));
	
	// Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:			//erste Section (Nachrichten)    	
			
			line = &text[cell_index->row];
		
			if(line->typ == INFO || line->typ == ANTW){
				font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
			}
		
			if(line->hight > 18){s_path = s_path_2;}
			if(line->hight > 36){s_path = s_path_3;}
			if(line->hight > 54){s_path = s_path_4;}
			if(line->hight > 72){s_path = s_path_5;}	
		
			//draw background
			#ifdef PBL_COLOR
			gpath_draw_filled(ctx, s_path);
			#endif
			gpath_draw_outline(ctx, s_path);			
		
			graphics_draw_text(ctx, 
				(char*)line->text,
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
     
			line = &antw[cell_index->row];
		
			if(line->typ == INFO || line->typ == ANTW){
				font = fonts_get_system_font((settings.font == 0 ? FONT_KEY_GOTHIC_18_BOLD : FONT_KEY_GOTHIC_24_BOLD));
			}
		
			if(line->hight > 18){s_path = s_path_2;}
			if(line->hight > 36){s_path = s_path_3;}
			if(line->hight > 54){s_path = s_path_4;}
			if(line->hight > 72){s_path = s_path_5;}
			
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
				(char*)line->text,
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
	
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "show timeline window");}
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
	
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "set timeline callbacks");}
	
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
		
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "timeline callbacks ready");}
	
	//Menu config
	menu_layer_set_normal_colors(s_menu_layer, GColorClear, timeline_text_color);
	menu_layer_set_highlight_colors(s_menu_layer, GColorClear, timeline_text_color);
	#if PBL_ROUND
		menu_layer_pad_bottom_enable(s_menu_layer, true);
	#else
		menu_layer_set_center_focused(s_menu_layer, false);
	#endif
	
	
	//set menu index to the old postionen
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "set menu index");}
	menu_layer_set_selected_index(s_menu_layer, settings.menu_index, MenuRowAlignCenter, false);
	
	//start the game
	if(debug_timeline){APP_LOG(APP_LOG_LEVEL_DEBUG, "start game action");}
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
	if(debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: select");}
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
					window_stack_push(s_mile_window, PBL_IF_ROUND_ELSE(false, true));	
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
					window_stack_push(s_welc_window, PBL_IF_ROUND_ELSE(false, true));	
				break;
				case 1:
					//show credits
					window_stack_push(s_cred_window, true);
				break;
				case 2:	//reset data
					//delete the savegame
					persist_delete(SETTINGS_KEY);
				
					// reset settings
					settings.vibe = true;
					settings.rapid_mode = false;
					settings.sleep_time = true;
	
					//clear history
					for(int x = 0; x < MAX_MILE_COUNT; x++){
						mile[x].id = 0;
					}
					for(int x = 0; x < MAX_DEADS; x++){
						settings.found_deads[x] = false;
					}
					for(int x = 0; x < MAX_ALIVES; x++){
						settings.found_alives[x] = false;
					}
					
					//restart game
					setNextId(0);
					wakeup_cancel_all();
					settings.wakeup_reason = 0;
	
					clear_menu();
				
					window_stack_pop_all(false);
					window_stack_push(s_welc_window, PBL_IF_ROUND_ELSE(false, true));	
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
	if(debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: num row");}
  switch (section_index) {
    case 0:			
			//settings
			return 4;
    case 1:
			//achievments
      return 2;
		case 2:
			//credits + welc + reset
			return 3;
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
	if(debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: draw header");}
  switch (section_index) {
    case 0:
        // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, lang.set_settings);
      break;
    case 1:
        // Draw title text in the section header
			menu_cell_basic_header_draw(ctx, cell_layer, lang.set_achiev);        
    	break;
		case 2:
			menu_cell_basic_header_draw(ctx, cell_layer, lang.set_extra);        
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
					menu_cell_basic_draw(ctx, cell_layer, lang.set_vibe, (settings.vibe ? lang.on : lang.off), NULL);
				break;
				case 1:
					menu_cell_basic_draw(ctx, cell_layer, lang.set_rapid, (settings.rapid_mode ? lang.on : lang.off), NULL);
				break;
				case 2:
					menu_cell_basic_draw(ctx, cell_layer, lang.set_rest_time, (settings.sleep_time ? "22:00-8:00" : lang.off), NULL);
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
					menu_cell_basic_draw(ctx, cell_layer, lang.set_milestone, NULL, NULL);	//"go back in time"
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
					menu_cell_basic_draw(ctx, cell_layer, buf, lang.set_alive, NULL);		
				break;
				case 1: 					
					for(int x = 0; x < MAX_DEADS; x++){
						if(settings.found_deads[x]){i++;}
					}
					snprintf(buf, sizeof(buf), "%d / %d", i, MAX_DEADS);
					menu_cell_basic_draw(ctx, cell_layer, buf, lang.set_die, NULL);
				break;
			}
		break;
		case 2:
			switch (cell_index->row){
				case 0:
					menu_cell_basic_draw(ctx, cell_layer, lang.set_intro, NULL, NULL);
				break;
				case 1:
					menu_cell_basic_draw(ctx, cell_layer, lang.set_credits, NULL, NULL);
				break;
				case 2:
					menu_cell_basic_draw(ctx, cell_layer, lang.set_reset, NULL, NULL); //"delete all"
				break;
			}
		break;
  }
}

static int16_t  set_get_separator_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_contex){
	return 0;	
}

static void 		set_window_load(Window *window) {
  
	if(debug_settings){APP_LOG(APP_LOG_LEVEL_DEBUG, "start setting menu");}
	
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
		setNextId(0);
		wakeup_cancel_all();
		settings.wakeup_reason = 0;

		clear_menu();
		window_stack_pop_all(false);
		window_stack_push(s_welc_window, PBL_IF_ROUND_ELSE(false, true));	
		
	}else{
		//Milestone
		//cell_index->row-1
		setNextId(mile[cell_index->row-1].next_id);
		
		wakeup_cancel_all();
		settings.wakeup_reason = 0;

		clear_menu();
		window_stack_pop_all(false);
		window_stack_push(s_main_window, PBL_IF_ROUND_ELSE(false, true));	
	}		
}

static uint16_t mil_get_num_sections_callback(MenuLayer *menu_layer, void *data) {	
	return 1;
}

static uint16_t mil_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
	//if(debug_milestone){APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTINGS: num row");}
	int rows = 1;
	
	if(section_index == 0){
		//count milestones
		for(int x = 0; x < MAX_MILE_COUNT; x++){
				if(mile[x].id > 0){
					rows++;
				}else{
					break;
				}
		}
		return rows;
	}else{
		return 0;
	}
}

static int16_t 	mil_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void 		mil_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
	if(section_index == 0){
		menu_cell_basic_header_draw(ctx, cell_layer, lang.mile_header);
	}
	/*
  switch (section_index) {
    case 0:
        // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, lang.mile_header);
    break;
  }
	*/
}

static void 		mil_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
	 	
	//Menu Text
	if(cell_index->row == 0){
		//restart
		menu_cell_basic_draw(ctx, cell_layer, lang.mile_1row, NULL, NULL);
	}else{
		//Milestones
		menu_cell_basic_draw(ctx, cell_layer, (char*)mile[cell_index->row-1].text, NULL, NULL);
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
	window_stack_push(s_main_window, PBL_IF_ROUND_ELSE(false, true));
}

void welc_click_config_provider() {
  
	window_single_click_subscribe(BUTTON_ID_SELECT, welc_select_click_handler);
	//window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  //window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void welc_window_load(Window *window) {
	
	if(debug_welcome){APP_LOG(APP_LOG_LEVEL_DEBUG, "start welcome window");}
	
	window_layer = window_get_root_layer(window);
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
	
	window_layer = window_get_root_layer(window);
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
	
	window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_bounds = GRect(0, 0, 139, 2000);
	
	s_welc_layer = scroll_layer_create(bounds);
	
	scroll_layer_set_click_config_onto_window(s_welc_layer, window);
	scroll_layer_set_callbacks(s_welc_layer, (ScrollLayerCallbacks){
		.click_config_provider = cred_click_config_provider
	});
	
	window_set_background_color(s_cred_window, credits_bg_color);
	
	//draw Layer inputs	
	//headline
	s_welc_text1 = text_layer_create(GRect(0, 0, max_bounds.size.w, 100));
	text_layer_set_text(s_welc_text1, lang.name);	//Headline
  text_layer_set_font(s_welc_text1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(s_welc_text1, GTextAlignmentCenter);	
	text_layer_set_background_color(s_welc_text1, GColorClear);
	text_layer_set_text_color(s_welc_text1, credits_text_color);
	//credits
	s_welc_text2 = text_layer_create(GRect(5,60, max_bounds.size.w-20, max_bounds.size.h));
	text_layer_set_text(s_welc_text2, lang.credits);			//description text
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
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free());}
	
	//set resource handle
	rh = resource_get_handle(RESOURCE_ID_STORY_EN);
	//rh = resource_get_handle(RESOURCE_ID_TEST);
	//set filesize
	filesize = resource_size(rh);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Story File Size: %d Byte", filesize);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d Zeichen", filesize);
	
	//check_file();
	
	//register wakeup_handler
	wakeup_service_subscribe(wakeup_handler);
	
	//seed random generator
	srand(time(NULL));
	
	//load settings
	load_settings();
		
	//------------ init pages
	
	//init timeline
  s_main_window = window_create();	//timeline
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load 	= main_window_load,
    .unload = main_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: timeline");}
	
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
	
	//init credits layer
	s_cred_window = window_create();
  window_set_window_handlers(s_cred_window, (WindowHandlers){
    .load 	= cred_window_load,
    .unload = cred_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: credits");}
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Free heap: %d", (int)heap_bytes_free());}
	
	//---------- lanch----
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "launche_reason: %d", launch_reason());}
	
	//App start from wakeup
  if(launch_reason() == APP_LAUNCH_WAKEUP) { 

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START wakeup handler");}    
		wakeup_handler();

	//normal app system start
  } else {																		

		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "START normal start - no wakeup");}
		
		if(settings.next_id > 0 && settings.active_text_count == 0){
			
			if(settings.next_id == 1){ 		//first run
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Welcome window... (FIRST RUN)");}
			
				wakeup_cancel_all();
				settings.wakeup_reason = 0;		//0 = nothing, 1 = new text, 2= wait for antw
				
				//setNextStep(1);
	
				#if PBL_ROUND
					window_stack_push(s_welc_window, false);	
				#else
					window_stack_push(s_welc_window, true);	
				#endif
			}else{													//show currend game
				if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window (SHOW CURREND GAME, GO ON WITH NEXT STEP)");}

				window_stack_push(s_main_window, PBL_IF_ROUND_ELSE(false, true));
			}
	
		}else{			
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "PUSH Timeline window (next_step == NULL)");}

			window_stack_push(s_main_window, PBL_IF_ROUND_ELSE(false, true));	
		}
				
  }  
	
}

static void deinit(void) {
	save_settings();
	
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
