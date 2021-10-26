/**************************************************************************/
/* File: can.h - CAN driver IOCTL and messages interface                  */
/*                                                                        */
/* LinCAN - (Not only) Linux CAN bus driver                               */
/* Copyright (C) 2002-2009 DCE FEE CTU Prague <http://dce.felk.cvut.cz>   */
/* Copyright (C) 2002-2009 Pavel Pisa <pisa@cmp.felk.cvut.cz>             */
/* Funded by OCERA and FRESCOR IST projects                               */
/* Based on CAN driver code by Arnaud Westenberg <arnaud@wanadoo.nl>      */
/*                                                                        */
/* LinCAN is free software; you can redistribute it and/or modify it      */
/* under terms of the GNU General Public License as published by the      */
/* Free Software Foundation; either version 2, or (at your option) any    */
/* later version.  LinCAN is distributed in the hope that it will be      */
/* useful, but WITHOUT ANY WARRANTY; without even the implied warranty    */
/* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU    */
/* General Public License for more details. You should have received a    */
/* copy of the GNU General Public License along with LinCAN; see file     */
/* COPYING. If not, write to the Free Software Foundation, 675 Mass Ave,  */
/* Cambridge, MA 02139, USA.                                              */
/*                                                                        */
/* To allow use of LinCAN in the compact embedded systems firmware        */
/* and RT-executives (RTEMS for example), main authors agree with next    */
/* special exception:                                                     */
/*                                                                        */
/* Including LinCAN header files in a file, instantiating LinCAN generics */
/* or templates, or linking other files with LinCAN objects to produce    */
/* an application image/executable, does not by itself cause the          */
/* resulting application image/executable to be covered by                */
/* the GNU General Public License.                                        */
/* This exception does not however invalidate any other reasons           */
/* why the executable file might be covered by the GNU Public License.    */
/* Publication of enhanced or derived LinCAN files is required although.  */
/**************************************************************************/

#ifndef _CAN_DRVAPI_T_H
#define _CAN_DRVAPI_T_H

#ifdef __KERNEL__

#include <linux/time.h>
#include <linux/types.h>
#include <linux/ioctl.h>

#else /* __KERNEL__ */

#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#endif /* __KERNEL__ */

#include "./canmsg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CAN ioctl magic number */
#define CAN_IOC_MAGIC 'd'

typedef unsigned long bittiming_t;
typedef unsigned short channel_t;

/**
 * struct can_baudparams_t - datatype for calling CONF_BAUDPARAMS IOCTL 
 * @flags: reserved for additional flags for chip configuration, should be written -1 or 0
 * @baudrate: baud rate in Hz
 * @sjw: synchronization jump width (0-3) prescaled clock cycles
 * @sampl_pt: sample point in % (0-100) sets (TSEG1+1)/(TSEG1+TSEG2+2) ratio
 * 
 * The structure is used to configure new set of parameters into CAN controller chip.
 * If default value of some field should be preserved, fill field by value -1.
 */
struct can_baudparams_t {
	long flags;
	long baudrate;
	long sjw;
	long sample_pt;
};

/* CAN ioctl functions */
#define CAN_DRV_QUERY _IO(CAN_IOC_MAGIC, 0)
#define CAN_DRV_QRY_BRANCH    0	/* returns driver branch value - "LINC" for LinCAN driver */
#define CAN_DRV_QRY_VERSION   1	/* returns driver version as (major<<16) | (minor<<8) | patch */
#define CAN_DRV_QRY_MSGFORMAT 2	/* format of canmsg_t structure */

#define CMD_START _IOW(CAN_IOC_MAGIC, 1, channel_t)
#define CMD_STOP _IOW(CAN_IOC_MAGIC, 2, channel_t)
//#define CMD_RESET 3

#define CONF_BAUD _IOW(CAN_IOC_MAGIC, 4, bittiming_t)
//#define CONF_ACCM
//#define CONF_XTDACCM
//#define CONF_TIMING
//#define CONF_OMODE
#define CONF_FILTER _IOW(CAN_IOC_MAGIC, 8, unsigned char)

//#define CONF_FENABLE
//#define CONF_FDISABLE

#define STAT _IO(CAN_IOC_MAGIC, 9)
#define CANQUE_FILTER _IOW(CAN_IOC_MAGIC, 10, struct canfilt_t)
#define CANQUE_FLUSH  _IO(CAN_IOC_MAGIC, 11)
#define CONF_BAUDPARAMS  _IOW(CAN_IOC_MAGIC, 11, struct can_baudparams_t)
#define CANRTR_READ  _IOWR(CAN_IOC_MAGIC, 12, struct canmsg_t)

#ifdef __cplusplus
} /* extern "C"*/
#endif

#endif /*_CAN_DRVAPI_T_H*/
