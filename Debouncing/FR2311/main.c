// Author: Matt Delengowski
// Lab: 4
// Section: Software Debounce
// Device: FR2311

#include <msp430.h>

int debounce_state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                              // to activate previously configured port settings

    TB0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TB0CCR0 = 50000;        //overflow every 10ms

    P1DIR &= ~BIT1;         //SET P1.1 AS INPUT
    P1REN |= BIT1;          //ENABLED PULL UP OR DOWN FOR P1.1
    P1OUT |= BIT1;          //SPECIFIED AS A PULLUP FOR P1.1
    P1IE |= BIT1;           //Enable Button P1.1 Interrupt
    P1IFG &= ~BIT1;         //Clear FLAG for P1.1 Button Interrupt
    P1IES |= BIT1;          //Button P1.1 Interrupt on Pos Edge

    P1DIR |= BIT0;          //Set LED P1.0 as output
    P1OUT &= ~BIT0;         //Set LED P1.0 off initially
    __enable_interrupt();

}

#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
    switch(P1IV)
    {
    case P1IV_P1IFG1:
        {
        switch(debounce_state)
            {
            case 0: //OFF -> GOING ON
                TB0CTL = TBSSEL_2 + MC_1 + TBCLR;       // SMCLK (1mhz) in continous mode
                P1IE &= ~BIT1;                          // disable interrupts for P1.1 button
                break;
            case 1: //ON -> GOING OFF
                TB0CTL = TBSSEL_2 + MC_1 + TBCLR;       // SMCLK (1mhz) in continous mode
                P1IE &= ~BIT1;                          // disable interrupts for P1.1 button
                break;
            }
        break;
        }
    }
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void Timer_B0 (void)
{
    switch(debounce_state)
    {
    case 0://GOING ON -> ON
        P1OUT ^= BIT0;              //Switch state of LED P1.1
        P1IE |= BIT1;               //RE-ENABLE INTERRUPTS P1.1 Button
        P1IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TB0CTL &= ~TBSSEL_2;        //Stop TimerA0
        TB0CTL |= TBCLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P1IE |= BIT1;               //SET P1.1 INTERRUPT ENABLED (S2)
        P1IFG &= ~BIT1;             //P1.1 IFG CLEARED
        P1IES |= BIT1;              //Set P1.1 button interrupt to High to Low
        TB0CTL &= ~TBSSEL_2;        //Stop TimerA0
        TB0CTL |= TBCLR;            //Clear TimerA0
        debounce_state = 0;                  //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
