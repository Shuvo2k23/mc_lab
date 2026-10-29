// Master I2C Communication Protocol

// List of Register Declarations
#define RCC_BASE   (0x40023800)
#define GPIOA_BASE (0x40020000)
#define GPIOB_BASE (0x40020400)
#define I2C1_BASE  (0x40005400)
#define TIM2_BASE  (0x40000000)

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x40))

#define GPIOA_MODER (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_OTYPER (*(volatile unsigned int *)(GPIOA_BASE + 0x04))
#define GPIOA_ODR   (*(volatile unsigned int *)(GPIOA_BASE + 0x14))

#define GPIOB_MODER (*(volatile unsigned int *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER (*(volatile unsigned int *)(GPIOB_BASE + 0x04))
#define GPIOB_PUPDR (*(volatile unsigned int *)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRH  (*(volatile unsigned int *)(GPIOB_BASE + 0x24))

#define I2C1_CR1 (*(volatile unsigned int *)(I2C1_BASE + 0x00))
#define I2C1_CR2 (*(volatile unsigned int *)(I2C1_BASE + 0x04))
#define I2C1_CCR (*(volatile unsigned int *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE (*(volatile unsigned int *)(I2C1_BASE + 0x20))
#define I2C1_SR1 (*(volatile unsigned int *)(I2C1_BASE + 0x14))
#define I2C1_SR2 (*(volatile unsigned int *)(I2C1_BASE + 0x18))
#define I2C1_DR  (*(volatile unsigned int *)(I2C1_BASE + 0x10))

#define TIM2_PSC (*(volatile unsigned int *)(TIM2_BASE + 0x28))
#define TIM2_ARR (*(volatile unsigned int *)(TIM2_BASE + 0x2C))
#define TIM2_CR1 (*(volatile unsigned int *)(TIM2_BASE + 0x00))
#define TIM2_SR  (*(volatile unsigned int *)(TIM2_BASE + 0x10))

//function definition to reduce warning
void I2C1_Init();  // 3) STM32_Nucleo_64_boards_Manualcs Page: 1, 35


//send function
void I2C1_Send(char saddr, int n, char* str);


//main function
int main(void) {
	I2C1_Init();
	while(1) {
		I2C1_Send(0x12, 6, "CSE-RU");
	}
}



