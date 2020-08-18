/******************************************************************************/
/* File    : nmea_uart.h                                                      */
/* Function: Header file of 'nmea_uart.c'                                     */
/* Author  : Robert Delien                                                    */
/* Copyright (C) 2010, Clockwork Engineering                                  */
/******************************************************************************/
#ifndef NMEA_UART_H
#define NMEA_UART_H


void           nmea_uart_init  (unsigned long  bitrate,
                                unsigned char  flow);
void           nmea_uart_term  (void);
void           nmea_uart_rx_isr(void);
void           nmea_uart_tx_isr(void);
void           nmea_work       (void);


#endif /* NMEA_UART_H */
