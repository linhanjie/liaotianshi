#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <stdio.h>


typedef struct hash_node {
    char * key;
    void * value;
    struct hash_node * next;
} hash_node_t;

typedef struct hash_table {
    int nr_nodes;
    hash_node_t ** bucket;
} hash_table_t;


hash_table_t *hash_table_create();

int insert_hash_node(hash_table_t *table, char * key, void *value);

hash_node_t * search_hash_node(hash_table_t *table, char * key);

int del_hash_node_key(hash_table_t *table, char * key);

int del_hash_node(hash_table_t *table, hash_node_t *node);

void dump_hash_table(hash_table_t *table, void (*) (void *));

#endif


