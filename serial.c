#include <htc.h>

#include "serial.h"

#define _XTAL_FREQ 32000000

#define RXBUFFER			/* Use buffers for received characters */
//#define TXBUFFER			/* Use buffers for transmitted character (MAKE SURE TO ENABLE INTERRUPTS BEFORE TRANSMITTING ANYTHING) */
#define BUFFER_SIZE		8	/* Buffer size. Has to be a power of 2 and congruent with queue.head and queue.tail or roll-overs need to be taken into account */
#define BUFFER_SPARE		2	/* Minumum number of free positions before issuing Xoff */

#define INTDIV(t,n)		((2*(t)+(n))/(2*(n)))		/* Macro for integer division with proper round-off (BEWARE OF OVERFLOW!) */
#define FREE(h,t,s)		(((h)>=(t))?((s)-((h)-(t))-1):((t)-(h))-1)

#define XON			0x11	/* ASCII value for Xon (^S) */
#define XOFF			0x13	/* ASCII value for Xoff (^Q) */


struct queue {
	char		buffer[BUFFER_SIZE];	/* Here's where the data goes */
	unsigned	head		: 3;	/* Index to a currently free position in buffer */
	unsigned	tail		: 3;	/* Index to the oldest occupied position in buffer, if not equal to head */
	unsigned	xon_enabled	: 1;	/* Specifies if Xon/Xoff should be issued/adhered to */
	unsigned	xon_state	: 1;	/* Keeps track of current Xon/Xoff state for this queue */
};

#ifdef RXBUFFER
struct queue		rx;
#endif /* RXBUFFER */
#ifdef TXBUFFER
struct queue		tx;
#endif /* TXBUFFER */


void serial_init(unsigned long bitrate, unsigned char flow)
{
#ifdef RXBUFFER
	rx.head        = 0;
	rx.tail        = 0;
	rx.xon_enabled = flow;
	rx.xon_state   = 1;
#endif /* RXBUFFER */

#ifdef TXBUFFER
	tx.head        = 0;
	tx.tail        = 0;
	tx.xon_enabled = flow;
	tx.xon_state   = 1;
#endif /* TXBUFFER */

	BAUD2CONbits.BRG16 = 1;	/* Use 16-bit bit rate generation */
	if (BAUD2CONbits.BRG16) {
		/* 16-bit bit rate generation */
		unsigned long	divider = INTDIV(INTDIV(_XTAL_FREQ, 4UL), bitrate)-1;

		SP2BRGL =  divider & 0x00FF;
		SP2BRGH = (divider & 0xFF00) >> 8;
	} else
		/* 8-bit bit rate generation */
		SPBRG = INTDIV(INTDIV(_XTAL_FREQ, bitrate), 16UL)-1;

	TX2STAbits.CSRC = 1;	/* Clock source from BRG */
	TX2STAbits.BRGH = 1;	/* High-speed bit rate generation */
	TX2STAbits.SYNC = 0;	/* Asynchronous mode */
	RC2STAbits.SPEN = 1;	/* Enable serial port pins */
	RC2IE           = 0;	/* Disable rx interrupt */
	TX2IE           = 0;	/* Disable tx interrupt */
	RC2STAbits.RX9  = 0;	/* 8-bit reception mode */
	TX2STAbits.TX9  = 0;	/* 8-bit transmission mode */
	RC2STAbits.CREN = 0;	/* Reset receiver */
	RC2STAbits.CREN = 1;	/* Enable reception */
	TX2STAbits.TXEN = 0;	/* Reset transmitter */
	TX2STAbits.TXEN = 1;	/* Enable transmission */

#ifdef RXBUFFER
	RC2IE = 1;	/* Enable rx interrupt */

	if (rx.xon_enabled) {
		TX2REG = XON;
		rx.xon_state = 1;
	}
#endif /* RXBUFFER */
}


void serial_term(void)
{
#ifdef RXBUFFER
	RC2IE = 0;	/* Disable rx interrupt */
#endif /* RXBUFFER */
#ifdef TXBUFFER
	while (tx.head != tx.tail);
	TX2IE = 0;	/* Disable tx interrupt */
#endif /* TXBUFFER */

#ifdef RXBUFFER
	if (rx.xon_enabled && rx.xon_state) {
		while(!TX2IF);
		TX2REG = XOFF;
		rx.xon_state = 0;
	}
#endif /* RXBUFFER */
	while(!TX2IF);	/* Wait for the last character to go */

	RC2STAbits.CREN = 0;	/* Disable reception */
	TX2STAbits.TXEN = 0;	/* Disable transmission */
}


