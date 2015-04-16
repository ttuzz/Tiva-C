#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "ekler/fifo.h"
//#include <iostream>

struct my_fifo uart_fifo ;
unsigned int BaudRate=9600;
unsigned int delayMs=150,i=0,say=0,passed=0,istemciYakalandi=0,checkLoopControl=0,checkLoopControlData=0;
char a=0x22,carry;
bool testNeeD=0;
bool istemci=0;
int CHECK=0;
int k=0;

const char DdD[187]={"<html><head><title>ESP8266 User Program Example</title></head><body><p><b>Be Different Everytime!!</b></p><p><b>esp8266 server on Tiva C TM4C123GH6PM'Microcontroller</b></p></body></html>"};

bool fifoRxYakalamsi=0;
bool istemciGeldi=0;

char dd;
//char compare[]={0x4c,0x69,0x6e,0x6b};		 //Link
char compare[FIFO_SIZE];//=//{0x4f,0x4b,0x0d,0x0a};		 //OK\r\n
char nextStep[FIFO_SIZE];


unsigned int protokolHataKontrol(char aa);
//____________________________________________________________________________________
unsigned char my_getc(){
    while(fifo_available(&uart_fifo) == 0);
    return fifo_read(&uart_fifo);
}
void putItCompare(char * veri,int size){
	int for1;
	for(for1=0;for1<size;for1++){
		nextStep[for1]=* veri;
		veri++;
	}
	UARTSend(UART0_BASE,"putItCompare'den cikiyor\r\n");
	check_returnA(size);
}
int check_returnA(int size)
{
	int hatasizlik=0,j;
    for(i=0;i<FIFO_SIZE;i++){			//fifodan veriyi al
    	compare[i]=fifo_read(&uart_fifo);
    }/*
	UARTSend(UART0_BASE,"check aktif edildi\r\n");
	UARTSend(UART0_BASE,"compare____\r\n");
	UARTSend(UART0_BASE,compare);
	UARTSend(UART0_BASE,"nextstep___\r\n");
	UARTSend(UART0_BASE,nextStep);*/

    for(i=0;sizeof compare;i++){
	    if(compare[i]== nextStep[0]){
		    	while(1){
		    		for(j=1;j<size;j++){
		    			i++;
		    			if(!(compare[i]== nextStep[j])){CHECK=0;return CHECK;}
		    			//if(compare[i]== nextStep[j])
		    		}
		    		CHECK=1;return CHECK;
		    	}
	    }
}
}

