/************************************************************************************/
/*                                 PROYECTO                                         */
/************************************************************************************/
/*
  1- Mira la luminosidad del ambiente.
  2- Compara la luminosidad con cierto umbral.
  3- Si todavia hay luz, se duerme durante 10 minutos, tras ello se despierta y pasa a 1.
  4- Si hay suficiente oscuridad, detecta presencia.
  5- Si hay presencia, enciende la luz y se duerme durante 1 minuto, tras ello se despierta y pasa a 1.
  6- Si no hay presencia, se apaga la luz y se duerme durante 1 segundo, tras ello se despierta y pasa a 1.
  
  Habra por tanto dos modos de funcionamiento según la luminosidad:
  De dia: Duerme y se despierta cada 10 minutos para ver si se ha hecho de noche.
  De noche: Duerme y se despierta cada 1 segundo para ver si hay presencia. Si hay presencia enciende la luz y se espera (durmiendo) 1 minuto. Si no hay presencia apaga la luz.
  
  Nota: Al programar la posicion del interruptor hay que tenerlo en cuenta para que no este en un estado de pull (activado) ya que es un pin de programacion.
  
  Attiny84 1MHz
*/

#include <avr/delay.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include "relay.h"

/************************************************************************************/
/************************************************************************************/

// CONSTANTES
#define P_NC0 0                 // Pin 2
#define P_NC1 1                 // Pin 3
#define P_MODO 2                // Pin 5
#define P_RELE 3                // Pin 6
#define P_INTERRUPTOR 4         // Pin 7
#define P_LED_NORM 5            // Pin 8
#define P_LED_PRES 7            // Pin 9
#define P_LED_AUTO 6            // Pin 10
#define P_LDR A2                // Pin 11 (8)
#define P_PIR 9                 // Pin 12
#define P_BUZZER A0             // Pin 13 (10)

#define NORMAL 0
#define AUTOMATICO 1
#define PRESENCIA 2
#define UMBRAL_LUMINOSIDAD 800
#define MODO_INICIAL PRESENCIA


// VARIABLES
volatile int modo;

RELAY rele(P_RELE, LOW); // OJO CON LA LOGICA
int val_pir, val_ldr;

volatile int wd_counter = 0;
volatile int encendido, t0;

/************************************************************************************/
/************************************************************************************/
// RUTINAS DE TRATAMIENTO DE INTERRUPCION
ISR(WDT_vect)
{
    wd_counter++;
}

void Int0ISR()
{
    wdt_reset();
    _delay_ms(250);
    wdt_reset();
    
    switch(modo)
    {
        case NORMAL:
            modo = AUTOMATICO;
            break;
        case AUTOMATICO:
            modo = PRESENCIA;
            break;
        case PRESENCIA:
            modo = NORMAL;
            break;
        default:
            modo = NORMAL;
            break;
    }
    
    encendido = 0;
    wd_counter = 75;
    t0 = micros();
    
    detachInterrupt(INT0);
}

/************************************************************************************/
/************************************************************************************/
// PROGRAMA DE CONFIGURACION
void setup()
{
    configurar_pines();
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    modo = MODO_INICIAL;
    encendido = 0;
    t0 = 0;
    
    inicializar_sistema();
}

/************************************************************************************/
/************************************************************************************/
// BUCLE PRINCIPAL
void loop()
{   
    act_modo_leds();
    
    attachInterrupt(0, Int0ISR, LOW);
    
    switch(modo)
    {
        case NORMAL:
            modo_normal();
            break;
        case AUTOMATICO:
            modo_auto();
            break;
        case PRESENCIA:
            modo_presencia();
            break;
        default:
            break;
    }
}

/************************************************************************************/
/************************************************************************************/
// FUNCIONES
void configurar_pines()
{
    pinMode(P_PIR,INPUT);
    pinMode(P_MODO,INPUT_PULLUP);
    pinMode(P_INTERRUPTOR,INPUT_PULLUP);
    pinMode(P_NC0,INPUT_PULLUP);
    pinMode(P_NC1,INPUT_PULLUP);
    
    pinMode(P_LED_NORM,OUTPUT);
    pinMode(P_LED_PRES,OUTPUT);
    pinMode(P_LED_AUTO,OUTPUT);
    digitalWrite(P_LED_NORM,HIGH);
    digitalWrite(P_LED_AUTO,HIGH);
    digitalWrite(P_LED_PRES,HIGH);
}

void inicializar_sistema()
{
    digitalWrite(P_LED_NORM,LOW);
    //beep(250);
    wd_dormir_250ms();
    digitalWrite(P_LED_AUTO,LOW);
    //beep(250);
    wd_dormir_250ms();
    digitalWrite(P_LED_PRES,LOW);
    //beep(250);
    wd_dormir_250ms();
    
    rele.configure();
    
    digitalWrite(P_LED_NORM,HIGH);
    digitalWrite(P_LED_AUTO,HIGH);
    digitalWrite(P_LED_PRES,HIGH);
    //beep(250);
    //wd_dormir_250ms();
}

void act_modo_leds()
{
    switch(modo)
    {
        case NORMAL:
            if(encendido == 0)
            {
                digitalWrite(P_LED_NORM,LOW);
                digitalWrite(P_LED_AUTO,HIGH);
                digitalWrite(P_LED_PRES,HIGH);
                //beep(250);
                wd_dormir_250ms();
                encendido = 1;
            }
            break;
        case AUTOMATICO:
            if(encendido == 0)
            {
                digitalWrite(P_LED_NORM,HIGH);
                digitalWrite(P_LED_AUTO,LOW);
                digitalWrite(P_LED_PRES,HIGH);
                //beep(250);
                wd_dormir_250ms();
                encendido = 1;
            }
            break;
        case PRESENCIA:
            if(encendido == 0)
            {
                digitalWrite(P_LED_NORM,HIGH);
                digitalWrite(P_LED_AUTO,HIGH);
                digitalWrite(P_LED_PRES,LOW);
                //beep(250);
                wd_dormir_250ms();
                encendido = 1;
            }
            break;
        default:
            break;
    }
}

