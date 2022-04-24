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

// Constantes
#define _XTAL_FREQ 4000000
#define TMR0_setup 236          // Asignar valor de reinicio para el TMR0

// Declarar variables
uint8_t selector = 0;
int valor = 0;
int unidades = 0;
int decenas = 0;
int centenas = 0;
uint8_t valor_tabla = 0;

//Prototipo de funciones
void setup(void);
void multiplexado(uint8_t select);
int map(uint8_t val);
void obtener_decimal(int value);
void tabla_7seg(int decimal);

//Interrupciones
void __interrupt() isr (void){
    if(PIR1bits.ADIF){                      // Evaluar bandera de interrupción del ADC
        if(ADCON0bits.CHS == 0){            // Si hubo interrupción, evaluar canal del ADC
            PORTB = ADRESH;                 // Asignar ADRESH al PORTB si el canal es AN0
        }
        else if(ADCON0bits.CHS == 1){
            //PORTC = ADRESH;
            valor = map(ADRESH);            // Mapear ADRESH si el canal es AN1
        }
        PIR1bits.ADIF = 0;                  // Limpiar bandera de interrupción del ADC
    }
    else if (INTCONbits.T0IF){  // Evaluar bandera de interrupción del TMR0
        TMR0 = TMR0_setup;      // Reiniciar TMR0
        INTCONbits.T0IF = 0;    // Limpiar bandera de interrupción del TMR0
        selector++;             // Incremetnar variable selector
        if (selector>2)         // Reiniciar dicha variable si es mayor que 2
            selector = 0;
    }
    return;
}

// Ciclo principal
void main (void){
    setup();                        
    while(1){
        if(ADCON0bits.GO == 0){             // Verificar que no hay proceso de conversión
            if(ADCON0bits.CHS == 0)
                ADCON0bits.CHS = 0b0001;    // Cambiar de canal
            else if(ADCON0bits.CHS == 1)
                ADCON0bits.CHS = 0b0000;    // Cambiar de canal
            __delay_us(40);                 // Esperar 40 us (tiempo de adquisición)
            ADCON0bits.GO = 1;              // Iniciar porceso de conversión
        }
        obtener_decimal(valor);             // Función para obtener decimal
        multiplexado(selector);             // Multiplexado de displays
    }
    return;
}

// Configuración
void setup(void){
    
    OSCCONbits.IRCF = 0b0110;   // FOSC: 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    ANSEL = 0b00000011;         // AN0 y AN1 como entradas analógicas
    ANSELH = 0;                 // I/O digitales para PORTB
    TRISA = 0b00000011;         // AN0 y AN1 como entradas
    PORTA = 0;                  // Limpiar PORTA
    TRISB = 0;                  // PORTB como salida
    PORTB = 0;                  // Limpiar PORTB
    TRISC = 0;                  // PORTC como salida
    PORTC = 0;                  // Limpiar PORTC
    TRISD = 0;                  // PORTD como salida
    PORTD = 0;                  // Limpiar PORTD               
    
    OPTION_REGbits.T0CS = 0;    // Configurar reloj interno para TMR0
    OPTION_REGbits.PSA = 0;     // Asignar prescaler al TMR0
    OPTION_REGbits.PS = 0b111;  // Prescaler 1:256
    TMR0 = TMR0_setup; 
    
    ADCON0bits.ADCS =0b01;      // FOSC/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Selecionar AN0
    ADCON1bits.ADFM = 0;        // Justificar a la izquierda
    ADCON0bits.ADON = 1;        // Habilitar modulo ADC
    __delay_us(40);             // Tiempo de muestreo

    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupción de perifericos
    INTCONbits.T0IE = 1;        // Habilitar interrupción del TRM0
    INTCONbits.T0IF = 0;        // Limpiar bandera de interrupción del TRM0
    PIE1bits.ADIE = 1;          // Habilitar interrupción del ADC
    PIR1bits.ADIF = 0;          // Limpiar bandera del ADC
    
    return;
}

// Función para mapear valor de ADRESH
int map(uint8_t val){
    return 2*val;               // Multiplicar por 2 para convertir de 0 a 500 (aprox)       
}

// Función para obtener el decimal del contador en PORTA
void obtener_decimal(int valor){
    centenas = 0;
    decenas = 0;
    unidades = 0;
    if (valor >= 100){              // Evaluar si el valor es mayor a 100
        centenas = valor/100;       // Si lo es, dividir entre 100 y luego obtener el módulo/residuo
        valor = valor%100;
    }
    if (valor >= 10){               // Evaluar si ahora el valor es mayor a 10
        decenas = valor/10;         // Si lo es, dividir entre 10 y obtener el módulo
        unidades = valor%10;        // Pasar el módulo a la variable de las unidades
    }
    if (valor < 10)                 // Evaluar si el valor no es mayor que 100 ni mayor que 10 
        unidades = valor;           // Si no lo es, guardar dicho valor directamente en la variable de unidades
    return;
}

// Multiplexado de displays
void multiplexado(uint8_t selector){
    PORTD = 0;                  // Apagar PORTD (apagar todos los displays)
    PORTCbits.RC7 = 1;
    switch(selector){           // Evaluar variable para determinar el display que debe encenderse
        case 0:
            tabla_7seg(unidades);     // Buscar equivalente de unidades para mostrar en display
            PORTC = valor_tabla;      // Mover el valor del display de unidades al PORTC
            PORTDbits.RD0 = 1;        // Encedecer el display de unidades
            break;
        case 1:
            tabla_7seg(decenas);      // Buscar equivalente de decenas para mostrar en display
            PORTC = valor_tabla;      // Mover el valor del display de decenas al PORTC
            PORTDbits.RD1 = 1;        // Encender el display de decenas
            break;
        case 2:
            tabla_7seg(centenas);     // Buscar equivalente de centenas para mostrar en display
            PORTC = valor_tabla;      // Mover el valor del display de centenas al PORTC
            PORTCbits.RC7 = 1;        // Encender punto decimal para el tercer dígito
            PORTDbits.RD2 = 1;        // Encender el display de centenas
            break;
        default:
            PORTD = 0;                // Apagar todos los displays
    }
    return;
}

void tabla_7seg(int  decimal){
    switch(decimal){
        case 0:
            valor_tabla = 0b00111111;       // 0 en display 7 seg
            break;
        case 1:
            valor_tabla = 0b00000110;       // 1 en display 7 seg
            break;
        case 2:
            valor_tabla = 0b01011011;       // 2 en display 7 seg
            break;
        case 3:
            valor_tabla = 0b01001111;       // 3 en display 7 seg
            break;
        case 4:
            valor_tabla = 0b01100110;       // 4 en display 7 seg
            break;
        case 5:
            valor_tabla = 0b01101101;       // 5 en display 7 seg
            break;
        case 6:
            valor_tabla = 0b01111101;       // 6 en display 7 seg
            break;
        case 7:
            valor_tabla = 0b00000111;       // 7 en display 7 seg
            break;
        case 8:
            valor_tabla = 0b01111111;       // 8 en display 7 seg
            break;
        case 9:
            valor_tabla = 0b01101111;       // 9 en display 7 seg
            break;
        default:
            valor_tabla = 0b00000000;       // Display apagado
            break;
    }
    return;
}