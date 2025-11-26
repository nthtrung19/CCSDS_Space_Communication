/*
**  CCSDS Header Definitions - Universal Portable Version
**  Works on Little Endian (Intel/ARM) and Big Endian (PowerPC/SPARC)
**  without modification.
*/

#ifndef _ccsds_
#define _ccsds_

/*
** Includes
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> 

/* 
** Platform Independent Types
*/
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t  int32;

/* 
** Configuration
*/
#define CCSDS_TIME_SIZE 6

/*
** -------------------------------------------------------------------------
** STRUCTURE DEFINITIONS
** All headers are defined as uint8 arrays to enforce Big Endian layout.
** -------------------------------------------------------------------------
*/

/*----- CCSDS packet primary header (6 Bytes) -----*/
typedef struct {

   uint8   StreamId[2];  
      /* Byte 0: Version(3)|Type(1)|SecHdr(1)|APID_Hi(3) */
      /* Byte 1: APID_Lo(8) */

   uint8   Sequence[2];  
      /* Byte 0: SeqFlags(2)|SeqCount_Hi(6) */
      /* Byte 1: SeqCount_Lo(8) */

   uint8  Length[2];     
      /* Byte 0: Length_Hi(8) */
      /* Byte 1: Length_Lo(8) */

} CCSDS_PriHdr_t;

/*----- CCSDS command secondary header (2 Bytes) -----*/
typedef struct {

   uint8  Command[2];    
      /* Byte 0: Reserved(1)|FuncCode(7) */
      /* Byte 1: Checksum(8) */

} CCSDS_CmdSecHdr_t;

/*----- CCSDS telemetry secondary header (6 Bytes) -----*/
typedef struct {
   uint8  Time[CCSDS_TIME_SIZE];
} CCSDS_TlmSecHdr_t;

/*----- Generic Space Packet (Base) -----*/
typedef struct {
    CCSDS_PriHdr_t Hdr;    
} CCSDS_SpacePacket_t;

/*----- Generic Command Packet -----*/
typedef struct {
    CCSDS_SpacePacket_t SpacePacket;  
    CCSDS_CmdSecHdr_t   Sec;
} CCSDS_CommandPacket_t;

/*----- Generic Telemetry Packet -----*/
typedef struct {
    CCSDS_SpacePacket_t SpacePacket;   
    CCSDS_TlmSecHdr_t   Sec;
} CCSDS_TelemetryPacket_t;


/*
** -------------------------------------------------------------------------
** CONSTANTS
** -------------------------------------------------------------------------
*/
#define CCSDS_TLM  0
#define CCSDS_CMD  1
#define CCSDS_NO_SEC_HDR   0
#define CCSDS_HAS_SEC_HDR  1
#define CCSDS_INIT_SEQ      0
#define CCSDS_INIT_SEQFLG   3  
#define CCSDS_INIT_FC       0
#define CCSDS_INIT_CHECKSUM 0


/*
** -------------------------------------------------------------------------
** ACCESS MACROS (The Core Logic)
** These macros handle the conversion between "Math" and "Memory bytes".
** -------------------------------------------------------------------------
*/

/* --- Primary Header Macros --- */

/* STREAM ID (Version, Type, SecHdr, APID) */
/* Read: Reconstruct 16-bit value from 2 bytes */
#define CCSDS_RD_SID(phdr)         (((phdr).StreamId[0] << 8) + ((phdr).StreamId[1]))
/* Write: Split 16-bit value into 2 bytes */
#define CCSDS_WR_SID(phdr,value)   ( ((phdr).StreamId[0] = (value >> 8)   ) ,\
                                     ((phdr).StreamId[1] = (value & 0xff) ) )

/* APID Access (Masking logic) */
#define CCSDS_RD_APID(phdr)        (CCSDS_RD_SID(phdr) & 0x07FF)
#define CCSDS_WR_APID(phdr,value)  ((((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xF8) | ((value >> 8) & 0x07))) ,\
                                   (((phdr).StreamId[1]  = ((value)) & 0xff)) )

