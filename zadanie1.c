/*
 * File:   newmainXC16.c
 * Author: local
 *
 * Created on 8 maja 2024, 15:51
 */

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
#include "libpic30.h"

// Zadanie 1.

int GrayConvert(unsigned char i) {
    return (i >> 1) ^ i;
}

int main(void) {
    int licznik = 6;
    unsigned char portValue = 0;
    char currentS6 = 0, currentS7 = 1, prevS6 = 0, prevS7 = 1;
    TRISA = 0x0000;
    TRISD = 0xFFFF;
    
    // przypisanie przycisków
    while(1){
        prevS6 = PORTDbits.RD6;
        prevS7 = PORTDbits.RD7;
        currentS6 = PORTDbits.RD6;
        currentS7 = PORTDbits.RD7;
        
        //zmiana podprogramu o 1 do przodu
        if(currentS6 - prevS6 == 1)
        {
            if(currentS6 == 1)
            {
                licznik++;
            }
        }
    
        // zmiana podprogramu o 1 do tylu
        if(currentS7 - prevS7 == 1)
        {
            if(currentS7 == 1)
            {
                licznik--;
            }
        }
        
        // podprogram nr 1 - licznik zliczajacy w gore 0-255
        if(licznik == 0)
        {
            portValue = 0;
            while(1) 
            {
                LATA = portValue;   
                __delay32(5000000);
                portValue++;
            }
        }

        // podprogram nr 2 - licznik zliczajacy w dol 255-0
        if(licznik == 1)
        {
            portValue = 255;
            while(1) 
            {
                LATA = portValue;   
                __delay32(5000000);
                portValue--;
            }
        }

        // podprogram nr 3 - licznik w kodzie Graya zliczajacy w gore 0-255
        if(licznik == 2)
        {
            portValue = 0;
            while(1)
            {
                LATA = GrayConvert(portValue);
                __delay32(5000000);
                portValue++;
            }
        }

        // podprogram nr 4 - licznik w kodzie Graya zliczajacy w dol 255-0
        if(licznik == 3)
        {
            portValue = 255;
            while(1)
            {
                LATA = GrayConvert(portValue);
                __delay32(5000000);
                portValue--;
            }
        }
        
        // podprogram nr 5 - 2x4 bitowy licznik w kodzie BCD zliczajacy w górę 0-99
        if(licznik == 4)
        {
            LATA = (((portValue/10) << 4) | (portValue % 10));
            __delay32(5000000);
            if(portValue <= 99)
            {
                portValue++;
            }
            else 
            {
                portValue = 0;
            }
        }
        
        // podprogram nr 6 - 2x4 bitowy licznik w kodzie BCD zliczajacy w dół 99-0
        if(licznik == 5)
        {
            LATA = (((portValue/10) << 4) | (portValue % 10));
            __delay32(5000000);
            if(portValue > 0)
            {
                portValue--;
            }
            else
            {
                portValue = 99;
            }  
        }
        
        // 3 bitowy wężyk poruszający się lewo-prawo
        if(licznik == 6)
        {
            unsigned char portValue[10] = {7, 14, 28, 56, 112, 224, 112, 56, 28, 14};
            unsigned int i = 0;
            while(1)
            {
                LATA = portValue[i];
                __delay32(2000000);
                i++;
                if(i==10)
                {
                    i = 0;
                }
            }
        }
    }
    


   return 0;
}
