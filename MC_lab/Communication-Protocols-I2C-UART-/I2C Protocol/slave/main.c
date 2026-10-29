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
#define I2C1_OAR1 (*(volatile unsigned int *)(I2C1_BASE + 0x08))


void I2C1_Init();
void I2C1_Read(int n, char* str);

int main(){
	I2C1_Init();
	char str[6];
	while(1){
//I2C1_Send(0x12, 6, "CSE_RU");

		I2C1_Read(6, str);
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

	// set slave address
	I2C1_OAR1 = (0x12<<1);

	//Enable slave address
	I2C1_OAR1 |= (1<<14);

	// Periferal Enable
	I2C1_CR1 |= (1<<0);


}

void I2C1_Read(int n, char* str){


	//Enable acknowlege bit
	I2C1_CR1 |=(1<<10);

	// Wait untill the address bit is set


	while(!(I2C1_SR1 & (1<<1))){}
	(void)I2C1_SR2;
	for(int i=0;i<n;i++){
		while(!(I2C1_SR1 & (1<<6))){}
		str[i] = I2C1_DR;
	}

	I2C1_CR1 &= ~(1<<10);





}
