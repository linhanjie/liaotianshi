#include "user.h"
#include "log.h"
#include <unistd.h>
#include <linux/fcntl.h>

//#define LOG_TAG "USER"

static hash_table_t *table;
static char *users_db_file;

static void print_user(void *value) {
    user_t *user = value;
    LOG_DEBUG("user %s, %s", user->name, user->passwd);
}

user_t *search_user(char *name) {
    hash_node_t *node = search_hash_node(table, name);
    if (node)
        return (user_t *)node->value;

    return NULL;
}


hash_table_t * load_users_from_file(char *file) {


    LOG_DEBUG("%s() start", __func__);
    users_db_file = file;

    table = hash_table_create(1024);
    if (!table) 
    {
        return table;
    }


    int fd = open(file, O_RDWR);
    if (fd == -1) 
    {
        LOG_DEBUG("open file %s failed", file);
        return NULL;
    }


    for (;;) {
        user_t *user = (user_t *)malloc(sizeof(user_t));
        if (!user) {
            LOG_DEBUG("malloc user failed");
            return NULL;
        }

        int ret = read(fd, user->name, MAX_NAME_LEN);
        if (!ret) {
            break;
        }

        if (ret == -1) {
            LOG_DEBUG("read name failed, ret = %d", ret);
            return NULL;
        } else if (ret != MAX_NAME_LEN) {
            LOG_DEBUG("read name failed, ret = %d", ret);
            return NULL;
        } else {
            user->name[ret-1] = 0;
        }

        ret = read(fd, user->passwd, MAX_PASSWD_LEN);

        if (ret == -1) {
            LOG_DEBUG("read passwd failed, ret = %d", ret);
            return NULL;
        } else if (ret != MAX_PASSWD_LEN) {
            LOG_DEBUG("read passwd failed, ret = %d", ret);
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
        LOG_DEBUG("insert hash node failed");
        return 1;
    }

    int fd = open(users_db_file, O_RDWR, 0644);
    if (fd == -1) 
    {
        LOG_DEBUG("open file failed: %s", users_db_file);
        return 1;
    }

    lseek(fd, 0, SEEK_END);

    int n = write(fd, user->name, sizeof(user->name));
    LOG_DEBUG("wirte name %d", n);
    n = write(fd, user->passwd, sizeof(user->passwd));
    LOG_DEBUG("wirte passwd %d", n);

    close(fd);
    return 0;
}

