/*
**  CCSDS Implementation - Universal Portable Version
*/

#include "ccsds.h"

/******************************************************************************
**  Function:  CCSDS_LoadCheckSum()
*/
void CCSDS_LoadCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   uint8 CheckSum;

   /* Clear checksum (Set Byte 7 to 0) */
   CCSDS_WR_CHECKSUM(PktPtr->Sec, 0);

   /* Calculate and Load */
   CheckSum = CCSDS_ComputeCheckSum(PktPtr);
   CCSDS_WR_CHECKSUM(PktPtr->Sec, CheckSum);
} 

/******************************************************************************
**  Function:  CCSDS_ValidCheckSum()
*/
bool CCSDS_ValidCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   return (CCSDS_ComputeCheckSum(PktPtr) == 0);
} 

/******************************************************************************
**  Function:  CCSDS_ComputeCheckSum()
*/
uint8 CCSDS_ComputeCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   /* Use macros to get length correctly */
   uint16   PktLen   = CCSDS_RD_LEN(PktPtr->SpacePacket.Hdr);
   
   /* Access as raw bytes - Works universally */
   uint8   *BytePtr  = (uint8 *)PktPtr;
   uint8    CheckSum = 0xFF;
   
   while (PktLen--)  CheckSum ^= *(BytePtr++);

   return CheckSum;
} 

/******************************************************************************
**  Function:  CCSDS_BuildTelecommand()
*/
uint16 CCSDS_BuildTelecommand(uint8       *PacketBuf,
                              uint16       PacketBufSize,
                              uint16       Apid,
                              uint16       SeqCount,
                              uint8        FuncCode,
                              const uint8 *Payload,
                              uint16       PayloadLen)
{
    CCSDS_CommandPacket_t *PktPtr;
    uint16                 HeaderSize;
    uint32                 TotalLen32;
    uint16                 TotalLen;
    uint16                 i;
    uint8                 *DataPtr;

    if (PacketBuf == NULL) return 0;

    HeaderSize = (uint16)sizeof(CCSDS_CommandPacket_t); 
    TotalLen32 = (uint32)HeaderSize + (uint32)PayloadLen;

    if (TotalLen32 > PacketBufSize || TotalLen32 > 0xFFFF) return 0;

    TotalLen = (uint16)TotalLen32;
    PktPtr = (CCSDS_CommandPacket_t *)PacketBuf;

    /* Clear Headers */
    CCSDS_CLR_PRI_HDR(PktPtr->SpacePacket.Hdr);
    CCSDS_CLR_CMDSEC_HDR(PktPtr->Sec);

    /* Set Primary Header - Macros handle the Endianness */
    CCSDS_WR_APID (PktPtr->SpacePacket.Hdr, Apid);
    CCSDS_WR_TYPE (PktPtr->SpacePacket.Hdr, CCSDS_CMD);         
    CCSDS_WR_SHDR (PktPtr->SpacePacket.Hdr, CCSDS_HAS_SEC_HDR); 
    CCSDS_WR_VERS (PktPtr->SpacePacket.Hdr, 0);                 
    CCSDS_WR_SEQ  (PktPtr->SpacePacket.Hdr, SeqCount);          
    CCSDS_WR_LEN  (PktPtr->SpacePacket.Hdr, TotalLen);          

    /* Set Secondary Header */
    CCSDS_WR_FC(PktPtr->Sec, FuncCode);

    /* Copy Payload */
    DataPtr = PacketBuf + HeaderSize;
    if(Payload != NULL && PayloadLen > 0)
    {
        for (i = 0; i < PayloadLen; ++i) DataPtr[i] = Payload[i];
    }

    /* Checksum */
    CCSDS_LoadCheckSum(PktPtr);

    return TotalLen;
}