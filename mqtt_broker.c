/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 * 
 * Andrew Ijano Lopes - NUSP 10297797
 * 
 */


#define _GNU_SOURCE
#include "mqtt_broker.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "mqtt_handler.h"
#include "tcp_handler.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    show_usage_and_exit(argv[0]);
  }

  int listenfd = TCP_init_socket();
  if (listenfd == -1) {
    exit_with_message("fail to initialize socket");
  }

  int bind_result = TCP_bind_socket_address(listenfd, argv[1]);
  if (bind_result == -1) {
    exit_with_message("fail to bind port");
  }

  int listen_result = TCP_listen_connections(listenfd);
  if (listen_result == -1) {
    exit_with_message("fail to listen to connections");
  }

  printf("broker running on port %s\n", argv[1]);

  for (;;) {
    int *connfd_p = malloc(sizeof(int));

    *connfd_p = TCP_await_connection(listenfd);
    if (*connfd_p == -1) {
      exit_with_message("fail to accept connection");
    }

    pthread_t thread;
    pthread_create(&thread, NULL, handle_connection, (void *)connfd_p);
  }

  printf("\nbroker terminating\n");
  TCP_close_socket(listenfd);
  exit(0);
}

void *handle_connection(void *connfd_p) {
  int connfd = *((int *)connfd_p);

  printf("\nconnection %d open\n", connfd);

  uint8_t packet[MAX_PACKET_SIZE + 1];
  ssize_t packet_size, retval = 0;

  while (1) {
    packet_size = read(connfd, packet, HEADER_LENGTH);
    if (packet_size <= 0) {
      if (packet_size < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        continue;
      }
      break;
    }
    if (packet_size < HEADER_LENGTH) break;
    uint8_t header = packet[0];
    uint8_t body_size = packet[1];
    uint8_t message_type = (header >> 4) & 0x0F;
    uint8_t recv_size = body_size;

    do {
      retval = read(connfd, packet + HEADER_LENGTH + (body_size - recv_size), recv_size);
      if (retval <= 0) {
        if (retval < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) continue;
        TCP_close_socket(connfd);
        free(connfd_p);
        return NULL;
      }
      packet_size += retval;
      recv_size -= (uint8_t)retval;
    } while ((errno == EAGAIN || errno == EWOULDBLOCK) && recv_size > 0);

    printf("client %d with message type ", connfd);

    uint8_t response[MAX_PACKET_SIZE];
    ssize_t response_size =
      handle_message(response, message_type, connfd, packet, packet_size);
    if (response_size < 0) break;
    uint8_t *buf2 = response;
    do {
      retval = write(connfd, buf2, response_size);
      if (retval < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) break;
      buf2 += retval;
      response_size -= retval;
    } while ((errno == EAGAIN || errno == EWOULDBLOCK) && response_size > 0);
  }

  TCP_close_socket(connfd);
  printf("connection %d closed\n\n", connfd);
  free(connfd_p);
  return NULL;
}

int handle_message(uint8_t *response, uint8_t message_type, int connfd,
                   uint8_t *packet, ssize_t packet_size) {
  switch (message_type) {
    case CONNECT:
      return MQTT_handle_connect(response);
    case PUBLISH:
      return MQTT_handle_publish(packet, packet_size);
    case SUBSCRIBE:
      return MQTT_handle_subscribe(response, connfd, packet);
    case UNSUBSCRIBE:
      return MQTT_handle_unsubscribe(response, connfd, packet);
    case PINGREQ:
      return MQTT_handle_ping(response);
    case DISCONNECT:
      return MQTT_handle_disconnect(connfd);
    default:
      return MQTT_handle_default(message_type);
  }
}

void show_usage_and_exit(char *prog_name) {
  fprintf(stderr, "Usage: %s <port>\n", prog_name);
  fprintf(stderr, "You will run a MQTT broker in port <port> TCP\n");
  exit(1);
}

void exit_with_message(char *message) {
  perror(message);
  exit(-1);
}
