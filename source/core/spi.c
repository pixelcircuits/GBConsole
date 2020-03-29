//based on the bcm2835 library http://www.airspayce.com/mikem/bcm2835/index.html
//and the BCM2835 ARM Peripherals datasheet
#include "spi.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h> 
#include <sys/mman.h>

/// Size of memory page/block on RPi
#define PAGE_SIZE               (4*1024)
#define BLOCK_SIZE              (4*1024)

/// Base Physical Address of the BCM 2835 peripheral registers
#define BCM2835_PERI_BASE               0x20000000
#define BCM2835_GPIO_BASE               (BCM2835_PERI_BASE + 0x200000)
#define BCM2835_SPI0_BASE               (BCM2835_PERI_BASE + 0x204000)

/// GPIO register offsets from BCM2835_GPIO_BASE. Offsets into the GPIO Peripheral block in bytes per 6.1 Register View
#define BCM2835_GPFSEL0                      0x0000 ///< GPIO Function Select 0
#define BCM2835_GPIO_FSEL_MASK               0b111  ///< Function select bits mask
#define BCM2835_GPSET0                       0x001c ///< GPIO Pin Output Set 0
#define BCM2835_GPCLR0                       0x0028 ///< GPIO Pin Output Clear 0

/// GPIO function select
#define BCM2835_GPIO_FSEL_INPT               0b000 ///< Input
#define BCM2835_GPIO_FSEL_OUTP               0b001 ///< Output
#define BCM2835_GPIO_FSEL_ALT0               0b100 ///< Alternate function 0
#define BCM2835_GPIO_FSEL_ALT1               0b101 ///< Alternate function 1
#define BCM2835_GPIO_FSEL_ALT2               0b110 ///< Alternate function 2
#define BCM2835_GPIO_FSEL_ALT3               0b111 ///< Alternate function 3
#define BCM2835_GPIO_FSEL_ALT4               0b011 ///< Alternate function 4
#define BCM2835_GPIO_FSEL_ALT5               0b010 ///< Alternate function 5
#define BCM2835_GPIO_FSEL_MASK               0b111 ///< Function select bits mask

/// Offsets into the SPI Peripheral block in bytes per 10.5 SPI Register Map
#define BCM2835_SPI0_CS                      0x0000 ///< SPI Master Control and Status
#define BCM2835_SPI0_FIFO                    0x0004 ///< SPI Master TX and RX FIFOs
#define BCM2835_SPI0_CLK                     0x0008 ///< SPI Master Clock Divider
#define BCM2835_SPI0_DLEN                    0x000c ///< SPI Master Data Length
#define BCM2835_SPI0_LTOH                    0x0010 ///< SPI LOSSI mode TOH
#define BCM2835_SPI0_DC                      0x0014 ///< SPI DMA DREQ Controls

/// Register masks for SPI0_CS
#define BCM2835_SPI0_CS_LEN_LONG             0x02000000 ///< Enable Long data word in Lossi mode if DMA_LEN is set
#define BCM2835_SPI0_CS_DMA_LEN              0x01000000 ///< Enable DMA mode in Lossi mode
#define BCM2835_SPI0_CS_CSPOL2               0x00800000 ///< Chip Select 2 Polarity
#define BCM2835_SPI0_CS_CSPOL1               0x00400000 ///< Chip Select 1 Polarity
#define BCM2835_SPI0_CS_CSPOL0               0x00200000 ///< Chip Select 0 Polarity
#define BCM2835_SPI0_CS_RXF                  0x00100000 ///< RXF - RX FIFO Full
#define BCM2835_SPI0_CS_RXR                  0x00080000 ///< RXR RX FIFO needs Reading ( full)
#define BCM2835_SPI0_CS_TXD                  0x00040000 ///< TXD TX FIFO can accept Data
#define BCM2835_SPI0_CS_RXD                  0x00020000 ///< RXD RX FIFO contains Data
#define BCM2835_SPI0_CS_DONE                 0x00010000 ///< Done transfer Done
#define BCM2835_SPI0_CS_TE_EN                0x00008000 ///< Unused
#define BCM2835_SPI0_CS_LMONO                0x00004000 ///< Unused
#define BCM2835_SPI0_CS_LEN                  0x00002000 ///< LEN LoSSI enable
#define BCM2835_SPI0_CS_REN                  0x00001000 ///< REN Read Enable
#define BCM2835_SPI0_CS_ADCS                 0x00000800 ///< ADCS Automatically Deassert Chip Select
#define BCM2835_SPI0_CS_INTR                 0x00000400 ///< INTR Interrupt on RXR
#define BCM2835_SPI0_CS_INTD                 0x00000200 ///< INTD Interrupt on Done
#define BCM2835_SPI0_CS_DMAEN                0x00000100 ///< DMAEN DMA Enable
#define BCM2835_SPI0_CS_TA                   0x00000080 ///< Transfer Active
#define BCM2835_SPI0_CS_CSPOL                0x00000040 ///< Chip Select Polarity
#define BCM2835_SPI0_CS_CLEAR                0x00000030 ///< Clear FIFO Clear RX and TX
#define BCM2835_SPI0_CS_CLEAR_RX             0x00000020 ///< Clear FIFO Clear RX 
#define BCM2835_SPI0_CS_CLEAR_TX             0x00000010 ///< Clear FIFO Clear TX 
#define BCM2835_SPI0_CS_CPOL                 0x00000008 ///< Clock Polarity
#define BCM2835_SPI0_CS_CPHA                 0x00000004 ///< Clock Phase
#define BCM2835_SPI0_CS_CS                   0x00000003 ///< Chip Select