/* Packet Type (CMD/TLM) */
#define CCSDS_RD_TYPE(phdr)        (((phdr).StreamId[0] & 0x10) >> 4)
#define CCSDS_WR_TYPE(phdr,value)  ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xEF) | ((value << 4) & 0x10))

/* Secondary Header Flag */
#define CCSDS_RD_SHDR(phdr)        (((phdr).StreamId[0] & 0x08) >> 3)
#define CCSDS_WR_SHDR(phdr,value)  ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xf7) | ((value << 3) & 0x08))

/* CCSDS Version */
#define CCSDS_RD_VERS(phdr)        (((phdr).StreamId[0] & 0xE0) >> 5)
#define CCSDS_WR_VERS(phdr,value)  ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0x1F) | ((value << 5) & 0xE0))


/* SEQUENCE WORD (Flags, Count) */
#define CCSDS_RD_SEQ(phdr)         ((((phdr).Sequence[0] & 0x3F) << 8) + ((phdr).Sequence[1]))
#define CCSDS_WR_SEQ(phdr,value)   ((((phdr).Sequence[0] = ((phdr).Sequence[0] & 0xC0) | ((value >> 8) & 0x3f))) ,\
                                   (((phdr).Sequence[1]  = ((value)) & 0xff)) )

#define CCSDS_RD_SEQFLG(phdr)      (((phdr).Sequence[0] & 0xC0) >> 6)
#define CCSDS_WR_SEQFLG(phdr,value) ((phdr).Sequence[0] = ((phdr).Sequence[0] & 0x3F) | ((value << 6) & 0xC0) )


/* LENGTH WORD */
#define CCSDS_RD_LEN(phdr)         ( ( (phdr).Length[0] << 8) + (phdr).Length[1] + 7)
#define CCSDS_WR_LEN(phdr,value)   ((((phdr).Length[0] = ((value) - 7) >> 8)) ,\
                                   (((phdr).Length[1] = ((value) - 7) & 0xff)) )


/* --- Secondary Header Macros (Command) --- */

/* Function Code: Located in Command[0] (7 bits) */
#define CCSDS_RD_FC(shdr)           ((shdr).Command[0] & 0x7F)
/* Write: Preserves Reserved bit 0x80 */
#define CCSDS_WR_FC(shdr,value)     ((shdr).Command[0] = ((shdr).Command[0] & 0x80) | ((value) & 0x7F))

/* Checksum: Located in Command[1] (8 bits) */
#define CCSDS_RD_CHECKSUM(shdr)     ((shdr).Command[1])
#define CCSDS_WR_CHECKSUM(shdr,val) ((shdr).Command[1] = (val))


/* --- Clear Macros --- */

#define CCSDS_CLR_PRI_HDR(phdr) \
  ( (phdr).StreamId[0] = 0,\
    (phdr).StreamId[1] = 0,\
    (phdr).Sequence[0] = (CCSDS_INIT_SEQFLG << 6),\
    (phdr).Sequence[1] = 0,\
    (phdr).Length[0] = 0, \
    (phdr).Length[1] = 0 )

#define CCSDS_CLR_CMDSEC_HDR(shdr) \
  ( (shdr).Command[0] = (CCSDS_INIT_FC & 0x7F), \
    (shdr).Command[1] = (CCSDS_INIT_CHECKSUM) )


/*
** Exported Functions
*/
void CCSDS_LoadCheckSum (CCSDS_CommandPacket_t *PktPtr);
bool CCSDS_ValidCheckSum (CCSDS_CommandPacket_t *PktPtr);
uint8 CCSDS_ComputeCheckSum (CCSDS_CommandPacket_t *PktPtr);
uint16 CCSDS_BuildTelecommand(uint8       *PacketBuf,
                              uint16       PacketBufSize,
                              uint16       Apid,
                              uint16       SeqCount,
                              uint8        FuncCode,
                              const uint8 *Payload,
                              uint16       PayloadLen);

#endif  /* _ccsds_ */