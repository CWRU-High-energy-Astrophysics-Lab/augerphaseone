/***************************************************

   $Author: lgugli $
   $Date: 2016-08-19 15:49:23 +0000 (Fri, 19 Aug 2016) $
   $Rev: 61 $

***************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

#include "lsx_defs.h"
#include "gpscommon.h"
#include "fe_defs.h"
#include "msgsvr.h"
#include "central_local.h"
#include "download.h"
#include "t3_err.h"
#include "tocdas_names.h"
#include "logfile.h"
#include "gpsutil.h"
#include "slowcontrol.h"
#include "adcs_defs.h"
#include "adcs_conv.h"
#include "timestamp.h"
#include "acq_msg.h"
#include "tpcb.h"
#include "thrh_defs.h"
#include "lrstatus.h"
#include "lr_conv.h"
#include "monitor.h"
#include "linklib.h"

#define PM_VERSION 11

#define GPS_START_TIME 315964800

/*
  simule le Postmaster (Pm)

  en input: un numero de port et un nom de PC

  attend une connection

  Lit des commandes
  Repond par un ACK suivi d'un timeout
  Repond ensuite par un cte rendu d'execution
  Envoie des data au rythme defini dans mkcal.cfg

*/
extern char *optarg ;

int Port = 25200 ;

int Verbose = 0, Debug = 0, ViewT2 = 0 ;
int Accepted = 0, AcqStarted = 0 ;
int MonitShow = 1 ;

char *Options = "p:h:vd?" ;
static 	struct	sockaddr_in	sock_name ;
static int CsSocket ;

short LsId = 0 ;
short LsList[17] ;
int NbLsid = 1 ;
int NbReceived = 0 ;

void CreateSingleListe() ;
unsigned char * DoListe() ;

static int UtcOffset = 0 ;
static char * GpsUtc = "GPS" ;

typedef struct {
  int type ;
  int msg_nb ;
  int size ;
} WAITING_MESSAGE_HEADER ;

typedef struct {
  WAITING_MESSAGE_HEADER header ;
  unsigned char * buf ;
} WAITING_MESSAGE ;

static WAITING_MESSAGE * WaitingHead = NULL, * WaitingTail = NULL ;
#define WAITING_HEAD (void **)&WaitingHead
#define WAITING_TAIL  (void **)&WaitingTail

static time_t RecvdTime = 0 ;

#define PM_FIFO "/tmp/pm_fifo"
FILE * FifoIn = NULL ;
int FdFifo = -1 ;

static unsigned short T3EvId = 0 ;
static int T3Seconds ;
static int T3Micros ;
static char T3MicroRef = 100 ;
static unsigned char T3Delta = 255 ;

static char * PmrcName = NULL ;

static int DownNumber = 0 ;
static short CurDownSlice = 0 ;
static int NbDownSlice = 0 ;
static char DownFileName[256] ;
static char LocalFileName[2048] ;
static int DownLoading = 0 ;

static void Help()
{
#if PM_VERSION>10
  puts( "Pm [options]" ) ;
  printf( "Version V%d\n", PM_VERSION ) ;
#endif
  puts( "Options:" ) ;
  puts( " -h <host>  : IP Host Name" ) ;
  puts( " -p <nnnn>  : TCP Port number (default 10600)" ) ;
  //puts( " -n <name>  : Device controller name" ) ;
  puts( " -v         : Verbose" ) ;
  puts( " -d         : Debug" ) ;
  exit( 1 ) ;
}

void cleanup( int sock )
{
  if ( shutdown( sock, 2) == -1 ) perror( "shutdown" ) ;
  exit( 0 ) ;
}

void GotSignal( int sig ) 
{
  exit( sig ) ;
}


