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
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "ekler/fifo.h"
#include "inc/hw_types.h"
#include "utils/uartstdio.h"

#define SSID  "zcnzkn"  		    // change this to match your WiFi SSID
#define PASS  "Tarantula_oz_10@"  	// change this to match your WiFi password
#define BaudRate 9600				// baud rate
#define delayMs 100

//_________Flags
bool protokolTest=false;	//protokol hata kontrolü aktif pasif
bool fifoRxYakalamsi=false;
bool clientFound=false;
bool fsControl=false;		//fifo step control
//_________
struct my_fifo uart_fifo ;
unsigned char swap; //geçici fifo takas noktası

int i=0;
char carry;

//___________________________________________________________________________________
//#define  site "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"><html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>Home Designer</title><meta name=\"Keywords\" content=\"\"><meta name=\"Description\" content=\"\">  <style type=\"text/css\"><!--html, body {margin: 0;padding: 0;height: 100%;overflow: hidden;color: #FFF;background-color: #000;font-family: Arial, Helvetica, sans-serif;font-size: 13px;}a:link, a:visited { color: #06C; }a:hover { color: #C30; }--></style><style type=\"text/css\"></style></head><body><h1>&nbsp;<b><em>Be Different Everytime!!</b></em></h1><h3>&nbsp;<b>esp8266 server on Tiva C TM4C123GH6PM'Microcontroller</b></h3><button type=\"button\" onclick=\"alert('Hello world!')\">Click Me!</button></body></html>"
#define  site "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"><html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"en\" xml:lang=\"en\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>Home Designer</title><meta name=\"Keywords\" content=\"\"><meta name=\"Description\" content=\"\">  <style type=\"text/css\"><!--html, body {margin: 0;padding: 0;height: 100%;overflow: hidden;color: #FFF;background-color: #000;font-family: Arial, Helvetica, sans-serif;font-size: 13px;}a:link, a:visited { color: #06C; }a:hover { color: #C30; }--></style><style type=\"text/css\"></style></head><body><h1>&nbsp;<b><em>Be Different Everytime!!</b></em></h1><h3>&nbsp;<b>esp8266 server on Tiva C TM4C123GH6PM'Microcontroller</b></h3><a href=\"00\"><button type=\"00H\">00H</button></a><a href=\"01\"><button type=\"01H\">01H</button></a><a href=\"02\"><button type=\"02H\">02H</button></a><a href=\"04\"><button type=\"04H\">04H</button></a></body></html>"
//___________________________________________________________________________________

char asciiSite[3];
char site_uzantisi[30];int site_uzantisi_size;bool site_uzantisi_flag=false;
char veri[30];int veri_size;

