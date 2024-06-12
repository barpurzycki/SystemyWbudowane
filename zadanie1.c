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

// Definiowanie częstotliwości oscylatora i cyklu zegara
#define FOSC 40000000UL  // 40 MHz
#define FCY (FOSC / 2)   // 20 MHz

// Funkcja konwertująca wartość na kod Graya
int GrayConvert(unsigned char i) {
    return (i >> 1) ^ i;
}

// Funkcja opóźnienia w milisekundach
void delay_ms(unsigned int ms) {
    __delay32(ms * (FCY / 1000));
}

// Funkcja sprawdzająca stan przycisków i aktualizująca licznik
void checkButtons(int *licznik, char *prev6, char *prev7) {
    char current6 = PORTDbits.RD6;
    char current7 = PORTDbits.RD7;

    // Jeśli przycisk RD6 został wciśnięty, zmniejsz licznik
    if (current6 == 1 && *prev6 == 0) {
        (*licznik)--;
    }

    // Jeśli przycisk RD7 został wciśnięty, zwiększ licznik
    if (current7 == 1 && *prev7 == 0) {
        (*licznik)++;
    }

    // Aktualizacja stanów poprzednich przycisków
    *prev6 = current6;
    *prev7 = current7;

    // Zapobieganie przekroczeniu zakresu licznika
    if (*licznik < 0) {
        *licznik = 8;
    } else if (*licznik > 8) {
        *licznik = 0;
    }
}

