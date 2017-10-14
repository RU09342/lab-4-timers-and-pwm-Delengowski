# Lab 4 Software Debounce
## Goals
To write code to account for button bouncing. Button bouncing a real world problem in electronics. When a button is pressed, two metal contacts are supposed to touch and complete a circuit, this allows a signal to be sent (or be cut off, depending on switch). In a perfect world the contacts would touch and stay there, however this isn't the case because the contacts are nothing but thin pieces of metal. They flex, and would be best classified as elastic. When the button is pressed, the contacts touch, but then flex upwards and backdown (hence bouncing). This causes the processor to register multiple button presses when the user only wanted one. 

The ramifications of this could be quite severe. Imagine being the operator of some kind of heavy machinary, or something where precision matters greatly. You want to press the button once to increment (move, adjust, etc.) a milimeter, button bounce occurs and instead of 1 milimeter you go 5 or 10. To counter act button boucning you can have hardware or software solutions. The former uses OP amps to create a low pass filter, blocking any high frequency signals from pass (the faster the contacts bounce the higher the signal frequency); this method is best for push button switches (Davies MSPP430 Ch7.3). Additionally, one could use a SR flip flop if they were to use a toggle switch instead (Davies MSP430 Ch7.3).

Davies's MSP430 also describes a software solution for button debounce. It utilizes an emulated shift register and has the following pseudocode

       rotate raw input into msb of shift register
       if (deounbced state == released)
       {
        if (shift register <= threshold for press)
        {
        debounced state = pressed
        set flag to indicated H->L: change
        }
       }
       Else
       {
        if (shift register >= threshold for release)
        {
          debounced state = released
          set flag to indicated L -> H change
        }
       }
So you have a shiftregister that is being shifted right each time a timer in up mode enters its ISR. If the button is pressed, what instead happens is the MSB gets set high, which adjusts the value of the shift register to be greater than the button threshold causes the debounced state to go high. It is when this debounced state is high that you would set the button output. Eventually the shift register gets shifted back down to zero and the button output is turned off. 

This method was implemented, using the code provided by Davies. I had to make changes to how the registers were manipulated but was successful in getting it to compile. 

      #include <msp430.h>
      #include <intrinsics.h>             // Intrinsic functions
      #include <stdint.h>                 // Standard integer types

      union DebP2IN{                             // Debounced state of P2IN
          unsigned char byte;         // Complete byte
          struct bit {
              unsigned char DebP2IN_0 : 1;
              unsigned char DebP2IN_1 : 1;
              unsigned char DebP2IN_2 : 1;
              unsigned char DebP2IN_3 : 1;
              unsigned char DebP2IN_4 : 1;
              unsigned char DebP2IN_5 : 1;
              unsigned char DebP2IN_6 : 1;
              unsigned char DebP2IN_7 : 1;
          } bit1;                  // Individual bits
      } DebP2IN1;                 //both DebP2INx, bitx, refer to buttons.

      #define BUTTON1_PRESSED() !(P1IN & BIT1)
      #define DEBB1()   DebP2IN1.bit1.DebP2IN_1
      #define LED1_ON()  P1OUT |= BIT0;  //P1.0 LED ON
      #define LED1_OFF() P1OUT &= ~BIT0; //P1.0 LED OFF

      void main (void)
      {
          WDTCTL = WDTPW | WDTHOLD;               // stop watchdog timer
          PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode

          P1OUT &= ~BIT0;                     //Pre-load LED P1.0 Off
          P1DIR |= BIT0;                      //Set P1.0 LED to output

          P1DIR &= ~BIT1;                    //set P1.1 button to input
          P1REN |= BIT1;                     //Enable Resistor Button P1.1
          P1OUT |= BIT1;                     //Set as Pullup resistor Button P1.1

          DebP2IN1.byte = 0xFF;                //initialize debounce state of P1.1
          TA0CCR0 = 5000;                     // TA0 interrupt triggers every 5 ms
          TA0CCTL0 |= CCIE;                   //Enable TA0 interrupt
          TA0CTL |= (TASSEL_2 + MC_1 + TACLR);    //SMCLK (1mhz) count to CCR0, clear timer

          __enable_interrupt();

          for(;;)
          {
              if(DEBB1() == 1) //Update LED1 from debounced button
              {
                  LED1_ON();
              }
              else if (DEBB1() == 0)
              {
                  LED1_OFF();
              }

          }
      }

      #define PRESS_THRESHOLD     0x3F
      #define RELEASE_THRESHOLD   0xFC

      #pragma vector = TIMER0_A0_VECTOR
      __interrupt void TA0_ISR (void)
      {
         static uint8_t P11ShiftReg = 0xFF; // Shift Reg for history of P1.1

      P11ShiftReg >>= 1;                      // Update history in shift register
      if (BUTTON1_PRESSED())                         //Insert latest input from P1.1
      {
          P11ShiftReg |= BIT7;                //Set MSB of Shiftreg P1.1 to 1
      }

      if (DEBB1() == 0)                         //when debounce value is low
      {
          if(P11ShiftReg >= RELEASE_THRESHOLD)    //when button is released
          {
              DEBB1() = 1;                          //set new debounce state high
          }
      }
      else if(P11ShiftReg <= PRESS_THRESHOLD)     //when debounce value is high and button is pressed
          {
              DEBB1() = 0;                          //set new debounce state low
          }
      }

