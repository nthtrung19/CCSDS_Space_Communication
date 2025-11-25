/*
**  GSC-18128-1, "Core Flight Executive Version 6.7"
**
**  Copyright (c) 2006-2019 United States Government as represented by
**  the Administrator of the National Aeronautics and Space Administration.
**  All Rights Reserved.
*/

/******************************************************************************
** File:  ccsds.h
**
** Purpose:
**      Define typedefs and macros for CCSDS packet headers.
**      This file has been modified to be standalone (removing cFE dependencies)
**      while preserving the original structural definitions and comments.
**
******************************************************************************/

#ifndef _ccsds_
#define _ccsds_

/*
** Include Files
*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> /* For NULL definition */

/* 
** -------------------------------------------------------------------------
** STANDALONE TYPE DEFINITIONS
** (Originally found in common_types.h)
** -------------------------------------------------------------------------
*/
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int32_t  int32;

/* 
** -------------------------------------------------------------------------
** CONFIGURATION MACROS
** (Originally found in cfe_mission_cfg.h)
** -------------------------------------------------------------------------
*/

/* Define the Time Format used in the Telemetry Secondary Header */
/* We choose: 32 bits seconds + 16 bits subseconds */
#define CFE_MISSION_SB_TIME_32_16_SUBS  1
#define CFE_MISSION_SB_PACKET_TIME_FORMAT CFE_MISSION_SB_TIME_32_16_SUBS

/* CCSDS_TIME_SIZE is specific to the selected CFE_SB time format */
#if (CFE_MISSION_SB_PACKET_TIME_FORMAT == CFE_MISSION_SB_TIME_32_16_SUBS)
  /* 32 bits seconds + 16 bits subseconds */
  #define CCSDS_TIME_SIZE 6
#else
  /* Fallback for this standalone example */
  #define CCSDS_TIME_SIZE 6
#endif

/* 
** Macro to convert 16/32 bit types from platform "endianness" to Big Endian 
** Note: The CCSDS macros below handle byte shifting manually, so these are
** helper definitions for general use.
*/
#define CFE_MAKE_BIG16(n) ( (((n) << 8) & 0xFF00) | (((n) >> 8) & 0x00FF) )
#define CFE_MAKE_BIG32(n) ( (((n) << 24) & 0xFF000000) | (((n) << 8) & 0x00FF0000) | (((n) >> 8) & 0x0000FF00) | (((n) >> 24) & 0x000000FF) )


/*
** Type Definitions
*/

/**********************************************************************
** Structure definitions for CCSDS headers.  All items in the structure
** must be aligned on 16-bit words.  Bitfields must be avoided since
** some compilers (such as gcc) force them into 32-bit alignment.
**
** CCSDS headers must always be in network byte order per the standard.
** MSB at the lowest address which is commonly refered to as "BIG Endian"
**
**********************************************************************/

/*----- CCSDS packet primary header. (6 Bytes) -----*/

typedef struct {

   uint8   StreamId[2];  /* packet identifier word (stream ID) */
      /*  bits  shift   ------------ description ---------------- */
      /* 0x07FF    0  : application ID                            */
      /* 0x0800   11  : secondary header: 0 = absent, 1 = present */
      /* 0x1000   12  : packet type:      0 = TLM, 1 = CMD        */
      /* 0xE000   13  : CCSDS version:    0 = ver 1, 1 = ver 2    */

   uint8   Sequence[2];  /* packet sequence word */
      /*  bits  shift   ------------ description ---------------- */
      /* 0x3FFF    0  : sequence count                            */
      /* 0xC000   14  : segmentation flags:  3 = complete packet  */

   uint8  Length[2];     /* packet length word */
      /*  bits  shift   ------------ description ---------------- */
      /* 0xFFFF    0  : (total packet length) - 7                 */

} CCSDS_PriHdr_t;

/*----- CCSDS command secondary header. (2 Bytes) -----*/

typedef struct {

   uint16  Command;      /* command secondary header */
      /*  bits  shift   ------------ description ---------------- */
      /* 0x00FF    0  : checksum, calculated by ground system     */
      /* 0x7F00    8  : command function code                     */
      /* 0x8000   15  : reserved, set to 0                        */

} CCSDS_CmdSecHdr_t;

/*----- CCSDS telemetry secondary header. (Variable Size) -----*/

typedef struct {

   uint8  Time[CCSDS_TIME_SIZE];

} CCSDS_TlmSecHdr_t;

