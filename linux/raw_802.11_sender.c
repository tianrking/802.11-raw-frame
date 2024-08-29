#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>

#define PACKET_SIZE 1000

struct ieee80211_radiotap_header {
    uint8_t        it_version;
    uint8_t        it_pad;
    uint16_t       it_len;
    uint32_t       it_present;
} __attribute__((__packed__));

struct ieee80211_header {
    uint16_t frame_control;
    uint16_t duration_id;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_ctrl;
} __attribute__((__packed__));

int create_raw_socket(const char *dev)
{
    struct ifreq ifr;
    struct sockaddr_ll sll;
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        perror("ioctl");
        close(fd);
        exit(1);
    }

    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) == -1) {
        perror("bind");
        close(fd);
        exit(1);
    }

    return fd;
}

void construct_802_11_frame(uint8_t *buffer, const char *payload, size_t payload_len)
{
    struct ieee80211_radiotap_header *rtap_hdr = (struct ieee80211_radiotap_header *)buffer;
    rtap_hdr->it_version = 0;
    rtap_hdr->it_pad = 0;
    rtap_hdr->it_len = sizeof(struct ieee80211_radiotap_header);
    rtap_hdr->it_present = 0;

    struct ieee80211_header *hdr = (struct ieee80211_header *)(buffer + sizeof(struct ieee80211_radiotap_header));
    hdr->frame_control = 0x0008;  // Data frame
    hdr->duration_id = 0;
    memset(hdr->addr1, 0xFF, 6);  // Broadcast
    uint8_t src_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};  // Example source MAC
    memcpy(hdr->addr2, src_mac, 6);
    memcpy(hdr->addr3, src_mac, 6);
    hdr->seq_ctrl = 0;

    memcpy(buffer + sizeof(struct ieee80211_radiotap_header) + sizeof(struct ieee80211_header),
           payload, payload_len);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <interface> <message>\n", argv[0]);
        exit(1);
    }

    const char *dev = argv[1];
    const char *message = argv[2];
    uint8_t buffer[PACKET_SIZE] = {0};

    int sock_fd = create_raw_socket(dev);
    printf("Socket created on interface %s\n", dev);

    while (1) {
        construct_802_11_frame(buffer, message, strlen(message));
        
        int total_len = sizeof(struct ieee80211_radiotap_header) + 
                        sizeof(struct ieee80211_header) + 
                        strlen(message);

        int sent = send(sock_fd, buffer, total_len, 0);
        if (sent < 0) {
            perror("Send failed");
        } else {
            printf("Sent %d bytes: %s\n", sent, message);
        }

        sleep(1);  // Send every second
    }

    close(sock_fd);
    return 0;
}