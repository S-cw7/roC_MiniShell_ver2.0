/*----------------------------------------------------------------------------
 *  簡易版シェル
 *--------------------------------------------------------------------------*/

/*メモ--------------------------------------------------------------------------
/mnt/c/Users/mclif/proC/2
\Users\mclif\proC\2
同一ファイルで関数としてコマンドを実行したほうが良いのか，別ファイルを作成して実行させればよいのか
最後にファイルに分ける
historyの保存個数について
全て：データ容量が増える→拡張ってことにしてもいいけど。。。!!コマンドについては一つ前までのコマンドを保存しえとけば問題ない
MAX_HISTORY：データ容量の削減。題意通り
とりあえず、すべて保存する形式で
--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 *  インクルードファイル
 --------------------------------------------------------------------------*/
#include <err.h>
//#include <direct.h> // _getcwd
#include <dirent.h>     //opendir, readdir, closedir,_mkdir
#include <sys/stat.h>   //_mkdir
#include <stdio.h>      //perror
#include <stdlib.h>     //malloc, //getenv
#include <sys/types.h>
#include <unistd.h>     //getcwd, chdir, rmdir
#include <sys/wait.h>
#include<string.h> //strcmp
#include<ctype.h>   //isgraph

//#include "MyDefShell.h"


/*
 *  定数の定義
 */

#define BUFLEN    1024     /* コマンド用のバッファの大きさ */
#define MAXARGNUM  256     /* 最大の引数の数 */

#define MAX_PATH 256
#define MAX_COMMAND 32      //512
#define  MAX_HISTORY 5
#define  MAX_PROMPT 16
#define  MAX_ALIAS 32
#define MAX_LINES 1024      //cat関数で使用
#define  MAX_CHAR  512      //cat関数で使用

/*typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;
*/
//extern node_tag *pTop;

/*
 *  ローカルプロトタイプ宣言
 */
void child(char *argv[MAXARGNUM]);

int parse(char [], char *[]);
void execute_command(char *[], int);
void execute_function(char *[]);
int pushd(char *[]);
int ls(char *[]);
int pwd(char *[]);
int cd(char *[]);
void init_DirStack();
int dirs();
int popd();
void init_HisStack();
int record_history(char *[]);
int show_history(char *[]);
int former_history(char *[]);
int funcs_history(char *[]);
int nth_hisotry(char *[]);
int nformer_history(char *[]);
//int search_history();
int string_history(char *[]);
void show_command(char *[]);
void strcpy_skip(char*, char*, int);
char *wildcard(char []);
int func_prompt(char *[]);

int alias(char *[]);
void init_AliasStack();
int show_alias();
int record_alias();
int unalias(char *[]);
int check_alias(char *[]);
int delete_alias(char *[]);

int rm(char *[]);
int my_mkdir(char *[]);
int my_rmdir(char *[]);
int cat(char *[]);

char prompt[MAX_PROMPT] = "Command:";



/*---[lsに必要な宣言など]----------------------------------------------------------------------*/
//#define PATH_SIZEぐらい
/*-----------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
 *
 *  関数名   : main
 *
 *  作業内容 : シェルのプロンプトを実現する
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    char command_buffer[BUFLEN]; /* コマンド用のバッファ */
    char *buffer_p; /* コマンド用のバッファ */
    char *args[MAXARGNUM];       /* 引数へのポインタの配列 */
    int command_status;          /* コマンドの状態を表す
                                    command_status = 0 : フォアグラウンドで実行
                                    command_status = 1 : バックグラウンドで実行
                                    command_status = 2 : シェルの終了
                                    command_status = 3 : 何もしない */
    /*
     *  無限にループする

     */
    init_DirStack();
    init_HisStack();
    init_AliasStack();
    for(;;) {

        /*
         *  プロンプトを表示する
         */

        printf("%s ", prompt);
        /*
         *  標準入力から１行を command_buffer へ読み込む
         *  入力が何もなければ改行を出力してプロンプト表示へ戻る
         */

        if(fgets(command_buffer, BUFLEN, stdin) == NULL) {
            printf("\n");
            continue;
        }

        /*
         *  入力されたバッファ内のコマンドを解析する
         *
         *  返り値はコマンドの状態
         */
        //ワイルドカードの実現
        //printf("length:%ld\n", strlen(wildcard(command_buffer)));
 
        /*printf("buffer_p:%s\n", buffer_p);
        if(buffer_p != NULL){
            strcpy(command_buffer, buffer_p);
        }
        */
        if(strstr(command_buffer, "*") != NULL){
            if((buffer_p =wildcard(command_buffer)) == NULL){
                printf("2.The command is too long.\n");
                continue;         
            }else{
                printf("buffer_p:%s\n", buffer_p);
                if(buffer_p != NULL){
                    strcpy(command_buffer, buffer_p);
                }  
            } 
        }       
        command_status = parse(command_buffer, args);
        printf("status:%d\n", command_status);

int k=0;
for (k = 0;  args[k] != NULL; k++) {
printf("args[%d]:%s\n", k, args[k]);
}


        /*
         *  終了コマンドならばプログラムを終了
         *  引数が何もなければプロンプト表示へ戻る
         */

        if(command_status == 2) {
            printf("done.\n");
            exit(0);        //EXIT_SUCCESS とすると終了しない．なぜ？***
        } else if(command_status == 3) {
            continue;       //再びfor文の先頭へ
        }

        /*
         *  コマンドを実行する
         */
        //status = 0 or 1
        record_history(args);
        execute_command(args, command_status);
    }

    return 0;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : parse
 *
 *  作業内容 : バッファ内のコマンドと引数を解析する
 *
 *  引数     :
 *
 *  返り値   : コマンドの状態を表す :
 *                0 : フォアグラウンドで実行
 *                1 : バックグラウンドで実行
 *                2 : シェルの終了
 *                3 : 何もしない
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

