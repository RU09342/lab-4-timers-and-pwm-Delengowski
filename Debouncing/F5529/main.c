// Author: Matt Delengowski
// Lab: 4
// Section: Software Debounce
// Device: F5529

#include <msp430.h>

int debounce_state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // stop watchdog timer

    TA0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TA0CCR0 = 50000;        //overflow every 10ms

    P2DIR &= ~BIT1;         //SET P2.1 AS INPUT
    P2REN |= BIT1;          //ENABLED PULL UP OR DOWN FOR P2.1
    P2OUT |= BIT1;          //SPECIFIED AS A PULLUP FOR P2.1
    P2IE |= BIT1;           //Enable Button P2.1 Interrupt
    P2IFG &= ~BIT1;         //Clear FLAG for P2.1 Button Interrupt
    P2IES |= BIT1;          //Button P2.1 Interrupt on Pos Edge

    P1DIR |= BIT0;          //Set LED P1.0 as output
    P1OUT &= ~BIT0;         //Set LED P1.0 off initially
    __enable_interrupt();

}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    switch(P2IV)
    {
    case P2IV_P2IFG1:
        {
        switch(debounce_state)
            {
            case 0: //OFF -> GOING ON
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P2IE &= ~BIT1;                          // disable interrupts for P2.1 button
                break;
            case 1: //ON -> GOING OFF
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P2IE &= ~BIT1;                          // disable interrupts for P2.1 button
                break;
            }
        break;
        }
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
    switch(debounce_state)
    {
    case 0://GOING ON -> ON
        P1OUT ^= BIT0;              //Switch state of LED P1.0
        P2IE |= BIT1;               //RE-ENABLE INTERRUPTS P2.1 Button
        P2IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P2IE |= BIT1;               //SET P2.1 INTERRUPT ENABLED (S2)
        P2IFG &= ~BIT1;             //P2.1 IFG CLEARED
        P2IES |= BIT1;              //Set P2.1 button interrupt to High to Low
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 0;                  //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
