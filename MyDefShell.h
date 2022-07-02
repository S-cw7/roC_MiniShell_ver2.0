//#ifndef MyDefShell_H //二重でincludeされることを防ぐ
//#define MyDefShell_H

typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;

extern struct node *pTop;

//include guardeが必要？

//#endif