#include <xc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uart1.h"

#include "nmea.h"


/******************************************************************************/
/*** Macros                                                                 ***/
/******************************************************************************/
//#define                    DEBUG

#define NMEA_HEADER        '$'
#define NMEA_TRAILER1      '\r'
#define NMEA_TRAILER2      '\n'
#define NMEA_SEPARATOR     ','
#define NMEA_CHECKSUM_SEPARATOR  '*'

#define NMEA_LEN_MAX       82
#define NMEA_HEADER_LEN    1
#define NMEA_CHECKSUM_SEPARATOR_LEN  1
#define NMEA_CHECKSUM_LEN  2
#define NMEA_TRAILER_LEN   2
#define NMEA_DATA_LEN_MAX  (NMEA_LEN_MAX - NMEA_HEADER_LEN - NMEA_CHECKSUM_SEPARATOR_LEN - NMEA_CHECKSUM_LEN - NMEA_TRAILER_LEN)

#define NMEA_ARGS_MAX      12


/******************************************************************************/
/*** Global Data                                                            ***/
/******************************************************************************/
extern const struct nmea_t  nmea[];


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
static unsigned char calc_checksum(char *buf, unsigned char len)
{
	unsigned char  ndx;
	unsigned char  calcsum = 0;

	for (ndx = 0; ndx < len; ndx++)
		calcsum ^= buf[ndx];

	return calcsum;
}


static int keyword2index(char *keyword)
{
	int  ndx = 0;

	while (nmea[ndx].keyword) {
		if (!strcmp (keyword, nmea[ndx].keyword))
			return ndx;
		ndx++;
	}

	return (-1);
}


static void proc_nmea_sentence(char *sentence, unsigned char len)
{
	int            ndx;
	char           *endptr;
	unsigned long  checksum;
	unsigned char  calcsum;
	int            argc = 0;
	char           *argv[NMEA_ARGS_MAX];

	if (len < NMEA_CHECKSUM_SEPARATOR_LEN + NMEA_CHECKSUM_LEN) {
#ifdef DEBUG
		printf("NMEA: Dropping under-sized sentence '%s'\n", sentence);
#endif /* DEBUG */
		return;
	}

	if (sentence[len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN] != NMEA_CHECKSUM_SEPARATOR) {
#ifdef DEBUG
		printf("NMEA: Dropping sentence without checksum separator '%s'\n", sentence);
#endif /* DEBUG */
		return;
	}

	checksum = strtoul(&sentence[len - NMEA_CHECKSUM_LEN], &endptr, 16);
	if (*endptr != '\0') {
#ifdef DEBUG
		printf("NMEA: Dropping sentence with non-numerical checksum '%s'\n", sentence);
#endif /* DEBUG */
		return;
	}

	calcsum = calc_checksum(sentence, len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN);
	if (calcsum != checksum) {
#ifdef DEBUG
		printf("NMEA: Dropping sentence with bad checksum 0x%.2x '%s'\n", calcsum, sentence);
#endif /* DEBUG */
		return;
	}
	sentence[len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN] = '\0';
	len -= NMEA_CHECKSUM_LEN + NMEA_CHECKSUM_SEPARATOR_LEN;

	/* Build argument list */
	while (*sentence != '\0') {
		/* Replace leading separators with 0-terminations and add an empty argument for each one */
		while (*sentence == NMEA_SEPARATOR) {
			if (argc >= NMEA_ARGS_MAX) {
#ifdef DEBUG
				printf("NMEA: Dropping sentence with too many arguments\n");
#endif /* DEBUG */
				return;
			}
			*sentence = '\0';
			argv[argc++] = sentence;
			sentence++;
		}

		/* Store the beginning of this argument */
		if (*sentence != '\0') {
			if (argc >= NMEA_ARGS_MAX) {
#ifdef DEBUG
				printf("NMEA: Dropping sentence with too many arguments\n");
#endif /* DEBUG */
				return;
			}
			argv[argc++] = sentence;
		}

		/* Skip characters until past argument (indicated by a separator or a 0-termination) */
		while (*sentence != '\0' &&
		       *sentence != NMEA_SEPARATOR) {
			sentence++;
		}

		/* If the argument was terminated by a separator, replace is with a 0-termination */
		if (*sentence == NMEA_SEPARATOR) {
			*sentence = '\0';
			sentence++;
		}
	}

	if ((ndx = keyword2index(argv[0])) < 0) {
#ifdef DEBUG
		printf("NMEA: unsupported sentence '%s'\n", argv[0]);
#endif /* DEBUG */
		return;
	}

	nmea[ndx].function(argc, argv);
}