void start_station (void);
void siteSize (int descriment);
void send_homepage(void);
void linkOrveri(void);
void link_kontrol (void);
//____________________________________________________________________________________
int Control_return(char * nextStep){

	int j=0;
	while(!(fifo_available(&uart_fifo) == 0)){		//distance değeri 0 olana kadar çevrim
		swap=fifo_read(&uart_fifo);
	    if(swap == nextStep[j]){			//fifoda chek etmek istediği,ilk charı bulana kadar karşılaştır
	    	while(swap == nextStep[j]){	//tüm check datasını kontrol et
	    		j++;
				if(nextStep[j] == '\0'){fsControl=true;return fsControl;}
				swap=fifo_read(&uart_fifo);
	    	}
	    	  fsControl=false;return fsControl;
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
void ConfigureUART1(void){
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
/*void TimerConf(void){
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ROM_SysCtlClockGet() / 2);
	TimerIntRegister(TIMER0_BASE, TIMER_A, google_ping);
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);



}*/
//_________________________________________________________________________________________
int main(void){
    ROM_FPUEnable();
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_SYSDIV_5);
    ROM_IntMasterEnable();
    ConfigureUART0();
    ConfigureUART1();
   // TimerConf();
    //________________________________________
    UARTSend(UART0_BASE,"Protokol_Baslatiliyor\r\n");
    start_station();
    fifo_init(&uart_fifo);  //fifo kayıtçılarını sıfırla
	fifoRxYakalamsi=true;	//fifo gelen rx yakalaması aktif edildi
    clientFound=false;		//istemci yakalama kayıtçısı resetlendi
    site_uzantisi_flag=true;//site uzantısı bulma etkin
    UARTSend(UART0_BASE,"'istemci_Bekleniyor\r\n");
    //________________________________________
    while(1) {
    	SysCtlDelay((SysCtlClockGet()/3000)*1000); //loop gecikmesi
    	if(clientFound){			//esp8266'dan veri geldi
    		linkOrveri();
        }
    	    clientFound=false; //yeni istemciyi bulmak için sıfırla
    }
}

//_________________________________________________________________________________________
void start_station (void){				//wifi station starting protokol
	for(i=0;i<9;i++){
		switch(i){
		case 0:  UARTSend(UART1_BASE,"AT\r\n"); 		break;
		case 1:  UARTSend(UART1_BASE,"AT+RST\r\n");
		 SysCtlDelay((SysCtlClockGet()/3000)*3000);		break;
		case 2:	 UARTSend(UART1_BASE,"ATE0\r\n");		break;
		case 3:	 UARTSend(UART1_BASE,"AT+CWMODE=1\r\n");break;
		case 4:  UARTSend(UART1_BASE,"AT+CIPMUX=1\r\n");break;
		case 5:
			UARTSend(UART1_BASE,"AT+CWJAP=\"");
			UARTSend(UART1_BASE,SSID);
			UARTSend(UART1_BASE,"\",\"");
			UARTSend(UART1_BASE,PASS);
			UARTSend(UART1_BASE,"\"\r\n");
			SysCtlDelay((SysCtlClockGet()/3000)*4000);  break;

		case 6:  UARTSend(UART1_BASE,"AT+CIFSR\r\n");   break;
		case 7:  UARTSend(UART1_BASE,"AT+CIPSERVER=1,80\r\n");	break;
		case 8:
			fifoRxYakalamsi=true;	//fifo gelen rx yakalaması aktif edildi
			protokolTest=true;
			UARTSend(UART1_BASE,"AT+CIPSTO=500\r\n");   break;
		}
		SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
		if(protokolTest){Control_return("OK\r\n");}
	    if(protokolTest && (!fsControl)){UARTSend(UART0_BASE,"Hata_Protokol_Yeniden_Baslatilacak\r\n");i=0;protokolTest=false;fifo_init(&uart_fifo);}
	    if(protokolTest &&   fsControl) {UARTSend(UART0_BASE,"Protokol_Basariyla_Baslatildi\r\n");fsControl=false;fifo_init(&uart_fifo);}
	}
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
//_________________________________________________________________________________________
/*void google_ping(void){



}*/
//_________________________________________________________________________________________
void send_homepage(void){
    siteSize(sizeof site);					 //gönderilecek verirnin boyutunu belirle
    UARTSend(UART1_BASE,"AT+CIPSEND=0,");	 //data göndermeye başla kodu
    UARTSend(UART1_BASE,asciiSite);			 //data göndermeye başla kodu
    UARTSend(UART1_BASE,"\r\n"); 			 //data göndermeye başla kodu
    SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
    UARTSend(UART1_BASE,site);					//datayı gonder
    SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
    UARTSend(UART1_BASE,"AT+CIPCLOSE=\"5\"\r\n"); //TCP 3handshake 5channel ,Close
    SysCtlDelay((SysCtlClockGet()/3000)*delayMs);
}
//_________________________________________________________________________________________

void linkOrveri(void){

SysCtlDelay((SysCtlClockGet()/3000)*200);  //istemci bilgi göndermesinin tamamlanmasını bekle
Control_return(":");		//fifoda : harfine bak
//saçma bir hata olursa fifoyu sıfırla
if(!fsControl){UARTSend(UART0_BASE,"istemci_hatası  ----");fifo_init(&uart_fifo);}
if(fsControl){						// : datası fifoda bulundu
fsControl=false;				    //fifo kontrol kayıtçısını sıfırla
get_struct_indis(&uart_fifo); 		// :'nin fifodaki adresini kaydet
//________________________________________
Control_return("HTTP");		  		//site isteği mi?
if(!fsControl){						//HTTP'den hatalı dönmesi demek veri geldi demektir
	UARTSend(UART0_BASE,"veri buldu\r\n");
	set_struct_indis(&uart_fifo); 		    // fifoda : konumuna geri dön
	int linkD=0;
	swap=fifo_read(&uart_fifo);
	while(!(swap=='\r')){					//gelen veri bittikten sonra yazılan standart data
		veri[linkD]=swap;
		linkD++;
		swap=fifo_read(&uart_fifo);
	}
	veri_size=linkD;
	UARTSend(UART0_BASE,veri);
	UARTSend(UART0_BASE,"\r\n\r\n");
	fifo_init(&uart_fifo);
}



if(fsControl){fsControl=false;			//HTTP başarılı döngügüne göre site ama uzantısı var mı? yoksa ana sayfamı ?
set_struct_indis(&uart_fifo); 		    // fifoda : konumuna geri dön
Control_return("GET /");					// fifoda / noktasına ilerle
fsControl=false; 						//fifo step controlü sıfırla
link_kontrol();
}
}

}

//______________________________
void link_kontrol (void){
	int linkC=0;
swap=fifo_read(&uart_fifo);
	while(!(swap==0x20)){
		site_uzantisi[linkC]=swap;
		linkC++;
		swap=fifo_read(&uart_fifo);
	}
	site_uzantisi_size=linkC;
	if(site_uzantisi_size==0){
		UARTSend(UART0_BASE,"\r\nanasayfa\r\n\r\n");
		send_homepage();
	}
	else{
		UARTSend(UART0_BASE,"\r\nuzantı var\r\n");
		UARTSend(UART0_BASE,site_uzantisi);
		UARTSend(UART0_BASE,"\r\n\r\n");
	}
	fifo_init(&uart_fifo);
}
