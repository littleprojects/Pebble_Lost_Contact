#include <pebble.h>

typedef struct {
	char 			*name;
	char 			*desc;
	
	char			*text_wait;
	char 			*text_msg;
	
	char			*credits;
} LANG;

extern LANG lang;