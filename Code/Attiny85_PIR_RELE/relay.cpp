#include "relay.h"

RELAY::RELAY(int pin_rele, int state_rele)
{
    pin = pin_rele;
    stat = state_rele;
}
  
void RELAY::configure()
{
    pinMode(pin, OUTPUT);    
    digitalWrite(pin, stat);
    
    //test(500);
}

void RELAY::on()
{
    if(stat == 0)
    {
        digitalWrite(pin, HIGH);
        stat = 1;
    }
}

void RELAY::off()
{
    if(stat == 1)
    {
        digitalWrite(pin, LOW);
        stat = 0;
    }
}

void RELAY::test(unsigned long time)
{
    int i;
    
    if((time > 0) && (time <= MAX_TIME))
    {
        if(stat == 1)
        {
            for(i = 0; i < 2; i++)
            {
                off();
                delay(time);
                on();
                
                delay(time*2);
                
                off();
                delay(time);
                on();
            }
        }
        else
        {
            for(i = 0; i < 2; i++)
            {
                on();
                delay(time);
                off();
                
                delay(time*2);
                
                on();
                delay(time);
                off();
            }
        }
    }
}

void RELAY::turn()
{
    stat = !stat;
    digitalWrite(pin, stat);
}

int RELAY::state()
{
    return stat;
}
