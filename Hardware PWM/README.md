# Lab 4 Hardware PWM
## Goals
To implement Pulse Width Modulation of an LED using port select registers to direct Timer output to the LED
## Code
Pulse Width Module (pwm) is controlled by manipulating the duty cycle of the signal. Where duty cycle is defined 

    duty cycle = time on/time off
  
Two capture compare registers were used to perform this, CCR0 and CCR1. By directing the output of TimerA0.1 (which is compared against CCR1) using PxSEL0 and PxSEL1 (on the FR6989 for example), when TimerA0 is counting up towards CCR1, the LED is on. When TA0 surpasses CCR1 the LED goes off, until it hits CCR0 and goes until it hits CCR1 again. The final requirement of PWM is that the Output mode be set to 7 (set/reset on most devices). 

To increment the PWM is to adjust the value of CCR1. Every time the button is pressed it is incremented by 10% until it reaches the value of CCR0. Button debouncing was implemented to guauratee the value only increasing by 10% each button press.  The visual signs of adjusting the duty cycle is that the higher the duty cycle, the brighter the LED. 

Note: The LED pins on the F5529 did not have the ability to multiplex TA to that pin. So, a work around was made by multiplexing TA to a GPIO and then connecting that GPIO pin to the LED pin using a jump wire. A gif of this was taken to show as proof. 

# Hardware PWM
Now that you have done the software version of PWM, now it is time to start leveraging the other features of these Timer Modules.

## Task
You need to replicate the same behavior as in the software PWM, only using the Timer Modules ability to directly output to a GPIO Pin instead of managing them in software. 

### Hints 
Read up on the P1SEL registers as well as look at the Timer modules ability to multiplex.

## Extra Work
### Using ACLK
Some of these microprocessors have a built in ACLK which is extremely slow compared to your up to 25MHz available on some of them. What is the overall impact on the system when using this clock? Can you actually use your PWM code with a clock that slow?

### Ultra Low Power
Using a combination of ACLK, Low Power Modes, and any other means you may deem necessary, optimize this PWM code to run at 50% duty cycle with a LED on the MSP430FR5994. In particular, time how long your code can run on the fully charged super capacitor. You do not need to worry about the button control in this case, and you will probably want to disable all the GPIO that you are not using (nudge, nudge, hint, hint).
