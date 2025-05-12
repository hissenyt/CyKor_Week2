    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <dirent.h>
    #include <errno.h>

    #define CMD_MAX_LEN 100
    #define DRC_MAX_LEN 200
    #define MAX_LEN 1000
    #define ARG_MAX_LEN 20
    #define CMD_PASS 1
    #define CMD_FAIL 0

    char drc[DRC_MAX_LEN] = {0,};
    //char kw[KW_MAX_LEN] = {0,};

    int execute_command(char *cmd);
    int seperate_command(char *cmd);
    void pipeline(char **cmds, int i, int n);
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
            char *argv[ARG_MAX_LEN];
            for (int i = 0; i < argn; i++){
                argv[i] = arg[i];
            }
            argv[argn] = NULL;

            pid_t pid = fork();
            if (pid == 0){
                execvp(arg[0], argv);
                exit(1);
            }
            else {
                int stat;
                waitpid(pid, &stat, 0);
                return (WIFEXITED(stat) && WEXITSTATUS(stat) == 0) ? CMD_PASS : CMD_FAIL;
            }
        }
    }

    int seperate_command(char *cmd){
        if (strstr(cmd, "&&") == NULL && strstr(cmd, "&")){
            char *split[ARG_MAX_LEN] = {0,}; int splitn = 0; char *tokens; int i;
            tokens = strtok(cmd, "&");
            while (tokens != NULL && splitn < ARG_MAX_LEN) {
                split[splitn++] = tokens;
                tokens = strtok(NULL, "&");
            }
            split[splitn] = NULL;
            if (fork() == 0){
                seperate_command(split[0]);
                exit(0);
            }
            return CMD_PASS;
        }
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
            char *split[ARG_MAX_LEN] = {0,}; int splitn = 0; char *tokens; int i;
            tokens = strtok(cmd, "|");
            while (tokens != NULL && ARG_MAX_LEN) {
                split[splitn++] = tokens;
                tokens = strtok(NULL, "|");
            }
            split[splitn] = NULL;
            pid_t pid = fork();
            if (pid == 0){
                pipeline(split, 0, splitn);
            }
            else {
                int stat;
                waitpid(pid, &stat, 0);
            }
        }

        else return execute_command(cmd);
    }

    void pipeline(char **cmds, int i, int n){
        if (i == n - 1) {execute_command(cmds[i]); exit(0);}
        else {
            int fd[2];
            if (pipe(fd) == -1) {
                fprintf(stderr, "pipe error: %s\n", strerror(errno));
                exit(0);
            }
            pid_t pid = fork();
            if (pid == 0) {
                dup2(fd[1], 1);
                close(fd[0]);
                close(fd[1]);
                execute_command(cmds[i]);
                exit(0);
            }
            else {
                dup2(fd[0], 0);
                close(fd[0]);
                close(fd[1]);
                pipeline(cmds, i + 1, n);
            }
        }
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
            printf("%s\n", dirp->d_name);
        }
        printf("\n");
        closedir(dp);
        return CMD_PASS;
    }