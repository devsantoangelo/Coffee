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

float minPriceProduct 		= 1000.00;
float minPriceProductMember = 1000.00;
float maxPriceProduct 		= 0.00;

TimeProcess timeUpdateTime( 600 );

TimeProcess timeConsumer( 2 );

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

int idProdutConsumer = 0;

void consume( int option );
void configWiFi();
void configSelectProducts();
void startTableProducts();
void startPeripherals();
void setUser( int id, String name, int member, double balance );
void apiSucess( String dataObj );
void apiError( String dataObj );
void sendData( String dataJson );
void calculetePulse(float value );

void setup() {

	Serial.begin(9600);

	configWiFi();

	startPeripherals();

	startTableProducts();

	configSelectProducts();
/********************************************************/
	calculetePulse( 0.01 );
	calculetePulse( 0.49 );
	calculetePulse( 0.99 );
	calculetePulse( 1.99 );
	calculetePulse( 5.99 );
	calculetePulse( 15.99 );
	calculetePulse( 95.99 );
/********************************************************/

}

void loop() {
	// aguardando identificação no cartão
	if ( ( optionMoment == 0 ) && (rdm6300.get_new_tag_id() ) ) {

		actionButton = 0;

		user.userIDE = String( rdm6300.get_tag_id(), HEX );

		api.getSaldo( String(user.userIDE), setUser );

		float value = ( user.member == 1 ) ? minPriceProductMember : minPriceProduct;

		if ( user.balance > value ) {
			Serial.println( user.name + ", seu saldo é: " + user.balance );
			Serial.println("Escolha uma opção ( 1 a 4) " );
			optionMoment = 1;
			calculetePulse(user.balance);
		}
		else {
			Serial.println( String(user.balance) + " é insuficiente" );
			Serial.println("Aproxime o seu Cartão do Leitor");
		}
	}

	if ( timeConsumer.verify( ) && ( idProdutConsumer > 0 ) ) {
		idProdutConsumer = idProdutConsumer - 1;
		consume( idProdutConsumer );
		idProdutConsumer = 0;
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

		String param = "{\"matricula\" : " + String(user.id) 
						+ ", \"credencial\" : \"" + String(user.userIDE) 
						+ "\", \"produto\" : " + String(tableProducts[option].id) + "}" ;
		
		
		Serial.println(param);
		
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

	if ( minPriceProduct > price )
	  minPriceProduct = price;
	if ( minPriceProductMember > priceMember )
	  minPriceProductMember = priceMember;
    if ( maxPriceProduct < price )
	  maxPriceProduct = price;

}

void startTableProducts() {
	api.getProduts( addItemTablePrices );
}

void IRAM_ATTR detectB1() { 
	actionButton = 1;
}

void IRAM_ATTR detectB2() { 
	actionButton = 2;
}

void IRAM_ATTR detectB3() { 
	actionButton = 3;
}

void IRAM_ATTR detectB4() { 
	actionButton = 4;
}

void IRAM_ATTR TimerConsumer() {

	if ( actionButton > 0 ) {

		int idx = actionButton - 1;

		float value = ( user.member == 1 ) ? tableProducts[idx].priceMember : tableProducts[idx].price;
		
		idProdutConsumer = ( value < user.balance ) ? actionButton : 0;

		actionButton = 0;
		
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

void calculetePulse( float value ) {

	int real = (int) value;
	int cents = (int) ( ( value - real) * 100 );

	int tenReals	= 0;
	int fiveReals 	= 0;
	int oneReal 	= 0;

	int tenCents    = 0;

	if ( real >= 10 ) {
		tenReals = real / 10;
		real   	= real % 10 ;
	}

	if ( real >= 5 ) {
		fiveReals = real / 5;
		real   	 = real % 5 ;
	}

	if ( real > 0 ) {
		oneReal = real;
	}

	if ( cents >= 10 ) {
		tenCents = cents / 10;
		cents = cents % 10;
	}

	if ( cents >= 1 ) {
		tenCents += 1;		
		if ( tenCents == 10 ) {
			oneReal += 1;
			tenCents = 0;
		}
	}
int totalPulsos = tenReals + fiveReals + oneReal + tenCents;
	Serial.printf("[%i * r$10] [%i * r$ 5] + [%i * r$ 1] + [%i r$ 0,10] <=> valor [%.2f]  [%i pulsos]\n", 
							tenReals, fiveReals, oneReal, tenCents, value, totalPulsos ) ;

}