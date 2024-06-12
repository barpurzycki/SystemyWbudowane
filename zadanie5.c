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

// Funkcja wysyłająca komendę do LCD
void LCD_sendCommand(unsigned char command){
    LCD_RW = 0;     // Zapis
    LCD_RS = 0;     // Przesylanie komend
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = command; // Wyslanie komendy
    __delay_us(50); // Opoznienie konieczne dla zapisania danych
    LCD_E = 0;      // Konieczne zablokowanie transmisji po przeslaniu komunikatu
}

// Funkcja wysyłająca dane do LCD
void LCD_sendData(unsigned char data){
    LCD_RW = 0;     // Zapis
    LCD_RS = 1;     // Przesylanie danych
    LCD_E = 1;      // Otwarcie transmisji danych
    LCD_DATA = data; // Wyslanie danych
    __delay_us(50); // Opoznienie konieczne dla zapisania danych
    LCD_E = 0;      // Konieczne zablokowanie transmisji po przeslaniu danych
}

// Funkcja print wyświetlająca kolejne 8-bitowe znaki w pętli while
void LCD_print(unsigned char* string){
    while(*string){
        LCD_sendData(*string++); // Wysylanie kolejnych znakow
    }
}

// Funkcja ustawiająca kursor na LCD
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

// Funkcja inicjalizująca wyświetlacz LCD
void LCD_init(){
    __delay_ms(20);           // Opoznienie poczatkowe
    LCD_sendCommand(LCD_CONFIG); // Konfiguracja LCD
    __delay_us(50);           // Opoznienie wymagane przez specyfikacje wyswietlacza
    LCD_sendCommand(LCD_ON);  // Wlaczenie LCD
    __delay_us(50);           // Opoznienie wymagane przez specyfikacje wyswietlacza
    LCD_sendCommand(LCD_CLEAR); // Czyszczenie wyswietlacza
    __delay_ms(2);            // Opoznienie wymagane przez specyfikacje wyswietlacza
}

// Funkcja wyświetlająca czas na LCD
void displayTime(unsigned char row, unsigned int time) {
    unsigned char minutes = time / 60; // Przeliczenie czasu na minuty
    unsigned char seconds = time % 60; // Przeliczenie czasu na sekundy
    LCD_setCursor(row, 0);             // Ustawienie kursora na odpowiedni wiersz
    LCD_sendData('0' + (minutes / 10)); // Wyświetlenie dziesiątek minut
    LCD_sendData('0' + (minutes % 10)); // Wyświetlenie jedności minut
    LCD_sendData(':');
    LCD_sendData('0' + (seconds / 10)); // Wyświetlenie dziesiątek sekund
    LCD_sendData('0' + (seconds % 10)); // Wyświetlenie jedności sekund
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
    // Zmienne przechowujące stany przycisków i tury graczy
    bool currentP1 = 0, prevP1 = 0;
    bool currentP2 = 0, prevP2 = 0;
    bool p1_turn = true;             // Flaga określająca czy tura gracza 1
    unsigned int t1 = 60;            // Czas dla gracza 1
    unsigned int t2 = 60;            // Czas dla gracza 2

    // Konfiguracja portów
    TRISB = 0x7FFF;   // RB15 jako wyjście
    TRISD = 0xFFE7;   // RD4, RD5 jako wyjścia
    TRISE = 0x0000;   // RE jako wyjścia

    // Inicjalizacja LCD
    LCD_init(); 

    while(1) {
        // Aktualizacja stanów przycisków
        prevP1 = currentP1;
        prevP2 = currentP2;

        currentP1 = readButton(&PORTD, 7); // Odczyt przycisku gracza 1
        currentP2 = readButton(&PORTD, 6); // Odczyt przycisku gracza 2

        // Zmiana tury na gracza 1, jeśli przycisk został naciśnięty i tura nie należy do gracza 1
        if (currentP1 && !prevP1 && !p1_turn) {
            p1_turn = true;
        }

        // Zmiana tury na gracza 2, jeśli przycisk został naciśnięty i tura należy do gracza 1
        if (currentP2 && !prevP2 && p1_turn) {
            p1_turn = false;
        }

        // Zmniejszanie czasu gracza 1, jeśli jego tura i czas większy od 0
        if (p1_turn && t1 > 0) {
            __delay_ms(1000); // Opóźnienie 1 sekundy
            t1--; // Zmniejszenie czasu o 1 sekundę
        }

        // Zmniejszanie czasu gracza 2, jeśli jego tura i czas większy od 0
        if (!p1_turn && t2 > 0) {
            __delay_ms(1000); // Opóźnienie 1 sekundy
            t2--; // Zmniejszenie czasu o 1 sekundę
        }
        
        // Sprawdzenie końca czasu dla graczy
        if (t1 == 0 || t2 == 0) {
            LCD_setCursor(1, 0); // Ustawienie kursora na pierwszej linii
            LCD_print("Koniec czasu"); // Wyświetlenie komunikatu końca czasu
            LCD_setCursor(2, 0); // Ustawienie kursora na drugiej linii
            if (t1 == 0) {
                LCD_print("Gracz 2 wygrywa"); // Wyświetlenie zwycięzcy, jeśli graczowi 1 skończył się czas
            } else {
                LCD_print("Gracz 1 wygrywa"); // Wyświetlenie zwycięzcy, jeśli graczowi 2 skończył się czas
            }
            while(1); // Zatrzymanie programu po końcu czasu
        }

        // Wyświetlanie czasu dla obu graczy
        displayTime(1, t1); // Wyświetlenie czasu dla gracza 1
        displayTime(2, t2); // Wyświetlenie czasu dla gracza 2
    }   
}