void modo_normal()
{
    while(modo == NORMAL)
    {
        if(digitalRead(P_INTERRUPTOR) == LOW)
            rele.on();
        else
            rele.off();
        
        wd_dormir_250ms();
    }
    rele.off();
}

void modo_auto()
{
    while(modo == AUTOMATICO)
    {
        lectura_sensor_luz();
        
        if(noche())
            rele.on();
        else
            rele.off();
            
        wd_dormir_10m();
    }
    rele.off();
}

void modo_presencia()
{
    while(modo == PRESENCIA)
    {
        lectura_sensores();
        
        if(noche())
        {
            if(val_pir == HIGH)
            {
                rele.on();
                //wd_delay(45000);
                wd_dormir_30s();
                //wd_dormir_30s();
                
                lectura_sensor_pir();
                if(val_pir == LOW)
                    rele.off();
            }
            else
            {
                rele.off();
            }
            
            wd_dormir_1s();
            wd_dormir_1s();
        }
        else
        {
            wd_dormir_10m();
        }
    }
    rele.off();
}

int noche()
{
    return (val_ldr > UMBRAL_LUMINOSIDAD);
}

void lectura_sensores()
{
    lectura_sensor_pir();
    lectura_sensor_luz();
}

void lectura_sensor_luz()
{
    int i;
    
    val_ldr = 0;
    for(i = 0; i < 4; i++)
    {
        val_ldr = val_ldr + analogRead(P_LDR);
    }
    val_ldr = val_ldr/4;
}

void lectura_sensor_pir()
{
    val_pir = digitalRead(P_PIR);
}

void beep(unsigned long delayms)
{
    analogWrite(P_BUZZER, 10);
    delay(delayms);
    analogWrite(P_BUZZER, 0);
    delay(delayms);  
}

void wd_dormir_250ms()
{
    setup_watchdog(WDTO_250MS); // 250ms
    
    sleep_mode(); // Sustituye a: sleep_enable(); sleep_cpu(); sleep_disable();
    
    wd_counter = 0;
    
    wdt_disable();
}

void wd_dormir_1s()
{
    setup_watchdog(WDTO_1S); // 1s
    
    sleep_mode(); // Sustituye a: sleep_enable(); sleep_cpu(); sleep_disable();
    
    wd_counter = 0;
    
    wdt_disable();
}

void wd_dormir_30s()
{
    while(wd_counter < 15)
    {
        setup_watchdog(WDTO_2S); // 4s
        sleep_mode(); // Sustituye a: sleep_enable(); sleep_cpu(); sleep_disable();
        wdt_disable();
    }
    wd_counter = 0;
}


void wd_dormir_1m()
{
    while(wd_counter < 15)
    {
        setup_watchdog(WDTO_4S); // 4s
        sleep_mode(); // Sustituye a: sleep_enable(); sleep_cpu(); sleep_disable();
        wdt_disable();
    }
    wd_counter = 0;
}

void wd_dormir_10m()
{
    while(wd_counter < 75) // 600s (10min) / 8s
    {
        setup_watchdog(WDTO_8S); // 8s
        sleep_mode(); // Sustituye a: sleep_enable(); sleep_cpu(); sleep_disable();
        wdt_disable();
    }
    wd_counter = 0;
}

void wd_delay(unsigned long t_ms) // OJO: LA SENSIBILIDAD SERA DE 15MS (UN CAMBIO MENOR A 15MS NO SE DETECTA Y, EL VALOR MINIMO SERA DE 15MS)
{
    int num_veces = 0;
    
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    
    if(t_ms <= 15)
    {
        setup_watchdog(WDTO_15MS);
        sleep_mode();
    }
    else if(t_ms == 30)
    {
        setup_watchdog(WDTO_30MS);
        sleep_mode();
    }
    else if(t_ms == 60)
    {
        setup_watchdog(WDTO_60MS);
        sleep_mode();
    }
    else if(t_ms == 120)
    {
        setup_watchdog(WDTO_120MS);
        sleep_mode();
    }
    else if(t_ms == 250)
    {
        setup_watchdog(WDTO_250MS);
        sleep_mode();
    }
    else if(t_ms == 500)
    {
        setup_watchdog(WDTO_500MS);
        sleep_mode();
    }
    else if(t_ms == 1000)
    {
        setup_watchdog(WDTO_1S);
        sleep_mode();
    }
    else if(t_ms == 2000)
    {
        setup_watchdog(WDTO_2S);
        sleep_mode();
    }
    else if(t_ms == 4000)
    {
        setup_watchdog(WDTO_4S);
        sleep_mode();
    }
    else if(t_ms == 8000)
    {
        setup_watchdog(WDTO_8S);
        sleep_mode();
    }
    else
    {
        num_veces = t_ms/15;
        while(wd_counter < num_veces)
        {
            setup_watchdog(WDTO_15MS);
            sleep_mode();
        }
    }
    
    wdt_disable();
    wd_counter = 0; 
}

void setup_watchdog(int timerPrescaler)
{
    if ((timerPrescaler < 10) && (timerPrescaler > 0))
    {    
        byte bb = timerPrescaler & 7;    
        if(timerPrescaler > 7)
            bb |= (1<<5); // Establece los bits necesarios
        
        //This order of commands is important and cannot be combined
        MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
        WDTCSR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
        WDTCSR = bb; //Set new watchdog timeout value
        WDTCSR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int
    }
}
