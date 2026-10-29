#define RCC_BASE (0x40023800)
#define GPIOB_BASE (0x40020400)
#define I2C1_BASE (0x40005400)
#define TIM2_BASE (0x40000000)



#define RCC_AHB1ENR (*(volatile unsigned int*)( RCC_BASE + 0x30))
#define GPIOB_MODER (*(volatile unsigned int*)(GPIOB_BASE + 0))
#define GPIOB_OTYPER (*(volatile unsigned int*)(GPIOB_BASE + 0x04))
#define GPIOB_PUPDR (*(volatile unsigned int*)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRH (*(volatile unsigned int *)(GPIOB_BASE + 0x24))
#define RCC_APB1ENR (*(volatile unsigned int*)(RCC_BASE + 0x40))
#define I2C1_CR1 (*(volatile unsigned int* )(I2C1_BASE + 0))
#define I2C1_CR2 (*(volatile unsigned int*)(I2C1_BASE + 0x04))
#define I2C1_CCR (*(volatile unsigned int*)(I2C1_BASE + 0x1C))
#define I2C1_TRISE (*(volatile unsigned int*)(I2C1_BASE + 0x20))

// DEFINATION for Send Function
#define I2C1_SR2 (*(volatile unsigned int*)(I2C1_BASE + 0x18))
#define I2C1_SR1 (*(volatile unsigned int*)(I2C1_BASE + 0x14))
#define I2C1_DR (*(volatile unsigned int *)(I2C1_BASE + 0x10))

void I2C1_Init();
void I2C1_Send(char saddr, int n, char* str);

int main(){
	I2C1_Init();
	while(1){
		I2C1_Send(0x12, 6, "CSE_RU");
	}
}

void I2C1_Init(){
	//Enable the clock
	RCC_AHB1ENR |= (1<<1);

	// Alternate mode selection
	GPIOB_MODER |=(1<<19);
	GPIOB_MODER |=(1<<17);
	GPIOB_MODER &=~(1<<18);
	GPIOB_MODER &=~(1<<16);

	// Output typer = Open drain
	GPIOB_OTYPER |=(1<<8);
	GPIOB_OTYPER |=(1<<9);

	// Make Pull up of for the SCL and SDA line
	GPIOB_PUPDR |= (1<<18);
	GPIOB_PUPDR |= (1<<16);
	GPIOB_PUPDR &= ~(1<<17);
	GPIOB_PUPDR &= ~(1<<19);

	// Make sure you use the I2C1 as SDA and SCL line
	GPIOB_AFRH &= ~(0xFF<<0);
	GPIOB_AFRH |= (1<<2);
	GPIOB_AFRH |= (1<<6);

	// Clock Enable for APB1
	RCC_APB1ENR|=(1<<21);

	//Reset the I2C_CR1 register
	I2C1_CR1 |= (1<<15);
	// Return it at the previous stare
	I2C1_CR1 &= ~(1<<15);

	//Make the PCLK = 16 MHz
	I2C1_CR2 |=(1<<4);
	// Set CCR = 80 and
	I2C1_CCR = 80;

	//SET TRISE = 17 and get the output frequency = 100KHz
	I2C1_TRISE = 17;

	// Periferal Enable
	I2C1_CR1 |= (1<<0);


}

void I2C1_Send(char saddr, int n, char* str){


	// Check whether the bus is busy or free
	while(I2C1_SR2 & (1<<1)){}

	// Generate the start bit
	I2C1_CR1 |=(1<<8);

	// Wait untill the start bit is generated
	while(!(I2C1_SR1 & (1<<0))){}

	//Send the address
	I2C1_DR = (saddr<<1);

	// wait untill the address is matched
	while(!(I2C1_SR1 & (1<<1))){}

	(void)I2C1_SR2;

	for(int i=0;i<n;i++){
		while(!(I2C1_SR1 & (1<<7))){}
		I2C1_DR = *str++;
	}

	while(!(I2C1_SR1 & (1<<2))){}
	I2C1_CR1 |=(1<<9);









}
