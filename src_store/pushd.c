#include <stdlib.h>
#include <stdio.h>

typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;

typedef struct list{
    node_tag* Top;
}list_tag;


node_tag* make_Node(char* path){
    node_tag* Node = NULL;
    printf("path:%s\n", path);
    Node = (node_tag*)malloc(sizeof(node_tag));
    if(Node == NULL){
        printf("Fails to init node\n");     //Failed to allocate dynamic memory of Node
        return NULL;
    }
    printf("path2:%s\n", path);
    Node->dir_path = path;
    printf("path3:%s\n", path);
    Node->Next = NULL;
    printf("Create new Node. path:%s\n", path);
    return Node;
}


list_tag* make_List(char* path){
    list_tag* List = NULL;
    
    List = (list_tag*)malloc(sizeof(list_tag));

    if(List == NULL){
        printf("Falis to init list\n");     //Failed to allocate dynamic memory of List
        return NULL;
    }
    List->Top = make_Node(path);
    if(List->Top == NULL){
        printf("Falis to init lis\n");
        free(List);
        return NULL;
    }
    //printfList(List);
    
    return List;
}

list_tag* Stack = NULL;

int main(int argc, char *args[]){
     printf("1\n");
    node_tag* Node = NULL;

    //リストが空の場合はノードを生成
    if(Stack->Top == NULL){
 printf("2\n");
        Stack->Top = make_Node(args[1]);
 printf("3\n");
        if(Stack->Top == NULL){
            printf("Fials to push\n");
        }
        //printfList(List);
        printf("First Node and create List\n");
        return 0;
    }
    
    Node = Stack->Top;
    
    while(Node->Next != NULL){
        Node = Node->Next;        
    };
    
    Node->Next = make_Node(args[1]);

    if(Node->Next == NULL){
        printf("Fials to push\n");
        return 1; 
    }
    
    //printfList(List);
    
    printf("Add Node\n");

    return 0;
}