Ultimately this method was discarded, when I attempted to implement this debounce method into my PWM code, it failed miserably. Instead a C coded state machine was implemented (which looks eerily close to a state machine that would be implemented in verilog). This was idea was found on a TI forum. There are four states to account for state 1:Off-> Going On, State 2:Going On -> On, State 3:On -> Going Off, State 4:Going Off -> Off (in this order). The transitions of this state machine are the button press interrupts and timer up mode interrupts.  

The button is pressed, enter state 1 and start timerA0, kill interrupts for button (so you dont back to state 1). When TA0 interrupt fires enter state2, kill the timer, enable button interrupts but flip the edge select, set debounce state to 1, send button output. When the button is released enter state 3, start the timer and kill button interrupts again. When TA0 interrupt occurs enter state four, enable button interrupts again and kill the timer (this state acts as a restart of sorts), set debounce state to zero. A code snippet of this is shown below. 

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
                    TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                    P1IE &= ~BIT1;                          // disable interrupts for P1.1 button
                    break;
                case 1: //ON -> GOING OFF
                    TA0CTL = TASSEL_2 + MC_1 + TACLR;       // SMCLK (1mhz) in continous mode
                    P1IE &= ~BIT1;                          // disable interrupts for P1.1 button
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
            P1OUT ^= BIT0;              //Switch state of LED P1.1
            P1IE |= BIT1;               //RE-ENABLE INTERRUPTS P1.1 Button
            P1IES &= ~BIT1;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (button release)
            TA0CTL &= ~TASSEL_2;        //Stop TimerA0
            TA0CTL |= TACLR;            //Clear TimerA0
            debounce_state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
            P1IE |= BIT1;               //SET P1.1 INTERRUPT ENABLED (S2)
            P1IFG &= ~BIT1;             //P1.1 IFG CLEARED
            P1IES |= BIT1;              //Set P1.1 button interrupt to High to Low
            TA0CTL &= ~TASSEL_2;        //Stop TimerA0
            TA0CTL |= TACLR;            //Clear TimerA0
            debounce_state = 0;                  //UPON ANOTHER BUTTON PRESS, GOTO CASE 0 OF PORT 1 ISR
            break;
        }

    }


# Software Debouncing
In previously labs, we talked about how objects such as switches can cause some nasty effects since they are actually a mechanical system at heart. We talked about the simple hardware method of debouncing, but due to the many different design constraints, you may not be able to add or adjust hardware. Debouncing is also only one of many applications which would require the use of built in Timers to allow for other processes to take place.

## Task
You need to utilize the TIMER modules within the MSP430 processors to implement a debounced switch to control the state of an LED. You most likely will want to hook up your buttons on the development boards to an oscilloscope to see how much time it takes for the buttons to settle. The idea here is that your processor should be able to run other code, while relying on timers and interrupts to manage the debouncing in the background. You should not be using polling techniques for this assignment. Your code should also be able to detect 

### Hints
You need to take a look at how the P1IE and P1IES registers work and how to control them within an interrupt routine. Remember that the debouncing is not going to be the main process you are going to run by the end of the lab.

## Extra Work
### Low Power Modes
Go into the datasheets or look online for information about the low power modes of your processors and using Energy Trace, see what the lowest power consumption you can achieve while still running your debouncing code. Take a note when your processor is not driving the LED (or unplug the header connecting the LED and check) but running the interrupt routine for your debouncing.

### Double the fun
Can you expand your code to debounce two switches? Do you have to use two Timer peripherals to do this?
