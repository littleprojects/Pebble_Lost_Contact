#include "pebble.h"

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
*/

/*
	delete the game before you reinstall it
	- after reinstall the pointer of the savegame points to nowhere and the game crash
	- same the game with the game ids -> needs more resources to find the Steps
	
	Idee: Gewählte Anwort mit in Timeline aufnehmen
			*	ältere Elemente immer löschen (First in, first out que)
			* que in Add Text möglich (clear Menu anpassen/ nur noch antw count = 0)(selced index anpassen --)
				
	TODO:
	Save 
		- history
		- game stats
			* Stats
			* achievements
	Settings
	  - Go back (History)
	Stats
		- credits		
	
	Nice Backgrounds + colors
	
	Special:		
		- Fake System Data in Settings
			* Empfang
			* Akku
			* Empfangene Nachrichten
		- Incoming Msg / Waiting for Response
			* Zeit (Status bar)
			
	Welcome Card:
		- add action bar. (up/down/ play)
*/

static bool debug = false;
#define MAX_TEXT 20
#define MAX_ANTW 5

#define SETTINGS_KEY 1

#define welcome_text_color				GColorBlack
#define welcome_bg_color					PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)

#define timeline_text_color 			GColorBlack
#define timeline_box_border_color GColorBlack
#define timeline_box_fill_color   PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite)

// Blue-Black OxfordBlue-White
#define settings_text_color				PBL_IF_COLOR_ELSE(GColorBlack, GColorBlack)
#define settings_bg_color					PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)
#define settings_text_color_hi		PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite)
#define settings_bg_color_hi			PBL_IF_COLOR_ELSE(GColorDarkGreen, GColorBlack)

#define credits_text_color				GColorBlack
#define credits_bg_color					PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite)



enum TYPE{
  INFO,			//create a Info Textbox
	TEXT,			//create a normal Textbox
	ANTW,			//create a answer
	CLEAR,		//clear all messages
	SHOW_OVERLAY,
	HIDE_OVERLAY,
	DIE_AKKU,
	SHOW_WON,
	SHOW_DIE,
	ACHIEVEMENT_X,
	CHANGE_SETTING_X,
	SPECIAL,
};

typedef struct {
	int 			id;
	int 			next;
	uint16_t 	delay;
	enum TYPE type;
	char 			*text;
} STEP;

typedef struct {
	char 			*name;
	char 			*desc;
	uint16_t 	step_count;
	char			*step_count_text;
	char			*version;
	char			*credits;
	STEP 			step[];
} GAME_FLOW;