int main(void) {
    int licznik = 0;  // Inicjalizacja licznika
    unsigned char portValue = 0;  // Wartość wyjściowa portu
    char prev6, prev7;  // Poprzednie stany przycisków
    TRISA = 0x0000;  // Ustawienie portu A jako wyjściowy
    TRISD = 0xFFFF;  // Ustawienie portu D jako wejściowy

    // Inicjalizacja stanów poprzednich przycisków
    prev6 = PORTDbits.RD6;
    prev7 = PORTDbits.RD7;

    while (1) {
        checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków


Jasne, oto Twój kod z dodanymi komentarzami wyjaśniającymi jego działanie:

c
Skopiuj kod
// Konfiguracja bitów konfiguracji mikrokontrolera
// CONFIG2
#pragma config POSCMOD = HS      // High-Speed Crystal Mode
#pragma config OSCIOFNC = OFF    // OSC2 Pin Function (OSC2 is clock output)
#pragma config FCKSM = CSDCMD    // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRIPLL    // Oscillator Select (Primary Oscillator with PLL module (HSPLL, ECPLL))
#pragma config IESO = OFF        // Internal External Switchover (Disabled)

// CONFIG1
#pragma config WDTPS = PS32768   // Watchdog Timer Postscale Select (1:32,768)
#pragma config FWPSA = PR128     // WDT Prescaler (1:128)
#pragma config WINDIS = ON       // Windowed WDT (Windowed Watchdog Timer enabled)
#pragma config FWDTEN = OFF      // Watchdog Timer (WDT disabled)
#pragma config ICS = PGx2        // Emulator Pin Placement Select (Emulator functions are shared with PGEC2/PGED2)
#pragma config GWRP = OFF        // General Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF         // General Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF      // JTAG Port Enable (JTAG port is disabled)

#include "xc.h"
#include "libpic30.h"

// Definiowanie częstotliwości oscylatora i cyklu zegara
#define FOSC 40000000UL  // 40 MHz
#define FCY (FOSC / 2)   // 20 MHz

// Funkcja konwertująca wartość na kod Graya
int GrayConvert(unsigned char i) {
    return (i >> 1) ^ i;
}

// Funkcja opóźnienia w milisekundach
void delay_ms(unsigned int ms) {
    __delay32(ms * (FCY / 1000));
}

// Funkcja sprawdzająca stan przycisków i aktualizująca licznik
void checkButtons(int *licznik, char *prev6, char *prev7) {
    char current6 = PORTDbits.RD6;
    char current7 = PORTDbits.RD7;

    // Jeśli przycisk RD6 został wciśnięty, zmniejsz licznik
    if (current6 == 1 && *prev6 == 0) {
        (*licznik)--;
    }

    // Jeśli przycisk RD7 został wciśnięty, zwiększ licznik
    if (current7 == 1 && *prev7 == 0) {
        (*licznik)++;
    }

    // Aktualizacja stanów poprzednich przycisków
    *prev6 = current6;
    *prev7 = current7;

    // Zapobieganie przekroczeniu zakresu licznika
    if (*licznik < 0) {
        *licznik = 8;
    } else if (*licznik > 8) {
        *licznik = 0;
    }
}

int main(void) {
    int licznik = 0;  // Inicjalizacja licznika
    unsigned char portValue = 0;  // Wartość wyjściowa portu
    char prev6, prev7;  // Poprzednie stany przycisków
    TRISA = 0x0000;  // Ustawienie portu A jako wyjściowy
    TRISD = 0xFFFF;  // Ustawienie portu D jako wejściowy

    // Inicjalizacja stanów poprzednich przycisków
    prev6 = PORTDbits.RD6;
    prev7 = PORTDbits.RD7;

    // Główna pętla programu
    while (1) {
        checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków

        switch (licznik) {
            case 0: // Licznik rosnący 0-255
                portValue = 0;
                while (licznik == 0) {
                    LATA = portValue;  // Ustawienie wartości portu
                    delay_ms(200);  // Opóźnienie
                    portValue++;  // Zwiększenie wartości portu
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 1: // Licznik malejący 255-0
                portValue = 255;  // Inicjalizacja wartości portu na 255
                while (licznik == 1) {  
                    LATA = portValue;  // Ustawienie wartości portu 
                    delay_ms(200);  // Opóźnienie
                    portValue--;  // Zmniejszenie wartości portu
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 2: // Licznik Graya rosnący 0-255
                portValue = 0;  // Inicjalizacja wartości portu na 0
                while (licznik == 2) { 
                    LATA = GrayConvert(portValue);  // Konwersja na kod Graya i ustawienie wartości portu
                    delay_ms(200);  // Opóźnienie
                    portValue++;  // Zwiększenie wartości portu
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 3: // Licznik Graya malejący 255-0
                portValue = 255;  // Inicjalizacja wartości portu na 255
                while (licznik == 3) { 
                    LATA = GrayConvert(portValue);  // Konwersja na kod Graya i ustawienie wartości portu 
                    delay_ms(200);  // Opóźnienie 
                    portValue--;  // Zmniejszenie wartości portu
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 4: // Licznik BCD rosnący 0-99
                portValue = 0;  // Inicjalizacja wartości portu na 0
                while (licznik == 4) { 
                    LATA = ((portValue / 10) << 4) | (portValue % 10);  // Konwersja na kod BCD i ustawienie wartości portu 
                    delay_ms(200);  // Opóźnienie
                    portValue++;  // Zwiększenie wartości portu
                    if (portValue > 99) portValue = 0;  // Reset wartości portu, jeśli przekroczy 99
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 5: // Licznik BCD malejący 99-0
                portValue = 99;  // Inicjalizacja wartości portu na 99
                while (licznik == 5) { 
                    LATA = ((portValue / 10) << 4) | (portValue % 10);  // Konwersja na kod BCD i ustawienie wartości portu 
                    delay_ms(200);  // Opóźnienie 
                    portValue--;  // Zmniejszenie wartości portu
                    if (portValue > 99) portValue = 99;  // Zapobieganie przekroczeniu wartości 99
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 6: // 3-bitowy wąż poruszający się w lewo i prawo
                {
                    unsigned char snake[10] = {7, 14, 28, 56, 112, 224, 112, 56, 28, 14};  // Wzorzec ruchu węża
                    unsigned int i = 0;  // Indeks dla wzorca
                    while (licznik == 6) {  
                        LATA = snake[i];  // Ustawienie wartości portu zgodnie ze wzorcem
                        delay_ms(200);  // Opóźnienie 
                        i++;  // Zwiększenie indeksu
                        if (i >= 10) 
                        {
                            i = 0;
                        }  // Reset indeksu po osiągnięciu końca wzorca
                        checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                    }
                }
                break;
            case 7: // Kolejka
                portValue = 0;  // Inicjalizacja wartości portu na 0
                while (licznik == 7) {  
                    for (int i = 0; i < 8; i++) {  // Pętla dla każdego bitu w bajcie
                        int x = 1;  // Inicjalizacja zmiennej do przesuwania bitów
                        for (int j = i; j < 8; j++) {  // Pętla do przesuwania bitów
                            LATA = portValue + x;  // Ustawienie wartości portu 
                            x = x << 1;  // Przesunięcie bitowe w lewo
                            delay_ms(200);  // Opóźnienie 
                            checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                            if (licznik != 7) break;  // Wyjście z pętli, jeśli licznik się zmieni
                        }
                        portValue += x >> 1;  // Aktualizacja wartości portu
                        checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                        if (licznik != 7) break;  // Wyjście z pętli, jeśli licznik się zmieni
                    }
                    portValue = 0;  // Reset wartości portu
                    checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie przycisków
                }
                break;
            case 8: // Generator LFSR
                {
                    unsigned char lfsr = 0b1110011;  // Inicjalizacja rejestru przesuwającego
                    while (licznik == 8) {  
                        unsigned char bit = ((lfsr >> 5) ^ (lfsr >> 4) ^ (lfsr >> 3) ^ (lfsr >> 0)) & 1;  // Obliczenie nowego bitu
                        lfsr = (lfsr << 1) | bit;  // Przesunięcie i dodanie nowego bitu
                        lfsr &= 0x3F;  // Maskowanie do 6 bitów
                        LATA = lfsr;  // Ustawienie wartości portu 
                        delay_ms(200);  // Opóźnienie 
                        checkButtons(&licznik, &prev6, &prev7);  // Sprawdzanie  przycisków
                    }
                }
                break;
        }
    }
    return 0;
}