void I2C1_Init() {

	// 2) STM32f446re_DataSheet - 58
	// 2) STM32f446re_DataSheet - 16
	// 1) STM32f446xx_Reference - 144
	/*Enable clock of GPIOB through AHB1ENR--144*/
	RCC_AHB1ENR |= (1<<1);

	// 2) STM32f446re_DataSheet - 58
	// 1) STM32f446xx_Reference - 187
	/*Set PB8 & PB9 mode to alternate function through GPIOB_MODER , default input mode-- 187*/
	GPIOB_MODER |= (1<<19);  // Set 1
	GPIOB_MODER &= ~(1<<18); // Set 0 or Reset
	GPIOB_MODER |= (1<<17);
	GPIOB_MODER &= ~(1<<16);

	// I2C Open Drain Gif (https://www.linkedin.com/pulse/i2c-how-does-select-push-pull-output-open-drain-nianhang-hu-jdtmc/)
	// 1) STM32f446xx_Reference - 188
	/*Set PB8 & PB9 output type to open-drain for high to low --188*/
	GPIOB_OTYPER |= (1<<8);
	GPIOB_OTYPER |= (1<<9);

	// 1) STM32f446xx_Reference - 189
	/*Set PB8 & PB9 to output pull-up for always high and take internal power -- 189*/
	GPIOB_PUPDR &= ~(1<<19);
	GPIOB_PUPDR |= (1<<18);
	GPIOB_PUPDR &= ~(1<<17);
	GPIOB_PUPDR |= (1<<16);

	// 2) STM32f446re_DataSheet - 58
	// 1) STM32f446xx_Reference - 192
	/*Set PB8 & PB9 alternate function type to I2C (AF4) for handling multiple function -- 192*/
	GPIOB_AFRH &= ~(0xFF<<0);
	GPIOB_AFRH |= (1<<2);
	GPIOB_AFRH |= (1<<6);


	// 2) STM32f446re_DataSheet - 16
	// 1) STM32f446xx_Reference - 147
	/*Enable clock of I2C1 --147*/
	RCC_APB1ENR |= (1<<21);

	// 1) STM32f446xx_Reference - 780
	/*Enter I2C peripheral reset mode -- 780*/
	I2C1_CR1 |= (1<<15);

	// 1) STM32f446xx_Reference - 780
	/*Come out of reset mode --780*/
	I2C1_CR1 &= ~(1<<15);

	// 2) STM32f446re_DataSheet - 16
	// 1) STM32f446xx_Reference - 782, 783
	/*Set I2C clock frequency (default 16 MHz) -- 782*/
	I2C1_CR2 |= (1<<4);

	
	// *** Important Part ***
	// 2) STM32f446re_DataSheet - 16 
	// 1) STM32f446xx_Reference - 790
	/*Set I2C in standard mode, 100 KHz -- 790*/
	//	       CCR = f_PCLK1 / (2 * f_SCL)
	//	    	   = 16,000,000 / (2 * 100,000)
	//	    	   = 80


	I2C1_CCR = 80;

	// 1) STM32f446xx_Reference - 761, 792
	/*Set rise time of Clock Frequency --- 761  & 791*/

	// 		   TRISE = (t_r * f_PCLK1) + 1
	// 		   		 = (1x10^-6 * 16x10^6) + 1
	// 		   		 = 17
	I2C1_TRISE = 17;

	// 1) STM32f446xx_Reference - 780, 782
	/*Now finally enable the I2C1 , Must in Last-- 780*/
	I2C1_CR1 |= (1<<0);

}

// Need address in I2C than UART, Here, saddr - Slave Address (See I2C and UART) 
void I2C1_Send(char saddr, int n, char* str) {
	
	// 1) STM32f446xx_Reference - 761, 789, 790
	/*Wait when bus is busy 789*/
	while(I2C1_SR2 & (1<<1)) {}

	// 1) STM32f446xx_Reference - 780, 785, 788
	/*Generate start condition and wait -- 780 & 785*/
	I2C1_CR1 |= (1<<8);             // Set Start Condition
	while(!(I2C1_SR1 & (1<<0))) {}  // Then Check and Wait is it Set or Not


	// See I2C Message Packet Start->Address
	// 1) STM32f446xx_Reference - 785, 788, I2C Address Frame, 762,785 
	// 1) STM32f446xx_Reference - 785, 788 (ACK as Address Match)
	/*Send slave address + write (0) bit and wait for ACK -- 762 & 785*/
	I2C1_DR = (saddr<<1);  // Set 0 to bit 0 position (master write), automaticall done because of left shift
	while(!(I2C1_SR1 & (1<<1))) {}   // 1st Read SR1

	// 1) STM32f446xx_Reference - 788 
	/*Clear ADDR flag to ensure address is sent -- 788*/
	(void)I2C1_SR2;   // 2nd Read SR2 int temp = I2C1_SR2 will generate warning

	// Start Data Transfer Byte by Byte
	// 1) STM32f446xx_Reference - 762 
	// Check whether Data Register is Empty or Not.
	// 1) STM32f446xx_Reference - 785, 787

	for(int i = 0; i < n; i++) {

		/*Wait until transmitter is empty -- 785*/
		while(!(I2C1_SR1 & (1<<7))) {}

		/*Place the data to the data register one byte at time*/
		I2C1_DR = *str++;
	}

	// 1) STM32f446xx_Reference - 785, 787
	/*Wait until transfer finished --788*/
	while(!(I2C1_SR1 & (1<<2))) {}

	// 1) STM32f446xx_Reference - 780, 781
	/*Generate stop -- 780*/
	I2C1_CR1 |= (1<<9);
}
