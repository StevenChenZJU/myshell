#include "command.h"
using namespace std;
// 内部命令
/**
 * cd指令用于转换工作目录，当前工作目录会显示在命令提示符上
 * */
RTCODE command_cd(vector<string>& args){
    string dest; // 目标文件夹
    if(args.size() < 1){
        // 默认返回HOME
        dest = getenv("HOME");
    }
    else{
        // 获取绝对路径,以便于更改PWD
        dest = GetAbsolutePath(args[0]);
    }
    if(isDir(dest)){
        //需要同时改变cwd和PWD环境变量
        if(!chdir(dest.c_str()) && !setenv("PWD", dest.c_str(), 1)){
            cout<<"Sucess; $PWD = "<<getenv("PWD")<<endl;
            return RT_SUCCESS;
        }
        else{
            cout<<"Fail at cd"<<endl;
            return RT_ERROR;
        }
    }
    else{
        cout<<dest+" is not a directory"<<endl;
        return RT_ERROR;
    } 
}
/**
 * clr指令用于清屏并将光标移到左上角
 * */
RTCODE command_clr(vector<string>& args){
    cout<<"\033[2J\33[0;0H"<<flush;
    return RT_SUCCESS;
}
/**
 * 用于输出当前工作目录
 * */
RTCODE command_pwd(vector<string>& args){
    char pwd[PATH_MAX];
    cout<<getcwd(pwd, PATH_MAX)<<endl;
    return RT_SUCCESS;
}
/**
 * 输出当前时间
 * */
RTCODE command_time(vector<string>& args){
    time_t time_stamp;
    time(&time_stamp);
    cout<<ctime(&time_stamp)<<flush;
    return RT_SUCCESS;
}
/**
 * 输出所有环境变量
 */
RTCODE command_environ(vector<string>& args){
    char** env = environ;
    vector<string> result;
    while(*env != NULL){
        result.push_back(*env);
        env++;
    }
    // 排序后输出
    sort(result.begin(), result.end());
    for(auto i: result){
        cout<<i<<endl;
    }
    return RT_SUCCESS;
}
/**
 * 列出指定文件夹中的所有文件
 * */
RTCODE command_dir(vector<string>& args){
    if(args.size() > 0 && isDir(args[0])){
        // 第一个参数是文件夹
        string name = args[0];
        DIR* dirp = opendir(name.c_str());
        struct dirent * dp;
        vector<string> result;
        // 读取文件夹信息
        while ((dp = readdir(dirp)) != NULL) {
            result.push_back(dp->d_name);
        }
        // 排序后输出
        sort(result.begin(), result.end());
        for(auto i : result){
            cout<<i<<"\t";
        }
        cout<<endl;
        closedir(dirp);
        return RT_SUCCESS;
    }
    else{
        cout<<"Usage: dir <directory>"<<endl;
        return RT_ERROR;
    }
}
/**
 * 退出myshell
 * */
RTCODE command_quit(vector<string>& args){
    cout<<"exit"<<endl;
    exit(0);
    return RT_ESCAPE;
}
/**
 * 回显字符串
 * 支持变量(变量已经在之前被替换了)
 * */
RTCODE command_echo(vector<string>& args){
    for(auto i: args)
        cout<<i<<" ";
    cout<<endl;
    return RT_SUCCESS;
}
/**
 * 移动argument参数
 * */
RTCODE command_shift(vector<string>& args){
    if(args.size() > 0){
        // 尝试移动,失败说明不是数字
        try{
            int shift_num = stoi(args[0]);
            ShiftN(shift_num);
            return RT_SUCCESS;
        } catch(exception e){
            cout<<args[0]<<" is not a integer"<<endl;
            return RT_ERROR;
        }
    }
    else{
        // 默认移动一位
        ShiftN(1);
        return RT_SUCCESS;
    }
}
// 执行命令, 并直接退出当前进程
RTCODE command_exec(vector<string>& args){
    pid_t pid = fork();
    if(pid < (pid_t)0){
        cout<<"error in fork()"<<endl;
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        int rtcode = Execute(args);
        exit(rtcode);
    }
    else{
        waitpid(pid, NULL, 0);
        exit(0);
    }
    // 直接退出Shell
}
/**
 * 显示或修改umask
 * */
