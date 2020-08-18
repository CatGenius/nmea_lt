/******************************************************************************/
/* File    : nmea.h                                                           */
/* Function: Header file of 'nmea.c'                                          */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2010, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef NMEA_H
#define NMEA_H


/******************************************************************************/
/*** Types                                                                  ***/
/******************************************************************************/
struct nmea_t {
	const char  *keyword;
	void        (*function)(int argc, char *argv[]);
};


/******************************************************************************/
/*** Functions                                                              ***/
/******************************************************************************/
void nmea_work(void);


#endif /* NMEA_H */
