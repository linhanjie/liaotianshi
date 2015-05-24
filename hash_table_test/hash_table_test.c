#include "../hash_table.h"



struct student {
    char name[20];
    int age;
};


struct student stus[]  = {
    {"linhanjie", 20},
    {"linhanjie1", 21},
    {"linhanjie2", 22},
    {"linhanjie3", 23},
    {"linhanjie4", 24},
    {"linhanjie5", 25},
    {"linhanjie6", 26},
    {"linhanjie7", 27},
    {"linhanjie2", 88},
};

void print_age(int *age) {
    printf("age = %d\n", *(int *)age);
}

int main(int argc, char **argv) 
{

   hash_table_t *table = hash_table_create(2);

   int i;
   for (i=0; i<sizeof(stus)/sizeof(stus[0]); i++) {
        insert_hash_node(table, stus[i].name, &stus[i].age);
   }

   dump_hash_table(table, print_age);

   del_hash_node_key(table, "linhanjie3");
   dump_hash_table(table, print_age);
   del_hash_node_key(table, "linhanjiexxx");
   
   dump_hash_table(table, print_age);

   return 0;
    


}
