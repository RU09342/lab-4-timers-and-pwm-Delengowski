// Author: Matt Delengowski
// Lab: 4
// Section: Software Debounce
// Device: FR5994

#include <msp430.h>

int debounce_state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                 // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
                                              // to activate previously configured port settings

    TA0CCTL0 = CCIE;        // CCR0 interrupt enabled
    TA0CCR0 = 50000;        //overflow every 10ms

    P5DIR &= ~BIT6;         //SET P5.6 AS INPUT
    P5REN |= BIT6;          //ENABLED PULL UP OR DOWN FOR P5.6
    P5OUT |= BIT6;          //SPECIFIED AS A PULLUP FOR P5.6
    P5IE |= BIT6;           //Enable Button P5.6 Interrupt
    P5IFG &= ~BIT6;         //Clear FLAG for P5.6 Button Interrupt
    P5IES |= BIT6;          //Button P5.6 Interrupt on Pos Edge

    P1DIR |= BIT0;          //Set LED P1.0 as output
    P1OUT &= ~BIT0;         //Set LED P1.0 off initially
    __enable_interrupt();

}

#pragma vector=PORT5_VECTOR
__interrupt void PORT_5(void)
{
    switch(P5IV)
    {
    case P5IV_P5IFG6:
        {
        switch(debounce_state)
            {
            case 0: //OFF -> GOING ON
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P5IE &= ~BIT6;                          // disable interrupts for P5.6 button
                break;
            case 1: //ON -> GOING OFF
                TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                P5IE &= ~BIT6;                          // disable interrupts for P5.6 button
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
        P1OUT ^= BIT0;              //Switch state of LED P5.6
        P5IE |= BIT6;               //RE-ENABLE INTERRUPTS P5.6 Button
        P5IES &= ~BIT6;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P5IE |= BIT6;               //SET P5.6 INTERRUPT ENABLED (S2)
        P5IFG &= ~BIT6;             //P5.6 IFG CLEARED
        P5IES |= BIT6;              //Set P5.6 button interrupt to High to Low
        TA0CTL &= ~TASSEL_2;        //Stop TimerA0
        TA0CTL |= TACLR;            //Clear TimerA0
        debounce_state = 0;                  //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