/*----- CCSDS APID Qualifier Fields -----*/
typedef struct {

   uint8 APIDQSubsystem[2];

      /*  bits  shift   ------------ description ---------------- */
      /* 0x01FF   0  : Subsystem Id  mission defined              */
      /* 0x0200   9  : Playback flag  0 = original, 1 = playback  */
      /* 0x0400  10  : Endian:   Big = 0, Little (Intel) = 1      */
      /* 0xF800  11  : EDS Version for packet definition used     */
 
   uint8 APIDQSystemId[2];
      /* 0xFFFF   0  : System Id      mission defined             */

} CCSDS_APIDqualifiers_t;

/* 
** Wrapper Structs:
** These combine the Primary Header and the specific Secondary Header.
** They are "Generic" definitions used to cast raw buffers.
*/

/*----- Generic Space Packet (Base) -----*/
typedef struct
{
    CCSDS_PriHdr_t      Hdr;    /**< Complete "version 1" (standard) header */
    /* Note: V2 APID Qualifiers omitted for simplicity in this project */
} CCSDS_SpacePacket_t;

/*----- Generic combined command header (Pri + Cmd Sec) -----*/
typedef struct
{
    CCSDS_SpacePacket_t  SpacePacket;   /**< \brief Standard Header on all packets  */
    CCSDS_CmdSecHdr_t    Sec;
} CCSDS_CommandPacket_t;

/*----- Generic combined telemetry header (Pri + Tlm Sec) -----*/
typedef struct
{
    CCSDS_SpacePacket_t  SpacePacket;   /**< \brief Standard Header on all packets */
    CCSDS_TlmSecHdr_t    Sec;
} CCSDS_TelemetryPacket_t;


/*
** Macro Definitions
*/

/**********************************************************************
** Constant values.
**********************************************************************/

/* Value of packet type for a telemetry packet. */
#define CCSDS_TLM  0
/* Value of packet type for a command packet. */
#define CCSDS_CMD  1

/* Value of secondary header flag if secondary header not present. */
#define CCSDS_NO_SEC_HDR   0
/* Value of secondary header flag if secondary header exists. */
#define CCSDS_HAS_SEC_HDR  1

/* Initial values for CCSDS header fields. */
#define CCSDS_INIT_SEQ      0
#define CCSDS_INIT_SEQFLG   3  /* 3 = Unsegmented (Complete) Packet */
#define CCSDS_INIT_FC       0
#define CCSDS_INIT_CHECKSUM 0


/**********************************************************************
** Macros for reading and writing bit fields in a 16-bit integer.
** These are used to implement the read and write macros below.
**********************************************************************/

/* Read bits specified by 'mask' from 'word' and shift down by 'shift'. */
#define CCSDS_RD_BITS(word,mask,shift) \
   (((word) & mask) >> shift)

/* Shift 'value' up by 'shift' and write to those bits in 'word' that
** are specified by 'mask'.  Other bits in 'word' are unchanged.   */
#define CCSDS_WR_BITS(word,mask,shift,value) \
   ((word) = (uint16)(((word) & ~mask) | (((value) & (mask >> shift)) << shift)))


/**********************************************************************
** Macros for reading and writing the fields in a CCSDS header.
**
** CRITICAL NOTE: These macros perform byte shifting manually to ensure
** data is written in BIG ENDIAN (Network Order) regardless of the
** processor's architecture (Intel is Little Endian).
**********************************************************************/

/* Read entire stream ID from primary header. */
#define CCSDS_RD_SID(phdr)         (((phdr).StreamId[0] << 8) + ((phdr).StreamId[1]))
/* Write entire stream ID to primary header. */
#define CCSDS_WR_SID(phdr,value)   ( ((phdr).StreamId[0] = (value >> 8)   ) ,\
                                     ((phdr).StreamId[1] = (value & 0xff) ) )

/* Read application ID from primary header. */
#define CCSDS_RD_APID(phdr)         (CCSDS_RD_SID(phdr) & 0x07FF)
/* Write application ID to primary header. */
#define CCSDS_WR_APID(phdr,value)  ((((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xF8) | ((value >> 8) & 0x07))) ,\
                                   (((phdr).StreamId[1]  = ((value)) & 0xff)) )

/* Read secondary header flag from primary header. */
#define CCSDS_RD_SHDR(phdr)         (((phdr).StreamId[0] & 0x08) >> 3)
/* Write secondary header flag to primary header. */
#define CCSDS_WR_SHDR(phdr,value)   ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xf7) | ((value << 3) & 0x08))

/* Read packet type (0=TLM,1=CMD) from primary header. */
#define CCSDS_RD_TYPE(phdr)         (((phdr).StreamId[0] & 0x10) >> 4)
/* Write packet type (0=TLM,1=CMD) to primary header. */
#define CCSDS_WR_TYPE(phdr,value)   ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0xEF) | ((value << 4) & 0x10))

