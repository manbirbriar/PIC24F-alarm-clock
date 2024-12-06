#include "xc.h"
#include "TimeDelay.h"


void setup_timer1() {
    T1CONbits.TCKPS = 0b10; // Prescaler 1:64
    PR1 = 3000*4;           // Set PR1 to 4 x 3000ms for 3 second overflow at 500 kHz Fcy
    TMR1 = 0;               // Clear Timer1 register
    T1CONbits.TCS = 0;      // Use internal clock (Fosc/2)
    IFS0bits.T1IF = 0;      // Clear Timer1 interrupt flag
    T1CONbits.TON = 1;      // Start Timer1
}

void delay_ms(uint16_t time_ms){    
    //T2CON configuration
    T2CONbits.T32 = 0; // disable 32-bit timer mode to operate in 16-bit
    T2CONbits.TCKPS = 0b10; // prescalar value of 64
    T2CONbits.TSIDL = 0; //operates in idle mode
    T2CONbits.TCS = 0; //use internal clock
    
    //Timer 2 interrupt configuration
    IPC1bits.T2IP = 4; //priority level
    IFS0bits.T2IF = 0; //clears timer 2 interrupt flag
    IEC0bits.T2IE = 1; //enable time interrupt

    PR2 = time_ms*4; //PR2 calculation
    TMR2= 0;

    T2CONbits.TON = 1;
    Idle();
    
}  

// Timer 2 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF=0; //Clear timer 2 interrupt flag
    TMR2 = 0; //Stops TMR2
    T2CONbits.TON=0; //stop timer
    return;

}