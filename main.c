#include "msp430g2553.h"
//#include "io430.h"
//hej
unsigned int adc[] = {0, 0, 0};
//unsigned int SensorData[] = {0, 0, 0};
unsigned int tempC;
//int voltage;
unsigned char *PTxData;
//int TXByteCtr;
char TxData[] = {0, 0, 0};
//char TxData[] = {0, 0, 0};
int v[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
float Cfactor = 0.3515625;      // 360/1024


//******************************************************************************
// FUNCTIONS
//******************************************************************************

void init_i2c()
{
  P1SEL |= BIT6 + BIT7;                   // Assign I2C pins to USCI_B0, 1.6 = SCL, 1.7 = SDL
  P1SEL2|= BIT6 + BIT7;                   // Assign I2C pins to USCI_B0
  UCB0CTL1 |= UCSWRST;                    // Enable SW reset
  UCB0CTL0 = UCMODE_3 + UCSYNC;           // I2C Slave, synchronous mode
  UCB0I2COA = 0x53;                       // Own Address is 053h
  UCB0I2CSA = 0x53;                       // Own Address is 053h
  UCB0CTL1 &= ~UCSWRST;                   // Clear SW reset, resume operation
  UCB0I2CIE |= UCSTPIE + UCSTTIE;         // Enable STT and STP interrupt
  IE2 |= UCB0TXIE;                        // Enable TX interrupt
}

// Configure the ADC
void configureAdc()
{
  //ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE + REFON;
  ADC10CTL0 = MSC + ADC10SHT_2 + ADC10ON + ADC10IE;
  //ADC10CTL1 = INCH_3 + ADC10DIV_3;
  ADC10CTL1 = CONSEQ_1 + INCH_2;// + ADC10DIV_3;
  ADC10DTC1 = 0x03;
  ADC10AE0 |= 0x07;
  //ADC10AE0 |= 0x08;                       // P1_3
  //ADC10AE0 |= 0x10;                       // P1_4
  //ADC10AE0 |= 0x20;                       // P1_5
  //ADC10AE0 |= 0x38;                       // P1_3, P1_4, P1_5
  BCSCTL1 = CALBC1_1MHZ;                  // Clock calibration
  DCOCTL = CALDCO_1MHZ;                   // Clock calibration
  //BCSCTL2 &= ~(DIVS_3);
  P1DIR = 0x10;
  P1OUT = 0x10;
  //P1SEL |= 0x08;                        // Select P1_3
  //P1SEL |= 0x10;                        // Select P1_4
  //P1SEL |= 0x20;                        // Select P1_5
  //P1SEL |= 0x02;                        // Select P1_2

}

#define NUMBER_OF_SAMPLES 10

// Fill the TxData buffer
//char fillBuffer()
void fillBuffer()
{
  
//  int i = 0;
//  while (i <= 10)
//  {
    //__delay_cycles(100000);                   // delay 0.1 sec 
    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);
    ADC10SA = (unsigned int)adc;
    ADC10CTL0 |= ENC + ADC10SC;                 // Enable conversion
    //__bis_SR_register(CPUOFF + GIE);          // Low power mode with enabled interrupts
    //SensorValue = ADC10MEM;                     // ADC conversion
   
    for ( int i = 0; i < 3; i++ )
    {
    TxData[i] = (int)(adc[i] * Cfactor) - 50;
//    Sensor2 = (int)(adc[1] * Cfactor) - 50;
    }
    
    PTxData = (unsigned char *)TxData;
    
  //Sensor1= SensorValue[0];
  //Sensor2= SensorValue[1];
  __bis_SR_register(CPUOFF + GIE);
}

//ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  __bic_SR_register_on_exit(CPUOFF);      // Return to active mode
}


int main(void) 
{
  WDTCTL = WDTPW + WDTHOLD;               // Stop watchdog timer

  init_i2c();
  configureAdc();
  fillBuffer();
  P1OUT = ~(0x10);
  
  while(1)
  { 
    
    fillBuffer();
    __delay_cycles(5000000); 
    P1OUT = (0x10);
  }
}
//------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move data from MSP430 memory to the
// I2C master. PTxData points to the next byte to be transmitted, and TXByteCtr
// keeps track of the number of bytes transmitted.
//------------------------------------------------------------------------------

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  UCB0TXBUF = *PTxData++;
  //__bic_SR_register_on_exit(CPUOFF);      // Exit LPM0 if data was
  //UCB0TXBUF = *PTxData++;         // Transmit data at address PTxData
  //TXByteCtr++;                    // Increment TX byte counter
  
}

//------------------------------------------------------------------------------
// The USCI_B0 state ISR is used to wake up the CPU from LPM0 in order to do
// processing in the main program after data has been transmitted. LPM0 is
// only exit in case of a (re-)start or stop condition when actual data
// was transmitted.
//------------------------------------------------------------------------------

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  //if (UCSTTIFG)
  // status pin = 0
  P1OUT = ~(0x10);
  UCB0STAT &= ~(UCSTTIFG + UCSTTIFG);     // Clear interrupt flags
  //UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);     // Clear interrupt flags
  //if (TXByteCtr)                          // Check TX byte counter
  __bic_SR_register_on_exit(CPUOFF);      // Exit LPM0 if data was
} 