//_________________________________________________________________________________________
//_________________________________________________________________________________________
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
//_________________________________________________________________________________________
void start (void);
void sendhex_uart1(void){
    while(!ROM_UARTCharPutNonBlocking(UART1_BASE, 0x22));// "
}
//_________________________________________________________________________________________
//_________________________________________________________________________________________
void UARTIntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);
    ROM_UARTIntClear(UART0_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART0_BASE)){
       ROM_UARTCharPutNonBlocking(UART1_BASE,ROM_UARTCharGetNonBlocking(UART0_BASE));}
}
//_________________________________________________________________________________________
void UARTIntHandlerB(void)
{
	istemciGeldi=1;
    uint32_t ui32Status;
    // Get the interrrupt status.
    ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
    // Clear the asserted interrupts.
    ROM_UARTIntClear(UART1_BASE, ui32Status);
    // Loop while there are characters in the receive FIFO.
    while(ROM_UARTCharsAvail(UART1_BASE))
    {
        // Read the next character from the UART1 and write it back to the UART0.
    	carry=ROM_UARTCharGetNonBlocking(UART1_BASE);
    	//_________________________________________
    	if(testNeeD){protokolHataKontrol(carry);}	//protokol olarak yazıldığı için ve yakalaması kolay olduğu için böyle
    	if(fifoRxYakalamsi){fifo_write(&uart_fifo, carry);}
     	//_________________________________________
    	ROM_UARTCharPutNonBlocking(UART0_BASE,carry);
    }
}
//_________________________________________________________________________________________
void UARTSend(uint32_t nBase,char *pui8Buffer)
{
	while(*pui8Buffer!='\0'){
		if(UARTSpaceAvail(nBase)){
			while(!ROM_UARTCharPutNonBlocking(nBase, *pui8Buffer++));
		}
	}
}
//_________________________________________________________________________________________
void ConfigureUART0(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), BaudRate,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART0_BASE, UARTIntHandler);
}
//_________________________________________________________________________________________
void ConfigureUART1(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), BaudRate,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    // Enable the UART interrupt.
    ROM_IntEnable(INT_UART1);
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART1_BASE, UARTIntHandlerB);
}
//_________________________________________________________________________________________
int main(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    // Set the clocking to run directly from the crystal.
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // Enable processor interrupts.
    ROM_IntMasterEnable();
    ConfigureUART0();
    ConfigureUART1();
    // Prompt for text to be entered.
    UARTSend(UART0_BASE,"Protokol_Baslatiliyor\r\n");
    SysCtlDelay((SysCtlClockGet()/3000)*1000);
    //________________________________________
    start();
    //buraya geldiğinde testNeeD=1 aktif ve sistem istemci bekliyor.
    //check_returnA("OK");
   // if(CHECK){UARTSend(UART0_BASE,"koda calisti");CHECK=0;}
    //testNeeD=1;

	fifoRxYakalamsi=1;	//fifo gelen rx yakalaması aktif edildi
    istemciGeldi=0;		//rx içerisinde istemci bekleniyor
    UARTSend(UART0_BASE,"'istemci bekleniyor\r\n");
    //________________________________________
    while(1) {

    	SysCtlDelay((SysCtlClockGet()/3000)*2000);
    	if(istemciGeldi){
    		SysCtlDelay((SysCtlClockGet()/3000)*3000);  //istemci bilgi göndermesinin tamamlanmasını bekle
    		//___________doğru tamamlandımı kontrol et
    	    putItCompare("link",4);
    	    if(!CHECK){UARTSend(UART0_BASE,"istemci  ----");}
    	    if(CHECK){UARTSend(UART0_BASE,"istemci ++++ veri gonderilebilir\r\n");}

    	    UARTSend(UART1_BASE,"AT+CIPSEND=0,187\r\n"); //data göndermeye başla kodu
    	    SysCtlDelay((SysCtlClockGet()/3000)*500);
    	    UARTSend(UART1_BASE,DdD);
    	    SysCtlDelay((SysCtlClockGet()/3000)*2000);
    	    //____bitirme kodu
    	    UARTSend(UART1_BASE,"AT+CIPCLOSE=");
    	    sendhex_uart1();
    	    UARTSend(UART1_BASE,"5");
    	    sendhex_uart1();
    	    UARTSend(UART1_BASE,"\r\n");
			//___

    	    //____________
    	    istemciGeldi=0; //yeni sitemciyi bulmak için sıfırla
        }
    }
}
//_________________________________________________________________________________________
void start (void){
	for(i=0;i<9;i++){
		switch(i){
		case 0:  UARTSend(UART1_BASE,"AT\r\n"); 		break;
		case 1:  UARTSend(UART1_BASE,"AT+RST\r\n");
		 SysCtlDelay((SysCtlClockGet()/3000)*3000);		break;
		case 2:	 UARTSend(UART1_BASE,"ATE0\r\n");		break;
		case 3:	 UARTSend(UART1_BASE,"AT+CWMODE=1\r\n");break;
		case 4:  UARTSend(UART1_BASE,"AT+CIPMUX=1\r\n");break;
		case 5:
			UARTSend(UART1_BASE,"AT+CWJAP=");
			sendhex_uart1();UARTSend(UART1_BASE,"zcnzkn");sendhex_uart1();
			UARTSend(UART1_BASE,",");
			sendhex_uart1();UARTSend(UART1_BASE,"Tarantula_oz_10@");sendhex_uart1();
			UARTSend(UART1_BASE,"\r\n");
			SysCtlDelay((SysCtlClockGet()/3000)*4000);  break;

		case 6:  UARTSend(UART1_BASE,"AT+CIFSR\r\n");   break;
		case 7:  UARTSend(UART1_BASE,"AT+CIPSERVER=1,80\r\n");	break;
		case 8:
			testNeeD=1;
			UARTSend(UART1_BASE,"AT+CIPSTO=500\r\n");   break;
		}
		SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
	    if(testNeeD && (!passed)){UARTSend(UART0_BASE,"Hata_Protokol_Yeniden_Baslatilacak\r\n");i=0;testNeeD=0;}
	    if(testNeeD &&   passed) {UARTSend(UART0_BASE,"Protokol_Basariyla_Baslatildi\r\n");passed=0;}

	}
}
//_________________________________________________________________________________________
//protokol olarak yazıldığı için ve yakalarken karşılaştırma kolay olduğu için böyle
unsigned int protokolHataKontrol(char aa){		//protokol hata kontrolü
	if(aa=='O'){say=1;}
	if(say==1 && (aa=='K')){passed=1;say=0;return passed;}
}
//_________________________________________________________________________________________