RTCODE command_umask(vector<string>& args){
    if(args.size() < 1){
        // 如果没有参数
        cout<<setfill('0')<<setw(3)<<setbase(8)<<GetUmask()<<endl;
        return RT_SUCCESS;
    }
    else{
        try{
            // 查看是否是有效的三位八进制
            int new_mask = stoi(args[0], 0, 8);
            if(new_mask < 01000 && new_mask >= 0){
                int old_mask = umask(new_mask);
                cout<<"UMASK:"
                <<setfill('0')<<setw(3)<<setbase(8)<<old_mask
                <<"->"
                <<setfill('0')<<setw(3)<<setbase(8)<<new_mask
                <<endl;
                
            }
            else  
                throw invalid_argument("invalid");
            return RT_SUCCESS;
        } catch(exception e){
            cout<<args[0]<<" is not a legal argument"<<endl;
            return RT_ERROR;
        }
    }
}
/**
 * 显示所有变量,包括用户变量
 * */
RTCODE command_set(vector<string>& args){
    // 不支持任何参数
    // 输出所有shell变量和环境变量
    char** env = environ;
    vector<string> result;
    while(*env != NULL){
        result.push_back(*env);
        env++;
    }
    // 加入所有用户变量
    for(auto i: VAR){
        result.push_back(i.first+"="+i.second);
    }
    // 排序后输出
    sort(result.begin(), result.end());
    for(auto i: result){
        cout<<i<<endl;
    }
    return RT_SUCCESS;
}
/**
 * 清楚变量的定义
 * 包括环境变量的定义
 */
RTCODE command_unset(vector<string>& args){
    if(args.size() < 1){
        cout<<"Usage: unset <var name>"<<endl;
        return RT_ERROR;
    }
    else{
        if(VAR.find(args[0])!= VAR.end()){
            VAR.erase(args[0]);
        }
        unsetenv(args[0].c_str());
        return RT_SUCCESS;
    }
}
/**
 * 测试表达式
 */
RTCODE command_test(vector<string>& args){
    if(args.size() == 2){
        // 是否为空
        if(args[0] == string("-z")){
            int res = args[1].empty();
            cout<<res<<endl;
            return RT_SUCCESS;
        }
        // 是否是文件夹
        else if(args[0] == string("-d")){
            int res = isDir(GetAbsolutePath(args[1]));
            cout<<res<<endl;
            return RT_SUCCESS;
        }
        else{
            cout<<"Unknown "<<args[0]<<endl;
            return RT_ERROR;
        }
    }
    else if(args.size() == 3){
        //是否相等
        if(args[1] == string("-eq")){
            int res = (args[0] == args[2]);
            cout<<res<<endl;
            return RT_SUCCESS;
        }
        // 是否不等
        else if(args[1] == string("-ne")){
            int res = (args[0] != args[2]);
            cout<<res<<endl;
            return RT_SUCCESS;
        }
        else{
            cout<<"Unknown "<<args[1]<<endl;
            return RT_ERROR;
        }
    }
    else{
        cout<<"Wrong number of Arguments"<<endl;
        return RT_ERROR;
    }
}
/**
 * 输出用户手册并用more过滤
 */
RTCODE command_help(vector<string> args){
    stringstream ss(HELP);
    FILE *fpout;
    // 打开一个管道并新建进程
    if((fpout = popen(PAGER, "w")) == NULL){
        cout<<"popen error"<<endl;
        return RT_ERROR;
    }  
    else{
        string line;
        // 将用户手册逐行输出到more的管道
        while(getline(ss, line)){
            line = line + "\n";
            if(fputs(line.c_str(), fpout) == EOF){
                cout<<"fputs error to pipe"<<endl;
                pclose(fpout);
                return RT_ERROR;
            }
        }
        if(pclose(fpout) == -1){
            cout<<"pclose error"<<endl;
            return RT_ERROR;
        }
            
    }
    return RT_SUCCESS;
}
/**
 * 将后台进程号为args[0]的进程
 * 移动到前台作为前台进程
 */
