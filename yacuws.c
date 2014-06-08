/**
 * Yet Another Completely Useless Web Server
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345

int main(int argc, char** argv)
{
        int listen_fd, request_fd;

        static struct sockaddr_in server_addr;
        static struct sockaddr_in client_addr;

        socklen_t len;

        if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("Error initalizing socket (socket)\n");
                return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);

        if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
                printf("Error binding socket (bind)\n");
                return -1;
        }

        if (listen(listen_fd, 5) < 0) {
                printf("Error initalizing listening (listen)\n");
                return -1;
        }

        while (1) {
                len = sizeof(client_addr);
                if ((request_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len)) < 0) {
                        printf("Error accepting request (accept)\n");
                        return -1;
                }
                printf("Got socket!\n");
        }

        return 0;
}
