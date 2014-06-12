/**
 * Yet Another Completely Useless Web Server
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 12345
#define BUFSIZE 8192

#define log_error(x) log(x, 1)

void log(char *str, int err) {
        printf("%s\n", str);
        if (err)
                perror(strerror(errno));

        exit(-1);
}

/* Reads the request from `request_fd`, finds the path,
 * checks its correctness and writes back requested file */
void handle_request(int request_fd) {
        printf("Handling request!\n");

        long ret;
        char buffer[BUFSIZE + 1]

        ret = read(request_fd, buffer, BUFSIZE);

        if (ret == 0 || ret == -1) {
                log_error("Error reading the request (read)");
        }

        /* Null terminate the buffer */
        if (ret > 0 && ret <= BUFSIZE) {
                buffer[ret] = '\0';
        } else {
                log_error("Error responding to the request (request too big)");
        }

        if (strncmp(buffer, "GET ", 4) != 0 || strncmp(buffer, "get ", 4) != 0) {
                log_error("Error responding to the request (no GET)");
        }

        if (strstr(&buffer[5], "..")) {

        }

        close(request_fd);
}

int main(int argc, char** argv)
{
        int listen_fd, request_fd;

        static struct sockaddr_in server_addr;
        static struct sockaddr_in client_addr;

        socklen_t len;

        int pid;

        if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                log_error("Error initalizing socket (socket)");
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(PORT);

        if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
                log_error("Error binding socket (bind)");
        }

        if (listen(listen_fd, 5) < 0) {
                log_error("Error initalizing listening (listen)");
        }

        while (1) {
                len = sizeof(client_addr);
                if ((request_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &len)) < 0) {
                        log_error("Error accepting request (accept)");
                }

                if ((pid = fork()) < 0) {
                        log_error("Problem with forking!");
                }

                if (pid == 0) {
                        /* in request handler (child), closing
                         * the listener and handling the request */
                        close(listen_fd);
                        handle_request(request_fd);

                        exit(1);
                } else {
                        /* in parent, closing the request socket */
                        close(request_fd);
                }
        }

        return 0;
}
