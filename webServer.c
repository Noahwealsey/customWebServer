#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>



int main() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    ssize_t bytes_sent;

    if (s == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr = {
        AF_INET,
        htons(36927),
        0
    };

    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 10);

    int client_fd = accept(s, 0, 0);
    char buffer[256] = {0};
    recv(client_fd, buffer, 256, 0);

    char *f = buffer + 5;
    *strchr(f, ' ') = 0;
    int opened_fd = open(f, O_RDONLY);

    struct stat file_stat;
    fstat(opened_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    char headers[256];
    snprintf(headers, sizeof(headers),
             "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n",
             (long)file_size);
    send(client_fd, headers, strlen(headers), 0);

    while (bytes_sent = sendfile(client_fd, opened_fd, 0, file_size)) {
        if (bytes_sent == -1) {
            perror("sendfile");
            break;
        }
    }

    close(opened_fd);
    close(client_fd);
    close(s);

    return 0;
}
