// Author: Matt Delengowski
// Lab: 4
// Section: Software Debounce
// Device: G2553

#include <msp430.h>

int debounce_state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // stop watchdog timer

    TA0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TA0CCR0 = 50000;        //overflow every 10ms

    P1DIR &= ~BIT3;         //SET P1.3 AS INPUT
    P1REN |= BIT3;          //ENABLED PULL UP OR DOWN FOR P1.3
    P1OUT |= BIT3;          //SPECIFIED AS A PULLUP FOR P1.3
    P1IE |= BIT3;           //Enable Button P1.3 Interrupt
    P1IFG &= ~BIT3;         //Clear FLAG for P1.3 Button Interrupt
    P1IES |= BIT3;          //Button P1.3 Interrupt on Pos Edge

    P1DIR |= BIT0;          //Set LED P1.0 as output
    P1OUT &= ~BIT0;         //Set LED P1.0 off initially
    __enable_interrupt();

}

#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
        switch(debounce_state)
            {
            case 0: //OFF -> GOING ON
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P1IE &= ~BIT3;                          // disable interrupts for P1.3 button
                P1IFG &= ~BIT3;
                break;
            case 1: //ON -> GOING OFF
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P1IE &= ~BIT3;                          // disable interrupts for P1.3 button
                P1IFG &= ~BIT3;
                break;
            }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    switch(debounce_state)
    {
    case 0://GOING ON -> ON
        P1OUT ^= BIT0;              //Switch state of LED P1.0
        P1IE |= BIT3;               //RE-ENABLE INTERRUPTS P1.3 Button
        P1IES &= ~BIT3;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P1IE |= BIT3;               //SET P1.3 INTERRUPT ENABLED (S2)
        P1IFG &= ~BIT3;             //P1.3 IFG CLEARED
        P1IES |= BIT3;              //Set P1.3 button interrupt to High to Low
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 0;                  //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