char *wildcard(char buffer[]){
    DIR *dir;
    struct dirent *dir_elements;
    char current_path[MAX_PATH];
    char result[BUFLEN];
    char *p_result;
    char *p;
    char *p_tmp;
    int j=0;
    int i=0;

 /*   if(strstr(buffer, "*") == NULL){
        return NULL;
    }
*/    
    getcwd(current_path, MAX_PATH);
    printf("current_path:\"%s\n\"", current_path);

    dir=opendir(current_path);
    if (dir == NULL) {
        printf("Couldn't open the dir\n");//****
        return 0;
    }

    //文字列を区切り文字で分解する
    p = strtok(buffer, "*\n");        //"*"は1つしか含まれていないものとする
    p_tmp = p;      //"*"の一つ前のポインタで，*string, string*を処理する時に使う
    printf("buffer:%s\n", buffer);
    printf("j[%d]:%s\n",j, p);
    strcpy(result, p);
    //p_tmp = strtok(p, " ");
    p = strtok(NULL,"*");
    //string*
    //*string
    //*のみ
    if(isgraph(*(p-2))){            //"cp l*s"などの場合は"string*"で処理
        printf("execute:string*\n");    
        for(i=0; strncmp((p-2-i), " ", 1) != 0; i++){
            strcpy(&result[strlen(p_tmp)-i-1], "\0");
            printf("i:%d, *(p-2-i):%c, result:%s\n", i, *(p-2-i), result);
        }
        strcpy(&result[strlen(p_tmp)-i-1], "\0");
        printf("*(p-2-i):%c, i:%d\n",*(p-1-i), i);
        printf("*(p-2):%c, i:%d\n",*(p-2), i);
        printf("i:%d,result:%s, strlen(result):%ld\n", i, result,strlen(result));
        while((dir_elements = readdir(dir)) != NULL){
            printf("%s\n", dir_elements->d_name);
            if(strncmp(dir_elements->d_name, (p-1-i), i) == 0){     //(p-2-i)でなく(p-1-i)であるのは,(p-2-i)が " "であるため
                printf("+%s\n", dir_elements->d_name);  
                if((strlen(result)+strlen(dir_elements->d_name)+1) > BUFLEN){//最後に+1は空白分。
                    printf("length:%ld\n", strlen(result));
                    printf("0.The command is too long.\n");
                    return NULL;  
                }
                strcat(result, " ");
                strcat(result, dir_elements->d_name);
            }

        }

    }else if(isgraph(*p) ){
    //if(strncmp(p, "\n") == 0 || strncmp(p, "\0") == 0 || strncmp(p, " ") == 0/*|| p==NULL */){
        printf("execute:*string\n");
        for(i=0; isprint(*(p+i)) && strncmp((p+i)," ",1); i++){
            
            printf("i:%d, *(p+i):%c, result:%s, strlen(p_tmp):%ld, strlen(result):%ld\n", i, *(p+i), result,strlen(p_tmp),strlen(result));
        }//i=6
        strcpy(&result[strlen(p_tmp)-1], "\0");
        while((dir_elements = readdir(dir)) != NULL){
            printf("%s,(dir_elements->d_name-i):%c\n", dir_elements->d_name,*(dir_elements->d_name+strlen(dir_elements->d_name)-i));
            if(strncmp((dir_elements->d_name+strlen(dir_elements->d_name)-i), p, i) == 0){     //(p-2-i)でなく(p-1-i)であるのは,(p-2-i)が " "であるため
            //strrchrでもできたかも.でも"ls *s -la"のように*stringの後にも文字が続く場合に対応できなく，コードの複雑さは同じの可能性が高い
                printf("+%s\n", dir_elements->d_name);  
                if((strlen(result)+strlen(dir_elements->d_name)+1) > BUFLEN){//最後に+1は空白分
                    printf("length:%ld\n", strlen(result));
                    printf("0.The command is too long.\n");
                    return NULL;  
                }
                strcat(result, " ");
                strcat(result, dir_elements->d_name);
            }
           
        }
         p = p+i;
    }else{
        printf("execute:only *\n");
        while((dir_elements = readdir(dir)) != NULL){
            printf("%s\n", dir_elements->d_name);
            if((strlen(result)+strlen(dir_elements->d_name)+1) > BUFLEN){//最後に+1は空白分。
                printf("length:%ld\n", strlen(result));

                printf("0.The command is too long.\n");
                return NULL;  
            }
            strcat(result, " ");
            strcat(result, dir_elements->d_name);
        }
    }

   /* while((dir_elements = readdir(dir)) != NULL){
        printf("%s\n", dir_elements->d_name);
        if((strlen(result)+strlen(dir_elements->d_name)+1) > BUFLEN){//最後に+1は空白分。
            printf("length:%ld\n", strlen(result));

            printf("0.The command is too long.\n");
            return NULL;  
        }
        strcat(result, " ");
        strcat(result, dir_elements->d_name);
    }*/
    printf("1.result:%s\n", result);

    if(p!=NULL){
        strcat(result, p);
    }
    printf("result:%s\n", result);
    closedir(dir);
    if(strlen(result) > BUFLEN){
        printf("length:%ld\n", strlen(result));

        printf("1.The command is too long.\n");
        return NULL;
    }
    p_result = &result[0];
    printf("2222\n");
    printf("length:%ld\n", strlen(p_result));
    return p_result;

}
//エラー処理***
//https://www.delftstack.com/ja/howto/c/opendir-in-c/#%25E3%2583%2587%25E3%2582%25A3%25E3%2583%25AC%25E3%2582%25AF%25E3%2583%2588%25E3%2583%25AA%25E3%2582%25A8%25E3%2583%25B3%25E3%2583%2588%25E3%2583%25AA%25E3%2581%25AE%25E5%258F%258D%25E5%25BE%25A9%25E5%2587%25A6%25E7%2590%2586%25E3%2581%25AB-readdir-%25E9%2596%25A2%25E6%2595%25B0%25E3%2582%2592%25E4%25BD%25BF%25E7%2594%25A8%25E3%2581%2599%25E3%2582%258B
/*
int wildcard(char *args[]){

}
*/
int parse(char buffer[],        /* バッファ */
          char *args[])         /* 引数へのポインタ配列 */
{
    int arg_index;   /* 引数用のインデックス */
    int status;   /* コマンドの状態を表す */

    /*
     *  変数の初期化
     */

    arg_index = 0;
    status = 0;

    /*
     *  バッファ内の最後にある改行をヌル文字へ変更
     */

    *(buffer + (strlen(buffer) - 1)) = '\0';

    /*
     *  バッファが終了を表すコマンド（"exit"）ならば
     *  コマンドの状態を表す返り値を 2 に設定してリターンする
     */

    if(strcmp(buffer, "exit") == 0) {

        status = 2;
        return status;
    }

    /*
     *  バッファ内の文字がなくなるまで繰り返す
     *  （ヌル文字が出てくるまで繰り返す）
     */

    while(*buffer != '\0') {

        /*
         *  空白類（空白とタブ）をヌル文字に置き換える
         *  これによってバッファ内の各引数が分割される
         */

        while(*buffer == ' ' || *buffer == '\t') {
            *(buffer++) = '\0';
        }

        /*
         * 空白の後が終端文字であればループを抜ける
         */

        if(*buffer == '\0') {
            break;
        }

        /*
         *  空白部分は読み飛ばされたはず
         *  buffer は現在は arg_index + 1 個めの引数の先頭を指している
         *
         *  引数の先頭へのポインタを引数へのポインタ配列に格納する
         */

        args[arg_index] = buffer;
        ++arg_index;

        /*
         *  引数部分を読み飛ばす
         *  （ヌル文字でも空白類でもない場合に読み進める）
         */

        while((*buffer != '\0') && (*buffer != ' ') && (*buffer != '\t')) {
            ++buffer;
        }
    }

    /*
     *  最後の引数の次にはヌルへのポインタを格納する
     */

    args[arg_index] = NULL;

    /*
     *  最後の引数をチェックして "&" ならば
     *
     *  "&" を引数から削る
     *  コマンドの状態を表す status に 1 を設定する
     *
     *  そうでなければ status に 0 を設定する
     */

    if(arg_index > 0 && strcmp(args[arg_index - 1], "&") == 0) {

        --arg_index;
        args[arg_index] = '\0';
        status = 1;

    } else {

        status = 0;

    }

    /*
     *  引数が何もなかった場合
     */

    if(arg_index == 0) {
        status = 3;
    }

    /*
     *  コマンドの状態を返す
     */
    //*args[argc+1] = NULL execvp(args[0], args);のために必要ないのか
    for (int i=0; i<arg_index; i++){
        printf("arg[%d] : %c\n", i, *args[i]); //firefox & の時はsegment error, cd a.out などはaが出力される
    }
    return status;
}

