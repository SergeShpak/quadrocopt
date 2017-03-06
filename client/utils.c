#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "include/utils.h"

SimpleNode *initialize_simple_node(void *el) {
  SimpleNode *node = (SimpleNode *) malloc(sizeof(SimpleNode));
  node->el = el;
  return node;
}

void free_simple_node(SimpleNode *node, void (*free_func)(void *)) {
  free_func(node->el);
  free(node);
}


BiNode *initialize_binode(void *el) {
  BiNode *node = (BiNode *) malloc(sizeof(BiNode));
  node->el = el;
  return node;
}

void free_binode(BiNode *node, void (*free_func)(void *)) {
  free_func(node->el);
  free(node);
}


SimpleLinkedList *initialize_simple_linked_list(SimpleNode *head) {
  SimpleLinkedList *list = 
                        (SimpleLinkedList *) malloc(sizeof(SimpleLinkedList));
  list->head = head;
  list->tail = head;
  head->next = NULL;
  return list;
}

SimpleLinkedList *add_to_simple_linked_list(SimpleLinkedList *list,
                                            SimpleNode *node) {
  list->tail->next = node;
  list->tail->next = NULL;
  return list;
}

void free_simple_linked_list(SimpleLinkedList *list, 
                              void (*free_func)(void*)) {
  SimpleNode *curr_node = list->head;
  SimpleNode *node_to_free;
  while (NULL != curr_node) {
    node_to_free = curr_node;
    curr_node = curr_node->next;
    free_simple_node(node_to_free, free_func); 
  } 
  free(list);
}


DoubleLinkedList *initialize_double_linked_list(BiNode *node) {
  DoubleLinkedList *list = 
                        (DoubleLinkedList *) malloc(sizeof(DoubleLinkedList));
  list->head = node;
  list->tail = node;
  if (NULL == node) {
    list->nodes_count = 0;
    return list;
  }
  list->tail->next = NULL;
  list->tail->prev = NULL;
  list->head->next = NULL;
  list->head->next = NULL;
  list->nodes_count = 1;
  return list;
}

DoubleLinkedList *add_to_double_linked_list(DoubleLinkedList *list, 
                                            BiNode *node) {
  BiNode *prev_tail = list->tail;
  list->tail = node;
  if (NULL == node) {
    return list;
  }
  list->tail->next = NULL;
  list->tail->prev = prev_tail;
  list->nodes_count += 1;
  return list;
}

BiNode *RemoveFromDoubleLinkedListRear(DoubleLinkedList *list) {
  BiNode *tail_node = list->tail;
  list->tail = list->tail->prev;
  if (NULL != tail_node) {
    list->nodes_count -= 1;
  }
  return tail_node;
}


void exit_error(char *err_msg) {
  fprintf(stderr, "%s", err_msg);
  exit(1); 
}

void safe_print(char *msg, pthread_mutex_t *mu) {
  pthread_mutex_lock(mu);
  printf("%s", msg);
  pthread_mutex_unlock(mu);  
}

int get_random_int(int low, int high) {
  int r_int = rand() % (high - low) + low;
  return r_int;
}

void rand_sleep(int low, int high) {
  int period_rand = get_random_int(low, high);
  usleep(period_rand);
}

float *get_random_float_buf(size_t buf_len) {
  float *buf = (float *) malloc(sizeof(float) * buf_len);
  int i;
  for (i = 0; i < buf_len; i++) {
    float cur_float = ((float)rand()/(float)(RAND_MAX)) * 100;
    buf[i] = cur_float;
  }
  return buf;
}

float *copy_arr_of_floats(float *arr, size_t arr_size) {
  size_t copy_size = sizeof(float) * arr_size;
  float *copy = (float *) malloc(copy_size);
  memcpy(copy, arr, copy_size);
  return copy;
}
