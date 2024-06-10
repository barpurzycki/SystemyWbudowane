/*
 * File:   newmainXC16.c
 *
 * Created on April 16, 2024, 4:50 PM
 */

// CONFIG2
#pragma config POSCMOD = NONE             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC           // Oscillator Select (Primary Oscillator with PLL module (HSPLL, ECPLL))
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

#include "xc.h"
#include <string.h>
#include <libpic30.h>

#define FCY       4000000UL
#define LCD_E     LATDbits.LATD4
#define LCD_RW    LATDbits.LATD5
#define LCD_RS    LATBbits.LATB15
#define LCD_DATA  LATE

#define BTN_A (PORTDbits.RD6 == 0)
#define BTN_B (PORTDbits.RD7 == 0)
#define BTN_C (PORTAbits.RA7 == 0)
#define BTN_D (PORTDbits.RD13 == 0)

#define LCD_CLEAR               0b1
#define LCD_CR                  0b10
#define _LCD_INPUTMODE(ID, S)   (0b100 + (ID<<1) + S)
#define _LCD_SCREENMODE(D,C,B)  (0b1000 + (D<<2) + (C<<1) + B)
#define _LCD_CONFIG(DL,N,F)     (0b100000 + (DL<<4) + (N<<3) + (F<<2))
#define LCD_ON                  _LCD_SCREENMODE(1,0,0)
#define LCD_OFF                 _LCD_SCREENMODE(0,0,0)
#define LCD_CONFIG              _LCD_CONFIG(1,1,0)
#define LCD_CUST_CHAR           0x40
#define LCD_SHIFT_R             0x1D
#define LCD_CURSORPOS(P)        (0b10000000 + (P&0b1111111))
//#define LCD_ON                  0x0C        //D = 1, C = 0
//#define LCD_OFF                 0x08        //D = 0, C = 0
//#define LCD_CONFIG              0x38        //DL = 1, N = 1, F = 0

long synctime = 0;
void sync_to(long t){
    if (t > synctime){
        __delay32(t-synctime);
    }
    synctime = 0;
}
void sleep_sync(long a){
    __delay32(a);
    synctime += a;
}
void sleep_sync_us(long us){
    sleep_sync(FCY/1000000*us);
}
void sleep_sync_ms(long us){
    sleep_sync(FCY/1000*us);
}

void toString(char* target, int digits, unsigned n){
    for (int x = 0; x < digits; x++){
        target[digits-1-x] = '0' + (n%10);
        n /= 10;
    }
}

void LCD_sendCommand(uint8_t command){
    LCD_RW = 0;
    LCD_RS = 0;
    LCD_E = 1;
    LCD_DATA = command;
    sleep_sync_us(50);
    LCD_E = 0;
    sleep_sync_us(50);
}

void LCD_sendData(unsigned char data){
    LCD_RW = 0;
    LCD_RS = 1;
    LCD_E = 1;
    LCD_DATA = data;
    sleep_sync_us(50);
    LCD_E = 0;
    sleep_sync_us(50);
}

void LCD_print(char* text, int line){
    int c = 0;
    if (line != 2){
        LCD_sendCommand(LCD_CURSORPOS(line == 0 ? 0x00 : 0x40));
    }
    while (*text && c++ < 0x10){
        LCD_sendData(*(text++));
    }
}

void LCD_init(){
    sleep_sync_ms(20);
    LCD_sendCommand(LCD_CONFIG);
    LCD_sendCommand(LCD_ON);
    LCD_sendCommand(LCD_CLEAR);
    sleep_sync_ms(2);
}


struct PlayerTime {
    unsigned min;
    unsigned sec;
};

struct PlayerTime p1Time;
struct PlayerTime p2Time;
unsigned currentState = 0;
unsigned lastPlayerWin = 0;

void state_setTimes(){
    char line1[0x10] = {' '};
    char line2[0x10] = {' '};
    memset(line1, ' ', 0x10);
    memset(line2, ' ', 0x10);
    line2[0] = '+';
    line2[3] = '-';
    line2[6] = '<';
    line2[7] = '>';
    line2[9] = 'S';
    p1Time = (struct PlayerTime){20,0};
    p2Time = (struct PlayerTime){20,0};
    unsigned sel = 0;
    while (1){
        if (BTN_A){
            unsigned* v = sel == 0 ? &p1Time.min : &p2Time.min;
            if (*v < 99){
                (*v)++;
            }
            sleep_sync_ms(250);
        }
        if (BTN_B){
            unsigned* v = sel == 0 ? &p1Time.min : &p2Time.min;
            if (*v > 1){
                (*v)--;
            }
            sleep_sync_ms(250);
        }
        if (BTN_C){
            sel = 1-sel;
            sleep_sync_ms(250);
        }
        if (BTN_D){
            currentState++;
            sleep_sync_ms(250);
            break;
        }
        
        line1[0] = sel == 0 ? '>' : ' ';
        toString(line1+1, 2, p1Time.min);
        line1[3] = ':';
        toString(line1+4, 2, p1Time.sec);
        
        line1[9] = sel == 1 ? '>' : ' ';
        toString(line1+10, 2, p2Time.min);
        line1[12] = ':';
        toString(line1+13, 2, p2Time.sec);
        
        LCD_print(line1, 0);
        LCD_print(line2, 1);
        
    }
}
void state_ingame(){
    char line1[0x10] = {' '};
    char line2[0x10] = {' '};
    memset(line1, ' ', 0x10);
    memset(line2, ' ', 0x10);
    while (1){
        unsigned player = (ADC1BUF0 > 0b1000000000) ? 1 : 0;
        struct PlayerTime* alt = (player ? &p2Time : &p1Time);
        if (alt->sec-- == 0){
            if (alt->min-- == 0){
                lastPlayerWin = 1-player;
                currentState++;
                break;
            }
            alt->sec = 59;
            
        }
        if (BTN_A){
            currentState = 0;
            break;
            sleep_sync_ms(250);
        }
        
        line1[0] = !player ? '|' : ' ';
        toString(line1+1, 2, p1Time.min);
        line1[3] = ':';
        toString(line1+4, 2, p1Time.sec);
        
        line1[9] = player ? '|' : ' ';
        toString(line1+10, 2, p2Time.min);
        line1[12] = ':';
        toString(line1+13, 2, p2Time.sec);
        
        LCD_print(line1, 0);
        LCD_print(line2, 1);
        
        sync_to(FCY);
    }
}
void state_end(){
    char c[2] = {0};
    c[0] = lastPlayerWin ? '2' : '1';
    while (1){
        LCD_print("WYGRYWA GRACZ ", 0);
        LCD_print(c, 2);
        LCD_print("                ", 1);
        if (BTN_A){
            currentState = 0;
            break;
        }
    }
}

int main(void) {
    
    TRISA = 0b10000000;
    TRISB = 0x7FFF; //potencjometr, czujnik temperatury
    TRISD = 0b10000011000000;
    TRISE = 0;
    
    AD1CON1 = 0x80E4;   //konfiguracja
    AD1CON2 = 0x0404;   //kana?y, wielo czy jednokana?owo
    AD1CON3 = 0x0F00;
    AD1CHS = 0;         //wyb�r fizycznego urz?dzenia
    AD1CSSL = 0x0020;
    
    LCD_init();
    
    p1Time = (struct PlayerTime){20,0};
    p2Time = (struct PlayerTime){20,0};
    
    void (*states[])() = {
        state_setTimes,
        state_ingame,
        state_end
    };
    
    while(1){
        states[currentState]();
    }
    
    return 0;
}
