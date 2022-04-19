/* 
 * File:   mainL8.c
 * Author: Jeferson Noj
 *
 * Created on 19 de abril de 2022, 07:16 AM
 */

// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

//Constantes
#define _XTAL_FREQ 4000000

//Prototipo de funciones
void setup(void);

//Interrupciones
void __interrupt() isr (void){
    if(PIR1bits.ADIF){
        if(ADCON0bits.CHS == 0){
            PORTC = ADRESH;
        }
        else if(ADCON0bits.CHS == 1){
            PORTB = ADRESH;
        }
        PIR1bits.ADIF = 0;
    }
    return;
}

// Ciclo principal
void main (void){
    setup();                        
    while(1){
        if(ADCON0bits.GO == 0){
            if(ADCON0bits.CHS == 0)
                ADCON0bits.CHS = 0b0001;
            else if(ADCON0bits.CHS == 1)
                ADCON0bits.CHS = 0b0000;
            __delay_us(40);
            ADCON0bits.GO = 1; 
        }
    }
    return;
}

// Configuración
void setup(void){
    
    OSCCONbits.IRCF = 0b0110;    // FOSC: 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    ANSEL = 0b00000011;         // AN0 y AN1 como entradas analógicas
    ANSELH = 0;                 // I/O digitales para PORTB
    TRISA = 0b00000011;         // AN0 y AN1 como entradas
    PORTA = 0;                  // Limpiar PORTA
    TRISB = 0;                  // PORTB como salida
    PORTB = 0;                  // Limpiar PORTB
    TRISC = 0;                  // PORTC como salida
    PORTC = 0;                  // Limpiar PORTC
    //TRISD = 0;                  // PORTD como salida
    //PORTD = 0;                  // Limpiar PORTD               
    
    ADCON0bits.ADCS =0b01;      // FOSC/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Selecionar AN0
    ADCON1bits.ADFM = 0;        // Justificar a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);             // Tiempo de muestreo

    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupción de perifericos
    PIE1bits.ADIE = 1;          // Habilitar interrupción del ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera del ADC
    
    return;
}