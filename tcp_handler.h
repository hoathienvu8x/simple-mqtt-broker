/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 *
 * Andrew Ijano Lopes - NUSP 10297797
 *
 */

#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#include <stddef.h>

#define LISTEN_QUEUE 40

int TCP_init_socket();

int TCP_close_socket(int fd);

int TCP_bind_socket_address(int socketfd, char *port);

int TCP_listen_connections(int listenfd);

int TCP_await_connection(int listenfd);

int TCP_set_nonblocking(int fd);

int TCP_recv_data(int fd, void *buf, size_t len);

int TCP_send_data(int fd, const void *buf, size_t len);

#endif
