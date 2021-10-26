/**************************************************************************/
/* File: canmsg.h - common kernel-space and user-space CAN message struct */
/*                                                                        */
/* LinCAN - (Not only) Linux CAN bus driver                               */
/* Copyright (C) 2002-2009 DCE FEE CTU Prague <http://dce.felk.cvut.cz>   */
/* Copyright (C) 2002-2009 Pavel Pisa <pisa@cmp.felk.cvut.cz>             */
/* Funded by OCERA and FRESCOR IST projects                               */
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

#ifndef _CANMSG_T_H
#define _CANMSG_T_H

#ifdef __KERNEL__

#include <linux/time.h>
#include <linux/types.h>

#else /* __KERNEL__ */

#include <inttypes.h>
#include <sys/time.h>
#include <sys/types.h>

#endif /* __KERNEL__ */

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * CAN_MSG_VERSION_2 enables new canmsg_t layout compatible with
 * can4linux project from http://www.port.de/
 * 
 */
#define CAN_MSG_VERSION_2

/* Number of data bytes in one CAN message */
#define CAN_MSG_LENGTH 8

#ifdef CAN_MSG_VERSION_2

typedef struct timeval canmsg_tstamp_t ;

typedef unsigned long canmsg_id_t;

/**
 * struct canmsg_t - structure representing CAN message
 * @flags:  message flags
 *      %MSG_RTR .. message is Remote Transmission Request,
 *	%MSG_EXT .. message with extended ID, 
 *      %MSG_OVR .. indication of queue overflow condition,
 *	%MSG_LOCAL .. message originates from this node.
 * @cob:    communication object number (not used)
 * @id:     ID of CAN message
 * @timestamp: not used
 * @length: length of used data
 * @data:   data bytes buffer
 *
 * Header: canmsg.h
 */
struct canmsg_t {
	int             flags;
	int             cob;
	canmsg_id_t     id;
	canmsg_tstamp_t timestamp;
	unsigned short  length;
	unsigned char   data[CAN_MSG_LENGTH];
};

#else /*CAN_MSG_VERSION_2*/
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif
/* Old, deprecated version of canmsg_t structure */
struct canmsg_t {
	short		flags;
	int		cob;
	canmsg_id_t	id;
	unsigned long	timestamp;
	unsigned int	length;
	unsigned char	data[CAN_MSG_LENGTH];
} PACKED;
#endif /*CAN_MSG_VERSION_2*/

typedef struct canmsg_t canmsg_t;

/**
 * struct canfilt_t - structure for acceptance filter setup
 * @flags:  message flags
 *      %MSG_RTR .. message is Remote Transmission Request,
 *	%MSG_EXT .. message with extended ID, 
 *      %MSG_OVR .. indication of queue overflow condition,
 *	%MSG_LOCAL .. message originates from this node.
 *	there are corresponding mask bits
 *	%MSG_RTR_MASK, %MSG_EXT_MASK, %MSG_LOCAL_MASK.
 *	%MSG_PROCESSLOCAL enables local messages processing in the
 *	combination with global setting
 * @queid:  CAN queue identification in the case of the multiple
 *	    queues per one user (open instance)
 * @cob:    communication object number (not used)
 * @id:     selected required value of cared ID id bits
 * @mask:   select bits significand for the comparation;
 *          1 .. take care about corresponding ID bit, 0 .. don't care
 *
 * Header: canmsg.h
 */
struct canfilt_t {
	int		flags;
	int		queid;
	int		cob;
	canmsg_id_t	id;
	canmsg_id_t	mask;
};

typedef struct canfilt_t canfilt_t;

/* Definitions to use for canmsg_t and canfilt_t flags */
#define MSG_RTR   (1<<0)
#define MSG_OVR   (1<<1)
#define MSG_EXT   (1<<2)
#define MSG_LOCAL (1<<3)
/* If you change above lines, check canque_filtid2internal function */

/* Additional definitions used for canfilt_t only */
#define MSG_FILT_MASK_SHIFT   8
#define MSG_RTR_MASK   (MSG_RTR<<MSG_FILT_MASK_SHIFT)
#define MSG_EXT_MASK   (MSG_EXT<<MSG_FILT_MASK_SHIFT)
#define MSG_LOCAL_MASK (MSG_LOCAL<<MSG_FILT_MASK_SHIFT)
#define MSG_PROCESSLOCAL (MSG_OVR<<MSG_FILT_MASK_SHIFT)

/* Can message ID mask */
#define MSG_ID_MASK ((1l<<29)-1)

#ifdef __cplusplus
} /* extern "C"*/
#endif

#endif /*_CANMSG_T_H*/
