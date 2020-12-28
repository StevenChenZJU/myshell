#ifndef _UTIL_H_
#define _UTIL_H_

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <regex>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/limits.h>
#include <ctime>
#include <dirent.h>
#define RT_SUCCESS 0
#define RT_ERROR 1
#define RT_ESCAPE 255
#define PAGER "${PAGER:-more}"

typedef int RTCODE;
typedef RTCODE (*FUNCTION_TYPE)(std::vector<std::string> &);
// 后台进程状态
enum JobStatus
{
    Stopped,
    Running,
    Done
};
// 后台进程信息
struct Job
{
    static int max_id;
    int id; // id号,逐步递增
    pid_t pgid; // 所在进程组编号
    JobStatus status; // 状态
    std::string command; // 指令
    Job(pid_t pgid, std::string command) : pgid(pgid), status(Stopped), command(command), id(++max_id) {}
    // 转换为字符串输出
    std::string toString()
    {
        return "[" + std::to_string(id) + "]\t" + std::to_string(pgid) + "\t" + std::string(status ? "Running" : "Stopped") + "\t\t" + command;
    }
};
// 全局变量声明
extern RTCODE LAST_RETURN; // $? for shell
extern int UMASK;
extern pid_t ROOT_PGID;
extern std::map<std::string, std::string> VAR;
extern std::vector<std::string> ARGS;
extern std::vector<Job> JOBS;
// 函数声明
void SigIgnore();
void SigDefault();
void ExitShell(int rtcode);
int GetUmask();
bool isNumber(std::string s);
bool isDir(std::string path);
void ShiftN(int N);
void ProcessComment(std::string& s);
void Trim(std::string& s);
std::string ParseQuote(std::string input);
std::string ReplacingVar(std::string input);
std::vector<std::string> Parse(std::string input, char delim = ' ');
std::string GetAbsolutePath(std::string relativePath);
std::string GetRelativePath(std::string absolutePath);
bool isVariableAssignment(std::string str);
void AssignVar(std::string str);

#endif