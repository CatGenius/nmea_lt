//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "uart1.h"
#include "uart2.h"
#include "mkdst.h"
#include "cmdline.h"
#include "nmea.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define MINUTES_PER_HOUR        (60)
#define SECONDS_PER_MINUTES     (60)
#define SECONDS_PER_HOUR        (MINUTES_PER_HOUR * SECONDS_PER_MINUTES)
#define DST_OFFSET_S            (1 * SECONDS_PER_HOUR)
#define LT_OFFSET_S             (1 * SECONDS_PER_HOUR)

#define TIME_ZONE               (1)
#define TIME_ZONE_M             (TIME_ZONE * MINUTES_PER_HOUR)

//#define TEST_MKDST
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))


/******************************************************************************/
/* Globals                                                                    */
/******************************************************************************/
const struct command_t  commands[] = {
	{"?",     cmdline_help},
	{"help",  cmdline_help},
	{"echo",  cmdline_echo},
	{NULL,    NULL}
};

static void handle_gprmc(int argc, char *argv[]);
const struct nmea_t     nmea[] = {
	{"GPRMC", handle_gprmc},
	{NULL,    NULL}
};


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
static void disable_peripherals(void)
{
/*
	PMD0bits.IOCMD   = 0;
	PMD0bits.CLKRMD  = 0;
	PMD0bits.NVMMD   = 0;
	PMD0bits.FVRMD   = 0;
	PMD0bits.SYSCMD  = 0;

	PMD1bits.TMR0MD  = 0;
	PMD1bits.TMR1MD  = 0;
	PMD1bits.TMR2MD  = 0;
	PMD1bits.DDS1MD  = 0;
*/
	PMD2bits.ZCDMD   = 0;
	PMD2bits.CMP1MD  = 0;
	PMD2bits.CMP2MD  = 0;
	PMD2bits.ADCMD   = 0;
	PMD2bits.DAC1MD  = 0;

	PMD3bits.CCP1MD  = 0;
	PMD3bits.CCP2MD  = 0;
	PMD3bits.PWM3MD  = 0;
	PMD3bits.PWM4MD  = 0;
	PMD3bits.PWM5MD  = 0;
	PMD3bits.PWM6MD  = 0;

	PMD4bits.CWG1MD  = 0;
	PMD4bits.MSSP1MD = 0;
//	PMD4bits.UART1MD = 1;
//	PMD4bits.UART2MD = 1;

	PMD5bits.CLC1MD  = 0;
	PMD5bits.CLC2MD  = 0;
	PMD5bits.CLC3MD  = 0;
	PMD5bits.CLC4MD  = 0;
}


static void init_clocks(void)
{
	/* Select HFINTOSC, divide by 1*/
	OSCCON1bits.NDIV = 0;
	OSCCON1bits.NOSC = 6;

	/* Clock switch may proceed when the oscillator selected by NOSC is ready may proceed. */
	/* Secondary oscillator operating in Low-power mode */
	OSCCON3 = 0x00;

	/* Disable EXTOEN, HFOEN. MFOEN, LFOEN, SOSCEN and ADOEN */
	OSCEN = 0x00;

	/* Set HFINTOSC to 32MHz */
	OSCFRQbits.HFFRQ = 0x06;

	/* Reset oscillator tuning */
	OSCTUNEbits.HFTUN = 0x00;
}


static void init_pins(void)
{
	/* Disable analog input on pins used digitally */
	ANSELC &= ~((1 << 1) |  /* Pin RC1 */
	            (1 << 5));  /* Pin RC5 */

	/* Unlock Peripheral Pin Select module (PPS) */
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 0;

	/* UART1 */
	RX1DTPPS = 0x15;  /* Connect RX1 to RC5 input pin */
	RC4PPS   = 0x0f;  /* Connect RC4 output to TX1 */

	/* UART2 */
	RX2DTPPS = 0x11;  /* Connect RX2 to RC1 input pin */
	RC0PPS   = 0x11;  /* Connect RC0 output pin to TX2 */

	/* Lock Peripheral Pin Select module (PPS) */
	PPSLOCK = 0x55;
	PPSLOCK = 0xAA;
	PPSLOCKbits.PPSLOCKED = 1;
}


static void init_interrupt(void)
{
	/* Enable peripheral interrupts */
	PEIE = 1;

	/* Enable interrupts */
	GIE = 1;
}


static void handle_gprmc(int argc, char *argv[])
{
	char           *time = argv[1];
	char           *date = argv[9];
	unsigned long  value;
	char           *endptr;
	struct tm      utc;
	struct tm      local;
	time_t         utc_secs;
	time_t         local_secs;

	value = strtoul(&time[4], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_sec = value;
	time[4] = '\0';

	value = strtoul(&time[2], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_min = value;
	time[2] = '\0';

	value = strtoul(&time[0], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_hour = value;

	value = strtoul(&date[4], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_year = value + 100;
	date[4] = '\0';

	value = strtoul(&date[2], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_mon = value - 1;
	date[2] = '\0';

	value = strtoul(&date[0], &endptr, 10);
	if (*endptr != '\0')
		return;
	utc.tm_mday = value;

	/* Convert broken-down UTC time to seconds and complete with weekday */
	/* (Caveat: XC8 mktime) does NOT fill out tm_wday or tm_yday */
	utc_secs = mktime(&utc);
//	if ()

	/* Set the DST flag (tzset() is not supported) */
	mkdst(utc_secs, &local);

	/* Add local time offset and daylight saving time to UTC to get local time */
	local_secs = utc_secs +
	             LT_OFFSET_S +
	             ((local.tm_isdst > 0) ? DST_OFFSET_S : 0);

//	printf("time = %d:%d:%d, date = %d/%d/%d\n", utc.tm_hour, utc.tm_min, utc.tm_sec, utc.tm_mday, utc.tm_mon, utc.tm_year);
	printf("time = %ld\n", local_secs);
}


static void interrupt isr(void)
{
	/* Timer 1 interrupt */
	if (TMR1IF) {
		/* Reset interrupt */
		TMR1IF = 0;
		/* Handle interrupt */
//		timer_isr();
	}

	/* (E)USART 1 interrupts */
	if (RC1IF)
		uart1_rx_isr();
	if (TX1IF)
		uart1_tx_isr();

	/* (E)USART 2 interrupts */
	if (RC2IF)
		uart2_rx_isr();
	if (TX2IF)
		uart2_tx_isr();
}


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
void main(void)
{
	disable_peripherals();
	init_clocks();
	init_pins();

	/* Initialize the serial port for stdio */
	uart2_init(115200, 0);
	uart1_init(4800, 0);

	printf("\n*** NMEA local time converter ***\n");
	if (!nPOR)
		printf("Power-on reset\n");
	else if (!nBOR)
		printf("Brown-out reset\n");
	else if (!__timeout)
		printf("Watchdog reset\n");
	else if (!__powerdown)
		printf("Pin reset (sleep)\n");
	else
		printf("Pin reset\n");
	nPOR = 1;
	nBOR = 1;

#ifdef TEST_MKDST
	test_mkdst();
#endif /* TEST_MKDST */

	cmdline_init();

	/* Initialize interrupts */
	init_interrupt();

	/* Execute the run loop */
	for(;;) {
		cmdline_work();
		nmea_work();
		CLRWDT();
	}
}
