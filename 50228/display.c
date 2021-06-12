#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "iocompat.h"	

#include "pins.h"
#include "pd44.h"
#include "schiphol50228.h"

ISR (TIMER1_OVF_vect)	
{
}

void UART_init() {
	UCSR0A = 0;
	UCSR0B = (1<<4) + (1<<3); // RX/TX on.
#define UBR 51 /* 9600 */
	UBRRH=  (UBR >> 8) & 0x0F;
	UBRR0 = (UBR >> 0) & 0xFF;
}

void inline UART_send_char(unsigned char c) {
        /* wait for the register to clear */
	while ((UCSR0A & (1 << 5)) == 0) {};  
	UDR0 = c;
}

void UART_send(char * str) {
     for(char *p = str; *p; p++) UART_send_char(*p);
}

int UART_get(void) {
	if (UCSR0A & (1<<7))
		return UDR0;
	return -1;
};

int main (void)
{
    UART_init();
    UART_send("\n\n\n" __FILE__ " " __DATE__ " " __TIME__ "\n\n"); 

    TCCR1A = TIMER1_PWM_INIT;
    TCCR1B |= TIMER1_CLOCKSOURCE;
    OCR = 0;
    DDROC = _BV (OC1);
    TIMSK = _BV (TOIE1);
    sei ();

    init50228();
    pd44_init();

    for(int displ = 0; displ < 6; displ++) {
	char * str = "No  ";
	str[3] = displ + '0';
        select50228(displ);
        pd44_brigthness(3);
	setDisplay(displ, str);
    } 
    _delay_ms(2000);

    char str[] = "All your bases are belong to us         ";
    int at = 0;

    for (int i = 0; i < 50 ; i++) {
       for(int displ = 0; displ < 6; displ++) {
        select50228(displ);
	pd44_sendChar(3, str[ ( displ*4 + i + 0 ) % (sizeof(str)-1) ]);
	pd44_sendChar(2, str[ ( displ*4 + i + 1 ) % (sizeof(str)-1) ]);
	pd44_sendChar(1, str[ ( displ*4 + i + 2 ) % (sizeof(str)-1) ]);
	pd44_sendChar(0, str[ ( displ*4 + i + 3 ) % (sizeof(str)-1) ]);
        };
        _delay_ms(100); // sleep_mode();
      };
    
    UART_send("Ready for input\n"); 
    for(;;) { 
      int c = UART_get();
      if (c>0 && at < sizeof(str) -1) {
		if (c == 10) {
			at = 0;
		} else if (c >= ' ') {
			str[at++] = c;
			str[at] = 0;
		};
		setFullDisplay(str);
	};
    }
    return (0);
}