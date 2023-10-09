#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<string.h>
#include<signal.h>
pid_t pid = -1;

char* buf[6] = {0};
int buf_count = 0;
pid_t pids[6] = {0};
int pid_cnt;

int *pipes[6];
int command_count = 0;
int child_count = 0;
struct proc {
    int PID;
    char CMD[100];
    char STATE;
    int EXCODE;
    char EXSIG[20];
    int PPID;
    char USER[6];
    char SYS[6];
    int VCTX;
    int NVCTX;
};

void sigint_handler(int sig){
    if(sig == SIGINT ){
        if(pid == getpid()){
            printf("\n");
            fclose(stdin);
        }
    }
}

void sigintchild_handler(int sig){
    printf("child sigint\n");
    exit(1);
}


struct proc print_proc(pid_t cpid){
        if(cpid == 0) return (struct proc){0};
        struct proc p;
        // pid_t cpid = wait(NULL);
        // pid_t cpid = getpid();
        char prefix[100] = "/proc/";
        sprintf(prefix, "/proc/%d/stat", cpid);
        
        
        FILE* fp = fopen(prefix, "r");
        if(fp == NULL){
            printf("JCshell: \'%d\': No such process\n", cpid);
            return (struct proc){0};
        }
        char waste[10];
        fscanf(fp, "%d %s %c %d", &p.PID, p.CMD, &p.STATE, &p.PPID);
        for(int i=0 ;i<9;i++){
            fscanf(fp, "%s", waste);
        }
        fscanf(fp, "%s %s", p.USER, p.SYS);
        fclose(fp);

        
        
        sprintf(prefix, "/proc/%d/status", cpid);
        fp = fopen(prefix, "r");
        char title[20];
        if(fp == NULL){
            printf("JCshell: \'%d\': No such process\n", cpid);
            return (struct proc){0};
        }
        while(fscanf(fp, "%s", title) != EOF){
            if(strcmp(title, "voluntary_ctxt_switches:") == 0){
                fscanf(fp, "%d", &p.VCTX);
            }else if(strcmp(title, "nonvoluntary_ctxt_switches:") == 0){
                fscanf(fp, "%d", &p.NVCTX);
            }
        }

        fclose(fp);

        return p;
        // exit(0);
    
}

void sigchld_handler(int sig){
    if(sig == SIGCHLD){
        struct proc p = print_proc(pids[pid_cnt]);

        pid_cnt++;
        if(child_count != command_count){
            close(pipes[child_count][0]);
            close(pipes[child_count][1]);
            child_count++;
        }
        wait(&p.EXCODE);
        if(p.EXCODE == SIGINT){
            strcpy(p.EXSIG, "Interrupt");
        }else if(p.EXCODE == SIGKILL){
            strcpy(p.EXSIG, "Killed");
        }else if(p.EXCODE == 0){
            strcpy(p.EXSIG, "\0");
        }else{
            strcpy(p.EXSIG, "Other Signal");
        }
        char* proc_static = (char*)malloc(sizeof(char)*100);
        sprintf(proc_static,"(PID)%d (CMD)%s (STATE)%c (EXCODE)%d (EXSIG)%s (PPID)%d (USER)%s (SYS)%s (VCTX)%d (NVCTX)%d\n", p.PID, p.CMD, p.STATE,p.EXCODE, p.EXSIG, p.PPID, p.USER, p.SYS, p.VCTX, p.NVCTX);
        buf[buf_count] = proc_static;
        buf_count++;
    }
}

char** analysis_multi(char* input, int* cnt){
    char* temp2 = strtok(input, "|");
    int count = 0;
    char* cmd;
    char** args = (char**)malloc(sizeof(char)*5);
    memset(args, 0, sizeof(char)*6);
    while(temp2){
        char* temp1 = temp2;
        temp2 = strtok(NULL, "|");
        while(temp1[0] == ' '){
            temp1++;
        }
        if(count == 0){
            if(temp2){
                cmd = (char*)malloc(sizeof(char)*(temp2-temp1+1));
            }else{
                cmd = (char*)malloc(sizeof(char)*(input+1024-temp1));
            }
            strcpy(cmd, temp1);
            
            args[0] = cmd;
        }else if(!temp2 && count !=0){
            args[count] = (char*)malloc(sizeof(char)*(input+1024-temp1));
            strcpy(args[count], temp1);
            
        }else{
            args[count] = (char*)malloc(sizeof(char)*(temp2-temp1+1));
            strcpy(args[count], temp1);
        }
        count ++;
        // printf("%s\n", args[count-1]);
        if(count > 5){
            printf("JCshell: too many commands\n");
            return NULL;
        }
    }
    // printf("%d\n", count);
    *cnt = count;
    return args;
}

