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


// Definicja funkcji delay w us i ms - operujacych na jednostkach czasu zamiast cykli pracy oscylatora
void __delay_us(unsigned long us){
    __delay32(us * FCY / 1000000);
}

void __delay_ms(unsigned long ms){
    __delay32(ms * FCY / 1000);
}

// Funkcja wysylajaca komendy do LCD
void LCD_sendCommand(unsigned char command){
    LCD_RW = 0;     // Zapis
    LCD_RS = 0;     // Przesylanie komend
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = command; // Wyslanie komendy
    __delay_us(50); // Opoznienie konieczne dla zapisania danych
    LCD_E = 0;      // Konieczne zablokowanie transmisji po przeslaniu komunikatu
}

// Funkcja wysylajaca dane do LCD
void LCD_sendData(unsigned char data){
    LCD_RW = 0;     // Zapis
    LCD_RS = 1;     // Przesylanie danych
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = data; // Wyslanie danych
    __delay_us(50);
    LCD_E = 0;
}

// Funkcja print wyswietlajaca kolejne 8-bitowe znaki w petli while - * oznacza przypisanie nie wartosci zmiennej lecz jej adresu.
void LCD_print(unsigned char* string){
    while(*string){
        LCD_sendData(*string++); // Wysylanie kolejnych znakow
    }
}

// Funkcja ustawiajaca kursor na LCD
void LCD_setCursor(unsigned char row, unsigned char col){
    unsigned char address;
    if(row == 1){
        address = LCD_CURSOR + LINE1 + col; // Adres pierwszej linii
    }
    if (row == 2){
        address = LCD_CURSOR + LINE2 + col; // Adres drugiej linii
    }
    LCD_sendCommand(address); // Ustawienie kursora
}

// Funkcja inicjalizujaca wyswietlacz LCD. Wysyla niezbedne komendy jak LCD_CONFIG i LCD_ON
void LCD_init(){
    __delay_ms(20);           // Opoznienie poczatkowe
    LCD_sendCommand(LCD_CONFIG); // Konfiguracja LCD
    __delay_us(50);           // Opoznienie wymagane przez specyfikacje wyswietlacza
    LCD_sendCommand(LCD_ON);  // Wlaczenie LCD
    __delay_us(50);
    LCD_sendCommand(LCD_CLEAR); // Czyszczenie wyswietlacza
    __delay_ms(2);
}

// Funkcja odczytująca wartość z ADC i zwracająca ją
unsigned int display(void){
    AD1CON1bits.SAMP = 1;     // Rozpoczęcie próbkowania
    while(!AD1CON1bits.DONE); // Czekanie na zakończenie konwersji
    return ADC1BUF0;          // Zwrot wartości z bufora ADC
}

// Funkcja wyświetlająca ustawiony czas na LCD
void time_display(unsigned int time){
    unsigned char minutes = time / 60;
    unsigned char seconds = time % 60;

    LCD_setCursor(1, 0);      // Ustawienie kursora na pierwszej linii
    LCD_print("Set time: ");
    LCD_sendData('0' + minutes);   // Wyświetlenie minut
    LCD_sendData(':');
    LCD_sendData('0' + (seconds / 10)); // Wyświetlenie dziesiątek sekund
    LCD_sendData('0' + (seconds % 10)); // Wyświetlenie jednostek sekund
}

// Funkcja wyświetlająca ustawioną moc na LCD
void power_display(unsigned int power){
    unsigned char hundreds = power / 100;
    unsigned char tens = (power / 10) % 10;
    unsigned char units = power % 10;

    LCD_setCursor(2, 0);      // Ustawienie kursora na drugiej linii
    LCD_print("Set power: ");
    LCD_sendData('0' + hundreds); // Wyświetlenie setek
    LCD_sendData('0' + tens);     // Wyświetlenie dziesiątek
    LCD_sendData('0' + units);    // Wyświetlenie jednostek
}

// Funkcja sprawdzająca stan przycisku
bool readButton(volatile unsigned char *port, unsigned char pin) {
    if ((*port & (1 << pin)) == 0) { // Sprawdzenie czy przycisk jest wciśnięty
        __delay_ms(50);             
        if ((*port & (1 << pin)) == 0) { // Sprawdzenie ponowne
            return true;             // Przycisk jest wciśnięty
        }
    }
    return false;                    // Przycisk nie jest wciśnięty
}

int main(void) {
    bool currentS6 = 0, prevS6 = 0;
    bool currentS7 = 0, prevS7 = 0;
    bool currentS8 = 0, prevS8 = 0;
    unsigned int power = 0;
    unsigned int time = 0;
    bool start_button = false;

    // Konfiguracja ADC
    AD1CON1 = 0x80E4; 
    AD1CON2 = 0x0404;
    AD1CON3 = 0x0F00;
    AD1CHS = 0;
    AD1CSSL = 0x0020;

    // Konfiguracja portów
    TRISB = 0x7FFF;  // RB15 jako wyjście
    TRISD = 0xFFE7;  // RD4, RD5 jako wyjścia
    TRISE = 0x0000;  // RE jako wyjścia

    // Inicjalizacja LCD
    LCD_init(); 

    while(1) {
        // Aktualizacja stanów przycisków
        prevS6 = currentS6;
        prevS7 = currentS7;
        prevS8 = currentS8;

        currentS6 = readButton(&PORTD, 6);
        currentS7 = readButton(&PORTD, 7);
        currentS8 = readButton(&PORTD, 8);
      
        // Odczyt wartości z ADC
        power = display();
        if (power > 999) { 
            power = 999; // Ograniczenie maksymalnej wartości mocy do 999
        }

        // Obsługa przycisku S6
        if (currentS6 && !prevS6) {
            time += 15; // Dodanie 15 sekund do czasu
        }

        // Obsługa przycisku S7
        if (currentS7 && !prevS7) {
            start_button = !start_button; // Przełączenie stanu przycisku start
        }

        // Obsługa przycisku S8
        if (currentS8 && !prevS8) {
            time = 0; // Resetowanie czasu
            start_button = false; // Zatrzymanie licznika
        }

        // Aktualizacja czasu co sekundę
        if (start_button && time > 0) {
            __delay_ms(1000);
            time--;
        }

        // Zatrzymanie licznika gdy czas dobiegnie końca
        if (time == 0) {
            start_button = false;
        }

        // Wyświetlenie wartości mocy i czasu na LCD
        power_display(power);
        time_display(time);
    }
}