/*----------------------------------------------------------------------------
 *
 *  関数名   : execute_command
 *
 *  作業内容 : 引数として与えられたコマンドを実行する
 *             コマンドの状態がフォアグラウンドならば、コマンドを
 *             実行している子プロセスの終了を待つ
 *             バックグラウンドならば子プロセスの終了を待たずに
 *             main 関数に返る（プロンプト表示に戻る）
 *
 *  引数     :
 *
 *  返り値   :
 *
 *  注意     :
 *
 *--------------------------------------------------------------------------*/

void execute_command(char *args[],    /* 引数の配列 */
                     int command_status)     /* コマンドの状態 */
{
    int pid;      /* プロセスＩＤ */
    int status;   /* 子プロセスの終了ステータス */

    int ret;
    printf("Parent PID by getppid() = %d\n", getppid());
    execute_function(args);

}

/*---[pushdに必要な宣言など]----------------------------------------------------------------------*/

typedef struct node{
    char*           dir_path;
    struct node*    Prev;
    struct node*    Next;
}node_tag;

typedef struct history{
    char*           commands[MAXARGNUM];
    struct history*    Prev;
    struct history*    Next;
}his_tag;

typedef struct alias{
    char* command;
    char* new_command;
    //struct alias*    Prev;//要らないかも
    struct alias*    Next;
}alias_tag;

node_tag pTop;   //*Topにしたければ実態を代入する必要あり
node_tag *pTail;   //*Topにしたければ実態を代入する必要あり

his_tag pTop_his;   //*Topにしたければ実態を代入する必要あり
his_tag *pTail_his;   //*Topにしたければ実態を代入する必要あり

his_tag *pMid_his;   //*Topにしたければ実態を代入する必要あり

alias_tag pTop_alias;

/*-----------------------------------------------------------------------------------------------*/
/*
    char *p_ls = "ls";
    char *p_cd = "cd";
    char *p_pushd = "pushd";
    char *p_dirs = "dirs";
    char *p_popd = "popd";
    char *p_show_his = "history";
    char *p_former_his = "!!";
    char *p_search_his = "!";
    char *p_prompt = "prompt";
    char *p_alias = "alias";
    char *p_unalias = "unalias";
*/

char *str_commands[] = {
    "ls",
    "pwd",
    "cd",
    "pushd",
    "dirs",
    "popd",
    "history",
    "!!",
    "!",
    "prompt",
    "alias",
    "unalias",
    "rm",
    "mkdir",
    "rmdir",
    "cat"
};
int (*funcs_command[])(char *[]) = {
    &ls,
    &pwd,
    &cd,
    &pushd,
    &dirs,
    &popd,
    &show_history,
    &former_history,
    &funcs_history,
    &func_prompt,
    &alias,
    &unalias,
    &rm,
    &my_mkdir,
    &my_rmdir,
    &cat
}; 
int num_funcs() {
    return sizeof(str_commands) / sizeof(char *);
}






void execute_function(char* args[]){    //args[0]:コマンド名および関数名，args[1]以降：コマンドの引数
    printf("execute_function_args[0]:%s\n", args[0]); 
/*    char n_ls[MAX_ALIAS] = "ls";
    char n_cd[MAX_ALIAS] = "cd";
    char n_pushd[MAX_ALIAS] = "pushd";
    char n_dirs[MAX_ALIAS] = "dirs";
    char n_popd[MAX_ALIAS] = "popd";
    char n_show_his[MAX_ALIAS] = "history";
    char n_former_his[MAX_ALIAS] = "!!";
    char n_search_his[MAX_ALIAS] = "!";
    char n_prompt[MAX_ALIAS] = "prompt";
    char n_alias[MAX_ALIAS] = "alias";
    char n_unalias[MAX_ALIAS] = "unalias";
*/
   /* char n_ls = "ls";
    char n_cd = "cd";
    char n_pushd = "pushd";
    char n_dirs = "dirs";
    char n_popd = "popd";
    char n_show_his = "history";
    char n_former_his = "!!";
    char n_search_his = "!";
    char n_prompt = "prompt";
    char n_alias = "alias";
    char n_unalias = "unalias";
    char *p_ls = &n_ls;
    char *p_cd = &n_cd;
    char *p_pushd = &n_pushd;
    char *p_dirs = &n_dirs;
    char *p_popd = &n_pushd;
    char *p_show_his = &n_show_his;
    char *p_former_his = &n_former_his;
    char *p_search_his = &n_search_his;
    char *p_prompt = &n_prompt;
    char *p_alias = &n_alias;
    char *p_unalias = &n_unalias;
*/
    
    int i;
    int j=0;
    for (i = 0; i < num_funcs(); i++) {
        if (strcmp(args[0], str_commands[i]) == 0) {
            printf("execute:%s\n", str_commands[i]);
            (*funcs_command[i])(args);
            node_tag *p;
            
            for(p=&pTop; p->Next != NULL ; p = p->Next){
                printf("j[%d]=",j);
                printf("p->dir_path:%s\n", p->dir_path);
                j++;
            }
            break;
        }
        
    }
    if(strncmp(args[0], "!", 1) == 0){  //この時点で"!!"ではない
       funcs_history(args); 
    }
    check_alias(args);
    printf("execute_function_end\n");

/*printf("p_pushd:%s\n", p_pushd);
    if (strcmp(args[0] , p_ls)==0){
        printf("execute:ls\n");
        ls(args);
    }else if (strcmp(args[0] , p_cd)==0){
        printf("execute:cd\n");
        cd(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }
    }else if (strcmp(args[0] , p_pushd)==0){
        
        printf("execute:pushd\npushd1_function\n");
        printf("mainTop1:%s\n", pTop.dir_path);
        pushd(args); 
        /*node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        } */
/*
    }else if (strcmp(args[0] , p_dirs)==0){
        
        printf("execute:dirs\n");
        dirs(args); 
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_popd)==0){
        
        printf("execute:popd\n");
        popd(args);
        dirs(args); 
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_show_his)==0){
        
        printf("execute:history\n");
        show_history(args); 
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_former_his)==0){
        
        printf("execute:!!\n");
        former_history(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strncmp(args[0] , p_search_his, 1)==0){
        
        printf("execute:!string\n");
        string_history(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_prompt)==0){
        
        printf("execute:prompt\n");
        func_prompt(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_alias)==0){
        
        printf("execute:alias\n");
        alias(args);
        printf("p_command:%s\n", p_pushd);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else if (strcmp(args[0] , p_unalias)==0){
        
        printf("execute:unalias\n");
        unalias(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
    }else{
             
        printf("execute:check alias\n");
        check_alias(args);
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }     
    }
    printf("execute_function_end\n");*/
}




