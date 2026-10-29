/**
 * SPI TWO-WAY SWITCH — SLAVE — Pure Bare Metal — NUCLEO-F446RE
 * Uses SPI2 (PB13/14/15) so the LED (PA5) and button (PC13) stay free.
 *
 * The master drives the bus, so this board just answers each transfer:
 *   'S' from master → the master pressed → my LED blinks 5× → I answer 'D' (ack).
 *   'K' from master → my press was acked → little 2-blink.
 *   Otherwise I load my BUTTON STATE ('B' pressed / 0 released) so the
 *   master can see when I press.
 * Self-check on every byte: if my previous reply is still waiting in the
 * transmit buffer (I am one byte late) or an unknown byte arrives (bit sync
 * lost), I reset SPI2 through RCC and carry on in step.
 * Idle: dark.
 *
 * WIRING (3 wires + GND): PB13(SCK)←PB13, PB15(MOSI)←PB15, PB14(MISO)→PB14, GND↔GND
 * LED: PA5   BUTTON: PC13   LINK: SPI2 slave
 */

#define BLINK_MS  200

#define RCC_AHB1ENR  (*(volatile unsigned int *)0x40023830)
#define RCC_APB1RSTR (*(volatile unsigned int *)0x40023820)
#define RCC_APB1ENR  (*(volatile unsigned int *)0x40023840)
#define GPIOA_MODER  (*(volatile unsigned int *)0x40020000)
#define GPIOA_ODR    (*(volatile unsigned int *)0x40020014)
#define GPIOB_MODER  (*(volatile unsigned int *)0x40020400)
#define GPIOB_PUPDR  (*(volatile unsigned int *)0x4002040C)
#define GPIOB_AFRH   (*(volatile unsigned int *)0x40020424)
#define GPIOC_PUPDR  (*(volatile unsigned int *)0x4002080C)
#define GPIOC_IDR    (*(volatile unsigned int *)0x40020810)
#define SPI2_CR1     (*(volatile unsigned int *)0x40003800)
#define SPI2_SR      (*(volatile unsigned int *)0x40003808)
#define SPI2_DR      (*(volatile unsigned int *)0x4000380C)

void SystemInit(void) __attribute__((weak));        void SystemInit(void) { }
void __libc_init_array(void) __attribute__((weak)); void __libc_init_array(void) { }

static void delay_ms(unsigned int ms) {
    for (unsigned int m = 0; m < ms; m++)
        for (volatile unsigned int i = 0; i < 1300; i++);
}
static int pressed(void) { return (GPIOC_IDR & (1 << 13)) == 0; }
static void blink(int times) {
    for (int i = 0; i < times; i++) {
        GPIOA_ODR |=  (1 << 5);  delay_ms(BLINK_MS);
        GPIOA_ODR &= ~(1 << 5);  delay_ms(BLINK_MS);
    }
}
static void little_blink(void) {
    for (int i = 0; i < 2; i++) {
        GPIOA_ODR |=  (1 << 5);  delay_ms(120);
        GPIOA_ODR &= ~(1 << 5);  delay_ms(120);
    }
}
/* Full SPI2 reset: empties both buffers and restarts the bit counter */
static void spi_reset(void) {
    RCC_APB1RSTR |=  (1 << 14);
    RCC_APB1RSTR &= ~(1 << 14);
    SPI2_CR1 = (1 << 9) | (1 << 6);       /* slave, software NSS selected, enable */
}

int main(void)
{
    RCC_AHB1ENR |= (1 << 0) | (1 << 1) | (1 << 2);   /* GPIOA + GPIOB + GPIOC */
    RCC_APB1ENR |= (1 << 14);                        /* SPI2 */

    GPIOA_MODER &= ~(3 << 10);  GPIOA_MODER |= (1 << 10);          /* PA5 LED */
    GPIOB_MODER &= ~((3 << 26) | (3 << 28) | (3U << 30));          /* PB13/14/15 AF5 */
    GPIOB_MODER |=  ((2 << 26) | (2 << 28) | (2U << 30));
    GPIOB_AFRH  &= ~((0xF << 20) | (0xF << 24) | (0xFU << 28));
    GPIOB_AFRH  |=  ((5 << 20) | (5 << 24) | (5U << 28));
    GPIOB_PUPDR &= ~((3 << 26) | (3U << 30));                      /* SCK + MOSI pull-down: */
    GPIOB_PUPDR |=  ((2 << 26) | (2U << 30));                      /* no fake clocks when master is off */
    GPIOC_PUPDR &= ~(3 << 26);  GPIOC_PUPDR |= (1 << 26);          /* PC13 pull-up */

    spi_reset();                                     /* clean start as slave */

    while (!(SPI2_SR & (1 << 1)));                 /* TXE */
    SPI2_DR = 0x00;                                /* preload: not pressed */

    while (1) {
        while (!(SPI2_SR & (1 << 0)));                 /* RXNE: a transfer came */
        unsigned int  sr  = SPI2_SR;
        unsigned char cmd = (unsigned char)SPI2_DR;
        if (!(sr & (1 << 1))) spi_reset();            /* TXE=0: last reply still queued → I am
                                                         one byte late → reset to get back in step */
        unsigned char reply = pressed() ? 'B' : 0x00;  /* normally: my button state */

        if      (cmd == 'S') { blink(5); reply = 'D'; }    /* master pressed → I blink → ack 'D' */
        else if (cmd == 'K') little_blink();               /* my press acked → little blink    */
        else if (cmd != 0x00) { delay_ms(1); spi_reset(); } /* unknown byte: bit sync lost → reset */

        while (!(SPI2_SR & (1 << 1)));                 /* TXE */
        SPI2_DR = reply;
    }
}
