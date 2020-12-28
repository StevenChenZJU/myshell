# 功能描述
使用任何一种程序设计语言实现一个shell 程序的基本功能。
shell 或者命令行解释器是操作系统中最基本的用户接口。写一个简单的shell 程序——myshell，它具有以下属性：
(一)	 这个shell 程序必须支持以下内部命令：bg✅、fg✅、jobs✅、help ✅、test✅ 、unset✅ 、set✅ 、umask✅、cd✅ 、clr✅、dir✅、echo ✅、exec✅ 、exit✅ 、environ✅、pwd✅ 、quit✅、shift✅ 、time ✅。部分命令解释如下：
1)✅	cd <directory>  ——把当前默认目录改变为<directory>。如果没有<directory>参数，则显示当前目录。如该目录不存在，会出现合适的错误信息。这个命令也可以改变PWD 环境变量。
2)✅	pwd ——显示当前目录。
3)✅	time ——显示当前时间
4)✅	clr  ——清屏。
5)✅	dir <directory>  ——列出目录<directory>的内容。
6)✅	environ  ——列出所有的环境变量。
7)✅	echo <comment>  ——在屏幕上显示<comment>并换行（多个空格和制表符可能被缩减为一个空格）。
8)✅	help ——显示用户手册，并且使用more 命令过滤。
9)✅	quit  ——退出shell。
10)✅ shell 的环境变量应该包含shell=<pathname>/myshell，其中<pathname>/myshell 是可执行程序shell 的完整路径（不是你的目录下的路径，而是它执行程序的路径）。
(二)✅	 其他的命令行输入被解释为程序调用，shell 创建并执行这个程序，并作为自己的子进程。程序的执行的环境变量包含一下条目：
parent=<pathname>/myshell。
(三)✅	 shell 必须能够从文件中提取命令行输入，例如shell 使用以下命令行被调用：
myshell batchfile 
这个批处理文件应该包含一组命令集，当到达文件结尾时shell 退出。很明显，如果shell 被调用时没有使用参数，它会在屏幕上显示提示符请求用户输入。
(四)✅	 shell 必须支持I/O 重定向，stdin 和stdout，或者其中之一，例如命令行为：
programname arg1 arg2 < inputfile > outputfile 
使用arg1 和arg2 执行程序programname，输入文件流被替换为inputfile，输出文件流被替换为outputfile。
stdout 重定向应该支持以下内部命令：dir、environ、echo、help。
使用输出重定向时，如果重定向字符是>，则创建输出文件，如果存在则覆盖之；如果重定向字符为>>，也会创建输出文件，如果存在则添加到文件尾。
(五)✅	 shell 必须支持后台程序执行。如果在命令行后添加&字符，在加载完程序后需要立刻返回命令行提示符。
(六)✅	 必须支持管道（“|”）操作。
(七)✅	 命令行提示符必须包含当前路径。

