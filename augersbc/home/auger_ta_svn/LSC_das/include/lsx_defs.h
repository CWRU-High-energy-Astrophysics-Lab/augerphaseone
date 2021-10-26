#if !defined(__H_)
#define __H_

/*******************************************

  $Author:: guglielmi          $
  $Date:: 2011-04-11 12:24:04 #$
  $Revision:: 913              $

********************************************/

#define BS_TO_PM_PREAMBLE	       	"!BS2PC!"
#define BS_TO_PM_PREAMBLE_LENGTH	7
/* CRC is 32 bits */
#define CRC_LENGTH 4
/* After the preamble, Length (short), Type (char), BsId (char) */
#define LTB_LENGTH 4
#define DATUM_BYTE_LENGTH 1
#define BSID_BYTE_LENGTH 1
#define ETX_BYTE_LENGTH 1

#define ETX 0xFF

#define BS_TO_PM_OVERHEAD (BS_TO_PM_PREAMBLE_LENGTH+LTB_LENGTH+CRC_LENGTH+1)


#define PM_TO_BS_PREAMBLE                "  !PC2BS!"
#define PM_TO_BS_PREAMBL_LENGTH          9

/* The pkt length is NOT included in the length of the pkt ! */
#define PKT_HEADER_LENGTH 7
#define PKT_HEADER_LENGTH_LENGTH 2

/* This is for type, bsid and CRC (not the header, not the length, not the
   End Marker
*/
#define DATA_LENGTH_MODIFIER 6
#endif