void serial_rx_isr(void)
{
#ifdef RXBUFFER
	/* Handle overflow errors */
	if (RC2STAbits.OERR) {
		rx.buffer[rx.head] = RC2REG; /* Read RX register, but do not queue */
		TX2STAbits.TXEN = 0;
		TX2STAbits.TXEN = 1;
		RC2STAbits.CREN = 0;
		RC2STAbits.CREN = 1;
		return;
	}
	/* Handle framing errors */
	if (RC2STAbits.FERR) {
		rx.buffer[rx.head] = RC2REG; /* Read RX register, but do not queue */
		TX2STAbits.TXEN = 0;
		TX2STAbits.TXEN = 1;
		return;
	}
	/* Copy the character from RX register into RX queue */
	rx.buffer[rx.head] = RC2REG;
#ifdef TXBUFFER
	/* Check if an Xon or Xoff needs to be handled */
	if (tx.xon_enabled) {
		if (tx.xon_state && (rx.buffer[rx.head] == XOFF)) {
			tx.xon_state = 0;
			TX2IE = 0;	/* Disable tx interrupt to stop transmitting */
			return;
		} else if (!tx.xon_state && (rx.buffer[rx.head] == XON)) {
			tx.xon_state = 1;
			/* Enable tx interrupt if tx queue is not empty */
			if (tx.head != tx.tail)
				TX2IE = 1;
			return;
		}
	}
#endif /* TXBUFFER */
	/* Queue the character */
	rx.head++;
	/* Check if an Xoff is in required */
	if (rx.xon_enabled &&
	    rx.xon_state &&
	    (FREE(rx.head, rx.tail, BUFFER_SIZE) <= (BUFFER_SPARE))) {
		while(!TX2IF);	// TBD: Could potentially wait forever here
		TX2REG = XOFF;
		rx.xon_state = 0;
	}
	/* Check for an overflow */
	if (rx.head == rx.tail)
		/* Dequeue the oldest character */
		rx.tail++;
#endif /* RXBUFFER */
}


void serial_tx_isr(void)
{
#ifdef TXBUFFER
	/* Copy the character from the TX queue into the TX register */
	TX2REG = tx.buffer[tx.tail];
	/* Dequeue the character */
	tx.tail++;

	/* Disable tx interrupt if queue is empty */
	if (tx.head == tx.tail)
		TX2IE = 0;
#endif /* TXBUFFER */
}


/* Write a character to the serial port */
void putch(char ch)
{
#ifdef TXBUFFER
	unsigned char	queued = 0;

	do {
#ifdef RXBUFFER
		RC2IE = 0;	/* Disable rx interrupt for concurrency */
#endif /* RXBUFFER */
		TX2IE = 0;	/* Disable tx interrupt for concurrency */

		/* Check if there's room in the queue ((tx.head + 1) != tx.tail won't work due to bitfields being cast) */
		if (FREE(tx.head, tx.tail, BUFFER_SIZE)) {
			/* Copy the character into the TX queue */
			tx.buffer[tx.head] = ch;
			/* Queue the character */
			tx.head++;
			queued = 1;
		} else
			CLRWDT();
		if (!tx.xon_enabled || tx.xon_state)
			TX2IE = 1;	/* Re-enable tx interrupt */
#ifdef RXBUFFER
		RC2IE = 1;	/* Re-enable rx interrupt */
#endif /* RXBUFFER */
	} while (!queued);
#else
	if (!RC2STAbits.SPEN)
		return;
	while(!TX2IF) {	/* Wait for TX2REG to be empty */
		if (RC2STAbits.OERR) {
			TX2STAbits.TXEN = 0;
			TX2STAbits.TXEN = 1;
			RC2STAbits.CREN = 0;
			RC2STAbits.CREN = 1;
		}
		if (RC2STAbits.FERR) {
			volatile unsigned char dummy;

			dummy = RC2REG;
			TX2STAbits.TXEN = 0;
			TX2STAbits.TXEN = 1;
		}
		CLRWDT();
	}
	TX2REG = ch;
	CLRWDT();
#endif /* TXBUFFER */
}


/* Read a character from the serial port */
unsigned char readch(char *ch)
{
#ifdef RXBUFFER
	unsigned char	result = 0;

	RC2IE = 0;	/* Disable rx interrupt for concurrency */
#ifdef TXBUFFER
	TX2IE = 0;	/* Disable tx interrupt for concurrency */
#endif /* TXBUFFER */

	/* Check if there's anything to read */
	if (rx.head != rx.tail) {
		/* Copy the character from the RX queue */
		*ch = rx.buffer[rx.tail];
		/* Dequeue the character */
		rx.tail++;
		/* Check if an Xon is in required */
		if (rx.xon_enabled &&
		    !rx.xon_state &&
		    (rx.head == rx.tail)) {
			while(!TX2IF);
			TX2REG = XON;
			rx.xon_state = 1;
		}
		result = 1;
	}

	RC2IE = 1;	/* Re-enable rx interrupt */
#ifdef TXBUFFER
	if ((tx.head != tx.tail) && (!tx.xon_enabled || tx.xon_state))
		TX2IE = 1;	/* Re-enable tx interrupt */
#endif /* TXBUFFER */

	return result;

#else /* !RXBUFFER */
	if (!RCIF)
		return 0;

	if (RC2STAbits.OERR) {
		TX2STAbits.TXEN = 0;
		TX2STAbits.TXEN = 1;
		RC2STAbits.CREN = 0;
		RC2STAbits.CREN = 1;
		return 0;
	}
	if (RC2STAbits.FERR) {
		*ch = RC2REG;
		TX2STAbits.TXEN = 0;
		TX2STAbits.TXEN = 1;
		return 0;
	}

	*ch = RC2REG;
	return 1;
#endif /* RXBUFFER */
}
