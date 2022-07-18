#include <stdio.h>      //perror
#include <unistd.h>     //getcwd, chdir
#include <stdlib.h>     //getenv

#define PATH_SIZE 512

//拡張機能なし。

int main(int argc, char *args[]){
    printf("argc:%d, args[0]:%s\n", argc, args[0]);
    char path_name[PATH_SIZE];
    char *HOME_path;
    getcwd(path_name, PATH_SIZE);       //getcwd使用の注意，path_nameがPATH＿SIZEを超えた場合のエラー処理
    printf("current dir : %s\n", path_name);

    if(args[1] == NULL){        //ディレクトリ名が指定されなかった場合, path_nameに環境変数 HOME に指定されたディレクトリへのパスを代入する
        HOME_path = getenv("HOME");

        printf("HOME：%s\n", HOME_path);
        args[1] = HOME_path;

        if (HOME_path == NULL) {
            printf("Couldn't find HOME(environment variable)\n");
            return 1;
            //異常終了
        }

    }

    if(chdir(args[1]) != 0){        
        //正常終了しなかった場合，作業ディレクトリを変えずchdirはー１を返す．
        //***：エラーコードに応じてメッセージを出力させる。https://www.ibm.com/docs/ja/zos/2.3.0?topic=functions-chdir-change-working-directory
        //相対パスをchdirで実装できるか：chdir関数には、絶対パス・相対パス・ルート相対パスすべて指定できるため、自由にカレントディレクトリを変更できます。
        //https://nidea.jp/lang-c/2022/01/move-current-directory/
        printf("Couldn't move to the path\n");
        perror("");
    }else{
        //正常終了した場合，chdirは０を返す 
        getcwd(path_name, PATH_SIZE);       //getcwd使用の注意，path_nameがPATH＿SIZEを超えた場合のエラー処理
        printf("current dir : %s\n", path_name);
    }
    return 0;
    
    //***問題:カレントディレクトリの変更が反映されない
}