static char * the_date_time( time_t * ttt )
{
  static char the_date[32] ;
  struct tm ttm ;

  ttm = *gmtime( ttt ) ;

  sprintf( the_date, "%04d/%02d/%02d %02d:%02d:%02d",
           ttm.tm_year+1900, ttm.tm_mon+1, ttm.tm_mday,
           ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return the_date ;
}

static char * set_date()
{
  static char the_date[32] ;
  time_t ttt = time( NULL ) ;
  struct tm ttm ;

  ttm = *gmtime( &ttt ) ;

  sprintf( the_date, "%04d%02d%02d_%02d%02d%02d",
           ttm.tm_year+1900, ttm.tm_mon+1, ttm.tm_mday,
           ttm.tm_hour, ttm.tm_min, ttm.tm_sec ) ;

  return the_date ;
}

static void SavePmrc()
{
  FILE * fout ;

  if ( (fout = fopen( PmrcName, "w" )) == NULL ) return ;
  fprintf( fout, "%d\n", T3EvId ) ;
  fclose( fout ) ;
}

static void InitPmrc()
{
  char * home, str[32] ;
  FILE * fin ;
  int evid ;

  /* Open file '~/.pmrc' */
  if ( (home = getenv( "HOME" )) != NULL ) {
    PmrcName = malloc( strlen( home ) + 32 ) ;
    sprintf( PmrcName, "%s/.pmrc", home ) ;
    fin = fopen( PmrcName, "r" ) ;
    if ( fin == NULL ) return ;
    /* Get last T3EvtId */
    fgets( str, 32, fin ) ;
    sscanf( str, "%d", &evid ) ;
    T3EvId = evid ;
    fclose( fin ) ;
  }
  return ;
}

void UploadSave( unsigned char * msg, int length ) ;

static FILE * T2Out = NULL ;
static char * T2FileName = NULL ;
static int SaveT2 = 1 ;
static int LastT2Sec = 0, LastT2Micro = 0 ;

void SaveT2Yes( unsigned char * msg, int nt2 )
{
  int length ;


  if ( T2Out == NULL ) return ;
  fwrite( &RecvdTime, sizeof( int ), 1, T2Out ) ;

  length = nt2*3 + 4 ;
  fwrite( &nt2, sizeof( int ), 1, T2Out ) ;
  fwrite( msg, 1, length, T2Out ) ;
  fflush( T2Out ) ;
}

void ShowT2Yes( unsigned char * msg, int length )
{
  //short length ;
  unsigned char * pmsg ;
  int nbt2, i, k ;

  //length = bytes_to_short( msg ) ;
  pmsg = msg + MESSAGE_HEADER_LENGTH ;

  nbt2 = (length - 9)/3 ;
  LastT2Sec = bytes_to_int( pmsg ) ;
  if ( ViewT2 == 1 )
    printf( "  NbT2: %d, GPS Seconds: %u, %s Date: %s \n", nbt2,
	    LastT2Sec, GpsUtc,
	    Gps2Utc( LastT2Sec, UtcOffset ) ) ;

  if ( ViewT2 == 1 && Verbose ) {
    unsigned char * pmic = pmsg + 4 ;

    /* Dump micros */
    printf( "Micros:\n" ) ;
    for ( k = 1, i = 0 ; i<nbt2 ; i++, pmic += 3 ) {
      printf( "%06d\n", bytes3_to_int( pmic ) ) ; //Prefer padded int here
      if ( (k % 8) == 0 ) printf( "\n" ) ;
      k++ ;
    }
    printf( "\n" ) ;
  }
  unsigned int t2val, tag ;

  t2val = bytes3_to_int( pmsg + 4 + (nbt2-1)*3 ) ;
  tag = t2val & MASK_T2_TAG ;
  /* If tag == GRB_T2_TAG , it is a GRB value, not a normal T2 */
  if ( tag == GRB_T2_TAG ) {
    t2val = bytes3_to_int( pmsg + 4 + (nbt2-2)*3 ) ;
  }
  LastT2Micro = t2val & MASK_T2_MICROS ;

  if ( SaveT2 ) SaveT2Yes( pmsg, nbt2 ) ;
}

void ShowT3Evt( unsigned char * msg, int length )
{
  unsigned char * pmsg ;
  unsigned char * pdata ;
  T3_EVT_MESSAGE *pt3 ;
  T3_EVT_MESSAGE_HEADER *pht3 ;
  int err_idx ;
  int data_length ;
  int evtid ;

  pmsg = msg + MESSAGE_HEADER_LENGTH ;
  pt3 = (T3_EVT_MESSAGE *)pmsg ;
  pdata = pt3->data ;
  pht3 = (T3_EVT_MESSAGE_HEADER *)&pt3->header;

  data_length = length - MESSAGE_HEADER_LENGTH - 
    sizeof(T3_EVT_MESSAGE_HEADER) ;
  err_idx = pht3->error ;
  if ( err_idx >= M_T3_UNKNOWN ) err_idx = M_T3_UNKNOWN ;
  evtid = ntohs( pht3->evtid ) ;

  printf( "   EvtId: %d, Error %d - %s\n",
	  evtid, pht3->error, T3Errors[err_idx] ) ;
  if ( pht3->error == 0 ) {
    /* Save the event data bzipped */
    FILE * fout ;
    char fname[32] ;

    sprintf( fname, "t3_%05d.bz2", evtid ) ;
    LogPrint( LOG_INFO, "Save '%s', size: %d\n", fname, data_length ) ;
    printf( "Save '%s', size: %d\n", fname, data_length ) ;
    fout = fopen( fname, "w" ) ;
    fwrite( pdata, data_length, 1, fout ) ;
    fclose( fout ) ;
  }
}

void ShowGeneric( unsigned char * msg )
{
  //short length ;
  char * pmsg ;

  //length = bytes_to_short( msg ) ;
  pmsg = (char *)msg + MESSAGE_HEADER_LENGTH ;

  printf( "----------------------------------------------------\n" ) ;
  printf( "%s\n", pmsg ) ;
  printf( "----------------------------------------------------\n" ) ;

}

void ShowMonit(  unsigned char * msg )
{
  //short length ;
  unsigned char * pmsg ;
  MONITOR_BLOCK mon_block ;
  int i ;

  //length = bytes_to_short( msg ) ;
  pmsg = msg + MESSAGE_HEADER_LENGTH ;

  memcpy( &mon_block, pmsg, sizeof( MONITOR_BLOCK ) ) ;

  if ( mon_block.time == 0 ) {
    printf( "   No Monitor Data Available Yet\n" ) ;
    return ;
  }

  if ( MonitShow ) {
    printf( "GPS Seconds: %u (offset: %d), %s Date: %s, Count: %d\n",
	    mon_block.time, mon_block.gps_utc_offset,
	    GpsUtc,
	    Gps2Utc( mon_block.time, UtcOffset ), mon_block.count ) ;
    UtcOffset = mon_block.gps_utc_offset ;
    GpsUtc = "UTC" ;

    for( i = 0 ; i<MAX_NB_ADC ; i++ ) {
      printf( "%12s: %.3f (%.3f %.3f)\n",
	      AdcsConv[i].name, (float)mon_block.adc[i]/SCALE_FACTOR,
	      mon_block.adc_min[i]/SCALE_FACTOR,
	      mon_block.adc_max[i]/SCALE_FACTOR ) ;
    }
    /* Now Th Rh */
    printf( "%12s: %.1f (%.1f %.1f)\n", PCB_TEMPERATURE_NAME,
	    (float)mon_block.pcb_th/SCALE_FACTOR,
	    (float)mon_block.pcb_th_min/SCALE_FACTOR,
	    (float)mon_block.pcb_th_max/SCALE_FACTOR ) ;
    printf( "%12s: %.1f (%.1f %.1f)\n", PCB_HYGROMETRY_NAME,
	    (float)mon_block.pcb_rh/SCALE_FACTOR,
	    (float)mon_block.pcb_rh_min/SCALE_FACTOR,
	    (float)mon_block.pcb_rh_max/SCALE_FACTOR ) ;

    printf( "TPCB Load Voltage: %.1f V, Current: %.f mA\n",
	    (mon_block.tpcb_load_voltage*LOAD_VOLTAGE_PER_BIT)/1000.,
	    mon_block.tpcb_load_current*LOAD_CURRENT_PER_BIT ) ;
    printf( "TPCB Panel Voltage: %.1f V, Current: %.f mA\n",
	    (mon_block.tpcb_panel_voltage*PANEL_VOLTAGE_PER_BIT)/1000.,
	    mon_block.tpcb_panel_current*PANEL_CURRENT_PER_BIT ) ;
    printf( "TPCB Pressure: %.1f hPa, Temperature: %.1f C\n",
	    mon_block.tpcb_pressure*PRESSURE_MILLIBAR_PER_BIT,
	    mon_block.tpcb_temperature*TEMP_DEGRE_PER_BIT ) ;
    /* Now Local Radio Data */
    for ( i = 0 ; i<MAX_LR_MONIT_DATA ; i += 2 ) {
      int k, j ;
      for ( j = i, k = 0 ; j<MAX_LR_MONIT_DATA && k < 2 ; j++, k++ )
	printf( "%s: %d [%x], ", LrConv[j].name, mon_block.lr_data[j],
		mon_block.lr_data[j]) ;
      printf( "\n" ) ;
    }
  }

  FILE * mout ;
  mout = fopen( "monit.log", "a+" ) ;
  fprintf( mout, "*********** %u %d\n", mon_block.time,
	   mon_block.gps_utc_offset ) ;
  for( i = 0 ; i<MAX_NB_ADC ; i++ ) {
    fprintf( mout, "%s %u %.3f %.3f %.3f\n",
	    AdcsConv[i].name, mon_block.time,
	     (float)mon_block.adc[i]/SCALE_FACTOR,
	    mon_block.adc_min[i]/SCALE_FACTOR,
	    mon_block.adc_max[i]/SCALE_FACTOR ) ;
  }
  /* Now Th Rh */
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", PCB_TEMPERATURE_NAME,
	   mon_block.time,
	   (float)mon_block.pcb_th/SCALE_FACTOR,
	   (float)mon_block.pcb_th_min/SCALE_FACTOR,
	   (float)mon_block.pcb_th_max/SCALE_FACTOR ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", PCB_HYGROMETRY_NAME,
	   mon_block.time,
	   (float)mon_block.pcb_rh/SCALE_FACTOR,
	   (float)mon_block.pcb_rh_min/SCALE_FACTOR,
	   (float)mon_block.pcb_rh_max/SCALE_FACTOR ) ;

  fprintf( mout, "%s %u %.1f %.1f %.1f\n", LOAD_VOLTAGE_NAME, mon_block.time,
	   (mon_block.tpcb_load_voltage*LOAD_VOLTAGE_PER_BIT)/1000.,
	   (mon_block.tpcb_load_voltage*LOAD_VOLTAGE_PER_BIT)/1000.,
	   (mon_block.tpcb_load_voltage*LOAD_VOLTAGE_PER_BIT)/1000. ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", LOAD_CURRENT_NAME, mon_block.time,
	   mon_block.tpcb_load_current*LOAD_CURRENT_PER_BIT,
	   mon_block.tpcb_load_current*LOAD_CURRENT_PER_BIT,
	   mon_block.tpcb_load_current*LOAD_CURRENT_PER_BIT ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", PANEL_VOLTAGE_NAME,
	   mon_block.time,
	   (mon_block.tpcb_panel_voltage*LOAD_VOLTAGE_PER_BIT)/1000.,
	   (mon_block.tpcb_panel_voltage*LOAD_VOLTAGE_PER_BIT)/1000.,
	   (mon_block.tpcb_panel_voltage*LOAD_VOLTAGE_PER_BIT)/1000. ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", PANEL_CURRENT_NAME,
	   mon_block.time,
	   mon_block.tpcb_panel_current*LOAD_CURRENT_PER_BIT,
	   mon_block.tpcb_panel_current*LOAD_CURRENT_PER_BIT,
	   mon_block.tpcb_panel_current*LOAD_CURRENT_PER_BIT ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", TPCB_PRESSURE_NAME, mon_block.time,
	   mon_block.tpcb_pressure*PRESSURE_MILLIBAR_PER_BIT,
	   mon_block.tpcb_pressure*PRESSURE_MILLIBAR_PER_BIT,
	   mon_block.tpcb_pressure*PRESSURE_MILLIBAR_PER_BIT ) ;
  fprintf( mout, "%s %u %.1f %.1f %.1f\n", TPCB_TEMPERATURE_NAME,
	   mon_block.time,
	   mon_block.tpcb_temperature*TEMP_DEGRE_PER_BIT,
	   mon_block.tpcb_temperature*TEMP_DEGRE_PER_BIT,
	   mon_block.tpcb_temperature*TEMP_DEGRE_PER_BIT ) ;

  for( i = 0 ; i<MAX_LR_MONIT_DATA ; i++ )
      fprintf( mout, "%s %u %d %d %d\n", LrConv[i].name, mon_block.time,
	     mon_block.lr_data[i], mon_block.lr_data[i],
	     mon_block.lr_data[i] ) ;
  fclose( mout ) ;
}

void ShowMready( unsigned char * msg )
{
  //short length ;
  unsigned char * pmsg ;
  M_READY_MESSAGE ready ;

  //length = bytes_to_short( msg ) ;
  pmsg = msg + MESSAGE_HEADER_LENGTH ;

  memcpy( &ready, pmsg, sizeof( M_READY_MESSAGE ) ) ;
  printf( "Run Status: %x, Poweron_reset: %x, SW Version: %x, Config Version: %d\n",
	  ready.run_status, ready.poweron_reset,
	  ntohl( ready.software_version ), ntohl( ready.config_version ) ) ;

  UtcOffset = ntohs(ready.utc_offset) ;
  GpsUtc = "UTC" ;
  printf( "Northing: %d, Easting: %d, Height: %d\n",
	  ntohl( ready.northing ), ntohl( ready.easting ),
	  ntohl( ready.height ) ) ;
  printf( "Cur GPS Seconds: %u, %s Time: %s. Offset: %d\n",
	  (unsigned int)ntohl(ready.cur_time), GpsUtc,
	  Gps2Utc((unsigned int)ntohl(ready.cur_time), UtcOffset ),
	  UtcOffset ) ;
}

void dump_pkt( unsigned char * buf, int mln )
{
  int j ;

  if ( Verbose ) for( j = 0 ; j<mln ; j+=16 ) {
    int k = j, m = j+16 ;
    char str[128] ;

    if ( m >= mln ) m = mln ;
    *str = '\0' ;
    for( ; k<m ; k++, buf++ ) {
      char s[8] ;
      sprintf( s, " %02x", *buf ) ;
      strcat( str, s ) ;
    }
    LogPrint( LOG_INFO, " %s\n", str ) ;
  }

}