// Locals to hold pointers to the hardware
static volatile uint32_t *spi_gpio = (uint32_t*)MAP_FAILED;
static volatile uint32_t *spi_spi0 = (uint32_t*)MAP_FAILED;
static int spi_fd = -1;
static uint8_t *spi_gpioMem = NULL;
static uint8_t *spi_spi0Mem = NULL;
static uint32_t spi_lockKey = 0; 

// Helper functions
static void spi_peri_write(volatile uint32_t* paddr, uint32_t value);
static void spi_peri_write_nb(volatile uint32_t* paddr, uint32_t value);
static uint32_t spi_peri_read(volatile uint32_t* paddr);
static uint32_t spi_peri_read_nb(volatile uint32_t* paddr);
static void spi_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask);
static void spi_gpio_fsel(uint8_t pin, uint8_t mode);
static void spi_sleep(uint32_t usec);

// Initialise and begin the SPI peripheral
int spi_init(uint32_t clockSpeedHz)
{
	uint16_t clockDivider = 400000000 / clockSpeedHz;
	uint8_t *mapaddr;
	
	// Already initialized?
	if(spi_fd != -1) return 0;

	// Open the master /dev/memory device
	if((spi_fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
		fprintf(stderr, "spi_init: Unable to open /dev/mem: %s\n", strerror(errno));
		spi_close();
		return 1;
	}

	// GPIO
	if((spi_gpioMem = (uint8_t*)malloc(BLOCK_SIZE + (PAGE_SIZE - 1))) == NULL) {
		fprintf(stderr, "spi_init: GPIO malloc failed: %s\n", strerror(errno));
		spi_close();
		return 1;
	}
	mapaddr = spi_gpioMem;
	if(((uint32_t)mapaddr % PAGE_SIZE) != 0) mapaddr += PAGE_SIZE - ((uint32_t)mapaddr % PAGE_SIZE);
	spi_gpio = (uint32_t *)mmap(mapaddr, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, spi_fd, BCM2835_GPIO_BASE);
	if((int32_t)spi_gpio < 0) {
		fprintf(stderr, "spi_init: GPIO mmap failed: %s\n", strerror(errno));
		spi_close();
		return 1;
	}

	// SPI0
	if((spi_spi0Mem = (uint8_t*)malloc(BLOCK_SIZE + (PAGE_SIZE - 1))) == NULL) {
		fprintf(stderr, "spi_init: SPI0 malloc failed: %s\n", strerror(errno));
		spi_close();
		return 1;
	}
	mapaddr = spi_spi0Mem;
	if(((uint32_t)mapaddr % PAGE_SIZE) != 0) mapaddr += PAGE_SIZE - ((uint32_t)mapaddr % PAGE_SIZE);
	spi_spi0 = (uint32_t *)mmap(mapaddr, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, spi_fd, BCM2835_SPI0_BASE);
	if((int32_t)spi_spi0 < 0) {
		fprintf(stderr, "spi_init: SPI0 mmap failed: %s\n", strerror(errno));
		spi_close();
		return 1;
	}

	// Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
	spi_gpio_fsel(8, BCM2835_GPIO_FSEL_ALT0); // CE0
	spi_gpio_fsel(9, BCM2835_GPIO_FSEL_ALT0); // MISO
	spi_gpio_fsel(10, BCM2835_GPIO_FSEL_ALT0); // MOSI
	spi_gpio_fsel(11, BCM2835_GPIO_FSEL_ALT0); // CLK

	// Set the SPI CS register
	volatile uint32_t* paddr_cs = spi_spi0 + BCM2835_SPI0_CS/4;
	spi_peri_write(paddr_cs, 0);

	// Set the SPI CLK register
	volatile uint32_t* paddr_clk = spi_spi0 + BCM2835_SPI0_CLK/4;
	spi_peri_write(paddr_clk, clockDivider);

	// Clear TX and RX fifos
	spi_peri_write_nb(paddr_cs, BCM2835_SPI0_CS_CLEAR);

	return 0;
}

// Checks if the SPI interface is initialized
uint8_t spi_isInit()
{
	if(spi_fd != -1) return 1;
	return 0;
}

// Enables the given pin as an additional CS pin
void spi_enableSelectPin(uint8_t pin)
{
	spi_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
}

// Sets the state of the given additional CS pin
void spi_setSelectPin(uint8_t pin, uint8_t on)
{
    if (on) {
		volatile uint32_t* paddr = spi_gpio + BCM2835_GPSET0/4 + pin/32;
		spi_peri_write(paddr, 1 << pin);
    } else {
		volatile uint32_t* paddr = spi_gpio + BCM2835_GPCLR0/4 + pin/32;
		spi_peri_write(paddr, 1 << pin);
	}
}

// Disables the given pin as an additional CS pin
void spi_disableSelectPin(uint8_t pin)
{
	spi_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
}

// Locks the SPI interface from use in other threads
void spi_obtainLock(uint32_t key, uint8_t disableCS)
{
	if(spi_lockKey != key) {
		while(spi_lockKey != 0) spi_sleep(100*1000);
		spi_lockKey = key;
	}
	
	if(disableCS > 0) {
		spi_gpio_fsel(8, BCM2835_GPIO_FSEL_OUTP);
		volatile uint32_t* paddr = spi_gpio + BCM2835_GPSET0/4 + 8/32;
		spi_peri_write(paddr, 1 << 8);
	}
}

// Unlocks the SPI interface for use in other threads
void spi_unlock(uint32_t key)
{
	if(spi_lockKey == key) {
		spi_gpio_fsel(8, BCM2835_GPIO_FSEL_ALT0);
		spi_lockKey = 0;
	}
}

// Writes (and reads) an number of bytes to SPI
void spi_transfer(uint8_t* buf, uint32_t len)
{
	// This is Polled transfer as per section 10.6.1
	volatile uint32_t* paddr = spi_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = spi_spi0 + BCM2835_SPI0_FIFO/4;

	// Clear TX and RX fifos
	spi_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

	// Set TA = 1
	spi_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	uint32_t i;
	for(i = 0; i < len; i++)
	{
		// Maybe wait for TXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_TXD));

		// Write to FIFO, no barrier
		spi_peri_write_nb(fifo, buf[i]);

		// Wait for RXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_RXD));

		// then read the data byte
		buf[i] = spi_peri_read_nb(fifo);
	}
	
	// Wait for DONE to be set
	while(!(spi_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE));

	// Set TA = 0, and also set the barrier
	spi_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

// Starts a long read operation by writing the given bytes to SPI
void spi_read_start(uint8_t* buf, uint32_t len)
{
	volatile uint32_t* paddr = spi_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = spi_spi0 + BCM2835_SPI0_FIFO/4;

	// Clear TX and RX fifos
	spi_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);

	// Set TA = 1
	spi_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);

	uint32_t i;
	for(i = 0; i < len; i++)
	{
		// Wait for TXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_TXD));

		// Write to FIFO, no barrier
		spi_peri_write_nb(fifo, buf[i]);

		// Wait for RXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_RXD));

		// then read the data byte
		spi_peri_read_nb(fifo);
	}
	
	// Wait for DONE to be set
	while(!(spi_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE));	
}

