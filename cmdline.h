/******************************************************************************/
/* File    : cmdline.h                                                        */
/* Function: Header file of 'cmdline.c'                                       */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 1999-2020, Clockwork Engineering                             */
/******************************************************************************/
#ifndef CMDLINE_H
#define CMDLINE_H


#define LINEBUFFER_MAX  (20)  /* Maximum length of a complete command line */
#define ARGS_MAX        (4)   /* Maximum number of arguments, including command */

#define ERR_OK          ( 0)
#define ERR_SYNTAX      (-1)
#define ERR_IO          (-2)
#define ERR_PARAM       (-3)

struct command {
	const char  *cmd;
	int         (*function)(int argc, char *argv[]);
};

/* Generic */
void  cmdline_init(void);
void  cmdline_work(void);

/* Command implementations */
int   echo        (int argc, char* argv[]);
int   help        (int argc, char* argv[]);


#endif /* CMDLINE_H */
