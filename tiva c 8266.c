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

#define SSID  "zcnzkn"              // change this to match your WiFi SSID
#define PASS  "Tarantula_oz_10@"    // change this to match your WiFi password
#define BaudRate 9600               // baud rate
#define delayMs 150

//_________Flags
bool protokolTest=false;    //protokol hata kontrolü aktif pasif
bool fifoRxYakalamsi=false;
bool clientFound=false;
bool passed=false;          //protokol hata kontrolü
bool fsControl=false;       //fifo step control
//_________
struct my_fifo uart_fifo ;
//unsigned int BaudRate=9600;   //baud rate oranları
//unsigned int delayMs=150; //komut arası geçş süresi

int i=0;


char carry;


int say=0;
//___________________________________________________________________________________
#define  site "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"><html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>Home Designer</title><meta name=\"Keywords\" content=\"\"><meta name=\"Description\" content=\"\">  <style type=\"text/css\"><!--html, body {margin: 0;padding: 0;height: 100%;overflow: hidden;color: #FFF;background-color: #000;font-family: Arial, Helvetica, sans-serif;font-size: 13px;}a:link, a:visited { color: #06C; }a:hover { color: #C30; }--></style><style type=\"text/css\"></style></head><body><h1>&nbsp;<b><em>Be Different Everytime!!</b></em></h1><h3>&nbsp;<b>esp8266 server on Tiva C TM4C123GH6PM'Microcontroller</b></h3></body></html>"
//___________________________________________________________________________________


char compare[16];
char nextStep[16];
char asciiSite[3];