char * find_msg_name( int type )
{
  msg_names_t * pname ;

  for( pname = ToCdasMsgNames ; pname->name != NULL ; pname++ )
    if ( pname->value == type ) return pname->name ;

  return NULL ;
}

void decode_msg( unsigned char * msg, int mln, int type )
{
  unsigned char * pmsg ;
  char * p ;
  RecvdTime = time( NULL ) ;

  if ( (p = find_msg_name( type )) != NULL ) {
    int do_print = 1 ;

    if ( type == M_T2_YES && ViewT2 == 0 ) do_print = 0 ;
    if ( do_print ) {
      printf( "\n>>>> Id: %d - %s - Msg Type %d [%s] (%d bytes)\n",
	      LsId,
	      the_date_time(&RecvdTime ), type, p, mln ) ;
      //printf( "         Received %s", ctime( &RecvdTime ) ) ;
    }
    LogPrint( LOG_INFO,
	      "decode_msg - Message Received - type %d [%s], %d bytes\n",
	      type, p, mln ) ;
  }
  else {
    printf( "\n >>>> Id: %d Message type %d Unknown, %d bytes\n",
	    LsId, type, mln ) ;
    LogPrint( LOG_WARNING,
	      "decode_msg - Message type %d Unknown, %d bytes\n", type, mln ) ;
    return ;
  }

  switch( type ) {
  case M_SHELL_CMD_ACK:
    {
      unsigned int ack ;
      pmsg = msg + MESSAGE_HEADER_LENGTH ;
      ack =  ntohl( bytes_to_int( pmsg ) ) ;
      printf( "  Ack: 0x%x (%u)\n", ack, ack ) ;
      LogPrint( LOG_INFO, "Shell Cmd Ack: %x (%u)\n", ack, ack) ;
    }
    break ;
  case M_UPLOAD_SEND:
    printf( "  Ack: %d bytes\n", mln ) ;
    fflush( stdout ) ;
    LogPrint( LOG_INFO, "\nUpload Ack: %d bytes\n", mln ) ;
    /* Save the data to file */
    UploadSave( msg, mln ) ;
    break ;
  case M_T2_YES:
    ShowT2Yes( msg, mln ) ;
    break ;
  case M_READY:
    ShowMready( msg );
    break ;
  case M_GENERIC_STR:
    ShowGeneric( msg );
    break ;
  case M_MONIT_SEND:
    ShowMonit( msg ) ;
    break ;
  case M_DOWNLOAD_ACK:
    {
      unsigned short miss ;
      pmsg = msg + MESSAGE_HEADER_LENGTH ;
      pmsg += 2 ;
      miss = bytes_to_short( pmsg ) ;
      printf( "    Missing Slices: %d\n", bytes_to_short( pmsg ) ) ;
      if ( miss == 0xFFFF ) {
	printf( "   Download Error (cant open remote file)\n" ) ;
	DownLoading = 0 ;
      }
      else if ( CurDownSlice == NbDownSlice ) {
	printf( "   Download Finished\n" ) ;
	DownLoading = 0 ;
      }
      else printf( "   Download Acq\n" ) ;
    }
    break ;
  case M_T3_EVT:
    ShowT3Evt( msg, mln ) ;
    break ;
  default:
    printf( "         NOT Yet Implemented\n" ) ;
    break ;
  }
}

WAITING_MESSAGE * find_waiting_msg( int msg_nb )
{
  WAITING_MESSAGE * cur ;

  for( cur = WaitingHead ; cur != NULL ; cur = LinkNext( cur ) ) {
    if ( Verbose )
      printf( "Check msg_nb %d vs cur-msg_nb %d\n",
	      msg_nb,  cur->header.msg_nb ) ;
    if ( cur->header.msg_nb == msg_nb ) {
      if ( Verbose ) printf( " ** Found Waiting Message\n" ) ;
      return cur ;
    }
  }
  printf( "Cant find waiting msg #%d\n", msg_nb ) ;
  return NULL ;
}

/** 
 * Decode a LS Frame into messages. If a message is sliced, at the first slice
 * a "wait message" is created. At the next slices (if any ) the slice is
 * added to the waiting message. At the last slice, the message is completed
 * and the message is decoded. The process is made for each message in the
 * frame.
 * 
 * A CS packet is made as follows:
 *
 *    - CS Pkt
 *       - Preambule = "!BS2PC!"
 *       - Length (2 bytes): la longueur de ce qui suit SAUF elle-meme
 *       - Type ( 1 byte): 0x44 = 'D'
 *       - BsID ( 1 Byte)
 *       - LS Frame
 *       - CRC (4 bytes): sur tout depuis la length incluse (mais pas le CRC)
 *       - ETX (1 byte): 0xFF
 *         NOTE: on ne verifie PAS le CRC, c'est totalement superfetatoire !
 *    - LS Frame
 *       - Length ( 2 bytes): toute la frame SAUF elle-meme
 *       - FrameNb (1 byte): Numero de la frame
 *       - Reserved (1 byte): 0x00
 *       - LsId (2 bytes):
 *       - Nb de Messages (1 byte): Nb de messages dans la frame.
 *       - Les LS Messages:
 *    - LS Message
 *       - Length (2 bytes): tout le message y compris la length
 *       - Slice (1 byte): slice completion (2 bits) + Slice number
 *       - Type ( 1 byte): Message type
 *       - Message Nb (1 byte): message nb (6 bits) + version (2 bits)
 *       - Le contenu du message (Length - 5 bytes)
 *
 * @param buf The actual frame
 */
