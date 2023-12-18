#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

class Api {

  private : 
    const char* baseURL;
    WiFiClient client;
    HTTPClient http;

    using FuncCallBackUser      = void(*)(int id, String name, int member, double balance );
    using FuncCallBackProducts  = void(*)( int id, float price, float priceMember, int button );
    using FuncCallBackResp      = void(*)( String ret );
    
    void deserialize(String json, FuncCallBackUser fnc ) {

      StaticJsonDocument<400> doc;
      deserializeJson(doc, json);
      auto id       = doc["matricula"].as<int>();
      auto name     = doc["nome"].as<const char*>();
      auto member   = doc["associado"].as<int>();
      auto balance  = doc["saldo"].as<float>();
      
      (* fnc)( id, name, member, balance );

    }

    void deserializeProducts( String json, FuncCallBackProducts fnc ) {

      StaticJsonDocument<700> doc;
      DeserializationError err = deserializeJson(doc, json);

      if (  err ) {
        Serial.println("erro lista de produtos");
        Serial.println(err.f_str());
      }
      else {
        JsonArray arr = doc.as<JsonArray>();
        for (JsonObject repo : arr ) {

          (* fnc)(  repo["id"].as<int>(), 
                    repo["valor"].as<float>(),
                    repo["valor_associado"].as<float>(),
                    repo["botao"].as<int>() );
        }
      }
    }


  public:

    Api( const char* serverName ) {
      baseURL = serverName;
    }

    void postConsumer( const String registro, FuncCallBackResp sucess, FuncCallBackResp error  ) {

      String urlConsumer = baseURL;
      
      urlConsumer.concat("movements/add" );
      
      http.begin(client, urlConsumer);

      http.addHeader("content-type", "application/json");

      String payload = "{}"; 

      int httpResponseCode = http.POST( registro );

      if ( httpResponseCode > 0 ) {
        payload = http.getString();
        Serial.println(payload);
      } 

      http.end();

      if ( ( httpResponseCode >= 200 ) && ( httpResponseCode <= 299 ) ) {
        (* sucess)( registro );
      }
      else {
        (* error)( registro );
      }

    }

    void getSaldo( String credential, FuncCallBackUser fnc ) {

        String urlSaldo = baseURL;
        urlSaldo.concat("getSaldo/" + credential );
        http.begin(client, urlSaldo);
        int httpResponseCode = http.GET();

        if ( httpResponseCode > 0 ) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);

            String payload = "{}"; 
            payload = http.getString();
            deserialize(payload, fnc);
        }
        else {
            Serial.print("\n Error code: ");
            Serial.println(httpResponseCode);
        }
  
        http.end();
    }

    void getProduts(FuncCallBackProducts fnc) {
  
        String urlProduts = baseURL;
        urlProduts.concat("getProducts");
        http.begin(client, urlProduts);

        int httpResponseCode = http.GET();

        if ( httpResponseCode > 0 ) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = "{}"; 
            payload = http.getString();
            deserializeProducts( payload, fnc );
        }
        else {
            Serial.print("\n Error code: ");
            Serial.println(httpResponseCode);
        }
  
        http.end();

    }

};