#ifndef artx_SERIAL_H_
#define artx_SERIAL_H_

/*******************************************************************************
*
* ARTX serial interface
*
********************************************************************************
*
* ARTX - A realtime executive library for Atmel AVR microcontrollers
*
* Copyright (C) 2007-2015 Marcus Holland-Moritz.
*
* ARTX is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ARTX is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ARTX.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

/**
 *  \file artx/serial.h
 *  \brief Serial interface
 */

#include <avr/pgmspace.h>

#include "artx/artx.h"

#if ARTX_ENABLE_SERIAL

/**
 *  Parity
 */
enum ARTX_serial_parity
{
  ARTX_SP_NONE = 0x0,            //!< No parity
  ARTX_SP_EVEN = 0x2,            //!< Even parity
  ARTX_SP_ODD  = 0x3             //!< Odd parity
};

/**
 *  Mode
 */
enum ARTX_serial_mode
{
  ARTX_SM_READ         = 0x1,    //!< Read only
  ARTX_SM_WRITE        = 0x2,    //!< Write only
  ARTX_SM_RW           = 0x3,    //!< Read/Write
  ARTX_SM_DOUBLE_SPEED = 0x4     //!< Double speed mode
};

/**
 *  Initialize serial interface
 *
 *  Call this routine to (re-)initialize the serial interface.
 *  The baud rate will be adjusted to the closest matching
 *  rate available for the current #ARTX_CLOCK_FREQUENCY.
 *
 *  \param rate                  Baud rate.
 *
 *  \param data_bits             Number of data bits.
 *
 *  \param stop_bits             Number of stop bits.
 *
 *  \param parity                Parity to use.
 *
 *  \param double_speed_mode     Whether or not to use double speed mode.
 */

static inline void ARTX_serial_init_x(uint8_t uart, uint32_t rate,
                                      uint8_t data_bits, uint8_t stop_bits,
                                      enum ARTX_serial_parity parity,
                                      enum ARTX_serial_mode mode)
{
  uint16_t ubrr = (ARTX_CLOCK_FREQUENCY + (mode & ARTX_SM_DOUBLE_SPEED ? 4 : 8)*rate) /
                  ((mode & ARTX_SM_DOUBLE_SPEED ? 8 : 16)*rate) - 1;

#define artx_INIT_USARTx(usart)                                                     \
          UBRR ## usart = ubrr;                                                     \
          UCSR ## usart ## A = (!!(mode & ARTX_SM_DOUBLE_SPEED)) << U2X ## usart;   \
          UCSR ## usart ## B = ((!!(mode & ARTX_SM_READ))  << RXEN ## usart) |      \
                               ((!!(mode & ARTX_SM_WRITE)) << TXEN ## usart);       \
          UCSR ## usart ## C = (((uint8_t) parity) << UPM ## usart ## 0) |          \
                               ((stop_bits > 1) << USBS ## usart) |                 \
                               ((data_bits - 5) << UCSZ ## usart ## 0)

  switch (uart)
  {
#if defined(UCSRA) || defined(UCSR0A)
    case 0:
# if defined(UCSRA)
      UBRRH = ubrr >> 8;
      UBRRL = ubrr & 0xFF;
      UCSRA = (!!(mode & ARTX_SM_DOUBLE_SPEED)) << U2X;
      UCSRB = ((!!(mode & ARTX_SM_READ)) << RXEN) | ((!!(mode & ARTX_SM_WRITE)) << TXEN);
      UCSRC = (1 << URSEL)                 |
              (((uint8_t) parity) << UPM0) |
              ((stop_bits > 1) << USBS)    |
              ((data_bits - 5) << UCSZ0);
# else
      artx_INIT_USARTx(0);
# endif
      break;
#endif

#if defined(UCSR1A)
    case 1:
      artx_INIT_USARTx(1);
      break;
#endif

#if defined(UCSR2A)
    case 2:
      artx_INIT_USARTx(2);
      break;
#endif

#if defined(UCSR3A)
    case 3:
      artx_INIT_USARTx(3);
      break;
#endif

    default:
      /* TODO: fail! */
      break;
  }

#undef artx_INIT_USARTx
}

void ARTX_serial_tx_byte_x(uint8_t uart, uint8_t data);
void ARTX_serial_tx_data_x(uint8_t uart, const void *data, uint8_t len);
void ARTX_serial_tx_string_x(uint8_t uart, const char *str);
void ARTX_serial_tx_string_pgm_x(uint8_t uart, PGM_P str);

void ARTX_serial_rxc_int_x(uint8_t uart, uint8_t enable);

#ifndef ARTX_DEFAULT_SERIAL_UART
# define ARTX_DEFAULT_SERIAL_UART 0
#endif

#define ARTX_serial_init(rate, data_bits, stop_bits, parity, mode) \
          ARTX_serial_init_x(ARTX_DEFAULT_SERIAL_UART, rate, data_bits, stop_bits, parity, mode)

#define ARTX_serial_tx_byte(data) \
          ARTX_serial_tx_byte_x(ARTX_DEFAULT_SERIAL_UART, data)

#define ARTX_serial_tx_data(data, len) \
          ARTX_serial_tx_data_x(ARTX_DEFAULT_SERIAL_UART, data, len)

#define ARTX_serial_tx_string(str) \
          ARTX_serial_tx_string_x(ARTX_DEFAULT_SERIAL_UART, str)

#define ARTX_serial_tx_string_pgm(str) \
          ARTX_serial_tx_string_pgm_x(ARTX_DEFAULT_SERIAL_UART, str)

#define ARTX_serial_rxc_int(enable) \
          ARTX_serial_rxc_int_x(ARTX_DEFAULT_SERIAL_UART, enable)

#endif

#endif
