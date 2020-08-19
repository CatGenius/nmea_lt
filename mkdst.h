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
void mkdst     (time_t     utc_secs,
                struct tm  *local);
void test_mkdst(void);


#endif /* MKDST_H */
