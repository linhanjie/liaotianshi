#include "user.h"

#include <unistd.h>
#include <linux/fcntl.h>

static hash_table_t *table;

static void print_user(void *value) {
    user_t *user = value;
    printf("user %s, %s\n", user->name, user->passwd);
}

hash_table_t * load_users_from_file(char *file) {

    table = hash_table_create(1024);
    if (!table) 
    {
        return table;
    }


    int fd = open(file, O_RDWR);
    if (fd == -1) 
    {
        printf("open file %s failed\n", file);
        return NULL;
    }


    for (;;) {
        user_t *user = (user_t *)malloc(sizeof(user_t));
        if (!user) {
            printf("malloc user failed\n");
            return NULL;
        }

        int ret = read(fd, user->name, MAX_NAME_LEN);
        if (!ret) {
            break;
        }

        if (ret == -1) {
            printf("read name failed, ret = %d\n", ret);
            return NULL;
        } else if (ret != MAX_NAME_LEN) {
            printf("read name failed, ret = %d\n", ret);
            return NULL;
        } else {
            user->name[ret] = 0;
        }
        
        ret = read(fd, user->passwd, MAX_PASSWD_LEN);

        if (ret == -1) {
            printf("read passwd failed, ret = %d\n", ret);
            return NULL;
        } else if (ret != MAX_PASSWD_LEN) {
            printf("read passwd failed, ret = %d\n", ret);
            return NULL;
        } else {
            user->passwd[ret] = 0;
        }

       
        insert_hash_node(table, user->name, user);
    
    }

    dump_hash_table(table, print_user);
    

    return table;
}

int save_user_to_file(user_t *user) 
{

    return 0;
}

