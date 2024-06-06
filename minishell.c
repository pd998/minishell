#include <stdio.h> 
#include <dirent.h> 
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

pid_t forground_pid;
char userName[500];
char path[5000];

void removeLeading(char *str, char *str1)
{
    int idx = 0, j, k = 0;
    while (str[idx] == ' ' || str[idx] == '\t')
    {
        idx++;
    }
 
    for (j = idx; str[j] != '\0' && str[j] != '\n'; j++)
    {
        str1[k] = str[j];
        k++;
    }
 
    str1[k] = '\0';
}

void child_handler(int sig)
{
    int olderrno = errno;
    pid_t background_pid;
    while ((background_pid = waitpid(-1, NULL, WNOHANG)) > 0 && background_pid!=forground_pid) {
        char buf[50000];
        sprintf(buf, "\npid %d done\nminishell:%s:%s$ ", background_pid, userName, path);
        write(2, buf, strnlen(buf,sizeof(buf)));
    }
    errno = olderrno;
}


int main(int argc, char **argv) 
{ 
    struct passwd *pw;
    pw = getpwuid(getuid());

    struct sigaction action;
    action.sa_handler = child_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &action, NULL);

    if(pw)
        strcpy(userName ,pw->pw_name);
    else
        sprintf(userName, "%d", getuid());

    while (1)
    {
        int background = 0;
        pid_t pid;
        if(getcwd(path, 5000) == NULL)
            strcpy(path, "???");
        fprintf(stderr, "minishell:%s:%s$ ", userName, path);

        int MAX_LEN = 4095;
        char *cmd = malloc(sizeof(char) * (MAX_LEN+1));
        int length = 0;
        
        char ch;
        ch = getchar();
        while ((ch != '\n')){
            if(length==MAX_LEN-1) { 
                MAX_LEN *= 2; 
                cmd = realloc(cmd, sizeof(char) * (MAX_LEN+1));
            }
            cmd[length] = ch;
            length++;
            ch = getchar();
        }
        cmd[length] = '\0';
        char* token = strtok(cmd, " \t");
        int maxNumberOfArg = 16;
        char **tokenizeCmd = malloc((maxNumberOfArg+1) * sizeof(char*));
        int numberOfArg = 0;
        while (token != NULL) {
            tokenizeCmd[numberOfArg] = malloc((strlen(token) + 1)  * sizeof(char));
            removeLeading(token, tokenizeCmd[numberOfArg]);
            token = strtok(NULL, " \t");
            if(strlen(tokenizeCmd[numberOfArg]) != 0)
                numberOfArg++;
            if(numberOfArg>=maxNumberOfArg-1)
            {
                maxNumberOfArg *= 2;
                tokenizeCmd = realloc(tokenizeCmd, sizeof(char*) * (maxNumberOfArg+1));
            }
        }

        if(numberOfArg==0 || (numberOfArg==1 && strcmp(tokenizeCmd[0], "&")==0)){
            free(cmd);
            for(int i=0;i<numberOfArg;i++)
                free(tokenizeCmd[i]);
            free(tokenizeCmd);
            continue;
        }
        
        if(strcmp(tokenizeCmd[numberOfArg-1], "&")==0){
            background = 1;
            free(tokenizeCmd[numberOfArg-1]);
            tokenizeCmd[numberOfArg-1] = NULL;
            numberOfArg--;
        }else if(tokenizeCmd[numberOfArg-1][strlen(tokenizeCmd[numberOfArg-1])-1]=='&'){
            background = 1;
            tokenizeCmd[numberOfArg-1][strlen(tokenizeCmd[numberOfArg-1])-1] = '\0';
        }
        
        if(strcmp(tokenizeCmd[0], "exit")==0){
            free(cmd);
            for(int i=0;i<numberOfArg;i++)
                free(tokenizeCmd[i]);
            free(tokenizeCmd);
            if(numberOfArg==1 && background==0){
                break;
            }
            fprintf(stderr, "minishell: exit: invalid arguments\n");
            continue;
        }

        if(strcmp(tokenizeCmd[0], "cd")==0){
            if(numberOfArg==1 || (numberOfArg==2 && strcmp(tokenizeCmd[1], "~")==0))
                chdir(pw->pw_dir);
            else if(numberOfArg==2){
                if (chdir(tokenizeCmd[1]) != 0)
                    fprintf(stderr,"minishell: %s: %s: %s", tokenizeCmd[0], strerror(errno), tokenizeCmd[1]);
            }
            else
                fprintf(stderr,"it takes up to one arguments");
            
            free(cmd);
            for(int i=0;i<numberOfArg;i++)
                free(tokenizeCmd[i]);
            free(tokenizeCmd);
            continue;
        }
        tokenizeCmd = realloc(tokenizeCmd, sizeof(char*) * (numberOfArg+1));
        numberOfArg++;
        tokenizeCmd[numberOfArg-1] = NULL;
        if ((pid = fork()) < 0) {
            perror("fork");
        }else if (pid == 0) {
            free(cmd);
            if(execvp(tokenizeCmd[0], tokenizeCmd)<0)
                fprintf(stderr,"minishell: %s: %s\n", tokenizeCmd[0], strerror(errno));
            for(int i=0;i<numberOfArg;i++)
                free(tokenizeCmd[i]);
            free(tokenizeCmd);
            exit(1);
        }

        if(background==0){
            forground_pid = pid;
            waitpid(pid,NULL,0);
            free(cmd);
            for(int i=0;i<numberOfArg;i++)
                free(tokenizeCmd[i]);
            free(tokenizeCmd);
            continue;
        }

        strcpy(cmd, tokenizeCmd[0]);

        for(int i=1;i<numberOfArg-1;i++)
        {
            strcat(cmd, " ");
            strcat(cmd, tokenizeCmd[i]);
        }

        fprintf(stderr,"pid %d cmd: %s\n", pid, cmd);
            
        free(cmd);
        for(int i=0;i<numberOfArg;i++)
            free(tokenizeCmd[i]);
        free(tokenizeCmd);
    }

    return 0; 
} 