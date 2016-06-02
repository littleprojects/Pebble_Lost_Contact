#include <pebble.h>

/* TODO
*
*	Dynamische Text lentgh im struct
*
*/ //TODO

#define degug					1
#define debug_parser 	0

#define MAX_TEXT_LEN	100
#define MAX_HEAD_LEN 	20
#define MAX_LINE_LEN	(MAX_TEXT_LEN + MAX_HEAD_LEN)

#define MAX_TEXT_COUNT 20
#define MAX_ANTW_COUNT 2
#define MAX_MILE_COUNT 10

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


static Window *s_window;
static TextLayer *s_text_layer;

static void init(void) {
	
	//set resource handle
	rh = resource_get_handle(RESOURCE_ID_STORY_EN);
	//set filesize
	filesize = resource_size(rh);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d Byte", filesize);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Size: %d Zeichen", filesize);
	
	check_file();
	
/*
	APP_LOG(APP_LOG_LEVEL_DEBUG, "\n\nSEARCH_ID 4\n\n");
	search_id(4);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK start: %d", work.start);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK len: %d", work.len);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK id: %d", work.id);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK nid: %d", work.next_id);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK typ: %d", work.typ);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK text: %s", (char*)work.text);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK spec: %d", work.special);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "\n\nSEARCH_ID 6\n\n");	
	search_id(6);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK start: %d", work.start);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK len: %d", work.len);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK id: %d", work.id);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK nid: %d", work.next_id);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK typ: %d", work.typ);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK text: %s", (char*)work.text);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "WORK spec: %d", work.special);
*/
	
	
	// Create a window and get information about the window
	s_window = window_create();
  Layer *window_layer = window_get_root_layer(s_window);
  GRect bounds = layer_get_bounds(window_layer);
	
	//str to int atoi((const char *)s_buffer)
	
  // Create a text layer and set the text
	s_text_layer = text_layer_create(bounds);
	text_layer_set_text(s_text_layer, "test");//(const char *)buffer);
  
  // Set the font and text alignment
	text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

	// Add the text layer to the window
	layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_text_layer));
  
  // Enable text flow and paging on the text layer, with a slight inset of 10, for round screens
  text_layer_enable_screen_text_flow_and_paging(s_text_layer, 10);

	// Push the window, setting the window animation to 'true'
	window_stack_push(s_window, true);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

static void deinit(void) {
	// Destroy the text layer
	text_layer_destroy(s_text_layer);
	
	// Destroy the window
	window_destroy(s_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