int protokolHataKontrol(char aa);
void start (void);
void siteSize (int descriment);
//____________________________________________________________________________________
unsigned char my_getc(){
    while(fifo_available(&uart_fifo) == 0);
    return fifo_read(&uart_fifo);
}
int putItCompare(char * veri,int size){
    int for1;
    for(for1=0;for1<size;for1++){
        nextStep[for1]=* veri;
        veri++;
    }
    return fsControl_returnA(size);
}
int fsControl_returnA(int size)
{
    int j;
    for(i=0;i<FIFO_SIZE;i++){           //fifodan veriyi al
        compare[i]=fifo_read(&uart_fifo);
    }
/*
    UARTSend(UART0_BASE,"\r\n______fifo________\r\n");
    UARTSend(UART0_BASE,compare);
    UARTSend(UART0_BASE,"\r\n_____\r\n");
    UARTSend(UART0_BASE,"\r\n______compare_____\r\n");
    UARTSend(UART0_BASE,nextStep);
    UARTSend(UART0_BASE,"\r\n_____\r\n");*/

    for(i=0;sizeof compare;i++){
        if(compare[i]== nextStep[0]){
                while(1){
                    for(j=1;j<size;j++){
                        i++;
                        if(!(compare[i]== nextStep[j])){fsControl=false;return fsControl;}
                    }
                    fsControl=true;return fsControl;
                }
        }
}
    fsControl=false;return fsControl;
}
//_________________________________________________________________________________________
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif
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
    clientFound=true;
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
        if(protokolTest){protokolHataKontrol(carry);}   //protokol olarak yazıldığı için ve yakalaması kolay olduğu için böyle
        if(fifoRxYakalamsi){fifo_write(&uart_fifo, carry);}//fifo veri yakalaması
        //_________________________________________
        ROM_UARTCharPutNonBlocking(UART0_BASE,carry);
    }
}
//_________________________________________________________________________________________
void UARTSend (uint32_t nBase,char *pui8Buffer)
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
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);
    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), BaudRate,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                             UART_CONFIG_PAR_NONE));
    ROM_IntEnable(INT_UART1);
    ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    UARTIntRegister(UART1_BASE, UARTIntHandlerB);
}
//_________________________________________________________________________________________
int main(void){
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    ROM_IntMasterEnable();
    ConfigureUART0();
    ConfigureUART1();

    UARTSend(UART0_BASE,"Protokol_Baslatiliyor\r\n");
    SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
    //________________________________________
    start();
    fifoRxYakalamsi=true;   //fifo gelen rx yakalaması aktif edildi
    clientFound=false;      //rx içerisinde istemci bekleniyor
    UARTSend(UART0_BASE,"'istemci_Bekleniyor\r\n");
    //________________________________________
    while(1) {

        SysCtlDelay((SysCtlClockGet()/3000)*2000); //loop gecikmesi
        if(clientFound){            //8266'dan veri geldi
            SysCtlDelay((SysCtlClockGet()/3000)*750);  //istemci bilgi göndermesinin tamamlanmasını bekle
            //___________doğru tamamlandımı kontrol et
            putItCompare("\r\nOK\r\n",6);
            if(!fsControl){UARTSend(UART0_BASE,"istemci  ----");}

            if(fsControl){fsControl=false;
            siteSize(sizeof site);                   //gönderilecek verirnin boyutunu belirle
            UARTSend(UART1_BASE,"AT+CIPSEND=0,");    //data göndermeye başla kodu
            UARTSend(UART1_BASE,asciiSite);          //data göndermeye başla kodu
            UARTSend(UART1_BASE,"\r\n");             //data göndermeye başla kodu
            SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
            UARTSend(UART1_BASE,site);                  //gönderme kodu
            SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
        //    while(!fsControl){putItCompare("SEND OK\r\n",9);} //Veri gönderilene kadar bekle
            //____bitirme kodu
            UARTSend(UART1_BASE,"AT+CIPCLOSE=\"5\"\r\n");
            SysCtlDelay((SysCtlClockGet()/3000)*1000);
        }
            clientFound=false; //yeni istemciyi bulmak için sıfırla
    }
}
}
//_________________________________________________________________________________________
void start (void){
    for(i=0;i<9;i++){
        switch(i){
        case 0:  UARTSend(UART1_BASE,"AT\r\n");         break;
        case 1:  UARTSend(UART1_BASE,"AT+RST\r\n");
         SysCtlDelay((SysCtlClockGet()/3000)*3000);     break;
        case 2:  UARTSend(UART1_BASE,"ATE0\r\n");       break;
        case 3:  UARTSend(UART1_BASE,"AT+CWMODE=1\r\n");break;
        case 4:  UARTSend(UART1_BASE,"AT+CIPMUX=1\r\n");break;
        case 5:
            UARTSend(UART1_BASE,"AT+CWJAP=\"");
            UARTSend(UART1_BASE,SSID);
            UARTSend(UART1_BASE,"\",\"");
            UARTSend(UART1_BASE,PASS);
            UARTSend(UART1_BASE,"\"\r\n");
            SysCtlDelay((SysCtlClockGet()/3000)*4000);  break;

        case 6:  UARTSend(UART1_BASE,"AT+CIFSR\r\n");   break;
        case 7:  UARTSend(UART1_BASE,"AT+CIPSERVER=1,80\r\n");  break;
        case 8:
            fifoRxYakalamsi=true;
            protokolTest=true;
            UARTSend(UART1_BASE,"AT+CIPSTO=500\r\n");   break;
        }
        SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
        if(protokolTest && (!passed)){UARTSend(UART0_BASE,"Hata_Protokol_Yeniden_Baslatilacak\r\n");i=0;protokolTest=false;}
        if(protokolTest &&   passed) {UARTSend(UART0_BASE,"Protokol_Basariyla_Baslatildi\r\n");passed=false;}
    }
}
//_________________________________________________________________________________________
//protokol olarak yazıldığı için ve yakalarken karşılaştırma kolay olduğu için böyle
int protokolHataKontrol(char aa){       //protokol hata kontrolü

    if(aa=='O'){say=1;}
    if(say==1 && (aa=='K')){passed=true;say=0;return passed;}
}
//_________________________________________________________________________________________
void siteSize (int descriment){
    descriment--;
    asciiSite[2] = descriment%10 + 0x30;
    descriment /= 10;
    asciiSite[1] = descriment%10 + 0x30;
    descriment /= 10;
    asciiSite[0] = descriment%10 + 0x30;
}

