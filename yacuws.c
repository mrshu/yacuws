/**
 * Yet Another Completely Useless Web Server
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <dirent.h>

#include <magic.h>

int PORT;
#define BUFSIZE 8192

#define DEBUG 0

#define STR(x) #x
#define TOSTR(x) STR(x)
#define dprint_str(str) if (DEBUG) printf(__FILE__ ":" TOSTR(__LINE__) "> " TOSTR(str) \
" = '%s'\n", str)

#define dprint_int(str) if (DEBUG) printf(__FILE__ ":" TOSTR(__LINE__) "> " TOSTR(str) \
" = %d\n", str)

#define dprint_char(str) if(DEBUG) printf(__FILE__ ":" TOSTR(__LINE__) "> " TOSTR(str) \
" = %c\n", str)



#define log_error(x) _log(x, 1)
#define log_debug(x) _log(x, 0)

const char* NOT_FOUND_NOT_FOUND = "HTTP/1.1 418 I'm a teapot (RFC 2324)\r\n\r\n<h1>Something is seriously wrong</h1><br>Even the file (./htdocs/404.html) with 'Not found' message was not found (and I might be a <a href='http://www.error418.org/'>teapot</a>).";

magic_t magic_cookie;

void _log(char *str, int err)
{
        if (err) {
                printf("yacuws: %s\n", str);
                perror(strerror(errno));
                exit(-1);
        } else {
                printf("=> yacuws: %s\n", str);
        }
}

char* build_dir_listing(char* directory)
{
        char out[10* BUFSIZE];
        char path[BUFSIZE];
        DIR *dir;
        struct dirent *ent;
        struct stat statbuf;

        int len = 0;

        len += snprintf(out, BUFSIZE,  "<html><head><title>Dir listing of %s</title></head><body><h1>Dir listing of %s</h1><ul>", directory, directory);
        dprint_str(out);

        if ((dir = opendir (directory)) != NULL) {
                while ((ent = readdir (dir)) != NULL) {
                        dprint_str(ent->d_name);

                        snprintf(path, BUFSIZE, "%s/%s", directory, ent->d_name);
                        stat(path, &statbuf);

                        if (S_ISDIR(statbuf.st_mode))
                                len += snprintf (&out[len], BUFSIZE, "<li><a href='/%s%s/'>%s</a></li>\n", directory, ent->d_name, ent->d_name);
                        else
                                len += snprintf (&out[len], BUFSIZE, "<li><a href='/%s%s'>%s</a></li>\n", directory, ent->d_name, ent->d_name);

                        if (len > 10*BUFSIZE)
                                break;
                }
                dprint_str(out);

                snprintf (&out[len], BUFSIZE, "</ul></body></html>\n");
                closedir (dir);
        } else {
                log_error("Error reading directory (opendir)");
        }

        return out;
}

/* Responds to a request with a given HTTP response code and file.
 * Note: also shutdowns and closes the socket file descriptor */
int respond_with_file(char* response, char* filename, int target_fd)
{
        char buffer[BUFSIZE];
        char bigger_buffer[11*BUFSIZE];
        const char* mime;
        const char* out;

        ssize_t len;
        long length;

        int file = open(filename, O_RDONLY);

        if (file == -1) {
                log_debug("Error opening the requested file (open)");

                dprint_str(filename);
                dprint_str(&filename[strlen(filename) - 10]);
                if (strlen(filename) > 10) {
                        if (strncmp(&filename[strlen(filename) - 10], "index.html", 10) == 0) {
                                filename[strlen(filename) - 10] = '\0';
                                out = build_dir_listing(filename);

                                dprint_str(out);

                                snprintf(bigger_buffer, 11*BUFSIZE - 1, "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n%s", (long int) strlen(out), out);

                                dprint_str(bigger_buffer);

                                send(target_fd, bigger_buffer, strlen(bigger_buffer), MSG_NOSIGNAL);
                                shutdown(target_fd, SHUT_RDWR);

                                return 1;

                        }
                }

                file = open("./htdocs/404.html", O_RDONLY);
                if (file == -1) {
                        log_debug("Error opening the ./htdocs/404.html file (open)");

                        snprintf(buffer, BUFSIZE - 1, "%s", NOT_FOUND_NOT_FOUND);
                        send(target_fd, buffer, strlen(buffer), MSG_NOSIGNAL);
                        shutdown(target_fd, SHUT_RDWR);

                        return 1;
                }
        }


        mime = magic_file(magic_cookie, filename);

        length = lseek(file, 0, SEEK_END);
        lseek(file, 0, SEEK_SET);

        snprintf(buffer, BUFSIZE - 1, "HTTP/1.1 %s\r\nConnection: close\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", response, mime, length);

        send(target_fd, buffer, strlen(buffer), MSG_NOSIGNAL);
        len = read(file, buffer, BUFSIZE);

        while (len > 0) {
                send(target_fd, buffer, len, MSG_NOSIGNAL);
                len = read(file, buffer, BUFSIZE);
        }

        log_debug("Successfully responded with a file");

        close(file);

        shutdown(target_fd, SHUT_RDWR);
        close(target_fd);
        return 0;
}

/* Reads the request from `request_fd`, finds the path,
 * checks its correctness and writes back requested file */
void handle_request(int request_fd)
{
        long ret;
        char buffer[BUFSIZE + 1];
        char filename[BUFSIZE - 4 + 1];
        char tmp[BUFSIZE - 4 + 1];

        magic_cookie = magic_open(MAGIC_MIME_TYPE);
        magic_load(magic_cookie, NULL);

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

        if (strncmp(buffer, "GET ", 4) == 0) {
                sscanf(buffer, "GET %s", tmp);
        } else if (strncmp(buffer, "get ", 4) == 0) {
                sscanf(buffer, "get %s", tmp);
        } else {
                respond_with_file("501 Not Implemented", "./htdocs/501.html", request_fd);
                log_debug("Error responding to the request (no GET)");
                exit(-1);
        }

        if (strlen(tmp) > 0) {
                if (tmp[strlen(tmp) -1] == '/')
                        snprintf(filename, BUFSIZE, ".%sindex.html", tmp);
                else
                        snprintf(filename, BUFSIZE, ".%s", tmp);
        } else {
                log_error("Error responding to the request (apparently no filename)");
        }

        if (strstr(filename, "..")) {
                respond_with_file("400 Bad Request", "./htdocs/400.html", request_fd);
                log_debug("Error responding to the request (.. present)");
                exit(-1);
        }

        respond_with_file("200 OK", filename, request_fd);

}

int main(int argc, char** argv)
{
        int listen_fd, request_fd;

        static struct sockaddr_in server_addr;
        static struct sockaddr_in client_addr;

        socklen_t len;
        int pid;
        int i;

        PORT = 12345;

        for (i = 1; i < argc; i++) {
                if((!strcmp(argv[i], "-p")) && (i+1<argc)) {
                        PORT = atoi(argv[++i]);
                } else if ((!strcmp(argv[i], "-d")) && (i+1<argc)) {
                        if (chdir(argv[++i]) != 0) {
                                log_error("Error changing current working directory");
                        }
                } else {
                        printf("usage: yacuws [-p port] [-d directory]");
                        exit(0);

                }
        }

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

        log_debug("Web server listening.");

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
