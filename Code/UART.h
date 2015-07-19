/* 
 * File:   UART.h
 * Author: digibird1
 * (c) 2015
 * https://digibird1.wordpress.com/
 *
 * Due to the buffer length of 64 it is recommended to
 * send max between 60-64 symbols at a time
 * in order to avoid the buffer to overflow!  
 * 
 * Baudrate: 9600, 19200 and 38400 tested;
 * 
 * Tested with PIC16F690
 * 
 * Created on July 17, 2015, 9:30 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define BUFFER_SIZE 64    
//Ring Buffer
char buffer[BUFFER_SIZE];
unsigned int in = 0, out = 0;
unsigned int AvailableData=0;//Keeps track how much data is available in the buffer

void UART_AddByteToBuffer(char value) { //add the next byte to the buffer
  buffer[in] = value;
  in++; 
  AvailableData++;
  if (in == BUFFER_SIZE) in = 0; // Wrap around 
}

char UART_GetNextByteFromBuffer() { //get the next byte from the buffer 
  char result = buffer[out];
  out++;
  AvailableData--;
  if (out == BUFFER_SIZE) out = 0;// Wrap around 
  return result;
}
    
int UART_Reset(){
    AvailableData=0;
    in=0;
    out=0;
}    
    
    

void UART_Init(){

    //Set the baud rate

    /*
     BaudPeriod=(_XTAL_FREQ/BaudRate/64)-1;
     */


   TXSTAbits.BRGH=1; //High BaudRate divide by 16 compared to the std div by 64
   SPBRG=51;//51;//Set the Baud Rate 9600=51; 19200=25; 38400=12;

   TXSTAbits.TXEN=1;//enable the transmitter
   TXSTAbits.SYNC=0;//asynchronous operation
   TXSTAbits.TX9=0; //Transmit 8 bits not 9 bit
   //TXIE for interrupt       


   RCSTAbits.SPEN=1;//enables the EUSART and automatically configures the TX/CK I/O pin as an output
   RCSTAbits.RX9=0;//8 bit
   //RCSTAbits.ADDEN=0; // this is needed for 9 bit mode
   RCSTAbits.CREN=1; //Enable reception

   //Interrupts for RX
   PIE1bits.RCIE=1;
   INTCONbits.PEIE=1;
   INTCONbits.GIE=1;  
    /*
           TXSTA=0b00100110;
    RCSTA=0b10010000;
    PIE1= 0b00100000;
    INTCON = 0b11000000;

    /*If the TX/CK pin is shared with an analog peripheral the analog I/O
     function must be disabled by clearing the corresponding
     ANSEL bit */          
}
    
    
int UART_writeByte(char b){//return status code
    /*
     * 
    In other words, the TXIF bit is only clear when the TSR
    is busy with a character and a new character has been
    queued for transmission in the TXREG. 

     Load 8-bit data into the TXREG register. This
     will start the transmission.

     */

    while(PIR1bits.TXIF==0);//Wait until register is ready to get the new character
    TXREG=b;//Write byte into TX register

    return 0;
}
    
int UART_writeString(const char *str){
    while(*str!='\0')
    {
        UART_writeByte(*str);
        str++;
    }
}
    
//Function from stdio can not be compiled to convert number to string :(
//so we do it by hand
//Fixme this function could probably be in a string library
//Then only the converted string needs to be send
int  UART_writeNumber(long int n){
    const int MaxLength=10;//Max 10 digits in the number

    if(n<0){
        UART_writeByte('-');
        n*=-1;
    }

    char InverseNumber[10];
    unsigned s=0;

    if(n==0){
        InverseNumber[s]=(char)(n%10+48);
        s++;
    }

    //save the number backwards
    while(n!=0 && s<MaxLength){
        InverseNumber[s]=(char)(n%10+48);//shift by 48 to convert to char number in ASCII
        n/=10;
        s++;
    }

    //print it forward
    for(int i=s-1;i>=0;i--){
        UART_writeByte(InverseNumber[i]);
    }

}
//This function prints the bit pattern of a byte
//Useful for debugging, and printing out registers 
int UART_writeBitPattern(char byte){
    for(int i=7;i>=0;i--){
        if((byte >> i) & 1){
            UART_writeByte('1');
        }
        else
            UART_writeByte('0');
    }
}
    
    
    
    
/*
 This function needs to be called in in interrupt routine
 void interrupt ISR(void)
{
    //UART interrupt
    if(RCIF && RCIE){
        UART_Interrupt();
        return;
    }
}
 */

//As soon as a byte is available at the UART controller
//This function should be called by the interrupt controller
//and the byte should be saved in the buffer
void UART_Interrupt(){
 //     if(UART_DataAvailable()<BUFFER_SIZE-1){//Check if space is available in buffer
        char RX_BIT=RCREG;//read the received data
        UART_AddByteToBuffer(RX_BIT);//Add data to buffer
//     }
}

//Returns how many bytes are available in the buffer for reading
int UART_DataAvailable(){

    if(AvailableData>=BUFFER_SIZE) return -1;//Loss of data has occured

    return AvailableData;
}

//read a single byte from the UART buffer
char UART_ReadByte(){
    return UART_GetNextByteFromBuffer();
}

//read a string from the UART with max 64 characters
//string ends are maked with the '\0' symbole
const char* UART_ReadString(){
    //FIXME should the buffer be static????
    static char buffer[65];//Max string length is 64 characters + '\0'
    char a='A';
    unsigned int s=0;
    while(UART_DataAvailable()>0){
        a=UART_ReadByte();
        if(a!='\n' && a!='\0' && a!='\r' && s<64){
           buffer[s]=a; 
           s++; 
        }
        else{
            break;
        }

    }

    buffer[s]='\0';
    return &buffer;
}

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

