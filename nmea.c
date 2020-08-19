#include <xc.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uart1.h"

#include "nmea.h"


/******************************************************************************/
/*** Macros                                                                 ***/
/******************************************************************************/
#define NMEA_ARGS_MAX  12


/******************************************************************************/
/*** Global Data                                                            ***/
/******************************************************************************/
extern const struct nmea_t  nmea[];


/******************************************************************************/
/* Static functions                                                           */
/******************************************************************************/
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
	unsigned char  calcsum = 0;
	int            argc = 0;
	char           *argv[NMEA_ARGS_MAX];

	if (len < 4) {
		printf("NMEA: Dropping under-sized message '%s'\n", sentence);
		return;
	}

	if (sentence[len - 3] != '*') {
		printf("NMEA: Dropping message without checksum separator '%s'\n", sentence);
		return;
	}

	checksum = strtoul(&sentence[len - 2], &endptr, 16);
	if (*endptr != '\0') {
		printf("NMEA: Dropping message non-numerical checksum '%s'\n", sentence);
		return;
	}

	for (ndx = 0; ndx < len - 3; ndx++)
		calcsum ^= sentence[ndx];
	if (calcsum != checksum) {
		printf("NMEA: Dropping message with bad checksum 0x%.2x '%s'\n", calcsum, sentence);
		return;
	}
	sentence[len - 3] = '\0';
	len -= 3;

	/* Build argument list */
	while (*sentence != '\0') {
		/* Replace leading commas with 0-terminations and add an empty argument for each one */
		while (*sentence == ',') {
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

		/* Skip characters until past argument (indicated by a separating comma or a 0-termination) */
		while (*sentence != '\0' &&
		       *sentence != ',') {
			sentence++;
		}

		/* If the argument was terminated by a separating comma, replace is with a 0-termination */
		if (*sentence == ',') {
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
	static char           sentence[82 + 1];
	static char           len;
	static unsigned char  receiving = 0;

	/* Test if we need to start receiving */
	if (!receiving) {
		switch (byte) {
		default:
		case '\r':
			printf("NMEA: Discarding OOB byte 0x%.2x\n", byte);
		case '\n':
			return;

		case '$':
			receiving = 1;
			len = 0;
			return;
		}
	}

	/* Test if we need to end receiving */
	if (byte == '\n' ||
	    byte == '\r') {
		receiving = 0;
		sentence[len] = '\0';
		proc_nmea_sentence(sentence, len);
		return;
	}

	/* Copy the received byte into the current received sentence */
	sentence[len] = byte;

	if (++len > 81) {
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
