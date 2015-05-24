#ifndef _USER_H_
#define _USER_H_


#include <stdio.h>
#include "hash_table.h"

#define MAX_NAME_LEN 20
#define MAX_PASSWD_LEN 20

typedef struct user {
    char name[MAX_NAME_LEN+1];
    char passwd[MAX_PASSWD_LEN+1];
} user_t;

hash_table_t * load_users_from_file(char *file);

int save_user_to_file(user_t *user);

#endif
