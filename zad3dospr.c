/*
 * File:   newmainXC16.c
 * Author: local
 *
 * Created on 17 kwietnia 2024, 15:20
 */

#pragma config POSCMOD = NONE             // Primary Oscillator Select (primary oscillator disabled)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC Oscillator without Postscaler)
#pragma config IESO = OFF               // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) disabled)
// CONFIG1
#pragma config WDTPS = PS32768 // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128 // WDT Prescaler (1:128)
#pragma config WINDIS = ON // Watchdog Timer Window Mode disabled
#pragma config FWDTEN = OFF // Watchdog Timer disabled
#pragma config ICS = PGx2 // Emulator/debugger uses EMUC2/EMUD2
#pragma config GWRP = OFF // Writes to program memory allowed
#pragma config GCP = OFF // Code protection is disabled
#pragma config JTAGEN = OFF // JTAG port is disabled

#include "xc.h"
#include <libpic30.h>

// Definicja makra dla częstotliwości roboczej oscylatora
#define FCY 4000000UL   // Częstotliwość robocza oscylatora (FCY = FOSC / 2, gdzie FOSC = 8 MHz)

// Definicje makr dla pinów LCD
#define LCD_E LATDbits.LATD4  
#define LCD_RW LATDbits.LATD5
#define LCD_RS LATBbits.LATB15
#define LCD_DATA LATE

// Definicje makr dla komend LCD
#define LCD_CLEAR 0x01        // Komenda czyszczenia ekranu LCD
#define LCD_HOME 0x02         // Komenda ustawienia kursora na początku
#define LCD_ON 0x0C           // Komenda włączenia wyświetlacza LCD
#define LCD_OFF 0x08          // Komenda wyłączenia wyświetlacza LCD
#define LCD_CONFIG 0x38       // Konfiguracja wyświetlacza LCD
#define LCD_CURSOR 0x80       // Komenda ustawienia kursora
#define LINE1 0x00            // Adres pierwszej linii
#define LINE2 0x40            // Adres drugiej linii
#define LCD_CUST_CHAR 0x40    // Komenda zapisu niestandardowego znaku
#define LCD_SHIFT_R 0x1D      // Komenda przesunięcia w prawo
#define LCD_SHIFT_L 0x1B      // Komenda przesunięcia w lewo

// Definicje funkcji opóźnienia w mikrosekundach i milisekundach
void __delay_us(unsigned long us){
    __delay32(us * FCY / 1000000); // Opóźnienie w mikrosekundach
}

void __delay_ms(unsigned long ms){
    __delay32(ms * FCY / 1000);    // Opóźnienie w milisekundach
}

// Definicje funkcji wysyłających komendy i dane do LCD
void LCD_sendCommand(unsigned char command){
    LCD_RW = 0;     // Ustawienie bitu RW na 0 (zapis)
    LCD_RS = 0;     // Ustawienie bitu RS na 0 (komenda)
    LCD_E = 1;      // Ustawienie bitu E na 1 (początek transmisji)
    LCD_DATA = command; // Wysłanie komendy do LCD
    __delay_us(50); // Opóźnienie dla zapisania danych
    LCD_E = 0;      // Ustawienie bitu E na 0 (koniec transmisji)
}

void LCD_sendData(unsigned char data){
    LCD_RW = 0;     // Ustawienie bitu RW na 0 (zapis)
    LCD_RS = 1;     // Ustawienie bitu RS na 1 (dane)
    LCD_E = 1;      // Ustawienie bitu E na 1 (początek transmisji)
    LCD_DATA = data; // Wysłanie danych do LCD
    __delay_us(50); // Opóźnienie dla zapisania danych
    LCD_E = 0;      // Ustawienie bitu E na 0 (koniec transmisji)
}

// Definicja niestandardowego znaku
unsigned char symbol1[8] = {
    0b00000,
    0b00000,
    0b01010,
    0b10101,
    0b10001,
    0b01010,
    0b00100,
    0b00000
};

// Funkcja drukowania ciągu znaków na LCD
void LCD_print(unsigned char* string){
    while(*string){
        LCD_sendData(*string++); // Wysyłanie kolejnych znaków
    }
}

// Funkcja ustawiania kursora na LCD
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

// Funkcja zapisu niestandardowego znaku do LCD
void LCD_saveCustChar(unsigned char slot, unsigned char *array){
    unsigned char i;
    LCD_sendCommand(LCD_CUST_CHAR + (slot*8)); // Komenda zapisu znaku
    for(i=0; i<8; i++) {
        LCD_sendData(array[i]); // Zapis poszczególnych linii znaku
    }
}

// Funkcja inicjalizująca wyświetlacz LCD
void LCD_init(){
    __delay_ms(20); // Opóźnienie początkowe
    LCD_sendCommand(LCD_CONFIG); // Konfiguracja LCD
    __delay_us(50); // Opóźnienie
    LCD_sendCommand(LCD_ON); // Włączenie LCD
    __delay_us(50); // Opóźnienie
    LCD_sendCommand(LCD_CLEAR); // Czyszczenie ekranu LCD
    __delay_ms(2); // Opóźnienie
}

int main(void) {
    TRISB = 0x7FFF; // Ustawienie kierunku pinów (RB15 jako wyjście)
    TRISD = 0x0000; // Ustawienie kierunku pinów (RD4, RD5 jako wyjścia)
    TRISE = 0x0000; // Ustawienie kierunku pinów (RE jako wyjścia)
    
    LCD_init(); // Inicjalizacja wyświetlacza LCD
    LCD_saveCustChar(0, symbol1); // Zapis niestandardowego znaku
    
    while(1){
        LCD_setCursor(1, 0); // Ustawienie kursora na początku pierwszego wiersza
        LCD_sendData(0); // Wysłanie niestandardowego znaku
        LCD_print("Kup gry w okazyjnej cenie!"); // Wyświetlenie napisu w pierwszym wierszu
        LCD_sendData(0); // Wysłanie niestandardowego znaku
        LCD_setCursor(2, 0); // Ustawienie kursora na początku drugiego wiersza
        LCD_sendData(0); // Wysłanie niestandardowego znaku
        LCD_print("Oferta dostepna tylko dzisiaj!"); // Wyświetlenie napisu w drugim wierszu
        LCD_sendData(0); // Wysłanie niestandardowego znaku
        __delay_ms(300); // Opóźnienie 300 ms
        LCD_sendCommand(LCD_SHIFT_L); // Przesunięcie napisu w lewo
    }
    return 0;
}