//拡張機能なし。隠しファイルも自動的に表示(ls -laと同じ)


//拡張機能なし。
//解放する必要あり(memset, getcwd, chdir)


int cd(char *args[]){
    char cd_path[MAX_PATH];
    char *HOME_path;
    if(args[1] == NULL){        //ディレクトリ名が指定されなかった場合, path_nameに環境変数 HOME に指定されたディレクトリへのパスを代入する
        HOME_path = getenv("HOME");
        args[1] = HOME_path;
        if (HOME_path == NULL) {
            printf("Couldn't find HOME(environment variable)\n");
            return 1;
            //異常終了
        } 
    }
    //memset(cd_path, '\0', MAX_PATH);    
    printf("current dir : %s\n", cd_path);
    if(chdir(args[1]) != 0){ 
        printf("Couldn't move to the path\n");
        perror("");
    }else{
        getcwd(cd_path, MAX_PATH);
        printf("cd_path:%s\n", cd_path);
    }
    node_tag *p;
    int i=0;
    for(p=&pTop; p->Next != NULL ; p = p->Next){
        printf("i[%d]=",i);
        printf("p->dir_path:%s\n", p->dir_path);
        i++;
    }
    return 0;
    
    //***問題:カレントディレクトリの変更が反映されない
}

void init_DirStack(){
    pTop.dir_path = "Top";
    pTop.Prev = NULL;   
    pTop.Next = NULL;   
}

int pushd(char *args[]){
    char pushd_path[MAX_PATH];
    //memset(pushd_path, '\0', MAX_PATH);    
    printf("current dir : %s\n", pushd_path);
    getcwd(pushd_path, MAX_PATH);


    node_tag *p;
    int i=0;
    for(p=&pTop; p->Next != NULL ; p = p->Next){
        printf("i[%d]=",i);
        printf("p->dir_path:%s\n", p->dir_path);
        i++;
    }
    node_tag *NewNode;
    NewNode = (node_tag*)malloc(sizeof(node_tag));   
    NewNode->dir_path = (char*)malloc(sizeof(char)*(strlen(pushd_path) + 1));
    
    if( NewNode == NULL ){
        fputs( "メモリ割り当てに失敗しました。", stderr );
        return 1;
    } 

    strcpy(NewNode->dir_path, pushd_path); // 文字列はコピーする

    NewNode->Prev = p;
    NewNode->Next = NULL;
    p->Next = NewNode;
    pTail = NewNode;  

    return 0;
}
//dirsコマンド
//案１：単方向リストのまま，一回値を取り出して，逆に表示する
//案２：双方向リストにして、後ろからも終えるようにする
//案２を採用。dirsコマンドだけでなく，popdコマンドでも末尾を使用するため．また，前から辿って後ろから再度表示させる実行時間と，node_tag->Prevに必要なデータ容量を比較してどちらが良いか。

int  dirs(){
    node_tag *p;
    int i=0;
    p=pTail;
    printf("---[dirs]----------------------------------------------------------------\n");
    if(p->Prev == NULL){    //  何もスタックされていない状態で表示を求められたとき
        printf("Nothing is stored in the directory stack.\n");        
        return 0;
    }
    while(p->Prev != NULL){
        printf("i[%d]=",i);
        printf("p->dir_path:%s\n", p->dir_path);
        i++;
        p = p->Prev;
    } 
    printf("-----------------------------------------------------\n");
    return 0;
}

int popd(){

    if(chdir(pTail->dir_path) != 0){ 
        printf("Couldn't move to the path\n");
        perror("");
        return 1;
    }
    
    return 0;
}

void init_HisStack(){
/* 配列tableの各要素（ポインタ）をNULLポインタで初期化 */
    int i;
    for (i = 0; i < MAXARGNUM; i++){
        pTop_his.commands[i] = NULL;
    }
    pTop_his.Prev = NULL;   
    pTop_his.Next = NULL;   
}


int record_history(char* args[]){

    his_tag *p;
    int i=0;
    int nHis = 0; //そのコマンドが何番目か表示するのに使用。デバッグ用
    for(p = &pTop_his; p->Next != NULL ; p = p->Next){
        i++;
    }
    nHis = i;

    his_tag *NewHis;
    NewHis = (his_tag*)malloc(sizeof(his_tag));  

    //for (i = 0; i < MAXARGNUM || args[i] != NULL; i++) {
    for (i = 0;  args[i] != NULL;) {
        /* 入力文字列の長さ＋１文字分のメモリを確保し、そのアドレスをポインタ配列tableの要素として記憶する */
         NewHis->commands[i] = (char*)malloc(sizeof(char)*(strlen(args[i])+1));  
        if( NewHis->commands[i] == NULL ){
            fputs( "メモリ割り当てに失敗しました。", stderr );
            return 1;
        }
        i++;
    }
    printf("xxxxxxxi:%d, %d, %s, %s\n", i, args[1]!=NULL, args[1], args[0]);
    /*if(i ==  MAX_HISTORY){
        printf("ヒストリとして保存するコマンドの上限値に達しました\n");
        printf("これ以降のコマンドは保存されません\n");
        return 1;
    }*/

    int j;
    for(j=0 ;args[j] != NULL;){
        strcpy(NewHis->commands[j], args[j]); // 文字列はコピーする
        j++;
    }
    printf("yyyyyyyj:%d, %d, %s\n", j, args[1]!=NULL, args[1]);

    //NewNode->dir_path = pushd_path;
    NewHis->Prev = p;
    NewHis->Next = NULL;
    p->Next = NewHis;
    pTail_his = NewHis;  
    if(nHis == MAX_HISTORY){
        pMid_his = NewHis; 
        printf("pMid_his commands:%s\n", pMid_his->commands[0]);     
    } 
    printf("commands[%d]:%s\n", nHis, NewHis->commands[0]); 
    return 0;
}

