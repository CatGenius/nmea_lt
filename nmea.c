#include <xc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uart1.h"

#include "nmea.h"


/******************************************************************************/
/*** Macros                                                                 ***/
/******************************************************************************/
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


static void proc_nmea_sentence(char *sentence, char len)
{
	int            ndx;
	char           *endptr;
	unsigned long  checksum;
	unsigned char  calcsum;
	int            argc = 0;
	char           *argv[NMEA_ARGS_MAX];

	if (len < NMEA_CHECKSUM_SEPARATOR_LEN + NMEA_CHECKSUM_LEN) {
		printf("NMEA: Dropping under-sized message '%s'\n", sentence);
		return;
	}

	if (sentence[len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN] != NMEA_CHECKSUM_SEPARATOR) {
		printf("NMEA: Dropping message without checksum separator '%s'\n", sentence);
		return;
	}

	checksum = strtoul(&sentence[len - NMEA_CHECKSUM_LEN], &endptr, 16);
	if (*endptr != '\0') {
		printf("NMEA: Dropping message non-numerical checksum '%s'\n", sentence);
		return;
	}

	calcsum = calc_checksum(sentence, len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN);
	if (calcsum != checksum) {
		printf("NMEA: Dropping message with bad checksum 0x%.2x '%s'\n", calcsum, sentence);
		return;
	}
	sentence[len - NMEA_CHECKSUM_LEN - NMEA_CHECKSUM_SEPARATOR_LEN] = '\0';
	len -= NMEA_CHECKSUM_LEN + NMEA_CHECKSUM_SEPARATOR_LEN;

	/* Build argument list */
	while (*sentence != '\0') {
		/* Replace leading separators with 0-terminations and add an empty argument for each one */
		while (*sentence == NMEA_SEPARATOR) {
			if (argc >= NMEA_ARGS_MAX) {
				printf("NMEA: Dropping message with too many arguments\n");
				return;
			}
			*sentence = '\0';
			argv[argc++] = sentence;
			sentence++;
		}

		/* Store the beginning of this argument */
		if (*sentence != '\0') {
			if (argc >= NMEA_ARGS_MAX) {
				printf("NMEA: Dropping message with too many arguments\n");
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
		printf("NMEA: unsupported sentence '%s'\n", argv[0]);
		return;
	}

	nmea[ndx].function(argc, argv);
}


static void proc_nmea_char(char byte)
{
	static char           sentence[NMEA_DATA_LEN_MAX +
	                               NMEA_CHECKSUM_SEPARATOR_LEN +
	                               NMEA_CHECKSUM_LEN + 1];
	static char           len;
	static unsigned char  receiving = 0;

	/* Test if we need to start receiving */
	if (!receiving) {
		switch (byte) {
		default:
		case NMEA_TRAILER1:
			printf("NMEA: Discarding OOB byte 0x%.2x\n", byte);
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
		printf("NMEA: Discarding over-sized sentence '%s'\n", sentence);
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