// Reads a single byte from SPI (continuation for long read)
uint8_t spi_read_cont()
{
	volatile uint32_t* paddr = spi_spi0 + BCM2835_SPI0_CS/4;
	volatile uint32_t* fifo = spi_spi0 + BCM2835_SPI0_FIFO/4;

	// Write to FIFO, no barrier
	spi_peri_write_nb(fifo, 0x00);

	// Wait for RXD
	while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_RXD));

	// then read the data byte
	return (uint8_t)spi_peri_read_nb(fifo);
}

// Ends a long read operation by writing the given bytes to SPI
void spi_read_end(uint8_t* buf, uint32_t len)
{
    volatile uint32_t* paddr = spi_spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = spi_spi0 + BCM2835_SPI0_FIFO/4;
	
	uint32_t i;
	for(i = 0; i < len; i++)
	{
		// Wait for TXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_TXD));

		// Write to FIFO, no barrier
		spi_peri_write_nb(fifo, buf[i]);

		// Wait for RXD
		while(!(spi_peri_read(paddr) & BCM2835_SPI0_CS_RXD));

		// then read the data byte
		spi_peri_read_nb(fifo);
	}
	
	// Wait for DONE to be set
	while(!(spi_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE));

	// Set TA = 0, and also set the barrier
	spi_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

// Closes the SPI interface
int spi_close()
{  
	// Free memory
	if(spi_gpio != MAP_FAILED) {
		//set all the SPI0 pins back to input
		spi_gpio_fsel(8, BCM2835_GPIO_FSEL_INPT); // CE0
		spi_gpio_fsel(9, BCM2835_GPIO_FSEL_INPT); // MISO
		spi_gpio_fsel(10, BCM2835_GPIO_FSEL_INPT); // MOSI
		spi_gpio_fsel(11, BCM2835_GPIO_FSEL_INPT); // CLK
		
	    munmap((void*)spi_gpio, BLOCK_SIZE);
	    spi_gpio = (uint32_t*)MAP_FAILED;
	}
	if(spi_gpioMem) {
	    free(spi_gpioMem);
	    spi_gpioMem = NULL;
	}
	if(spi_spi0 != MAP_FAILED) {
	    munmap((void*)spi_spi0, BLOCK_SIZE);
	    spi_spi0 = (uint32_t*)MAP_FAILED;
	}
	if(spi_spi0Mem) {
	    free(spi_spi0Mem);
	    spi_spi0Mem = NULL;
	}
	if(spi_fd >= 0) {
	    close(spi_fd);
	    spi_fd = -1;
	}
	
    return 0;
}    

// Helper function definitions
static void spi_peri_write(volatile uint32_t* paddr, uint32_t value) {
	*paddr = value;
	*paddr = value;
}
static void spi_peri_write_nb(volatile uint32_t* paddr, uint32_t value) {
	*paddr = value;
}
static uint32_t spi_peri_read(volatile uint32_t* paddr) {
	uint32_t ret = *paddr;
	ret = *paddr;
	return ret;
}
static uint32_t spi_peri_read_nb(volatile uint32_t* paddr) {
	return *paddr;
}
static void spi_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask) {
    uint32_t v = spi_peri_read(paddr);
    v = (v & ~mask) | (value & mask);
    spi_peri_write(paddr, v);
}
static void spi_gpio_fsel(uint8_t pin, uint8_t mode) {
    // Function selects are 10 pins per 32 bit word, 3 bits per pin
    volatile uint32_t* paddr = spi_gpio + BCM2835_GPFSEL0/4 + (pin/10);
    uint8_t   shift = (pin % 10) * 3;
    uint32_t  mask = BCM2835_GPIO_FSEL_MASK << shift;
    uint32_t  value = mode << shift;
    spi_peri_set_bits(paddr, value, mask);
}
static void spi_sleep(uint32_t usec) {
    struct timespec ts;
    ts.tv_sec = usec / 1000000;
    ts.tv_nsec = (usec % 1000000) * 1000;
	nanosleep(&ts, &ts);
}
