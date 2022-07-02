#include <stdio.h>
#include <dirent.h>     //opendir, readdir, closedir
#include <unistd.h>     //getcwd

#define PATH_SIZE 512

//拡張機能なし。隠しファイルも自動的に表示(ls -laと同じ)

int main(int argc, char *args[]){
    printf("argc:%d, args[0]:%s\n", argc, args[0]);

    DIR *dir;
    struct dirent *dir_info;

    char current_path[PATH_SIZE];
    getcwd(current_path, PATH_SIZE);
    printf("current dir : %s\n", current_path);
    char *path =  current_path;

    if (args[1] != NULL){
        path = args[1];
    }
    printf("path:%s\n", path);
    dir = opendir(path);

    if (dir == NULL){
        printf("Error\n");
        return 0;
    }

    while((dir_info = readdir(dir)) != NULL){
        printf("%s\n", dir_info->d_name);
    }
    closedir(dir);
    return 0;
    
}