int show_history(char* args[]){ //本当は引数はいらないが、関数ポインタの配列で統一するために必要
    his_tag *p;
    int i=0;
    int j=0;
    p=pTail_his;
printf("args[0]:%s\n", args[0]);
    printf("-----[history_後ろから]-----------------------------------------------\n");

    while(p->Prev != NULL /*&& i < MAX_HISTORY*/){
//printf("111\n");
        printf("i[%d]=",i);
//printf("112\n");
        printf("p->commands:");
//printf("113\n");
//printf("p->commands[0]:%s, p->commands[1]:%s\n", p->commands[0], p->commands[1]);
//printf("p->commands[0]:%s\n", p->commands[0]);
        for(j=0; p->commands[j] != NULL;){
//printf("114\n");
            printf("%s ", p->commands[j]);
//printf("115\n");
            j++;
        }
//printf("116\n");
        p = p->Prev;
        i++;
        printf("\n");
    }
    printf("-----------------------------------------\n");

    p = &pTop_his;
    i = 0;
    p = p->Next;
        printf("-----[history_前から]-----------------------------------------------\n");
    while(p->Next != NULL && i < MAX_HISTORY){
        printf("i[%d]=",i);
        printf("p->commands:");
        for(j=0; p->commands[j] != NULL;){
        printf("%s ", p->commands[j]);
        j++;
        }
        p = p->Next;
        i++;
        printf("\n");
    }
    //ここからもっときれいにコード書けるはず
     printf("i[%d]=",i);
    printf("p->commands:");
    for(j=0; p->commands[j] != NULL;){
    printf("%s ", p->commands[j]);
    j++;
    }
    printf("\n");
    printf("-----------------------------------------\n");
    return 0;
}


//上限値が32個だからと言って，32こで保存をやめてしまうと，!!や!stringコマンドで不具合が出るので，結局すべて保存する
int former_history(char* args[]){//本当は引数はいらないが、関数ポインタの配列で統一するために必要
    his_tag *p;
    p = pTail_his->Prev;
    if(p->Prev == NULL){
        printf("Counldn't execute the former command\n");
        exit(1);        //return 1;とすると二回目にも”!!”とすると，無限ループに入る
    }
//一番最初にこのコマンドを打った時にエラーを出す必要あり
    printf("former_command:%s\n", p->commands[0]); 
    printf("execute:run former commands\n");
    execute_function(p->commands);      //execute_commandsでも可
    printf("execute:record former commands\n"); 
    record_history(p->commands); 
    return 0;
}
//!string, !n, !-nで場合分け
int funcs_history(char *args[]){
        printf("*(args[0]+1):%c\n", *(args[0]+1));
    if(strncmp(args[0], "!-", 2) == 0){
        nformer_history(args);
    }else if (*(args[0]+1)>='0'&& *(args[0]+1)<='9'){
        nth_hisotry(args);
    }else{
        string_history(args);
    }
    return 0;

}
int nth_hisotry(char *args[]){      //nは０から数える
    printf("execute: !n\n");
    his_tag *p;
    int i=0;
    int j;
    /* p=pT_his;//pMid_his < pTop_his;一つ前は確実に存在し，"!p"そのものであるために飛ばす****確認必要
    //p=pMid_hisにすると、!stringでバグる****
    //p=p->Prev; //一つ前は確実に存在し，"!p"そのものであるために飛ばす****確認
    */
    p = &pTop_his;
    p = p->Next;
    char str_n[MAX_COMMAND-1];        //数字なので別のMaxを用意してもいいかも。
    int int_n;
    char *endptr;
 printf("1\n");
    strcpy_skip(str_n, args[0], 1);
    int_n = (int)strtol(str_n, &endptr,10);
    if(*endptr != '\0'){
        printf("Please enter only the number.\n");
        return 1;
    }
    printf("str_n:\"%s\", args:",str_n);
    show_command(args);
    printf("\n");
printf("2\n");
    while(p->Next != NULL){
        printf("i[%d]=",i);
        printf("p->commands:");
        for(j=0; p->commands[j] != NULL;){
        printf("%s ", p->commands[j]);
        j++;
        }
        if(int_n == i){
            printf("execute:commands ");
            show_command(p->commands);
            printf("\n");
            execute_function(p->commands);      //execute_commandsでも可

            record_history(p->commands); 

            return 0;
        }
        p = p->Next;
        i++;
        printf("\n");
    }
    printf("Now only %d history is stored.\n", i);
    printf("Couldn't execute the %s command\n", str_n);  
    return 0;
}
int nformer_history(char *args[]){
    printf("execute: !-n\n");
    his_tag *p;
    int i=0;
    int j=0;
    p=pTail_his;
    char str_n[MAX_COMMAND-2];        //数字なので別のMaxを用意してもいいかも。
    int int_n;
    char *endptr;
    strcpy_skip(str_n, args[0], 2);
    int_n = (int)strtol(str_n, &endptr,10);
    if(*endptr != '\0'){
        printf("Please enter only the number.\n");
        return 1;
    }
    printf("str_n:\"%s\", args:",str_n);
    show_command(args);
    printf("\n");

    while(p->Prev != NULL /*&& i < MAX_HISTORY*/){
        printf("i[%d]=",i);
        printf("p->commands:");
//printf("p->commands[0]:%s, p->commands[1]:%s\n", p->commands[0], p->commands[1]);
printf("p->commands[0]:%s\n", p->commands[0]);
        for(j=0; p->commands[j] != NULL;){
            printf("%s ", p->commands[j]);
            j++;
        }
        if(int_n == i){
            printf("execute:commands ");
            show_command(p->commands);
            printf("\n");
            execute_function(p->commands);      //execute_commandsでも可

            record_history(p->commands); 

            return 0;
        }
printf("116\n");
        p = p->Prev;
        i++;
        printf("\n");
    }
    printf("Now only %d history is stored.\n", i);
    printf("Couldn't execute the %s command from the back.\n", str_n);  

    return 0;
}