// ---------------------------------- GAME DATA ----------------------------------------------------------------------
GAME_FLOW g = {
	.name = "Lost Contact\nMoonbase",
	.desc =	"Rette Tom.\n\nTom ist in Schwierigkeiten und nur du kannst ihm helfen.\n\nDu bist die letzte Verbindung die er hat. Helf Tom bei wichtigen Entscheidungen und sicher so sein Überleben.\n",
	.step_count = 265,						//WICHTIG - step_count immer aktuell halten
	.step = {
//ID, nextID, Delay, Type, Text, Special ID 
		{  0,   1,  1, INFO, "[Signal gefunden]"},
		{  1,   2,  1, INFO, "[Verbinde]"},
		{  2,   3,  1, INFO, "[Empfange Daten]"},
		{  3,   4,  1, TEXT, "Hallo..."},
		{  4,   5,  1, TEXT, "Geht das Ding? Ist da jemand..."},
		{  5,  10,  1, TEXT, "Hallo? Ich könnte wirklich etwas Hilfe gebrauchen."},
																						
		{  10, 11,  0, ANTW, "Hallo..."},
		{  10, 12,  0, ANTW, "Was ist den los?"},
																						// 8
		{  11, 13,  1, TEXT, "WOW, das alte Ding hier geht noch"},
		{  12, 13,  1, TEXT, "Ja, wenn ich das wüsste..."},
		{  13, 14,  1, TEXT, "Der Strom ist ausgefallen. Nur die Notbeleuchtung ist an."},
		{  14, 15,  1, TEXT, "Ziehmlich dunkel hier."},
		{  15, 20,  1, TEXT, "Hab zum glück diesen alten Notsender gefunden."},
																					
		{  20, 22,  0, ANTW, "Wer ist den da?"},
		{  20, 21,  0, ANTW, "Wo ist den hier?"},
																						//15
		{  21, 22,  1, TEXT, "Hier? Das sollte eigentlich immer noch die gute alte MB1 sein."},
		{  22, 23,  1, TEXT, "Ohh warte. Wo sind blos meine Manieren..."},
		{  23, 24,  1, TEXT, "Ich bin Tom. Student auf der MB1."},
		{  24, 25,  1, TEXT, "Entschuldige. Ich bin etwas durcheinander..."},
		{  25, 26,  1, TEXT, "Hier gab es noch nie einen Stromausfall."},
		{  26, 30,  1, TEXT, "Das ist gar nicht gut. Gar nicht Gut..."},
		
		{  30, 31,  0, ANTW, "Was ist den die MB1?"},
		{  30, 33,  0, ANTW, "MB1? Wo war das nochmal?"},	
																						//23
		{  31, 32,  1, TEXT, "MB1 ist die Moonbase1."},
		{  32, 33,  1, TEXT, "Viele vergessen die alte Kiste hier immer..."},
		{  33, 34,  1, TEXT, "Die MB1 ist die erste Station auf dem Mond."},
		{  34, 40,  1, TEXT, "Da war die NASA wirklich sehr creativ beim Namen: Moonbase1."},
		
		{  40, 41,  0, ANTW, "Und was machst du auf der MB1?"},
		{  40, 41,  0, ANTW, "Was studierst du denn?"},	
																						//29
		{  41, 42,  1, TEXT, "Ich studiere Mondgeschichte... "},
		{  42, 43,  1, TEXT, "Nicht weit von der MB1 ist damals Appolo 13 gelandet."},
		{  43, 44,  1, TEXT, "Deswegen bin ich hier. Ich schreibe meine Abschlussarbeit darüber."},
		{  44, 45,  1, TEXT, "Ohh Man, es tut so gut wieder mit jemand zu reden."},
		{  45, 46,  1, TEXT, "Naja, ich meine schreiben..."},
		{  46, 50,  1, TEXT, "Du bist der einzige den ich erreichen konnte."},
			
		{  50, 53,  0, ANTW, "Was ist den los?"},
		{  50, 51,  0, ANTW, "Warte. Mondgeschichte???"},	
																						//37
		{  51, 52,  1, TEXT, "Was denn... das war NC frei."},
		{  52, 53,  1, TEXT, "Ich hab gerade ein paar andere Sorgen."},
		{  53, 54,  1, TEXT, "Irgendwas stimmt nicht."},
		{  54, 55,  1, TEXT, "Seit Stunden ist der Strom weg. Und es ist gar keiner mehr da."},
		{  55, 56,  1, TEXT, "Ich hab bis vor einer Stunde hier gewartet bis jemand kommt. "},
		{  56, 57,  1, TEXT, "Dann hab ich mich ein bisschen umgesehen und diesen alten Sender gefunden."},
		{  57, 60,  1, TEXT, "Naja, er hat mich gefunden. Der Sender ist hier so das einzige das leuchtet."},
	
		{  60, 61,  0, ANTW, "Warte noch etwas."},
		{  60, 71,  0, ANTW, "Seh dich doch etwas um."},	
																						//46			
		{  61, 62,  1, TEXT, "OK. Wenn du das sagst."},
		{  62, 63,  1, TEXT, "Sicher hast du recht und der Strom gleich wieder da."},
		{  63, 64,  1, TEXT, "Es ist nur so dunkel."},
		{  64, 65,  1, TEXT, "Man könnte die Zeit nutzen und etwas schlafen."},
		{  65, 66,  1, TEXT, "Ich leg mich kurz hin."},
		{  66, 81, 10, INFO, "Tom ist beschäftigt."},
		
		{  71, 72,  1, TEXT, "OK. Gute Idee."},
		{  72, 73,  1, TEXT, "Es ist nur so verdamt dunkel."},
		{  73, 74,  1, TEXT, "Man kann gar nicht richtig sehen."},
		{  74, 76,  1, TEXT, "Warte, hier war doch irgendwo eine Taschenlampe."},
		{  75, 81, 10, INFO, "Tom ist beschäftigt."},
																						//57
		{  81, 82,  1, TEXT, "Hey, da bin ich wieder."},
		{  82, 83,  1, TEXT, "Du wirst es nicht glauben. Ich habe eine Taschenlampe gefunden."},
		{  83, 84,  1, TEXT, "Naja, eher ein Lämpchen. Hell ist die nicht gerade."},
		{  84, 85,  1, TEXT, "So, wohin jetzt?"},
		{  85, 90,  1, TEXT, "Soll ich mich hier in den Laboren umsehen oder in den Quatieren?"},
			
		{  90, 91,  0, ANTW, "Bleib im Labor."},
		{  90, 91,  0, ANTW, "Geh zu den Quatieren."},
//Umschauen																	//64
		{  91, 92,  1, TEXT, "Ist mir auch lieber."},
		{  92, 93,  1, TEXT, "Hoffentlich ist noch jemand da."},
		{  93, 94,  1, TEXT, "Mit dem bisschen Licht fühl ich mich schon etwas besser."},
		{  94, 95,  1, TEXT, "Aber es ist immernoch gespenstisch Still..."},
		{  95, 96,  1, TEXT, "Ich meld mich wieder wenn ich was gefunden habe."},
		{  96, 97, 30, INFO, "Tom ist beschäftigt."},
		
		{  97, 98,  1, TEXT, "Da bin ich wieder."},
		{  98,100,  1, TEXT, "Und ich glaube wir haben ein Problem..."},
		
		{ 100,101,  0, ANTW, "Hast du was gefunden?"},
		{ 100,102,  0, ANTW, "Was ist los?"},	
//Akku leer																//74
		{ 101,102,  1, TEXT, "Gefunden hab ich leider nichts."},
		{ 102,103,  1, TEXT, "Ich glaub der Akku vom Notsender ist bald leer."},
		{ 103,104,  1, TEXT, "Er hat angefangen zu piepsen."},
		{ 103,104,  1, TEXT, "Und der Ladeanzeige ist gelb."},
		{ 104,105,  1, TEXT, "Das ist echt wieder typisch..."},
		{ 105,106,  1, TEXT, "Wenn man was braucht, ist der Akku leer."},
		{ 106,110,  1, TEXT, "Das ist nicht mein Tag heute."},
		
		{ 110,111,  0, ANTW, "Der hälten noch ewig."},
		{ 110,121,  0, ANTW, "Such ein Ladegerät."},
//Akku ganz leer												//83
		{ 111,112,  1, TEXT, "Echt? Das ist ja super"},
		{ 112,113,  1, TEXT, "Dann schau ich mich noch etwas um"},
		{ 113,114, 30, INFO, "Tom ist beschäftigt."},
			
		{ 114,115,  1, TEXT, "Hallo."},
		{ 115,116,  1, TEXT, "Das piepsen ist noch nerviger geworden."},
		{ 116,117,  1, TEXT, "Und die Anzeige steht jetzt auf rot."},
		{ 117,118,  1, TEXT, "Ich sollt......"},
		{ 118,119,  1, INFO, "[Signal schwach]"},
		{ 119,1001, 1, INFO, "[Signal verloren]"},
//Akku ladegerät												//92
		{ 121,122,  1, TEXT, "Bin schon unterwegs."},
		{ 122,123,  1, TEXT, "Geb mir ein paar Minuten."},
		{ 123,124, 30, INFO, "Tom ist beschäftigt."},
			
		{ 121,122,  1, TEXT, "Hey, ich hab ein Netzteil mit USB Kabel gefunden."},
		{ 122,130,  1, TEXT, "Aber das bringt mir nichts ohne Strom."}, //95
		
		{ 130,131,  0, ANTW, "Gibt es Baterien?"},
		{ 130,133,  0, ANTW, "Habt ihr Solarzellen?"},		
//Batterien das Solar zellen					//99
		{ 131,132,  1, TEXT, "Ja, drüben beim Solarfeld."},
		{ 132,135,  1, TEXT, "Sie Speichern den Strom für die Nacht."},
			
		{ 133,134,  1, TEXT, "Ja, es gibt ein Solarfeld."},
		{ 134,135,  1, TEXT, "Die Idee gefällt mir."},	
			
		{ 135,136,  1, TEXT, "Aber dafür muss ich rüber in die andere Kuppel."},
		{ 136,140,  1, TEXT, "Ich beeile mich lieber..."},
//losgehen														105
		{ 140,141,  0, ANTW, "Ja, mach schnell."},
		{ 140,141,  0, ANTW, "Sei vorsichtig."},	
			
		{ 141,142,  1, TEXT, "Ist gut."},
		{ 142,143,  1, TEXT, "Bis gleich."},
		{ 143,144, 10, INFO, "Tom ist beschäftigt."},
			
		{ 144,150,  1, TEXT, "Ahhh... Hilfe."},
			
		{ 150,151,  0, ANTW, "Was ist los?"},
		{ 150,153,  0, ANTW, "Bist du Okay?"},	
//geräusch		112
		{ 151,152,  1, TEXT, "Ja, danke. "},
		{ 152,153,  1, TEXT, "Ich bin Okay. Glaub ich"},
		{ 153,154,  1, TEXT, "Da war ein Geräusch hinter mir."},
		{ 154,155,  1, TEXT, "So ein leises trippeln..."},
		{ 155,156,  1, TEXT, "Und dann ein schreckliches Kratzen auf Metal."},	
		{ 156,160,  1, TEXT, "Ich konnte nicht sehen was es ist."},
			
		{ 160,161,  0, ANTW, "Los, hinterher."},
		{ 160,190,  0, ANTW, "Schnell weg."},	
			
		{ 161,162,  1, TEXT, "Echt? Bist du sicher? "},
		{ 162,163,  1, TEXT, "Ich hab echt weiche Knie."},
		{ 163,164,  1, TEXT, "So fängt jeder Horrorfilm an."},
		{ 164,165,  1, TEXT, "Erst ein Geräusch. Und dann sind alle Tot..."},
		{ 165,170,  1, TEXT, "Und was ist mit Notsender? Der piepst noch."},

		{ 170,190,  0, ANTW, "Hast recht. Weg da."},
		{ 170,171,  0, ANTW, "Komm. Sieh nach."},	
//sieh nach							127
		{ 171,172,  1, TEXT, "Manooo, muss."},
		{ 172,173,  1, TEXT, "Aber wenn da ein Weltraum Monster auf mich wartet..."},	
		{ 173,174,  1, TEXT, "und mich für sein Frühstück hält."},	
		{ 174,175,  1, TEXT, "Dann bist du schuld."},
		{ 175,176, 10, INFO, "Tom ist beschäftigt."},	
			
		{ 176,177,  1, TEXT, "Puhhh, das ist ein Nervenkitzel."},	
		{ 177,178,  1, TEXT, "Aber hier ist nichts zu sehen."},	
		{ 178,179,  1, TEXT, "Was es auch war, es ist weg."},	
		{ 179,180,  1, TEXT, "Ich weiß nicht was mir mehr Angst macht..."},
		{ 180,181,  1, TEXT, "Das da was war oder das es jetzt weg ist."},	
			
		{ 181,182,  0, ANTW, "Keine Angst."},
		{ 181,182,  0, ANTW, "Ok, zurück zur Batterie."},
												//139
		{ 182,183,  1, TEXT, "Na du hast gut reden."},	
		{ 183,184,  1, TEXT, "Du sitzt auch nicht allein in einer dunklen Staion."},	
		{ 184,185,  1, TEXT, "Ich geh wieder zurück."},				
		{ 185,200, 30, INFO, "Tom ist beschäftigt."},
			
//schnell weg	- 190				//144
		{ 190,191,  1, TEXT, "Nicht lieber als das."},	
		{ 191,192,  1, TEXT, "Was war das?"},	
		{ 192,193,  1, TEXT, "Ich weiß nicht was unheilicher ist..."},
		{ 193,194,  1, TEXT, "Das da was ist."},	
		{ 194,195,  1, TEXT, "Oder das es jetzt wieder weg ist."},	
		{ 195,196,  1, TEXT, "Bin gleich im Batterie Raum."},
		{ 196,200, 30, INFO, "Tom ist beschäftigt."},
			
//batterien 200						//152
		
		{ 200,201,  1, TEXT, "So, ich bin bei den Batterien."},	
		{ 201,202,  1, TEXT, "Sie scheinen geladen zu sein."},
		{ 202,203,  1, TEXT, "Die Kontroll LED auf den Batterien sind Grün."},
		{ 203,210,  1, TEXT, "Er hat angefangen zu piepsen."},
			
		{ 210,211,  0, ANTW, "Warum gibt es dann kein Licht?"},
		{ 210,220,  0, ANTW, "Was ist da noch?"},
			
		{ 211,212,  1, TEXT, "Ich glaub die Staion wurde nur Anfangs nur durch Solarzellen versorgt."},
		{ 212,213,  1, TEXT, "Aber dann hat man einen Reaktor eingesetzt."},
		{ 213,214,  1, TEXT, "Hmm. Ob der Reaktor am Stromausfall schuld ist."},
		{ 214,215,  1, TEXT, "Nein. Der Sender piepst jetzt schneller."},	
		{ 214,220,  1, TEXT, "Die Akkuanzeige ist jetzt rot."},
//TODO:										//163
		{ 220,221,  0, ANTW, "Schließ das Ladekabel an."},
		{ 220,225,  0, ANTW, "Such ein Netzteil."},
			
		{ 221,222,  1, TEXT, "Das geht nicht."},
		{ 222,223,  1, TEXT, "Man braucht ein besonderen Stecker."},
		{ 223,224,  1, TEXT, "Ich such mal ein Gerät mit so einem Stecker."},
		{ 224,230,  5, INFO, "Tom ist beschäftig."},	
		
		{ 225,226,  1, TEXT, "Schon dabei..."},	
		{ 226,227,  5, INFO, "Tom ist beschäftigt."},
			
		{ 230,231,  1, TEXT, "Hey ich hab was:"},
		{ 231,232,  1, TEXT, "Sieht aus wie ein multi Ladegerät."},
		{ 232,233,  1, TEXT, "Man kann da ne menge dran einstellen."},
		{ 233,240,  1, TEXT, "Und Klemmen um Kabel anzuschließen."},
//Ladegerät gefunden			175
		{ 240,241,  0, ANTW, "Super!"},
		{ 240,241,  0, ANTW, "Perfekt."},	
			
		{ 241,242,  1, TEXT, "An dem Ding kann man eine Menge einstellen."},
		{ 242,243,  1, TEXT, "Der erste Schalter ist für:"},
		{ 243,244,  1, TEXT, "Gleichspannung oder Wechselspannung."},
		{ 244,245,  1, TEXT, "Was auch immer das ist..."},
		{ 245,246,  1, TEXT, "Es klingt ungesund."},
		{ 246,250,  1, TEXT, "Was soll ich für USB einstellen?"},
			
		{ 250,301,  0, ANTW, "Wechselspannung"},	// --> Fail
		{ 250,251,  0, ANTW, "Gleichspannung"},
//Spannungs typ							185
		{ 251,252,  1, TEXT, "Ok, hab ich eingestellt."},
		{ 252,253,  1, TEXT, "Hier ist noch ein Regler für V."},
		{ 253,254,  1, TEXT, "Der steht bei ca 9."},
		{ 254,260,  1, TEXT, "Soll ich den so lassen?"},
			
		{ 260,261,  0, ANTW, "Dreh den auf 5."},
		{ 260,312,  0, ANTW, "Dreh den auf 12."},		// --> Fail
//Spannung									191
		{ 261,263,  1, TEXT, "So wenig?"},
		//{ 262,263,  1, TEXT, "Mehr ist immer gut."},
		{ 263,264,  1, TEXT, "Gut, du scheinst dich ja auszukennen"},
		{ 264,265,  1, TEXT, "Sollte so passen."},
		{ 265,266,  1, TEXT, "Jetzt noch das USB Kabel zerschneiden..."},
		{ 266,270,  5, INFO, "Tom ist beschäftigt."},
//Kabel											196
		{ 270,271,  1, TEXT, "Ohh. Hier sind vier Kabel."},
		{ 271,272,  1, TEXT, "Ein paar ist Weiß und Grün."},
		{ 272,273,  1, TEXT, "Das andere Rot und Schwarz."},
		{ 273,280,  1, TEXT, "Welches muss ich denn Anschließen?"},
			
		{ 280,284,  0, ANTW, "Rot und Schwarz."},
		{ 280,281,  0, ANTW, "Weiß und Grün."},	 
//Kabelfarbe								201			
		{ 281,282,  1, TEXT, "Hey, warte."},
		{ 282,283,  1, TEXT, "Die Farben an den Klemmen sind Rot und Schwarz."},
		{ 283,285,  1, TEXT, "Ich schließ das andere an."},
			
		{ 284,285,  1, TEXT, "Hey, die Farben an den Klemmen hab die gleiche Farbe."},
		{ 285,286,  1, TEXT, "Also Rotes Kabel an Rot."},
		{ 286,287,  1, TEXT, "Und Schwarz an Schwarze Klemme."},
		{ 287,290,  1, TEXT, "Kabel ist dran."},	
			
		{ 290,294,  0, ANTW, "Einschalten."},
		{ 290,291,  0, ANTW, "Viel Glück."},	 //	--> Fail
			
		{ 291,292,  1, TEXT, "Was?"},
		{ 292,293,  1, TEXT, "Mach mir jetzt keine Angst."},
		{ 293,294,  1, TEXT, "Ich dachte du weißt was du...."},
		{ 294,295,  1, TEXT, "Okay."},
		{ 295,296,  1, TEXT, "Wünsch mir Glü..."},
			
		{ 296,297,  1, INFO, "[Signal schwach]"},
		{ 297,000,  1, INFO, "[Signal verloren]"},  //--> linken akku überlebt	
//Akku Bad way							217
			
		{ 301,302,  1, TEXT, "Ok, hab ich eingestellt."},
		{ 302,303,  1, TEXT, "Hier ist noch ein Regler für V."},
		{ 303,304,  1, TEXT, "Der steht bei ca 9."},
		{ 304,310,  1, TEXT, "Soll ich den so lassen?"},
			
		{ 310,311,  0, ANTW, "Dreh den auf 5."},
		{ 310,312,  0, ANTW, "Dreh den auf 12."},		// --> Fail
//Spannung Bad way					223
		{ 311,313,  1, TEXT, "So wenig?"},
		{ 312,313,  1, TEXT, "Mehr ist immer gut."},
		{ 313,314,  1, TEXT, "Gut, du scheinst dich ja auszukennen"},
		{ 314,315,  1, TEXT, "Sollte so passen."},
		{ 315,316,  1, TEXT, "Jetzt noch das USB Kabel zerschneiden..."},
		{ 316,320,  5, INFO, "Tom ist beschäftigt."},
//Kabel bad way							229
		{ 320,321,  1, TEXT, "Ohh. Hier sind vier Kabel."},
		{ 321,322,  1, TEXT, "Ein paar ist Weiß und Grün."},
		{ 322,323,  1, TEXT, "Das andere Rot und Schwarz."},
		{ 323,330,  1, TEXT, "Welches muss ich denn Anschließen?"},
			
		{ 330,334,  0, ANTW, "Rot und Schwarz."},
		{ 330,331,  0, ANTW, "Weiß und Grün."},	 
//Kabelfarbe								235
		{ 331,332,  1, TEXT, "Hey, warte."},
		{ 332,333,  1, TEXT, "Die Farben an den Klemmen sind Rot und Schwarz."},
		{ 333,335,  1, TEXT, "Ich schließ das andere an."},
			
		{ 334,335,  1, TEXT, "Hey, die Farben an den Klemmen hab die gleiche Farbe."},
		{ 335,336,  1, TEXT, "Also Rotes Kabel an Rot."},
		{ 336,337,  1, TEXT, "Und Schwarz an Schwarze Klemme."},
		{ 337,340,  1, TEXT, "Kabel ist dran."},	
			
		{ 340,344,  0, ANTW, "Einschalten."},
		{ 340,341,  0, ANTW, "Viel Glück."},	 //	--> Fail
			
		{ 341,342,  1, TEXT, "Was?"},
		{ 342,343,  1, TEXT, "Mach mir jetzt keine Angst."},
		{ 343,344,  1, TEXT, "Ich dachte du weißt was du...."},
		{ 344,345,  1, TEXT, "Okay."},
		{ 345,346,  1, TEXT, "Wünsch mir Glü..."},
			
		{ 346,347,  1, INFO, "[Signal schwach]"},
		{ 347,1001, 1, INFO, "[Signal verloren]"},  //--> linken die Akku			
			
//Akku überlebt							251
		{ 350,351,  1, INFO, "[Signal gefunden]"},
		{ 351,352,  1, INFO, "[Verbinde]"},
		{ 352,353,  1, INFO, "[Empfange Daten]"},
		{ 353,354,  1, TEXT, "Das war aber knapp."},
		{ 354,355,  1, TEXT, "Der Sender ist kurz ausgegangen."},
		{ 355,356,  1, TEXT, "Ich hatte echt schon Panik."},
		{ 356,357,  1, TEXT, "Denn lass ich jetzt mal Aufladen."},
		{ 357,358,  1, TEXT, "Ich meld mich Wenn der Sender voll ist."},
		{ 358,500,60*4, INFO, "Tom ist beschäftigt."},
			
			
//end of Text								260
			
		{ 500,501,  1, TEXT, "[Text alle]"},
		{ 501,1100, 1, INFO, "ENDE"},
																			//262
			
		//{  74, 76,  1, TEXT, "Hab ich schon erwähnt: Es ist toten still hier.", 0},
//Die Akku
		{1001,1002,  0, DIE_AKKU, ""},
		{1002,1100,  0, INFO, "[Tom verloren]"},
		
		
		{1100,   0,  1, SPECIAL, "Tom gerettet"}
		
		//WICHTIG - step_count immer aktuell halten !!! IMMER
		
//ID, nextID, Delay, Type, Text, Special ID 
	},
	.credits = "Version 0.1\n\nDavid Lenk 2016\n\n200 Nachrichten\n\nHast du Ideen oder Fehler gefunden:\nmail.mail@mail.de\n\nWenn dir das Spiel gefällt. Geb mir ein Kaffe aus: PayPal..."
};
// ---------------------------------- GAME DATA -------- Ende---------------------------------------------------------

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

