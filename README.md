ARTX
=====

ARTX is a pre-emptive real-time operating system kernel for Atmel's
8-bit AVR microcontrollers.

Its primary goal is a small memory footprint. The minimal configuration
uses only 420 bytes of flash memory. Various optional components can be
configured at compile time, increasing both functionality and code size.

There is support for EEPROM access, serial and SPI communication, as well
as experimental 1-wire bus code. The kernel supports an arbitrary number
of tasks with different intervals, priorities and stack sizes. There are
weird features like synchronization of a kernel tick with an external
time source.

It is also possible to build ARTX with support for monitoring, in which
case it'll occupy a serial port to send out monitoring packets that can
be displayed using a small GUI to determine information like live task
load and stack usage.

ARTX is the abbreviation for *A*tmel/*A*VR *R*eal *T*ime e*X*ecutive.

Please have a look at the source code for more documentation. You can
run

  make dox

to generate the documentation from the embedded doxygen comments.