void DecodePkt( unsigned char * frame )
{
  int nmsg, i ;
  short length ;
  unsigned char pktnb ;
  unsigned char * buf ;

  length = bytes_to_short( frame ) + 2 ;
  pktnb = *(frame+2) ;

  short prev_id = LsId ;
  LsId = bytes_to_short( frame+4 ) ;
  if ( LsId == 0 ) {
    /* Fake message from CS to wakeup Pm
       Ignore it
    */
    return ;
  }
  if ( prev_id != LsId ) {
    CreateSingleListe() ;
    prev_id = LsId ;
  }

  buf = frame + 6 ;		/**< Beginning of  messages in the frame */
  nmsg = *buf++ ;		/**< Nb of messages in the frame */

  LogPrint( LOG_INFO, ">>> DecodePkt - length: %d, Pkt #: %u, lsid: %d\n",
	    length, pktnb, LsId ) ;
  LogPrint( LOG_INFO, "   Nb msg: %d\n", nmsg ) ;
  if ( Verbose )printf( "\n   Got Pkt %d bytes from LsId %d, Nb msg: %d\n",
			 length, LsId, nmsg ) ;

  /* buf1 points to the message header (ie the size of the msg) of 
     the 1st message */
  unsigned char * buf1 = buf ;

  for( i = 0 ; i < nmsg ; i++ ) {
    short mln = bytes_to_short( buf1 ) ; /**< Length of the message */
    short dmln = mln - MESSAGE_HEADER_LENGTH ; /**< Length of the data of the
						message */
    int nsize ;
    unsigned char type = *(buf1+3) ;
    unsigned int slice, msg_nb ;

    msg_nb = (*(buf1+4) & 0xFC ) >> 2 ;

    if ( Verbose )
      printf( "  Msg #%d: Size: %d, Slice: %02x, Type: %02x, Number: %02x\n",
	      i+1, mln, *(buf1+2), *(buf1+3), *(buf1+4) ) ;
    dump_pkt( buf1 + MESSAGE_HEADER_LENGTH, dmln ) ;

    /* Now handle the slices if any:
       if FIRST slice, create a message.
       else if NEXT slice or LAST slice, add data to the waiting message.

       if LAST slice or ALL message (ie message complete), decode the message
       if LAST slice, delete the waiting message.

       NOTE: as it is possible to have several messages sliced in the same
             frame, the "waiting messages" are put into a linked list. The
	     messages in the list are searched for (LAST or NEXT slice) by the
	     message number. This supposes that no more than 64 different
	     messages can be sent at the same moment.
    */
    slice = *(buf1+2) ;
    int completion ;

    completion = (slice & COMPLETION_MASK ) ;
    if ( completion == COMPLETION_FIRST ) {
      /* Create a new waiting event */
      WAITING_MESSAGE * new = NULL ;

      if ( Verbose )
	printf( "   FIRST Slice, msg nb: %d - size: %d\n", msg_nb, mln ) ;
      LogPrint( LOG_INFO, " FIRST Slice, msg nb: %d, size: %d\n", msg_nb,
		mln ) ;

      /* Create the waiting message */
      new = LinkCreate( sizeof( WAITING_MESSAGE ) ) ;
      if ( new == NULL ) {
	printf( "Can't Create Waiting Message\n" ) ;
	LogPrint( LOG_FATAL, "Can't Create Waiting Message\n" ) ;
	exit( 0 ) ;
      }
      /* Allocate memory for the content of the message */
      new->buf = malloc( mln ) ;
      if ( new->buf == NULL ) {
	printf( "Can't Allocate Mem for new Message\n" ) ;
	LogPrint( LOG_FATAL, "Can't Allocate Mem for new Message\n" ) ;
	exit( 0 ) ;
      }

      /* Copy the message slice to the allocated area.
	 For the FIRST slice, include the msg header */
      memcpy( new->buf, buf1, mln ) ;
      new->header.type = type ;
      new->header.size = mln ;
      new->header.msg_nb = msg_nb ;

      /* Add the message to the linked list */
      LinkAddHead( new, WAITING_HEAD, WAITING_TAIL ) ;
    }
    else if ( completion == COMPLETION_NEXT ) {
      LogPrint( LOG_INFO, "NEXT Slice, msg_nb = %d, size: %d\n", msg_nb,
		dmln ) ;
      if ( Verbose )
	printf( "   NEXT Slice: %d, msg_nb = %d\n", slice & 0x3F, msg_nb ) ;
      /* Search the waiting message with message nb */
      WAITING_MESSAGE * old ;

      old = find_waiting_msg( msg_nb ) ;
      if ( old == NULL ) {
	/* Message not found ! Big problem ! ignore the message */
	LogPrint( LOG_WARNING, "Could not find a waiting message nb %d\n",
		  msg_nb ) ;
	buf1 += mln ;
	continue ;
      }
      /* Increase data buffer size */
      int pos = old->header.size ;

      nsize = old->header.size + dmln ;
      /* Expand area for the message */
      unsigned char * pr ;
      pr = realloc( old->buf, nsize ) ;
      if ( pr == NULL ) {
	printf( "Can't Expand Mem for old Message\n" ) ;
	LogPrint( LOG_FATAL, "Can't Expand Mem for old Message\n" ) ;
	exit( 0 ) ;
      }
      old->buf = pr ;

      old->header.size = nsize ;

      /* Copy additional message data (no header) */
      memcpy( old->buf + pos, buf1 + MESSAGE_HEADER_LENGTH, dmln ) ;
    }
    else if ( completion == COMPLETION_LAST ) {
      LogPrint( LOG_INFO, "LAST Slice, msg_nb = %d, size: %d\n", msg_nb,
       		dmln ) ; 
      if ( Verbose ) printf( "   LAST Slice, msg_nb = %d\n", msg_nb ) ;
      /* Find the waiting msg */
      WAITING_MESSAGE * old ;

      old = find_waiting_msg( msg_nb ) ;
      if ( old == NULL ) {
	/* Message not found ! Big problem ! ignore the message */
	LogPrint( LOG_WARNING, "Could not find the waiting message nb %d\n",
		  msg_nb ) ;
	buf1 += mln ;
	continue ;
      }
      int pos = old->header.size ;

      /* Increase data buffer size */
      nsize = old->header.size + dmln ;
      LogPrint( LOG_INFO, "Total Size: %d\n", nsize ) ;
      unsigned char * pr ;
      pr = realloc( old->buf, nsize ) ;
      if ( pr == NULL ) {
	printf( "Can't Expand Mem for old Message\n" ) ;
	LogPrint( LOG_FATAL, "Can't Expand Mem for old Message\n" ) ;
	exit( 0 ) ;
      }
      old->buf = pr ;
      old->header.size = nsize ;

      /* Copy additional message data (no header) */
      memcpy( old->buf + pos, buf1+MESSAGE_HEADER_LENGTH, dmln ) ;

      /* Decode the msg */
      decode_msg( old->buf, old->header.size, type ) ;

      /* Free the data area */
      free( old->buf ) ;
      /* Unlink and Delete the waiting msg */
      LinkUnlink( old ) ;
      LinkDelete( old ) ;
    }
    else if ( completion == COMPLETION_ALL ) {
      LogPrint( LOG_INFO, "ALL Slice, msg_nb = %d\n", msg_nb ) ;
      if ( Verbose ) printf( "   ALL Slice, msg_nb = %d\n", msg_nb ) ;
      /* Just decode the message */
      /* CAVEAT: The final size of the message is NOT in the message header,
	 the header contains only the size of the first slice !!!!
      */
      decode_msg( buf1, mln, type ) ;
    }
    buf1 += mln ;		/**< Point to the header of the next message */
  }
  unsigned int crc = 0 ;

  crc = bytes_to_int( buf1 ) ;
  buf1 += 4 ;
  if ( Verbose ) LogPrint( LOG_INFO, "CRC: %x, ETX: %02x\n", crc, *buf1 ) ;
}

#define PREAMBL_COUNT_MAX 500
int get_preambl()
{
  /* 
     read 1 by 1 until prambl found.
  */
  char * expected = BS_TO_PM_PREAMBLE ;
  char preambl[8], *pp, c ;
  int step = 0, found = 0 ;
  int count = 0 ;
  int err = 0 ;

  memset( preambl, 0, 8 ) ;
  pp = preambl ;

  for( ; found != 1 && count < PREAMBL_COUNT_MAX ; count++ ) {
    if ( read( CsSocket, &c, 1 ) != 1 ) {
      LogPrint( LOG_ERROR, "Error while reading preambl: %s\n",
		strerror( errno ) ) ;
      return -1 ;
    }
    switch ( step ) {
    case 0:
      /* expect leading '!' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
		  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
	return 0 ;
      }
      break ;
    case 1:
      /* expect 'B' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    case 2:
      /* expect 'S' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    case 3:
      /* expect '2' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    case 4:
      /* expect 'P' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    case 5:
      /* expect 'C' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	step++ ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    case 6:
      /* expect trailing '!' */
      if ( c == expected[step] ) {
	*pp++ = c ;
	found = 1 ;
      }
      else {
	//LogPrint( LOG_WARNING, "'%c' expected, got '%02x'\n",
	//  expected[step], c ) ;
	pp = preambl ;
	step = 0 ;
	err++ ;
      }
      break ;
    }
  }
  if ( found == 1 ) {
    LogPrint( LOG_INFO, "Got Preambl '%s'\n", preambl ) ;
    if ( err != 0 )
      printf( " \a\a\a******** Preambl Error Found ********\n" ) ;
    return 1 ;
  }
  else {
    LogPrint( LOG_ERROR, "Preambl NOT found\n" ) ;
    return 0 ;
  }
}

/** 
 * Read a CS packet, check it, and extract the LS frame.
 * 
 * 
 * @return The nb of bytes, 0 if bad preamble, -1 if error
 */
int GetBsxMsg()
{
  /**< The CS pkt max size is the max size of an LS frame + the overhead
     of the CS packet */
  unsigned char buf[TO_CDAS_PKT_SIZE + BS_TO_PM_OVERHEAD + 16] ;
  short length ;
  int n ;
  char datum, bsid ;
  struct timeval tm ;
  /*
    1. get preamble "!BS2PM!"
    2. get the size
    3. get the rest
  */
  gettimeofday( &tm, NULL ) ;
  LogPrint( LOG_INFO, ">>> GetBsxMsg at %d.%06d\n", tm.tv_sec, tm.tv_usec ) ;

  if ( ( n = get_preambl()) != 1 ) {
    return n ;
  }

  n = read( CsSocket, &length, 2 ) ;
  if ( n <= 0 ) return -1 ;
  length = ntohs( length ) ;
  LogPrint( LOG_INFO, "Got length: %d [%x]\n", length, length ) ;
  /* Length does not include length, Datum, BsId and ETX */
  n = read( CsSocket, &datum, 1 ) ;
  if ( n <= 0 ) return -1 ;
  n = read( CsSocket, &bsid, 1 ) ;
  if ( n <= 0 ) return -1 ;
  n = read( CsSocket, buf, length - 2 ) ; /* Read data, CRC and ETX */
  if ( n <= 0 ) return -1 ;
  LogPrint( LOG_INFO, "Datum: %c, BsId: %d, Left: %d\n", datum, bsid, n ) ;

  DecodePkt( buf ) ;
  NbReceived++ ;

  return n ;
}

