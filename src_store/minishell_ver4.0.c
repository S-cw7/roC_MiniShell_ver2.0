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

#define max_PATH 256

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
int cd(char *[]);


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
void execute_function(char* args[]){    //args[0]:コマンド名および関数名，args[1]以降：コマンドの引数
    printf("args[0]:%s\n", args[0]); 
    char *p_ls = "ls";
    char *p_cd = "cd";
    char *p_pushd = "pushd";
    printf("strcmp(args[0] , p_ls)==0:%d\n",strcmp(args[0] , p_ls)==0);
    if (strcmp(args[0] , p_ls)==0){
        //ls(args);
         printf("execute:ls\n");
         ls(args);
    }else if (strcmp(args[0] , p_cd)==0){
        //cd(args);
        printf("execute:cd\n");
         printf("mainTop1:%s\n", pTop.dir_path);
        cd(args);
        printf("mainTop2:%s\n", pTop.dir_path);
    }else if (strcmp(args[0] , p_pushd)==0){
        
        printf("execute:pushd\npushd1_function\n");
        printf("mainTop1:%s\n", pTop.dir_path);
        pushd(args); 
        printf("mainTop2:%s\n", pTop.dir_path);  
    }
    printf("execute_function_end\n");
}

/*---[pushdに必要な宣言など]----------------------------------------------------------------------*/

#define PATH_SIZE 512
typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;

node_tag pTop;   //*Topにしたければ実態を代入する必要あり
/*-----------------------------------------------------------------------------------------------*/


int pushd(char *args[]){
    printf("Top1:%s\n", pTop.dir_path);
    char pushd_path[PATH_SIZE];
    getcwd(pushd_path, PATH_SIZE);
    printf("Top2:%s\n", pTop.dir_path);
    printf("current dir : %s\n", pushd_path);


    //もし最初に追加する場合
    if(pTop.dir_path == NULL){
        pTop.dir_path = pushd_path;
        pTop.Next = NULL;
        printf("1\n");
    printf("Top:%s\n", pTop.dir_path);
    
    }else{
        //二回目以降に追加する場合
        printf("Top:%s\n", pTop.dir_path);
        node_tag *p;
        int i=0;
        for(p=pTop.Next; p != NULL ; p = p->Next){
            printf("i:%d\n",i);
            i++;
        }
    printf("Top3:%s\n", pTop.dir_path);
        printf("2\n");
        printf("p = NULL:%d\n",(p!= NULL));
        node_tag *NewNode;
        NewNode = (node_tag*)malloc(sizeof(node_tag));
printf("Top4:%s\n", pTop.dir_path);
        printf("3\n");
        NewNode->dir_path = pushd_path;
        NewNode->Next = NULL;
        printf("4\n");
        //p->Next = NewNode;
    printf("Top:%s, Next:%s\n", pTop.dir_path, NewNode->dir_path);

    }
 
    return 0;
}


//拡張機能なし。隠しファイルも自動的に表示(ls -laと同じ)

int ls(char *args[]){
    printf("args[0]:%s\n", args[0]);

    DIR *dir;
    struct dirent *dir_info;

    char current_path[PATH_SIZE];
    getcwd(current_path, PATH_SIZE);
    printf("current dir : %s\n", current_path);
    char *ls_path =  current_path;

    if (args[1] != NULL){
        ls_path = args[1];
    }
    printf("s_path:%s\n", ls_path);
    dir = opendir(ls_path);

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

//拡張機能なし。

int cd(char *args[]){
    printf("Top1:%s\n", pTop.dir_path); 
    printf("args[0]:%s\n", args[0]);
    char cd_path[PATH_SIZE];
    char *HOME_path;
     printf("Top2:%s\n", pTop.dir_path); 
    getcwd(cd_path, PATH_SIZE);       //getcwd使用の注意，path_nameがPATH＿SIZEを超えた場合のエラー処理
    printf("current dir : %s\n", cd_path);
     printf("Top3:%s\n", pTop.dir_path); 
    if(args[1] == NULL){        //ディレクトリ名が指定されなかった場合, path_nameに環境変数 HOME に指定されたディレクトリへのパスを代入する
        HOME_path = getenv("HOME");

        printf("HOME：%s\n", HOME_path);
        args[1] = HOME_path;
     printf("Top4:%s\n", pTop.dir_path); 
        if (HOME_path == NULL) {
            printf("Couldn't find HOME(environment variable)\n");
            return 1;
            //異常終了
        }
     printf("Top4:%s\n", pTop.dir_path); 
    }

    if(chdir(args[1]) != 0){  
     printf("Top5:%s\n", pTop.dir_path);       
        //正常終了しなかった場合，作業ディレクトリを変えずchdirはー１を返す．
        //***：エラーコードに応じてメッセージを出力させる。https://www.ibm.com/docs/ja/zos/2.3.0?topic=functions-chdir-change-working-directory
        //相対パスをchdirで実装できるか：chdir関数には、絶対パス・相対パス・ルート相対パスすべて指定できるため、自由にカレントディレクトリを変更できます。
        //https://nidea.jp/lang-c/2022/01/move-current-directory/
        printf("Couldn't move to the path\n");
        perror("");
    }else{
        printf("current dir : %s\n", cd_path);
         printf("Top5:%s\n", pTop.dir_path); 
        //正常終了した場合，chdirは０を返す 
        //getcwd(cd_path, PATH_SIZE);       //getcwd使用の注意，path_nameがPATH＿SIZEを超えた場合のエラー処理
         printf("Top6:%s\n", pTop.dir_path); 
        printf("current dir : %s\n", cd_path);
    }
     printf("Top7:%s\n", pTop.dir_path); 
    return 0;
    
    //***問題:カレントディレクトリの変更が反映されない
}

/*void child(char *argv[MAXARGNUM]) {
  execvp(argv[0], argv);
}
*/
/*-- END OF FILE -----------------------------------------------------------*/