//Save
typedef struct {
  //current game 
	STEP *active_text[MAX_TEXT];
	STEP *active_antw[MAX_ANTW];
	STEP *next_step;
	int   active_text_count;
  int   active_antw_count;
	//int 	current_section;
	int	wakeup_reason;					//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer)
	MenuIndex	menu_index;			//save the current position in the timeline
	//settings	
	bool vibe;
	bool rapid_mode;
	bool sleep_time;
	
	//history
	STEP *history[MAX_TEXT];
	int 	history_count;
	/*
	ToDo:
		- history
			*days
		- game stats
			* Stats
			* achievements
	*/	
} __attribute__((__packed__)) persist;

//set defaults
persist settings = {
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
	.history = {NULL},
	.history_count = 0
};

static GPathInfo path_1 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,18+19},{135,18+24}, {8,18+24},{3,18+19}} };
static GPathInfo path_2 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,36+19},{135,36+24}, {8,36+24},{3,36+19}} };	
static GPathInfo path_3 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,54+19},{135,54+24}, {8,54+24},{3,54+19}} };	
static GPathInfo path_4 = { .num_points = 8,.points = (GPoint []) {	{3,10},{8,5}, {135,5},{140,10}, {140,72+19},{135,72+24}, {8,72+24},{3,72+19}} };	
static GPath *s_path_1;
static GPath *s_path_2;
static GPath *s_path_3;
static GPath *s_path_4;

