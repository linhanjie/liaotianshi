#include "user.h"

#include <unistd.h>
#include <linux/fcntl.h>

static hash_table_t *table;
static char *users_db_file;

static void print_user(void *value) {
    user_t *user = value;
    printf("user %s, %s\n", user->name, user->passwd);
}

user_t *search_user(char *name) {
    hash_node_t *node = search_hash_node(table, name);
    if (node)
        return (user_t *)node->value;

    return NULL;
}


hash_table_t * load_users_from_file(char *file) {

    users_db_file = file;

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
            user->name[ret-1] = 0;
        }

        ret = read(fd, user->passwd, MAX_PASSWD_LEN);

        if (ret == -1) {
            printf("read passwd failed, ret = %d\n", ret);
            return NULL;
        } else if (ret != MAX_PASSWD_LEN) {
            printf("read passwd failed, ret = %d\n", ret);
            return NULL;
        } else {
            user->passwd[ret-1] = 0;
        }


        insert_hash_node(table, user->name, user);

    }
    //add builtin user     
    user_t *user = (user_t *)malloc(sizeof(user_t));
    strcpy(user->name, "linhanjie");
    strcpy(user->passwd, "123456");
    insert_hash_node(table, user->name, user);
    
   user = (user_t *)malloc(sizeof(user_t));
    strcpy(user->name, "chengjianxi");
    strcpy(user->passwd, "123456");
    insert_hash_node(table, user->name, user);
   
    user = (user_t *)malloc(sizeof(user_t));
    strcpy(user->name, "test1");
    strcpy(user->passwd, "test1");
    insert_hash_node(table, user->name, user);
    
    user = (user_t *)malloc(sizeof(user_t));
    strcpy(user->name, "test2");
    strcpy(user->passwd, "test2");
    insert_hash_node(table, user->name, user);

    dump_hash_table(table, print_user);


    close(fd);
    return table;
}

int add_save_user(user_t *user)
{

    int ret;
    ret = insert_hash_node(table, user->name, user);

    if (ret) {
        printf("insert hash node failed\n");
        return 1;
    }

    FILE * fp = fopen(users_db_file, "wb");
    if (!fp) 
    {
        printf("open file failed: %s\n", users_db_file);
        return 1;
    }

    fseek(fp, 0, SEEK_END);

    fwrite(user->name, sizeof(user->name - 1), 1, fp);
    fwrite(user->passwd, sizeof(user->passwd - 1), 1, fp);


    fclose(fp);
    return 0;
}

