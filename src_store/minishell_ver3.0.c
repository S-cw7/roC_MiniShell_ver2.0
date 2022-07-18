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

#include <stdio.h>
#include <stdlib.h>     //malloc
#include <sys/types.h>
#include <unistd.h>     //getcwd
#include <sys/wait.h>
#include <string.h>

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
int pushd1(char *[]);


/*---[pushd1に必要な宣言など]------------------------------------------------*/

#define PATH_SIZE 512
typedef struct node{
    char*           dir_path;
    struct node*    Next;
}node_tag;

node_tag *pTop;   //*Topにしたければ実態を代入する必要あり
/*---------------------------------------------------*/

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

    /*
     *  子プロセスを生成する
     *
     *  生成できたかを確認し、失敗ならばプログラムを終了する
     */

    pid = fork();       
        //戻り値は分岐したプロセスが親プロセスの場合は、分岐した子プロセスのプロセスIDになります。
        //子プロセスの場合は、0になります。forkが失敗した場合は-1
    printf("fork PID:%d\n--------------------------\n", pid);
    if(pid == -1){
        perror("Counld not fork");
        exit(1);
    /*
     *  子プロセスの場合には引数として与えられたものを実行する
     *
     *  引数の配列は以下を仮定している
     *  ・第１引数には実行されるプログラムを示す文字列が格納されている
     *  ・引数の配列はヌルポインタで終了している
     */

    }else if(pid == 0){
        printf("Child Process: %d\n", pid);
        printf("Child PID by getppid() = %d\n", getpid());
        printf("Parent PID by getppid() = %d\n", getppid());
        //child(args);
    /*
     *  コマンドの状態がバックグラウンドなら関数を抜ける
     */

        if(command_status == 1){
            printf("Child Process: %d, background\n", pid);
            return;
        }


    /*
     *  ここにくるのはコマンドの状態がフォアグラウンドの場合
     *
     *  親プロセスの場合に子プロセスの終了を待つ
     */

    

        printf("children process end1\n");
        
        
        //char path[max_PATH] = "/mnt/c/Users/mclif/proC/2/";  
        char path[max_PATH] = "./";  
            //***カレントディレのパス．replitでは"/home/runner/report2-SQMQ/"となる．
            //本来はexeclp("ls", "-a", "-l", NULL); でファイル名を渡せばよいが，今回は外部のファイルかカレントディレクトリのファイルか分かりにくいので明示的にパスを指定する．
            //"/mnt/c/Users/mclif/proC/2/"としてもいいが，""./ls"だけでも十分で汎用性が上がるので，こちらを選択。詳しくはhttps://ja.manpages.org/execvp/3
        strcat(path, args[0]);
        printf("path:%s\n", path);
        int res;
        execute_function(args);
        printf("children process end2, res:%d\n", res);

        printf("Parent Process: Child PID = %d, Parent PID:%d\n", pid, getpid());
        //ret = wait(&status);
        printf("Parent Process end\n");
        if (ret < 0) {
            perror("wait");
            exit(2);  //***
        }
        //printf("Top:%s\n", pTop->dir_path);
        _exit(0);

    /*memo
//char *argv_ts[] = {"ls", "-a", "-l", NULL};
//execvp(argv_ts[0], argv_ts);
//res = execv(argv_ts[0], argv_ts);
//execvp("/ls", args);
//execl("/bin/ls", "", NULL);
//_exit(0);       //_exit()は成功の場合は0, 
    */
    }
    //wait()がエラーになった時の処理？***https://hiroyukichishiro.com/process-in-c-language/#forkwait

    return;
}

void execute_function(char* args[]){    //args[0]:コマンド名および関数名，args[1]以降：コマンドの引数
    printf("args[0]:%s\n", args[0]);
    pushd1(args); 
    if (args[0] == "ls"){
        //ls(args);
    }else if (args[0] == "cd"){
        //cd(args);
    }else if (args[0] == "pushd1"){
        printf("pushd1_function\n");
        pushd1(args);   
    }
    printf("execute_function_end\n");
}

int pushd1(char *args[]){
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

/*void child(char *argv[MAXARGNUM]) {
  execvp(argv[0], argv);
}
*/
/*-- END OF FILE -----------------------------------------------------------*/