//-------------------------------- save - load settings -------------------

static void save_persistent_settings() {
  //if(settings.active_antw_count > 0){
		//settings.menu_index = menu_layer_get_selected_index(s_menu_layer);	
	//}	
	//write in 
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void load_persistent_settings() {  
  if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == (int)sizeof(settings)){
		persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	
	}else{
		//save date the first time
		save_persistent_settings();
	}
}

// ------------------------------------- Game start ------------------------

static void setWakeup(enum TYPE reason, int min){
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
	
	wakeup_time = time(NULL) + (min * 60) + (addHour) + (60 * 60 * 24);			//24h later
	wakeup_schedule(wakeup_time, reason, false);
	
	//set wakeup reason
	settings.wakeup_reason = reason;	//save the wakeup reason (0= nichts, 1= incomming Msg, 2= waiting for answer

	//save settings
	save_persistent_settings();
}

static void setNextStep(int id){
	//int out = -1;	
	settings.next_step = NULL;
	for(int x = 0; x < g.step_count; x++){
		if(id == g.step[x].id){
			//out = g.step[x].id;
			settings.next_step = &g.step[x];		
			break;
		}
	}
}

static void addText(STEP *step){
	if(settings.active_text_count+1 < MAX_TEXT){
		settings.active_text[settings.active_text_count] = step;
		settings.active_text_count++;
		//MSG counter erhöhen für game stats
		if(settings.active_text_count < 3){
			menu_layer_set_selected_index(s_menu_layer, MenuIndex(0,settings.active_text_count), MenuRowAlignCenter, true);
		}
	}else{
		//Error MSG
		if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "ERROR: to many Text Messages in the Menu");}
		//erste Nachrichten löschen (Like a fifo) + Menu_select--
	}
	
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

