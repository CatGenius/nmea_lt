/******************************************************************************/
/* File    : cmdline.c                                                        */
/* Function: Command line module                                              */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 1999-2020, Clockwork Engineering                             */
/* History : 13 Feb 2011 by R. Delien:                                        */
/*           - Ported from other project                                      */
/******************************************************************************/
#include <htc.h>
#include <stdio.h>
#include <string.h>

#include "cmdline.h"


/******************************************************************************/
/*** Macros                                                                 ***/
/******************************************************************************/
#define PROMPT                 "# "


/******************************************************************************/
/*** Global Data                                                            ***/
/******************************************************************************/
extern const struct command_t  commands[];
static char                    linebuffer[CMDLINE_LENGTH_MAX];
static unsigned char           localecho = 1;


/******************************************************************************/
/*** Static functions                                                       ***/
/******************************************************************************/
static int cmd2index(char *cmd)
{
	int  index = 0;

	while (commands[index].cmd) {
		if (!strcmp (cmd, commands[index].cmd))
			return index;
		index++;
	}

	return -1;
}


static void proc_line(char *line)
{
	unsigned int  len = strlen(line);
	int           argc = 0;
	char          *argv[ARGS_MAX];
	int           index;

	/* Trim trailing white spaces */
	while (len && (line[len-1] == ' ' || line[len-1] == '\t')) {
		line[len-1] = 0;
		len--;
	}

	/* Build argument list */
	while (*line) {
		/* Skip leading white spaces and replace them with 0-terminations */
		while ((*line == ' ') || (*line == '\t')) {
			*line = 0;
			line++;
		}

		/* Store the beginning of this argument */
		if (*line) {
			argv[argc] = line;
			argc++;
		}

		/* Find the separating white space */
		while (*line && *line != ' ' && *line != '\t')
			line++;
	}

	index = cmd2index(argv[0]);
	if (index >= 0) {
		switch (commands[index].function(argc, argv)) {
		case ERR_OK:
			break;
		case ERR_SYNTAX:
			printf("Syntax error\n");
			break;
		case ERR_IO:
			printf("I/O error\n");
			break;
		case ERR_PARAM:
			printf("Parameter error\n");
			break;
		default:
			printf("Unknown error\n");
		}
	} else
		printf("Unknown command '%s'\n", argv[0]);
}


static void proc_char(char rxd)
{
	static unsigned char  curcolumn = 0;

	if ((rxd >= ' ') && (rxd <= '~')) {
		if (curcolumn < (CMDLINE_LENGTH_MAX-1)) {
			/* Add readable characters to the line as long as the buffer permits */
			linebuffer[curcolumn] = rxd;
			curcolumn++;

			if (localecho)
				putchar(rxd);
		} else
			/* Sound the bell is the buffer is full */
			if (localecho)
				putchar('\a');
	} else if (rxd == '\r') {
		if (localecho)
			putchar('\n');

		if (curcolumn) {
			/* Terminate string */
			linebuffer[curcolumn] = 0;
			/* Process string */
			proc_line(linebuffer);
		}
		curcolumn = 0;

		if (localecho)
			printf(PROMPT);
	} else if (rxd == 0x7f) {
		/* Delete last character from the line */
		if (curcolumn) {
			/* Remove last character from the buffer */
			curcolumn--;

			if (localecho)
				putchar(rxd);
		}
	}
}


/******************************************************************************/
/*** Functions                                                              ***/
/******************************************************************************/
void cmdline_init(void)
{
	if (localecho)
		printf(PROMPT);
}


void cmdline_work(void)
{
	char  byte;

	/* Read input data from stdin */
	while ((byte = getchar()) != (char)EOF)
		/* Process the byte */
		proc_char((char)byte);
}


/******************************************************************************/
/*** Built-in commands                                                      ***/
/******************************************************************************/
int cmdline_echo(int argc, char *argv[])
{
	if (argc > 2)
		return ERR_SYNTAX;

	if (argc == 2) {
		if (!strncmp(argv[1], "on", CMDLINE_LENGTH_MAX))
			localecho = 1;
		else if (!strncmp(argv[1], "off", CMDLINE_LENGTH_MAX))
			localecho = 0;
		else
			return ERR_SYNTAX;
	}
	printf("Echo: %s\n", localecho?"on":"off");

	return ERR_OK;
}


#ifdef CMDLINE_HELP
int cmdline_help(int argc, char *argv[])
{
	int  index = 0;

	if (argc > 1)
		return ERR_SYNTAX;

	printf("Known commands:\n");
	while (commands[index].function) {
		printf("%s\n", commands[index].cmd);
		index++;
	}

	return ERR_OK;
}
#endif /* CMDLINE_HELP */
