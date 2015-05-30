#include "message_queue.h"
#include <sys/types.h>
#include  <sys/syscall.h>



#include <stdio.h>  
#include <pthread.h>  
  


pid_t gettid()
{      
    return syscall(SYS_gettid);
} 


message_queue_t queue;

void work_thread(void *data)  
{  
    printf(">>>> thread: %d start\n", gettid());
           
    int val;
    for (;;) {
        dequeue_message(&queue, &val);
        printf(">>> thread: %d dequeue val = %d\n", gettid(), val);
       int i; 
        for (i=0; i<10*1000*1000; i++);
    }
}  
  
  
int main(void)  
{  
    message_queue_init(&queue);


    pthread_t id_1;  
    int i,ret;  
    
    for (i=1; i<=100; i++){
        enqueue_message(&queue, i);
    }    


    for (i=0; i<10; i++) {
         ret=pthread_create(&id_1,NULL, work_thread,NULL);  
        if(ret!=0)  
        {  
        printf("Create pthread error!\n");  
         return -1;  
        }  
    }


    sleep(5);

    for (i=101; i<=200; i++){
        enqueue_message(&queue, i);
    }    

    pthread_join(id_1, NULL);

return 0;
}
