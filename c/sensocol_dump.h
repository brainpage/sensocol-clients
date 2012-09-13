#ifndef _SENSOCOL_DUMP_H
#define _SENSOCOL_DUMP_H
#include "sensocol.h"

/********************************
 * THIS FILE IS PURELY FOR DEBUGGING AND DEVELOPMENT PURPOSES
 * AND SHOULD NOT BE INCLUDED IN AN EMBEDDED SYSTEM
 *********************************/

/**
 * \brief           Print out the contents of a sensocol packet to stdout
 * \param packet    Initialized sensocol packet
 *****/
void sensocol_print(sensocol_packet_t *packet);

/**
 * \brief           Prints out the binary and Hex representation of a byte array
 * \param out       Where to print? (stdout/etc.).
 * \param addr      Address of Bytes
 * \param len       Length to Print
 */
void dump(FILE * out, const char *addr, const long len);
#endif
