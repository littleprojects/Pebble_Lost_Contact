#include <pebble.h>
#include "lang.h"

LANG lang  = {
	//welcome page
	.name = "LAST SIGNAL\nMoonbase",
	.desc =	"Save Tom!\n\nTom is in trouble and only YOU can help him!\n\nYou are his last remaining connection. Help him with the right decisions and secure his survival!\n",
	
	//info page
	.text_wait 		= "Tom waits for a response.",	
	.text_msg 		= "Receive new message.",
	
	//timeline
	.time_msg			= "Messages:",
	.time_send		= "send:",
	.time_reseiv	= "reseiving data",//"Tom is writing...",
	.time_wait		= "Wait for Tom",
	.time_busy		= "Tom is busy.",
	
	//milestone menu
	.mile_header 	= "Milestones:",
	.mile_1row 		= "Beginning",
	
	//settings menu
	.on						= "on",
	.off					= "off",
	
	.set_settings = "Settings:",
	.set_vibe			= "Vibration",
	.set_rest_time= "Rest period",
	.set_rapid		= "Rapid mode",
	.set_milestone= "Milestones ",
	.set_achiev		= "Achievements:",
	.set_alive		= "found ways to survive",
	.set_die			= "found ways to die",
	.set_extra		= "Extras:",
	.set_intro		= "Show intro",
	.set_credits	= "Show credits",
	.set_reset		= "Reset game",
	
	//credits
	.credits = "Version 0.9\n\nDavid Lenk 2016\n\n270 Messages\n\nThis game is just a prototype.\n\nIf you like it and you want more, give me a heart or send me a email."
};

extern LANG lang;