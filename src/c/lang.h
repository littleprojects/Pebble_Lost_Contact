#include <pebble.h>

typedef struct {
	//welcome page
	char 			*name;
	char 			*desc;
	
	//info_message
	char			*text_wait;
	char 			*text_msg;
	
	//timeline
	char			*time_msg;
	char			*time_send;
	char			*time_reseiv;
	char			*time_wait;
	char			*time_busy;
	
	//milestone
	char			*mile_header;
	char			*mile_1row;
	
	//settings
	char			*on;
	char			*off;
	
	char			*set_settings;
	char			*set_vibe;
	char			*set_rapid;
	char			*set_rest_time;
	char			*set_milestone;	
	char			*set_achiev;
	char			*set_alive;
	char			*set_die;
	char			*set_extra;
	char			*set_intro;
	char			*set_reset;
	char			*set_credits;
	
	//credits
	char			*credits;
} LANG;

extern LANG lang;