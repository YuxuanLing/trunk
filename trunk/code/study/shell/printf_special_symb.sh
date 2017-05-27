#!/bin/bash
#
## 变量说明: 
## $$ 
## Shell本身的PID（ProcessID） 
## $! 
## Shell最后运行的后台Process的PID 
## $? 
## 最后运行的命令的结束代码（返回值） 
## $- 
## 使用Set命令设定的Flag一览 
## $* 
## 所有参数列表。如"$*"用「"」括起来的情况、以"$1 $2 … $n"的形式输出所有参数。 
## $@ 
## 所有参数列表。如"$@"用「"」括起来的情况、以"$1" "$2" … "$n" 的形式输出所有参数。 
## $# 
## 添加到Shell的参数个数 
## $0 
## Shell本身的文件名 
## $1～$n 
## 添加到Shell的各参数值。$1是第1参数、$2是第2参数…。
## ${name:-default} 使用一个默认值（一般是空值）来代替那些空的或者没有赋值的变量name；
## ${name:=default}使用指定值来代替空的或者没有赋值的变量name；
## ${name:?message}如果变量为空或者未赋值，那么就会显示出错误信息并中止脚本的执行同时返回退出码1。
## ${#name} 给出name的长度
## ${name%word} 从name的尾部开始删除与word匹配的最小部分，然后返回剩余部分
## ${name%%word} 从name的尾部开始删除与word匹配的最长部分，然后返回剩余部分
## ${name#word} 从name的头部开始删除与word匹配的最小部分，然后返回剩余部分
## ${name##word} 从name的头部开始删除与word匹配的最长部分，然后返回剩余部分

cat <<EOF
Usage: ./printf_special_symb  qq 0.12345
EOF
printf "The complete list \$$ is %s\n" "$$"
printf "The complete list \$! is %s\n" "$!"
printf "The complete list \$? is %s\n" "$?"
printf "The complete list \$* is %s\n" "$*"
printf "The complete list \$@ is %s\n" "$@"
printf "The complete list \$# is %s\n" "$#"
printf "The complete list \$0 is %s\n" "$0"
printf "The complete list \$1 is %s\n" "$1"
printf "The complete list \$2 is %s\n" "$2"