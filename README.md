# minishell
 - C言語で実装した、UNIX風の簡易シェルプログラム  
 - 外部コマンドの実行に加え、ディレクトリ操作・ヒストリ機能・ワイルドカード展開など、複数の機能をサポート

## 主な機能

- 外部コマンドの実行（`ls`, `pwd` など）
- バックグラウンド実行（例：`sleep 10 &`）
- `exit` コマンドでシェル終了
- ディレクトリ管理機能：
  - `cd`, `pushd`, `popd`, `dirs`
- ヒストリ機能：
  - `history`, `!!`, `!string`, `!n`, `!-n`
- プロンプト変更：`prompt [文字列]`
- スクリプト機能：`./mysh < script.txt`
- ワイルドカード補完：`*`, `*string`, `string*`
- エイリアス機能：`alias`, `unalias`
- 自作コマンド追加（例：`pwd`, `rm`, `mkdir`, `cat`）


## 使い方

```bash
gcc minishell.c
