/******************************************************************************/
/* File    : mkdst.h                                                          */
/* Function: Header file of 'mkdst.c'                                         */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2020, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef MKDST_H
#define MKDST_H


/******************************************************************************/
/*** Functions                                                              ***/
/******************************************************************************/
unsigned char dst_eu     (struct rtctime_t  *utc,
                          unsigned char     weekday);
void          test_dst_eu(void);


#endif /* MKDST_H */
