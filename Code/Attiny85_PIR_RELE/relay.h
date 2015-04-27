#ifndef RELAY_h
  #define RELAY_h
  
  #if (ARDUINO >= 100)
      #include <Arduino.h>
  #else
      #include <WProgram.h>
  #endif
  
  #define MAX_TIME 5000

  class RELAY
  {
      public:
          RELAY(int pin_rele, int state_rele);
          void configure();
          void turn();
          void on();
          void off();
          int state();
          void test(unsigned long time);
          
      private:
          int pin, stat;
  };
  
#endif
