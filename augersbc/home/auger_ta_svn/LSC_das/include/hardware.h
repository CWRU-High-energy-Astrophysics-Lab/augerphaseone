#if !defined(_HARDWARE_H_)
#define _HARDWARE_H_

/*******************************************

  $Author: guglielmi $
  $Date: 2010-12-01 14:21:49 +0100 (Wed, 01 Dec 2010) $
  $Revision: 307 $

********************************************/
/* CPU
   ARM920T rev 0 (v4l)
   CPU Clk: 180 MHz
   Bus Clk: 80 MHz
*/

/* RAM
   128 Mega
*/

/* ROM Boot */

/* USB */

/* CANBUS */

/* Serial IO */

/* Ethernet */

/* SPI */
/*
  SPI0: Eprom
  SPI1: DAC (/dev/spidev0.1)
  SPI2: ADCs (/dev/spidev0.2)
    Temperature
    Channel 0:
    Channel 1:

  SPI3: Canbus (mcp2515 interface)
*/

/* CSR
   CSR5 = 0x3081 (FE)
   CSR6 = 0x3081 (TTAG)
   CSR7 = 0x3081 (Slow control)

    NWS: 0000001 (1 Wait state)
    WSEN: 1 (WS Enabled)
    TDF: 0000
    BAT: 1
    DBW: 01
    DRP: 0
*/

#define BASE_CS5 0x60000000
#define BASE_CS6 0x70000000
#define BASE_CS7 0x80000000

/* FPGA */
/*
   TTAG:
    70000000  Calibration 100MHz par le 1PPS   (27bits)
    70000004  FIFO Event fast 100MHz   (27bits)
    70000008  FIFO Event fast 1PPS  (28bits)

    70000014  Compteur 1PPS  (28bits)
    70000018  Compteur 100MHz  (27bits)
    7000001C  TTAG_ID = $54544147 (pour detecter la presence d'un TimeTagging)
*/
#define BASE_TTAG BASE_CS6         /* as TTAG is using CS6 */

/*
   FE:

*/
#define BASE_FE BASE_CS5           /* as FE is using CS5 */

/*
  Slow Control
*/
#define BASE_SC BASE_CS7           /* as SC is using CS7 */
#define SC_SIZE 0x200

#endif
