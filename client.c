/*
** File: flight_software.c
** Role: RECEIVER (Spacecraft Flight Software)
** Description: Receives raw bytes, validates checksum, and decodes CCSDS headers.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

#include "ccsds.h"

#define LISTEN_PORT 8888
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
    printf("   [FLIGHT SOFTWARE] RADIO BUFFER DUMP (%d bytes)\n", len);
    printf("=================================================================\n");
    printf("| Byte |  Binary  | Hex | Description                   | ASCII |\n");
    printf("|------|----------|-----|-------------------------------|-------|\n");

    for (int i = 0; i < len; i++) {
        uint8 b = buffer[i];
        printf("|  %02d  | ", i);
        print_byte_as_bits(b);
        printf(" |  %02X | Raw Byte Buffer               |   %c   |\n", b, isprint(b) ? b : '.');
    }
    printf("=================================================================\n\n");
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    uint8 buffer[BUF_SIZE];
    socklen_t addr_len;

    // 1. Create UDP Socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    servaddr.sin_port = htons(LISTEN_PORT);

    // 2. Bind Socket (Open the radio receiver)
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    printf("[FLIGHT SOFTWARE] Boot successful. Listening on port %d...\n", LISTEN_PORT);

    while (1) {
        addr_len = sizeof(cliaddr);
        
        // 3. Receive Raw Data (Simulating Radio Link)
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&cliaddr, &addr_len);

        if (n > 0) {
            // 4. Show Raw Data (Layer 1 View)
            visualize_packet(buffer, n);

            // 5. CCSDS Processing (Layer 2 View)
            CCSDS_CommandPacket_t *pkt = (CCSDS_CommandPacket_t *)buffer;

            printf("   [CCSDS DECODER ENGINE]\n");
            
            // Check Integrity
            if (CCSDS_ValidCheckSum(pkt)) {
                printf("   [+] Integrity Check: PASSED (Valid Checksum)\n");

                // Decode Headers using CCSDS Macros
                uint16 rcv_apid = CCSDS_RD_APID(pkt->SpacePacket.Hdr);
                uint16 rcv_seq  = CCSDS_RD_SEQ(pkt->SpacePacket.Hdr);
                uint16 rcv_len  = CCSDS_RD_LEN(pkt->SpacePacket.Hdr);
                uint8  rcv_fc   = CCSDS_RD_FC(pkt->Sec);
                
                // Extract Payload
                char *payload_str = (char *)(buffer + sizeof(CCSDS_CommandPacket_t));

                // Process Command
                printf("   [+] Packet Details:\n");
                printf("       - Application ID: 0x%03X (%d)\n", rcv_apid, rcv_apid);
                printf("       - Sequence Count: %d\n", rcv_seq);
                printf("       - Total Length:   %d bytes\n", rcv_len);
                printf("       - Function Code:  0x%02X\n", rcv_fc);
                printf("   [+] Payload Content: \"%s\"\n", payload_str);
                printf("   [+] Action: Dispatching to Application %d...\n", rcv_apid);

            } else {
                printf("   [-] Integrity Check: FAILED! Dropping packet.\n");
            }
        }
    }

    close(sockfd);
    return 0;
}