RTCODE command_fg(vector<string> args){
    if(args.size() < 1){
        cout<<"Too few argument"<<endl;
        return EXIT_FAILURE;
    }
    else if(args.size() > 1){
        cout<<"Too many argument"<<endl;
        return EXIT_FAILURE;
    }
    else{
        if(isNumber(args[0])){
            int jobno = stoi(args[0]);
            for(auto it = JOBS.begin(); it != JOBS.end(); it++){
                Job& job = *it;
                if(job.id == jobno){
                    // 发送信号继续运行
                    killpg(job.pgid, SIGCONT);
                    // 设置前台运行
                    tcsetpgrp(STDIN_FILENO, job.pgid);
                    tcsetpgrp(STDOUT_FILENO, job.pgid);
                    waitid(P_PGID, job.pgid, NULL, WUNTRACED);
                    // 重置前台进程组
                    tcsetpgrp(STDIN_FILENO, getpgrp());
                    tcsetpgrp(STDOUT_FILENO, getpgrp());
                    JOBS.erase(it);
                    return EXIT_SUCCESS;
                }
            }
            cout<<"No Job "<<args[0]<<endl;
            return EXIT_FAILURE;
        }
        else{
            cout<<"invalid argument: "<<args[0]<<endl;
            return EXIT_FAILURE;
        }
    }
}
/**
 * 将后面跟着的命令放到后台
 */
