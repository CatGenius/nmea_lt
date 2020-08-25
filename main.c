//#include "configbits.h"		/* PIC MCU configuration bits, include before anything else */
#include <xc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uart1.h"
#include "uart2.h"
#include "rtc.h"
#include "cmdline.h"
#include "nmea.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define DST_OFFSET_S            (1 * SECONDS_PER_HOUR)
#define LT_OFFSET_S             (1 * SECONDS_PER_HOUR)

#define TIME_ZONE               (1)
#define TIME_ZONE_M             (TIME_ZONE * MINUTES_PER_HOUR)

//#define TEST_DST
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))


/******************************************************************************/
/* Global Data                                                                */
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
static void update_rtc(rtcsecs_t utc)
{
	static rtcsecs_t  last_set = 0;

	printf("Time = %lu", utc);

	if (last_set)
		printf(", diff = %lu", utc - last_set);

	printf("\n");

	last_set = utc;
}


static int get_octets(char *str, unsigned char *octet[])
{
	int  ndx;

	/* Walk the given string, back to front, two digits at a time */
	for (ndx = 4; ndx >= 0; ndx -= 2) {
		char           *endptr;
		unsigned long  value = strtoul(&str[ndx], &endptr, 10);  /* Convert this part of the string to a numerical value */

		/* Test for trailing garbage */
		if (*endptr != '\0') {
			printf("Error converting %s to a number\n", &str[ndx]);
			return -1;
		}

		/* Copy the numerical value into the corresponding octet */
		*octet[ndx >> 1] = (unsigned char)value;

		/* Overwrite the current part of the sting with a 0-terminator, so after we move back to the front, the string stops here */
		str[ndx] = '\0';
	}

	return 0;
}


static void handle_gprmc(int argc, char *argv[])
{
	struct rtctime_t  utc;
	struct rtctime_t  local;
	rtcsecs_t         utc_secs;
	rtcsecs_t         local_secs;
	unsigned char     *octet[3];
	int               ndx;

	/* Check validity mark */
	if (strcmp(argv[2], "A"))
		return;

	/* Get the 3 octets holding the time from the time argument */
	octet[0] = &utc.hour;
	octet[1] = &utc.min;
	octet[2] = &utc.sec;
	if (get_octets(argv[1], octet))
		return;

	/* Get the 3 octets holding the date from the date argument */
	octet[0] = &utc.day;
	octet[1] = &utc.mon;
	octet[2] = &utc.year;
	if (get_octets(argv[9], octet))
		return;

	/* Make month 0-based */
	utc.mon--;
	/* Make year range from 2006 to 2105 */
	if (utc.year < 6)
		utc.year += 100;

	/* Convert broken-down UTC time to seconds and complete with weekday */
	if (rtc_time2secs(&utc, &utc_secs) < 0)
		return;

	/* Send the newly received UTC time to the Real Time Clock */
	update_rtc(utc_secs);

	/* Add local time offset and daylight saving time to UTC to get local time */
	local_secs = utc_secs +
	             LT_OFFSET_S +
	             (rtc_dst_eu(&utc, rtc_weekday(utc_secs)) ? DST_OFFSET_S : 0);

	/* Break down local time in seconds */
	rtc_secs2time(local_secs, &local);

	/* Make month 1-based */
	local.mon++;
	/* Make year 2000-based */
	local.year %= 100;

	/* Print the broken-down time back into the corresponding arguments (snprintf is not supported, modulo 100 should offer enough protection) */
	sprintf(argv[1], "%02d%02d%02d", local.hour % 100, local.min % 100, local.sec % 100);
	sprintf(argv[9], "%02d%02d%02d", local.day % 100, local.mon % 100, local.year);

	nmea_send(argc, argv);

	return;
}


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


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
void __interrupt() isr(void)
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

#ifdef TEST_DST
	rtc_dst_eu_test();
#endif /* TEST_DST */

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
