/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 *
 * Andrew Ijano Lopes - NUSP 10297797
 *
 */

#include "subscription_manager.h"
#include "tcp_handler.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

Subscription subscriptions[MAX_SUBSCRIBERS];
ssize_t subscriptions_size = 0;

pthread_mutex_t mutex;

void SUBS_add_subscription(int connfd, char *topic) {
  int i;
  pthread_mutex_lock(&mutex);
  for (i = 0; i < subscriptions_size; i++) {
    if (subscriptions[i].connfd == -1) {
      break;
    }
  }
  if (i < MAX_SUBSCRIBERS) {
    subscriptions[i] = (Subscription){connfd, topic};
    subscriptions_size++;
    printf("-- adding subscriber %d in topic: '%s'\n", connfd, topic);
  }
  pthread_mutex_unlock(&mutex);
}

void SUBS_remove_subscription(int connfd) {
  int subscription_index = -1, i;
  pthread_mutex_lock(&mutex);
  for (i = 0; i < subscriptions_size; i++) {
    if (connfd == subscriptions[i].connfd) {
      subscription_index = i;
      break;
    }
  }

  if (subscription_index < 0) {
    pthread_mutex_unlock(&mutex);
    return;
  }

  subscriptions[subscription_index].connfd = -1;
  subscriptions_size--;
  pthread_mutex_unlock(&mutex);
  printf("-- removing subscriber %d\n", connfd);
}

void SUBS_publish_message(char *topic, uint8_t *publishing_packet,
                          ssize_t packet_size) {
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < subscriptions_size; i++) {
    Subscription sub = subscriptions[i];
    if (strcmp(topic, sub.topic) == 0) {
      printf("-- publishing in topic '%s' to subscriber %d\n", topic,
           sub.connfd);
      TCP_send_data(sub.connfd, publishing_packet, packet_size);
    }
  }
  pthread_mutex_unlock(&mutex);
}
