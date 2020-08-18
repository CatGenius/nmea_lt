/******************************************************************************/
/* File    : serial.h                                                         */
/* Function: Header file of 'serial.c'                                        */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2010, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef SERIAL_H
#define SERIAL_H


void           serial_init  (unsigned long  bitrate,
                             unsigned char  flow);
void           serial_term  (void);
void           serial_rx_isr(void);
void           serial_tx_isr(void);


#endif /* SERIAL_H */