// single command no pipe
char** analysis_single(char* command){
    while(command[0] == ' '){
        command++;
    }
    // strtok(char s[], const char *delim)
    // 首次调用时s为要分解的字符串，delim为分隔符字符串。
    // 之后s为NULL
    char* temp2 = strtok(command, " ");
    int count = 0; // count string number
    char* cmd;
    char** args = (char**)malloc(sizeof(char)*30);
    memset(args, 0, sizeof(char)*31); // why 31?
    while(temp2){
        char* temp1 = temp2;
        temp2 = strtok(NULL, " ");
        while(temp1[0] == ' '){
            temp1++;
        }
        // 从这里开始没懂
        if(count == 0){
            if(temp2){
                cmd = (char*)malloc(sizeof(char)*(temp2-temp1+1)); // why
            }else{
                cmd = (char*)malloc(sizeof(char)*(command+1024-temp1));
            }
            strcpy(cmd, temp1);
            
            args[0] = cmd;
        }else if(!temp2 && count !=0){
            args[count] = (char*)malloc(sizeof(char)*(command+1024-temp1));
            strcpy(args[count], temp1);
            
        }else{
            args[count] = (char*)malloc(sizeof(char)*(temp2-temp1+1));
            strcpy(args[count], temp1);
        }
        count ++;
        // 到这里 ↑ 没懂
        if(count > 30){
            printf("JCshell: too many arguments\n");
            return NULL;
        }
    }
    if(memcmp(cmd, "exit", 4) == 0){
        if(count > 1){
            printf("\"exit\" with too many arguments\n");
            return NULL;
        }
        else{
            printf("JCshell: Terminated\n");
            exit(0);
        }
    }
    
    return args;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
    pid = getpid();
    while (1){
        printf("## JCshell [%d] ## ", pid);
        char input[1024] = {'\0'};

        for(int i=0; i<6;i++){
            if(buf[i]){
                free(buf[i]);
                buf[i] = NULL;
            }
        }
        buf_count = 0;

        freopen("/dev/tty", "r", stdin);
        gets(input, sizeof(input), stdin);
        if(input[0] == '\0') continue;

        command_count = 0;
        char** commands = analysis_multi(input, &command_count);
        if(commands == NULL){ 
            continue;
        }
        pid_cnt = 0;
        
        char** cmds[6];
        int valid = 1;
        for(int i=0;i<command_count;i++){
            cmds[i] = analysis_single(commands[i]);
            // printf("%c: command\n", commands[i][0]);
            if(cmds[i] == NULL) {
                valid = 0;
                break;
            }
            // printf("1");
        }
        if(!valid) continue;
        // char** args = cmds[0];

        
        

        for(int i=0; i<command_count;i++){
            pipes[i] = (int*)malloc(sizeof(int)*2);
            pipe(pipes[i]);
        }

        for(int i=0; i<command_count; i++){ 
            // printf("cmd %d\n", i);
            pid_t p = fork();
            pids[i] = p;
            if(p != 0){
                continue;
            }else{
                signal(SIGINT, sigintchild_handler);
                for(int j=0; j<command_count; j++){
                    if(j!=i-1){
                        close(pipes[j][0]);
                    }
                    if(j!=i){
                        close(pipes[j][1]);
                    }
                }

                if(i != 0){
                    
                    if(dup2(pipes[i-1][0], STDIN_FILENO) ==-1){
                        printf("dup2 stdin error\n");
                    }
                }
                if(i != command_count-1){
                    
                    if(dup2(pipes[i][1], STDOUT_FILENO) == -1){
                        printf("dup2 stdout error\n");
                    }
                    
                }

                if(execvp(cmds[i][0], cmds[i]) == -1){
                    printf("JCshell: \'%s\': No such file or directory\n", cmds[i][0]);
                    exit(0);
                }

                
            }
        }

        while(child_count != command_count);
        // 
        for(int i=0; buf[i];i++){
            printf("%s", buf[i]);
        }
    }
}