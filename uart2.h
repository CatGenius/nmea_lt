/******************************************************************************/
/* File    : uart2.h                                                          */
/* Function: Header file of 'uart2.c'                                         */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2010, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef UART2_H
#define UART2_H


void           uart2_init  (unsigned long  bitrate,
                            unsigned char  flow);
void           uart2_term  (void);
void           uart2_rx_isr(void);
void           uart2_tx_isr(void);


#endif /* UART2_H */
