#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#ifndef NARGS
#define NARGS 4
#endif



int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if(pid == -1){
        exit(-1);
    }
    if(pid == 0){
        int i = 2;
        while(i <= NARGS + 1){
            size_t n = 0;
            int read = getline(&argv[i], &n, stdin);
            if(read == -1){
                break;
            }
            argv[i][read -1] = '\0';
			argv[i + 1] = NULL;
            i= i+1;
        }
        execvp(argv[1], argv + 1);
        exit(0);
        }
    else{
        wait(NULL);
        exit(0);
    }

}