#include <rdm6300.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Api.h>
#include "ESP8266TimerInterrupt.h"
#include <DB.h>
#include <TimeProcess.h>

#define RDM6300_RX_PIN 		 4 
#define READ_LED_PIN 		16 

#define BOTAO_1 			14  //D5
#define BOTAO_2 			12  //D6
#define BOTAO_3 			13  //D7
#define BOTAO_4 			15  //D8

Rdm6300 		rdm6300;
ESP8266Timer 	ITimer;
WiFiManager 	wm;
Api 			api("http://192.168.1.219/coffee/public/api/");
DB 				integrated("/integrated.txt");
DB 				pendent("/pendents.txt");

int 	optionMoment 	= 0;
int 	idxProducts  	= 0;
bool 	stPendents 		= false;

TimeProcess timeUpdateTime( 600 );

typedef struct {
	int  	id;
	float   price;
	float   priceMember;
	int 	button;
} sProduct;
sProduct tableProducts[7];

typedef struct {
	String 	userIDE;
	int 	id;
	String 	name;
	int 	member;
	double 	balance;
} sUser;
sUser user;

volatile int actionButton 	= 0;
unsigned int intPinB1 		= BOTAO_1;
unsigned int intPinB2 		= BOTAO_2;
unsigned int intPinB3 		= BOTAO_3;
unsigned int intPinB4 		= BOTAO_4;

void consume( int option );
void configWiFi();
void configSelectProducts();
void startTableProducts();
void startPeripherals();
void setUser( int id, String name, int member, double balance );
void apiSucess( String dataObj );
void apiError( String dataObj );
void sendData( String dataJson );

void setup() {

	Serial.begin(9600);

	configWiFi();

	startPeripherals();

	startTableProducts();

	configSelectProducts();

}

void loop() {
	// aguardando identificação no cartão
	if ( ( optionMoment == 0 ) && (rdm6300.get_new_tag_id() ) ) {

		actionButton = 0;

		user.userIDE = String( rdm6300.get_tag_id(), HEX );
		api.getSaldo( user.userIDE, setUser );
		Serial.println( user.name + ", seu saldo é: " + user.balance );
		Serial.println("Escolha uma opção ( 1 a 4) " );

		optionMoment = 1;
		
	}

}

void setUser(int id, String name, int member, double balance ) {
	user.id = id;
	user.name = name;
	user.member = member;
	user.balance = balance;
}

void processPendents() {
  stPendents = false;
  pendent.listAll( sendData );
}

void apiSucess( String dataObj ) {
  integrated.salve( dataObj );
  if ( stPendents ) {
    processPendents();
  }
}

void apiError( String dataObj ) {
  pendent.salve( dataObj );
  stPendents = true;
}

void sendData( String dataJson ) {
  api.postConsumer( dataJson, apiSucess, apiError );
}

void consume( int option ) {

	float value = ( user.member == 1 ) ? tableProducts[option].priceMember : tableProducts[option].price;

	if ( user.balance >= value ) {

		Serial.printf( "Opção escolhida [%i] \n", tableProducts[option].button );
		String param = String(user.id) + "/" + String(tableProducts[option].id);
		
		api.postConsumer( param, apiSucess, apiError );
		
		optionMoment = 0;
		user.balance = 0;
		user.id 	 = 0;
		user.member  = 0;
		user.name    = "";
		user.userIDE = "";

		Serial.println("Aproxime o seu Cartão do Leitor");
	}
	else 
		Serial.println("Sem saldo!");

	actionButton = 0;
	
}

void configWiFi() {
	wm.setConfigPortalTimeout(60);  
  	bool res = wm.autoConnect("USA-cafee", "usa1984");
  	if ( res ) 
  		Serial.println("Conectado");
  	else {
    	Serial.println("falhao ao conectar!");
    	ESP.restart();
  	}
}

void startPeripherals() {

	// botões para selecionar produtos
	pinMode(BOTAO_1, INPUT_PULLUP);
	pinMode(BOTAO_2, INPUT_PULLUP);
	pinMode(BOTAO_3, INPUT_PULLUP);
	pinMode(BOTAO_4, INPUT_PULLUP);

	// leds
	pinMode(READ_LED_PIN, OUTPUT);
	digitalWrite(READ_LED_PIN, LOW);

	// leitor de cartão
	rdm6300.begin(RDM6300_RX_PIN);
}

void addItemTablePrices( int id, float price, float priceMember, int button ) {
	tableProducts[idxProducts].id 			= id;
	tableProducts[idxProducts].price 		= price;
	tableProducts[idxProducts].priceMember 	= priceMember;
	tableProducts[idxProducts].button 		= button;
	idxProducts += 1;

	Serial.printf("id = %i price = %f member = %f buttom = %i \n", id, price, priceMember, button );
}

void startTableProducts() {
	Serial.println("chando api produtos ");
	api.getProduts( addItemTablePrices );
	Serial.println("Aproxime o seu Cartão do Leitor");
}

void IRAM_ATTR detectB1() { 
	actionButton = ( user.balance == 0.00  ) ? 0 : 1 ;
}
void IRAM_ATTR detectB2() { 
	actionButton = ( user.balance == 0.00  ) ? 0 : 2 ;
}
void IRAM_ATTR detectB3() { 
	actionButton = ( user.balance == 0.00  ) ? 0 : 3 ;
}
void IRAM_ATTR detectB4() { 
	actionButton = ( user.balance == 0.00  ) ? 0 : 4 ;
}

void IRAM_ATTR TimerConsumer() {
	if ( actionButton > 0 ) {
		consume( actionButton - 1 );
	}
}

void configSelectProducts() {
	if ( ITimer.attachInterruptInterval( 2000, TimerConsumer ) ) {
    	Serial.print(F("Starting  ITimer OK, millis() = "));
    	Serial.println(millis());
	}
	attachInterrupt(digitalPinToInterrupt(intPinB1), detectB1, RISING);
	attachInterrupt(digitalPinToInterrupt(intPinB2), detectB2, RISING);
	attachInterrupt(digitalPinToInterrupt(intPinB3), detectB3, RISING);
	attachInterrupt(digitalPinToInterrupt(intPinB4), detectB4, RISING);

	actionButton = 0;
	user.balance = 0;
}