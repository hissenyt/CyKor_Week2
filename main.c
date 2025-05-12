#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#define CMD_MAX_LEN 100
#define DRC_MAX_LEN 200
#define ARG_MAX_LEN 20
#define CMD_PASS 1
#define CMD_FAIL 0

char drc[DRC_MAX_LEN] = {0,};
//char kw[KW_MAX_LEN] = {0,};

int execute_command(char *cmd);
int seperate_command(char *cmd);
int cmd_pwd(void);
int cmd_cd(char *dst);
int cmd_ls(void);
//int cmd_echo(void);
void cmd_exit(void);

int main() {
    char cmd[CMD_MAX_LEN];

    while (1){
        getcwd(drc, sizeof(drc));   // 현재 디렉토리 값을 drc에 저장함.
        printf("hissen@HISSEN:%s$ ", drc);
       
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {   // 입력 오류가 발생한 경우우
            printf("Error detected.\n");
            break;
        }

        cmd[strcspn(cmd, "\n")] = 0;

        if (strcmp(cmd, "exit") == 0) break;  // exit 명령어가 들어온 경우우
        seperate_command(cmd);

    }

    return 0;
}

int execute_command(char *cmd) {

    char *arg[ARG_MAX_LEN] = {0,};
    int argn = 0;
    char *token;
    token = strtok(cmd, " ");
    argn = 0;
    //sscanf(cmd, "%s", kw);
    while (token != NULL && argn < ARG_MAX_LEN){
        arg[argn++] = token;
        token = strtok(NULL, " ");
    }
    arg[argn] = NULL;

    if (strcmp(arg[0], "help") == 0) {
        printf("Available commands: help, exit\n");
    } 
    else if (strcmp(arg[0], "pwd") == 0){
        return cmd_pwd();
    }
    else if (strcmp(arg[0], "cd") == 0){
        return cmd_cd(arg[1]);
    }
    else if (strcmp(arg[0], "ls") == 0){
        return cmd_ls();
    }
    else {
        printf("Command not recognized: '%s'\n", cmd);
        return CMD_FAIL;
    }
}

int seperate_command(char *cmd){
    if (strstr(cmd, ";")){
        char *split[ARG_MAX_LEN] = {0,}; int splitn = 0; char *tokens; int i;
        tokens = strtok(cmd, ";");
        while (tokens != NULL && splitn < ARG_MAX_LEN) {
            split[splitn++] = tokens;
            tokens = strtok(NULL, ";");
        }
        split[splitn] = NULL;
        for (i = 0; i<splitn; i++){
            seperate_command(split[i]);
        }
        return CMD_PASS;
    }
    else if (strstr(cmd, "||")){
        char *split[ARG_MAX_LEN] = {0,}; int splitn = 0; char *tokens; int i;
        tokens = strtok(cmd, "||");
        while (tokens != NULL && ARG_MAX_LEN) {
            split[splitn++] = tokens;
            tokens = strtok(NULL, "||");
        }
        split[splitn] = NULL;
        for (i = 0; i<splitn; i++){
            if (seperate_command(split[i]) == CMD_PASS) {return CMD_FAIL; break;}
            else continue;
        }
        if (i == splitn - 1) return CMD_PASS;
    }
    else if (strstr(cmd, "&&")){
        char *split[ARG_MAX_LEN] = {0,}; int splitn = 0; char *tokens; int i;
        tokens = strtok(cmd, "&&");
        while (tokens != NULL && ARG_MAX_LEN) {
            split[splitn++] = tokens;
            tokens = strtok(NULL, "&&");
        }
        split[splitn] = NULL;
        for (i = 0;i<splitn;i++){
            if (execute_command(split[i]) == CMD_FAIL) {return CMD_FAIL; break;}
            else continue;
        }
        if (i == splitn - 1) return CMD_PASS;
    }
    else if (strstr(cmd, "|")){
        
    }

    else return execute_command(cmd);
}

int cmd_pwd(void){
    printf("%s\n", drc);
    return CMD_PASS;
}

int cmd_cd(char *dst){
    //if (strcmp(dst, "/") == 0) {chdir("/");return;}
    if (strcmp(dst, "~") == 0) {chdir("//home/hissen"); getcwd(drc, sizeof(drc)); return CMD_PASS;}     // 어케 해결하지..
    // if (strcmp(dst, "..") == 0){
        
    // }
    //char fdst[DRC_MAX_LEN];
    //strcat(drc, "/");
    //strcat(drc, dst);
    if (chdir(dst) == -1) {
        printf("-bash: cd: %s: No such file or directory\n", dst);
        return CMD_FAIL;
    }
    else {
        //printf("Successfully moved to %s\n", dst);
        getcwd(drc, sizeof(drc));
        return CMD_PASS;
    }
}

int cmd_ls(void){
    DIR *dp;
    struct dirent *dirp;

    dp = opendir(".");
    while (dirp = readdir(dp)){
        printf("%s\t", dirp->d_name);
    }
    printf("\n");
    closedir(dp);
    return CMD_PASS;
}