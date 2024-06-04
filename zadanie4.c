/*
 * File:   newmainXC16.c
 * Author: local
 *
 * Created on 29 maja 2024, 15:16
 */

// CONFIGURATION BITS
#pragma config POSCMOD = NONE             // Primary Oscillator Select (primary oscillator disabled)
#pragma config OSCIOFNC = OFF             // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD             // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC                // Oscillator Select (Fast RC Oscillator without Postscaler)
#pragma config IESO = OFF                 // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)
#pragma config WDTPS = PS32768            // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128              // WDT Prescaler (1:128)
#pragma config WINDIS = ON                // Watchdog Timer Window Mode disabled
#pragma config FWDTEN = OFF               // Watchdog Timer disabled
#pragma config ICS = PGx2                 // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF                 // Writes to program memory allowed
#pragma config GCP = OFF                  // Code protection is disabled
#pragma config JTAGEN = OFF               // JTAG port is disabled

#include "xc.h"
#include <libpic30.h>
#include <stdbool.h>

// Defninicja makr tak by kod byl czytelny, przejrzysty, deskryptywny i przyjazny
// uzytkownikowi
#define FCY         4000000UL   // czestotliwosc robocza oscylatora jako polowa 
                                //czestotliwosci (FNOSC = FRC -> FCY = 4000000)
// Zdefiniowanie poszczegolnych pinow jako odpowiednie makra
#define LCD_E       LATDbits.LATD4  
#define LCD_RW      LATDbits.LATD5
#define LCD_RS      LATBbits.LATB15
#define LCD_DATA    LATE

// Przypisanie wartosci poszcegolnych komend do wlasciwych makr
#define LCD_CLEAR       0x01    //0b00000001
#define LCD_HOME        0x02    //0b00000010
#define LCD_ON          0x0C    //0b00001100
#define LCD_OFF         0x08    //0b00001000
#define LCD_CONFIG      0x38    //0b00111000
#define LCD_CURSOR      0x80
#define LINE1           0x00
#define LINE2           0x40
#define LCD_CUST_CHAR   0x40
#define LCD_SHIFT_R     0x1D
#define LCD_SHIFT_L     0x1B

// Definicja funkcji delay w us i ms - operujacych na jednostkach czasu zamiast
// cykli pracy oscylatora

void __delay_us(unsigned long us){
    __delay32(us*FCY/1000000);
}

void __delay_ms(unsigned long ms){
    __delay32(ms*FCY/1000);
}

void LCD_sendCommand(unsigned char command){
    LCD_RW = 0;     // Zapis
    LCD_RS = 0;     // Przesylanie komend
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = command;
    __delay_us(50); // Opoznienie konieczne dla zapisania danych.
    LCD_E = 0;      // Konieczne zablokowanie transmisji po przeslaniu komunikatu.
}

void LCD_sendData(unsigned char data){
    LCD_RW = 0;
    LCD_RS = 1;     // Przesylanie danych
    LCD_E = 1;
    LCD_DATA = data;
    __delay_us(50);
    LCD_E = 0;
}

// Funkcja print wyswietlajaca kolejne 8-bitowe znaki w petli while - * oznacza
// przypisanie nie wartosci zmiennej lecz jej adresu.

void LCD_print(unsigned char* string){
    while(*string){
        LCD_sendData(*string++);
    }
}

void LCD_setCursor(unsigned char row, unsigned char col){
    unsigned char address;
    if(row == 1){
        address = LCD_CURSOR + LINE1 + col;
    }
    if (row == 2){
        address = LCD_CURSOR + LINE2 + col;
    }
    LCD_sendCommand(address);
}

// Funkcja inicjalizujaca wyswietlacz LCD. Wysyla niezbedne komendy jak LCD_CONFIG
// i LCD_ON

void LCD_init(){
    __delay_ms(20);
    LCD_sendCommand(LCD_CONFIG);
    __delay_us(50);     // opoznienia wynikaja ze specyfikacji wyswietlacza i czasu
                        // przetwarzania poszczegolnych komend
    LCD_sendCommand(LCD_ON);
    __delay_us(50);
    LCD_sendCommand(LCD_CLEAR);
    __delay_ms(2);
}

unsigned int display (void){
    AD1CON1bits.SAMP = 1;
    while(!AD1CON1bits.DONE);
    return ADC1BUF0;
}

void time_display(unsigned int time){
    unsigned char minutes = time / 60;
    unsigned char seconds = time % 60;

    LCD_setCursor(1, 0);
    LCD_print("Set time: ");
    LCD_sendData('0' + minutes);
    LCD_sendData(':');
    LCD_sendData('0' + (seconds / 10)); 
    LCD_sendData('0' + (seconds % 10)); 
}

void power_display(unsigned int power){
    unsigned char hundreds = power / 100;
    unsigned char tens = (power / 10) % 10;
    unsigned char units = power % 10;

    LCD_setCursor(2, 0);
    LCD_print("Set power: ");
    LCD_sendData('0' + hundreds); 
    LCD_sendData('0' + tens);
    LCD_sendData('0' + units);
}

bool readButton(volatile unsigned char *port, unsigned char pin) {
    if ((*port & (1 << pin)) == 0) {
        __delay_ms(50);
        if ((*port & (1 << pin)) == 0) {
            return true;
        }
    }
    return false;
}

int main(void) {
    bool currentS6 = 0, prevS6 = 0;
    bool currentS7 = 0, prevS7 = 0;
    bool currentS8 = 0, prevS8 = 0;
    unsigned int power = 0;
    unsigned int time = 0;
    bool start_button = false;

    AD1CON1 = 0x80E4; 
    AD1CON2 = 0x0404;
    AD1CON3 = 0x0F00;
    AD1CHS = 0;
    AD1CSSL = 0x0020;
    TRISB = 0x7FFF;   
    TRISD = 0xFFE7;
    TRISE = 0x0000;

    LCD_init(); 

    while(1) {
        power = display();

        if (power > 999) { 
            power = 999;
        }

        prevS6 = currentS6;
        prevS7 = currentS7;
        prevS8 = currentS8;

        currentS6 = readButton(&PORTD, 6);
        currentS7 = readButton(&PORTD, 7);
        currentS8 = readButton(&PORTD, 8);

        if (currentS6 && !prevS6) {
            time += 15;
        }

        if (currentS7 && !prevS7) {
            start_button = !start_button; 
        }

        if (currentS8 && !prevS8) {
            time = 0;
            start_button = false;
        }

        if (start_button && time > 0) {
            __delay_ms(1000);
            time--;
        }

        if (time == 0) {
            start_button = false;
        }

        power_display(power);
        time_display(time);
    }
}
