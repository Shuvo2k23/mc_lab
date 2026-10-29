// Master SPI Communication Protocol (STM32F446RE, Nucleo-64, register level)
//
// ===================== WIRE CONNECTION (Master board <-> Slave board) =====================
//
//   Master board (SPI1)            Slave board (SPI1)
//   -------------------            ------------------
//   PA5  (SCK,  D13)   --------->  PA5  (SCK,  D13)
//   PA7  (MOSI, D11)   --------->  PA7  (MOSI, D11)
//   PA6  (MISO, D12)   <---------  PA6  (MISO, D12)
//   PA4  (CS/NSS, A2)  --------->  PA4  (NSS,  A2)
//   GND                ---------   GND      <-- common ground is a MUST
//
//   - Logic level is 3.3 V on both boards. Do NOT connect the 3.3V/5V pins together.
//   - Keep wires short (< 20 cm) at 1 MHz.
//   - PA5 is also the on-board LED (LD2); it will flicker with the clock. That is normal.
//
// ===================== HOW TO SEE DATA IN DEBUG MODE =====================
//   The received data is on the SLAVE board (see spi_slave.c).
//   Debug the slave, add  rx_buf  and  frame_count  to "Live Expressions"
//   (STM32CubeIDE) or the Watch window (Keil). rx_buf should read "CSE-RU".
//
// SPI Mode 0 (CPOL=0, CPHA=0), 8-bit, MSB first, 1 MHz (16 MHz HSI / 16)

// List of Register Declarations
#define RCC_BASE   (0x40023800)
#define GPIOA_BASE (0x40020000)
#define SPI1_BASE  (0x40013000)

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44))

#define GPIOA_MODER   (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_OSPEEDR (*(volatile unsigned int *)(GPIOA_BASE + 0x08))
#define GPIOA_ODR     (*(volatile unsigned int *)(GPIOA_BASE + 0x14))
#define GPIOA_AFRL    (*(volatile unsigned int *)(GPIOA_BASE + 0x20))

#define SPI1_CR1 (*(volatile unsigned int *)(SPI1_BASE + 0x00))
#define SPI1_CR2 (*(volatile unsigned int *)(SPI1_BASE + 0x04))
#define SPI1_SR  (*(volatile unsigned int *)(SPI1_BASE + 0x08))
#define SPI1_DR  (*(volatile unsigned int *)(SPI1_BASE + 0x0C))

#define CS_LOW()  (GPIOA_ODR &= ~(1<<4))
#define CS_HIGH() (GPIOA_ODR |=  (1<<4))

void SPI1_Init(void);
void SPI1_Send(int n, const char *str);
void delay(volatile int t);

int main(void) {
	SPI1_Init();
	while(1) {
		SPI1_Send(6, "CSE-RU");
		delay(200000);          // gap between frames
	}
}

void SPI1_Init(void) {

	/*Enable clock of GPIOA through AHB1ENR*/
	RCC_AHB1ENR |= (1<<0);

	/*PA4 = CS, driven by software as general purpose output (01), idle HIGH*/
	CS_HIGH();
	GPIOA_MODER &= ~(3<<8);
	GPIOA_MODER |=  (1<<8);

	/*PA5 (SCK), PA6 (MISO), PA7 (MOSI) -> alternate function (10)*/
	GPIOA_MODER &= ~((3<<10) | (3<<12) | (3<<14));
	GPIOA_MODER |=  ((2<<10) | (2<<12) | (2<<14));

	/*High speed for PA5, PA6, PA7 (10)*/
	GPIOA_OSPEEDR |= (2<<10) | (2<<12) | (2<<14);

	/*AF5 (SPI1) for PA5, PA6, PA7 -- AFRL bits [23:20], [27:24], [31:28]*/
	GPIOA_AFRL &= ~((0xF<<20) | (0xF<<24) | (0xF<<28));
	GPIOA_AFRL |=  ((5<<20)   | (5<<24)   | (5<<28));

	/*Enable clock of SPI1 (APB2ENR bit 12)*/
	RCC_APB2ENR |= (1<<12);

	/*CR1 configuration (SPI disabled while configuring)
	  BR[5:3] = 011 -> fPCLK2/16 = 1 MHz
	  MSTR (2) = 1 master
	  SSM (9) = 1, SSI (8) = 1 -> software slave management, NSS internally high
	  CPOL (1) = 0, CPHA (0) = 0, DFF (11) = 0 (8 bit), LSBFIRST (7) = 0 (MSB first)*/
	SPI1_CR1 = (3<<3) | (1<<2) | (1<<9) | (1<<8);

	/*Enable SPI1 -- must be last*/
	SPI1_CR1 |= (1<<6);
}

void SPI1_Send(int n, const char *str) {

	/*Select slave*/
	CS_LOW();

	for(int i = 0; i < n; i++) {

		/*Wait until transmit buffer is empty (TXE)*/
		while(!(SPI1_SR & (1<<1))) {}

		/*Write data, this starts the clock*/
		SPI1_DR = *str++;

		/*Wait until a byte is received (RXNE) and discard it (full duplex)*/
		while(!(SPI1_SR & (1<<0))) {}
		(void)SPI1_DR;
	}

	/*Wait until last transfer is finished (BSY = 0)*/
	while(SPI1_SR & (1<<7)) {}

	/*Deselect slave*/
	CS_HIGH();
}

void delay(volatile int t) {
	while(t--) {}
}
