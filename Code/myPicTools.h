/* 
 * File:   myPicTools.h
 * Author: digibird1
 * (c) 2015
 * https://digibird1.wordpress.com/
 *
 * Created on July 18, 2015, 11:12 AM
 */

#ifndef MYPICTOOLS_H
#define	MYPICTOOLS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define HIGH 1
#define LOW 0
    
    
#include <pic16f690.h>
#include <xc.h>  
#include <stdlib.h>
    
volatile unsigned char ShadowPortA; 
volatile unsigned char ShadowPortB;
volatile unsigned char ShadowPortC;

/*
 This is to avoid read-modify-write-effect
 * http://www.microchip.com/forums/m478014.aspx
 */

//enum for the I/O registers which get shadow registers
typedef enum {Reg_PORTA, Reg_PORTB, Reg_PORTC} RegisterType;


void initShadowRegisters(){
    ShadowPortA=PORTA;
    ShadowPortB=PORTB;
    ShadowPortC=PORTC;
}


    
//Use the shadow registers to set or clear a bit
//useful to avoid the read-modify-write-effect
void SetBitReg(RegisterType Register, unsigned Bit, unsigned Value){
    
    
   volatile unsigned char* ShadowReg;
   volatile unsigned char* HardWareReg;
    //Map the Hardware Port and Shadow ports to a pointer
    //for universal code
    switch(Register){
        case Reg_PORTA :
            ShadowReg=&ShadowPortA;
            HardWareReg=&PORTA;
            break;
        case Reg_PORTB  :
            ShadowReg=&ShadowPortB;
            HardWareReg=&PORTB;
            break;
        case Reg_PORTC  :
            ShadowReg=&ShadowPortC;
            HardWareReg=&PORTC;
            break;
            
        default : 
            ShadowReg=NULL;
            HardWareReg=NULL;
    }
    
    if(Value==HIGH){//Set bit
        *ShadowReg |= 1 << Bit;
    }
    if(Value==LOW){//clear bit
        *ShadowReg &= ~(1 << Bit);
    }
    //Write the shadow register to the hardware port
    *HardWareReg=*ShadowReg;
}

//Help function to help to check if an individual bit is set or not
unsigned int checkBit(volatile unsigned char Register, unsigned int Bit){
    return (Register >> Bit) & 1;
}


#ifdef	__cplusplus
}
#endif

#endif	/* MYPICTOOLS_H */

