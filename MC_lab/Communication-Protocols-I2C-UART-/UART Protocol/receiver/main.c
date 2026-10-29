/* 
	UART Receiver
 	PA10  -> USART1_RX  (AF7) D2
*/
#define RCC_BASE (0x40023800)
#define GPIOA_BASE (0x40020000)
#define UART_BASE (0x40011000)



#define RCC_AHB1ENR (*(volatile unsigned int*)( RCC_BASE + 0x30))
#define GPIOA_MODER (*(volatile unsigned int*)(GPIOA_BASE + 0))
#define GPIOA_AFRH (*(volatile unsigned int*)(GPIOA_BASE + 0x24))
#define RCC_APB2ENR (*(volatile unsigned int*)(RCC_BASE + 0x44))
#define UART1_BRR (*(volatile unsigned int*)(UART_BASE + 0x08))
#define UART1_CR1 (*(volatile unsigned int*)(UART_BASE + 0x0C))
#define UART1_SR (*(volatile unsigned int*)(UART_BASE + 0))
#define UART1_DR (*(volatile unsigned int*)(UART_BASE + 0x04))


#define SYSCLK 16000000U //HSI 16 MHz
#define BAUD_RATE 9600U
void UART1_INIT();
void UART_READ(int n, char *str);

int main(){
	UART1_INIT();
	char str[6];
	while(1){


	UART_READ(6, str);

	}
}

void UART1_INIT(){
	//Enable clock
	RCC_AHB1ENR |= (1<<0); //RCC_AHB1ENR bit0 = GPIOAEN

	/* Alternate mode enable
	 Configure PA10 (D2) as Alternate Function
	MODER10[1:0] = 10 (Alternate Function mode)
	*/
	GPIOA_MODER &=~(1<<20);
	GPIOA_MODER |=(1<<21);

	//Introduce UART1 and for that set AF7

	GPIOA_AFRH &=~(0xF<<8);
	GPIOA_AFRH |=(0x7<<8);

	// Enable APB2
	RCC_APB2ENR|=(1<<4);

	//SET BAUD RATE
	UART1_BRR = ((SYSCLK + (BAUD_RATE/2))/BAUD_RATE);

	UART1_CR1 |= (1<<2);//Receiver ENALE CR1 bit2 = RE , Page - 839

	UART1_CR1 |=(1<<13); //UART_ENABLE

}

void UART_READ(int n, char* str){

	for(int i=0;i<n;i++){
		/*
		   Wait until RXNE = 1
           SR bit5 (RXNE) → Data available in DR
		   Page - 835
		 */
		while(!(UART1_SR & (1<<5))){}

		//Read received byte from DR
		str[i] = UART1_DR;
	}

}