int string_history(char *args[]){
    printf("execute: !string\n");
    his_tag *p;
    int i=0;
    p=pTail_his;//pMid_his < pTop_his;一つ前は確実に存在し，"!p"そのものであるために飛ばす****確認必要
    //p=pMid_hisにすると、!stringでバグる****
    //p=p->Prev; //一つ前は確実に存在し，"!p"そのものであるために飛ばす****確認
    char args_tmp[MAX_COMMAND];
printf("1\n");
    strcpy_skip(args_tmp, args[0], 1);
    printf("args_tmp:\"%s\", args:",args_tmp);
    show_command(args);
    printf("\n");
printf("2\n");

    while(p->Prev != NULL /*&& i < MAX_HISTORY*/){
        printf("i[%d]=",i);
        printf("p->commands:");
        show_command(p->commands);
        printf("\n");   
        printf("[p->commadns[0]:%s]\n",p->commands[0]);     
        printf("[args_tmp:%s]\n",args_tmp);     
        if(strncmp(args_tmp, p->commands[0], strlen(args_tmp))== 0){
            printf("execute:commands ");
            show_command(p->commands);
            printf("\n");
            execute_function(p->commands);      //execute_commandsでも可

            record_history(p->commands); 

            return 0;
        }//strchr(args_tmp, p->commands)

        p = p->Prev;
        i++;
        printf("\n");
    }
    printf("Couldn't execute such a command\n");
    return 1;

}

/*----------------------------------------------------------------------------
 *
 *  関数名   : strcpy_skip
 *
 *  作業内容 : 
 *
 *  引数     :
 *
 *  返り値   : なし．第一引数のsに，第二引数のtの文字配列のn文字目以降をそのまま返す．
 *
 *  注意     :
 *
 *-------------------------------------------------------------------------*/

void strcpy_skip(char *s, char *t, int n)
{
	int	i;
	i = n;

	while((s[i-n] = t[i]) != '\0'){
        s[i-n] = t[i];
        i++;
    }
		
}

void show_command(char *command[]){
    int i;
    for(i=0; command[i] != NULL;){
        printf("%s ", command[i]);
        i++;
    }
}

int  func_prompt(char *args[]){
    char str_prompt[MAX_PROMPT];
    if(strlen(args[1]) > MAX_PROMPT){
        printf("Counldn't change prompt\n");
        return 1;
    }
    strcpy(prompt, args[1]);
    printf("prompt:%s, args[1]:%s\n",prompt, args[1]);


    return 0;
}
int alias(char *args[]){
    int i;
    if(args[1] == NULL){
        show_alias();
        printf("end execute show_alias\n");
        return 0;
    }else{
        //command2のポインタを探す
        //[案１]関数呼び出し前に探索→この関数に渡してもらう
        //そもそもエイリアスを付けたいコマンドがあるか、確認
        int flag=0;
        for (i = 0; i < num_funcs(); i++) {
            if (strcmp(args[2], str_commands[i]) == 0) {
                flag++;
                break;
            }
        }
        if(flag == 0){
            printf("The command doesn't exist.\n");
            return 0;
        }
        char *p_new_command;
        //動的にコマンド別名のchar型配列を確保する
        p_new_command = (char*)malloc(sizeof(char)*(MAX_ALIAS+1));
        if(p_new_command==NULL){
            printf("Couldn't change the name of the command\n");
            return 1;
        }
        if(strlen(args[1]) > MAX_ALIAS){
            printf("Couldn't change the name of the command\n");
            return 1;            
        }       //***まとめてもいいかも
        strcpy(p_new_command, args[1]);
        //command2のポインタにcommand1のポインタを代入する
printf("1,p_command:%s,  p_new_command:%s\n", args[2], p_new_command);
        //p_pushd = p_new_command;
printf("2,p_command:%s,  p_new_command:%s\n", args[2], p_new_command);
        record_alias(args[2], p_new_command);
        show_alias();

        return 0;

    }
}

int check_alias(char* args[]){
    alias_tag *p;
    p = &pTop_alias;
    if(p->Next == NULL){    //  何も記録していない状態で表示を求められたとき
        printf("No alias is set.\n");
        return 0;
    }
    p = p->Next;
    while(p != NULL){   //p->Next != NULL
        if(strcmp(p->new_command, args[0]) == 0){
            printf("p->command:%s\n", p->command);
            args[0] = p->command;
            printf("args[0]:%s\n", args[0]);
            execute_function(args);
        }
        p = p->Next;
    }
    return 0;
}

//最後とunalias時にfreeを忘れない
//s2の指しているデータを、s1にコピーします。s1の指すメモリブロックの大きさがs2のそれよりも小さいと、他のデータに上書きをしてしまう可能性があります。その結果、何が起こるか予測がつきません。動作が異常になるか、最悪の場合システムがクラッシュして暴走します
void init_AliasStack(){
/* 配列tableの各要素（ポインタ）をNULLポインタで初期化 */
    pTop_alias.command =NULL;
    pTop_alias.new_command = NULL; 
    pTop_alias.Next = NULL;   
}


int show_alias(){
    alias_tag *p;
    int i=0;
    p = &pTop_alias;
    if(p->Next == NULL){    //  何も記録していない状態で表示を求められたとき
        printf("-----[alias_前から]-----------------------------------------------\n"); 
        printf("No alias is set.\n");
        printf("-----------------------------------------\n");
        
        return 0;
    }
    p = p->Next;
    printf("-----[alias_前から]-----------------------------------------------\n");
    while(p->Next != NULL){
        printf("i[%d]=",i);
        printf("p->command:%s p->new_command:%s\n", p->command, p->new_command);
        p = p->Next;
        i++;
    }
    //ここまでだけだと、最後の一つが表示されない
    printf("i[%d]=",i);
    printf("p->command:%s p->new_command:%s\n", p->command, p->new_command);
    printf("-----------------------------------------\n");
    return 0;
}
//hisやstackを利用して、短方向リストでするか、配列の動的確保で実現するか
//単方向リストは削除が新たに作るのが手間だが、配列では二つの情報を保存できない
int record_alias(char *p_command, char *p_new_command){
printf("0\n");
  
    alias_tag *p;
    int i=0;
    for(p = &pTop_alias; p->Next != NULL ; p = p->Next);
printf("1\n");
    alias_tag *Newalias;
    Newalias = (alias_tag*)malloc(sizeof(alias_tag));
printf("2\n");

    Newalias->command = p_command;  
    Newalias->new_command = p_new_command; 
    Newalias->Next = NULL; 
    p->Next = Newalias;
   // pTail_his = NewHis;***  
printf("3\n");

    return 0;
}