/* Read CCSDS version from primary header. */
#define CCSDS_RD_VERS(phdr)        (((phdr).StreamId[0] & 0xE0) >> 5)
/* Write CCSDS version to primary header. */
#define CCSDS_WR_VERS(phdr,value)  ((phdr).StreamId[0] = ((phdr).StreamId[0] & 0x1F) | ((value << 5) & 0xE0))

/* Read sequence count from primary header. */
#define CCSDS_RD_SEQ(phdr)         ((((phdr).Sequence[0] & 0x3F) << 8) + ((phdr).Sequence[1]))
/* Write sequence count to primary header. */
#define CCSDS_WR_SEQ(phdr,value)   ((((phdr).Sequence[0] = ((phdr).Sequence[0] & 0xC0) | ((value >> 8) & 0x3f))) ,\
                                   (((phdr).Sequence[1]  = ((value)) & 0xff)) )

/* Read sequence flags from primary header. */
#define CCSDS_RD_SEQFLG(phdr)       (((phdr).Sequence[0] & 0xC0) >> 6)
/* Write sequence flags to primary header. */
#define CCSDS_WR_SEQFLG(phdr,value) ((phdr).Sequence[0] = ((phdr).Sequence[0] & 0x3F) | ((value << 6) & 0xC0) )

/* Read total packet length from primary header. */
#define CCSDS_RD_LEN(phdr)     ( ( (phdr).Length[0] << 8) + (phdr).Length[1] + 7)
/* Write total packet length to primary header. */
#define CCSDS_WR_LEN(phdr,value)   ((((phdr).Length[0] = ((value) - 7) >> 8)) ,\
                                   (((phdr).Length[1] = ((value) - 7) & 0xff)) )

/* Read function code from command secondary header. */
#define CCSDS_RD_FC(shdr)           CCSDS_RD_BITS((shdr).Command, 0x7F00, 8)
/* Write function code to command secondary header. */
#define CCSDS_WR_FC(shdr,value)     CCSDS_WR_BITS((shdr).Command, 0x7F00, 8, value)

/* Read checksum from command secondary header. */
#define CCSDS_RD_CHECKSUM(shdr)     CCSDS_RD_BITS((shdr).Command, 0x00FF, 0)
/* Write checksum to command secondary header. */
#define CCSDS_WR_CHECKSUM(shdr,val) CCSDS_WR_BITS((shdr).Command, 0x00FF, 0, val)

/**********************************************************************
** Macros for clearing a CCSDS header to a standard initial state.
**********************************************************************/

/* Clear primary header. */
#define CCSDS_CLR_PRI_HDR(phdr) \
  ( (phdr).StreamId[0] = 0,\
    (phdr).StreamId[1] = 0,\
    (phdr).Sequence[0] = (CCSDS_INIT_SEQFLG << 6),\
    (phdr).Sequence[1] = 0,\
    (phdr).Length[0] = 0, \
    (phdr).Length[1] = 0 )

/* Clear command secondary header. */
#define CCSDS_CLR_CMDSEC_HDR(shdr) \
  ( (shdr).Command = (CCSDS_INIT_CHECKSUM << 0) | (CCSDS_INIT_FC << 8) )


/*
** Exported Functions Prototypes
*/

/******************************************************************************
**  Function:  CCSDS_LoadCheckSum()
**  Purpose:   Compute and load a checksum for a CCSDS command packet.
*/
void CCSDS_LoadCheckSum (CCSDS_CommandPacket_t *PktPtr);

/******************************************************************************
**  Function:  CCSDS_ValidCheckSum()
**  Purpose:   Determine whether a checksum in a command packet is valid.
*/
bool CCSDS_ValidCheckSum (CCSDS_CommandPacket_t *PktPtr);

/******************************************************************************
**  Function:  CCSDS_ComputeCheckSum()
**  Purpose:   Compute the checksum (XOR of all bytes) for a command packet.
*/
uint8 CCSDS_ComputeCheckSum (CCSDS_CommandPacket_t *PktPtr);

/******************************************************************************
**  Function:  CCSDS_BuildTelecommand()
**  Purpose:   Helper to build a complete CCSDS Telecommand in a buffer.
*/
uint16 CCSDS_BuildTelecommand(uint8       *PacketBuf,
                              uint16       PacketBufSize,
                              uint16       Apid,
                              uint16       SeqCount,
                              uint8        FuncCode,
                              const uint8 *Payload,
                              uint16       PayloadLen);

#endif  /* _ccsds_ */