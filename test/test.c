 #include<stdio.h>
#include<stdlib.h>
int main()
{
    int key;
    while(1)
    {
        key=getchar(); 
        if(key==27) break;
        if(key>31 && key<127) /*.......*/
        {printf(".. %c .    . ESC..!\n",key);continue;} 

        key=getchar();   
        if(key==72) printf(".. . .72    . ESC..!\n");
        if(key==80) printf(".. . . 80   . ESC..!\n");
        if(key==75) printf(".. . . 75   . ESC..!\n");
        if(key==77) printf(".. . .  77  . ESC..!\n");
    }return 0;
}
  