int unalias(char *args[]){
    alias_tag *p;
    int i=0;
    p = &pTop_alias;
    if(p->Next == NULL){    //  何も記録していない状態で表示を求められたとき
        printf("No alias is set.\n");
        return 0;
    }
    p = p->Next;
    while(p != NULL){   //p->Next != NULL
        if(strcmp(p->new_command, args[1]) == 0){
            args[0] = p->command;
            printf("args[1]:%s\n", args[1]);
            delete_alias(args);
            return 0;
        }
        p = p->Next;
        i++;
    }
    return 0;
}

int delete_alias(char* args[]){
    alias_tag *pNow;
    alias_tag *pPrev = NULL;
    pNow = &pTop_alias;
    if(pNow->Next == NULL){    //  何も記録していない状態で表示を求められたとき
        printf("No alias is set.\n");
        return 0;
    }
    pPrev = pNow;
    pNow = pNow->Next;
    while(pNow != NULL){   //p->Next != NULL
        if(strcmp(pNow->new_command, args[1]) == 0){
            printf("args[1]:%s\n", args[1]);
            /*消すノードの直前と直後をつなぐ*/
            pPrev->Next = pNow->Next;
            /*ノードを消す*/
            free(pNow);
            return 0;
        }
        pPrev = pNow;
        pNow = pNow->Next;
    }
    printf("The alias doesn't exist.\n");
    return 0;
}

 
int ls(char *args[]){
    DIR *dir;
    struct dirent *dir_info;

    char current_path[MAX_PATH];
    getcwd(current_path, MAX_PATH);
    printf("current dir : %s\n", current_path);
    char *path =  current_path;

    if (args[1] != NULL){
        path = args[1];
    }
    printf("path:%s\n", path);
    dir = opendir(path);

    if (dir == NULL){
        printf("Error\n");
        return 1;
    }

    while((dir_info = readdir(dir)) != NULL){
        printf("%s\n", dir_info->d_name);
    }
    closedir(dir);
    return 0;
    
}

int pwd(char* args[]){
    char current_path[MAX_PATH];
    
    if(getcwd(current_path, MAX_PATH) == NULL){
        printf("Couldn't get the current directory.\n");
        return 1;
    }
    printf("%s\n", current_path);
    return 0;
}

int mv(char* args[]){
    //p53
    //ファイルの移動:mv [ファイル名1] [存在しているディレクトリ名2]
    //ディレクトリの移動:mv [ディレクトリ名1] [存在しているディレクトリ名2] p74
    //ファイルの名前変更：mv [ファイル名1] [ファイル名2](存在しているファイル名を指定すると上書きされる．)
    //ディレクトリの名前変更：mv [ディレクトリ名1] [存在しないディレクトリ名2]
    return 0;
}

int rm(char* args[]){
    //p54
    //rm [なし/-i] [ファイル名]：ファイルを削除する。-iで確認メッセージを表示する
    //ただしファイルはカレントディレクトリにあるものとする．
    char current_path[MAX_PATH];
    char file[MAX_PATH];
    char c[1];

    if(getcwd(current_path, MAX_PATH) == NULL){
        printf("Couldn't get the current directory.\n");
        return 1;
    }

    if((strlen(current_path)+1) > MAX_PATH){//+1は/のため, strcpyとstrcatの両方のエラー検出
        printf("The name of the path is too long\n");
        return 1;
    }
    strcat(current_path, "/");
    printf("%s\n", current_path);

    int i;
    if(strcmp(args[1], "-i") == 0){
        for(i=2; args[i] !=NULL ; i++){
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);
            printf("rm: remove directory \"%s\"?(y/n):", args[i]);
            scanf("%1s%*[^\n]", c);
            getchar();
            //https://marycore.jp/prog/c-lang/scanf-string-safely/

            if(strncmp(c, "y", 1)!=0){
                continue;
            }
            if(remove(file) == 0){
                printf("%sを削除しました。\n", args[i]);
            }else{
                printf("rm:Couldn't remove \"%s\"\n", args[i]);
            }
        }
    }else{
        for(i=1; args[i] !=NULL ; i++){

            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);
            if(remove(file) == 0){
                printf("%sを削除しました。\n", args[i]);
            }else{
                printf("rm:Couldn't remove \"%s\"\n", args[i]);
            }
        }
 
    }
    return 0;
}


int my_mkdir(char *args[]){
    //mkdir [-p/なし] [ディレクトリ名s]：－pを付けると途中のディレクトリも含めて作成する。
    //p71
    //指定したディレクトリが既に存在する場合、mkdir関数は失敗します。
    //指定したディテクトリ名に存在しないディレクトリ名が含まれる場合も失敗します。
    //ただしファイルはカレントディレクトリにあるものとする．
    char current_path[MAX_PATH];
    char file[MAX_PATH];
    char c[1];

    if(getcwd(current_path, MAX_PATH) == NULL){
        printf("Couldn't get the current directory.\n");
        return 1;
    }

    if((strlen(current_path)+1) > MAX_PATH){//+1は/のため, strcpyとstrcatの両方のエラー検出
        printf("The name of the path is too long\n");
        return 1;
    }
    strcat(current_path, "/");
    printf("%s\n", current_path);
    //オプションはなしで
    int i;
    for(i=1; args[i] !=NULL ; i++){
        strcpy(file,current_path);

        if((strlen(args[i])+strlen(file)) > MAX_PATH){
            printf("The name of command is too long.\n");
            return 1;
        }
        strcat(file,args[i]);
        printf("%s\n", file);
        if (mkdir(file,
                S_IRUSR | S_IWUSR | S_IXUSR |
                S_IRGRP | S_IWGRP | S_IXGRP |
                S_IROTH | S_IWOTH | S_IXOTH) == 0){
            printf("ディレクトリ%sを作成しました。\n", file);
        }else{
            printf("Couldn't create a new directory.\n。");

        }

    }
    return 0;
}


