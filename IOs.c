#include "TimeDelay.h"
#include "IOs.h"
#include <stdio.h>

uint16_t PB3_event; // PB3 event flag
uint16_t PB2_event; // PB2 event flag
uint16_t PB1_event; // PB1 event flag

uint16_t minutes = 0; // minutes counter
uint16_t seconds = 0; // seconds counter

int go = 0; // flag to check if minutes button has been held for long enough to increment by 5
int faster = 0; // flag to check if seconds button has been held for long enough to increment by 5

int timer = 0; // timer flag to toggle timer countdown on and off

void IOinit(){
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    newClk(500); // set clock freqwuncy to 500kHz
    
    // Configure RB8 as an Output and Initialize It Low
    TRISBbits.TRISB8 = 0;   // Set RB8 as an output
    LATBbits.LATB8 = 0;     // Initialize RB8 to low (0)
    
    // Configure RA4 as an Input with Pull-Up and Change Notification
    TRISAbits.TRISA4 = 1;   // Set RA4 as an input
    CNPU1bits.CN0PUE = 1;   // Enable pull-up resistor on CN0 (RA4)
    CNEN1bits.CN0IE = 1;    // Enable Change Notification Interrupt on CN0
    
     // Configure RB4 as an Input with Pull-Up and Change Notification
    TRISBbits.TRISB4 = 1;   // Set RB4 as an input
    CNPU1bits.CN1PUE = 1;   // Enable pull-up resistor on CN1 (RB4)
    CNEN1bits.CN1IE = 1;    // Enable Change Notification Interrupt on CN1

    // Configure RA2 as an Input with Pull-Up and Change Notification
    TRISAbits.TRISA2 = 1;   // Set RA2 as an input
    CNPU2bits.CN30PUE = 1;  // Enable pull-up resistor on CN30 (RA2)
    CNEN2bits.CN30IE = 1;   // Enable Change Notification Interrupt on CN30
    
    // Clear Event Flags and Reset Variables
    PB3_event = 0;
    PB2_event = 0;
    PB1_event = 0;
    
    // Configure Interrupt Priorities and Enable Interrupts
    IPC4bits.CNIP = 6;  // Set Change Notification Interrupt Priority to 6
    IFS1bits.CNIF = 0;  // Clear the Change Notification Interrupt Flag
    IEC1bits.CNIE = 1;  // Enable the Change Notification Interrupt
}

void IOcheck(){
    // PB1 pressed
    if (PB1_event && !PB2_event && !PB3_event) {    
        LATBbits.LATB8 = 1; //  Turn light on first
        while (PB1_event && !PB2_event && !PB3_event) {  // Stay in the loop while PB1 is pressed
            go++;
            if (go >= 10) {  // After holding the button for some time, adjust to next number ending in 0 or 5
                goToNextMultipleOf5Minutes();  // Call the function to round up seconds to nearest multiple of 5           
            } else if (go < 10) {  // Increment normally before threshold
                minutes++;
                // Ensure the seconds wrap around after 60
                if (minutes >= 60) {
                    minutes = 0;
                }
            }
            displayTime("SET"); // Display the current time
            LATBbits.LATB8 = ~LATBbits.LATB8; // Toggle the LED on or off for flashing effect
            delay_ms(500);  // Adjust the pace of the increment here
        }
        go = 0;
        LATBbits.LATB8 = 0; //  Turn light off
    }
    // PB2 pressed
    else if (PB2_event && !PB1_event && !PB3_event) {
        LATBbits.LATB8 = 1; //  Turn light on first
        while (PB2_event && !PB1_event && !PB3_event) {  // Stay in the loop while PB2 is pressed
            go++;
            if (go >= 10) {  // After holding the button for some time, adjust to next number ending in 0 or 5                
                goToNextMultipleOf5();  // Call the function to round up seconds to nearest multiple of 5            
            } else if (go < 10) {  // Increment normally before threshold
                seconds++;
                // Ensure the seconds wrap around after 60
                if (seconds >= 60) {
                    seconds = 0;
                }
            }
            displayTime("SET"); // Display the current time
            LATBbits.LATB8 = ~LATBbits.LATB8; // Toggle the LED on or off for flashing effect
            delay_ms(100);  // Adjust the pace of the increment here
        }
        go = 0;
        LATBbits.LATB8 = 0; //  Turn light off
    }
    // PB3 pressed
    else if (PB3_event && !PB2_event && !PB1_event) {
        if(timer==1 && (minutes > 0 || seconds > 0)){
            LATBbits.LATB8 = 1;           // Turn light on first
            while(timer==1 && (minutes > 0 || seconds > 0)){ // if timer is set to countdown and there is time countdown
                // decrease by 1 second at a time and minutes when seconds = 0
                if (seconds == 0) {
                    if (minutes > 0) {
                        minutes--;
                        seconds = 59;
                    }
                } else {
                    seconds--;
                }
                LATBbits.LATB8 = ~LATBbits.LATB8; //toggle light on and off every second
                delay_ms(740); // delay to countdown every second     
                if ( minutes == 0 && seconds == 0 ){ // time done display finish and leave
                    displayTime("FIN");
                    LATBbits.LATB8 = 1;           // Keep light on
                    break;
                }
                displayTime("CNT"); // display 
            }
        }
        timer = 0;  // stop timer          
        setup_timer1();               // Initialize and start the timer
        while (PB3_event && !PB2_event && !PB1_event) {  // Stay in the loop while PB3 is pressed
            if (IFS0bits.T1IF) {      // Check if Timer1 overflowed (every 3s)
                IFS0bits.T1IF = 0;    // Clear the Timer1 interrupt flag
                seconds = 0;      // Reset seconds and minutes 
                minutes = 0;
                displayTime("CLR"); // Display clear
                LATBbits.LATB8 = 0; // Turn light off
            }            
        }
        TMR1 = 0;                     // Reset Timer1 after exiting the loop
        T1CONbits.TON = 0;            // Stop Timer1
    }    
}