static void proc_nmea_char(char byte)
{
	static char           sentence[NMEA_DATA_LEN_MAX +
	                               NMEA_CHECKSUM_SEPARATOR_LEN +
	                               NMEA_CHECKSUM_LEN + 1];
	static unsigned char  len;
	static unsigned char  receiving = 0;

	/* Test if we need to start receiving */
	if (!receiving) {
		switch (byte) {
		default:
		case NMEA_TRAILER1:
#ifdef DEBUG
			printf("NMEA: Discarding OOB byte 0x%.2x\n", byte);
#endif /* DEBUG */
		case NMEA_TRAILER2:
			return;

		case NMEA_HEADER:
			/* Start receiving and reset the received length */
			receiving = 1;
			len = 0;
			return;
		}
	}

	/* Test if we need to end receiving */
	if (byte == NMEA_TRAILER2 ||
	    byte == NMEA_TRAILER1) {
		receiving = 0;
		sentence[len] = '\0';
		proc_nmea_sentence(sentence, len);
		return;
	}

	/* Copy the received byte into the current received sentence */
	sentence[len] = byte;

	if (++len >= NMEA_DATA_LEN_MAX + NMEA_CHECKSUM_SEPARATOR_LEN + NMEA_CHECKSUM_LEN) {
		receiving = 0;
		sentence[len] = '\0';
#ifdef DEBUG
		printf("NMEA: Discarding over-sized sentence '%s'\n", sentence);
#endif /* DEBUG */
	}
}


/******************************************************************************/
/* Functions                                                                  */
/******************************************************************************/
void nmea_work(void)
{
	char  byte;

	while ((byte = uart1_getch()) != (char)EOF)
		proc_nmea_char(byte);
}


void nmea_send(int argc, char *argv[])
{
	static char    sentence[NMEA_LEN_MAX + 1] = { NMEA_HEADER };
	unsigned char  sentence_ndx = NMEA_HEADER_LEN;
	unsigned char  send_ndx;
	int            arg_ndx = 0;

	for (;;) {
		size_t  len = strlen(argv[arg_ndx]);

		/* Test if there's space for the argument */
		if (sentence_ndx + len > NMEA_LEN_MAX - NMEA_TRAILER_LEN - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN)
			return;

		/* Add the argument */
		strcpy(&sentence[sentence_ndx], argv[arg_ndx]);
		sentence_ndx += len;

		if (arg_ndx + 1 >= argc)
			break;

		/* Test if there's space for separator */
		if (sentence_ndx + len > NMEA_LEN_MAX - NMEA_TRAILER_LEN - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN)
			return;

		sentence[sentence_ndx] = NMEA_SEPARATOR;
		sentence_ndx++;

		arg_ndx++;
	}

	/* Add the checksum separator, the checksum and the trailer */
	sprintf(&sentence[sentence_ndx], "%c%.2X%c%c", NMEA_CHECKSUM_SEPARATOR, calc_checksum(&sentence[NMEA_HEADER_LEN], sentence_ndx - NMEA_HEADER_LEN), NMEA_TRAILER1, NMEA_TRAILER2);
	sentence_ndx += NMEA_CHECKSUM_SEPARATOR_LEN + NMEA_CHECKSUM_LEN + NMEA_TRAILER_LEN;

#ifdef DEBUG
	printf("Sending '%s'\n", sentence);
#endif /* DEBUG */

	/* Send the sentence to the serial port */
	for (send_ndx = 0; send_ndx < sentence_ndx; sentence_ndx++)
		uart1_putch(sentence[send_ndx]);
}
