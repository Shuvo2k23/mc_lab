/*
  UART Transmeter
  PA9  -> USART1_TX  (AF7) D8
*/
#define RCC_BASE (0x40023800)
#define GPIOA_BASE (0x40020000)
#define UART_BASE (0x40011000)



#define RCC_AHB1ENR (*(volatile unsigned int*)( RCC_BASE + 0x30)) // AHB1 peripheral clock enable
#define GPIOA_MODER (*(volatile unsigned int*)(GPIOA_BASE + 0)) // GPIO mode register
#define GPIOA_AFRH (*(volatile unsigned int*)(GPIOA_BASE + 0x24)) // Alternate function high register
#define RCC_APB2ENR (*(volatile unsigned int*)(RCC_BASE + 0x44))

#define UART1_BRR (*(volatile unsigned int*)(UART_BASE + 0x08)) // Baud rate register
#define UART1_CR1 (*(volatile unsigned int*)(UART_BASE + 0x0C)) // Control register 1
#define UART1_SR (*(volatile unsigned int*)(UART_BASE + 0)) // Status register
#define UART1_DR (*(volatile unsigned int*)(UART_BASE + 0x04)) // Data register

/* DataSheet
	Block Diagram : P-16
	Alternate Function: P-58
	Memory Map: P-65
*/


#define SYSCLK 16000000U // HSI clock 16 MHz
#define BAUD_RATE 9600U
void UART1_INIT();
void SEND(int n, char *str);

int main(){
	UART1_INIT();  // Initialize UART1
	while(1){
		SEND(6, "CSE_RU");
	}
}

void UART1_INIT(){
    /*
       Enable GPIOA Clock
       RCC_AHB1ENR bit0 = GPIOAEN
	   Page -144
     */
	RCC_AHB1ENR |= (1<<0);


    /*
      Set PA9 as Alternate Function mode
       MODER9[1:0] = 10 (Alternate Function)
	   Page - 187
     */

	GPIOA_MODER &=~(1<<18);
	GPIOA_MODER |=(1<<19);

	//Introduce USART1_TX and for that set AF7
	//Ref Page - 192 , DataSheet-57

	GPIOA_AFRH &=~(0xF<<4); 
	GPIOA_AFRH |=(0x7<<4);

	// Enable APB2 , P-149
	RCC_APB2ENR|=(1<<4); 

	 /* Set Baud Rate
       BRR = fCK / baud
       16000000 / 9600 ≈ 1667 (0x683)
	   round(a/b)=(a+b/2)/b
	 */
	UART1_BRR = ((SYSCLK + (BAUD_RATE/2))/BAUD_RATE); // Page - 838
	

	UART1_CR1 |= (1<<3);//TRANSMITER ENALE , P-839

	UART1_CR1 |=(1<<13); //UART_ENABLE

}

void SEND(int n, char* str){

	for(int i=0;i<n;i++){
		while(!(UART1_SR & (1<<7))){} //Page - 835
		UART1_DR = *str++; //Page-838
	}

}
