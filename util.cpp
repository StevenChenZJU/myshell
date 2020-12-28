#include "util.h"
using namespace std;
// 全局变量
int Job::max_id = 0; // 初始化
RTCODE LAST_RETURN=0; // $? for shell
int UMASK=0; // umask掩码
pid_t ROOT_PGID=0; // myshell终端的进程组编号
map<string, string> VAR; //用户变量
vector<string> ARGS; // 参数
vector<Job> JOBS; // 后台进程
// 函数定义
/**
 * 忽略myshell应该忽略的信号
 */
void SigIgnore(){
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
}
/**
 * 恢复myshell忽略了的信号
 * 用于子进程
 */
void SigDefault(){
    signal(SIGINT,  SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}
/**
 * 异常退出myshell
 */
void ExitShell(int rtcode){
    killpg(ROOT_PGID, 9); // kill the root
}
/**
 * 获取umask
 */
int GetUmask(){
    int mask = umask(0);
    umask(mask);
    return mask;
}
/**
 * 判断字符串是否是一个数字
 */
bool isNumber(string s){
    bool res = true;
    for(auto i: s){
        if(!isdigit(i)){
            res = false;
            break;
        }
    }
    return res;
}
/**
 * 判断给定的路径是否是一个文件夹的路径
 */
bool isDir(string path){
    struct stat buf;
    if(lstat(path.c_str(), &buf) < 0){
        cout<<"No such file!"<<endl;
        return false;
    }
    return S_ISDIR(buf.st_mode);
}
/**
 * 移动参数
 */
void ShiftN(int N){
    if(ARGS.size() >= N)
        ARGS = vector<string>(ARGS.begin() + N, ARGS.end());
    else
        ARGS.clear();
    // for(auto i: ARGS){
    //     cout<<i<<endl;
    // }
}
/**
 * 去掉注释
 */
void ProcessComment(string& s){
    auto index = s.find_first_of('#');
    if(index != string::npos){
        s = s.substr(0, index);
    }
    
}
// 去掉两端的空格
void Trim(string& s){
    auto index_start = s.find_first_not_of(' ');
    auto index_end = s.find_last_not_of(' ');
    if(!(index_start == string::npos) && !(index_end == string::npos))
        s = s.substr(index_start, index_end - index_start + 1);
}
// 处理多余的""双引号
string ParseQuote(string input){
    regex pattern("\"(.*)\"");
    return regex_replace(input, pattern, "$1"); // remove quating
}
// 替换变量
string ReplacingVar(string input){
    string res;
    while(!input.empty()){
        int start = input.find_first_of('$');
        // 寻找$
        if(start == string::npos){
            res = res + input;
            input = "";
        }
        else{
            string var_name;
            res = res + input.substr(0, start);
            int end = input.find_first_of('$', start+1);
            // 寻找下一个$
            if(end == string::npos){
                var_name = input.substr(start+1);
                input = "";
            }
            else{
                var_name = input.substr(start + 1, end - start - 1);
                input = input.substr(end);
            }
            // 判断是否是$N,是的话替换参数
            if(isNumber(var_name)){
                int i = ARGS.size() + 1;
                try{
                    i = stoi(var_name);
                } catch (exception e){
                    cout<<"Can not interpret "<<var_name<<endl;
                }
                res += ARGS.size() <= i ? "" : ARGS[i];
            }
            // 判断是否是$?
            else if(var_name == string("?")){
                res += to_string(LAST_RETURN);
            }
            // 判断是否是用户变量
            else if(VAR.find(var_name) == VAR.end()){
                // 如果不是用户变量并且不是环境变量,出错
                if(!getenv(var_name.c_str()))
                    throw invalid_argument("Unknown Varaible:"+var_name);
                else{
                    res += string(getenv(var_name.c_str()));
                }
            }
            // 是用户变量
            else{
                res += VAR.at(var_name);
            }
        }
    }
    return res;
}
// 分割字符串
vector<string> Parse(string input, char delim){
    vector<string> tokens;    
    // 把所有\t变为空格
    regex pattern_t("\t");
    input = regex_replace(input, pattern_t, " ");
    string intermediate; 
    stringstream check(input); 
    // 按照delim分隔, 默认' '
    while(getline(check, intermediate, delim)) 
    { 
        if(!intermediate.empty())
            tokens.push_back(ReplacingVar(ParseQuote(intermediate))); 
    } 
    return tokens;
}
// 获取绝对路径
string GetAbsolutePath(string relativePath){
    char realPath[PATH_MAX];
    string absolutePath;
    if(realpath(relativePath.c_str(),realPath))
        absolutePath = string(realPath);

    return absolutePath;
}
// 获取相对于HOME的相对路径
// 默认传入的是绝对路径
string GetRelativePath(string absolutePath){
    string HOME = getenv("HOME");
    return absolutePath.replace(0, HOME.length(), "~");
}
// 判断是否是参数赋值
bool isVariableAssignment(string str){
    auto index = str.find('=');
    return str.length()>2 
            && index != string::npos
            && index != 0;
}
// 参数赋值
void AssignVar(string str){
    auto index = str.find('=');
    string var = str.substr(0, index);
    string value = str.substr(index+1);
    VAR.insert({var, value});
}

