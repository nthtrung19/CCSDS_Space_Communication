/*
**  GSC-18128-1, "Core Flight Executive Version 6.7"
**  (Modified for Standalone Project)
*/

/******************************************************************************
** File:  ccsds.c
**
** Purpose:
**      Functions for working with CCSDS headers.
**      Implements Checksum logic and Packet Construction.
**
******************************************************************************/

/*
** Include Files
*/
#include "ccsds.h"

/******************************************************************************
**  Function:  CCSDS_LoadCheckSum()
**
**  Purpose:
**    Compute and load a checksum for a CCSDS command packet that has a
**    secondary header.
**
**  Arguments:
**    PktPtr   : Pointer to header of command packet.
*/
void CCSDS_LoadCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   uint8    CheckSum;

   /* 
   ** Step 1: Clear the checksum field. 
   ** If we don't clear it, the old value would be included in the XOR calculation,
   ** resulting in an incorrect new checksum.
   */
   CCSDS_WR_CHECKSUM(PktPtr->Sec, 0);

   /* 
   ** Step 2: Compute and load new checksum.
   */
   CheckSum = CCSDS_ComputeCheckSum(PktPtr);
   CCSDS_WR_CHECKSUM(PktPtr->Sec, CheckSum);

} /* END CCSDS_LoadCheckSum() */


/******************************************************************************
**  Function:  CCSDS_ValidCheckSum()
**
**  Purpose:
**    Determine whether a checksum in a command packet is valid.
**
**  Logic:
**    If a packet is uncorrupted, XORing all bytes (including the checksum byte)
**    should result in 0.
*/
bool CCSDS_ValidCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   return (CCSDS_ComputeCheckSum(PktPtr) == 0);

} /* END CCSDS_ValidCheckSum() */


/******************************************************************************
**  Function:  CCSDS_ComputeCheckSum()
**
**  Purpose:
**    Compute the checksum for a command packet.  The checksum is the XOR of
**    all bytes in the packet.
**
**  Note:
**    Standard CCSDS implementations often use a seed of 0xFF (all ones)
**    instead of 0x00 to detect null packets.
*/
uint8 CCSDS_ComputeCheckSum (CCSDS_CommandPacket_t *PktPtr)
{
   /* Read the total length of the packet using the header macro */
   uint16   PktLen   = CCSDS_RD_LEN(PktPtr->SpacePacket.Hdr);
   
   /* Treat the packet as a raw byte array */
   uint8   *BytePtr  = (uint8 *)PktPtr;
   uint8    CheckSum;

   /* Initialize Checksum with 0xFF */
   CheckSum = 0xFF;
   
   /* Iterate through every byte and XOR it */
   while (PktLen--)  CheckSum ^= *(BytePtr++);

   return CheckSum;

} /* END CCSDS_ComputeCheckSum() */


/******************************************************************************
**  Function:  CCSDS_BuildTelecommand()
**
**  Purpose:
**    A helper function to populate a buffer with a valid CCSDS Telecommand.
**    This function handles Endianness, Header flags, and Payload copying.
**
**  Returns:
**    Total length of the packet in bytes.
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

    /* Sanity Check: Ensure buffer exists */
    if (PacketBuf == NULL)
    {
        return 0;
    }

    /* Calculate Sizes */
    HeaderSize = (uint16)sizeof(CCSDS_CommandPacket_t); /* Pri (6) + CmdSec (2) = 8 bytes */
    TotalLen32 = (uint32)HeaderSize + (uint32)PayloadLen;

    /* Safety Check: Buffer Overflow protection */
    if (TotalLen32 > PacketBufSize || TotalLen32 > 0xFFFF)
    {
        return 0;
    }

    TotalLen = (uint16)TotalLen32;

    /* Cast the buffer to our Command Packet Structure */
    PktPtr = (CCSDS_CommandPacket_t *)PacketBuf;

    /* 
    ** --- Initialize Headers ---
    ** Clear headers to 0 to remove any garbage data in the buffer.
    */
    CCSDS_CLR_PRI_HDR(PktPtr->SpacePacket.Hdr);
    CCSDS_CLR_CMDSEC_HDR(PktPtr->Sec);

    /* 
    ** --- Set Primary Header Fields ---
    ** Using macros ensures Big Endian format regardless of CPU type.
    */
    CCSDS_WR_APID (PktPtr->SpacePacket.Hdr, Apid);
    CCSDS_WR_TYPE (PktPtr->SpacePacket.Hdr, CCSDS_CMD);         /* 1 = Command */
    CCSDS_WR_SHDR (PktPtr->SpacePacket.Hdr, CCSDS_HAS_SEC_HDR); /* 1 = Sec Header Present */
    CCSDS_WR_VERS (PktPtr->SpacePacket.Hdr, 0);                 /* 0 = Version 1 */
    CCSDS_WR_SEQ  (PktPtr->SpacePacket.Hdr, SeqCount);          /* Sequence Counter */
    CCSDS_WR_LEN  (PktPtr->SpacePacket.Hdr, TotalLen);          /* Total Length (Macros handle the -7) */

    /* 
    ** --- Set Secondary Header ---
    ** Set the Function Code (Instruction ID).
    */
    CCSDS_WR_FC(PktPtr->Sec, FuncCode);

    /* 
    ** --- Copy Payload ---
    ** Copy user data immediately following the headers.
    */
    DataPtr = PacketBuf + HeaderSize;
    if(Payload != NULL && PayloadLen > 0)
    {
        for (i = 0; i < PayloadLen; ++i)
        {
            DataPtr[i] = Payload[i];
        }
    }

    /* 
    ** --- Calculate Checksum ---
    ** Now that all data is written, calculate the final XOR checksum.
    */
    CCSDS_LoadCheckSum(PktPtr);

    return TotalLen;
}