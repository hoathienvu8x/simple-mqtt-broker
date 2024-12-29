/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 * 
 * Andrew Ijano Lopes - NUSP 10297797
 * 
 */

#include "tcp_handler.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int TCP_init_socket() {
    int on = 1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
      close(fd);
      return -1;
    }
    return fd;
}

int TCP_bind_socket_address(int socketfd, char *port) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(port));
    return bind(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
}

int TCP_listen_connections(int listenfd) {
    return listen(listenfd, LISTEN_QUEUE);
}

int TCP_await_connection(int listenfd) {
    return accept(listenfd, (struct sockaddr *)NULL, NULL);
}
