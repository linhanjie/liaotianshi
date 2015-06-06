#include "hash_table.h"

#define MIN_SIZE 1
static int hash_table_bucket_size = 0;

hash_table_t * hash_table_create(int size) {

    if (size < MIN_SIZE)
        size = MIN_SIZE;
    
    hash_table_bucket_size = size;

    hash_table_t *table = (hash_table_t *)malloc(sizeof(hash_table_t));
    if (!table) {
        LOG_ERR("malloc hash table faile");
        return NULL;
    }

    table->nr_nodes = 0;
    table->bucket = (hash_node_t **)malloc(sizeof(hash_node_t *) * hash_table_bucket_size);
    if (!table->bucket) 
    {
        LOG_ERR("malloc hash table bucket failed");
        return NULL;
    }
    memset(table->bucket, 0, sizeof(hash_node_t *) * hash_table_bucket_size);

    LOG_INFO("create hash table size = %d", hash_table_bucket_size);
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
    LOG_DEBUG("search_hash_node key = %s, hash = %d", key, hash);
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
        LOG_DEBUG("update node %s from %p to %p", key, node->value, value);
        node->value = value;
    } else {
        int hash = hash_func(key);
        hash_node_t *p = table->bucket[hash];

        hash_node_t *new_node = (hash_node_t *)malloc(sizeof(hash_node_t));
        if (!new_node) {
            LOG_ERR("malloc hash node failed");
            return 1;
        }

        LOG_DEBUG("new hash node key = %s, value = %p", key, value);
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
            LOG_ERR("invalid node key = %s", node->key);
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
        LOG_ERR("can't find node with key = %s", key);
        return 1;
    }

    return _del_hash_node(table, node, 0);

}


int del_hash_node(hash_table_t *table, hash_node_t *node) {

    return _del_hash_node(table, node, 1);
}

void dump_hash_table(hash_table_t *table, void (* func) (void *value) ) {

    LOG_INFO("hash table nodes num = %d", table->nr_nodes);


    int i;
    for (i=0; i<hash_table_bucket_size; i++) {
        hash_node_t *p = table->bucket[i];

        while (p) {
            if (p) {
                LOG_INFO("node key = %s", p->key);
                if (func) 
                {
                    func(p->value);
                }
            }

            p = p->next;
        }

    }
}

