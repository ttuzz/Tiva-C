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

#include "Petit/pff.h"
#include "Petit/pffconf.h"

unsigned int BaudRate=115200;

FRESULT rc;
FATFS fatfs;
UINT br,bw;

#define	WRITE_DATA_PACKET_SIZE	64
BYTE bWriteBuffer[WRITE_DATA_PACKET_SIZE];

int j,bSayac;

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void sd_card (void);

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    ui32Status = ROM_UARTIntStatus(UART0_BASE, true);
    ROM_UARTIntClear(UART0_BASE, ui32Status);
    while(ROM_UARTCharsAvail(UART0_BASE))
    {
        ROM_UARTCharPutNonBlocking(UART0_BASE,ROM_UARTCharGetNonBlocking(UART0_BASE));
    }
}
void UARTSend(uint32_t nBase,char *pui8Buffer)
{
	while(*pui8Buffer!='\0'){
		if(UARTSpaceAvail(nBase)){
			while(!ROM_UARTCharPutNonBlocking(nBase, *pui8Buffer++));
		}
	}
}
//*****************************************************************************
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
    // Enable the UART interrupt.
    ROM_IntEnable(INT_UART0);
    ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART0_BASE, UARTIntHandler);
}
//*****************************************************************************
int main(void)
{
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    ROM_IntMasterEnable();

    ConfigureUART0();
    UARTSend(UART0_BASE,"Hazir\n\r");

    sd_card();

    while(1) { }
}
//*****************************************************************************
void
sd_card (void){
    // try mounting sd card.
	rc = pf_mount(&fatfs);
	SysCtlDelay((SysCtlClockGet()/3000)*500);
	while(!(rc==FR_OK)){
		rc = pf_mount(&fatfs);
		SysCtlDelay((SysCtlClockGet()/3000)*500);
		UARTSend(UART0_BASE,"SD off\n\r");
	}
	UARTSend(UART0_BASE,"SD on\n\r");

	// open sd card data.
	rc = pf_open("a/deneme.txt");	SysCtlDelay((SysCtlClockGet()/3000)*500); 	UARTSend(UART0_BASE,"pf_open good\n\r");

	pf_read(bWriteBuffer,WRITE_DATA_PACKET_SIZE,&br);
	UARTSend(UART0_BASE,bWriteBuffer);		//okunan veriyi uarta yolla
	UARTSend(UART0_BASE,"\n\r");
	for (bSayac=0; bSayac<=WRITE_DATA_PACKET_SIZE; bSayac++)
	{
	if( bWriteBuffer[bSayac] >= 'a' && bWriteBuffer[bSayac] <= 'z')	// Karakter küçük harf mi?
		bWriteBuffer[bSayac] -= 0x20;								// Evet ise büyük harf yap.
	}

	pf_write(bWriteBuffer,WRITE_DATA_PACKET_SIZE,&br);	//geri yaz
	UARTSend(UART0_BASE,"pf_write ok\n\r");

	pf_write(0, 0, &bw); //close data
}