int my_rmdir(char* args[]){
    //rmdir [-ri/-p/なし] [ディレクトリ名s]:－riを付けると確認しながらディレクトリを削除する。ただし空のディレクトリしか削除できない。
    //－pを付けると途中のディレクトリも含めて削除する。p72
    //ディレクトリの削除

    char current_path[MAX_PATH];
    char file[MAX_PATH];
    char c[1];

    if(getcwd(current_path, MAX_PATH) == NULL){
        printf("Couldn't get the current directory.\n");
        return 1;
    }

    if((strlen(current_path)+1) > MAX_PATH){//+1は/のため, strcpyとstrcatの両方のエラー検出
        printf("The name of the path is too long\n");
        return 1;
    }
    strcat(current_path, "/");
    printf("%s\n", current_path);
//オプションはなし/-riのみで→－pを付けると途中のディレクトリも含めて削除する(https://nidea.jp/lang-c/2022/01/directory-delete/#index_id0)
//空ディレクトリのみ
    int i;
    if(strcmp(args[1], "-ri") == 0){
        for(i=2; args[i] !=NULL ; i++){
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);
            printf("rm: remove directory \"%s\"?(y/n):", args[i]);
            scanf("%1s%*[^\n]", c);
            getchar();
            //https://marycore.jp/prog/c-lang/scanf-string-safely/

            if(strncmp(c, "y", 1)!=0){
                printf("No\n");
                continue;
            }
            if (rmdir(file) == 0){
                printf("ディレクトリ%sを削除しました。\n", file);
            }else{
                printf("Couldn't remove \"%s\" directory.\n。",file);

            }

        }
    }else{
        for(i=1; args[i] !=NULL ; i++){
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);
            if (rmdir(file) == 0){
                printf("ディレクトリ%sを削除しました。\n", file);
            }else{
                printf("Couldn't remove \"%s\" directory.\n。",file);

            }

        }
 
    }
    return 0;
}

int cat(char* args[]){
    //cat [-n/-b/なし] [ファイル名]：-nで行番号を，-bで空白を除いて行番号を振って，ファイルの内容を表示する．
   //-l 数字でその数字の行数だけ表示
   //バグ：-l, -b,完成：なし,-n
   //p46

    char current_path[MAX_PATH];
    char file[MAX_PATH];
    char c[1];
    char ch;    // 読み込んだテキストを格納
    FILE *fp;
    fpos_t pos;
    int i, n_line, count;
    char str_line[MAX_CHAR];
    long out_line;

    if(getcwd(current_path, MAX_PATH) == NULL){
        printf("Couldn't get the current directory.\n");
        return 1;
    }

    if((strlen(current_path)+1) > MAX_PATH){//+1は/のため, strcpyとstrcatの両方のエラー検出
        printf("The name of the path is too long\n");
        return 1;
    }
    strcat(current_path, "/");
    printf("%s\n", current_path);

    if(strcmp(args[1], "-n") == 0){
        for(i=2; args[i] !=NULL ; i++){
            n_line = 1;
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);

            // ファイルオープン 
            fp = fopen(file, "r");
            //ファイルが適切に読み込まれているかのを確認する
            if( fp == NULL ) {
                printf("Couldn't open  %s\n", args[i]);
                return 1;
            }
            
            //テキストの読み込み&出力 
            printf("▼---[%s]------------------------------------------------------▼\n", args[i]);
            while(fgets(str_line, MAX_CHAR, fp) != NULL) {
                printf("%4d:",n_line);
                n_line++;
                printf("%s", str_line);
            }
            printf("\n---------------------------------------------\n");
            fclose(fp);

        }
    }else if(strcmp(args[1], "-b") == 0){
        for(i=2; args[i] !=NULL ; i++){
            n_line = 1;
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);

            // ファイルオープン 
            fp = fopen(file, "r");
            //ファイルが適切に読み込まれているかのを確認する
            if( fp == NULL ) {
                printf("Couldn't open  %s\n", args[i]);
                return 1;
            }
            
            //テキストの読み込み&出力 
            printf("▼---[%s]------------------------------------------------------▼\n", args[i]);
            while(fgets(str_line, MAX_CHAR, fp) != NULL) {
                if(*str_line == '\n'){
                    continue;
                    printf("skip\n");
                }
                printf("%4d:",n_line);
                n_line++;
                printf("%s", str_line);
            }
            printf("\n---------------------------------------------\n");
            fclose(fp);
        }
    }else if(strcmp(args[1], "-l") == 0){
        if(out_line = strtol(args[2], NULL, 10) == 0){
            printf("The arguments are invaild.\n");
            return 1;
        }
        for(i=3; args[i] !=NULL ; i++){     //args[2] は数字
            n_line = 1;
            strcpy(file,current_path);
            count = 0;

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);

            // ファイルオープン 
            fp = fopen(file, "r");
            //ファイルが適切に読み込まれているかのを確認する
            if( fp == NULL ) {
                printf("Couldn't open  %s\n", args[i]);
                return 1;
            }
            
            //テキストの読み込み&出力 
            printf("▼---[%s]------------------------------------------------------▼\n", args[i]);
            while(fgets(str_line, MAX_CHAR, fp) != NULL) {
                if(count == out_line){
                    scanf("%1s%*[^\n]", c);
                    count = 0;
                }
                printf("%4d:",n_line);
                n_line++;
                printf("%s", str_line);
                count++;
            }
            printf("\n---------------------------------------------\n");
            fclose(fp);
        }
    }else{
        for(i=1; args[i] !=NULL ; i++){
            strcpy(file,current_path);

            if((strlen(args[i])+strlen(file)) > MAX_PATH){
                printf("The name of command is too long.\n");
                return 1;
            }
            strcat(file,args[i]);
            printf("%s\n", file);

            // ファイルオープン 
            fp = fopen(file, "r");
            //ファイルが適切に読み込まれているかのを確認する
            if( fp == NULL ) {
                printf("Couldn't open  %s\n", args[i]);
                return 1;
            }
            
            //テキストの読み込み&出力 
            printf("▼---[%s]------------------------------------------------------▼\n", args[i]);
            while(fgets(str_line, MAX_CHAR, fp) != NULL) {
                printf("%s", str_line);
            }
            printf("\n---------------------------------------------\n");
    //***本当はファイルの末尾が改行文字かどうかで"\n"の有無を判断できればよい
    /*      fgetpos( fp, &pos );
            if(strncmp((&ch-1), "\n", 1) != 0){
                printf("*(&ch-1):%c",*(&ch-1));
                printf("\n");
            }
    */
            fclose(fp);

        }
    }
  
    return 0;
}

int find(char* args[]){
    //p57,find ~ -name myfile:~ファイル(ホームディレクトリ)を起点に，myfileを名前検索し，それを画面表示する．
    //find /bin -name "ch*":ワイルドカードでファイルを検索
    return 0;
}

//current file作成の関数作ってもいいかも****

/*-- END OF FILE -----------------------------------------------------------*/