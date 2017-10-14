// Author: Matt Delengowski
// Lab: 4
// Section: Hardware PWM
// Device: F5529
#include <msp430.h> 

volatile unsigned int Duty_Cycle_Button_Count;
int debounce_state = 0;

int main(void)
{
    Duty_Cycle_Button_Count = 5; //Set initial duty cycle count

    WDTCTL = WDTPW | WDTHOLD;   // stop Watchdog timer

    P1DIR |= BIT2;             //Set GPIO P1.2 to output
    P4DIR |= BIT7;             //Set LED P4.7 to output

    P1OUT &= ~BIT0;            //turn LED P1.0 off
    P4OUT &= ~BIT7;            //turn LED P4.7 off

    P2DIR &= ~BIT1;            //Set Button P2.1 to input

    P2REN |= BIT1;             //Enable Pull Up Resistor for P2.1

    P2OUT |= BIT1;             //Active High

    P2IE |= BIT1;              //Enable Interrupts P2.1

    P2IES |= BIT1;             //P2.1 Interrupt Triggers on Positive Edge(When pressing down)

    P1IFG &= ~BIT1;            //Clear P1.1 Interrupt Flag in case of immediate interrupt
    P1IFG &= ~BIT2;            //Clear P1.2 Interrupt Flag in case of immediate interrupt

    P1SEL |= BIT2;         //direct timer to GPIO 1.2

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
        if(!(P2IN & BIT1))
        {
            P4OUT |= BIT7;    //When Button P1.1 is pressed, turn LED 9.7 on
        }
        else
            {P4OUT &= ~BIT7;} //when Button P1.1 is not pressed, turn LED 9.7 off
    }

}

#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
    switch(P2IV)
    {
    case P2IV_P2IFG1: //resets the IFG flag
        {
            switch(debounce_state)
                  {
                  case 0: //OFF -> GOING ON
                      TA1CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                      P2IE &= ~BIT1;                          // disable interrupts for P2.1 button
                      break;
                  case 1: //ON -> GOING OFF
                      TA1CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                      P2IE &= ~BIT1;                          // disable interrupts for P2.1 button
                      break;
                  }

        }
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
            TA0CCR1 = 100 * Duty_Cycle_Button_Count;
        }
        else if(Duty_Cycle_Button_Count == 10)
        {
            Duty_Cycle_Button_Count = 0;
            TA0CCR1 = 100 * Duty_Cycle_Button_Count;
        }
        P2IE |= BIT1;               //RE-ENABLE INTERRUPTS P2.1 Button
        P2IES &= ~BIT1;             //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
        TA1CTL &= ~TASSEL_2;        //Stop TimerA0
        TA1CTL |= TACLR;            //Clear TimerA0
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        P2IE |= BIT1;               //SET P2.1 INTERRUPT ENABLED (S2)
        P2IFG &= ~BIT1;             //P2.1 IFG CLEARED
        P2IES |= BIT1;              //Set P2.1 button interrupt to High to Low
        TA1CTL &= ~TASSEL_2;        //Stop TimerA0
        TA1CTL |= TACLR;            //Clear TimerA0
        debounce_state = 0;         //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}
