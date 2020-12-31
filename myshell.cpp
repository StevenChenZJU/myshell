#include "myshell.h"
using namespace std;
/**
 * 返回当前myshell 的命令提示符
 * 包括颜色信息 
 */
string CommandLinePrompt(){
    // PS1 是shell 的变量而不是环境变量
    char pwd[PATH_MAX];
    string PS1;
    if(getcwd(pwd, PATH_MAX))
        // myshell用蓝色显示, pwd用绿色
        PS1 = "\033[01;32mmyshell\033[0m:\033[01;34m"
                                    + GetRelativePath(string(pwd))
                                    + "\033[0m$ ";
    return PS1;
}
/**
 * 处理重定向
 * 如果有重定向那么to_close是打开的文件的指针
 * 否则to_close为null
 * 返回处理过的parsed数组
 * */
vector<string> ProcessRedirect(vector<string> parsed, FILE* & to_close){
    to_close = nullptr;
    // 先搜索>>
    auto found_iter_double = find(parsed.begin(), parsed.end(), string(">>"));
    if(found_iter_double != parsed.end()){
        //有重定向>>参数
        cout<<"Redirect >>"<<endl;
        if(found_iter_double + 1 == parsed.end()) //如果最后一个, 出错
            throw invalid_argument(">> need a target");
        
        string target = *(found_iter_double + 1);
        // 以结尾追加的方式打开文件
        FILE* fd = fopen(target.c_str(), "a");
        if(fd == NULL)
            throw runtime_error("Can not open file " + target);
        dup2(fileno(fd), STDOUT_FILENO); // 代替标准输入
        to_close = fd;
        // 删除重定向提示
        parsed.erase(found_iter_double, found_iter_double+2);
    }
    else{
        auto found_iter_single = find(parsed.begin(), parsed.end(), string(">"));
        if(found_iter_single != parsed.end()){
            //有重定向参数>
            cout<<"Redirect >"<<endl;
            if(found_iter_double + 1 == parsed.end()) //如果最后一个, 出错
                throw invalid_argument("> need a target");
            
            string target = *(found_iter_single + 1);
            // 以打开新文件的方式打开
            FILE* fd = fopen(target.c_str(), "w");
            if(fd == NULL)
                throw runtime_error("Can not open file " + target);
            dup2(fileno(fd), STDOUT_FILENO);
            to_close = fd;
            parsed.erase(found_iter_single, found_iter_single+2);
        }
    }
    return parsed;
}
/**
 * 执行有管道操作的命令
 * 支持任意个管道
 * 通过迭代的方式提前建立管道
 * */
