#include <stdlib.h>     //malloc
#include <stdio.h>
#include <unistd.h>     //getcwd
#define PATH_SIZE 512

#include "MyDefShell.h"

/*typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;*/

node_tag *pTop;   //*Topにしたければ実態を代入する必要あり

int main(int argc, char *args[]){
    printf("args[1]:%s, args[2]:%s\n",args[1], args[2]);
    char path[PATH_SIZE];
    getcwd(path, PATH_SIZE);
    args[1] = path;
    args[2] = path;
    printf("current dir : %s\n", path);

    //もし最初に追加する場合
    if(pTop == NULL){
        node_tag Top;
        pTop = &Top;
        pTop->dir_path = args[1];
        pTop->Next = NULL;
        printf("1\n");
    printf("Top:%s\n", pTop->dir_path);
    
    }else{
        //二回目以降に追加する場合
        node_tag *p;
        for(p=pTop; p->Next != NULL ; p= p->Next);
        printf("2\n");

        node_tag *NewNode = NULL;
        NewNode = (node_tag*)malloc(sizeof(node_tag));
        printf("3\n");
        NewNode->dir_path = args[2];
        NewNode->Next = NULL;
        printf("4\n");
        p->Next = NewNode;
    printf("Top:%s, Next:%s\n", pTop->dir_path, NewNode->dir_path);

    }
 
    return 0;
}