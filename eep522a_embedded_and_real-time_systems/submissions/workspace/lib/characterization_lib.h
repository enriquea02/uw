
/*
 * MODULE: 
 *
 * characterization library header
 *
 * DESCRIPTION: 
 * This library contains functions that are useful for Raspberry Pi 4 device characterization.
 * 
 * ACKNOWLEDGEMENTS:
 * This library borrows from Andrew N. Sloss' prototype.c module developed for EP522A Embedded and Real-Time Systems course
 * 
 */

#ifndef _CHARACTERIZATION_H_
#define _CHARACTERIZATION_H_

/* -------------------------------------------------------------------------- */
/*                                   IMPORTS                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                 Data Types                                 */
/* -------------------------------------------------------------------------- */
typedef uint8_t byte;
typedef uint16_t halfword;
typedef uint32_t word;

/* -------------------------------------------------------------------------- */
/*                                   Globals                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  CONSTANTS                                 */
/* -------------------------------------------------------------------------- */

/* -------------------- Read / Write Speed Test Constants ------------------- */
const int KiB           = 1024;
const int MiB           = (1024*KiB);
const int TEN_MiB       = (10*MiB);
const int HUNDRED_MiB   = (100*MiB)

/* -------------------------------------------------------------------------- */
/*                                  Routines                                  */
/* -------------------------------------------------------------------------- */



#endif