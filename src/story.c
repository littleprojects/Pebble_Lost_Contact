#include <pebble.h>
#include "story.h"


// ---------------------------------- GAME DATA ----------------------------------------------------------------------
GAME_FLOW g = {
	.name = "Lost Contact\nMoonbase",
	.desc =	"Rette Tom.\n\nTom ist in Schwierigkeiten und nur du kannst ihm helfen.\n\nDu bist die letzte Verbindung die er hat. Helf Tom bei wichtigen Entscheidungen und sicher so sein Überleben.\n",
	.step_count = 267,						//WICHTIG - step_count immer aktuell halten
	.step = {
//ID, nextID, Delay, Type, Text, Special ID 		if nextId = 0 -> auto countup -> nextID = ID+1
		{  1,   0,  1, INFO, "[Signal gefunden]"},
		{  2,   0,  1, INFO, "[Verbinde]"},
		{  3,   0,  1, INFO, "[Empfange Daten]"},
		{  4,   0,  1, TEXT, "Hallo..."},
		{  5,   0,  1, TEXT, "Geht das Ding? Ist da jemand..."},
		{  6,  10,  1, TEXT, "Hallo? Ich könnte wirklich etwas Hilfe gebrauchen."},
																						
		{  10, 11,  0, ANTW, "Hallo..."},
		{  10, 12,  0, ANTW, "Was ist den los?"},
																						// 8
		{  11, 13,  1, TEXT, "WOW, das alte Ding hier geht noch"},
		{  12,  0,  1, TEXT, "Ja, wenn ich das wüsste..."},
		{  13,  0,  1, TEXT, "Der Strom ist ausgefallen. Nur die Notbeleuchtung ist an."},
		{  14,  0,  1, TEXT, "Ziehmlich dunkel hier."},
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
		{  50, 51,  0, ANTW, "Warte. Mondgeschichte?"},	
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
		{  74, 75,  1, TEXT, "Warte, hier war doch irgendwo eine Taschenlampe."},
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
		
		{  97, 98,  0, MILESTONE, "Probleme"},													//first MILESTONE
		{  98, 99,  1, TEXT, "Da bin ich wieder."},
		{  99,100,  1, TEXT, "Und ich glaube wir haben ein Problem..."},
		
		{ 100,101,  0, ANTW, "Hast du was gefunden?"},
		{ 100,102,  0, ANTW, "Was ist los?"},	
//Akku leer																//75
		{ 101,102,  1, TEXT, "Gefunden hab ich leider nichts."},
		{ 102,103,  1, TEXT, "Ich glaub der Akku vom Notsender ist bald leer."},
		{ 103,104,  1, TEXT, "Er hat angefangen zu piepsen."},
		{ 103,104,  1, TEXT, "Und der Ladeanzeige ist gelb."},
		{ 104,105,  1, TEXT, "Das ist echt wieder typisch..."},
		{ 105,106,  1, TEXT, "Wenn man was braucht, ist der Akku leer."},
		{ 106,110,  1, TEXT, "Das ist nicht mein Tag heute."},
		
		{ 110,111,  0, ANTW, "Der hälten noch ewig."},
		{ 110,121,  0, ANTW, "Such ein Ladegerät."},
//Akku ganz leer												//84
		{ 111,112,  1, TEXT, "Echt? Das ist ja super"},
		{ 112,113,  1, TEXT, "Dann schau ich mich noch etwas um"},
		{ 113,114, 30, INFO, "Tom ist beschäftigt."},
			
		{ 114,115,  1, TEXT, "Hallo."},
		{ 115,116,  1, TEXT, "Das piepsen ist noch nerviger geworden."},
		{ 116,117,  1, TEXT, "Und die Anzeige steht jetzt auf rot."},
		{ 117,118,  1, TEXT, "Ich sollt......"},
		{ 118,119,  1, INFO, "[Signal schwach]"},
		{ 119,1001, 1, INFO, "[Signal verloren]"},
//Akku ladegerät												//93
		{ 121,122,  1, TEXT, "Bin schon unterwegs."},
		{ 122,123,  1, TEXT, "Geb mir ein paar Minuten."},
		{ 123,125, 30, INFO, "Tom ist beschäftigt."},
			
		{ 125,126,  1, TEXT, "Hey, ich hab ein USB Ladekabel gefunden."},
		{ 126,130,  1, TEXT, "Aber das bringt mir nichts ohne Strom."}, //95
		
		{ 130,131,  0, ANTW, "Gibt es Baterien?"},
		{ 130,133,  0, ANTW, "Habt ihr Solarzellen?"},		
//Batterien das Solar zellen					//100
		{ 131,132,  1, TEXT, "Ja, drüben beim Solarfeld."},
		{ 132,135,  1, TEXT, "Sie Speichern den Strom für die Nacht."},
			
		{ 133,134,  1, TEXT, "Ja, es gibt ein Solarfeld."},
		{ 134,135,  1, TEXT, "Die Idee gefällt mir."},	
			
		{ 135,136,  1, TEXT, "Aber dafür muss ich rüber in die andere Kuppel."},
		{ 136,140,  1, TEXT, "Ich beeile mich lieber..."},
//losgehen														106
		{ 140,141,  0, ANTW, "Ja, mach schnell."},
		{ 140,141,  0, ANTW, "Sei vorsichtig."},	
			
		{ 141,142,  1, TEXT, "Ist gut."},
		{ 142,143,  1, TEXT, "Bis gleich."},
		{ 143,144, 10, INFO, "Tom ist beschäftigt."},
			
		{ 144,150,  1, TEXT, "Ahhh... Hilfe."},
			
		{ 150,153,  0, ANTW, "Was ist los?"},
		{ 150,151,  0, ANTW, "Bist du Okay?"},	
//geräusch													113
		{ 151,152,  1, TEXT, "Ja, danke. "},
		{ 152,153,  1, TEXT, "Ich bin Okay. Glaub ich."},
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
//sieh nach							128
		{ 171,172,  1, TEXT, "Manooo, muss das sein?"},
		{ 172,173,  1, TEXT, "Und wenn da ein Weltraum Monster auf mich wartet..."},	
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
												//140
		{ 182,183,  1, TEXT, "Na du hast gut reden."},	
		{ 183,184,  1, TEXT, "Du sitzt auch nicht allein in einer dunklen Staion."},	
		{ 184,185,  1, TEXT, "Ich geh wieder zurück."},				
		{ 185,200, 30, INFO, "Tom ist beschäftigt."},
			
//schnell weg	- 190				//145
		{ 190,191,  1, TEXT, "Nicht lieber als das."},	
		{ 191,192,  1, TEXT, "Was war das?"},	
		{ 192,193,  1, TEXT, "Ich weiß nicht was unheilicher ist..."},
		{ 193,194,  1, TEXT, "Das da was ist."},	
		{ 194,195,  1, TEXT, "Oder das es jetzt wieder weg ist."},	
		{ 195,196,  1, TEXT, "Bin gleich im Batterie Raum."},
		{ 196,200, 30, INFO, "Tom ist beschäftigt."},			
//batterien							//153
																				//second MILESTONE
		{ 200,201,  1, TEXT, "So, ich bin bei den Batterien."},	
		{ 201,202,  1, TEXT, "Sie scheinen geladen zu sein."},
		{ 202,210,  1, TEXT, "Die Kontroll LEDs der Batterien sind Grün."},
			
		{ 210,212,  0, ANTW, "Warum gibt es dann kein Licht?"},
		{ 210,211,  0, ANTW, "Was ist da noch?"},
			
		{ 211,214,  1, TEXT, "Battrien und ein paar Regale mit Geräten..."},
			
		{ 212,213,  1, TEXT, "Ich glaub die Staion wurde nur Anfangs nur durch Solarzellen versorgt."},
		{ 213,214,  1, TEXT, "Aber dann hat man einen Reaktor eingesetzt."},
		{ 214,215,  1, TEXT, "Hmm. Ob der Reaktor am Stromausfall schuld ist."},
		{ 215,216,  1, TEXT, "Ohh Nein: Der Sender piepst jetzt schneller."},	
		{ 216,220,  1, TEXT, "Die Akkuanzeige ist jetzt rot."},
//											//164
		{ 220,221,  0, ANTW, "Schließ das Ladekabel an."},
		{ 220,225,  0, ANTW, "Such ein Netzteil."},
			
		{ 221,222,  1, TEXT, "Das geht nicht."},
		{ 222,223,  1, TEXT, "Man braucht ein besonderen Stecker."},
		{ 223,224,  1, TEXT, "Ich such mal ein Gerät mit so einem Stecker."},
		{ 224,230,  5, INFO, "Tom ist beschäftig."},	
		
		{ 225,226,  1, TEXT, "Schon dabei..."},	
		{ 226,230,  5, INFO, "Tom ist beschäftigt."},
			
		{ 230,231,  1, TEXT, "Hey ich hab was:"},
		{ 231,232,  1, TEXT, "Sieht aus wie ein multi Ladegerät."},
		{ 232,233,  1, TEXT, "Man kann da ne menge dran einstellen."},
		{ 233,240,  1, TEXT, "Und Klemmen um ein Kabel anzuschließen sind auch dran."},
//Ladegerät gefunden			176
		{ 240,241,  0, ANTW, "Super!"},
		{ 240,241,  0, ANTW, "Perfekt."},	
			
		{ 241,242,  1, TEXT, "An dem Ding kann man eine Menge einstellen."},
		{ 242,243,  1, TEXT, "Der erste Schalter ist für:"},
		{ 243,244,  1, TEXT, "Gleichspannung oder Wechselspannung."},
		{ 244,245,  1, TEXT, "Was auch immer das ist."},
		{ 245,246,  1, TEXT, "Es klingt ungesund."},
		{ 246,250,  1, TEXT, "Was soll ich für USB einstellen?"},
			
		{ 250,301,  0, ANTW, "Wechselspannung"},	// --> Fail
		{ 250,251,  0, ANTW, "Gleichspannung"},
//Spannungs typ							186
		{ 251,252,  1, TEXT, "Ok, hab ich eingestellt."},
		{ 252,253,  1, TEXT, "Hier ist noch ein Regler für V."},
		{ 253,254,  1, TEXT, "Der steht bei ca 9."},
		{ 254,260,  1, TEXT, "Soll ich den so lassen?"},
			
		{ 260,261,  0, ANTW, "Dreh den auf 5."},
		{ 260,312,  0, ANTW, "Dreh den auf 12."},		// --> Fail
//Spannung									192
		{ 261,263,  1, TEXT, "So wenig?"},
		//{ 262,263,  1, TEXT, "Mehr ist immer gut."},
		{ 263,264,  1, TEXT, "Gut, du scheinst dich ja auszukennen"},
		{ 264,265,  1, TEXT, "So, sollte jetzt passen."},
		{ 265,266,  1, TEXT, "Jetzt muss ich noch das USB Kabel zerschneiden..."},
		{ 266,270,  5, INFO, "Tom ist beschäftigt."},
//Kabel											197
		{ 270,271,  1, TEXT, "Ohh. Hier sind vier Kabel."},
		{ 271,272,  1, TEXT, "Ein paar ist Weiß und Grün."},
		{ 272,273,  1, TEXT, "Das andere Rot und Schwarz."},
		{ 273,280,  1, TEXT, "Welches muss ich denn Anschließen?"},
			
		{ 280,284,  0, ANTW, "Rot und Schwarz."},
		{ 280,281,  0, ANTW, "Weiß und Grün."},	 
//Kabelfarbe								202		
		{ 281,282,  1, TEXT, "Hey, warte."},
		{ 282,283,  1, TEXT, "Die Farben an den Klemmen sind Rot und Schwarz."},
		{ 283,285,  1, TEXT, "Ich schließ das andere an."},
			
		{ 284,285,  1, TEXT, "Hey, die Farben an den Klemmen hab die gleiche Farbe."},
		{ 285,286,  1, TEXT, "Also Rotes Kabel an Rot."},
		{ 286,287,  1, TEXT, "Und Schwarz an Schwarze Klemme."},
		{ 287,288,  1, TEXT, "OK. Kabel ist dran."},
		{ 288,290,  1, TEXT, "Mehr ist nicht zu machen..."},
			
		{ 290,294,  0, ANTW, "Und Einschalten."},
		{ 290,291,  0, ANTW, "Viel Glück."},	 //	--> Fail
			
		{ 291,292,  1, TEXT, "Was?"},
		{ 292,293,  1, TEXT, "Mach mir jetzt keine Angst."},
		{ 293,294,  1, TEXT, "Ich dachte du weißt was du...."},
		{ 294,295,  1, TEXT, "Okay."},
		{ 295,296,  1, TEXT, "Wünsch mir Glü..."},
			
		{ 296,297,  2, INFO, "[Signal schwach]"},
		{ 297,350,  2, INFO, "[Signal verloren]"},
//Akku Bad way							219
			
		{ 301,302,  1, TEXT, "Ok, hab ich eingestellt."},
		{ 302,303,  1, TEXT, "Hier ist noch ein Regler für V."},
		{ 303,304,  1, TEXT, "Der steht bei ca 9."},
		{ 304,310,  1, TEXT, "Soll ich den so lassen?"},
			
		{ 310,311,  0, ANTW, "Dreh den auf 5."},
		{ 310,312,  0, ANTW, "Dreh den auf 12."},		// --> Fail
//Spannung Bad way					225
		{ 311,313,  1, TEXT, "So wenig?"},
		{ 312,313,  1, TEXT, "Mehr ist immer gut."},
		{ 313,314,  1, TEXT, "Gut, du scheinst dich ja auszukennen"},
		{ 314,315,  1, TEXT, "Sollte so passen."},
		{ 315,316,  1, TEXT, "Jetzt noch das USB Kabel zerschneiden..."},
		{ 316,320,  5, INFO, "Tom ist beschäftigt."},
//Kabel bad way							231
		{ 320,321,  1, TEXT, "Ohh. Hier sind vier Kabel."},
		{ 321,322,  1, TEXT, "Ein paar ist Weiß und Grün."},
		{ 322,323,  1, TEXT, "Das andere Rot und Schwarz."},
		{ 323,330,  1, TEXT, "Welches muss ich denn Anschließen?"},
			
		{ 330,334,  0, ANTW, "Rot und Schwarz."},
		{ 330,331,  0, ANTW, "Weiß und Grün."},	 
//Kabelfarbe								237
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
			
		{ 346,347,  2, INFO, "[Signal schwach]"},
		{ 347,1001, 2, INFO, "[Signal verloren]"},  //--> linken die Akku			
			
//Akku überlebt							253
		{ 350,351,  2, INFO, "[Signal gefunden]"},
		{ 351,352,  2, INFO, "[Verbinde]"},
		{ 352,353,  1, INFO, "[Empfange Daten]"},
		{ 353,354,  1, TEXT, "Das war knapp!"},
		{ 354,355,  1, TEXT, "Der Sender ist kurz ausgegangen."},
		{ 355,356,  1, TEXT, "Ich hatte schon echte Panik."},
		{ 356,357,  1, TEXT, "Denn lass ich jetzt mal Aufladen."},
		{ 357,358,  1, TEXT, "Ich meld mich Wenn der Sender voll ist."},
		{ 358,500,60*4, INFO, "Tom ist beschäftigt."},		//4h
//Fenster										262
			/*
		{ 360,361,  1, TEXT, "Hallo."},
		{ 361,370,  1, TEXT, "Bist du da?"},		//18 Msg in a row
			
		{ 370,371,  0, ANTW, "Ja."},
		{ 370,372,  0, ANTW, "Was ist los?"},
		
		{ 371,372,  1, TEXT, "Gut."},
		{ 372,373,  1, TEXT, "Ich schau "},
			
		*/
		
			//kann aus dem Fenster schauen
			//und die Erde sehen
			//sie ist wunderschön.
			//hab bei Sicherheitsunterweisung geschlafen (Geheimniss)
			
			//Moonbase A  weil Moonbase1 von der ESA ist und die NASA nicht zweiter sein wollte ;) 
			
//end of Text								262
			
		{ 500,501,  1, TEXT, "[Text alle]"},
		{ 501,1100, 1, INFO, "ENDE"},
																			//264
			
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
