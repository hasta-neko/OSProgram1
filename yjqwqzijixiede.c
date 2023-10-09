#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<signal.h>
pid_t pid = -1;


char** single_cmd(char* command){
    while(command[0] == ' ') command++;
    char* cmd_remain = strtok(command, " ");
    int cnt_str = 0;
    char** args = (char**)malloc(sizeof(char*) * 30);
    memset(args, 0, sizeof(char*) * 30);
    while(cmd_remain){
        char* cmd_cur = cmd_remain; // cmd_cur指向cmd_remain
        cmd_remain = strtok(NULL, " "); // cmd_remain指向下一个空格
        while(cmd_cur[0] == ' ') cmd_cur++; // 删除cmd_cur的前面的空格
        if (cnt_str == 0){
            
        }
    }


int main(int argc, char *argv[]){
    pid = getpid();
    while(1){
        printf("## JCshell [%d] ## ", pid);
        char input[100] = {'\0'}; // 用于存储用户输入的数据
    }
    
}