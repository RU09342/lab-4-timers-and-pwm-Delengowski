# Lab 4 Software PWM
## Goal
To adjust the duty cycle of an LED through use of software techniques (cannot use pin selects to multiplex timers to output). 
## Code
The most one needs to know for PWM is the equation for duty cycle, 

	dutycycle = time on/time off

This lab utilized polling and direct access to the timer register TAxR, to represent the time on. We then use a variale Duty _ Cycle to represent the time off. When TAxR is less than the duty cycle value, and Duty _ Cycle does not equal zero, turn the LED on. When TA0R is greater than the Duty _ Cycle, turn the LED off. The duty cycle can then be adjusted using a button, which increases it by 10% every time button is pressed (button debouncing was used to guarantee this), a second LED will turn on when the button is pressed to show the duty cycle being incremented. The max value of the Timer (CCR0) is 1000 and that is the max duty cycle. An example code snippet from FR6989 is shown below.  

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




# Software PWM
Most microprocessors will have a Timer module, but depending on the device, some may not come with pre-built PWM modules. Instead, you may have to utilize software techniques to synthesize PWM on your own.

## Task
You need to generate a 1kHz PWM signal with a duty cycle between 0% and 100%. Upon the processor starting up, you should PWM one of the on-board LEDs at a 50% duty cycle. Upon pressing one of the on-board buttons, the duty cycle of the LED should increase by 10%. Once you have reached 100%, your duty cycle should go back to 0% on the next button press. You also need to implement the other LED to light up when the Duty Cycle button is depressed and turns back off when it is let go. This is to help you figure out if the button has triggered multiple interrupts.

### Hints
You really, really, really, really need to hook up the output of your LED pin to an oscilloscope to make sure that the duty cycle is accurate. Also, since you are going to be doing a lot of initialization, it would be helpful for all persons involved if you created your main function like:
'''c
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	LEDSetup(); // Initialize our LEDS
	ButtonSetup();  // Initialize our button
	TimerA0Setup(); // Initialize Timer0
	TimerA1Setup(); // Initialize Timer1
	__bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}
'''
This way, each of the steps in initialization can be isolated for easier understanding and debugging.


## Extra Work
### Linear Brightness
Much like every other things with humans, not everything we interact with we perceive as linear. For senses such as sight or hearing, certain features such as volume or brightness have a logarithmic relationship with our senses. Instead of just incrementing by 10%, try making the brightness appear to change linearly. 

### Power Comparison
Since you are effectively turning the LED off for some period of time, it should follow that the amount of power you are using over time should be less. Using Energy Trace, compare the power consumption of the different duty cycles. What happens if you use the pre-divider in the timer module for the PWM (does it consume less power)?
