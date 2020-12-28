### Myshell设计文档

***

#### 设计思想

本设计的核心理念是实现一个类似于Bourne Again Shell(Bash)的拥有【内部指令】，【外部程序/指令调用】，【IO重定向（仅支持内部指令的输出重定向）】，【文件作为命令行输入】，【管道操作】以及【**作业控制**】的Linux交互终端。

本设计在进程调度上，尤其是在作业控制的前后台切换上，尽可能做到与bash类似，比如bash中内部指令（除了help）也是往往会在放到后台之前就终止；比如在bash中在管道操作中使用cd没法切换工作目录（因为在一个新的子进程下）。

#### 功能模块

myshell源代码分为myshell.cpp/h, command.cpp/h, manual.cpp/h, util.cpp/h四个部分

其中myshell.cpp/h是主函数部分，负责顶层的调用，包括对管道和重定向的处理

command.cpp/h负责内外部指令的具体执行，管理所有内部指令

manual.cpp中存放着用户手册

util.cpp/h管理全局变量和辅助函数

#### 功能实现

具体功能参见用户手册

myshell使用C++进行编写，其中使用了Linux C的系统API进行shell功能的编写，也是用了现代化的C++11中的特性，比如lambda函数对代码的可维护性进行提升。

#### 数据结构

自主定义的数据结构是后台进程信息的结构体Job

```c++
struct Job
{
    static int max_id;
    int id; // id号,逐步递增
    pid_t pgid; // 所在进程组编号
    JobStatus status; // 状态
    std::string command; // 指令
//...
}
```

负责存访后台进程的id， 进程组编号，状态，以及指令信息