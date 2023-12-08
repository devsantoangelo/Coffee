
#include <rdm6300.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Api.h>

#define RDM6300_RX_PIN 		 4 
#define READ_LED_PIN 		16 

#define BOTAO_1 			14  //D5
#define BOTAO_2 			12  //D6
#define BOTAO_3 			13  //D7
#define BOTAO_4 			15  //D8

bool ST_BOTAO_1 = 1;
bool ST_BOTAO_2 = 1;
bool ST_BOTAO_3 = 1;
bool ST_BOTAO_4 = 1;

Rdm6300 rdm6300;

int optionMoment = 0;

uint32_t userID;
String userIDEx;

WiFiManager wm;
Api api("http://192.168.1.219/coffee/public/api/");

void consume( int option );

void setup() {

  wm.setConfigPortalTimeout(60);  
  bool res = wm.autoConnect("USA-cafee", "usa1984");
  if ( res ) 
  	Serial.println("Conectado");
  else {
    Serial.println("falhao ao conectar!");
    ESP.restart();
  }
  

	//INICIALIZAR CONF BOTAO
	pinMode(BOTAO_1, INPUT_PULLUP);
	pinMode(BOTAO_2, INPUT_PULLUP);
	pinMode(BOTAO_3, INPUT_PULLUP);
	pinMode(BOTAO_4, INPUT_PULLUP);

	Serial.begin(9600);

	pinMode(READ_LED_PIN, OUTPUT);
	digitalWrite(READ_LED_PIN, LOW);


    delay(3000);
	Serial.println("");
	Serial.println("tentando...");
	rdm6300.begin(RDM6300_RX_PIN);
	
	Serial.println("Place RFID tag near the rdm6300");


}

void loop() {
	// aguardando identificação no cartão
	if ( optionMoment == 0 ) {
		if (rdm6300.get_new_tag_id() ) {

			userID 		= rdm6300.get_tag_id();
			userIDEx 	= String( userID, HEX );

api.get(userIDEx);

			Serial.println(userIDEx);		
			Serial.println("Escolha uma opção");



			optionMoment = 1;
		}
	}

	if ( optionMoment == 1 ) {

		if ( digitalRead(BOTAO_1) ) {
			consume(1);
		}
		if ( digitalRead(BOTAO_2) ) {
			consume(2);
		}
		if ( digitalRead(BOTAO_3) ) {
			consume(3);
		}
		if ( digitalRead(BOTAO_4) ) {
			consume(4);
		}
		delay(500);

	}
}

void consume( int option ) {
	
	Serial.println(" ");

	Serial.printf("\n[%i] escolheu a opção [%i]", userID, option);
	optionMoment 	= 0;
	userID 			= 0;

}


