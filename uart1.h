/******************************************************************************/
/* File    : uart1.h                                                          */
/* Function: Header file of 'uart1.c'                                         */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2010, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef UART1_H
#define UART1_H


void           uart1_init  (unsigned long  bitrate,
                            unsigned char  flow);
void           uart1_term  (void);
void           uart1_rx_isr(void);
void           uart1_tx_isr(void);
void           uart1_work  (void);


#endif /* UART1_H */
