#include "hash_table.h"

#define MIN_SIZE 1
static int hash_table_bucket_size = 0;

hash_table_t * hash_table_create(int size) {

    if (size < MIN_SIZE)
        size = MIN_SIZE;
    
    hash_table_bucket_size = size;

    hash_table_t *table = (hash_table_t *)malloc(sizeof(hash_table_t));
    if (!table) {
        printf("malloc hash table faile\n");
        return NULL;
    }

    table->nr_nodes = 0;
    table->bucket = (hash_node_t **)malloc(sizeof(hash_node_t *) * hash_table_bucket_size);
    if (!table->bucket) 
    {
        printf("malloc hash table bucket failed\n");
        return NULL;
    }
    memset(table->bucket, 0, sizeof(hash_node_t *) * hash_table_bucket_size);

    printf("create hash table size = %d\n", hash_table_bucket_size);
    return table;
}

static unsigned hash_func(char *key) {
    unsigned int hash = 0;
    int b = 378551;
    int a = 63689;
    int len = strlen(key);
    int i;
    
    for(i = 0; i < len; i++)
    {
        hash = hash * a + key[i];
        a = a * b;
    }
    return hash % hash_table_bucket_size;
}

hash_node_t * search_hash_node(hash_table_t *table, char * key) {

    int hash = hash_func(key);
    printf("search_hash_node key = %s, hash = %d\n", key, hash);
    hash_node_t *node = table->bucket[hash];

    while (node) {
        if (!strcmp(node->key, key))
            return node;

        node = node->next;
    }

    return NULL;
}

int insert_hash_node(hash_table_t *table, char * key, void *value) {

    hash_node_t *node = search_hash_node(table, key);
    if (node) {
        printf("update node %s from %p to %p\n", key, node->value, value);
        node->value = value;
    } else {
        int hash = hash_func(key);
        hash_node_t *p = table->bucket[hash];

        hash_node_t *new_node = (hash_node_t *)malloc(sizeof(hash_node_t));
        if (!new_node) {
            printf("malloc hash node failed\n");
            return 1;
        }

        printf("new hash node key = %s, value = %p\n", key, value);
        new_node->key = key;
        new_node->value = value;
        new_node->next = p;

        table->bucket[hash] = new_node;

        table->nr_nodes++;
    }
    return 0;
}


static int _del_hash_node(hash_table_t *table, hash_node_t * node, int need_check) {

    if (need_check) {
        if (!search_hash_node(table, node->key)) {
            printf("invalid node key = %s\n", node->key);
            return 1;
        }
    }
        
    int hash = hash_func(node->key);
        hash_node_t *p = table->bucket[hash];

    if (p == node) {
        table->bucket[hash] = node->next;
    } else {
        hash_node_t *old = p;
        p = p->next;
        while (p) {
            if (p == node) {
                old->next = p->next;
                break;
            }
            old = p;
            p = p->next;
        }

    }

    table->nr_nodes--;


    return 0;
}

int del_hash_node_key(hash_table_t *table, char * key) {
    hash_node_t *node = search_hash_node(table, key);
    if (!node) {
        printf("can't find node with key = %s\n", key);
        return 1;
    }

    return _del_hash_node(table, node, 0);

}


int del_hash_node(hash_table_t *table, hash_node_t *node) {

    return _del_hash_node(table, node, 1);
}

void dump_hash_table(hash_table_t *table, void (* func) (void *value) ) {

    printf("hash table nodes num = %d\n", table->nr_nodes);

    
    int i;
    for (i=0; i<hash_table_bucket_size; i++) {
        hash_node_t *p = table->bucket[i];

        while (p) {
          if (p) {
                printf("node key = %s\n", p->key);
                if (func) 
                {
                    func(p->value);
                }
          }

          p = p->next;


        
        }
    
    }
}

