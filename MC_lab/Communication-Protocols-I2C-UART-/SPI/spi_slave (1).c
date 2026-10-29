// Slave SPI Communication Protocol (STM32F446RE, Nucleo-64, register level)
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
//   - Power the master first (or both together) so CS is idle HIGH before the slave starts.
//
// ===================== HOW TO SEE DATA IN DEBUG MODE =====================
//   1) Flash spi_master.c on the master board and let it run.
//   2) Flash/debug this file on the slave board.
//   3) Add these to "Live Expressions" (STM32CubeIDE) / Watch window (Keil):
//         rx_buf        -> should show "CSE-RU"
//         frame_count   -> increases by 1 for every received frame
//   4) Or put a breakpoint on the  frame_count++  line and inspect rx_buf.
//
// SPI Mode 0 (CPOL=0, CPHA=0), 8-bit, MSB first. Slave uses hardware NSS (PA4).

// List of Register Declarations
#define RCC_BASE   (0x40023800)
#define GPIOA_BASE (0x40020000)
#define SPI1_BASE  (0x40013000)

#define RCC_AHB1ENR (*(volatile unsigned int *)(RCC_BASE + 0x30))
#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x44))

#define GPIOA_MODER   (*(volatile unsigned int *)(GPIOA_BASE + 0x00))
#define GPIOA_OSPEEDR (*(volatile unsigned int *)(GPIOA_BASE + 0x08))
#define GPIOA_IDR     (*(volatile unsigned int *)(GPIOA_BASE + 0x10))
#define GPIOA_AFRL    (*(volatile unsigned int *)(GPIOA_BASE + 0x20))

#define SPI1_CR1 (*(volatile unsigned int *)(SPI1_BASE + 0x00))
#define SPI1_CR2 (*(volatile unsigned int *)(SPI1_BASE + 0x04))
#define SPI1_SR  (*(volatile unsigned int *)(SPI1_BASE + 0x08))
#define SPI1_DR  (*(volatile unsigned int *)(SPI1_BASE + 0x0C))

#define NSS_IS_LOW() (!(GPIOA_IDR & (1<<4)))

// Global + volatile so the debugger can always show the values
volatile char         rx_buf[7];      // 6 data bytes + '\0' so it reads as a string
volatile unsigned int frame_count = 0;
volatile unsigned int last_sr = 0;    // SPI status after each frame (bit6 = OVR should be 0)

void SPI1_Init(void);
void SPI1_Read(int n, volatile char *str);

int main(void) {
	SPI1_Init();
	while(1) {
		SPI1_Read(6, rx_buf);
		rx_buf[6] = '\0';
		frame_count++;        // <-- put a breakpoint here to inspect rx_buf
	}
}

void SPI1_Init(void) {

	/*Enable clock of GPIOA through AHB1ENR*/
	RCC_AHB1ENR |= (1<<0);

	/*PA4 (NSS), PA5 (SCK), PA6 (MISO), PA7 (MOSI) -> alternate function (10)*/
	GPIOA_MODER &= ~(0xFF<<8);
	GPIOA_MODER |=  ((2<<8) | (2<<10) | (2<<12) | (2<<14));

	/*High speed for PA6 (MISO output)*/
	GPIOA_OSPEEDR |= (2<<12);

	/*AF5 (SPI1) for PA4..PA7 -- AFRL bits [31:16]*/
	GPIOA_AFRL &= ~(0xFFFF<<16);
	GPIOA_AFRL |=  ((5<<16) | (5<<20) | (5<<24) | (5<<28));

	/*Enable clock of SPI1 (APB2ENR bit 12)*/
	RCC_APB2ENR |= (1<<12);

	/*CR1 configuration (SPI disabled while configuring)
	  MSTR (2) = 0 -> slave
	  SSM (9) = 0 -> hardware NSS from PA4 (frame boundaries are detected correctly)
	  CPOL (1) = 0, CPHA (0) = 0, DFF (11) = 0 (8 bit), LSBFIRST (7) = 0 (MSB first)
	  Baud rate bits are ignored in slave mode*/
	SPI1_CR1 = 0;

	/*Preload dummy byte so MISO has something to shift out*/
	SPI1_DR = 0x00;

	/*Enable SPI1 -- must be last*/
	SPI1_CR1 |= (1<<6);
}

void SPI1_Read(int n, volatile char *str) {

	/*Wait until CS is HIGH (bus idle). After a breakpoint/halt this throws away
	  the half-finished frame, so we always start at the first byte of a frame*/
	while(NSS_IS_LOW()) {}

	/*Clear overrun flag (OVR): read DR first, then read SR -- this also flushes RXNE.
	  If OVR stays set, SPI keeps returning the OLD byte (e.g. "CCCCCC")*/
	(void)SPI1_DR;
	(void)SPI1_SR;

	/*Wait for master to pull CS low (start of frame)*/
	while(!NSS_IS_LOW()) {}

	for(int i = 0; i < n; i++) {

		/*Wait until a byte is received (RXNE)*/
		while(!(SPI1_SR & (1<<0))) {}

		/*Read received data (also clears RXNE)*/
		str[i] = SPI1_DR;
	}

	/*Keep last status for debugging: bit6 = OVR, bit0 = RXNE*/
	last_sr = SPI1_SR;
}