int GetConnectMsg()
{
  int len, length ;
  unsigned char buf[1024] ;

  len = read( CsSocket, &length, sizeof( length ) ) ;
  length = ntohl( length ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Got length %d bytes\n", length ) ;

  len = read( CsSocket, buf, length-sizeof( length ) ) ;
  if ( len > 0 && Verbose ) {
    printf( "GetConnectMsg: Got Connect Msg %d bytes\n", len ) ;
    LogPrint( LOG_INFO, "Got Connect Msg %d bytes\n", len ) ;
  }
  return len ;
}

int SendAckMsg()
{
  unsigned int Ack[28] ;

  int len, size ;
  size = sizeof( Ack ) ;
  len = write( CsSocket, Ack, size ) ;
  if ( len != size ) {
    printf(
	     "SendAckMsg: Write error, %d bytes written, should be %d\n",
	     len, size ) ;
    cleanup( CsSocket ) ;
  }
  else if ( Verbose ) {
    printf( "Sent Ack, %d bytes\n", size ) ;
  }
  return len ;

}

int SendBsxMsg( unsigned char * msg, int length )
{
  unsigned char * pkt ;
  unsigned char *ppkt ;
  short pkt_size ;
  unsigned int the_crc ;

  pkt_size = length +  PM_TO_BS_PREAMBL_LENGTH + LTB_LENGTH + CRC_LENGTH + 1 ;
  pkt = malloc( pkt_size + 32 ) ;

  ppkt = pkt ;
  memcpy( ppkt, PM_TO_BS_PREAMBLE, PM_TO_BS_PREAMBL_LENGTH ) ;
  ppkt += PM_TO_BS_PREAMBL_LENGTH ;
  /* Length does not include preamble nor itself */
  ppkt = short_to_bytes( ppkt, pkt_size - PM_TO_BS_PREAMBL_LENGTH - 2 ) ;
  *ppkt++ = 'D' ;
  *ppkt++ = 0 ; // BsuId, dont care
  memcpy( ppkt, msg, length ) ;
  ppkt += length ;

#if 0
  /* Now add the CRC */
  the_crc = CRC_32bit( ppkt + BS_TO_PM_PREAMBLE_LENGTH, length + 4 ) ;
#else
  /* Dont mind the CRC ! */
  the_crc = 0 ;
#endif
  ppkt = int_to_bytes( ppkt, the_crc ) ;
  /* And the End of pkt marker */
  *ppkt = ETX ;

  // Send
  int len ;
  len = write( CsSocket, pkt, pkt_size ) ;

  if ( len != pkt_size ) {
    LogPrint( LOG_INFO, "Write to socket Error: %s\n",
	      strerror( errno ) ) ;
    cleanup( CsSocket ) ;
  }
  else {
    time_t now = time( NULL ) ;
    printf( "Pkt sent to CS, %d bytes, payload: %d bytes - %s\n",
	    len, length, the_date_time( &now ) ) ;
    if ( Verbose ) {
      LogPrint( LOG_INFO, "Pkt sent to CS, %d bytes, payload: %d bytes\n",
		len, length ) ;
      LogPrint( LOG_INFO, "   CRC: %x\n", the_crc ) ;
    }
  }

  free( pkt ) ;

  return len ;

}

void sendEmptyMsg( int type )
{
  unsigned char msg[150], *pmsg ;
  short size, pkt_size ;

  size = 0 ; /* 0 length message */
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;

  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  /* The message */
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = type ;
  *pmsg++ = 0 ; // Version = 0

  LogPrint( LOG_INFO, "Send %d msg - length: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  SendBsxMsg( msg, pkt_size ) ;
}

void Reboot()
{
  sendEmptyMsg( M_REBOOT ) ;
  LogPrint( LOG_INFO, "Sent Reboot Message\n" ) ;
  printf( "Sent Reboot Message\n" ) ;
}

void SendMonitReq()
{
  sendEmptyMsg( M_MONIT_REQ ) ;
  LogPrint( LOG_INFO, "Sent MONIT_REQ Message\n" ) ;
  printf( "Sent MONIT_REQ Message\n" ) ;
}

void SendWakeup()
{
  sendEmptyMsg( M_WAKEUP ) ;
  LogPrint( LOG_INFO, "Sent M_WAKEUP Message\n" ) ;
  printf( "Sent M_WAKEUP Message\n" ) ;
}

void BuildShellCmd( char * cmd, int type )
{
  unsigned char msg[150], *pmsg ;
  short size, pkt_size ;

  size = strlen( cmd ) + 1 ; // includes the final '\0'
  LogPrint( LOG_INFO, "BuildShellCmd - '%s', Size: %d\n", cmd, size ) ;
  printf( "\n++++ Shell Cmd '%s'\n", cmd ) ;

  /* Format du paquet vers LS:
     length: 2 bytes
     rsvrd: 2 bytes
     destination: lsid 2 bytes (si 1 lsid ou broadcast)
     **** Suppressed: nbmsg: 1 byte
     puis le message:
     length: 2 bytes
     rsvrd: 1 byte
     slice: 1 byte
     type: 1 byte
     version: 1 byte
     et les data
  */
  /* mettre le lsid au bon endroit */
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;
  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message

  /* The message */
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = type ;
  *pmsg++ = 0 ; // Version = 0
  memcpy( pmsg, cmd, size ) ;

  LogPrint( LOG_INFO, "Send msg length: %d, total: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  SendBsxMsg( msg, pkt_size ) ;
}

void GenericMsg( char * cmd )
{
  char *fcmd ;

  fcmd = malloc( strlen( cmd ) + 64 ) ;
  sprintf( fcmd, "genmsg --msg=\"`%s 2>&1`\"", cmd ) ;
  if ( Verbose ) printf( "Shell Cmd: '%s'\n", fcmd ) ;
  BuildShellCmd( fcmd, M_SHELL_CMD) ;
  free( fcmd ) ;
}

void ShellCmd( char * cmd )
{
  BuildShellCmd( cmd, M_SHELL_CMD) ;
}

void SrvRestart()
{
  BuildShellCmd( "srv restart &", M_SHELL_CMD ) ;
}

void AcqRestart()
{
  BuildShellCmd( "das restart &", M_SHELL_CMD ) ;
}

void StartStopAcq( int start )
{
  if ( start == 1 ) {
    BuildShellCmd( "das start", M_SHELL_CMD ) ;
    AcqStarted = 1 ;
  }
  else {
    BuildShellCmd( "das stop", M_SHELL_CMD ) ;
    AcqStarted = 0 ;
  }
}

static void StartSaveTriggers()
{
  if ( T2Out != NULL ) return ; /* Already saving */
  else  {
    T2FileName = malloc( 128 ) ;
    sprintf( T2FileName, "t2_%s.dat", set_date() ) ;
    printf( "\nT2 File: '%s', ViewT2 = %d\n", T2FileName, ViewT2 ) ;
    T2Out = fopen( T2FileName, "w" ) ;
    free( T2FileName ) ;
  }
}

static void StopSaveTriggers()
{
  if ( T2Out == NULL ) return ; /* Already Stopped */
  else  {
    fclose( T2Out ) ;
    T2Out = NULL ;
  }
}

static void StartTrigger()
{
  if ( T2Out != NULL ) {
    fclose( T2Out ) ;
    T2Out = NULL ;
  }
  else {
    T2FileName = malloc( 128 ) ;
    sprintf( T2FileName, "t2_%s.dat", set_date() ) ;
    printf( "\nT2 File: '%s', ViewT2 = %d\n", T2FileName, ViewT2 ) ;
    T2Out = fopen( T2FileName, "w" ) ;
    free( T2FileName ) ;
  }
  BuildShellCmd( "stop -12345 control", M_SHELL_CMD ) ;
}

void StopTrigger()
{
  BuildShellCmd( "stop -12346 control", M_SHELL_CMD ) ;
  if ( T2Out != NULL ) {
    fclose( T2Out ) ;
    T2Out = NULL ;
  }
}

void strip_down_name( char * to, char * from )
{
  char * p ;

  if ( ( p = strrchr( from, '/' )) == NULL ) strcpy( to, from ) ;
  else strcpy( to, p+1 ) ;

}

/** 
 * First Download command. The message contains the file name, the number
 * of slices, the total length of the file. Upon reception of the Ack the 
 * slices are sent, one per second (see DownloadSlice).
 * 
 */
void DownloadCmd( char * cmd )
{
  char flength ;
  unsigned char msg[150], *pmsg ;
  short size, pkt_size ;
  short nb_of_slice ;

#if 0
  char fname[128] ;

  /* Get file Name */
  printf( "Enter Local File Name: " ) ;
  fgets( fname, 2048, stdin ) ;
  fname[strlen(fname)-1] = '\0' ;
  strcpy( LocalFileName, fname ) ;
#else
  char * pcmd = cmd ;

  pcmd = strchr( pcmd, ' ' ) ;
  sscanf( pcmd, "%s", LocalFileName ) ;
  printf( "Local File Name <%s>\n", LocalFileName ) ;
#endif
  if ( access( LocalFileName, 0 ) != 0 ) {
    printf( " ****** UNKNOWN File '%s'\n", LocalFileName ) ;
    return ;
  }

  strip_down_name( DownFileName, LocalFileName ) ;

#if 0
  printf( "Enter Remote File Name [%s] : ", DownFileName ) ;
  fgets( fname, 128, stdin ) ;
  fname[strlen(fname)-1] = '\0' ;
  if ( *fname != '\0' ) {
    flength = strlen( fname ) + 1 ;
    strcpy( DownFileName, fname ) ;
  }
  else flength = strlen( DownFileName ) ;
#else
  if ( (pcmd = strchr( pcmd+1, ' ') ) != NULL ) {
    sscanf( pcmd, "%s", DownFileName ) ;
  }
  flength = strlen( DownFileName ) ;
#endif

  printf( "Local File: %s, Remote: '%s'\n", LocalFileName, DownFileName ) ;

  /* Compute Nb of slices */
  struct stat sbuf ;

  stat( LocalFileName, &sbuf ) ;
  NbDownSlice = ((sbuf.st_size -1)/DOWN_MAX_SLICE_LENGTH) + 1 ;
  CurDownSlice = 0 ;
  nb_of_slice = NbDownSlice ;
  LogPrint( LOG_INFO, "File size: %d, nb slice: %d\n", (int)sbuf.st_size,
	    NbDownSlice ) ;
  printf( "File Size: %d, nb slices: %d\n", (int)sbuf.st_size,
	  NbDownSlice ) ;

  /* Prepare Pkt */
  size = sizeof( M_DOWNLOAD_MESSAGE ) - 2 + flength ;
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;
  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = M_DOWNLOAD ;
  *pmsg++ = 0 ; // Version = 0

  /* The message */
  M_DOWNLOAD_MESSAGE * pdown = (M_DOWNLOAD_MESSAGE *)pmsg ;
  DownNumber++ ;
  pdown->down_nb = htons( DownNumber ) ;
  pdown->slice_nb = 0xFFFF ;
  pdown->slice_length = htons( (short)DOWN_MAX_SLICE_LENGTH ) ;
  pdown->slice_expected = htons( nb_of_slice ) ;
  pdown->file_name_length = htons( flength ) ;
  strcpy( pdown->fname, DownFileName ) ;

  LogPrint( LOG_INFO,
	    "DownNb: %d, SliceExpected: %d, Flength: %d, File '%s'\n",
	    ntohs( pdown->down_nb ), ntohs( pdown->slice_expected),
	    ntohs( pdown->file_name_length),
	    pdown->fname ) ;
  /* Send Pkt */
  LogPrint( LOG_INFO, "Send msg length: %d, total: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  SendBsxMsg( msg, pkt_size ) ;

  DownLoading = 1 ;
}

void DownloadCheck()
{
  unsigned char msg[150], *pmsg ;
  short size, pkt_size ;

  if ( DownLoading == 0 ) return ;

  /* Prepare pkt */
  size = sizeof( M_DOWNLOAD_CHECK_MESSAGE ) ;
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;
  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = M_DOWNLOAD_CHECK ;
  *pmsg++ = 0 ; // Version = 0

  /* Message */
  M_DOWNLOAD_CHECK_MESSAGE * pdown = (M_DOWNLOAD_CHECK_MESSAGE *)pmsg ;
  pdown->down_nb = htons( DownNumber ) ;
  pdown->check_nb = 0x1234 ;
  pdown->nb_slice = htons( CurDownSlice ) ;

  LogPrint( LOG_INFO, "CurDownSlice: %d\n", ntohs(  pdown->nb_slice ) ) ;
  /* Send Pkt */
  LogPrint( LOG_INFO, "Send msg length: %d, total: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  printf( "DownLoadCheck\n" ) ;

  SendBsxMsg( msg, pkt_size ) ;

}

void DownloadSlice()
{
  FILE * fin ;
  unsigned char slice[DOWN_MAX_SLICE_LENGTH] ;
  short nb ;
  unsigned char *msg, *pmsg ;
  short size, pkt_size ;

  if ( DownLoading == 0 ) return ;

  /* Get a slice */
  fin = fopen( LocalFileName, "r" ) ;
  fseek( fin, CurDownSlice*DOWN_MAX_SLICE_LENGTH, SEEK_SET ) ;
  nb = fread( slice, 1, DOWN_MAX_SLICE_LENGTH, fin ) ;
  printf( "Slice #%d - Read %d bytes from file '%s'\n",
	  CurDownSlice, nb, LocalFileName ) ;
  fclose( fin ) ;

  /* Prepare pkt */
  size = sizeof( M_DOWNLOAD_SLICE_MESSAGE ) + nb ;
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;
  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  msg = malloc( pkt_size ) ;

  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = M_DOWNLOAD ;
  *pmsg++ = 0 ; // Version = 0

  /* The message */
  M_DOWNLOAD_MESSAGE * pdown = (M_DOWNLOAD_MESSAGE *)pmsg ;
  pdown->down_nb = htons( DownNumber ) ;
  pdown->slice_nb = htons( CurDownSlice ) ;
  CurDownSlice++ ;
  unsigned char * pslice = (unsigned char *)&pdown->slice_expected ;
  memcpy( pslice, slice, nb ) ;

  /* Send Pkt */
  printf( "Send msg length: %d, total: %d\n",
	  size, pkt_size ) ;
  /* Now send to Concentrator */
  SendBsxMsg( msg, pkt_size ) ;

  free( msg ) ;
}

#define UP_MAX_SLICE_LENGTH 200
static short UpPosition = 0 ;
static char UpFileName[128] ;
static char UpLocalFileName[256] ;
#define UPLOAD_OPEN_ERROR -1
#define UPLOAD_SEEK_ERROR -2

void UploadSave( unsigned char * msg, int length )
{
  //short length ;
  int data_length ;
  M_UPLOAD_ACQ_MESSAGE * ack ;

  //length = bytes_to_short( msg ) ;
  LogPrint( LOG_INFO, "Msg Length: %d, type: %d\n", length, *(msg+2) ) ;
  ack = malloc( length - MESSAGE_HEADER_LENGTH ) ;
  memcpy( ack, msg+5, length-MESSAGE_HEADER_LENGTH ) ;

  data_length =  ntohl( ack->size ) ;
  LogPrint( LOG_INFO, " Data Length: %d\n", data_length ) ;
  if ( data_length == UPLOAD_SEEK_ERROR ) {
    LogPrint( LOG_INFO, "End of upload '%s'\n", UpFileName ) ;
    printf( "\n  >>>>> End of upload '%s'\n", UpFileName ) ;
    /* Create file "upload.stat" to inform PMUP that upload is finished OK */
    system( "echo 'OK' >upload.stat" ) ;
  }
  else if ( data_length == UPLOAD_OPEN_ERROR ) {
    LogPrint( LOG_INFO, "Cant open remote '%s'\n", UpFileName ) ;
    printf( "  >>>>> Cant open remote '%s'\n", UpFileName ) ;
    /* Create file "upload.stat" to inform PMUP that upload is finished
     in error */
    system( "echo Error >upload.stat" ) ;
  }
  else {
    FILE * fout ;
    /* Create file "upready.stat" to inform PMUP that a slice has been
       received.
       Pmup deletes the file before sending a new Up command.
       Pmup wait until the file is writtent before sending a new Up command.
    */
    system( "echo Ready >>upready.stat" ) ;
    if ( UpPosition == 0 )
      fout = fopen( UpLocalFileName, "w" ) ;
    else
      fout = fopen( UpLocalFileName, "a+" ) ;
    fwrite( ack->buf, 1, data_length, fout ) ;
    fclose( fout ) ;
    UpPosition += data_length ;
  }

}

void UploadCmd( char * cmd )
{
#if 0
  char fname[128] ;


  /* Get file Name */
  printf( "Enter Remote File Name: " ) ;
  fgets( fname, 128, stdin ) ;
  fname[strlen(fname)-1] = '\0' ;
  strcpy( UpFileName, fname ) ;
  UpPosition = 0 ;
  printf( "Enter Local File Name: " ) ;
  fgets( fname, 256, stdin ) ;
  fname[strlen(fname)-1] = '\0' ;
  if ( *fname == '\0' ) strcpy( UpLocalFileName, UpFileName ) ;
  else strcpy( UpLocalFileName, fname ) ;
#else
  char * pcmd = cmd ;

  pcmd = strchr( pcmd, ' ' ) ;
  sscanf( pcmd, "%s", UpFileName ) ;
  printf( "Remote File Name <%s>\n", UpFileName ) ;


  strip_down_name( UpLocalFileName, UpFileName ) ;

  if ( (pcmd = strchr( pcmd+1, ' ') ) != NULL ) {
    sscanf( pcmd, "%s", UpLocalFileName ) ;
  }

  printf( "Up Remote File <%s>, Local File <%s>\n",
	  UpFileName, UpLocalFileName ) ;
  UpPosition = 0 ;
#endif
}

void UploadSlice()
{
  char flength ;
  unsigned char *msg, *pmsg ;
  short size, pkt_size ;

  flength = strlen( UpFileName ) + 1 ;

  /* Prepare Pkt */
  size = sizeof( M_UPLOAD_MESSAGE_HEADER ) + flength ;
  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;
  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  msg = malloc( pkt_size ) ;

  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = M_UPLOAD_REQ ;
  *pmsg++ = 0 ; // Version = 0

  /* The message */
  M_UPLOAD_MESSAGE * pup = (M_UPLOAD_MESSAGE *)pmsg ;

  pup->header.first = htonl( UpPosition ) ;
  pup->header.number = htonl( UP_MAX_SLICE_LENGTH ) ;
  strcpy( pup->file_name, UpFileName ) ;

  LogPrint( LOG_INFO,
	    "Position: %d, Size: %d, File '%s'\n",
	    ntohl( pup->header.first ),
	    ntohl( pup->header.number), pup->file_name ) ;
  /* Send Pkt */
  LogPrint( LOG_INFO, "Send msg length: %d, total: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  SendBsxMsg( msg, pkt_size ) ;
}

unsigned char * DoListe( unsigned char * pmsg )
{
  if ( Verbose ) printf( "\n Nb of LsIds in liste: %d\n", NbLsid ) ;
  if ( NbLsid == 1 ) {
    if ( Verbose ) printf( "   ==> %04x\n", (unsigned short)LsList[0] ) ;
    pmsg = short_to_bytes( pmsg, LsList[0] ) ;
  }
  else {
    int i ;
    for ( i = 0 ; i<= NbLsid ; i++ ) {
      if ( Verbose ) printf( " ==> %04X\n", LsList[i] ) ;
      pmsg = short_to_bytes( pmsg, LsList[i] ) ;
    }
  }
  return pmsg ;
}

unsigned char * DoMicroRefs( unsigned char * pmsg, char ref )
{
  if ( NbLsid == 1 ) {
    *pmsg++ = ref ;
  }
  else {
    int i ;
    for( i = 0 ; i < NbLsid ; i++ ) {
      if ( LsList[i] == LsId ) *pmsg++ = ref ;
      else *pmsg = 0 ;
    }
  }
  return pmsg ;
}

void CreateBroadcastListe()
{
  NbLsid = 1 ;
  LsList[0] = BROADCAST_DESTINATION << 14 ;
}

void CreateListe( char * msg )
{
  int i ;
  int liste[17] ;

  NbLsid =
    sscanf( msg,
	    "%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d",
	    &liste[1], &liste[2], &liste[3], &liste[4],
	    &liste[5], &liste[6], &liste[7], &liste[8],
	    &liste[9], &liste[10], &liste[11], &liste[12],
	    &liste[13], &liste[14], &liste[15], &liste[16]
	    ) ;
  LsList[0] = (LIST_DESTINATION << 14) | NbLsid ;
  printf( " **** LsList: %d LsId\n", NbLsid ) ;
  for ( i = 1 ; i <= NbLsid ; i++ ) {
    LsList[i] = liste[i] ;
    printf( "   %d\n", LsList[i] ) ;
  }

}

void CreateAntiListe( char * msg )
{
  int i ;
  int liste[17] ;

  NbLsid =
    sscanf( msg,
	    "%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d%*c%d",
	    &liste[1], &liste[2], &liste[3], &liste[4],
	    &liste[5], &liste[6], &liste[7], &liste[8],
	    &liste[9], &liste[10], &liste[11], &liste[12],
	    &liste[13], &liste[14], &liste[15], &liste[16]
	    ) ;
  LsList[0] = (ANTILIST_DESTINATION << 14) | NbLsid ;
  printf( " **** LsList: %d LsId\n", NbLsid ) ;
  for ( i = 1 ; i <= NbLsid ; i++ ) {
    LsList[i] = liste[i] ;
    printf( "   %d\n", LsList[i] ) ;
  }

}

void CreateSingleListe()
{
  NbLsid = 1 ;
  LsList[0] = LsId ;
}

void SetLsid()
{
  char str[32] ;
  int id ;

  printf( "Enter LsId: " ) ;
  fgets( str, 32, stdin ) ;
  str[strlen(str)-1] = '\0' ;
  sscanf( str, "%d", &id ) ;
  printf( " New LsId: %d\n", id ) ;
  /* Now send buildconfig to LS */
  char cmd[64] ;
  sprintf( cmd, "buildconfig --lsid=%d", id ) ;
  BuildShellCmd( cmd, M_SHELL_CMD ) ;

  LsId = id ;
}

typedef struct {
  unsigned short evid ;
  int seconds ;
  int micros ;
} T3_REQUEST_HEADER ;
#define T3_REQUEST_HEADER_LENGTH 10

void SendT3Yes( int again )
{
  /* Build a T3 request:
     The header
     Des microref (char) autant que de LsId dans la liste (tous a 0 sauf 
     celui correspondant a cette LS
     Finalement un delta
  */
  unsigned char msg[150], *pmsg ;
  short size, pkt_size ;

  if ( again == 0 ) {
    T3EvId++ ;
    T3Seconds = LastT2Sec ;
    T3Micros = LastT2Micro ;
    printf( "<<<< T3Request:  T3Evid = %d, T3Seconds=%d, T3Micros= %d\n",
	    T3EvId, T3Seconds, T3Micros ) ;
    SavePmrc() ;
  }
  else if ( again == 1 ) {
    printf( "<<<< T3Request AGAIN:  T3Evid = %d, T3Seconds=%d, T3Micros= %d\n",
	    T3EvId, T3Seconds, T3Micros ) ;
  }
  else if ( again == 2 ) {
    printf( "<<<< T3Request SAME:  T3Evid = %d, T3Seconds=%d, T3Micros= %d\n",
	    T3EvId, T3Seconds, T3Micros ) ;
  }
#if PM_VERSION>10
  else if ( again == 3 ) {
    T3EvId++ ;
    printf( "<<<< T3Request Full: T3Evid = %d, T3Seconds=%d, T3Micros= %d, T3Delta = %x\n",
	    T3EvId, T3Seconds, T3Micros, T3Delta ) ;
  }
#endif
  T3EvId &= 0x3FFF ;		/**< 2 MSbits are for "again" */

  /* Message size is HEader + delta + NbLsid*refs */
  size = T3_REQUEST_HEADER_LENGTH + (NbLsid*sizeof(T3MicroRef)) + 
    sizeof(T3Delta) ;

  pkt_size = size + PKT_HEADER_LENGTH + MESSAGE_HEADER_LENGTH ;

  if ( NbLsid != 1 ) pkt_size += NbLsid * 2 ;
  pmsg = msg ;
  pmsg = short_to_bytes( pmsg, pkt_size - PKT_HEADER_LENGTH_LENGTH ) ;
  *pmsg++ = 0 ; // rsvrd
  *pmsg++ = 0 ; // rsvrd

  pmsg = DoListe( pmsg ) ;

  //*pmsg++ = 1 ; // Nb de message

  /* The message */
  pmsg = short_to_bytes( pmsg, size + MESSAGE_HEADER_LENGTH ) ;
  *pmsg++ = 0 ; // no slice
  *pmsg++ = M_T3_YES ;
  *pmsg++ = 0 ; // Version = 0

  if ( again ==1 ) pmsg = short_to_bytes( pmsg, (T3EvId | 0xC000) ) ;
  else pmsg = short_to_bytes( pmsg, T3EvId ) ;
  pmsg = int_to_bytes( pmsg, T3Seconds ) ;
  pmsg = int_to_bytes( pmsg, T3Micros ) ;

  /* Now build the microrefs */
  pmsg = DoMicroRefs( pmsg, T3MicroRef ) ;
  *pmsg++ = T3Delta ;

  LogPrint( LOG_INFO, "Send msg length: %d, total: %d\n",
	    size, pkt_size ) ;
  /* Now send to Concentrator */
  dump_pkt( msg, pkt_size ) ;
  SendBsxMsg( msg, pkt_size ) ;

}


#if PM_VERSION>10

/*
  Decode the T3 message. Should be in the form:
     T3,<second>,<micros>,<window>[,<evid>]
       <second> = GPS time in seconds
       <micros> = GPS time microseconds
       <window> = window in micros
       <evid> = Event Id, used to create the T3 file name. If absent the default automatic
                event id computation is used.

    <second> ==> T3Seconds
    <micros> ==> T3Micros
    <window> ==> T3Delta ;
    <evid> ==> T3EvId (if present)

*/
void PrepT3Yes( char * msg )
{
  int anyint ;

  char * tok ;

  /* skip "t3," */
  tok = strtok( msg, "," ) ;
  

  /* get seconds, 1st param */
  tok = strtok( NULL, "," ) ;
  if ( tok == NULL ) {
    printf( "BadMissing 2nd T3 Parameter\n" ) ;
    return ;
  }
  sscanf( tok, "%d", &T3Seconds ) ;

  /* Get micros, 2nd ...*/
  tok = strtok( NULL, "," ) ;
  if ( tok == NULL ) {
    printf( "Missing 3rd T3 Parameter\n" ) ;
    return ;
  }
  sscanf( tok, "%d", &T3Micros ) ;

  /* get window, 3rd ... */
  tok = strtok( NULL, "," ) ;
  if ( tok == NULL ) {
    printf( "Missing 4th T3 Parameter\n" ) ;
    return ;
  }
  sscanf( tok, "%d", &anyint ) ;
  T3Delta = anyint ;
  
  /* get optional event id, 4th parameter */
  tok = strtok( NULL, "," ) ;
  if ( tok == NULL ) {
    printf( "Missing 4th T3 Parameter, use EvId = %d\n", T3EvId ) ;
  }
  else {
    sscanf( tok, "%d", &anyint ) ;
    T3EvId = anyint ;
  }

  SendT3Yes( 3 ) ;

  return ;
}

#endif

void HandleInputMsg( char * msg )
{
  //printf( " You said '%s'\n", msg ) ;
  if ( Verbose ) LogPrint( LOG_INFO, "Input Message '%s'\n", msg ) ;

  if ( strcmp( msg, "q" ) == 0 ) exit( 0 ) ;
  else if ( *msg == '\0' ) {
    printf( "  Nb Message Received: %d\n", NbReceived ) ;
  }
  else if ( strcmp( msg, "+" )  == 0 )Verbose++ ;
  else if ( strcmp( msg, "-" )  == 0 ){
    Verbose-- ;
    if ( Verbose < 0 ) Verbose = 0 ;
  }
  else if ( *msg == 'c' ) ShellCmd( msg + 2 ) ;
  else if ( *msg == 'D' ) DownloadCmd( msg ) ;
  else if ( strcmp( msg, "ds" ) == 0 ) DownloadSlice() ;
  else if ( strcmp( msg, "dc" ) == 0 ) DownloadCheck() ;
  else if ( *msg == 'U' ) UploadCmd( msg ) ;
  else if ( strcmp( msg, "u" ) == 0 ) UploadSlice() ;
  else if ( strcmp( msg, "l" ) == 0 ) CreateSingleListe() ;
  else if ( *msg == 'L' ) CreateListe( msg + 2 ) ;
  else if ( *msg == 'A' ) CreateAntiListe( msg + 2 ) ;
  else if ( strcmp( msg, "*" ) == 0 ) CreateBroadcastListe() ;
  else if ( strcmp( msg, "i" ) == 0 ) SetLsid() ;
  else if ( strcmp( msg, "w" ) == 0 ) SendWakeup() ;
  else if ( strcmp( msg, "T3" ) == 0 ) SendT3Yes( 0 ) ;
  else if ( strcmp( msg, "A3" ) == 0 ) SendT3Yes( 1 ) ;
  else if ( strcmp( msg, "S3" ) == 0 ) SendT3Yes( 2 ) ;
#if PM_VERSION>10
  else if ( strncmp( msg, "T3,", 3 ) == 0 ) PrepT3Yes( msg ) ;
#endif
  else if ( strcmp( msg, "M" ) == 0 ) SendMonitReq() ;
  else if ( strcmp( msg, "Mv" ) == 0 ) {
    MonitShow = MonitShow?0:1 ;
  }
  else if ( strcmp( msg, "S" ) == 0 ) StartStopAcq( 1 ) ;
  else if ( strcmp( msg, "s" ) == 0 ) StartStopAcq( 0 ) ;
  else if ( strcmp( msg, "T" ) == 0 ) StartTrigger() ;
  else if ( strcmp( msg, "t" ) == 0 ) StopTrigger() ;
  else if ( strcmp( msg, "St" ) == 0 ) StartSaveTriggers() ;
  else if ( strcmp( msg, "st" ) == 0 ) StopSaveTriggers() ;
  else if ( *msg == 'G' ) GenericMsg( msg + 1 ) ;
  else if ( strcmp( msg, "Rs" ) == 0 ) SrvRestart() ;
  else if ( strcmp( msg, "Ra" ) == 0 ) AcqRestart() ;
  else if ( strcmp( msg, "v" ) == 0 ) {
    ViewT2 = ViewT2 ?0:1 ;
    //printf( "ViewT2: %d\n", ViewT2 ) ;
  }
  else if ( strcmp( msg, "2" ) == 0 ) {
    if ( SaveT2 == 1 ) {
      SaveT2 = 0 ;
      if ( T2Out != NULL ) {
	fclose( T2Out ) ;
	T2Out = NULL ;
      }
      printf( "T2 Saving Stopped\n" ) ;
    }
    else if ( SaveT2 == 0 ) {
      SaveT2 = 1 ;
      printf( "Ready to Save T2\n" ) ;
    }
  }
  else if ( strcmp( msg, "Reboot" ) == 0 ) Reboot() ;
  else if ( strncmp( msg, "Comment", 7 ) == 0 ) {
    printf( "%s\n", msg+7 ) ;
    fflush( stdout ) ;
  }
  else {
    printf( " Unknown Command %s\n", msg ) ;
    //onlineHelp() ;
  }
}


int HandleAccepted()
{
  fd_set rdfs, wrfs, erfs ;
  struct timeval tmout ;
  int nbfs ;

  FD_ZERO( &rdfs ) ;
  FD_ZERO( &wrfs ) ;
  FD_ZERO( &erfs ) ;

  nbfs = CsSocket + 1 ;
  printf( "***** PM Connected to CS\n" ) ;
  if ( Verbose ) printf( "CsSocket: %d, Fifo: %d\n", CsSocket, FdFifo ) ;
  fflush( stdout ) ;

  for( ; ; ) {
    int ok ;

    tmout.tv_sec = 1 ;
    tmout.tv_usec = 0 ;
    FD_SET( CsSocket, &rdfs ) ;
    FD_SET( FdFifo, &rdfs ) ;

    ok = select( nbfs, &rdfs, NULL, NULL, &tmout ) ;

    if ( ok == -1 ) {
      cleanup( CsSocket ) ;
      exit( 0 ) ;
    }
    else if ( FD_ISSET( CsSocket, &rdfs ) ) {
      FD_CLR( CsSocket, &rdfs ) ;
      if ( Accepted == 0 ) {
	ok = GetConnectMsg() ;
	if ( ok <= 0 ) return -1 ;
	Accepted = 1 ;
	/* Reply with an ACK message */
	SendAckMsg() ;
	continue ;
      }
      else {
	ok = GetBsxMsg() ;
	if ( ok <= 0 ) return -1 ;
      }
    }
    else if ( FD_ISSET( FdFifo, &rdfs ) ) {
      char instr[1024] ;

      FD_CLR( FdFifo, &rdfs ) ;
      fgets( instr, 1024, FifoIn ) ;
      instr[strlen(instr)-1] = '\0' ;
      HandleInputMsg( instr ) ;
      fflush( stdout ) ;
    }
    /* Else timeout expired */
  }
}

static void create_fifo()
{
  /* Create FIFO */
  char cmd[64] ;

  sprintf( cmd, "mkfifo %s >/dev/null 2>&1", PM_FIFO ) ;
  system( cmd ) ;

  FifoIn = fopen( PM_FIFO, "r+" ) ;
  if ( FifoIn == NULL ) {
    printf( "Cant open Input Fifo '%s'\n", PM_FIFO ) ;
    exit( errno ) ;
  }
  FdFifo = fileno( FifoIn ) ;
  if ( Verbose ) {
    printf( "Fifo '%s' Created, fd = %d\n", PM_FIFO, FdFifo ) ;
    fflush( stdout ) ;
  }
}

int main( int argc, char **argv )
{


  int sock, opt ;
  struct	hostent		hostentstruct;
  struct	hostent		*hostentptr;
  static 	char	hostname[256];
  int 	namelength;
  time_t ttt ;
  fd_set rdfs, wrfs, erfs ;
  struct timeval tmout ;

  LogSetProgName( "pm" ) ;
  //LogSetNoclose() ;
  LogSetNewFile( "pm.log" ) ;
  
  strcpy( hostname, "localhost" ) ;

  while ( (opt = getopt( argc, argv, Options )) != EOF )
    switch( opt ) {
    case 'p': sscanf( optarg, "%d", &Port ) ;
      break ;
    case 'v': Verbose++ ; break ;
    case 'd': Debug++ ; break ;
    case 'h': strcpy( hostname, optarg ) ;
      break ;
    default: Help() ;
    }

  create_fifo() ;
  InitPmrc() ;

  if ( ( sock = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
    perror (" Socket ");
    exit( 0 );
  }

  if ( hostname[0] == '\0' ) {
    if ( gethostname ( hostname, sizeof (hostname))) {
      perror (" Gethostname ");
      cleanup( sock );
    }
  }

  if ( !( hostentptr = gethostbyname ( hostname ) ) ){
    perror (" Gethostbyname ");
    cleanup( sock );
  }
  LogPrint( LOG_INFO, "HostName: %s, Port: %d\n", hostname, Port ) ;

  printf( "***** PM Started\n" ) ;

  hostentstruct = *hostentptr;
  short sport = Port ;
  sock_name.sin_port = htons( sport ) ;
  sock_name.sin_family = AF_INET ;
  {
    int yes = 1 ;

    if ( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, 
                    sizeof(int)) != 0 ) {
      perror( "setsockopt() error");
    }
  }
  if ( bind( sock, (struct sockaddr *)&sock_name, sizeof (sock_name)) != 0 ) {
    LogPrint( LOG_INFO, "server_tcp Bind Error\n" ) ;
    while ( bind( sock, (struct sockaddr *)&sock_name, 
		  sizeof (sock_name)) != 0 ) {
      sleep( 1 ) ;
    }
  }

  CreateBroadcastListe() ;

  for( ; ; ) {
    ttt = time( NULL ) ;
    LogPrint( LOG_INFO, "    Ready - %s", ctime( &ttt ) ) ;

    if ( listen( sock, 1 ) < 0 ) {
      perror (" Listen ");
      cleanup( sock );
    }

    if ( Verbose > 1 ) LogPrint( LOG_INFO, "Listen OK on port %d\n", Port ) ;

    namelength = sizeof (struct sockaddr_in);

    FD_ZERO( &rdfs ) ;
    FD_ZERO( &wrfs ) ;
    FD_ZERO( &erfs ) ;

    for( ; ; ) {
      int ok ;
      struct sockaddr_in from ;

      tmout.tv_sec = 0 ;
      tmout.tv_usec = 100000 ;
      FD_SET( sock, &rdfs ) ;

      ok = select( sock+1, &rdfs, NULL, NULL, &tmout ) ;

      if ( ok == -1 ) {
	if ( Verbose > 1 ) LogPrint( LOG_INFO, "Main ok = %d\n", ok ) ;
	cleanup( sock ) ;
      }
      else if ( ok > 0 ) {
	if ( FD_ISSET( sock, &rdfs ) ) {
	  if ( Verbose ) LogPrint( LOG_INFO, "Got something from sock\n" ) ;
	  if ( ( CsSocket = accept ( sock, (struct sockaddr *)&from, 
				     (socklen_t *)&namelength)) == -1 ) {
	    perror(" Accept ");
	    cleanup( sock );
	  }
	  else {
	    if ( Verbose ) {
	      printf( "Accepted : %d\n", CsSocket ) ;
	    }
	    LogPrint( LOG_INFO, "Accepted : %d\n", CsSocket ) ;
	    FD_CLR( sock, &rdfs ) ;
	    if ( HandleAccepted() == -1 ) {
	      LogPrint( LOG_INFO, "Socket closed, listen again\n" ) ;
	      cleanup( CsSocket ) ;
	      break ;
	    }
	  }
	}
	FD_CLR( sock, &rdfs ) ;
      }
    }
  }
  cleanup( sock );
  exit( 0 ) ;
}
