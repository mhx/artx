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
 *  \file serial.c
 *  \brief Serial interface
 */

// TODO: see if we can do buffering and use the interrupt to transmit data


/*===== GLOBAL INCLUDES ======================================================*/

#include <avr/io.h>


/*===== LOCAL INCLUDES =======================================================*/

#include "artx/serial.h"

#if ARTX_ENABLE_SERIAL

/*===== DEFINES ==============================================================*/

/*===== TYPEDEFS =============================================================*/

/*===== STATIC FUNCTION PROTOTYPES ===========================================*/

/*===== EXTERNAL VARIABLES ===================================================*/

/*===== GLOBAL VARIABLES =====================================================*/

/*===== STATIC VARIABLES =====================================================*/

/*===== STATIC FUNCTIONS =====================================================*/

/*===== FUNCTIONS ============================================================*/

/**
 *  Transmit a single byte
 *
 *  Transmit a single byte via the serial interface.
 *  The routine blocks until the byte is transmitted.
 *
 *  \param data                  Byte to transmit.
 */

void ARTX_serial_tx_byte_x(uint8_t uart, uint8_t data)
{
#define artx_TX_BYTEx(usart)                                    \
      while ((UCSR ## usart ## A & (1 << UDRE ## usart)) == 0)  \
      {                                                         \
        (void) 0;                                               \
      }                                                         \
                                                                \
      UDR ## usart = data

  switch (uart)
  {
#if defined(UCSRA) || defined(UCSR0A)
    case 0:
# if defined(UCSRA)
      while ((UCSRA & (1 << UDRE)) == 0)
      {
        (void) 0;
      }

      UDR = data;
# else
      artx_TX_BYTEx(0);
# endif
      break;
#endif

#if defined(UCSR1A)
    case 1:
      artx_TX_BYTEx(1);
      break;
#endif

#if defined(UCSR2A)
    case 2:
      artx_TX_BYTEx(2);
      break;
#endif

#if defined(UCSR3A)
    case 3:
      artx_TX_BYTEx(3);
      break;
#endif
  }

#undef artx_TX_BYTEx
}

/**
 *  Transmit data from RAM
 *
 *  Transmit \p len bytes via the serial interface.
 *  The routine blocks until all data is transmitted.
 *
 *  \param data                  Pointer to data.
 *
 *  \param len                   Number of bytes to transmit.
 */

void ARTX_serial_tx_data_x(uint8_t uart, const void *data, uint8_t len)
{
  const uint8_t *p = (const uint8_t *) data;

  while (len--)
  {
    ARTX_serial_tx_byte_x(uart, *p++);
  }
}

/**
 *  Transmit C string from RAM
 *
 *  Transmit null-terminated string via the serial interface.
 *  The routine blocks until the whole string is transmitted.
 *
 *  \param str                   Pointer to null-terminated string.
 */

void ARTX_serial_tx_string_x(uint8_t uart, const char *str)
{
  while (*str)
  {
    ARTX_serial_tx_byte_x(uart, *str++);
  }
}

/**
 *  Transmit C string from flash memory
 *
 *  Transmit null-terminated string from flash memory via the serial
 *  interface. The routine blocks until the whole string is transmitted.
 *
 *  \param str                   Pointer to null-terminated string in
 *                               flash memory.
 */

void ARTX_serial_tx_string_pgm_x(uint8_t uart, PGM_P str)
{
  char byte;

  while ((byte = pgm_read_byte(str++)) != 0)
  {
    ARTX_serial_tx_byte_x(uart, byte);
  }
}

/**
 *  Enable/disable RX complete interrupt
 *
 *  \param enable                Whether to enable to disable
 */

void ARTX_serial_rxc_int_x(uint8_t uart, uint8_t enable)
{
#define artx_RXC_INTx(usart)                                    \
      if (enable)                                               \
      {                                                         \
        UCSR ## usart ## B |= (1 << RXCIE ## usart);            \
      }                                                         \
      else                                                      \
      {                                                         \
        UCSR ## usart ## B &= ~(1 << RXCIE ## usart);           \
      }

  switch (uart)
  {
#if defined(UCSRB) || defined(UCSR0B)
    case 0:
# if defined(UCSRB)
      if (enable)
      {
        UCSRB |= (1 << RXCIE);
      }
      else
      {
        UCSRB &= ~(1 << RXCIE);
      }
# else
      artx_RXC_INTx(0);
# endif
      break;
#endif

#if defined(UCSR1B)
    case 1:
      artx_RXC_INTx(1);
      break;
#endif
  }

#undef artx_RXC_INTx
}

#endif
