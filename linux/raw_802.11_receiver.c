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

    struct packet_mreq mr;
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = sll.sll_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        perror("setsockopt");
        close(fd);
        exit(1);
    }

    return fd;
}

void print_packet(uint8_t *data, int len)
{
    printf("Received packet (%d bytes):\n", len);
    for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    // 尝试打印payload部分的ASCII
    printf("Payload (ASCII):\n");
    for (int i = 24; i < len; i++) {  // 24是radiotap header和802.11 header的大致长度
        if (data[i] >= 32 && data[i] <= 126) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    printf("\n\n");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        exit(1);
    }

    const char *dev = argv[1];
    uint8_t buffer[PACKET_SIZE] = {0};

    int sock_fd = create_raw_socket(dev);
    printf("Listening on interface %s...\n", dev);

    while (1) {
        int recv_len = recv(sock_fd, buffer, PACKET_SIZE, 0);
        if (recv_len < 0) {
            perror("Receive failed");
        } else {
            print_packet(buffer, recv_len);
        }
    }

    close(sock_fd);
    return 0;
}