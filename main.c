/*
 * main.c
 *
 *  Created on: 16.02.2014
 *      Author: Frederick
 *
 *
 *  Überwacht den Akku-Zusand, schaltet Lampen vor Tiefenentladung ab.
 *
 *  Warnung bei x%
 *  	--> 1 LED-Streifen blinkt
 *
 *  Warnung bei x%
 *  	--> 2 LED-Streifen blinken
 *
 *  Abschalten bei x%
 *
 */

#include "config.h"

volatile unsigned char flag_1s;
volatile unsigned char flag_1min;
volatile unsigned char flag_1hour;
volatile unsigned char counter_menu;
volatile unsigned char counter_1min;

volatile unsigned char sec;
volatile unsigned char min;
volatile unsigned char hour = 17;	// 17
volatile int32_t	rtc_cal = (F_CAL-12800000)*256/100;	// Kalibrierung des 32K Quarzes

unsigned int adc_bat = 10000; // Batteriespannung mit 3 Nachkommastellen (mV)

#define UART_BAUD_RATE      9600


void init(){

	// Ports initialisieren
	setBits(DDR(LED_GREEN_1_PORT), LED_GREEN_1);
	setBits(DDR(LED_GREEN_2_PORT), LED_GREEN_2);
	setBits(DDR(LED_YELLOW_PORT), LED_YELLOW);
	setBits(DDR(LED_RED_PORT), LED_RED);

	led_green_1_on();
	led_green_2_on();
	led_yellow_on();
	led_red_on();

	DDRB |= (1<<PB0);
	PORTB |= (1<<PB0);


	// ADC initialisieren
	ADMUX = _BV(REFS0);					// AVcc = Ref MUX = ADC0
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1); 	// Prescaler = 64


	// Timer2 konfigurieren
	ASSR   = (1<< AS2);           	 // Timer2 asynchron takten
	_delay_ms(1000);             	 // Einschwingzeit des 32kHz Quarzes

	TCCR2B = 1;                   	 // Vorteiler 1 -> 7,8ms Überlaufperiode
	OCR2A  = 128;                  	 // PWM, Tastverhältnis 50%
	while((ASSR & (1<< TCR2BUB)));  // Warte auf das Ende des Zugriffs
	TIFR2  &= ~(1<<TOV2);           // Interrupts löschen
	TIMSK2 |= (1<<TOIE2);           // Timer overflow Interrupt freischalten


//	/////////////////// Energiesparen
//	// Analogcomparator ausschalten
//	ACSR = (1<<ACD);
//
//	// Clocks Abschalten von: TWI, Timer0, Timer1, USART1, USART0, ADC, SPI
//	PRR |= (1<<PRTWI) | (1<<PRTIM0) | (1<<PRTIM1) | (1<<4) | (1<<PRUSART0) | (1<<PRADC) | (1<<PRSPI);
}


// Timer2 overflow Interrupt
ISR(TIMER2_OVF_vect) {

//***ab hier RTC   ******************************************
	static uint8_t ticks;               // Hilfsvariable für Messintervall
    static int16_t time_error;          // RTC Fehlerkompensation

    // Zeitkritische Dinge, welche am Anfang der ISR stehen müssen!
    OCR2A=128;                          // Dummy-Write zur Sicherung des Timings
                                        // von Timer 2 im asynchronen Modus

    // RTC Fehler korrigieren

    if (time_error>999) {               // RTC zu schnell
        TCNT2 = 2;                      // Zähler einen Schritt zurück setzen (2 Takte Verzögerung!)
        time_error -= 1000;
    } else if (time_error<-999) {       // RTC zu langsam
        TCNT2 = 4;                      // Zähler einen Schritt vor setzen (2 Takte Verzögerung!)
        time_error += 1000;
    }

    // ab hier ist es nicht mehr zeitkritisch

    // Echtzeituhr
    ticks++;                            // 1/128tel Sekunde

//***bis hier RTC   ******************************************

	// 1/128 = 7,8ms Sekunden Intervall

	// Sekunden Intervall
	if (ticks==128) {                   // Sekundenintervall
        time_error += rtc_cal;          // RTC Fehler akkumulieren
		ticks=0;
        flag_1s =1;                     // setzte Flag für 1s Verarbeitung in main Endlosschleife

		if (counter_menu) counter_menu--; // Damit sich Menu nach bestimter zeit beendet

		if (sec < 59) sec++;
		else
		{
			sec = 0;					// neue Minute
			flag_1min = 1;				// Setze Flag für 1min Verarbeitung in main Endlosschleife

			if(min < 59) min++;			// neue Stunde
			else{
				flag_1hour = 1;
				min = 0;
			}
		}
    }
}

void get_adc(void)
{
	/************************************************************************/
	/*				 Batteriespannung messen								*/
	/************************************************************************/

//	uint32_t temp;
	unsigned int adc_value;

	PRR &= ~(1<<PRADC);				// ADC Clock einschalten
	ADCSRA |= (1<<ADEN);			// ADC einschalten
	ADMUX = 0;						// ADC2
	ADMUX |= (1<<REFS0); 			// AVcc als Referenz
	ADCSRA |= (1<<ADSC); 			// Umwandlung starten
	while ( ADCSRA & (1<<ADSC) ); 	// auf Abschluss der Konvertierung warten
	adc_value = ADCW;				// dummy auslesen
	ADCSRA |= (1<<ADSC); 			// Nochmal messen
	while ( ADCSRA & (1<<ADSC) );
	adc_value = ADCW;				// Ergebnis übernehmen
	ADCSRA &= ~(1<<ADEN);			// ADC wieder aus
	PRR |= (1<<PRADC);				// ADC Clock ausschalten

	// Spannung errechnen: (in mV)
	adc_bat = ((uint32_t)adc_value * 15928) >> 10;	// Rechnung in einem Schritt
}

int main(){

	char buffer[10];

	init();
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	sei();


	while(1){

		if(flag_1s){
			flag_1s = 0;

			get_adc();
			itoa(adc_bat,buffer,10);
			uart_puts(buffer);
			uart_putc('\r');

			if(adc_bat > _75PERCENT){
				led_green_1_on();
				led_green_2_on();
				led_yellow_on();
				led_red_on();
			}
			else if(adc_bat > _50PERCENT){
				led_green_2_on();
				led_yellow_on();
				led_red_on();
			}
			else if(adc_bat > _25PERCENT){
				led_yellow_on();
				led_red_on();
			}
			else if(adc_bat > _0PERCENT){
				led_red_on();
			}else{
				for(int i=0; i<5; i++){
					led_red_on();
					_delay_ms(500);
					led_red_off();
					_delay_ms(500);
				}
			}


			_delay_ms(500);


			led_green_1_off();
			led_green_2_off();
			led_yellow_off();
			led_red_off();
		}


		// Sleep
		SMCR |= (1<<SM1) | (1<<SM0);	// Sleep-Mode --> Power-Save
		cli();
		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
		sei();
	}


	return 0;
}
