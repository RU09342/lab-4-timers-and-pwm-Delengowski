// Author: Matt Delengowski
// Lab: 4
// Section: Hardware PWM
// Device: G2553
#include <msp430.h> 

volatile unsigned int Duty_Cycle_Button_Count;
int debounce_state = 0;

int main(void)
{
    Duty_Cycle_Button_Count = 5; //Set initial duty cycle count

    WDTCTL = WDTPW | WDTHOLD;   // stop Watchdog timer

    P1DIR |= BIT0;             //Set LED P1.0 to output
    P1DIR |= BIT6;             //Set LED P1.6 to output

    P1OUT &= ~BIT0;            //turn LED P1.0 off
    P1OUT &= ~BIT6;            //turn LED P1.6 off

    P1DIR &= ~BIT3;                 //SET P1.3 AS INPUT
     P1REN |= BIT3;                  //ENABLED PULL UP OR DOWN FOR P1.3
     P1OUT |= BIT3;                  //SPECIFIED AS A PULLUP FOR P1.3
     P1IE |= BIT3;                   //SET P1.3 INTERRUPT ENABLED (S2)
     P1IFG &= ~BIT3;                 //P1.3 IFG CLEARED

    P1SEL |= BIT0;         //Choose Pin 1.0

    TA0CCR0 = 1000;                    //Set Up Mode Limit
    TA0CCR1 = 500;                   //Sets Initial Duty Cycle to 50% (Duty = Time On/Peroid = CCR1/CCR0)
    TA0CCTL1 |= OUTMOD_7;
    TA0CTL |= TASSEL_2 + MC_1 + TACLR; //Use SMCLK (1 Mhz), enable Up Mode, Clear Timer Registers

    TA1CCTL0 = CCIE;        // CCR0 interrupt enabled -- Used for Debounce
    TA1CCR0 = 50000;        //overflow every 10ms -- Used for Debounce

    /*Notes
     Frequency of Pwm = Clock Freq/CCR0
     A 1Khz Freq_pwm is required, and Freq_clk must be held constant, as must CCR0 or else Freq_pwm will not be
     therefore to alter the duty cycle we must alter CCR1.
     Lab wants:
         start: 50%
         increment to 100% by 10%
         Drop to 0%
         Repeat
         ****See Excel File (Pwm calculations.xlsx) for calculations)****
     */

    __bis_SR_register(GIE);  //Enter LPM0 w/ interrupt

    for(;;)
    {
        if(!(P1IN & BIT3))
        {
            P1OUT |= BIT6;    //When Button P1.6 is pressed, turn LED 9.7 on
        }
        else
            {P1OUT &= ~BIT6;} //when Button P1.6 is not pressed, turn LED 9.7 off
    }

}

#pragma vector = PORT1_VECTOR
__interrupt void P1_ISR(void)
{
            switch(debounce_state)
                  {
                  case 0: //OFF -> GOING ON
                      TA1CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                      P1IE &= ~BIT3;                          // disable interrupts for P1.3 button
                      P1IFG &= ~BIT3;                         //Clear IF for Port 1
                      break;
                  case 1: //ON -> GOING OFF
                      TA1CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                      P1IE &= ~BIT3;                          // disable interrupts for P1.3 button
                      P1IFG &= ~BIT3;                         //Clear IF for Port 1
                      break;
                  }
}

#pragma vector=TIMER1_A0_VECTOR //flag resets automatically
__interrupt void Timer_A0 (void)
{
    switch(debounce_state)
    {
    case 0://GOING ON -> ON
        if(Duty_Cycle_Button_Count < 10) //Conditions for Duty Cycle Incrementing
        {
            Duty_Cycle_Button_Count++;
            TA0CCR1 += 100;
        }
        else if(Duty_Cycle_Button_Count == 10)
        {
            Duty_Cycle_Button_Count = 0;
            TA0CCR1 = 0;
        }
        P1IE |= BIT3;               //RE-ENABLE INTERRUPTS P1.3 Button
        P1IES &= ~BIT3;             //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TA1CTL &= ~TASSEL_2;        //Stop TimerA0
        TA1CTL |= TACLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P1IE |= BIT3;              //SET P1.1 INTERRUPT ENABLED (S2)
        P1IFG &= ~BIT3;            //P1.3 IFG CLEARED
        P1IES |= BIT3;             //Set P1.3 button interrupt to High to Low
        TA1CTL &= ~TASSEL_2;       //Stop TimerA0
        TA1CTL |= TACLR;           //Clear TimerA0
        debounce_state = 0;        //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
