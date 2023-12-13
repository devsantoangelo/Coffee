#include <Arduino.h>

class TimeProcess {

  private : 

    long unsigned pauseTime, lastMoment;

    using FuncCallBack = void(*)();

  public:
    TimeProcess( long unsigned seconds ) {
      pauseTime = seconds * 1000;
    }

    bool verify() {

      bool stWait = ( millis() - lastMoment ) > pauseTime ; 

      if ( stWait ) { 
        lastMoment = millis();
      }
      return stWait;

    }

    void execute( FuncCallBack func_ptr ) {
        
        if ( verify() ) {
           (* func_ptr)();
        }

    };


};