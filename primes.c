#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1



int es_primo(int num) {
    if (num <= 1) return 0; // 0 y 1 no son primos

    for (int i = 2; i <= num/2; i++) {
        if (num % i == 0) {
            return 0; // No es primo
        }
    }
    return 1; // Es primo
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <limite>\n", argv[0]);
        return 1;
    }

    int limit = atoi(argv[1]);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("Error al crear el pipe");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        return 1;
    } else if (pid == 0) {
        // Proceso hijo
        close(pipe_fd[READ_END]); // Cerrar el extremo de lectura

        for (int num = 2; num <= limit; num++) {
            int result = es_primo(num);
            if (result == 1) {
                int returned_value = write(pipe_fd[WRITE_END], &num, sizeof(num));
				returned_value = returned_value;
            }
        }

        close(pipe_fd[WRITE_END]);
        //printf("Soy el proceso hijo (%d)\n", getpid());
        return 0;
    } else {
        // Proceso padre
        close(pipe_fd[WRITE_END]); // Cerrar el extremo de escritura

        //printf("Soy el proceso padre (%d)\n", getpid());
        printf("Los números primos hasta %d son:\n", limit);
        int num;
        while (read(pipe_fd[READ_END], &num, sizeof(num)) > 0) {
            printf("primo %d\n", num);
        }
		fflush(stdout);
        close(pipe_fd[READ_END]);

        wait(NULL); // Esperar a que el hijo termine
    }

    return 0;
}

