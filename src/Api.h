#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

class Api {

  private : 
    const char* baseURL;
    WiFiClient client;
    HTTPClient http;

  public:
    Api( const char* serverName ) {
      baseURL = serverName;
    }

    int post( const String registro ) {

      http.begin(client, baseURL);

      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST( registro );
      String payload = "{}"; 

      if ( httpResponseCode > 0 ) {
        payload = http.getString();
      }
      // Free resources
      http.end();

      return httpResponseCode;

    }

    void get( String credential ) {

        String urlSaldo = baseURL;
        urlSaldo.concat("getSaldo/" + credential );

        Serial.println( urlSaldo );

        http.begin(client, urlSaldo);

        int httpResponseCode = http.GET();

        String payload = "{}"; 

        if ( httpResponseCode > 0 ) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            payload = http.getString();
        }
        else {
            Serial.print("\n Error code: ");
            Serial.println(httpResponseCode);
        }
  
        http.end();

        Serial.println( payload );

    }

};