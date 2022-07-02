/*----------------------------------------------------------------------------
 *  簡易版シェル
 *--------------------------------------------------------------------------*/

/*メモ--------------------------------------------------------------------------
/mnt/c/Users/mclif/proC/2
\Users\mclif\proC\2
同一ファイルで関数としてコマンドを実行したほうが良いのか，別ファイルを作成して実行させればよいのか
最後にファイルに分ける
--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 *  インクルードファイル
 --------------------------------------------------------------------------*/
#include <err.h>
//#include <direct.h> // _getcwd
#include <dirent.h>     //opendir, readdir, closedir
#include <stdio.h>      //perror
#include <stdlib.h>     //malloc, //getenv
#include <sys/types.h>
#include <unistd.h>     //getcwd, chdir
#include <sys/wait.h>
#include<string.h> //strcmp

//#include "MyDefShell.h"


/*
 *  定数の定義
 */

#define BUFLEN    1024     /* コマンド用のバッファの大きさ */
#define MAXARGNUM  256     /* 最大の引数の数 */

#define MAX_PATH 256

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
//int ls(char *[]);
int cd(char *[]);
void init_DirStack();
void dirs();
int popd();


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
    for(;;) {

        /*
         *  プロンプトを表示する
         */

        printf("> ");

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

        command_status = parse(command_buffer, args);
        printf("status:%d\n", command_status);


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
        printf("arg[%d] : %c\n",arg_index, *args[i]); //firefox & の時はsegment error, cd a.out などはaが出力される
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

node_tag pTop;   //*Topにしたければ実態を代入する必要あり
node_tag *pTail;   //*Topにしたければ実態を代入する必要あり


/*-----------------------------------------------------------------------------------------------*/

void execute_function(char* args[]){    //args[0]:コマンド名および関数名，args[1]以降：コマンドの引数
    printf("args[0]:%s\n", args[0]); 
    char *p_ls = "ls";
    char *p_cd = "cd";
    char *p_pushd = "pushd";
    char *p_dirs = "dirs";
    char *p_popd = "popd";


    printf("strcmp(args[0] , p_ls)==0:%d\n",strcmp(args[0] , p_ls)==0);
    if (strcmp(args[0] , p_ls)==0){
        //ls(args);
         printf("execute:ls\n");
        // ls(args);
    }else if (strcmp(args[0] , p_cd)==0){
        printf("execute:cd\n");
         printf("mainTop1:%s\n", pTop.dir_path);
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
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }  
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
    }
    printf("execute_function_end\n");
}




//拡張機能なし。隠しファイルも自動的に表示(ls -laと同じ)


//拡張機能なし。
//解放する必要あり(memset, getcwd, chdir)


int cd(char *args[]){
    printf("Top:%s\n", pTop.dir_path);
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
    printf("Top:%s\n", pTop.dir_path);
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
    printf("Top:%s\n", pTop.dir_path);

    //もし最初に追加する場合
    /*if(pTop.dir_path == NULL){
        pTop.dir_path = pushd_path;
        pTop.Next = NULL;
        printf("1\n");
        printf("Top:%s\n", pTop.dir_path);
    
    }else{
    */
        node_tag *p;
        int i=0;
        for(p=&pTop; p->Next != NULL ; p = p->Next){
            printf("i[%d]=",i);
            printf("p->dir_path:%s\n", p->dir_path);
            i++;
        }
        printf("1\n");
        node_tag *NewNode;
        NewNode = (node_tag*)malloc(sizeof(node_tag));   
        NewNode->dir_path = (char*)malloc(sizeof(char)*(strlen(pushd_path) + 1));
        printf("2\n");
        
        if( NewNode == NULL ){
            fputs( "メモリ割り当てに失敗しました。", stderr );
            return 1;
        } 
        printf("3\n");
    printf("NewNode->dir_path:%s,pushd_path:%s\n",NewNode->dir_path, pushd_path);
    //NewNode->dir_path
        strcpy(NewNode->dir_path, pushd_path); // 文字列はコピーする
        printf("4\n");

        //NewNode->dir_path = pushd_path;
        NewNode->Prev = p;
        NewNode->Next = NULL;
        p->Next = NewNode;
        pTail = NewNode;  
        printf("Top:%s, Next:%s\n", pTop.dir_path, NewNode->dir_path);
    
    return 0;
}
//dirsコマンド
//案１：単方向リストのまま，一回値を取り出して，逆に表示する
//案２：双方向リストにして、後ろからも終えるようにする
//案２を採用。dirsコマンドだけでなく，popdコマンドでも末尾を使用するため．また，前から辿って後ろから再度表示させる実行時間と，node_tag->Prevに必要なデータ容量を比較してどちらが良いか。

void dirs(){
    node_tag *p;
    int i=0;
    p=pTail;
    while(p->Prev != NULL){
        printf("i[%d]=",i);
        printf("p->dir_path:%s\n", p->dir_path);
        i++;
        p = p->Prev;
    } 
}

int popd(){
    char cd_path[MAX_PATH];

    if(chdir(pTail->dir_path) != 0){ 
        printf("Couldn't move to the path\n");
        perror("");
        return 1;
    }
    
    return 0;
}

/*void child(char *argv[MAXARGNUM]) {
  execvp(argv[0], argv);
}
*/
/*-- END OF FILE -----------------------------------------------------------*/
