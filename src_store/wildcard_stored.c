char *wildcard(char buffer[]){
    DIR *dir;
    struct dirent *dir_elements;
    char current_path[MAX_PATH];
    char result[MAX_COMMAND];
    char *p_result;
    char *p;
    int j=0;

    if(strstr(buffer, "*") == NULL){
        return NULL;
    }
    
    getcwd(current_path, MAX_PATH);
    printf("current_path:\"%s\n\"", current_path);

    dir=opendir(current_path);
    if (dir == NULL) {
        printf("Couldn't open the dir\n");//****
        return 0;
    }

    //文字列を区切り文字で分解する
    p = strtok(buffer, "*\n");        //"*"は1つしか含まれていないものとする
    printf("j[%d]:%s\n",j, p);
    strcpy(result, p);

    //string*
    //*string
    //*のみ
    //if

    while((dir_elements = readdir(dir)) != NULL){
        printf("%s\n", dir_elements->d_name);
        strcat(result, " ");
        strcat(result, dir_elements->d_name);
    }
    printf("result:%s\n", result);


    p = strtok(NULL,"*");
    if(p!=NULL){
        strcat(result, p);
    }
    printf("result:%s\n", result);
    closedir(dir);
    p_result = &result[0];
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