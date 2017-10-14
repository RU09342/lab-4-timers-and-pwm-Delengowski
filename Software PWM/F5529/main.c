// Author: Matt Delengowski
// Lab: 4
// Section: software PWM
// Device: F5529
#include <msp430.h> 

int Duty_Cycle = 500;
int debounce_state = 0;

void LEDSetup();
void ButtonSetup();
void TimerA0Setup();
void TimerA1Setup();

#define BUTTON1PRESSED !(P2IN & BIT1)
#define LED1_ON        P1OUT |= BIT0;                       //LED P1.0 ON
#define LED1_OFF       P1OUT &= ~BIT0;                      //LED P1.0 OFF
#define LED2_ON        P4OUT |= BIT7;                       //LED 4.7 ON
#define LED2_OFF       P4OUT &= ~BIT7;                      //LED 4.7 OFF
#define TimerA1_ON     TA1CTL = TASSEL_2 + MC_1 + TACLR;    // SMCLK (1mhz) in continous mode, clear timer
#define TimerA1_OFF    TA1CTL &= ~TASSEL_2 + TACLR;         //Stop TimerA1, clear timer
#define Dis_Int_But1   P2IE &= ~BIT1;                       // disable interrupts for P2.1 button
#define En_Int_But1    P2IE |= BIT1;                        //Enable Interrupts for P2.1 Button
#define ES_Neg_But1    P2IES &= ~BIT1;                      //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
#define ES_Pos_But1    P2IES |= BIT1;                       //TOGGLE INTERRUPT EDGE: HIGH TO LOW (BUTTON PRESS)


int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop Watchdog timer

    LEDSetup();
    ButtonSetup();
    TimerA0Setup();
    TimerA1Setup();


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

   __enable_interrupt();

    for(;;)
    {

        if((TA0R <= Duty_Cycle) && (Duty_Cycle != 0))
        {
            LED1_ON;
        }
        else if(TA0R > Duty_Cycle)
        {
            LED1_OFF;
        }

        if(BUTTON1PRESSED)
        {
            LED2_ON;
        }
        else {LED2_OFF}

    }
}
#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void)
{
    switch(P2IV)
    {
    case P2IV_P2IFG1:
        {
            switch(debounce_state)
                {
                case 0: //OFF -> GOING ON
                    TimerA1_ON;
                    Dis_Int_But1;
                    break;
                case 1: //ON -> GOING OFF
                    TimerA1_ON;
                    Dis_Int_But1;
                    break;
                }
        }
    }
}
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_ISR(void)
{
    switch(debounce_state)
    {
    case 0://GOING ON -> ON
        if(Duty_Cycle < 1000) //Conditions for Incrementing PWM
        {
            Duty_Cycle += 100;
        }
        else
        {
            Duty_Cycle = 0;
        }
        En_Int_But1;
        ES_Neg_But1;
        TimerA1_OFF;
        debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
        break;
    case 1://GOING OFF -> OFF
        En_Int_But1;
        P2IFG &= ~BIT1;             //Clear Flag P1.1 button interrupt
        ES_Pos_But1;
        TimerA1_OFF;
        debounce_state = 0;         //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
        break;
    }

}

void LEDSetup()
{
    P1DIR |= BIT0;             //Set LED P1.0 to output
    P4DIR |= BIT7;             //Set LED P4.7 to output

    P1OUT &= ~BIT0;            //turn LED P1.0 off
    P4OUT &= ~BIT7;            //turn LED P4.7 off
}

void ButtonSetup()
{
    P2DIR &= ~BIT1;            //Set Button P2.1 to input

    P2REN |= BIT1;             //Enable Pull Up Resistor for P2.1 Button

    P2OUT |= BIT1;             //Active High Button P2.1

    P2IE |= BIT1;              //Enable Interrupts P2.1 Button

    P2IES |= BIT1;             //P2.1 Interrupt Triggers on Positive Edge(When pressing down)

    P2IFG &= ~BIT1;            //Clear P2.1 Interrupt Flag in case of immediate interrupt
}

void TimerA0Setup()
{
    TA0CCR0 = 1000;                    //Set Up Mode Limit --Used for PWM
    TA0CTL |= TASSEL_2 + MC_1 + TACLR; //Use SMCLK (1 Mhz), enable Up Mode, Clear Timer Registers -- Used for PWM
}

void TimerA1Setup()
{
    TA1CCTL0 = CCIE;        // CCR0 interrupt enabled -- Used for Debounce
    TA1CCR0 = 50000;        //overflow every 10ms -- Used for Debounce
}
