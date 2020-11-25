#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ds3231.h"

//#define SETTIME
#define TESTTUBES

// RMINUTE
#define RMINUTE_RST_PIN     PC0
#define RMINUTE_RST_DDR     DDRC
#define RMINUTE_RST_PORT    PORTC

#define RMINUTE_CLK_PIN     PC1
#define RMINUTE_CLK_DDR     DDRC
#define RMINUTE_CLK_PORT    PORTC

// LMINUTE
#define LMINUTE_RST_PIN     PD7
#define LMINUTE_RST_DDR     DDRD
#define LMINUTE_RST_PORT    PORTD

#define LMINUTE_CLK_PIN     PB0
#define LMINUTE_CLK_DDR     DDRB
#define LMINUTE_CLK_PORT    PORTB

// RHOUR
#define RHOUR_RST_PIN     PD5
#define RHOUR_RST_DDR     DDRD
#define RHOUR_RST_PORT    PORTD

#define RHOUR_CLK_PIN     PD6
#define RHOUR_CLK_DDR     DDRD
#define RHOUR_CLK_PORT    PORTD

// LHOUR
#define LHOUR_RST_PIN     PD2
#define LHOUR_RST_DDR     DDRD
#define LHOUR_RST_PORT    PORTD

#define LHOUR_CLK_PIN     PD4
#define LHOUR_CLK_DDR     DDRD
#define LHOUR_CLK_PORT    PORTD

#define DOT_PIN             PB2
#define DOT_DDR            DDRB
#define DOT_PORT           PORTB

#define CLEANING_HOUR 4

uint8_t cycles[] = {1, 0, 9, 8, 7, 6, 5, 4, 3, 2};

enum DIGIT {LHOUR, RHOUR, LMINUTE, RMINUTE};

void resetDigit(enum DIGIT digit) {
    switch(digit) {
        case RMINUTE:
            RMINUTE_RST_PORT |= (1 << RMINUTE_RST_PIN);
            RMINUTE_RST_PORT &= ~(1 << RMINUTE_RST_PIN);
            break;
        case LMINUTE:
            LMINUTE_RST_PORT |= (1 << LMINUTE_RST_PIN);
            LMINUTE_RST_PORT &= ~(1 << LMINUTE_RST_PIN);
            break;
        case RHOUR:
            RHOUR_RST_PORT |= (1 << RHOUR_RST_PIN);
            RHOUR_RST_PORT &= ~(1 << RHOUR_RST_PIN);
            break;
        case LHOUR:
            LHOUR_RST_PORT |= (1 << LHOUR_RST_PIN);
            LHOUR_RST_PORT &= ~(1 << LHOUR_RST_PIN);
            break;
    }
}

void countDigit(enum DIGIT digit) {
    switch(digit) {
        case RMINUTE:
            RMINUTE_CLK_PORT ^= (1 << RMINUTE_CLK_PIN);
            RMINUTE_CLK_PORT ^= (1 << RMINUTE_CLK_PIN);
            break;
        case LMINUTE:
            LMINUTE_CLK_PORT ^= (1 << LMINUTE_CLK_PIN);
            LMINUTE_CLK_PORT ^= (1 << LMINUTE_CLK_PIN);
            break;
        case RHOUR:
            RHOUR_CLK_PORT ^= (1 << RHOUR_CLK_PIN);
            RHOUR_CLK_PORT ^= (1 << RHOUR_CLK_PIN);
            break;
        case LHOUR:
            LHOUR_CLK_PORT ^= (1 << LHOUR_CLK_PIN);
            LHOUR_CLK_PORT ^= (1 << LHOUR_CLK_PIN);
            break;
    }
}

void setDigit(enum DIGIT digit, uint8_t newVal) {
    resetDigit(digit);
    
    uint8_t c = cycles[newVal];
    
    while(c > 0) {
        countDigit(digit);
        c--;
    }
}

void clean(uint8_t minute) {
    if(minute < 50) {
        uint8_t digit = minute/5;
        setDigit(LHOUR, digit);
        setDigit(RHOUR, digit);
        setDigit(LMINUTE, digit);
        setDigit(RMINUTE, digit);
    }
    else {
        // turn off
    }
}

void initPins() {
   RMINUTE_RST_DDR |= (1 << RMINUTE_RST_PIN);
   RMINUTE_RST_PORT &= ~(1 << RMINUTE_RST_PIN);
   RMINUTE_CLK_DDR |= (1 << RMINUTE_CLK_PIN);
   
   LMINUTE_RST_DDR |= (1 << LMINUTE_RST_PIN);
   LMINUTE_RST_PORT &= ~(1 << LMINUTE_RST_PIN);
   LMINUTE_CLK_DDR |= (1 << LMINUTE_CLK_PIN);
   
   RHOUR_RST_DDR |= (1 << RHOUR_RST_PIN);
   RHOUR_RST_PORT &= ~(1 << RHOUR_RST_PIN);
   RHOUR_CLK_DDR |= (1 << RHOUR_CLK_PIN);
   
   LHOUR_RST_DDR |= (1 << LHOUR_RST_PIN);
   LHOUR_RST_PORT &= ~(1 << LHOUR_RST_PIN);
   LHOUR_CLK_DDR |= (1 << LHOUR_CLK_PIN);
   
   DOT_DDR      |= (1 << DOT_PIN);
}

int main (void) {
    initPins();
    initDS3231();
    _delay_ms(100);
    #ifdef SETTIME
        ds3231Use12HourMode(false);
        ds3231SetTime(22, 05, 10, false);
        ds3231SetFullDate(SUNDAY, 22, NOVEMBER, 20, 21);
    #endif
    
    #ifdef TESTTUBES
    _delay_ms(1000);
    int i;
    for(i = 0; i < 10; i++) {
     setDigit(LHOUR, i);
     setDigit(RHOUR, i);
     setDigit(LMINUTE, i);
     setDigit(RMINUTE, i);
     DOT_PORT ^= (1 << DOT_PIN);
     _delay_ms(1000);
    }
    #endif

   while(1) {
        DOT_PORT ^= (1 << DOT_PIN);
        uint8_t hour = ds3231GetHour();
        uint8_t minute = ds3231GetMinute();
        
        if(CLEANING_HOUR == hour) {
            clean(minute);
        } else {
            setDigit(LHOUR, hour/10);
            setDigit(RHOUR, hour%10);
            setDigit(LMINUTE, minute/10);
            setDigit(RMINUTE, minute%10);
        }
        _delay_ms(1000);
   }

   return 0;
}