void goToNextMultipleOf5() {
    int remainder = seconds % 5;
    // If the seconds are 55 or higher but less than 59, go directly to 59
    if (seconds >= 55 && seconds < 59) {
        seconds = 59;
        return; // Exit the function after setting to 59
    }

    // If the current seconds are 59, reset to 0
    if (seconds == 59) {
        seconds = 0;
        return; // Exit the function after resetting to 0
    }

    // Otherwise, go to the next multiple of 5
    int nextSeconds = seconds + (5 - remainder);

    // If the next value exceeds 59, reset to 0
    if (nextSeconds >= 60) {
        seconds = 0;
    } else {
        seconds = nextSeconds;
    }
}

void goToNextMultipleOf5Minutes() {
    int remainder = minutes % 5;
    // If the minutes are 55 or higher but less than 59, go directly to 59
    if (minutes >= 55 && minutes < 59) {
        minutes = 59;
        return; // Exit the function after setting to 59
    }

    // If the current minutes are 59, reset to 0
    if (minutes == 59) {
        minutes = 0;
        return; // Exit the function after resetting to 0
    }

    // Otherwise, go to the next multiple of 5
    int nextMinutes = minutes + (5 - remainder);

    // If the next value exceeds 59, reset to 0
    if (nextMinutes >= 60) {
        minutes = 0;
    } else {
        minutes = nextMinutes;
    }
}

void displayTime(const char* mode) {
    char minstr[5];  // Allocate a buffer large enough to hold the minutes string
    char secstr[5];  // Allocate a buffer large enough to hold the seconds string
    
    sprintf(minstr, "%02d", minutes);  // Convert the integer to a string
    sprintf(secstr, "%02d", seconds);  // Convert the integer to a string
    
    char time[50];  // Allocate enough space for the final string
    
    strcpy(time, mode);         // Copy the mode string to the time buffer
    strcat(time, " ");          // Append a space
    strcat(time, minstr);       // Append the minutes string
    strcat(time, "m : ");      // Append ' m : '
    strcat(time, secstr);       // Append the seconds string
    strcat(time, "s");         // Append ' s'
    
    if (strcmp(mode, "FIN") == 0) {  // Use strcmp for string comparison
        strcat(time, " -- ALARM");  // Append the alarm message if mode is FIN
    }
    
    Disp2String(time);  // Display the concatenated string
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){
    //Don't forget to clear the CN interrupt flag!
    IFS1bits.CNIF = 0;
    
    // Check the state of each button
    if (PORTAbits.RA4 == 0) { // PB1 is active low (pressed)
        PB1_event = 1;
    }
    if (PORTAbits.RA4 == 1) { // PB1 is not pressed
        PB1_event = 0;
    }
    
    if (PORTBbits.RB4 == 0) { // PB2 is active low (pressed)
        PB2_event = 1;
    }
    if (PORTBbits.RB4 == 1) { // PB2 is not pressed
        PB2_event = 0;
    }
    
    if (PORTAbits.RA2 == 0) { // PB3 is active low (pressed)
        PB3_event = 1;
        timer = !timer; // toggle timer countdown on and off
    }
    if (PORTAbits.RA2 == 1) { // PB3 is not pressed
        PB3_event = 0;  
    }
}


