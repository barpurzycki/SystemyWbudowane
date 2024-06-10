// CONFIG2
#pragma config POSCMOD = HS
#pragma config OSCIOFNC = OFF
#pragma config FCKSM = CSDCMD
#pragma config FNOSC = PRIPLL
#pragma config IESO = OFF

// CONFIG1
#pragma config WDTPS = PS32768
#pragma config FWPSA = PR128
#pragma config WINDIS = ON
#pragma config FWDTEN = OFF
#pragma config ICS = PGx2
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config JTAGEN = OFF

#include "xc.h"
#include "libpic30.h"

#define FOSC 40000000UL  // 40 MHz
#define FCY (FOSC / 2)   // 20 MHz

int GrayConvert(unsigned char i) {
    return (i >> 1) ^ i;
}

void delay_ms(unsigned int ms) {
    __delay32(ms * (FCY / 1000));
}

void checkButtons(int *licznik, char *prev6, char *prev7) {
    char current6 = PORTDbits.RD6;
    char current7 = PORTDbits.RD7;

    if (current6 == 1 && *prev6 == 0) {
        (*licznik)--;
    }

    if (current7 == 1 && *prev7 == 0) {
        (*licznik)++;
    }

    *prev6 = current6;
    *prev7 = current7;

    if (*licznik < 0) {
        *licznik = 8;
    } else if (*licznik > 8) {
        *licznik = 0;
    }
}

int main(void) {
    int licznik = 0;
    unsigned char portValue = 0;
    char prev6, prev7;
    TRISA = 0x0000;
    TRISD = 0xFFFF;

    prev6 = PORTDbits.RD6;
    prev7 = PORTDbits.RD7;

    while (1) {
        checkButtons(&licznik, &prev6, &prev7);

        switch (licznik) {
            case 0: // Counter counting up 0-255
                portValue = 0;
                while (licznik == 0) {
                    LATA = portValue;
                    delay_ms(200);
                    portValue++;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 1: // Counter counting down 255-0
                portValue = 255;
                while (licznik == 1) {
                    LATA = portValue;
                    delay_ms(200);
                    portValue--;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 2: // Gray code counter up 0-255
                portValue = 0;
                while (licznik == 2) {
                    LATA = GrayConvert(portValue);
                    delay_ms(200);
                    portValue++;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 3: // Gray code counter down 255-0
                portValue = 255;
                while (licznik == 3) {
                    LATA = GrayConvert(portValue);
                    delay_ms(200);
                    portValue--;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 4: // BCD counter up 0-99
                portValue = 0;
                while (licznik == 4) {
                    LATA = ((portValue / 10) << 4) | (portValue % 10);
                    delay_ms(200);
                    portValue++;
                    if (portValue > 99) portValue = 0;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 5: // BCD counter down 99-0
                portValue = 99;
                while (licznik == 5) {
                    LATA = ((portValue / 10) << 4) | (portValue % 10);
                    delay_ms(200);
                    portValue--;
                    if (portValue > 99) portValue = 99;
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 6: // 3-bit snake moving left-right
                {
                    unsigned char snake[10] = {7, 14, 28, 56, 112, 224, 112, 56, 28, 14};
                    unsigned int i = 0;
                    while (licznik == 6) {
                        LATA = snake[i];
                        delay_ms(200);
                        i++;
                        if (i >= 10) i = 0;
                        checkButtons(&licznik, &prev6, &prev7);
                    }
                }
                break;
            case 7: // queue
                portValue = 0;
                while (licznik == 7) {
                    for (int i = 0; i < 8; i++) {
                        int x = 1;
                        for (int j = i; j < 8; j++) {
                            LATA = portValue + x;
                            x = x << 1;
                            delay_ms(200);
                            checkButtons(&licznik, &prev6, &prev7);
                            if (licznik != 7) break;
                        }
                        portValue += x >> 1; 
                        checkButtons(&licznik, &prev6, &prev7);
                        if (licznik != 7) break;
                    }
                    portValue = 0; 
                    checkButtons(&licznik, &prev6, &prev7);
                }
                break;
            case 8: // generator
                {
                    unsigned char lfsr = 0b1110011; 
                    while (licznik == 8) {
                        unsigned char bit = ((lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3) ^ (lfsr >> 0)) & 1;
                        lfsr = (lfsr << 1) | bit;
                        lfsr &= 0x3F;
                        LATA = lfsr;
                        delay_ms(200);
                        checkButtons(&licznik, &prev6, &prev7);
                    }
                }
                break;
        }
    }
    return 0;
}
