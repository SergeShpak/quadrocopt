#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdarg.h>

typedef struct _SimpleNode SimpleNode;
typedef struct _BiNode BiNode;
typedef struct _SimpleLinkedList SimpleLinkedList;
typedef struct _DoubleLinkedList DoubleLinkedList;

typedef struct _FloatArray FloatArray;


struct _SimpleNode {
  void *el;
  SimpleNode *next;
};

SimpleNode *initialize_simple_node(void *el);
void free_simple_node(SimpleNode *node, void (*free_func)(void *));


struct _BiNode {
  void *el;
  BiNode *prev;
  BiNode *next;
};

BiNode *initialize_binode(void *el);
void free_binode(BiNode *node, void (*free_func)(void *));


struct _SimpleLinkedList {
  SimpleNode *head;
  SimpleNode *tail; 
};

SimpleLinkedList *initialize_simple_linked_list(SimpleNode *head);
SimpleLinkedList *add_to_simple_linked_list(SimpleLinkedList *list, 
                                          SimpleNode *node);
void free_simple_linked_list(SimpleLinkedList *list, 
                            void (*free_func)(void *el));


struct _DoubleLinkedList {
  BiNode *head;
  BiNode *tail;
  size_t nodes_count;
};

DoubleLinkedList *initialize_double_linked_list(BiNode *node);
DoubleLinkedList *add_to_double_linked_list(DoubleLinkedList *list,
                                            BiNode *node);
BiNode *remove_from_double_linked_list_rear(DoubleLinkedList *list);
void free_double_linked_list(DoubleLinkedList *list, void (*free_func)(void*));

struct _FloatArray {
  float *arr;
  size_t arr_len;
};

FloatArray *initialize_float_array(float *arr, size_t arr_len);
void free_float_array(FloatArray *float_arr);

void exit_error(char *err_msg);
void safe_print(char *msg, pthread_mutex_t *mu);
int get_random_int(int low, int high);
void rand_sleep(int low, int high);
float *get_random_float_buf(size_t len);
float *copy_arr_of_floats(float *arr, size_t arr_size);

#endif
