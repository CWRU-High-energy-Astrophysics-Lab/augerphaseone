#if !defined(_CAN_LIST_H_)

#define _CN_LIST_H_

typedef struct {
  int id ;
  char * comment ;
} CAN_LIST ;

CAN_LIST CanList[] = {
  { 0x002, "CAN Buff Stat" },
  { 0x003, "CAN Buff Stat" },
  { 0x0C4, "CAN Buff Stat Req" },
  { 0x0C5, "CAN Buff Stat Req" },
  { 0x0DD, "CGD Start Data 1" },
  { 0x0DF, "CGD Start Data 2" },
  { 0x0DC, "CGD Start Ack" },
  { 0x008, "ChMode Ack" },
  { 0x009, "ChMode Ack" },
  { 0x00A, "ChMode Cmd" },
  { 0x00B, "ChMode Cmd" },
  { 0x006, "ChMode Done" },
  { 0x007, "ChMode Done" },
  { 0x061, "Data Cmd Down" },
  { 0x083, "Data Cmd Down" },
  { 0x010, "Data Cmd Up" },
  { 0x056, "Data Cmd Up" },
  { 0x0D8, "Echo Reply" },
  { 0x0D9, "Echo Reply" },
  { 0x0DA, "Echo Req" },
  { 0x0DB, "Echo Req" },
  { 0x0D0, "GPS 1PPS Stat" },
  { 0x0D1, "GPS 1PPS Stat Req" },
  { 0x0D6, "GPS Date/Time Reply" },
  { 0x0D7, "GPS Date/Time Req" },
  { 0x0D2, "GPS Posit Reply 1" },
  { 0x0D4, "GPS Posit Reply 2" },
  { 0x0D5, "GPS Posit Req" },
  { 0x0CD, "HW Serial Num Req" },
  { 0x0CA, "HW Serial Num Reply" },
  { 0x0CB, "HW Serial Num Reply" },
  { 0x0CC, "HW Serial Num Req" },
  { 0x091, "Maint Cmd Down" },
  { 0x0B3, "Maint Cmd Down" },
  { 0x090, "Maint Cmd Up" },
  { 0x0B2, "Maint Cmd Up" },
  { 0x00E, "NOOP" },
  { 0x00F, "NOOP" },
  { 0x000, "Power status" },
  { 0x001, "Power Status" },
  { 0x00D, "Radio Trans Nack" },
  { 0x00C, "Reset HW Cmd" },
  { 0x0CE, "Routine Monitor Data" },
  { 0x0CF, "Routine Monitor Data" },
  { 0x004, "Stream Ack" },
  { 0x005, "Stream Ack" },
  { 0x0C6, "SW Version Reply" },
  { 0x0C7, "SW Version Reply" },
  { 0x0C8, "SW Version Req" },
  { 0x0C9, "SW Version Req" },
  { 0x085, "Term Data Cmd Down" },
  { 0x058, "Term Data Cmd Up" },
  { 0x0B5, "Term Maint Cmd Down" },
  { 0x0B4, "Term Maint Cmd Up" },
  { 0x0C1, "Wireless Net Stat" },
  { 0x0C2, "Wireless Net Stat Req" },
  { 0x100, NULL }
} ;


#endif
