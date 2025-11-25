/*
** File: ground_station.c
** Role: SENDER (Ground Station)
** Description: Encodes telecommands into CCSDS packets and transmits them via UDP.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#include "ccsds.h"

#define TARGET_IP   "127.0.0.1" // Loopback for local simulation
#define TARGET_PORT 8888
#define BUF_SIZE    1024

// --- VISUALIZATION HELPER ---
void print_byte_as_bits(uint8 byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%c", (byte & (1 << i)) ? '1' : '0');
        if (i == 4) printf(" "); 
    }
}

void visualize_packet(uint8* buffer, int len) {
    printf("\n=================================================================\n");
    printf("   [GROUND STATION] TRANSMITTING PACKET DUMP (%d bytes)\n", len);
    printf("=================================================================\n");
    printf("| Byte |  Binary  | Hex | Description                   | ASCII |\n");
    printf("|------|----------|-----|-------------------------------|-------|\n");

    for (int i = 0; i < len; i++) {
        uint8 b = buffer[i];
        char desc[32] = "";
        
        if (i == 0) strcpy(desc, "Pri Hdr: Ver/Type/Sec/APID(Hi)");
        else if (i == 1) strcpy(desc, "Pri Hdr: APID (Low)");
        else if (i == 2) strcpy(desc, "Pri Hdr: Seq Flags/Count(Hi)");
        else if (i == 3) strcpy(desc, "Pri Hdr: Seq Count (Low)");
        else if (i == 4) strcpy(desc, "Pri Hdr: Length (Hi)");
        else if (i == 5) strcpy(desc, "Pri Hdr: Length (Low)");
        else if (i == 6) strcpy(desc, "Sec Hdr: Func Code");
        else if (i == 7) strcpy(desc, "Sec Hdr: Checksum");
        else             snprintf(desc, 32, "Payload Data [%d]", i-8);

        printf("|  %02d  | ", i);
        print_byte_as_bits(b);
        printf(" |  %02X | %-29s |   %c   |\n", b, desc, isprint(b) ? b : '.');
        
        if (i == 5 || i == 7) 
            printf("|------|----------|-----|-------------------------------|-------|\n"); 
    }
    printf("=================================================================\n\n");
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    uint8 buffer[BUF_SIZE];
    
    // Configuration for the target spacecraft
    uint16 apid = 0x1A5; // Example APID: 421
    uint16 seq = 0;

    // 1. Create UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TARGET_PORT);
    servaddr.sin_addr.s_addr = inet_addr(TARGET_IP);

    printf("[GROUND STATION] System Online. Target: %s:%d\n", TARGET_IP, TARGET_PORT);

    while (1) {
        // 2. Prepare User Data (Payload)
        char payload[32];
        snprintf(payload, 32, "CMD_SEQ_%d", seq);
        uint8 func_code = 0x0A; // Example OpCode

        printf("[GROUND STATION] Preparing Command #%d...\n", seq);

        // 3. Encode CCSDS Packet
        uint16 len = CCSDS_BuildTelecommand(buffer, BUF_SIZE, apid, seq, func_code, (uint8*)payload, strlen(payload)+1);

        if (len > 0) {
            // 4. Visualize the Raw Binary
            visualize_packet(buffer, len);

            // 5. Transmit over Uplink (UDP)
            sendto(sockfd, buffer, len, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            printf("[GROUND STATION] Packet transmitted.\n");
        } else {
            printf("[GROUND STATION] Error building packet.\n");
        }

        seq++;
        sleep(3); // Wait 3 seconds before sending next command
    }

    close(sockfd);
    return 0;
}