RTCODE command_bg(vector<string> args){
    cout<<"Run in background: "<<args[0]<<endl;
    // 判断当前是否是新的子进程
    // 如果不是就新建一个子进程
    if(getpgid(0) == ROOT_PGID){
        int pid = fork();
        string input; // args连接成command
        for(auto i: args)
            input += i + " ";
        
        if(pid < (pid_t)0){
            perror("Error fork");
            ExitShell(EXIT_FAILURE);
        }
        if(pid == 0){
            setpgid(pid, 0);
            setenv("PARENT", getenv("SHELL"), 1); // 设置环境变量
            SigDefault();
            // 后台运行
            killpg(pid, SIGTSTP);   

            Execute(args);
            exit(0);
        }
        else{
            setpgid(pid, pid);
            int status;
            int rt = waitpid(pid, &status, WCONTINUED|WUNTRACED);
            if (rt == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            // 查看返回结果
            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
                // 放在后台进程中
                JOBS.push_back(Job(pid, input));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        }
    }
    else{
        killpg(0, SIGTSTP);
        Execute(args);
        return EXIT_SUCCESS;
    }
}
// 查看所有后台进程
RTCODE command_jobs(vector<string> args){
    for(auto job: JOBS){
        cout<<job.toString()<<endl;
    }
}
// 所有内部指令的映射
const static map<string, FUNCTION_TYPE> INNER_COMMAND = {
    {"cd",      [](vector<string>& s){return command_cd     (s);}},
    {"fg",      [](vector<string>& s){return command_fg     (s);}},
    {"bg",      [](vector<string>& s){return command_bg     (s);}},
    {"clr",     [](vector<string>& s){return command_clr    (s);}},
    {"pwd",     [](vector<string>& s){return command_pwd    (s);}},
    {"dir",     [](vector<string>& s){return command_dir    (s);}},
    {"set",     [](vector<string>& s){return command_set    (s);}},
    {"time",    [](vector<string>& s){return command_time   (s);}},
    {"exit",    [](vector<string>& s){return command_quit   (s);}},
    {"quit",    [](vector<string>& s){return command_quit   (s);}},
    {"echo",    [](vector<string>& s){return command_echo   (s);}},
    {"exec",    [](vector<string>& s){return command_exec   (s);}},
    {"test",    [](vector<string>& s){return command_test   (s);}},
    {"help",    [](vector<string>& s){return command_help   (s);}},
    {"jobs",    [](vector<string>& s){return command_jobs   (s);}},
    {"shift",   [](vector<string>& s){return command_shift  (s);}},
    {"umask",   [](vector<string>& s){return command_umask  (s);}},
    {"unset",   [](vector<string>& s){return command_unset  (s);}},
    {"environ", [](vector<string>& s){return command_environ(s);}}
};
// 指令相关函数
/**
 * 判断是否是内部指令
 */
bool isInnerCommand(string program){
    return INNER_COMMAND.find(program) != INNER_COMMAND.end();
}
/**
 * 从vector string 到char*[]
 */
static char** GetCharPtrArray(vector<string>& vs){
    char** res = new char * [vs.size()+1];
    for(int i = 0; i < vs.size(); i++){
        res[i] = new char [vs[i].length()+1];
        strcpy(res[i], vs[i].c_str());
    }
    res[vs.size()] = NULL;
    return res;
}
/**
 * 释放上面的函数产生的内存
 */
static int FreeCharPtrArray(char** cpa){
    char** temp = cpa;
    for(auto i = temp; *temp != NULL; temp++){
        delete [] (*temp);
    }
    delete [] cpa;
    return 0;
}
static char** cpa = NULL;

/**
 * 执行外部指令
 */
RTCODE ExecOuterCommand(vector<string>& parsed){
    cout<<"Execute outer command:"+parsed[0]<<endl;
    if(getpgrp() == ROOT_PGID){
        // 如果是在myshell终端所在的进程组
        // 即没有新建子进程
        // 则fork后执行
        pid_t pid = fork();
        if(pid < (pid_t)0){
            perror("Fork failure");
            ExitShell(EXIT_FAILURE);
        }
        if(pid == 0){
            setpgid(pid, pid);
            SigDefault();
            cpa = GetCharPtrArray(parsed);
            atexit([](){FreeCharPtrArray(cpa);});
            execvp(cpa[0], cpa);
            cout<<"Did not find program "<<cpa[0]<<endl;
            exit(0);
        }
        else{
            setpgid(pid, pid);
            // 设置为前台进程
            tcsetpgrp(STDIN_FILENO, pid);
            tcsetpgrp(STDOUT_FILENO, pid);
            int status;
            int rt = waitpid(pid, &status, WCONTINUED|WUNTRACED);
            // 回复前台进程
            tcsetpgrp(STDOUT_FILENO, getpgrp());
            tcsetpgrp(STDIN_FILENO, getpgrp());
            if (rt == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            // 判断返回结果
            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
                // 放在后台进程中
                JOBS.push_back(Job(pid, parsed[0]));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        }
    }
    else{
        // 如果已经在一个新的子进程
        // 直接执行
        cpa = GetCharPtrArray(parsed);
        atexit([](){FreeCharPtrArray(cpa);});
        execvp(cpa[0], cpa);
        cout<<"Did not find program "<<cpa[0]<<endl;
        exit(0);
    }
    
}
// 执行命令
RTCODE Execute(vector<string> parsed){
    if(parsed.empty())
        return RT_SUCCESS;
    if(isVariableAssignment(parsed[0])){
        AssignVar(parsed[0]);
        return RT_SUCCESS;
    }
    else if(isInnerCommand(parsed[0])) {
        vector<string> args(parsed.begin()+1, parsed.end());
        try{
            INNER_COMMAND.at(parsed[0])(args);
            return RT_SUCCESS;
        }
        catch (exception e) {
            cout<<"Fail at "<<parsed[0]<<endl;;
            return RT_ERROR;
        }
    }
    else{
        return ExecOuterCommand(parsed);
    }
}
