#include "msp.h" //must include device header file for specific msp432 series

volatile uint32_t pulse_count = 0;  // Global variable to store number of pulses in 1 second
volatile uint32_t rpm = 0; // Global variable to store calculated RPM

//PORT2_IRQHandler - Interrupt handler for Port 2 (this will be used for sensor input)

void PORT2_IRQHandler(void) {
    if (P2->IFG & BIT1) {   // to check if interrupt occurred on pin P2.1
        pulse_count++;  // to increment the pulse count (1 pulse = 1 rotation)
        P2->IFG &= ~BIT1;  // to clear the interrupt flag for P2.1
    }
}

//TA0_0_IRQHandler - Timer_A interrupt handler (it is called after every 1 second)

void TA0_0_IRQHandler(void) {
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; //to clear Timer interrupt flag
    rpm = pulse_count * 60; // to calculate RPM (pulses per second × 60)
    pulse_count = 0;  // Reset the pulse count for next interval
}

void main(void) {
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // Stop the Watchdog Timer (WDT)
    
    //GPIO SETUP FOR SENSOR INPUT
    P2->DIR &= ~BIT1;   // Set P2.1 as input pin (clear bit)
    P2->REN |= BIT1;    // Enable pull-up/pull-down resistor on P2.1
    P2->OUT |= BIT1;    // Set P2.1 to use pull-up resistor

    P2->IES |= BIT1;    // Interrupt on falling edge (sensor output drops from HIGH to LOW)
    P2->IFG &= ~BIT1;   // Clear interrupt flag for P2.1
    P2->IE |= BIT1;     // Enable interrupt for P2.1

    NVIC_EnableIRQ(PORT2_IRQn);  // Enable Port 2 interrupt in NVIC(nested vector interrupt controller)

    // TIMER_A SETUP FOR 1-SECOND INTERRUPT 
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK |   // Use SMCLK (3.0 MHz) as timer clock source
                    TIMER_A_CTL_MC__UP |        
                    TIMER_A_CTL_CLR;            
    TIMER_A0->CCR[0] = 3000000;  // Set timer period for 1 second (3000000 cycles at 3 MHz)

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // Enable interrupt for CCR0

    NVIC_EnableIRQ(TA0_0_IRQn);  // Enable Timer A0 interrupt in NVIC

    __enable_irq();  
}