// ------------------------------------ Game Main Function ----------------

//game action & timer callback
static void game_action(void *data){
	STEP *step = settings.next_step;
	int timer 	= 0;
	bool go_on 	= true;
	bool update_menu = false;
	
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
			window_stack_push(s_menu_window, true);	
		break;
		case 1: //Antwort Items
			clear_menu();
			
			antw = settings.active_antw[cell_index->row];
		  addText(antw);
		 	setNextStep(antw->next);
			
			//zwischenstand speichern
			save_persistent_settings();
		
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
*/

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
	text_layer_set_text(s_info_text, "Empfange neue Nachricht.");	//Headline
	if(settings.next_step == NULL){
		text_layer_set_text(s_info_text, "Tom wartet auf eine Antwort.");	//Headline
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
	save_persistent_settings();
	
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
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "check Step id: %d -> %d", id, out);}
	
	return out;
}

static void init() {
	//persist_delete(SETTINGS_KEY);
	
	srand(time(NULL));
	//if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "rand %d", rand() %3 );}
	
	//load data
	load_persistent_settings();	
	
	//Fehlerabfang falls die Pointer nicht mehr stimmen
	if(false){
		if(settings.active_text_count > 0 && !is_step_real(settings.active_text[settings.active_text_count-1]->id)){
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "RESET: text_id is no real ID -> reset text_count");}
			settings.active_text_count = 0;
			//setNextStep(0);
		}
		if(settings.active_antw_count > 0 && !is_step_real(settings.active_antw[settings.active_antw_count-1]->id)){
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "RESET: antw_id is no real Id -> rest antw_count");}
			settings.active_antw_count = 0;
		}
		if(settings.active_text_count == 0 && settings.active_antw_count == 0){
			if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "RESET: setNextStep = 0");}
			setNextStep(0);
		}
		//TODO: clear history
	}
	
	
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "Wakeup reason in Persistent: %d", settings.wakeup_reason);}
	
  s_main_window = window_create();	//timeline
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load 	= main_window_load,
    .unload = main_window_unload,
  });
  //window_stack_push(s_main_window, true);
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: timeline");}
	
	s_welc_window = window_create();	//welcome
  window_set_window_handlers(s_welc_window, (WindowHandlers){
    .load 	= welc_window_load,
    .unload = welc_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: welcome");}
	
	s_info_window = window_create();	//info (incoming Msg / waiting for answer)
  window_set_window_handlers(s_info_window, (WindowHandlers){
    .load 	= info_window_load,
    .unload = info_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: infocard");}
	
	s_menu_window = window_create();	//settings_menu
  window_set_window_handlers(s_menu_window, (WindowHandlers){
    .load 	= set_window_load,
    .unload = set_window_unload,
  });
	if(debug){APP_LOG(APP_LOG_LEVEL_DEBUG, "INIT: settings");}
	
	s_cred_window = window_create();	//settings_menu
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
		
		if(settings.next_step != NULL){
			
			if(settings.next_step->id == 0){ 		//first run
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
	save_persistent_settings();
	
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
