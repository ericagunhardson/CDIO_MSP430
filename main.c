#include "msp430g2553.h"
//#include "io430.h"

int SensorValue = 0;
int tempC;
int voltage;
int result;
float Cfactor = 0.3515625;      // 360/1024
//float Cfactor = 0.322265625;      // 330/1024
int v[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


//The function to configure the ADC
void configureAdc()
{
  //ADC10CTL0 = SREF_1 + ADC10SHT_0 + ADC10ON + ADC10IE + REFON;
  ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE + REFON;
  ADC10CTL1 = INCH_3 + ADC10DIV_3;
  ADC10AE0 |= 0x08;             // P1_0
}

// ADC10 interrupt service routine
//#pragma vector=ADC10_VECTOR

//__interrupt void ADC10_ISR (void)
//{
//  __bic_SR_register_on_exit(CPUOFF);      // Return to active mode
//}

int main(void) 
{
  WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer
  BCSCTL1 = CALBC1_1MHZ;                  //Clock calibration
  DCOCTL = CALDCO_1MHZ;
  BCSCTL2 &= ~(DIVS_3);
  P1SEL |= 0x08;                           // P 1_0
  P1DIR |= 0x01;                          // Make P1_0 output

  configureAdc();
  //__enable_interrupt();

    ADC10CTL0 |= ENC + ADC10SC;             //Enable conversion
    SensorValue = ADC10MEM;                 //ADC conversion
  
  while(1)
  {
    int i = 0;
    while(i <= 10)
    {
      __delay_cycles(100000);                //delay x sec
      P1OUT = ~(0x01);                         // Set P1_0 low
      ADC10CTL0 |= ENC + ADC10SC;             //Enable conversion
      //__bis_SR_register(CPUOFF + GIE);        //Low power mode with enabled interrupts
      SensorValue = ADC10MEM;                 //ADC conversion
      //voltage  = (SensorValue * 3600)/1024;
      tempC = (int)(SensorValue * Cfactor) - 50;
      v[i] = tempC; 
      i++; 
    }
    result = (v[0]+v[1]+v[2]+v[3]+v[4]+v[5]+v[6]+v[7]+v[8]+v[9]) / 10;
    P1OUT = 0x01;                          // Set P1_0 high
    
    while(1)
    {
      
    }
    
  }
}