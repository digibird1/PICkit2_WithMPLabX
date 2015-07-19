/*
 * File:   main.c
 * Author: digibird1
 * (c) 2015
 * https://digibird1.wordpress.com/
 http://www.microchip.com/forums/m827267.aspx#828385
 
 It seems for the output registers it is good to have a shadow 
 * register to avoid read-modify-write-effect
 
 */


#define _XTAL_FREQ 8000000

#include <xc.h>
#include <pic16f690.h>
#include "UART.h"
#include "myPicTools.h"


// BEGIN CONFIG
// CONFIG
#pragma config FOSC = INTRCIO         // Oscillator Selection bits (EC: I/O function on RA4/OSC2/CLKOUT pin, CLKIN on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
//END CONFIG

void interrupt ISR(void)
{
    //Turn ON/OFF RC3 in order to indicate 
    //that an interrupt occured
    //useful for debugging
    if(checkBit(ShadowPortC,3)>0){
        SetBitReg(Reg_PORTC,3,LOW);
    }
    else{
        SetBitReg(Reg_PORTC,3,HIGH);
    }
    
    //UART interrupt
    if(RCIF && RCIE){
        UART_Interrupt();
        
        return;
    }
}

//Read ADC
void Get_Inputs(void)
{
    __delay_ms(1);
    //Start conversion by setting the GO/DONE bit.
    ADCON0bits.GO_nDONE = 1;
    //Wait for ADC conversion to complete
    //Polling the GO/DONE bit
    // 0 = ADC completed
    // 1 = ADC in progress
    while(ADCON0bits.GO_nDONE == 1);
}


int main()
{
    
  initShadowRegisters();  
    
  TRISC = 0; //RC0 as Output PIN
  //TRISC1 = 0;
  
  
  
  //Clear Analog Inputs and make pins digital pins
  ANSEL=0b00000001;//set RA0 as analog pin
  ANSELH=0;
  
  IRCF0 = 0b111;//set prescaler
  
  UART_Init();
  
  UART_writeString("Startup ... done\n");
  
  
  
  //ADC
    //Select ADC conversion clock Frc
    ADCON1bits.ADCS = 0b111;
     //Configure voltage reference using VDD
    ADCON0bits.VCFG = 0;
    //Select ADC input channel (RA0/AN0)
    ADCON0bits.CHS = 0;
    //Select result format right justified
    //right=1 is good when using all 10 bits the two bytes can be concatenated 
    //easily into an integer
    //left=0 is good when using the 8 most significant bits
    ADCON0bits.ADFM = 1;
    //Turn on ADC module
    ADCON0bits.ADON = 1;
  

        
  
  while(1)
  {
//---------------------------------------------    
    //Blink LED on RC0 and RC1
    SetBitReg(Reg_PORTC,0,HIGH);
    SetBitReg(Reg_PORTC,1,LOW);

    __delay_ms(100); // 100ms Delay

    SetBitReg(Reg_PORTC,0,LOW);
    SetBitReg(Reg_PORTC,1,HIGH);

    __delay_ms(100); // 100ms Delay
//---------------------------------------------        

    
//--------------------------------------------- 
    //UART communication
    while(UART_DataAvailable()>0){
        //char a=UART_ReadByte();
        //UART_writeByte(a);
        //UART_writeByte('\n');
        
        UART_writeNumber(UART_DataAvailable());
        UART_writeByte('\n');  
        
        UART_writeString(UART_ReadString());
        UART_writeByte('\n');
    }
    
    if(UART_DataAvailable()<0){
        UART_writeString("Data Lost Reset UART\n");
        UART_Reset();
    }
    
    //Print a Register
 // UART_writeBitPattern(ANSELH);
 // UART_writeByte('\n');
//---------------------------------------------        

    
//---------------------------------------------    
        
    //Read ADC
    Get_Inputs();
    unsigned int ADC_Value=0;
    
    ADC_Value=ADRESH<<8 | ADRESL;
 
    UART_writeNumber(ADC_Value);
    UART_writeByte('\n');
    
    if(checkBit(ShadowPortC,1)>0)
       SetBitReg(Reg_PORTC,2,HIGH); 
    

 //---------------------------------------------    
  }
  return 0;
  
 
}