void ExecPipe(vector<string>& procs){
    int proc_count = procs.size();
    string proc;
    // 有管道的命令一定已经在一个新进程下了
    pid_t pgid = getpgid(0); // 获取当前进程的pgid
    vector<string> parsed;
    // 管道数量等于proc数量-1
    vector<pair<int, int>> pipes(proc_count-1, {0, 0});
    int mypipe[2]; // 用于调用pipe函数
    for(int i = 0; i < proc_count-1; i++){
        pipe(mypipe); // 建立管道
        if(pipe(mypipe) == -1){
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
        // 暂存
        pipes[i].first = mypipe[0];
        pipes[i].second = mypipe[1];
    }
    // 每一个proc单独创建一个进程
    // 用pipe实现进程间的通信
    for(int i = 0; i < procs.size(); i++){
        proc = procs[i];
        Trim(proc);
        parsed = Parse(proc);
        // 分解成一个个字符串,如果非空继续执行
        if(!parsed.empty()){
            int pid = fork();
            if(pid < (pid_t)0){
                perror("error in fork()");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0){
                // 子进程
                setpgid(0, pgid); // 和父进程同一组
                
                if(procs.size() > 1){
                    // 利用创建好的pipe
                    if(i == 0){
                        close(pipes[i].first); // 要提前关闭管道另一端
                        dup2(pipes[i].second, STDOUT_FILENO);
                        close(pipes[i].second); // 必须要关闭
                    }
                    else if(i == procs.size()-1){
                        close(pipes[i-1].second);
                        dup2(pipes[i-1].first, STDIN_FILENO);
                        close(pipes[i-1].first); // 必须要关闭
                    }
                    else{
                        close(pipes[i-1].second);
                        close(pipes[i].first);
                        dup2(pipes[i-1].first, STDIN_FILENO);
                        dup2(pipes[i].second, STDOUT_FILENO);
                        close(pipes[i].second); // 必须要关闭
                        close(pipes[i-1].first); // 必须要关闭
                    }
                }
                FILE* fd_to_close = NULL;
                // 处理是否有重定向
                vector<string> processed = ProcessRedirect(parsed, fd_to_close);
                int rtcode = Execute(processed);
                LAST_RETURN = rtcode;

                if(fd_to_close != NULL){
                    // 恢复重定向
                    fflush(fd_to_close);
                    fclose(fd_to_close);
                    freopen("/dev/tty", "w", stdout);
                }
                    
                exit(rtcode);
            }
            else{
                // 父进程
                setpgid(pid, pgid);
                waitpid(pid, NULL, 0);

            }
                
        }
        else{
            cout<<"Can not be empty between |"<<endl;
            throw runtime_error("Can not be empty between |");
        }
    }
    // 关闭所有管道
    for(int i = 0; i < proc_count-1; i++){
        close(pipes[i].first);
        close(pipes[i].second);
    }
}
void ExecString(string input){
    vector<string> procs; 
    int pgid = getpid();
    setpgid(0, 0);
    try{
        if(input.find('|') != string::npos){
            procs = Parse(input, '|');
            ExecPipe(procs);
        }
        else{
            // 没有pipe
            vector<string> parsed = Parse(input);
            FILE* fd_to_close = NULL;
            vector<string> processed = ProcessRedirect(parsed, fd_to_close);
            int rtcode = Execute(processed);
            LAST_RETURN = rtcode;
            if(fd_to_close != NULL){
                fflush(fd_to_close);
                fclose(fd_to_close);
                freopen("/dev/tty", "w", stdout);
            }
        }
        
    }
    catch( exception e ){
        cout<<"Can not Parse "<<input<<endl;
    }
}
/**
 * 判断是否需要新开一个子进程执行命令
 * 如果不是后台执行,并且没有管道操作就不需要
 * */
void ForkExec(string input){
    ProcessComment(input);
    Trim(input);
    // 如果没有pipe并且不是后台执行就不fork
    if(input.find('|') == string::npos 
        && input.find('&') == string::npos){
        ExecString(input);
        return;
    }
    int pid = fork();
    if(pid < (pid_t)0){
        perror("Error fork");
        ExitShell(EXIT_FAILURE);
    }
    if(pid == 0){
        setpgid(pid, 0);
        setenv("PARENT", getenv("SHELL"), 1); // 设置环境变量
        SigDefault();
        // 是否后台运行
        if(!input.empty() && *input.rbegin() == '&'){
            input = input.substr(0, input.length()-1);  
        }
        ExecString(input);
        exit(0);
    }
    else{
        setpgid(pid, pid);
        // 是否后台运行,如果是发送信号
        // 如果是内部指令,此时很可能已经结束了进程
        if(!input.empty() && *input.rbegin() == '&'){
            cout<<"Run in background: "<<input<<endl;
            // killpg(pid, SIGTSTP);   
        }
        else{
            // 设置为前台运行
            tcsetpgrp(STDOUT_FILENO, pid);
            tcsetpgrp(STDIN_FILENO, pid);
        }
        int status;
        int rt = waitpid(pid, &status, WCONTINUED|WUNTRACED);
        // 返回后回复tty的前台权限
        tcsetpgrp(STDOUT_FILENO, getpgrp());
        tcsetpgrp(STDIN_FILENO, getpgrp());
        if (rt == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        // 返回后查看发生了什么
        if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            // 如果是被暂停了,放入后台进程中
            printf("stopped by signal %d\n", WSTOPSIG(status));
            // 放在后台进程中
            JOBS.push_back(Job(pid, input));
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    }
    
}
// 循环器
RTCODE Looper(){
    while(true){
        cout<<CommandLinePrompt()<<flush;
        string input;
        getline(cin, input);
        ForkExec(input);
    }
}
// 脚本文件执行
RTCODE ScriptExecutor(int argc,char *argv[]){
    try{
        ifstream fin;
        fin.open(argv[1]);
        string input;
        while(getline(fin, input)){
            ForkExec(input);
        }
        return RT_SUCCESS;
    }
    catch( exception e ){
        cout<<"Unexpected Error!"<<endl;
        return RT_ERROR;
    }
}
// 处理输入的变量
void ParseVariable(int argc, char* argv[]){
    for(int i = 0; i < argc; i++){
        ARGS.push_back(argv[i]);
    }
}
int main(int argc,char *argv[]){
    // 建立新会话
    setsid();
    // 设置信号屏蔽
    SigIgnore();
    ROOT_PGID=getpgid(0); // 设置shell的进程组为全局变量
    // 设置SHELL环境变量为myshell
    if(setenv("SHELL", GetAbsolutePath(argv[0]).c_str(), 1))
        cout<<"Set SHELL error!"<<endl;
    RTCODE rtcode = RT_SUCCESS;
    ParseVariable(argc, argv);
    if (argc == 1) {
        rtcode = Looper();
    }
    else {
        rtcode = ScriptExecutor(argc, argv);
    }
    return rtcode;
}