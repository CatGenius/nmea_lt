/******************************************************************************/
/* File    : cmdline.h                                                        */
/* Function: Header file of 'cmdline.c'                                       */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 1999-2020, Clockwork Engineering                             */
/******************************************************************************/
#ifndef CMDLINE_H
#define CMDLINE_H


/******************************************************************************/
/*** Macros                                                                 ***/
/******************************************************************************/
#define CMDLINE_HELP                    /* Enable help command */
#define CMDLINE_LENGTH_MAX      ( 20)   /* Maximum length of a complete command line */
#define ARGS_MAX                (  4)   /* Maximum number of arguments, including command */


/* Errors reported by command implementations, triggering standard error messages */
#define ERR_OK                  ( 0)    /* Return this if the command was executed successfully */
#define ERR_SYNTAX              (-1)    /* Return this if the command contained a syntax error, such as too few or too many parameters */
#define ERR_IO                  (-2)    /* Return this if the command could not be executed due to external factors */
#define ERR_PARAM               (-3)    /* Return this if the command had erroneous parameters, such as erroneous values or types */


/******************************************************************************/
/*** Types                                                                  ***/
/******************************************************************************/
struct command_t {
	const char  *cmd;
	int         (*function)(int argc, char *argv[]);
};


/******************************************************************************/
/*** Functions                                                              ***/
/******************************************************************************/
void            cmdline_init            (void);
void            cmdline_work            (void);

/* Built-in command-line commands */
int             cmdline_echo            (int                    argc,
                                         char                   *argv[]);
#ifdef CMDLINE_HELP
int             cmdline_help            (int                    argc,
                                         char                   *argv[]);
#endif /* CMDLINE_HELP */


#endif /* CMDLINE_H */
