// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRIPLL           // Oscillator Select (Primary Oscillator with PLL module (HSPLL, ECPLL))
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
#include <stdio.h>
#include <stdlib.h>

#define FOSC 40000000UL  // 40 MHz
#define FCY (FOSC / 2)   // 20 MHz

// Funkcja opóźnienia w milisekundach
void delay_ms(unsigned int ms) {
    __delay32(ms * (FCY / 1000));
}

// Funkcja sprawdzająca stan przycisku
void checkButtons(unsigned int *licznik, char *prev6) {
    char current6 = PORTDbits.RD6;

    // Jeśli przycisk został wciśnięty
    if (current6 == 1 && *prev6 == 0) {
        (*licznik)++;
    }

    *prev6 = current6;
}

int main(void) {
    unsigned int licznik = 0, portValue = 0;
    char prev6 = 0;
    
    // Ustawienie wszystkich pinów portu A jako wyjścia
    TRISA = 0x0000;
    
    // Konfiguracja ADC 
    AD1CON1 = 0x80E4;
    AD1CON2 = 0x0404;
    AD1CON3 = 0x0F00;
    AD1CHS = 0x00;
    AD1CSSL = 0x20;

    while (1) {
        // Czekaj na zakończenie konwersji ADC
        while (!AD1CON1bits.DONE);
        
        // Sprawdzenie stanu przycisku
        checkButtons(&licznik, &prev6);
        
        // Odczyt wartości z ADC i przesunięcie bitowe
        portValue = ADC1BUF0;
        portValue = portValue >> 2;

        // Jeśli odczytana wartość jest większa lub równa 127
        if (portValue >= 127) { 
            licznik++;
            delay_ms(1000); // Opóźnienie 1000 ms

            // Jeśli licznik jest mniejszy niż 5
            if (licznik < 5) {
                LATA = 1;  // Ustawienie stanu wysokiego na porcie A
                delay_ms(1000); // Opóźnienie 1000 ms
                LATA = 0;  // Ustawienie stanu niskiego na porcie A
                delay_ms(1000); // Opóźnienie 1000 ms
            } else {
                LATA = 255;  // Ustawienie wszystkich bitów portu A na wysoki stan
            }

            // Czekaj, dopóki licznik jest większy lub równy 5 i przycisk jest wciśnięty
            while (licznik >= 5 && PORTDbits.RD6) {
                LATA = 255;  // Ustawienie wszystkich bitów portu A na wysoki stan
                checkButtons(&licznik, &prev6);  // Sprawdzenie stanu przycisku
            }
        } else {
            LATA = 0;  // Ustawienie stanu niskiego na porcie A
            licznik = 0;  // Reset licznika
        }

        delay_ms(100);  // Opóźnienie 100 ms
    }

    return 0;
}
